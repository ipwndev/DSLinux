#include "esd.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = ESD_DEFAULT_RATE;
    int length = 0, arg = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_RECORD ;
    esd_format_t format = 0;

    FILE *target = NULL;
    char *host = NULL;
    char *name = NULL;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf("usage:\n\t%s [-s server] [-b] [-m] [-r freq] < file\n",argv[0]);
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
	} else if (target) {
	    printf("%s: ignoring extra file '%s'\n", argv[0], argv[arg]);
	} else {
	    name = argv[ arg ];
	    target = fopen( name, "w" );
	    if (!target) {
		printf("%s: Couldn't write to '%s'\n", argv[0], argv[arg]);
		name = NULL;
	    }
	}
    }
    
    if (!target) {
	target = stdout;
    }

    format = bits | channels | mode | func;
    fprintf( stderr, "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    /* sock = esd_record_stream( format, rate ); */
    sock = esd_record_stream_fallback( format, rate, host, name );
    if ( sock <= 0 ) 
	return 1;
    
    while ( ( length = read( sock, buf, ESD_BUF_SIZE ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if( fwrite( buf, 1, length, target ) <= 0 )
	    return 1;
    }
    close( sock );
    return 0;
}
