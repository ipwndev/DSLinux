/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function does the actual moving of the audio data between the
   audio device (or more specificly the kernel audio buffer) and the
   ring buffer.  This function should only be executed as a
   thread.  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "rawrec.h"
#include "thread_functions.h"

/* Junk move_au uses to let the main thread (which gets to do signal
   handling) know that we have finished, either because we find we
   have run out of standard input or because we finished normally.
   See comments in play.c for a fuller explanation of how this works.
   Needs to be replaced when kernel/glibc thread support gets
   better. */
extern pthread_mutex_t tell_main_follower_done_mutex; 
extern int tell_main_follower_done;

void *move_au(move_au_th_arg_stt *th_arg)
{
  double bytes_done = 0;	   /* Number of bytes already moved. */

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
     segment and signal the move_fd_th when done. */
  if ( th_arg->startup_order == 1 ) {
    pthread_mutex_lock(&seg_mutex[0]);
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
    /* Let move_fd thread know that we have started, and that it is
       therefore safe to wrap back around to the first buffer
       segment.  */
    pthread_mutex_lock(&wrap_ready_mutex);
    wrap_ready = 1;		/* Set wrap_ready flag to true.  */
    pthread_cond_signal(&wrap_ready_cv);
    pthread_mutex_unlock(&wrap_ready_mutex);
    pthread_mutex_unlock(&startup_next_mutex);
    /* In this case, we're done with startup_next_cv and friends. */
    pthread_cond_destroy(&startup_next_cv);
    pthread_mutexattr_destroy(&startup_next_mutex_attr);
    pthread_mutex_destroy(&startup_next_mutex);
  }

  if ( th_arg->recorder ) {
    int bytes_to_read;		/* # of bytes to read with next read() */
    int bytes_read;		/* number of bytes read by read syscall */
    int crnt_seg = 0;		/* current ring buffer segment idnex */

    /* Perform thread duties and staggered locking.  */
    for ( bytes_done = 0 ; bytes_done < th_arg->byte_cnt ; 
               bytes_done += bytes_read ) {
      bytes_to_read = (int) min( th_arg->seg_sz, 
				 th_arg->byte_cnt - bytes_done);
      if ( (bytes_read = read(th_arg->fd, ringbufp + th_arg->seg_sz 
               * crnt_seg, (size_t) bytes_to_read)) == -1 )
	err_die("read syscall of dsp device failed: %s\n", strerror(errno));
      if ( bytes_read < bytes_to_read ) {
	err_die("read syscall read less than expected from dsp device, "
		"giving up\n");
      }

      /* If we have seen a signal which is to cause program exit...  */
      pthread_mutex_lock(&shutdown_signal_seen_mutex);
      if ( shutdown_signal_seen ) {
	/* It shouldn't matter whether we unlock the mutex if we saw
	   the signal or not.  I'm not sure doing so is really the
	   more righteous path either.  */
	pthread_mutex_unlock(&shutdown_signal_seen_mutex);
	/* Mark this as the last segment so move_fd knows what's up.  */
	is_last_seg[crnt_seg] = 1;
	break;			/* out of record loop. */
      }
      pthread_mutex_unlock(&shutdown_signal_seen_mutex);
      if ( pthread_mutex_trylock(&seg_mutex[(crnt_seg + 1) 
               % th_arg->ringbuf_segs]) == EBUSY ) {
	err_die("main buffer overflow, giving up\n");
      }
      pthread_mutex_unlock(&seg_mutex[crnt_seg]);
      crnt_seg = (crnt_seg + 1) % th_arg->ringbuf_segs;
    }
    /* Unlock the last segment locked by the above for loop. */
    pthread_mutex_unlock(&seg_mutex[crnt_seg]);

  } else {			  /* If not record, then play. */
    /* This for loop is similar to the above except it writes data to
       the audio device rather than reading from it, and we have to
       worry about pipeline undersupply if we are using stdio.
       Currently, if we see MAGIC_EMPTY_SEG_SEQ_LENGTH empty buffer
       segments in a row, we assume that the writer to our standard
       input has terminated or fritzed and exit. */
    int bytes_to_write;     	  /* # of bytes to write with next write() */
    int bytes_written = 0;	  /* bytes written by last write syscall */
    int crnt_seg = 0;		  /* current ring buffer segment index */
    int empty_seg_seq_length = 0; /* number of empty segments in a row */

    for ( bytes_done = 0 ; bytes_done < th_arg->byte_cnt ;
               bytes_done += bytes_written ) {
      if ( bytes_in_seg[crnt_seg] == 0 ) {
	empty_seg_seq_length++;
	bytes_written = 0;
      } else {			/* bytes_in_seg[crnt_seg] != 0 */
	empty_seg_seq_length = 0;
	/* To keep the audio device in sync, we need to write full
           samples, so we throw away a little bit of data if we got a
           wierd number of bytes in a segment due to pipeline
           undersupply. */
	/* FIXME: This approach seems totally broken, since presumably
	   the next segment will start with a partial sample which
	   will get munged with the next sample, causing the rest of
	   the playback to be noise. */
	bytes_to_write = bytes_in_seg[crnt_seg] - (bytes_in_seg[crnt_seg] 
						   % th_arg->sample_size);

	if ( (bytes_written = write(th_arg->fd, ringbufp + th_arg->seg_sz 
                    * crnt_seg, (size_t) bytes_to_write)) == -1 ) {
	  err_die("write to audio device failed: %s\n", strerror(errno));
	}
	if ( bytes_written < bytes_to_write ) {
	  err_die("wrote less than expedted to audio device, giving up\n");
	}
      }

      /* If we have seen a signal which is to cause program exit...  */
      pthread_mutex_lock(&shutdown_signal_seen_mutex);
      if ( shutdown_signal_seen ) {
	/* unlock the mutex to be sure move_fd gets a chance to check it.  */
	pthread_mutex_unlock(&shutdown_signal_seen_mutex);
	break;	 /* out of play loop.  */
      }
      pthread_mutex_unlock(&shutdown_signal_seen_mutex);

      /* If we have seen too many empty segments, react appropriately.  */
      if ( empty_seg_seq_length >= MAGIC_EMPTY_SEG_SEQ_LENGTH ) {
	if ( th_arg->limit_set == TRUE ) {
	  err_die("too many empty ring buffer segments (starved for standard "
		  "input), giving up\n");
	} else {	        /* th_arg->limit_set == FALSE */
	  break;		/* break out of play loop */
	}
      }

      /* Perform staggered locking, and update crnt_seg.  */
      pthread_mutex_lock(&seg_mutex[(crnt_seg + 1) % th_arg->ringbuf_segs]);
      pthread_mutex_unlock(&seg_mutex[crnt_seg]);
      crnt_seg = (crnt_seg + 1) % th_arg->ringbuf_segs;    
    }
    /* Unlock the last segment locked by the above for loop. */
    pthread_mutex_unlock(&seg_mutex[crnt_seg]);

    /* start signal handling hackery */
    /* We are playing, so move_au is the following thread.  Let the
       main thread know that we are done.  */
    pthread_mutex_lock(&tell_main_follower_done_mutex);
    tell_main_follower_done = 1;
    pthread_mutex_unlock(&tell_main_follower_done_mutex);
    /* end signal handling hackery */
  }

  return ( (void *) &th_success);
}
