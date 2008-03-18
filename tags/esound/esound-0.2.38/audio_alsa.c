/* Advanced Linux Sounds Architecture support for EsounD
   7-19-98: Nick Lopez( kimo_sabe@usa.net ) - it starts!
*/
#include "esd.h"

/* debugging messages for audio device */
static int driver_trace = 0;

#if defined(DRIVER_NEWALSA)
#  include <sys/asoundlib.h>
#else
#  include <sys/soundlib.h>
#endif

#if (SND_LIB_MINOR > 4)
#  define ALSA_5_API
#endif

static snd_pcm_t *alsa_sound_handle;

#ifdef ALSA_5_API
static snd_pcm_format_t alsa_format;
static snd_pcm_channel_info_t alsa_pinfo;
static int alsa_direction = SND_PCM_OPEN_PLAYBACK;
static int alsa_mode = SND_PCM_MODE_BLOCK;
static int alsa_channel = SND_PCM_CHANNEL_PLAYBACK;
#endif

/* so that EsounD can use other cards besides the first */
#ifndef ALSACARD
#  define ALSACARD 0
#endif
#ifndef ALSADEVICE
#  define ALSADEVICE 0
#endif

/* some identifiers changed names */
#ifndef SND_PCM_OPEN_RECORD
#define SND_PCM_OPEN_RECORD SND_PCM_OPEN_CAPTURE
#endif

#ifndef snd_pcm_record_format
#define snd_pcm_record_format snd_pcm_capture_format
#endif

#define ARCH_esd_audio_open
static void
alsa_print_error (int code, int card, int device) {
    if( driver_trace ) { 
	perror( "snd_ctl_open" );

	if( device >= 0 ) {
	    fprintf (stderr, "card %d pcm device %d open failed: %s\n",
	    	     card, device, snd_strerror( code ) );
	} else {
	    fprintf( stderr, "card %d open failed: %s\n", 
	  	     card, snd_strerror( code ) );
	}
    }    
}

int esd_audio_open()
{
    int mask, card=ALSACARD, device=ALSADEVICE;
    int nbr_cards, ret;
    char buf[256];
    struct snd_ctl_hw_info hw_info;
    static int frag_size = 4*1024;

#ifdef ALSA_5_API
    static int frag_count = 0;
#else
    static int frag_count = 2;
#endif

#ifdef ALSA_5_API
    snd_pcm_channel_params_t params;
    snd_pcm_channel_setup_t setup;
#else
    int alsa_direction = SND_PCM_OPEN_PLAYBACK;
    snd_pcm_format_t alsa_format;
    snd_pcm_playback_params_t params;
#endif

    snd_ctl_t *ctl_handle;

    if( driver_trace ) {
        fprintf( stderr, "Using ALSA %s\n", SND_LIB_VERSION_STR );
    }

    /* if recording, set for full duplex mode */
    if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD ) {
	alsa_direction = SND_PCM_OPEN_DUPLEX;
#ifdef ALSA_5_API
	/* alsa_channel = SND_PCM_CHANNEL_CAPTURE; */
#endif
    }
  
#if 0 /* single card code, just in case anyone needs it */
    if ( ret = snd_pcm_open( &alsa_sound_handle, ALSACARD, ALSADEVICE, alsa_direction ) < 0) {
      perror( "snd_pcm_open" );
          fprintf( stderr, "open failed: %s\n", snd_strerror( ret ) );
          esd_audio_close();
          esd_audio_fd = -1;
          return ( -1 );
    }
    
#else /* multiple card code, open the first available.  someone check it? */

    mask = snd_cards_mask();
    if ( !mask ) {
          fprintf( stderr, "audio_alsa: no cards found!" );
          esd_audio_close();
          esd_audio_fd = -1;
          return ( -1 );
    }

    alsa_sound_handle = NULL;

#ifdef ALSA_5_API
    nbr_cards = snd_cards();
#else
    nbr_cards = SND_CARDS;
