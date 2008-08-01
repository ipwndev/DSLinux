/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function performs various sanity checks on the parameters
   supplied by the user on the command line, makes mild or
   inconsequential adjustments (like setting the speed option to a
   multiple of the sound cards crystal oscillator frequency) and
   issues warnings or fatal errors as appropriate (given in particular
   the presense or absense of the verbose flag -v).  Not everything is
   checked for here.  For example, the file and device names given are
   tested for validity only when they are first used, since their
   status could concievably change between the time this function is
   executed and the time the files or devices are actually used.  On
   the other hand, things like the fundamental capabilities of the
   sampling hardware are unlikely to change in flight, and the user
   could really dream up a lot of crazy things to try, so the checks
   are isolated here in their own function. */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rawrec.h"

parameters_stt process_command_line(parameters_stt *clp)
{
  /* Some limits which may not have been set in the command line will
     probably be set by the time process_command_line finished
     running, notably ap.samp_limit_set will be true, unless standard
     io is being used. */
  parameters_stt ap;	        /* actual parameters for this execution */
  open_how how;			/* How to open audio device -- for
                                   reading or writing? */
  int bps;			/* the bits per sample for the format in use */
  struct stat file_stats;	/* for holding file stats */

  /* Begin with the actual parameters identical to those specified on
     the command line. */
  ap = *clp;  

  /* if we are selfishly holding the dsp device... */
  if ( clp->hold_audio_device == TRUE ) {
    /* then check to see if the device specified looks reasonable
       (otherwise we have to wait until it's first used, in case there
       is a competing program) */
    if ( stat(clp->audio_dev, &file_stats) == -1 ) {
      err_die("stat of audio dsp device %s failed: %s\n", clp->audio_dev, 
	      strerror(errno));
    }
    if ( !(S_ISCHR(file_stats.st_mode)) ) {
      err_die("%s is not a character special file (it doesn't look like a valid dsp device)\n", clp->audio_dev);
    }
  }

  /* Confirm that the dsp device can handle the parameters the user wants. */
  if ( clp->recorder == TRUE )
    how = FOR_READING;
  else				/* clp->recorder == FALSE */
    how = FOR_WRITING;
  switch ( test_dsp_params(clp->audio_dev, how, &(ap.speed), ap.format, 
			   &(ap.channels)) ) { 
  case INVALID_RATE: 
    /* If rate was invalid, test_dsp_params won't have adjusted ap.speed.  */
    err_die("the sampling rate (-s) could not be set to anything close to the intended value of %d\n", ap.speed);
  case INVALID_CHANNELS:
    err_die("the number of channels (-c) could not be set to the intended value of %d\n", ap.channels);
  case INVALID_FORMAT:  
    err_die("the sample format (-f) could not be set to the intended format: %s\n", ap.format);
  case INEXACT_ARG:
    /* test_dsp_params had to go with slight variation on requested values.  */
    if ( clp->channels != ap.channels )
      fprintf(stderr, "%s: warning: number of channels (-c) will be set to %d instead of requested value of %d\n", progname, ap.channels, clp->channels);
    if ( (clp->speed != ap.speed) && (clp->verbose == TRUE) ) 
      fprintf(stderr, "%s: warning: speed (-s) will be set to %d instead of requested value of %d\n", progname, ap.speed, clp->speed);
    break;
  case SUCCESS:
    break;
  default:
    err_die("BUG: arrived at invalid default case in switch in function "
	    "'%s'\n", __func__);
  }

  /* Determine the bits per sample from the format.  The format
     specified by the user (and stored in the structure pointed to by
     clp) is used, since if this differs from the driver compatible
     format selected in actual_params, then translation is presumably
     in use.  At present, no translation is performed, and I'm not at
     all sure I ever want to support translating between samples of
     different resolutions anyway. */
  if ( (strcmp(clp->format, "s16_le") == 0) 
               || (strcmp(clp->format, "u16_le") == 0)
               || (strcmp(clp->format, "s16_be") == 0)
               || (strcmp(clp->format, "u16_be") == 0)) {
    bps = 16;
  } else if ( (strcmp(clp->format, "s8") == 0) 
	      || (strcmp(clp->format, "u8") == 0) ) {
    bps = 8;
  } else {
    err_die("BUG: unrecognized sample format seen in function '%s'\n", 
	    __func__);
  }
  
  if ( (clp->ringbufsz * ( 8.0 / (double) bps) 
	* (1.0 / (double) clp->speed) < 1.0) && (clp->verbose == TRUE) ) {
    fprintf(stderr, "%s: warning: the main buffer size (specified with -B) will allow less than one second of data to be buffered at the specified sampling rate (-s), sample format (-f), and number of channels (-c)\n", progname);    
  }

  /* Try to shelter the user of SNDCTL_DSP_SETFREGMENT a bit. */
  if ( clp->set_fragsz == USER_FRAGSZ ) {
    double estimated_latency;	/* for the requested frag size, in seconds */

    if ( clp->fragsz < 16 ) {
      err_die("buffer fragment size requested (with -g) was less than the minimum allowable value of 16 (bytes)\n");
    }
    if ( (clp->fragsz < 256) && (clp->verbose == TRUE) )
      fprintf(stderr, "%s: warning: main buffer fragment size (specified with -g) is less than the recommended minimum value of 64 (bytes)\n", 
               progname);
    estimated_latency = ( (double) (clp->fragsz * 8)) 
               / ( (double) (bps * ap.channels * ap.speed));
    /* if estimated_latency is greater than 50 ms and verbose is on, warn */
    if ( (estimated_latency > 0.05) && (clp->verbose == TRUE) )
      fprintf(stderr, "%s: warning: the fragment size specified (with -g) will result in at least %.0f ms of latency at the specified sampling rate (-s), sample format (-g), and number of channels (-c)\n", progname, 
               estimated_latency * 1000.0);
    /* warn that really big fragments may not be granted by the driver */
    if ( (clp->fragsz > 16384) && (clp->verbose == TRUE) )
      fprintf(stderr, "%s: warning: large fragment sizes may not be granted by the audio device driver (the fragment size may be set to a smaller value)\n",
	       progname);
    /* make sure fragment size is a power of two */
    if ( (is_pow_two(clp->fragsz) == FALSE) && (clp->verbose == TRUE) ) 
      fprintf(stderr, "%s: warning: the fragment size (specified with -g) will be adjusted up from %ld to the nearest power of two\n", progname, 
	       clp->fragsz);
    for ( ap.fragsz = 16 ; ap.fragsz <= clp->fragsz ; ap.fragsz *= 2 )
      ;
    
  }
  
  /* if we are playing a datafile argument from the command line (as
     opposed to standard io), see if it looks reasonable. */
  if ( (clp->using_stdio == FALSE) && (clp->recorder == FALSE) ) {
    double play_bytes;		/* number of bytes to play */
    /* number of bytes to jump, initialized to default */
    double jump_bytes = 0;	

    if ( stat(clp->arg_file, &file_stats) == -1 ) {
      err_die("stat of argument file %s failed: %s\n", clp->arg_file, 
	      strerror(errno));
    }
    if ( S_ISDIR(file_stats.st_mode) ) {
      err_die("error: argument file %s is a directory, not a regular file\n", 
	      clp->arg_file);
    } else if ( !(S_ISREG(file_stats.st_mode)) ) {
      err_die("argument file %s is not a regular file\n", clp->arg_file);
    }

    /* Compute the number of bytes to jump into the file.  If both -j
       seconds and -J samples are specified, the one which is longest
       in real time (farthest into the file) is the one used. */
    if ( ap.time_startjump > 0 || ap.samp_startjump > 0 ) {
      if ( ap.time_startjump > ( (double) ap.samp_startjump) 
               / ap.speed ) 
	jump_bytes = floor( (double) ap.time_startjump 
			   * (double) ap.speed * (double) bps 
			   * (double) ap.channels / 8.0);
      else 
	jump_bytes = (double) ap.samp_startjump * (double) bps 
	             * (double) ap.channels / 8.0;      
    }

    /* Determine the actual amount of data to be played, not including
       pause time. */
    if ( (clp->time_limit_set == FALSE) && (clp->samp_limit_set == FALSE) ) 
      /* play_bytes is adjusted so that we will be trying to play what
         is left of the file after jumping jump_bytes into it. */
      play_bytes = (double) file_stats.st_size - jump_bytes; 
    else {			
      if ( ap.timelim > (double) ap.samplim / ap.speed )
	play_bytes = floor(ap.timelim * ap.speed) * ap.channels 
               * bps / 8; 
      else
	play_bytes = (double) ap.samplim * ap.channels * bps / 8; 
    }

    /* If there isn't enough data to play as required, warn and adjust
       as appropriate.  Note that we casually go through and set all
       these limits in ap without changing the samp_limit_set boolean
       flag since things work out all right with lower layers.  Note
       also that it's always samplim which ends up getting set to
       something greater than zero, i.e. ends up doing all the work
       since timelim might result in bad roundoff error.  In other
       words, this mess should probably be cleaned up somehow. */
    if ( (double) file_stats.st_size <= jump_bytes ) {
      if ( clp->verbose == TRUE )
	fprintf(stderr, "%s: warning: -j or -J option specified will result in the entire argument data file being skipped, and nothing played\n", progname);
      ap.timelim = 0;
      ap.samplim = 0;
      ap.time_startjump = 0;
      ap.samp_startjump = (long) file_stats.st_size / (ap.channels * bps / 8);
    } else if ( (double) file_stats.st_size < jump_bytes + play_bytes ) {
      if ( clp->verbose == TRUE )
	fprintf(stderr, "%s: warning: argument data file contains insufficient data to jump as specified (with -j or -J) and play the requested amount of data (-t or -T), playing to end of file\n", progname);
      ap.timelim = 0;
      ap.samplim = ( (double) file_stats.st_size - jump_bytes) 
               / (bps * ap.channels / 8);
    } else {			/* no modification needed */
      ap.timelim = 0;
      ap.samplim = play_bytes * 8 / (bps * ap.channels);
    }
    /* At this point, the time limit and sample limit are gauranteed
       to have been set.  Ok to do this because it only happens if we
       are playing from something other than standard io, so move_au
       will still know not to freak out if standard io ends and there
       is no limit set.  Fragile needs rework.  */
    ap.time_limit_set = TRUE;
    ap.samp_limit_set = TRUE;
  }

  if ( (clp->recorder == FALSE) && (clp->using_stdio == TRUE) 
               && (clp->time_limit_set == FALSE) 
               && (clp->samp_limit_set == FALSE) ) {
    /* Play as long as possible, until standard input ends.  Note that
       although samplim gets set here, its artificial, so we don't set
       the limit_set flags (yes this needs cleanup).  */
#ifdef __GNUC__
    ap.samplim = INT64_MAX;
#else
    ap.samplim = LONG_MAX; 
#endif
  }

  if ( (clp->recorder == TRUE) && (clp->time_limit_set == FALSE) 
               && (clp->samp_limit_set == FALSE) ) {
    /* Record as long as possible, until interrupted.  */
#ifdef __GNUC__
    ap.samplim = INT64_MAX;
#else
    ap.samplim = LONG_MAX; 
#endif
  }

  return ap;
}
