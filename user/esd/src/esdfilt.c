#include "esd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

int main(int argc, char **argv)
{
    octet buf[ESD_BUF_SIZE];
    int sock = -1, rate = ESD_DEFAULT_RATE;
    int length = 0, total = 0, arg = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY;
    esd_format_t format = 0;

    int half = 0, twice = 0;
    signed short* data;
    int samp;

    FILE *target = NULL;
    char *host = NULL;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf( "usage:\n" );
	    printf( "\t%s [-s server ][-b] [-m] [-r freq] [-half|-double] [file]\n",
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
	} else if (target) {
	    printf("%s: ignoring extra file '%s'\n", argv[0], argv[arg]);
	} else if ( !strcmp( "-half", argv[ arg ] ) ) {
	    half = 1;
	    printf( "halving data\n" );
	} else if ( !strcmp( "-double", argv[ arg ] ) ) {
	    twice = 1;
	} else {
	    target = fopen( argv[arg], "w" );
	    if (!target) {
		printf("%s: couldn't write to '%s'\n", argv[0], argv[arg]);
	    }
	}
    }

    /*
    if ( !target ) {
	target = stdout;
    }
    */
    
    format = bits | channels | mode | func;
    printf( "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    sock = esd_filter_stream( format, rate, host, argv[0] );
    if ( sock <= 0 ) 
	return 1;
    
    while ( ( length = read( sock, buf, ESD_BUF_SIZE ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( length < 0 ) 
	    break;

	/* half the data if desired */
	if ( half ) {
	    data = (signed short *) buf;
	    while ( (octet *) data < buf + length ) {
		samp = *data / 2; 
		*data = samp; 
		data++;
	    }
	}

	/* double the data if desired */
	if ( twice ) {
	    data = (signed short *) buf;
	    while ( (octet *) data < buf + length ) {
		samp = *data * 2; 
		*data = ( samp > SHRT_MAX ) ? SHRT_MAX : samp;
		data++;
	    }
	}

	/* share the altered data */	
	if( target && ( fwrite( buf, 1, length, target ) <= 0 ) )
	    return 1;
	write( sock, buf, length );

	printf( "\rtotal bytes streamed: %d", total );
	fflush( stdout );
	total += length;
    }
    close( sock );
    
    return 0;
}
