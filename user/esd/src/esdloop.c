#include "esd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include <signal.h>

/* prototype(s) */
void clean_exit(int signum);

volatile int terminate = 0;	/* signal will set this for a clean exit */

void clean_exit(int signum) {
    fprintf( stderr, "received signal %d: terminating...\n", signum );
    terminate = 1;
    return;
}

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = ESD_DEFAULT_RATE;
    int arg = 0, length = 0, total = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    int sample_id = 0;
    FILE *source = NULL;
    struct stat source_stats;
    char *host = NULL;
    char *name = NULL;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf("usage:\n\t%s [-s server ] [-b] [-m] [-r freq] < file\n",
		   argv[0]);
	    exit(0);
	}
	else if ( !strcmp( "-s", argv[ arg ] ) )
	    host = argv[ ++arg ];
	else if ( !strcmp( "-b", argv[ arg ] ) )
	    bits = ESD_BITS8;
	else if ( !strcmp( "-m", argv[ arg ] ) )
	    channels = ESD_MONO;
	else if ( !strcmp( "-r", argv[ arg ] ) )
	{
	    arg++;
	    rate = atoi( argv[ arg ] );
	} else {
	    name = argv[ arg ];
	    source = fopen( name, "r" );
	}
    }

    signal( SIGINT, clean_exit );

    if ( source == NULL ) {
	fprintf( stderr, "%s, sample file not specified\n", argv[ 0 ] );
	return -1;
    }
    
    format = bits | channels | mode | func;
    fprintf( stderr, "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    sock = esd_open_sound( host );
    if ( sock <= 0 ) 
	return 1;

    stat( name, &source_stats );
    sample_id = esd_sample_cache( sock, format, rate, source_stats.st_size, name );
    printf( "sample id is <%d>\n", sample_id );

    while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( ( length = write( sock, buf, length)  ) <= 0 )
	    return 1;
	else
	    total += length;
    }

    printf( "sample uploaded, %d bytes\n", total );
    esd_sample_loop( sock, sample_id );

    printf( "press <enter> to quit.\n" );
    getchar();

    /* TODO: make sample_free clean out all playing samples */
    esd_sample_stop( sock, sample_id );
    esd_sample_free( sock, sample_id );

    printf( "closing down\n" );
    close( sock );

    return 0;
}
