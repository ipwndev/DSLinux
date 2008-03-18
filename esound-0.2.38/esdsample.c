#include "esd.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* prototype(s) */
void clean_exit(int signum);

volatile int terminate = 0;

void clean_exit(int signum) {
    fprintf( stderr, "received signal %d: terminating...\n", signum );
    terminate = 1;
    return;
}

void usage(char *name) {
    printf("usage:\n\t%s [-s server] [-d [-b] [-m] [-r freq]] < file\n",
	   name);
    exit(0);
}

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = ESD_DEFAULT_RATE;
    int arg = 0, length = 0, total = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    int sample_id = 0, confirm_id = 0, reget_sample_id = 0;
    FILE *source = NULL;
    struct stat source_stats;
    char *host = NULL;
    char *name = NULL;
    char filename[ ESD_NAME_MAX ] = "";
    int cache_mode = 0; /* cache mode, 1 = data, 0 = file, 2 = existing */
    
    if (argc == 1)
	usage(argv[0]);

    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	    usage(argv[0]);
	else if ( !strcmp( "-s", argv[ arg ] ) )
	    host = argv[ ++arg ];
	else if ( !strcmp( "-b", argv[ arg ] ) )
	    bits = ESD_BITS8;
	else if ( !strcmp( "-m", argv[ arg ] ) )
	    channels = ESD_MONO;
	else if ( !strcmp( "-d", argv[ arg ] ) )
	    cache_mode = 1;
	else if ( !strcmp( "-e", argv[ arg ] ) )
	    cache_mode = 2;
	else if ( !strcmp( "-r", argv[ arg ] ) )
	{
	    arg++;
	    rate = atoi( argv[ arg ] );
	} else {
	    break;
	}
    }

    /* filename is the next option */
    /* construct name */
    strncpy( filename, argv[0], ESD_NAME_MAX - 2 );
    strcat( filename, ":" );
    strncpy( filename + strlen( filename ), argv[ arg ], 
	     ESD_NAME_MAX - strlen( filename ) );
    name = argv[ arg ];


    /* if we see any of these, terminate */
    signal( SIGINT, clean_exit );
    signal( SIGKILL, clean_exit );
    signal( SIGPIPE, clean_exit );

	sock = esd_open_sound( host );
    if ( cache_mode == 1 ) {
	source = fopen( name, "r" );

	if ( source == NULL ) {
	    fprintf( stderr, "%s, sample file not specified\n", argv[ 0 ] );
	    return -1;
	}
    
	if ( sock <= 0 ) 
	    return 1;
	format = bits | channels | mode | func;
	fprintf( stderr, "opening socket, format = 0x%08x at %d Hz\n", 
		format, rate );
	
	stat( name, &source_stats );
	sample_id = esd_sample_cache( sock, format, rate, source_stats.st_size, filename );
	printf( "sample id is <%d>\n", sample_id );
    
	while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
	{
	    /* fprintf( stderr, "read %d\n", length ); */
	    if ( ( length = write( sock, buf, length)  ) <= 0 )
		return 1;
	    else
		total += length;
	}
	
	confirm_id = esd_confirm_sample_cache( sock );

	if ( sample_id != confirm_id ) {
	    printf( "error while caching sample <%d>: confirm returned %d\n",
		    sample_id, confirm_id );
	    exit( 1 );
	}

	printf( "sample <%d> uploaded, %d bytes\n", sample_id, total );
    } 
    else if ( cache_mode == 0) {
	if ( sock <= 0 ) 
	    return 1;
	
	sample_id = esd_file_cache( sock, argv[0], name );
	printf( "sample id is <%d>\n", sample_id );
    
	/* if we see any of these, terminate */
	if ( sample_id < 0 ) {
	    printf( "error while caching sample <%d>: confirm value != sample_id\n",
		    sample_id );
	    exit( 1 );
	}

	printf( "sample <%d> uploaded: %s\n", sample_id, filename );
    } else if (cache_mode == 2) {
	strncpy( filename, name, ESD_NAME_MAX );
    }

    reget_sample_id = esd_sample_getid( sock, filename );

if(cache_mode != 2) {
    printf( "reget of sample %s id is <%d>\n", filename, reget_sample_id );
    if(( reget_sample_id != sample_id && cache_mode != 2)
       || reget_sample_id < 0) {
	printf( "sample id's do not make sense! (cache mode %d)\n", cache_mode );
	exit( 1 );
    }
} else sample_id = reget_sample_id;

    printf( "name is \'%s\', id is %d.\n", filename, sample_id );

    printf( "press \'q\' <enter> to quit, <enter> to trigger.\n" );
    while ( !terminate ) {
	if ( getchar() == 'q' ) break;
	printf( "<playing sample>\n" );
	esd_sample_play( sock, sample_id );
    }

    esd_sample_free( sock, sample_id );

    printf( "closing down\n" );
    close( sock );

    return 0;
}
