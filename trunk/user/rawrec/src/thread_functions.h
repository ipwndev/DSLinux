/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This header contains material concerned mainly with the threads and
   shared data.  It is included in other files besides those
   containing the thread functions.  */

#include <pthread.h>

/* Structure type for move_au thread function arguments. */
typedef struct {
  /* These first three variables are things that are likely to be
     needed by most any ring buffer designed to be accessed by
     threads. */
  int startup_order;		/* Indicates the order in which the
                                   thread will first access the ring
                                   buffer relative to the other
                                   threads. 1 ==> 1st, 2 ==> 2nd,
                                   etc. */
  int ringbuf_segs;		/* Number of ring buffer segments. */
  int seg_sz;		        /* Size of a ring buffer segment. */
  /* These variables deal with the threads particular job (moving
     audio data). */
  int fd;			/* Audio device file descriptor.  */ 
  int recorder;                 /* == 1 ==> record, == 2  ==> play. */
  double byte_cnt;		/* Bytes to record or play. */
  int sample_size;		/* size of a complete sample, in bytes */
  boolean limit_set;		/* Has the user specified -t or -T? */
} move_au_th_arg_stt;

/* Structure type for move_fd thread function arguments. */
typedef struct {
  /* These first three variables are things that are likely to be
     needed by most any ring buffer designed to be accessed by
     threads. */
  int startup_order;		/* Indicates the order in which the
                                   thread will first access the ring
                                   buffer relative to the other
                                   threads. 1 ==> 1st, 2 ==> 2nd,
                                   etc. */
  int ringbuf_segs;		/* Number of ring buffer segments. */
  int seg_sz;		        /* Size of a ring buffer segment. */
  /* These variables deal with the threads particular job (moving
     audio data). */
  int fd;			/* Disk file file descriptot.  */ 
  int recorder;                 /* == 1 ==> record, == 2  ==> play. */
  double byte_cnt;		/* Bytes to record or play. */
  boolean using_stdio;		/* Are we using stdio? */
} move_fd_th_arg_stt;

/* See globals.c for descriptions of the roles of these globals. */

/* Note that sig_th isn't used yet. */
extern pthread_t sig_th, move_au_th, move_fd_th;
extern pthread_attr_t sig_th_attr, move_au_attr, move_fd_attr;
extern struct sched_param move_au_param, move_fd_param; 

extern const int th_error;
extern const int th_success;

extern unsigned char *ringbufp;
extern long *bytes_in_seg;
extern int *is_last_seg;

extern int root_permissions_dropped;
extern pthread_cond_t root_permissions_dropped_cv;
extern pthread_mutex_t root_permissions_dropped_mutex;

extern int startup_next;
extern pthread_cond_t startup_next_cv;
extern pthread_mutexattr_t startup_next_mutex_attr;
extern pthread_mutex_t startup_next_mutex;

extern int wrap_ready;
extern pthread_cond_t wrap_ready_cv;
extern pthread_mutex_t wrap_ready_mutex;

extern pthread_mutexattr_t *seg_mutex_attr;
extern pthread_mutex_t *seg_mutex;

extern int shutdown_signal_seen;
extern pthread_mutex_t shutdown_signal_seen_mutex;

extern const int MAGIC_EMPTY_SEG_SEQ_LENGTH;

/* Prototypes for the thread functions themselves. */
void *move_au(move_au_th_arg_stt *th_arg);
void *move_fd(move_fd_th_arg_stt *th_arg);

