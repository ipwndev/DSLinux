/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function returns true if it's argument is a positive power of
   two, false otherwise.  Note that this is a slow but precise
   implementation, there is no possibility of rounding error causing
   an erroneous result.  */

#include "rawrec.h"

boolean is_pow_two(int x)
{
  int product;			/* running product */

  for ( product = 1 ; product <= x ; product *= 2 )
    if ( product == x )
      return TRUE;

  /* if product got bigger than x without ever equaling x, we end up here */
  return FALSE;
}
