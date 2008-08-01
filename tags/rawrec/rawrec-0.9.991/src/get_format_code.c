/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* argument: string representation of DSP sample format to use.

   return: corresponding integer format code for driver.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/soundcard.h>

#include "rawrec.h"

int get_format_code(const char *format)
{
  if ( strcmp(format, "s16_le") == 0 )
    return AFMT_S16_LE;
  if ( strcmp(format, "u16_le") == 0 )
    return AFMT_U16_LE;
  if ( strcmp(format, "s16_be") == 0 )
    return AFMT_S16_BE;
  if ( strcmp(format, "u16_be") == 0 )
    return AFMT_U16_BE;
  if ( strcmp(format, "s8") == 0 )
    return AFMT_S8;
  if ( strcmp(format, "u8") == 0 ) 
    return AFMT_U8;
  
  /* If we make it this far, its a bug.  */
  err_die("BUG: internal function '%s' got invalid string argument\n", 
	  __func__);
}
