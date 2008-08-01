/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function causes the calling process to sleep for time seconds
   or samples samples (given sampling rate speed Hz).  */

#include <math.h>
#include <time.h>

#include "rawrec.h"

void sleep_on_option(double time, double samples, int speed)
{
  if ( time > 0 || samples > 0 ) {
    struct timespec reqst, remst;

    if ( time > samples / speed ) {
      reqst.tv_sec = (time_t) floor(time);
      reqst.tv_nsec = (long) nearbyint((time - floor(time)) * 1000000);      
    } else {
      reqst.tv_sec = (time_t) floor(samples / speed);
      reqst.tv_nsec = (long) rint((samples / speed - floor(samples / speed))
                                  * 1000000);
    }    
    /* Do the actual sleeping. */     
    nanosleep(&reqst, &remst);

    /* Unimplemented, ultimately to be sleep continuation after interrupt.  */
    /*
    if ( nanosleep(&reqst, &remst) == -1 )
      ;
    */
  }

  return;
}
