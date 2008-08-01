/* Copyright (C) 2006  Britton Leo Kerin, see copyright.  */

/* Print an error on stderr using printf style formatting, then exit.  */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "rawrec.h"

void err_die(const char *error_message, ...)
{
  va_list ap;

  fprintf(stderr, "%s: ", progname);
  va_start(ap, error_message);
  vfprintf(stderr, error_message, ap);
  va_end(ap);
  
  exit(EXIT_FAILURE);
}
