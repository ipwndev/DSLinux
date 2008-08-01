/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function opens and initializes the ordinary raw data file
   which the audio stream is coming from or going to.  The argument
   'how' should be a flag, either FOR_READING or FOR_WRITING that
   describe the desired mode.  The file descriptor for the data file
   is returned. */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rawrec.h"

int data_init(const char *data_file, open_how how)
{
  int fd = -1;			/* Local copy of the descriptor. */
  /* Default ugo=rw permissions for data file, subject to umask. */
  mode_t fd_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  if ( how == FOR_WRITING ) {
    if ( (fd = open(data_file, O_WRONLY | O_CREAT | O_TRUNC, 
		    fd_mode)) == -1 ) {
      err_die("could not create %s for writing: %s\n", data_file, 
	      strerror(errno));
    }
  } else {			/* Readonly. */
    if ( (fd = open(data_file, O_RDONLY, fd_mode)) == -1 ) {
      err_die("could not open %s for reading: %s\n", data_file, 
	      strerror(errno));
    }
  }

  return fd;
}
