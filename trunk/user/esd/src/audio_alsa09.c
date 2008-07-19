/* -*- Mode: c; tab-width: 4; c-basic-offset: 4; -*-
 *
 *  ALSA 0.9 support for Esound
 *  by Santiago Otero (siryurian@terra.es)
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *                                                                             
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *                                                                             
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */


/* Debug flag */
int alsadbg = 0;
/* Error flag */
int alsaerr = 0;
#include <alsa/asoundlib.h>

/* FULL DUPLEX => two handlers */

static snd_pcm_t *alsa_playback_handle = NULL;
static snd_pcm_t *alsa_capture_handle = NULL;

static snd_output_t *output = NULL;

#define ACS  SND_PCM_ACCESS_RW_INTERLEAVED
#define BUFFERSIZE 16384



void print_state(void)
{
  
	int state;
	int err;
  
	snd_pcm_status_t *status;
  
	snd_pcm_status_alloca(&status);
  
	if ( (err = snd_pcm_status(alsa_playback_handle, status)) < 0) {
		fprintf(stderr, "Error getting status:%s\n", snd_strerror(err));
		return;
	}
  
  
	state = snd_pcm_status_get_state( status );
	switch(state) {
	case SND_PCM_STATE_OPEN :
		fprintf(stderr, "SND_PCM_STATE_OPEN\n");
		break;
	case SND_PCM_STATE_SETUP:
		fprintf(stderr, "SND_PCM_STATE_SETUP\n");
		break;
	case SND_PCM_STATE_PREPARED:
		fprintf(stderr,"SND_PCM_STATE_PREPARED\n");
		break;
	case SND_PCM_STATE_RUNNING:
		fprintf(stderr, "SND_PCM_STATE_RUNNING\n");
		break;
	case SND_PCM_STATE_XRUN:
		fprintf(stderr, "SND_PCM_STATE_XRUN\n");
		break;
	case SND_PCM_STATE_DRAINING:
		fprintf(stderr, "SND_PCM_STATE_DRAINING\n");
		break;
	case SND_PCM_STATE_PAUSED: 
		fprintf(stderr, "SND_PCM_STATE_PAUSED\n");
		break;
	case SND_PCM_STATE_SUSPENDED:
		fprintf(stderr, "SND_PCM_STATE_SUSPENDED\n");
		break;
	default:
		fprintf(stderr, "WARNING: unknown state %d\n", state);
	}
  
}

snd_pcm_t* initAlsa(char *dev, int format, int channels, int speed, int mode) 
{
  
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	int err;
	int periods;
  
#ifdef DRIVER_ALSA_09_NEW_PCM_API
	int t_dir=0;
	int t_speed=speed;
	snd_pcm_uframes_t t_bufsize=BUFFERSIZE;
#endif

	err = snd_pcm_open(&handle, dev, mode, SND_PCM_NONBLOCK);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -2;
		return NULL;
	}
	snd_pcm_nonblock(handle, 0);
	snd_pcm_hw_params_alloca(&hwparams);
  
	err = snd_pcm_hw_params_any(handle, hwparams);  
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
  
	err = snd_pcm_hw_params_set_access(handle, hwparams, ACS);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
  
	err = snd_pcm_hw_params_set_format(handle, hwparams, format);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
  
	err = snd_pcm_hw_params_set_channels(handle, hwparams,  channels);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
#ifndef DRIVER_ALSA_09_NEW_PCM_API
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, speed, 0);
#else
    err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &t_speed, &t_dir);
#endif
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
#ifndef DRIVER_ALSA_09_NEW_PCM_API
	if (err != speed) {
#else
	if (t_speed != speed) {
#endif
		if (alsadbg)
			fprintf(stderr, "Rate not avaliable %i != %i\n", speed, err);
		alsaerr = -1;
		return handle;
	}
  
	err = snd_pcm_hw_params_set_periods_integer(handle, hwparams);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
  
	periods = 2;
	err = snd_pcm_hw_params_set_periods_min(handle, hwparams, &periods, NULL);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
  
	periods = 64;
	err = snd_pcm_hw_params_set_periods_max(handle, hwparams, &periods, NULL);
	if (err < 0) {
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
#ifndef DRIVER_ALSA_09_NEW_PCM_API
	err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, BUFFERSIZE); 
#else
	err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &t_bufsize);
