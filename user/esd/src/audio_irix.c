/*
 * Audio code using SGI's dmedia library
 * - original code by Karl Anders Oygard
 * - recording support added by Kimball Thurston 7/22/98
 *
 * Currently the code is implemented using SGI's old library interface
 * ( 5.X and before ). This is still supported under at least 6.3 and 6.4
 * It is unknown how long they will continue to support this API...
 */

#include <assert.h>
#include <dmedia/audio.h>

ALport outaudioport = (ALport) 0;
ALport inaudioport = (ALport) 0;

long rate_params[] = { AL_OUTPUT_RATE, 0, AL_INPUT_RATE, 0 };
      
#define ARCH_esd_audio_open
int esd_audio_open()
{
    ALconfig audioconfig;
    audioconfig = ALnewconfig();
  
	rate_params[1] = esd_audio_rate;
	rate_params[3] = esd_audio_rate;

    if (!audioconfig) {
	printf( "Couldn't initialize new audio config\n" );
	esd_audio_fd = -1;
	return esd_audio_fd;
    } else {
	long pvbuf[] = { AL_OUTPUT_COUNT, 0, 
			 AL_MONITOR_CTL, 0, 
			 AL_OUTPUT_RATE, 0 };
    
	if (ALgetparams(AL_DEFAULT_DEVICE, pvbuf, 6) < 0)
	    if (oserror() == AL_BAD_DEVICE_ACCESS) {
		esd_audio_fd = -1;
		return esd_audio_fd;
	    }
    
	if (pvbuf[1] == 0 && pvbuf[3] == AL_MONITOR_OFF) {
	    ALsetparams(AL_DEFAULT_DEVICE, rate_params, 2);
	} else
	    if (pvbuf[5] != esd_audio_rate) {
		printf("audio device is already in use with wrong sample output rate\n");
		esd_audio_fd = -1;
		return esd_audio_fd;
	
	    }
    
	/* ALsetsampfmt(audioconfig, AL_SAMPFMT_TWOSCOMP); this is the default */
	/* ALsetwidth(audioconfig, AL_SAMPLE_16); this is the default */
    
	if ( (esd_audio_format & ESD_MASK_CHAN) == ESD_MONO)
	    ALsetchannels(audioconfig, AL_MONO);
	/* else ALsetchannels(audioconfig, AL_STEREO); this is the default */

	ALsetqueuesize(audioconfig, ESD_BUF_SIZE * 2);
    
	outaudioport = ALopenport("esd", "w", audioconfig);
	if (outaudioport == (ALport) 0) {
	    switch (oserror()) {
	    case AL_BAD_NO_PORTS:
		printf( "system is out of ports\n");
		esd_audio_fd = -1;
		return esd_audio_fd;
		break;
	
	    case AL_BAD_DEVICE_ACCESS:
		printf("couldn't access audio device\n");
		esd_audio_fd = -1;
		return esd_audio_fd;
		break;
	
	    case AL_BAD_OUT_OF_MEM:
		printf("out of memory\n");
		esd_audio_fd = -1;
		return esd_audio_fd;
		break;
	    }
	    /* don't know how we got here, but it must be bad */
	    esd_audio_fd = -1;
	    return esd_audio_fd;
	}
	ALsetfillpoint(outaudioport, ESD_BUF_SIZE);

	esd_audio_fd = ALgetfd(outaudioport);

	/*
	 * If we are recording, open a second port to read from
	 * and return that fd instead
	 */
	if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD ) {
	    inaudioport = ALopenport("esd", "r", audioconfig);
	    if (inaudioport == (ALport) 0) {
		switch (oserror()) {
		case AL_BAD_NO_PORTS:
		    printf( "system is out of ports\n");
		    esd_audio_fd = -1;
		    return esd_audio_fd;
		    break;
	
		case AL_BAD_DEVICE_ACCESS:
		    printf("couldn't access audio device\n");
		    esd_audio_fd = -1;
		    return esd_audio_fd;
		    break;
	
		case AL_BAD_OUT_OF_MEM:
		    printf("out of memory\n");
		    esd_audio_fd = -1;
		    return esd_audio_fd;
		    break;
		default:
		    printf( "Unknown error opening port\n" );
		}
				/* don't know how we got here, but it must be bad */
		esd_audio_fd = -1;
		return esd_audio_fd;
	    }
	    ALsetfillpoint(inaudioport, ESD_BUF_SIZE);
	    ALsetparams(AL_DEFAULT_DEVICE, (rate_params + 2), 2);

	    esd_audio_fd = ALgetfd(inaudioport);
	}

    }
    return esd_audio_fd;
}

#define ARCH_esd_audio_close
void esd_audio_close()
{
    /*
     * I guess this chunk of code is meant to make sure that
     * everything that was sent to the output got written
     * - let's leave that in for now, but it could cause a
     * delay when the user hits stop
     * -KDT
     */
    if (esd_audio_fd >= 0) {
	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(esd_audio_fd, &write_fds);	
    
	ALsetfillpoint(outaudioport, ESD_BUF_SIZE * 2);
	select(esd_audio_fd + 1, NULL, &write_fds, NULL, NULL);
    }
    
    if (outaudioport != (ALport) 0) {
      ALcloseport(outaudioport);
      outaudioport = (ALport) 0;
    }
    if (inaudioport != (ALport) 0) {
      ALcloseport(inaudioport);
      inaudioport = (ALport) 0;
    }
}

#define ARCH_esd_audio_write
int esd_audio_write(void *buffer, int buf_size)
{
	ALsetparams(AL_DEFAULT_DEVICE, rate_params, 2);
    if (ALwritesamps(outaudioport, buffer, buf_size / 2) == 0) {
	ALsetfillpoint(outaudioport, ESD_BUF_SIZE);
	return buf_size;
    }
    else
	return 0;    
}

#define ARCH_esd_audio_read
int esd_audio_read(void *buffer, int buf_size)
{
	ALsetparams(AL_DEFAULT_DEVICE, (rate_params + 2), 2);
    if (ALreadsamps(inaudioport, buffer, buf_size / 2) == 0) {
	ALsetfillpoint(inaudioport, ESD_BUF_SIZE);
	return buf_size;
    }
    else
	return 0;
}

#define ARCH_esd_audio_flush
void esd_audio_flush()
{
}
