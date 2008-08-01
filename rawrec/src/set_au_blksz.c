/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* Sets the block size to be used by the kernel sound driver via the
   SNDCTL_DSP_SETFRAGMENT mechanism. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "rawrec.h" 

void set_au_blksz(int audio_fd, int fragsz_arg)
{
  int arg = 0x7fff0000;	       /* we don't care how many fragments */
      
  /* Note that fragsz is assumed to be a power of two. */
  for ( ; fragsz_arg > 1 ; fragsz_arg /= 2 ) {
    arg += 1;
    if ( fragsz_arg % 2 != 0 ) {
      err_die("BUG: fragsz_arg to SNDCTL_DSP_SETFRAGMENT not a power of "
	      "two\n");
    }
  }
  if ( ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &arg) == -1 )
    err_die("BUG: SNDCTL_DSP_SETFRAGMENT ioctl failed: %s\n", strerror(errno));
}
