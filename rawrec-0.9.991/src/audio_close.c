/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function closes the audio device. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rawrec.h"

void audio_close(int audio_fd, const char *audio_dev)
{ 
  if ( close(audio_fd) == -1 )
    err_die("could not close %s: %s\n", audio_dev, strerror(errno));
}
