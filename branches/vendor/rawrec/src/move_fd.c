/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function does the actual moving of the audio data between the
   ring buffer and the arg_fd disk data file.  This function should
   only be executed as a thread.  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "rawrec.h"
#include "thread_functions.h"

/* Junk move_au uses to let the main thread (which gets to do signal
   handling) know that we have finished.  See comments in play.c for a
   fuller explanation of how this works.  Needs to be replaced when
   kernel/glibc thread support gets better. */
extern pthread_mutex_t tell_main_follower_done_mutex; 
extern int tell_main_follower_done;
 
void * move_fd(move_fd_th_arg_stt *th_arg)
{
  double bytes_done = 0;       /* Number of bytes already moved. */
  /* This flag is true if this is our first time around the ring buffer.  */
  int first_time_around = 1;   

  /* This global should have been set before any threads were spawned,
     and should never get changed.  */
  if ( have_root_authority ) {
    /* We want to drop root permissions as soon as all the threads
       have been created.  But effective root permissions are a
       process wide attribute (rather than a per thread attribute like
       in the old linux threads implementation) so if we drop them
       immediately on thread entry, we might not be able to create the
       second data thread (move_au or move_fd, depending on startup
       order).  So we wait here for the thread doing the spawning to
       set a condition variable indicating that root permissions have
       been dropped before going on.  */
    pthread_mutex_lock(&root_permissions_dropped_mutex);
    while ( !root_permissions_dropped ) {
      /* This loop guards against spurious wakeups, which according to
	 pthreads standard are possible.  */
      pthread_cond_wait(&root_permissions_dropped_cv, 
			&root_permissions_dropped_mutex);
    }
    pthread_mutex_unlock (&root_permissions_dropped_mutex);
  }

  /* If this thread has startup priority of 1, grab the first buffer
     segment and signal the move_au_th when done. */
  if ( th_arg->startup_order == 1 ) {
    pthread_mutex_lock(&seg_mutex[0]);
    /* Signal the startup_next condition variable.  Mutex needs attrs
       still. */
    pthread_mutex_lock(&startup_next_mutex);
    startup_next = 2;
    pthread_cond_signal(&startup_next_cv);
    pthread_mutex_unlock(&startup_next_mutex);
  } else {			
    /* In this program there are only two threads trying to access the
       ring buffer, and this else clause handles the case where this
       thread is not the first to get the buffer (where this thread
       has startup_order == 2). */
    pthread_mutex_lock(&startup_next_mutex);
    while ( startup_next != 2 ) 
      /* This loop guards against spurious wakeups.  In this
         application there is no third thread to slip in and change
         startup_next, and indeed if this value is being used as
         intended no slip in should ever occur. */
      pthread_cond_wait(&startup_next_cv, &startup_next_mutex);
    /* Grab what we've been waiting for clearance to grab. */
    pthread_mutex_lock(&seg_mutex[0]);
    pthread_mutex_unlock(&startup_next_mutex);
    /* In this case, we're done with startup_next_cv and friends. */
    pthread_cond_destroy(&startup_next_cv);
    pthread_mutexattr_destroy(&startup_next_mutex_attr);
    pthread_mutex_destroy(&startup_next_mutex);
  }

  if ( th_arg->recorder ) {
    int bytes_to_write;		/* # of bytes to write with next write() */
    int bytes_written;		/* number of bytes written by write syscall */
    int crnt_seg = 0;		/* current ring buffer segment index */

    /* Perform thread duties. */
    for ( bytes_done = 0 ; bytes_done < th_arg->byte_cnt ; 
               bytes_done += bytes_written ) {
      /* Note that this catches and reports on the broken pipe condition. */
      bytes_to_write = (int) min(th_arg->seg_sz, th_arg->byte_cnt 
				                 - bytes_done);
      if ( (bytes_written = write(th_arg->fd, ringbufp + th_arg->seg_sz 
               * crnt_seg, (size_t) bytes_to_write)) == -1 ) {
	err_die("write syscall to output failed: %s\n", strerror(errno));
      }
      if ( bytes_written < bytes_to_write ) {
	err_die("write syscall wrote less than expected to output, " 
		"giving up\n");
      }

      /* If this segment was marked as the last by the move_au thread,
	 break out of the record loop.  */
      if ( is_last_seg[crnt_seg] )
	break;

      pthread_mutex_lock(&seg_mutex[(crnt_seg + 1) % th_arg->ringbuf_segs]);
      pthread_mutex_unlock(&seg_mutex[crnt_seg]);
      crnt_seg = (crnt_seg + 1) % th_arg->ringbuf_segs;
    }
    /* Unlock the last segment locked by the above for loop. */
    pthread_mutex_unlock(&seg_mutex[crnt_seg]);

    /* start signal handling hackery */
    /* We are recording, so move_fd is the following thread.  Let the
       main thread know that we are done.  */
    pthread_mutex_lock(&tell_main_follower_done_mutex);
    tell_main_follower_done = 1;
    pthread_mutex_unlock(&tell_main_follower_done_mutex);
    /* end signal handling hackery */

  } else {  /* If not record, then play. */
    int bytes_to_read;		   /* # of bytes to read with next read() */
    int bytes_read;		   /* holds return of read() syscalls */
    int crnt_seg = 0;		   /* current ring buffer segment index */
    int empty_seg_seq_length = 0;  /* number of empty segments in a row */

    /* This for loop is similar to the above except it reads data from
       the file descriptor, rather then writing to it, so we have to
       worry about pipeline undersupply (instead of just assuming a
       terminal disaster if we read less than we wanted).  */
    for ( bytes_done = 0 ; bytes_done < th_arg->byte_cnt ; 
               bytes_done += bytes_read ) {
      bytes_to_read = min(th_arg->seg_sz, th_arg->byte_cnt - bytes_done);
      if ( (bytes_read = read(th_arg->fd, ringbufp + th_arg->seg_sz
               * crnt_seg, (size_t) bytes_to_read)) == -1 ) {
	err_die("read of input file descriptor failed: %s\n", strerror(errno));
      }

      /* If we were reading from a file and not a pipeline, and we got
         less than we asked for, give up (if we are using standard io,
         the case where we read many empty segments is handled by
         move_au, so that we don't quit prematurely if there is a
         bunch of good data in the ring buffer waiting to be played). */
      if ( (th_arg->using_stdio == FALSE) 
               && (bytes_read < bytes_to_read) ) {
	err_die("read syscall read less than expected from argument file, "
		"giving up\n");
      }
      /* keep track of the number of empty segments in a row */
      if ( bytes_read == 0 ) 
	empty_seg_seq_length++;
      else			/* bytes_read != 0 */
	empty_seg_seq_length = 0;
      /* tell the move_au thread how many bytes got read into crnt_seg */
      bytes_in_seg[crnt_seg] = bytes_read;

      /* Before we wrap back around to the start of the ring buffer
         for the first time, we need to make sure that the move_au
         thread has had a chance to at least get started and grab the
         first buffer segment. */
      if ( (first_time_around) && (crnt_seg == th_arg->ringbuf_segs - 1) ) {
	pthread_mutex_lock(&wrap_ready_mutex);
	while ( !wrap_ready )
	  /* This while loop guards against spurious wakeups.  */
	  pthread_cond_wait(&wrap_ready_cv, &wrap_ready_mutex);
	pthread_mutex_unlock(&wrap_ready_mutex);
	first_time_around = 0;	/* No longer our first time around.  */
	/* We are now done with wrap_ready_cv and friends.  */
	pthread_cond_destroy(&wrap_ready_cv);
	pthread_mutex_destroy(&wrap_ready_mutex);
      }

      /* If we have seen a signal which is to cause program exit,
         exit.  In this play context, move_fd leads move_au, but we
         don't want to wait for everything move_fd may have loaded
         into the buffer to play, so there is no need to notify
         move_au of the point where we quit, it will just quit as soon
         as it can anyway.  */
      pthread_mutex_lock(&shutdown_signal_seen_mutex);
      if ( shutdown_signal_seen ) {
	/* unlock the mutex to be sure move_au gets a chance to check it.  */
	pthread_mutex_unlock(&shutdown_signal_seen_mutex);
	break;  /* out of play loop.  */
      }
      pthread_mutex_unlock(&shutdown_signal_seen_mutex);

      if ( empty_seg_seq_length >= MAGIC_EMPTY_SEG_SEQ_LENGTH )
	/* This thread is done, move_au will worry about this
           condition when it catches up. */
	break;			/* out of play loop */

      /* Perform staggered locking, and update crnt_seg.  */
      pthread_mutex_lock(&seg_mutex[(crnt_seg + 1) % th_arg->ringbuf_segs]);
      pthread_mutex_unlock(&seg_mutex[crnt_seg]);
      crnt_seg = (crnt_seg + 1) % th_arg->ringbuf_segs;
    }
    /* Unlock the last segment locked by the above for loop. */
    pthread_mutex_unlock(&seg_mutex[crnt_seg]);
  }

  return ( (void *) &th_success);
}
