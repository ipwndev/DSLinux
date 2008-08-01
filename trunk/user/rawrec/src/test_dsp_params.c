/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function tests the supplied pcm parameters and returns a code
   indicating any problems encountered in setting them.  If the values
   are outside the capabilities of the hardware, an error is returned
   indicating this.  If the parameters can be set to values close to
   those requested, but hardware or driver limitations prevent an
   exact match, another code is returned.  If everything can be set
   exactly as requested, a success code is returned indicating
   this. */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "rawrec.h"

test_dsp_params_rtn test_dsp_params(char *audio_dev, open_how how,
				    int *speedp, char *format,
				    int *channelsp)
{
  int audio_fd;			/* file descriptor of /dev/dsp */
  int driver_format = -1;	/* sample format as a driver integer code */
  int ioctl_param;		/* arguments for audio ioctl calls */
  test_dsp_params_rtn rtn_code = SUCCESS;   /* return code for function */

  /* Open the device for reading or writing as requested. */
  if ( how == FOR_READING ) {
    if ( (audio_fd = open(audio_dev, O_RDONLY)) < 0 ) {
      err_die("could not open %s for reading: %s\n", audio_dev, 
	      strerror(errno));
    }
  }
  else {
    if ( (audio_fd = open(audio_dev, O_WRONLY)) < 0 ) {
      err_die("could not open %s for writing: %s\n", audio_dev, 
	      strerror(errno));
    }
  }

  /* Attempt to set the audio parameters to the requested values,
     report failures and warnings as appropriate via the return code. */

  if ( ioctl(audio_fd, SNDCTL_DSP_SYNC, NULL) == -1 ) {
    err_die("SNDCTL_DSP_SYNC ioctl on %s failed: %s\n", audio_dev,
	    strerror(errno));
  }

  /* Convert format code to integer code acceptable to the audio driver. */
  driver_format = get_format_code(format);

  ioctl_param = driver_format;
  if ( ioctl(audio_fd, SNDCTL_DSP_SETFMT, &ioctl_param) == -1 ) {
    err_die("SNDCTL_DSP_SETFMT ioctl on %s failed: %s\n", audio_dev, 
	    strerror(errno));
  }
  if ( ioctl_param != driver_format )
    return INVALID_FORMAT;

  /* Currently, only mono or stereo (1 or 2 channel) use is supported. */
  if ( (*channelsp != 1 && *channelsp != 2) )
    return INVALID_CHANNELS;
  ioctl_param = *channelsp;
  if ( ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &ioctl_param) == -1 ) {
    err_die("SNDCTL_DSP_CHANNELS ioctl on %s failed: %s\n", audio_dev,
	    strerror(errno));
  }  
  if ( ioctl_param != *channelsp ) {
    *channelsp = ioctl_param;
    rtn_code = INEXACT_ARG;
  }

  ioctl_param = *speedp;
  if ( ioctl(audio_fd, SNDCTL_DSP_SPEED, &ioctl_param) == -1 ) {
    err_die("SNDCTL_DSP_SPEED ioctl on %s failed: %s\n", audio_dev, 
	    strerror(errno));
  }
  if ( ioctl_param != *speedp) {
    /* I've never heard of a sound card that couldn't get within 300 Hz. */
    if ( abs(*speedp - ioctl_param) > 300 )
      return INVALID_RATE;	
    else {
      /* Here we modify the speed argument to reflect the true value used.  */
      *speedp = ioctl_param;
      rtn_code = INEXACT_ARG;
    }
  }

  if ( close(audio_fd) == -1 )
    err_die("could not close %s: %s\n", audio_dev, strerror(errno));

  return rtn_code;
}