#endif

    if( driver_trace ) {
       fprintf( stderr, "esd: Found %d card(s)\n", nbr_cards );
    }

    for ( card=0; ( card < nbr_cards ) && (alsa_sound_handle == NULL); card++ ) {
            if( driver_trace ) {
                fprintf( stderr, "esd: trying alsa card %d\n", card );
            }

            /* open sound card */
            ret = snd_ctl_open( &ctl_handle, card );
            if ( ret < 0 ) {
                alsa_print_error( ret, card, -1 );
                continue;
            }

	    if (driver_trace ) {
		fprintf( stderr, "opened alsa card %d\n", card );
	    }
  
            /* get info on sound card */
            ret = snd_ctl_hw_info( ctl_handle, &hw_info );
            if ( ret < 0 ) {
                alsa_print_error( ret, card, -1 );
                continue;
            }
            ret = snd_ctl_close( ctl_handle );
            if ( ret < 0 ) {
                alsa_print_error( ret, card, -1 );
                continue;
            }

            /* search for available pcm device on card */
            for ( device=0; 
		  (device < hw_info.pcmdevs) && (alsa_sound_handle == NULL);
		   device++ ) {
                ret = snd_pcm_open( &alsa_sound_handle,
				     card, device, alsa_direction );
                if ( ret < 0 ) {
                    alsa_print_error( ret, card, device );
                    alsa_sound_handle = NULL;
                    continue;
                }
            }
            device--;
	    
	    if ( (alsa_sound_handle != NULL) && driver_trace ) {
	       fprintf( stderr, "opened alsa card %d pcm device %d\n",
		      card, device );
	    }
     
    }
    card--;
  
    if ( alsa_sound_handle == NULL ) {
	fprintf( stderr, "Couldn't open any alsa card! Last card tried was %d\n"
		 ,card );
	fprintf( stderr, "Error opening card %d: %s\n", 
		 card, snd_strerror( ret ) );  
	
	esd_audio_close();
	esd_audio_fd = -1;
	return ( -1 );
    }
    
#endif  /* Multiple cards */

#ifdef ALSA_5_API
    memset(&alsa_pinfo, 0, sizeof(alsa_pinfo));
    ret = snd_pcm_channel_info( alsa_sound_handle, &alsa_pinfo );
    if ( ret ) {
        fprintf( stderr, "error: %s: in snd_pcm_channel_info\n", snd_strerror(ret) );
        return( -1 );
    }

    memset(&params, 0, sizeof(params));
    params.buf.block.frag_size = frag_size;
    params.buf.block.frags_max = frag_count;
    params.buf.block.frags_min = 1;
    params.channel = alsa_channel;
    params.mode = alsa_mode;
    params.start_mode = SND_PCM_START_FULL;
    params.stop_mode = SND_PCM_STOP_STOP;

    memset(&alsa_format, 0, sizeof(alsa_format));
#endif

    /* set the sound driver audio format for playback */
    alsa_format.format = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16 )  
      ? /* 16 bit */ SND_PCM_SFMT_S16 : /* 8 bit */ SND_PCM_SFMT_U8;
    alsa_format.rate = esd_audio_rate;

