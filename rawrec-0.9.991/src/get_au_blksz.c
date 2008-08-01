/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* Returns the block size used by the kernel sound driver.  The big
   buffer will be partitioned into chunks of this size. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "rawrec.h"

int get_au_blksz(int audio_fd)
{
  int blocksize;

  if ( ioctl(audio_fd, SNDCTL_DSP_GETBLKSIZE, &blocksize) == -1 )
    err_die("SNDCTL_DSP_GETBLKSIZE ioctl failed: %s\n", strerror(errno));

  return blocksize;
} 
