#include "esd.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = ESD_DEFAULT_RATE;
    int length = 0, arg = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    FILE *source = NULL;
    char *host = NULL;
    char *name = NULL;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf("usage:\n\t%s [-s server] [-n name] [-b] [-m] [-r freq] < file\n",
		   argv[0]);
	    exit(0);
	}
	else if ( !strcmp( "-n", argv[ arg ] ) )
	    name = argv[ ++arg ];
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
	} else if (source) {
	    printf("%s: ignoring extra file '%s'\n", argv[0], argv[arg]);
	} else {
	    name = argv[ arg ];
	    if ( (source = fopen( name, "r" )) == NULL ) {
		printf( "%s: couldn't open sound file: %s\n", argv[0], name );
		return 1;
	    }
	}
    }
    
    /* use stdin if no file specified */
    if (!source) {
	source = stdin;
    }

#ifdef __EMX__
    /* Switch input stream to binary mode under EMX */
    _fsetmode (source, "b");
#endif

    format = bits | channels | mode | func;
    fprintf( stderr, "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    /* sock = esd_play_stream( format, rate ); */
    sock = esd_play_stream_fallback( format, rate, host, name );
    if ( sock <= 0 ) 
	return 1;

    while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( write( sock, buf, length ) <= 0 ) 
	    return 1;
    }
    close( sock );

    return 0;
}

