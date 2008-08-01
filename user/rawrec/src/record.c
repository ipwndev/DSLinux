/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function performs the actual recording run as specified by the
   options. */

#include <errno.h>
#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rawrec.h"
#include "thread_functions.h"

/* This flag gets set from the handler for signals we watch for to do
   special shutdown processing (currently only SIGTERM and SIGINT),
   since we can't POSIXly make the pthread calls directly from the
   handler.  */
extern int got_watched_for_shutdown_signal;
void shutdown_signal_handler(int signum);
/* This allows the following thread (move_au for play, move_fd for
   record) to tell this main thread when they finish. */
extern pthread_mutex_t tell_main_follower_done_mutex;
extern int tell_main_follower_done;

void record(parameters_stt *clp) 
{
  int audio_fd = 0;	  /* Audio device file descriptor.  */
  int arg_fd;		  /* Argument file fd.  */
  int bps;		  /* Bits per sample for the format in use.  */
  double rec_bytes;	  /* Bytes to record, not counting silent padding.  */
  long audio_fragsz;	  /* Kernel audio fragment size. */
  int ringbuf_segs;	  /* Number of segments in ringbuf. */
  move_au_th_arg_stt au_th_arg;	  /* Thread argument structure.  */
  move_fd_th_arg_stt fd_th_arg;	  /* Thread argument structure.  */
  int rtn;		  /* For return values of pthread fctns. */
  /* Maximum priority of FIFO thread.  Should always be rinitialized
     elsewhere before use.  */
  int fifo_max_prio = 0;	  
  void *au_th_ret;   	  /* Audio thread return pointer. */
  void *fd_th_ret;	  /* File thread return pointer. */
  sigset_t all_sigs;      /* Full set of all signals.  */
  sigset_t tmp_mask;	  /* Temporary signal set storage.  */
  struct sigaction shutdown_handler_act;   /* For shutdown handler.  */
  double sleep_time_secs;	  /* Compute for nanosleep().  */
  struct timespec sleep_time;	  /* For nanosleep() call.  */
  /* The actual parameters used for the record/play run may differ slightly
     from those given on the command line due to hardware limitations. */
  parameters_stt actual_params;
  /* Pointer to the actual_params structure.  */
  parameters_stt *app = NULL;	  /* bogus initializer reasures compiler */

  /* If the user wants us to selfishly hold the audio device, grab it now. */
  if ( clp->hold_audio_device == TRUE ) {
    /* This function checks for incorrect semantics (incorrect syntax
       is caught by the option processing switch in main() and
       reported by usage()), issues appropriate warnings for values
       the hardware can't support (eg. 23477 Hz sampling rate is
       probably not supported, so the value will need to be rounded),
       and on success returns the actual driver-compatible parameters
       to be used for the recording or playing run into actual_params.
       Note that in some cases the information pointed to by clp may
       still be relevant, for example when the user requests a sample
       format not supported by the native driver but for which rawrec
       can provide translation (there aren't currently any such
       formats).  If the user has things really fouled up, execution
       will be terminated. */
    actual_params = process_command_line(clp);  
    app = &actual_params;
    audio_fd = audio_init(app->audio_dev, FOR_READING, app->speed, 
			  app->format, app->channels);
  }

  /* Pause execution as promised.  If more than one of -p seconds or
     -P samples are specified, the one which is longest in real time
     wins. */
  sleep_on_option(clp->time_startpause, clp->samp_startpause, clp->speed);

  /* If we arn't holding the audio device, we still have to
     process_the_command_line.  This needs to be done asap, so we do
     it here. */
  if ( clp->hold_audio_device == FALSE ) {
    actual_params = process_command_line(clp);
    app = &actual_params;
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
               || (strcmp(clp->format, "u16_be") == 0)) 
    bps = 16;
  else if ( (strcmp(clp->format, "s8") == 0) 
	       || (strcmp(clp->format, "u8") == 0) )
    bps = 8;
  else
    err_die("BUG: unrecognized sample format seen in function '%s'\n",
	    __func__);

  /* Open the argument data file to be written to, or use standard output. */
  if ( app->using_stdio == TRUE )
    arg_fd = STDOUT_FILENO;
  else
    arg_fd = data_init(app->arg_file, FOR_WRITING);
 
  /* Record silence at start as promised in options -r
     (start_record_time) and/or -R (start_record_samples).  If -r and
     -R are both present the one which is longer in real time wins. */
  write_silence_on_option(arg_fd, app->using_stdio, app->arg_file, 
			  app->time_startrec, app->samp_startrec, 
			  app->speed, app->format, bps, app->channels);

  /* Determine the actual amount of data to be recorded, not including
     pause time or silent padding. */
  if ( app->timelim > (double) app->samplim / app->speed )
    rec_bytes = rint(app->timelim * app->speed) * bps 
                * (app->channels) / 8;
  else
    /* Note that precedence is critical here (rec_bytes may overflow
       without parenthesies). */
    rec_bytes = (double) app->samplim * (bps * app->channels / 8);

  /* Open the audio device, set up the ring buffer and launch the
     threads.  */

  /* Unless we're already selfishly holding the audio device, open it now. */
  if ( app->hold_audio_device == FALSE )
    audio_fd = audio_init(app->audio_dev, FOR_READING, app->speed, 
			  app->format, app->channels);

  /* Set or get the kernel audio buffer block size. */
  if ( app->set_fragsz == USER_FRAGSZ ) {
    set_au_blksz(audio_fd, app->fragsz);
    audio_fragsz = app->fragsz;
  } else			/* app->set_fragsz == AUTO_FRAGSZ */
    /* Get the block size used by the audio driver. */          
    audio_fragsz = get_au_blksz(audio_fd);

  /* Set up the ring buffer, it's sub buffers, and their associated
     mutexs. */
  ringbuf_segs = ringbuf_init(app->ringbufsz, audio_fragsz);
  
  /* Fill the move_au_th argument structure. */
  au_th_arg.startup_order = 1;
  au_th_arg.ringbuf_segs = ringbuf_segs; 
  au_th_arg.seg_sz = audio_fragsz;
  au_th_arg.fd = audio_fd;
  au_th_arg.recorder = 1;
  au_th_arg.byte_cnt = rec_bytes;
  au_th_arg.sample_size = bps * app->channels / 8;
  if ( (app->time_limit_set) || (app->samp_limit_set) )
    au_th_arg.limit_set = TRUE;
  else
    au_th_arg.limit_set = FALSE;
  
  /* Fill the move_fd_th argument structure. */
  fd_th_arg.startup_order = 2;
  fd_th_arg.ringbuf_segs = ringbuf_segs; 
  fd_th_arg.seg_sz = audio_fragsz;
  fd_th_arg.fd = arg_fd;
  fd_th_arg.recorder = 1;
  fd_th_arg.byte_cnt = rec_bytes;
  fd_th_arg.using_stdio = app->using_stdio;

  /* Set thread attributes.  See thread_scheme.txt in docs/programmer
     for rational. */
  if ( (rtn = pthread_attr_init(&move_au_attr)) )
    err_die("BUG: pthread_attr_init failed: %s\n", strerror(rtn));
  if ( (rtn = pthread_attr_setdetachstate(&move_au_attr, 
					  PTHREAD_CREATE_JOINABLE)) ) {
    err_die("BUG: pthread_attr_setdetachstate failed: %s\n", strerror(rtn));
  }
#if defined (_POSIX_THREAD_PRIORITY_SCHEDULING) \
    && _POSIX_THREAD_PRIORITY_SCHEDULING != -1 \
    && _POSIX_THREAD_PRIORITY_SCHEDULING != 0
  if ( have_root_authority ) {
    if ( (rtn = pthread_attr_setinheritsched(&move_au_attr, 
					     PTHREAD_EXPLICIT_SCHED)) ) {
      err_die("BUG: pthread_attr_setinheritsched failed: %s\n", strerror(rtn));
    }
    if ( (rtn = pthread_attr_setschedpolicy(&move_au_attr, SCHED_FIFO)) )
      err_die("BUG: pthread_attr_setschedpolicy failed: %s\n", strerror(rtn));
    if ( (fifo_max_prio = sched_get_priority_max(SCHED_FIFO)) == -1 )
      err_die("BUG: sched_get_priority_max failed: %s\n", strerror(errno));
    move_au_param.sched_priority = fifo_max_prio;
    if ( (rtn = pthread_attr_setschedparam(&move_au_attr, &move_au_param)) )
      err_die("BUG: pthread_attr_setschedparam failed: %s\n", strerror(rtn));
    if ( (rtn = pthread_attr_setscope(&move_au_attr, PTHREAD_SCOPE_SYSTEM)) )
      err_die("BUG: pthread_attr_setscope failed: %s\n", strerror(rtn));
  }
#endif
  if ( (rtn = pthread_attr_init(&move_fd_attr)) )
    err_die("BUG: pthread_attr_init failed: %s\n", strerror(rtn));
  if ( (rtn = pthread_attr_setdetachstate(&move_fd_attr, 
					  PTHREAD_CREATE_JOINABLE)) ) {
    err_die("BUG: pthread_attr_setdetachstate failed: %s\n", strerror(rtn));
  }
#if defined (_POSIX_THREAD_PRIORITY_SCHEDULING) \
    && _POSIX_THREAD_PRIORITY_SCHEDULING != -1 \
    && _POSIX_THREAD_PRIORITY_SCHEDULING != 0
  if ( have_root_authority ) {
    if ( (rtn = pthread_attr_setinheritsched(&move_fd_attr, 
					     PTHREAD_EXPLICIT_SCHED)) ) {
      err_die("BUG: pthread_attr_setinheritsched failed: %s\n", strerror(rtn));
    }
    if ( (rtn = pthread_attr_setschedpolicy(&move_fd_attr, SCHED_FIFO)) )
      err_die("BUG: pthread_attr_setschedpolicy failed: %s\n", strerror(rtn));
    move_fd_param.sched_priority = fifo_max_prio;
    if ( (rtn = pthread_attr_setschedparam(&move_fd_attr, &move_fd_param)) )
      err_die("BUG: pthread_attr_setschedparam failed: %s\n", strerror(rtn));
    if ( (rtn = pthread_attr_setscope(&move_fd_attr, PTHREAD_SCOPE_SYSTEM)) )
      err_die("BUG: pthread_attr_setscope failed: %s\n", strerror(rtn));
  }
#endif

  /* Getting ugly.  Here we install a handler (which sets a global
     flag which the threads can poll in order to do graceful
     death). */
  shutdown_handler_act.sa_handler = shutdown_signal_handler;
  sigfillset(&(shutdown_handler_act.sa_mask));
  shutdown_handler_act.sa_flags = 0;
  if ( sigaction(SIGTERM, &shutdown_handler_act, NULL) == -1 )
    err_die("sigaction failed: %s\n", strerror(errno));
  if ( sigaction(SIGINT, &shutdown_handler_act, NULL) == -1 )
    err_die("sigaction failed: %s\n", strerror(errno));

  /* Set up an empty signal set.  */
  if ( sigfillset(&all_sigs) == -1 )
    err_die("sigfillset failed: %s\n", strerror(errno));

  /* POSIX requires all signal sets to be initialized before use.  */
  if ( sigemptyset(&tmp_mask) == -1 )
    err_die("sigemptyset failed: %s\n", strerror(errno));

  /* Block all signals in preperation for starting threads.  We are
     moving into some linux specific hackery now, though it shouldn't
     make problems elsewhere.  The threads never unblock the signals,
     so the processes they run in are effectively unsignalable to
     everything but the unblockable signals.  Once the threads are
     started, the main thread handles important signals (at the
     moment, SIGTERM) and nanosleeps.  On sight of an important
     signal, we set a locked flag indicating that the signal has
     occured.  The move_au and move_fd threads poll this flag as
     readers, and exit at the end of the segment in progress if they
     detect it.  The "segment in progress" is the one which move_au is
     acting on, i.e. when playing, we try to exit as soon as possible,
     we don't try to finish playing all buffered data move_fd may have
     loaded for use.  Note that large segment sizes may still result
     in significant signal response latency.  */
  if ( sigprocmask(SIG_BLOCK, &all_sigs, &tmp_mask) == -1 )
    err_die("sigprocmask failed: %s\n", strerror(errno));

  if ( have_root_authority ) {
    /* Get root authority (the saved set-user id should be root).  */
    if ( seteuid( (uid_t) 0 ) == -1 ) 
      err_die("seteuid( (uid_t) 0 ) failed: %s\n", strerror(errno));

    /* Entering critical section.  Lock down our memory, if possible.  */
#if defined (_POSIX_MEMLOCK) && _POSIX_MEMLOCK != -1 && _POSIX_MEMLOCK != 0
    if ( mlockall(MCL_CURRENT) == -1 )
      err_die("mlockall(MCL_CURRENT) failed: %s\n", strerror(errno));
#endif
  }

  /* Start threads.  If we have root authority, then these threads
     will be using real time scheduling, and we must therefore be
     root to start them, and they therefore inherit effective root
     ids, but drop them as soon as all threads have been created. */  
  if ( (rtn = pthread_create (&move_au_th, &move_au_attr, 
			     (void *(*)(void *)) move_au, &au_th_arg)) ) {
    err_die ("BUG: pthread_create failed to create audio thread: %s\n",
	     strerror (rtn));
  }
  if ( (rtn = pthread_create (&move_fd_th, &move_fd_attr, 
			      (void *(*)(void *)) move_fd, &fd_th_arg)) ) {
    err_die ("BUG: pthread_create failed to create file thread: %s\n",
	     strerror (rtn));
  }

  if ( have_root_authority ) {
    /* Drop back to real uid authority. */
    if ( seteuid(getuid()) == -1 )
      /* If for some crazy reason we fail to drop root permissions, exit
	 immediately without advertising the fact. */
      err_die("BUG: internal error\n");
    /* Let other threads know that we have dropped root permissions
       (they wait for this before doing anything).  */
    pthread_mutex_lock(&root_permissions_dropped_mutex);
    root_permissions_dropped = 1;
    pthread_cond_broadcast(&root_permissions_dropped_cv);
    pthread_mutex_unlock(&root_permissions_dropped_mutex);
  }

  /* Restore the default mask.  Note that at no time has the default
     action for the shutdown signals we handle (sloppy death in which
     this initial thread expires but leaves its spawned threads to
     thrash on for a brief period, hopefully this mess will be fixed
     with kernel 2.5 and glibc 2.3) been allowed to take place after
     threads have been spawned.  */
  if ( sigprocmask(SIG_SETMASK, &tmp_mask, NULL) == -1 )
    err_die("sigprocmask failed: %s\n", strerror(errno));

  /* Now here is some sad hackery.  We can't sigwait() because these
     are asynchronous signals we are worried about, and they might
     never arrive, which would cause our main thread to hang forever.
     We can't create a dedicated thread and deflect signals there,
     because Linux pthreads aren't POSIX-conformant in that respect.
     So, we nanosleep in a loop (note that if we don't have a time or
     sample limit set, then the sample limit will have been set to its
     maximum value in process_comand_line (needs cleanup) and wait for
     a signal to come in and fire a handler for the only signal we
     currently deal with, SIGTERM.  The SIGTERM handler sets a global
     flag indicating that we got a shutdown signal, and we pass the
     word on to the thread functions via a mutex protected flag, then
     break out of the loop.  Also, the move_au thread packs it in when
     it doesn't find any data to read or finishes normally.  It has to
     let this main thread know when this happens so we can stop
     sleeping and waiting for signals and commence pthread_joining.
     It tells us via mutex-protected tell_main_follower_done, which we
     poll from here every half audio fragsz worth of time (hopefully
     giving maximum latency < fragsz as advertized).  */
  sleep_time_secs = (8.0 * audio_fragsz) / (2 * bps 
					    * app->speed * app->channels);
  sleep_time.tv_sec = (time_t) floor(sleep_time_secs);
  sleep_time.tv_nsec = (long) ((sleep_time_secs - floor(sleep_time_secs)) 
			       * 1000000000.0);
  for ( ; ; ) {
    nanosleep(&sleep_time, NULL);

    /* Has the following thread (move_fd in this case, since we are
       playing) finished? */
    pthread_mutex_lock(&tell_main_follower_done_mutex);
    if ( tell_main_follower_done ) {
      pthread_mutex_unlock(&tell_main_follower_done_mutex);
      break;
    }
    pthread_mutex_unlock(&tell_main_follower_done_mutex);


    /* We will be good boys and block watched for signals before
       checking whether we have seen them or not, even though branch on
       an int should be pretty darn atomic, and if we have made it this
       far without getting the signal we no longer care much whether we
       get it or not.  */
    if ( sigemptyset(&tmp_mask) == -1 )
      err_die("sigemptyset failed: %s\n", strerror(errno));
    if ( sigaddset(&tmp_mask, SIGTERM) == -1 )
      err_die("sigaddset failed: %s\n", strerror(errno));
    if ( sigaddset(&tmp_mask, SIGINT) == -1 )
      err_die("sigaddset failed: %s\n", strerror(errno));
    if ( sigprocmask(SIG_BLOCK, &tmp_mask, NULL) == -1 )
      err_die("sigprocmask failed: %s\n", strerror(errno));
    if ( got_watched_for_shutdown_signal ) {
      /* Let the move_au and move_fd threads know that we have been
	 interrupted by a signal that we catch in order to do clean
	 program termination, then break out of nanosleep loop.
	 Threads check shutdown_signal_seen after reading or writing
	 each segment from the ring buffer.  This could be done
	 slightly more efficiently with a reader-writer lock for the
	 play case, but it probably isn't worth the hassle and
	 complexity.  */
      pthread_mutex_lock(&shutdown_signal_seen_mutex);
      shutdown_signal_seen = 1;
      pthread_mutex_unlock(&shutdown_signal_seen_mutex);
      if ( sigprocmask(SIG_UNBLOCK, &tmp_mask, NULL) == -1 )
	err_die("sigprocmask failed: %s\n", strerror(errno));
      break;
    }
    if ( sigprocmask(SIG_UNBLOCK, &tmp_mask, NULL) == -1 )
      err_die("sigprocmask failed: %s\n", strerror(errno));
  }


  /* Wait for threads to finish. */
  if ( (rtn = pthread_join(move_au_th, &au_th_ret)) )
    err_die("BUG: pthread_join failed: %s\n", strerror(rtn));
  if ( *( (int *) au_th_ret) == -1 )
    err_die("abnormal termination of move_au_th, aborting\n");
  if ( (rtn = pthread_join(move_fd_th, &fd_th_ret)) )
    err_die("BUG: pthread_join failed: %s\n", strerror(rtn));
  if ( *( (int *) fd_th_ret) == -1 )
    err_die("abnormal termination of move_fd_th, aborting\n");

  /* As per our promise not to do things like end record (-e or -E) or
     end pause (-z or -Z) if we got interrupted by a signal.  This
     behavior may change eventually, but for the moment, since we only
     do special processing on SIGTERM, and the other terminating
     signals cause the clunky natural death which doesn't honor these
     options, we do it this way for consistency.  */
  if ( got_watched_for_shutdown_signal ) {
    ringbuf_close();		/* Free ring buffer and paraphenalia.  */
    data_close(arg_fd, app->arg_file); /* Close the argument data file.  */
    audio_close(audio_fd, app->audio_dev);
    return;
  }

  /* Unlock our address space.  Requires root permissions, perhaps
     best not to bother. */
  /* if ( munlockall() == -1 ) {
   *   fprintf(stderr, "%s: munlockall() failed: ", progname);
   *   perror("");
   *   exit(EXIT_FAILURE);
   * }
   */ 

  ringbuf_close();   /* Free ring buffer and paraphenalia. */

  data_close(arg_fd, app->arg_file);   /* Close the argument data file. */

  /* Unless we are selfishly holding onto the audio device... */
  if ( app->hold_audio_device == FALSE )
    audio_close(audio_fd, app->audio_dev); 

  /* Record silence at end as promised in options -e (end_record_time)
     and -E (end_record_samples). */
  write_silence_on_option(arg_fd, app->using_stdio, app->arg_file, 
			  app->time_endrec, app->samp_endrec, app->speed, 
			  app->format, bps, app->channels);

  /* Pause at end as promised. */
  sleep_on_option(app->time_endpause, app->samp_endpause, app->speed);
 
  /* If we have been selfishly holding onto the audio device, let it go now. */
  if ( app->hold_audio_device == TRUE )
    audio_close(audio_fd, app->audio_dev);

  return;
}
