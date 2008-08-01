/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* Cleans up and closes the given data file as needed. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rawrec.h"

void data_close(int data_fd, const char *data_file) 
{
  if ( (close(data_fd)) == -1 )
    err_die("could not close data file %s: %s", data_file, strerror(errno)); 
}
