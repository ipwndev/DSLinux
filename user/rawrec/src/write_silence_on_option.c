/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function causes the calling process to write time_endrec or
   samp_endrec worth of silence (whichever is greater, given sampling
   speed) to file with file descriptor fd.  using_stdio flag and
   possibly argument file_name are passed to enable better error
   reporting.  bps is the number of bits per sample per channel. */

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rawrec.h"

void write_silence_on_option(int fd, boolean using_stdio, 
			     const char *file_name, double time_rec, 
			     long samp_rec, int speed, const char *format, 
			     int bps, int channels)
{
  double bytes_silence;		/* silence to write, in bytes */
  double bytes_done;		/* bytes of silence written so far */
  long bytes_to_write;		/* bytes to write with next write syscall */
  long bytes_written;		/* bytes written by last write syscall */
  /* bit pattern for a single eight bit channel's worth of silence */
  unsigned char eight_bit_silence[1];
  /* bit pattern for a single sixteen bit channel's worth of silence */
  unsigned char sixteen_bit_silence[2];
  /* a single (channels * bps) sized silence pattern */
  unsigned char *pattern_instance;
  int pattern_instance_sz;	/* size of a pattern instance */
  /* a buffer containing many repititions of the above pattern, for
     efficient use by read() and write() */
  unsigned char *pattern_buf;		
  /* the number of pattern repititions to put in the pattern_buf */
  const int PATTERNS_IN_PATTERN_BUF = 4096;
  int idx;			/* index for loops */

  if ( (strcmp(format, "s16_le") == 0) 
               || (strcmp(format, "u16_le") == 0)
               || (strcmp(format, "s16_be") == 0)
               || (strcmp(format, "u16_be") == 0)) 
    bps = 16;
  else if ( (strcmp(format, "s8") == 0) 
               || (strcmp(format, "u8") == 0) )
    bps = 8;
  else {
    err_die("BUG: unrecognized sample format seen in function '%s'\n", 
	    __func__);
  }
  pattern_instance_sz = channels * bps / 8;
  if ( (pattern_instance = (unsigned char *) malloc( 
               (size_t) pattern_instance_sz)) == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( (pattern_buf = (unsigned char *) malloc( 
               (size_t) (PATTERNS_IN_PATTERN_BUF * pattern_instance_sz)) ) 
               == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( time_rec > ( (double) samp_rec) / speed ) {
    bytes_silence = ceil(time_rec * (double) speed * (double) bps
			 * (double) channels / 8.0);
    /* Bytes of silence must be a multiple of full sample size.  */
    for ( ; bytes_silence / (channels * bps / 8) 
               < floor(bytes_silence / (channels * bps / 8)) 
	       ; bytes_silence++ )
      ;
  }
  else          
    bytes_silence = samp_rec * bps * (channels) / 8;

  /* Bit patterns for a single one channel sample of silence. */
  if ( (strcmp(format, "s16_le") == 0) || (strcmp(format, "s16_be") == 0) ) {
    sixteen_bit_silence[0] = 0x00;
    sixteen_bit_silence[1] = 0x00;
  }
  if ( strcmp(format, "u16_le") == 0 ) {
    sixteen_bit_silence[0] = 0x00;
    sixteen_bit_silence[1] = 0x80;
  }
  if ( strcmp(format, "u16_be") == 0 ) {
    sixteen_bit_silence[0] = 0x80;
    sixteen_bit_silence[1] = 0x00;
  }
  if ( strcmp(format, "s8") == 0 ) 
    eight_bit_silence[0] = 0x00;
  if ( strcmp(format, "u8") == 0 )
    eight_bit_silence[0] = 0x80;

  /* copy the silence pattern for a single channel accross multiple
     channels */
  for ( idx = 0 ; idx < channels ; idx++ ) {
    if ( bps == 16 ) {
      pattern_instance[idx * 2] = sixteen_bit_silence[0];
      pattern_instance[idx * 2 + 1] = sixteen_bit_silence[1];
    }
    else if ( bps == 8 ) {
      pattern_instance[idx] = eight_bit_silence[0];
    }
  }

  /* use the pattern_instance to fill the patter_buffer */
  for ( idx = 0 ; idx < 64 ; idx++ )
    memcpy(&(pattern_buf[idx * pattern_instance_sz]), pattern_instance, 
	   (size_t) pattern_instance_sz) ;
  for ( idx = 0 ; idx < PATTERNS_IN_PATTERN_BUF / 64 ; idx++ )
    memmove(&(pattern_buf[idx * pattern_instance_sz * 64]), pattern_buf, 
	    (size_t) (pattern_instance_sz * 64));

  /* write the silence to the output */
  for ( bytes_done = 0 ; bytes_done < bytes_silence ; 
               bytes_done += bytes_written ) {
    bytes_to_write = min(PATTERNS_IN_PATTERN_BUF * pattern_instance_sz,
			 bytes_silence - bytes_done);
    if ( (bytes_written = write(fd, pattern_buf, (size_t) bytes_to_write)) 
	 == -1 ) {
      if ( using_stdio == TRUE )
	err_die("write to standard output failed: %s\n", strerror(errno));
      else 
	err_die("write to %s failed: %s\n", file_name, strerror(errno));
    }
    if ( bytes_written < bytes_to_write ) {
      if (using_stdio == TRUE )
	err_die("wrote less than expected to standard output, giving up\n");
      else
	err_die("wrote less than expected to %s, giving up\n", file_name);
    }
  }    
 
  return;
}