#endif
	if (err < 0) { 
		if (alsadbg)
			fprintf(stderr, "Buffersize:%s\n", snd_strerror(err)); 
		alsaerr = -1;
		return handle; 
	} 
	err = snd_pcm_hw_params(handle, hwparams); 
	if (err < 0) { 
		if (alsadbg)
			fprintf(stderr, "%s\n", snd_strerror(err));
		alsaerr = -1;
		return handle;
	}
	if (alsadbg)
		snd_pcm_dump(handle, output);
	alsaerr = 0;
	return handle;
}

#define ARCH_esd_audio_devices

const char *esd_audio_devices(void)
{
        int card, err;
        static char *all_alsa_cards = NULL;
        char *alsa_card_tmp;
        snd_ctl_t *handle;
        snd_ctl_card_info_t *info;

        snd_ctl_card_info_alloca(&info);

        if(all_alsa_cards)
        {
                free(all_alsa_cards);
                all_alsa_cards = NULL;
        }

	card = -1;
        if(snd_card_next(&card) < 0 || card < 0)
        {
                /* No cards found */
        }
        else
        {
                while( card >= 0 )
                {
                        char name[32];
                        sprintf( name, "hw:%d", card );
                        err = snd_ctl_open(&handle, name, 0);
                        if( err < 0 )
                        {
                                fprintf( stderr, "audio_alsa: Error: control open (%i): %s\n", card, snd_strerror(err));
                                continue;
                        }
                        err = snd_ctl_card_info(handle, info);
                        if( err < 0 )
                        {
                                fprintf( stderr, "audio_alsa: Error: control hardware info (%i): %s\n", card, snd_strerror(err));
                                snd_ctl_close(handle);
                                continue;
                        }
                        alsa_card_tmp = malloc(strlen(snd_ctl_card_info_get_name(info))+20);
                        sprintf( alsa_card_tmp, "hw:%d  (%s)\n", card, snd_ctl_card_info_get_name(info) );
                        if(all_alsa_cards)
                        {
                                all_alsa_cards = realloc(all_alsa_cards, strlen(all_alsa_cards)+strlen(alsa_card_tmp)+30);
				strcat(all_alsa_cards, "                       ");
                                strcat(all_alsa_cards, alsa_card_tmp);
                                free(alsa_card_tmp);
                        }
                        else
                        {
                                all_alsa_cards = alsa_card_tmp;
                        }
                        snd_ctl_close(handle);
                        if(snd_card_next(&card) < 0)
                        {
                                break;
                        }
                }
        }
        if(all_alsa_cards)
        {
                return(all_alsa_cards);
        }
        else
        {
                return("No available cards found");
        }
}


#define ARCH_esd_audio_open

int esd_audio_open(void)
{
  
	int channels;
	int format;
	char *dev;
  
	if (alsadbg)
		fprintf(stderr, "esd_audio_open\n");

  
	if ((esd_audio_format & ESD_MASK_BITS) == ESD_BITS16) 
		format = SND_PCM_FORMAT_S16;
	else format = SND_PCM_FORMAT_U8;
  
	if ((esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO) 
		channels = 2;
	else channels = 1;
  
  
	snd_output_stdio_attach(&output, stderr, 0);
  
	if(esd_audio_device)
        {
		dev = (char*) malloc(strlen(esd_audio_device)+1);
		strcpy(dev, esd_audio_device);
        }
        else
        {
		/* bind to alsa default setting */
		dev = strdup ("default");
        }


	if (alsadbg)
		fprintf(stderr, "dev=%s\n",dev);

	alsa_playback_handle = initAlsa(dev, format, channels, 
									esd_audio_rate, SND_PCM_STREAM_PLAYBACK);
	if(alsaerr) 
	{
		if(alsaerr == -1) snd_pcm_close(alsa_playback_handle);
		if (alsadbg)
			fprintf(stderr, "Error opening device for playback\n");

		esd_audio_fd = -1;
		free(dev);
		return alsaerr;
	}
	if (alsadbg)
		fprintf(stderr, "Device open for playback\n");

	if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD ) {
		alsa_capture_handle = initAlsa(dev, format, channels, 
									   esd_audio_rate, SND_PCM_STREAM_CAPTURE);
		if (alsaerr) {
			if (alsaerr==-1) snd_pcm_close(alsa_capture_handle);
			if (alsadbg)
				fprintf(stderr, "Error opening device for capture\n");

			snd_pcm_close(alsa_playback_handle);
			esd_audio_fd = -1;
			free(dev);
			return alsaerr;
		}

		if (alsadbg)
			fprintf(stderr, "Device open for capture\n");

	}
	esd_audio_fd = 0;
	free(dev);
	if (alsadbg)
		print_state();

	return 0;
}

