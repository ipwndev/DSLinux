/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* Global variables.  Most are thread-related. */

#include <pthread.h>

/* Here because everybody needs it to report errors.  This gets set to
   the full name of the program as invoked on the command line
   (i.e. presumably argv[0]).  */
char *progname;

/* This gets set true on program start if the executable is suid root.  */
int have_root_authority;

/* "move*" because any given execution either records or plays, not
   both.  Data is moved by move_au_th between the sampling device and
   the main buffer, and by move_fd_th between the main buffer and the
   applicable file descriptor.  We don't know in advance which way. */
pthread_t move_au_th, move_fd_th;
pthread_attr_t move_au_attr, move_fd_attr;
struct sched_param move_au_param, move_fd_param; 

/* Return values for thread functions, because these functions can
   only return pointers, and pointers to automatic variables end up
   pointing to garbage.  Currently, only success or failure can be
   indicated. */
const int th_failure = -1;
const int th_success = 0;

/* The ring buffer is what all these other pthreads objects are
   synchronizing.  Note that not all data that goes in this buffer is
   necesarilly unsigned, but this doesn't matter, since all this
   buffer does is hold data that is on route to or from the audio
   driver, which interprets it according to the current mode for the
   device (set with SNDCTL_DSP_SETFMT). */
unsigned char *ringbufp;
/* When playing from standard input, it is possible that some buffer
   segments may not always end up completely full.  This array allows
   the move_fd thread to tell the mova_au thread how many good bytes
   are in each segment (the remaining bytes are garbage or data left
   over from the last time around the buffer). */
long *bytes_in_seg;
/* Allow whichever of move_au and move_fd is in the lead to mark the
   segment it is working on as the last segment to be processed.
   Currently this is used to achieve clean shutdown after receiving a
   signal for which we do special shutdown processing.  */
int *is_last_seg;

/* If we have_root_authority, we want to drop root permissions as soon
   as possible.  We have to get all the high priority threads created
   first though.  So the high priority threads all block at the start
   waiting for a signal that root permission have been dropped before
   doing anything.  */
int root_permissions_dropped = 0;
pthread_cond_t root_permissions_dropped_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t root_permissions_dropped_mutex = PTHREAD_MUTEX_INITIALIZER;

/* This condition variable is used ensure that any threads which
   concurrently begin to access the ring buffer do so in the desired
   order.  The first thread to get the ringbuffer will be the one with
   the startup_order field of it's argument structure equal to 1.
   When this thread is done it can broadcast a wakeup signal to the
   other threads waiting on the condition variable, and the one with
   startup_order == 2 can take over.  Note that it is completely up to
   the thread code implementor to specify when an given thread is
   'done' and the next thread should be allowed to start accessing the
   ring buffer, and to somehow determine from the startup_next cv
   which thread to startup next.  In conjunction with a staggered
   locking mechanism and the wrap_ready_cv, this allows just about any
   kind of access scheme you can dream up.  Most applications should
   be fairly simple.  */
int startup_next = 1; 
pthread_cond_t startup_next_cv = PTHREAD_COND_INITIALIZER; 
/* The mutex attribute object and mutex for the startup_next condition
   variable. */
pthread_mutexattr_t startup_next_mutex_attr;
pthread_mutex_t startup_next_mutex;

/* This condition variable indicates whether or not it has become safe
   for threads to wrap around the end of the ring buffer and start
   processing the first segment again.  This protects against the case
   where an early thread is so fast that it processes all the buffer
   segments and begins working on the first buffer segment again
   before other slower threads even get going and lock that buffer.  */
int wrap_ready = 0;
pthread_cond_t wrap_ready_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t wrap_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

/* These arrays work together to synchronize ring buffer segment
   access.  Note that each mutex protects both a single "ioctl(au_fd,
   SNDCTL_DSP_GETBLKSIZE, &blocksize)" or "SNDCTL_DSP_SETFRAGMENT"
   sized segment of the ring buffer, a corresponding element of the
   bytes_in_seg array, and a corresponding element of the is_last_seg
   array.  In other words, all data associated with a given ringbuffer
   segment is protected by an element of this mutex array.  */
pthread_mutexattr_t *seg_mutex_attr; 
pthread_mutex_t *seg_mutex;	

/* This allows the thread which does signal handling (which
   unfortunately is the initial thread, since Linux Threads doesn't
   yet conform to POSIX with respect to per thread signal masking) to
   indicate to other threads that a signal for which we do special
   shutdown processing has come in.  */
int shutdown_signal_seen;
pthread_mutex_t shutdown_signal_seen_mutex = PTHREAD_MUTEX_INITIALIZER;

/* This is a first attempt at sensible behavior for when we find
   ourselves waiting for standard input.  If we see
   MAGIC_EMPTY_SEG_SEQ_LENGTH empty segments in a row, we assume game
   over. */
const int MAGIC_EMPTY_SEG_SEQ_LENGTH = 5;
