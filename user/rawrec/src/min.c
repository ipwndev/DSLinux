/* Copyright (C) 2006  Britton Leo Kerin, see copyright.  */

/* Returns the smaller of it's two arguments.  */

#include "rawrec.h"

double min(double x1, double x2)
{
  if ( x1 < x2 )
    return x1;
  else 
    return x2;
}
