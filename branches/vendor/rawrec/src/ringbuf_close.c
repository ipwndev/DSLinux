/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function frees the big buffer and it's associated mutex,
   condition variable, and flag arrays.  See thread_scheme.txt in
   docs/programmer for a more general and complete descriptions of the
   ring buffer and it's synchronization paraphenalia. */

#include <stdlib.h>

#include "rawrec.h"
#include "thread_functions.h"

void ringbuf_close(void)
{
  free(ringbufp);
  free(bytes_in_seg);
  free(is_last_seg);
  free(seg_mutex);
  free(seg_mutex_attr);
}

