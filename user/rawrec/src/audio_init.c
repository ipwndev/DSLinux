/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* arguments: audio device name, mode to open with (how = FOR_READING
   or FOR_WRITING, sample format signedness and endianness, sampling
   rate, bits per sample, and number of channels
  
   return: audio fd on success, exit(EXIT_FAILURE) on bug or resource error 

   The audio_init function primes the audio device for later use by
   the threads that do the actual work.  audio_init is performed a
   single time on an as-soon-as-possible bases at high priority, and
   returns before any threads are spawned.  By performing the
   initialization here, the threads are kept simple and relatively
   condition-free. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "rawrec.h"

int audio_init(const char *audio_dev, open_how how, int speed, 
	       const char *format, int channels)
{
  int ioctl_param;		/* argument for audio ioctl calls */
  /* sample format as a driver integer code */
  int driver_format = -1;	/* bogus initializer reassures compiler */
  int audio_fd;			/* fd of dsp device */

  /* Open the device for reading or writing. */
  if ( how == FOR_READING ) {
    if ( (audio_fd = open(audio_dev, O_RDONLY)) < 0 ) {
      err_die("could not open %s for reading: %s\n", audio_dev, 
	      strerror(errno));
    }
  } else {			/* open for writing */
    if ( (audio_fd = open(audio_dev, O_WRONLY)) < 0 ) {
      err_die("could not open %s for writing: %s\n", audio_dev, 
	      strerror(errno));
    }
  }

  /* Set dsp parameters.  If any of these error conditions are true,
     it means we have had a bug, as the validity of the options should
     have been checked elsewhere (in this case in process_command_line
     and test_pcm_params).  Also, if the exact values requested are
     not available to the sound driver, it is considered a bug,
     e.g. if you request a sampling rate of speed = 20457 and the
     driver returns 20500 into speed, it is a bug.  Good programming
     requires that you test your parameters explicity to see if they
     are supported.  The test_pcm_params function provides a simple
     way of doing this. */

  if ( ioctl(audio_fd, SNDCTL_DSP_SYNC, NULL) == -1 ) {
    err_die("BUG: SNDCTL_DSP_SYNC ioctl on %s failed: %s", audio_dev, 
	    strerror(errno));
  }

  /* Convert format code to integer code acceptable to the audio driver. */
  driver_format = get_format_code(format);

  ioctl_param = driver_format;
  if ( ioctl(audio_fd, SNDCTL_DSP_SETFMT, &ioctl_param) == -1 ) {
    err_die("SNDCTL_DSP_SETFMT ioctl on %s failed: %s\n", audio_dev, 
	    strerror(errno));
  }
  if ( ioctl_param != driver_format ) {
    err_die("BUG: SNDCTL_DSP_SETFMT could not set format to specified "
	    "value\n");
  }

  ioctl_param = channels;
  if ( ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &ioctl_param) == -1 ) {
    err_die("BUG: SNDCTL_DSP_CHANNELS ioctl on %s failed: %s\n", audio_dev, 
	    strerror(errno));
  }
  if ( ioctl_param != channels ) {
    err_die("BUG: SNDCTL_DSP_CHANNELS could not set CHANNELS to specified "
	    "value\n");
  }
  ioctl_param = speed;
  if ( ioctl(audio_fd, SNDCTL_DSP_SPEED, &ioctl_param) == -1 ) {
    err_die("BUG: SNDCTL_DSP_SPEED ioctl on %s failed: %s\n", audio_dev, 
	    strerror(errno));
  }
  if ( ioctl_param != speed) {
    err_die("BUG: SNDCTL_DSP_SPEED ioctl could not set SPEED to specified "
	    "value\n");
  }
  
  return audio_fd;
}