#ifdef ALSA_5_API
    alsa_format.voices = ( ( esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO )
	                 ? 2 : 1;

    /* Use supported interleave */
    if(alsa_pinfo.flags & SND_PCM_CHNINFO_INTERLEAVE ) {
       alsa_format.interleave = 1;
       if( driver_trace) fprintf( stderr, "using interleave mode\n");
    }
     if(alsa_pinfo.flags & SND_PCM_CHNINFO_NONINTERLEAVE ) {
       alsa_format.interleave = 0;
       if( driver_trace) fprintf( stderr, "esd: using noninterleave mode\n");
    }

    memcpy(&params.format, &alsa_format, sizeof(alsa_format));

    snd_pcm_channel_flush( alsa_sound_handle, alsa_channel );

    ret = snd_pcm_channel_params( alsa_sound_handle, &params );
    if ( ret ) {
              fprintf( stderr, "error: %s: in snd_pcm_channel_params\n", snd_strerror(ret) );
          return( -1 );
    }

    ret = snd_pcm_channel_prepare( alsa_sound_handle, alsa_channel );
    if ( ret ) {
                      fprintf( stderr, "error: %s: in snd_pcm_channel_prepare\n", snd_strerror(ret) );
        return(-1);
    }


    memset(&setup, 0, sizeof(setup));
    setup.mode = alsa_mode;
    setup.channel = alsa_channel;
    ret = snd_pcm_channel_setup( alsa_sound_handle, &setup );
    if( ret ) {
        fprintf( stderr, "error: %s: in snd_pcm_channel_setup\n", snd_strerror(ret) );
        return(-1);
    }

    if ( params.format.rate != esd_audio_rate || params.format.voices != 2
                       || params.format.format != SND_PCM_SFMT_S16 ) {
                      fprintf( stderr, "set format didn't work.");
                      return(-1);
        }

#else
    alsa_format.channels = ( ( esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO )
              ? 2 : 1;

     if( alsa_direction == SND_PCM_OPEN_DUPLEX || alsa_direction == SND_PCM_OPEN_PLAYBACK ) {
        if ( ( ret = snd_pcm_playback_format( alsa_sound_handle, &alsa_format ) ) < 0 ) {
            fprintf( stderr, "set playback format failed: %s\n", snd_strerror( ret ) );
            esd_audio_close();
            esd_audio_fd = -1;
            return ( -1 );
              }
    }

    if( alsa_direction == SND_PCM_OPEN_DUPLEX || 
        alsa_direction == SND_PCM_OPEN_RECORD ) {
        if ( ( ret = snd_pcm_record_format( alsa_sound_handle, 
               &alsa_format ) ) < 0 ) {
            fprintf( stderr, "set record format failed: %s\n", 
	             snd_strerror( ret ) );
            esd_audio_close();
            esd_audio_fd = -1;
            return ( -1 );
        }
    }    

    params.fragment_size = frag_size;
    params.fragments_max = frag_count;
    params.fragments_room = 1;
    ret = snd_pcm_playback_params( alsa_sound_handle, &params );
    if ( ret ) {
	printf( "error: %s: in snd_pcm_playback_params\n", snd_strerror(ret) );
    }
    if ( alsa_format.rate != esd_audio_rate || alsa_format.channels != 2
       || alsa_format.format != SND_PCM_SFMT_S16 )
        fprintf( stderr, "set format didn't work.");

#endif /* ALSA_5_API */


/* shouldn't use non-blocking mode, because you have to sit in a loop rewriting
   data until success (eating cpu time in the process).  This wasn't being done,
   and didn't work on my machine.  Or you could use select(man page 2), I guess.
*/
#if 0
    ret = snd_pcm_block_mode( alsa_sound_handle, 1 );
    if ( ret )
	printf( "error: %s: in snd_pcm_block_mode\n", snd_strerror(ret));
#endif

    /* no descriptor for ALSAlib */
#ifdef ALSA_5_API
    return ( esd_audio_fd = snd_pcm_file_descriptor( alsa_sound_handle,
						     alsa_channel ) );
#else
    return ( esd_audio_fd = snd_pcm_file_descriptor(alsa_sound_handle) );
#endif

}

#define ARCH_esd_audio_close
void esd_audio_close()
{
    snd_pcm_close( alsa_sound_handle );
}

#define ARCH_esd_audio_pause
void esd_audio_pause()
{
    /* apparently this gets rid of pending data, which isn't the effect
       we're going for, namely, play the data in the buffers and stop */
    /* snd_pcm_drain_playback( handle ); */
}

#define ARCH_esd_audio_read
int esd_audio_read( void *buffer, int buf_size )
{
    return (snd_pcm_read( alsa_sound_handle, buffer, buf_size ));
}

#define ARCH_esd_audio_write
int esd_audio_write( void *buffer, int buf_size )
{
    int i=0;

#ifdef ALSA_5_API
    snd_pcm_channel_status_t status;
    int ret;
#endif

    i = snd_pcm_write( alsa_sound_handle, buffer, buf_size);
    if( i<0 ) {
#if 0
        fprintf( stderr, "error: %s: in snd_pcm_write\n", snd_strerror(i) );
#endif
    }

#ifdef ALSA_5_API
    status.channel = SND_PCM_CHANNEL_PLAYBACK;
    ret = snd_pcm_channel_status( alsa_sound_handle, &status );
    if( ret ) {
                  if( driver_trace ) fprintf( stderr, "error: %s: in snd_pcm_channel_status\n", snd_strerror(ret) );
              return(-1);
    }
    if( status.underrun ) {
        snd_pcm_channel_flush( alsa_sound_handle, alsa_channel );
        snd_pcm_playback_prepare( alsa_sound_handle );
        snd_pcm_write( alsa_sound_handle, buffer, buf_size );
        if (snd_pcm_channel_status( alsa_sound_handle, &status ) < 0 && driver_trace) {
            fprintf(stderr, "ALSA: could not get channel status. giving up\n");
            return -1;
              }
              if (status.underrun) {
                  if( driver_trace ) fprintf(stderr, "ALSA: write error. giving up\n");
                        return -1;
              }
    }
#endif /* ALSA_5_API */

    return (i);
}

#define ARCH_esd_audio_flush
void esd_audio_flush()
{
    fsync( esd_audio_fd );
    /*snd_pcm_flush_playback( handle );*/
}
