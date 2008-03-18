#include <esd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************************/
/* esdfile.c - audiofile wrappers for sane handling of files */

int esd_send_file( int esd, AFfilehandle au_file, int frame_length )
{
    /* data for transfer */
    char buf[ ESD_BUF_SIZE ];
    int frames_read;
    int buf_frames = ESD_BUF_SIZE / frame_length;

    while ( ( frames_read = afReadFrames( au_file, AF_DEFAULT_TRACK, 
					buf, buf_frames ) ) )
    {
	if ( write ( esd, buf, frames_read * frame_length ) <= 0)
	    return 1;
    }

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound sending file\n" );
    */

    return 0;
}

int esd_play_file( const char *name_prefix, const char *filename, int fallback )
{
    /* input from libaudiofile... */
    AFfilehandle in_file;
    int in_format, in_width, in_channels, frame_count;
    int in_compression;
    double in_rate;
    int bytes_per_frame;

    /* output to esound... */
    int out_sock, out_bits, out_channels, out_rate;
    int out_mode = ESD_STREAM, out_func = ESD_PLAY;
    esd_format_t out_format;
    char name[ ESD_NAME_MAX ] = "";

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound playing file\n" );
    */

    /* open the audio file */
    in_file = afOpenFile( filename, "rb", NULL );
    if ( !in_file )
	return 0;

    /* get audio file parameters */
    frame_count = afGetFrameCount( in_file, AF_DEFAULT_TRACK );
    in_channels = afGetChannels( in_file, AF_DEFAULT_TRACK );
    in_rate = afGetRate( in_file, AF_DEFAULT_TRACK );
    in_compression = afGetCompression ( in_file, AF_DEFAULT_TRACK );
    afGetSampleFormat( in_file, AF_DEFAULT_TRACK, &in_format, &in_width );

    if(getenv("ESDBG"))
    printf ("frames: %i channels: %i rate: %f format: %i width: %i\n",
    	        frame_count, in_channels, in_rate, in_format, in_width);

    /* This check was added because audiofile doesn't tell us whether or
     * not it will play compressed audio correctly. */
    if ( in_compression != AF_COMPRESSION_NONE )
    {
	/* fputs ("compressed audio not supported supported\n", stderr); */
	return 0;
    }

    /* convert audiofile parameters to EsounD parameters */
    if ( in_width == 8 )
	out_bits = ESD_BITS8;
    else if ( in_width == 16 )
	out_bits = ESD_BITS16;
    else
    {
	/* fputs ("only sample widths of 8 and 16 supported\n", stderr); */
	return 0;
    }

    bytes_per_frame = ( in_width  * in_channels ) / 8;

    if ( in_channels == 1 )
	out_channels = ESD_MONO;
    else if ( in_channels == 2 )
	out_channels = ESD_STEREO;
    else
    {
	/* fputs ("only 1 or 2 channel samples supported\n", stderr); */
	return 0;
    }

    out_format = out_bits | out_channels | out_mode | out_func;
    out_rate = (int) in_rate;

    /* construct name */
    if ( name_prefix ) {
	strncpy( name, name_prefix, ESD_NAME_MAX - 2 );
	strcat( name, ":" );
    }
    strncpy( name + strlen( name ), filename, ESD_NAME_MAX - strlen( name ) );

    /* connect to server and play stream */
    if ( fallback )
	out_sock = esd_play_stream_fallback( out_format, out_rate, NULL, name );
    else
	out_sock = esd_play_stream( out_format, out_rate, NULL, filename );

    if ( out_sock <= 0 )
	return 0;

    /* play */
    esd_send_file( out_sock, in_file, bytes_per_frame );

    /* close up and go home */
    close( out_sock );
    if ( afCloseFile ( in_file ) )
	return 0;

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound played file\n" );
    */

    return 1;
}

int esd_file_cache( int esd, const char *name_prefix, const char *filename )
{
    /* input from libaudiofile... */
    AFfilehandle in_file;
    int in_format, in_width, in_channels, frame_count;
    double in_rate;
    int bytes_per_frame;

    /* output to esound... */
    int out_bits, out_channels, out_rate;
    int out_mode = ESD_STREAM, out_func = ESD_PLAY;
    esd_format_t out_format;
    int length, sample_id, confirm_id;
    char name[ ESD_NAME_MAX ];

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound caching file\n" );
    */

    /* open the audio file */
    in_file = afOpenFile( filename, "rb", NULL );
    if ( !in_file )
	return -1;

    /* get audio file parameters */
    frame_count = afGetFrameCount( in_file, AF_DEFAULT_TRACK );
    in_channels = afGetChannels( in_file, AF_DEFAULT_TRACK );
    in_rate = afGetRate( in_file, AF_DEFAULT_TRACK );
    length = afGetTrackBytes( in_file, AF_DEFAULT_TRACK );
    afGetSampleFormat( in_file, AF_DEFAULT_TRACK, &in_format, &in_width );

    /*  printf ("frames: %i channels: %i rate: %f format: %i width: %i\n",
     *	        frame_count, in_channels, in_rate, in_format, in_width);
     */

    /* convert audiofile parameters to EsounD parameters */
    if ( in_width == 8 )
	out_bits = ESD_BITS8;
    else if ( in_width == 16 )
	out_bits = ESD_BITS16;
    else
    {
	/* fputs ("only sample widths of 8 and 16 supported\n", stderr); */
	return -1;
    }

    bytes_per_frame = ( in_width  * in_channels ) / 8;

    if ( in_channels == 1 )
	out_channels = ESD_MONO;
    else if ( in_channels == 2 )
	out_channels = ESD_STEREO;
    else
    {
	/* fputs ("only 1 or 2 channel samples supported\n", stderr); */
	return -1;
    }

    out_format = out_bits | out_channels | out_mode | out_func;
    out_rate = (int) in_rate;

    /* construct name */
    if ( name_prefix ) {
	strncpy( name, name_prefix, ESD_NAME_MAX - 2 );
	strcat( name, ":" );
    }
    strncpy( name + strlen( name ), filename, ESD_NAME_MAX - strlen( name ) );

    /* connect to server and play stream */
    sample_id = esd_sample_cache( esd, out_format, out_rate, 
				  length, name );

    /* play */
    esd_send_file( esd, in_file, bytes_per_frame );

    /* close up and go home */
    if ( afCloseFile ( in_file ) )
	return -1;

    confirm_id = esd_confirm_sample_cache( esd );
    if ( confirm_id != sample_id )
	return -1;

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound cached file\n" );
    */

    return sample_id;
}

