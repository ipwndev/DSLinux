/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function allocates a big locked down ring buffer consisting of
   many smaller sub buffers of size segsz bytes each, and returns a
   long integer representing the actual number of segment allocated
   (if the ringbufsz is not an even multiple of segsz, an extra
   segment is allocated).  An array of mutexs, a condition variable,
   an integer thread startup order counter, and a mutex for the
   startup order variable are also created and initialized with the
   appropriate scheduling options.  Note that the actual thread and
   thread attribute objects are not dealt with here, but in the record
   and play functions.  For a full description of the thread scheme
   and associated synchronization paraphenalia, see thread_scheme.txt
   in docs/programmer. */

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rawrec.h"
#include "thread_functions.h"

/* need to figure out how to lock these buffers down yet. */
long ringbuf_init(long ringbufsz, int segsz)
{
  long seg_cnt;			/* The number of sub buffers. */
  long actual_ringbufsz;	/* After rounding up to next multiple
				   of segsz. */
  int idx;			/* For the mutex and cond init loop. */
  int rtn;			/* For return values of pthread fctns. */

  /* FIXME: Linux doesn't set these symbols to the data like it
     supposed to, so we have to test for the magic numbers which mean
     the feature isn't available or requires run time test (we
     currently don't try to do run time tests.  If/when this changes
     this stuff should be fixed.  */
#if (defined (USE_FANCY_PRIORITY_CODE)) \
    && ((defined (_POSIX_THREAD_PRIO_INHERIT) \
	 && _POSIX_THREAD_PRIO_INHERIT != -1 \
	 && _POSIX_THREAD_PRIO_INHERIT != 0) \
	|| (defined (_POSIX_THREAD_PRIO_PROTECT) \
	    && _POSIX_THREAD_PRIO_PROTECT != -1 \
	    && _POSIX_THREAD_PRIO_PROTECT != 0))
  int mutex_protocol;		/* For checking the current mutex
                                   priority boost policy. */
#endif  

  /* This mutex protects a condition variable (initialized in
     globals.c) which controls the order in which threads will get to
     grab the first segment of the buffer. */
  if ( (rtn = pthread_mutexattr_init(&startup_next_mutex_attr)) )
    err_die("BUG: pthread_mutexattr_init failed: %s\n", strerror(rtn));
  /* Fight priority inversion.  Is this necessary for a mutex
     protecting a cv? */
#if (defined (USE_FANCY_PRIORITY_CODE)) \
    && (defined (_POSIX_THREAD_PRIO_INHERIT) \
        && _POSIX_THREAD_PRIO_INHERIT != -1 \
        && _POSIX_THREAD_PRIO_INHERIT != 0)
  pthread_mutexattr_getprotocol(&startup_next_mutex_attr, &mutex_protocol);
  if ( mutex_protocol != PTHREAD_PRIO_INHERIT )
    pthread_mutexattr_setprotocol(&startup_next_mutex_attr, 
                                  PTHREAD_PRIO_INHERIT);
#elif (defined (USE_FANCY_PRIORITY_CODE)) \
      && (defined (_POSIX_THREAD_PRIO_PROTECT) \
          && _POSIX_THREAD_PRIO_PROTECT != -1 \
          && _POSIX_THREAD_PRIO_PROTECT != 0)
  pthread_mutexattr_getprotocol(&startup_next_mutex_attr, &mutex_protocol);
  if ( mutex_protocol != PTHREAD_PRIO_PROTECT ) {
    pthread_mutexattr_setprotocol(&startup_next_mutex_attr, 
                                  PTHREAD_PRIO_PROTECT);
    pthread_mutexattr_setprioceiling(&startup_next_mutex_attr,
                                     sched_get_priority_max(SCHED_RR));
  }
#endif

  /* The number of sub buffers, and associated actual_ringbufsz. */
  seg_cnt = ringbufsz / segsz;
  if ( ringbufsz % segsz != 0 ) 
    seg_cnt++;			
  /* Insist on at least three segments.  This allows staggered locking
     to work correctly.  */
  if ( seg_cnt < 3 )		
    seg_cnt = 3;
  actual_ringbufsz = seg_cnt * segsz;

  /* Storage for the ring buffer and paraphenalia. */
  if ( (ringbufp = (unsigned char *) malloc( (size_t) actual_ringbufsz)) 
               == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( (bytes_in_seg = (long *) malloc( (size_t) (seg_cnt * sizeof(long))))
               == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( (is_last_seg = (int *) malloc( (size_t) (seg_cnt * sizeof(int)))) 
               == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( (seg_mutex = (pthread_mutex_t *) malloc( (size_t) (seg_cnt 
               * sizeof(pthread_mutex_t)))) == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
  if ( (seg_mutex_attr = (pthread_mutexattr_t *) malloc( (size_t) (seg_cnt 
               * sizeof(pthread_mutexattr_t)))) == NULL ) {
    err_die("malloc failed: %s\n", strerror(errno));
  }
    
  /* Initialize the mutexs.  */
  for ( idx = 0; idx < seg_cnt; idx++ ) {
    /* is_last_seg is for flagging segments to handle special termination. */
    is_last_seg[idx] = 0;

    /* Fight priority inversion.  If it comes up, the audio will skip,
       but this should help with damage control.  */
    if ( (rtn = pthread_mutexattr_init(&seg_mutex_attr[idx])) )
      err_die("BUG: pthread_mutexattr_init failed: %s\n", strerror(rtn));
#if (defined (USE_FANCY_PRIORITY_CODE)) \
    && (defined (_POSIX_THREAD_PRIO_INHERIT) \
        && _POSIX_THREAD_PRIO_INHERIT != -1 \
        && _POSIX_THREAD_PRIO_INHERIT != 0)
    pthread_mutexattr_getprotocol(&seg_mutex_attr[idx], &mutex_protocol);
    if ( mutex_protocol != PTHREAD_PRIO_INHERIT )
      pthread_mutexattr_setprotocol(&seg_mutex_attr[idx], 
                                    PTHREAD_PRIO_INHERIT);
#elif (defined (USE_FANCY_PRIORITY_CODE)) \
      && (defined (_POSIX_THREAD_PRIO_PROTECT) \
          && _POSIX_THREAD_PRIO_PROTECT != -1 \
          && _POSIX_THREAD_PRIO_PROTECT != 0)
    pthread_mutexattr_getprotocol(&seg_mutex_attr[idx], &mutex_protocol);
    if ( mutex_protocol != PTHREAD_PRIO_PROTECT ) {
      pthread_mutexattr_setprotocol(&seg_mutex_attr[idx], 
                                    PTHREAD_PRIO_PROTECT);
      pthread_mutexattr_setprioceiling(&seg_mutex_attr[idx],
                                       sched_get_priority_max(SCHED_RR));
    }
#endif
    if ( (rtn = pthread_mutex_init(&seg_mutex[idx], &seg_mutex_attr[idx])) )
      err_die("BUG: pthread_mutex_init failed: %s\n", strerror(rtn));
  }

  /* Return the number of segsz sized ring buffer segments allocated.  */
  return seg_cnt;	
}