#define ARCH_esd_audio_close
void esd_audio_close(void)
{
	if (alsadbg) {
		fprintf(stderr, "esd_audio_close\n");
		print_state();
	}

	if (alsa_playback_handle != NULL)
		snd_pcm_close( alsa_playback_handle );
	if (alsa_capture_handle != NULL)
		snd_pcm_close(alsa_capture_handle);
	alsa_playback_handle = NULL;
	alsa_capture_handle = NULL;
}

#define ARCH_esd_audio_pause
void esd_audio_pause(void)
{
  
	return;
}

#define ARCH_esd_audio_read
int esd_audio_read( void *buffer, int buf_size )
{
	int err;
	
	int len = snd_pcm_bytes_to_frames(alsa_capture_handle, buf_size);
	while ( ( err = snd_pcm_readi( alsa_capture_handle, buffer, len)) < 0) {
		if (alsadbg) {
			fprintf(stderr, "esd_audio_read\n");
			print_state();
		}

		if (err == -EPIPE) {
			if (alsadbg)
				fprintf(stderr, "EPIPE\n");

			if (( err = snd_pcm_prepare( alsa_capture_handle ) )< 0) {
				if (alsadbg)
					fprintf(stderr, "%s\n", snd_strerror(err));

				return -1;
			}
			continue;
		} else if ( err == -ESTRPIPE) {
			if (alsadbg)
				fprintf(stderr, "ESTRPIPE\n");

			while (( err = snd_pcm_resume(alsa_capture_handle)) == -EAGAIN)
				sleep(1);
			if (err < 0) {
				if (alsadbg)
					fprintf(stderr, "Preparing...\n");
				if (snd_pcm_prepare( alsa_capture_handle) < 0)
					return -1;
			}
			continue;
		} 
    
		err = snd_pcm_prepare(alsa_capture_handle) ;
		if (err < 0) {
			if (alsadbg) 
				fprintf(stderr, "%s\n", snd_strerror(err));
			return -1;
		}
	}
  
	return ( snd_pcm_frames_to_bytes(alsa_capture_handle, err) );
}


#define ARCH_esd_audio_write
int esd_audio_write( void *buffer, int buf_size )
{
	int err;
	int len = snd_pcm_bytes_to_frames(alsa_playback_handle, buf_size);

	while (len > 0) {
		while ( ( err = snd_pcm_writei( alsa_playback_handle, buffer, len)) < 0) {
			if (alsadbg) {
				fprintf(stderr, "esd_audio_write\n");
				print_state();
			}

			if (err == -EPIPE) {
				if (alsadbg)
					fprintf(stderr, "EPIPE\n");
			} else  if (err == -ESTRPIPE) {
				if (alsadbg)
					fprintf(stderr, "ESTRPIPE\n");
				while (( err = snd_pcm_resume(alsa_playback_handle)) == -EAGAIN)
					sleep(1);
			} 
			err = snd_pcm_prepare(alsa_playback_handle) ;
			if (err < 0) {
				if (alsadbg)
					fprintf(stderr, "%s\n", snd_strerror(err));
				return -1;
			}
			continue;
		}
		len -= err;
		buffer += snd_pcm_frames_to_bytes(alsa_playback_handle, err);
	}
  
	return buf_size;
}

#define ARCH_esd_audio_flush
void esd_audio_flush(void)
{
  
	if (alsadbg) {
		fprintf(stderr, "esd_audio_flush\n");
		print_state();
	}

	if (alsa_playback_handle != NULL)
		snd_pcm_drain( alsa_playback_handle );
  
	if (alsadbg) 
		print_state();
  
  
}
