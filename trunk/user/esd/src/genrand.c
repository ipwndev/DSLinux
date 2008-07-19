#include "config.h"

#include "genrand.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef  MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef  MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))


/* AIX requires this to be the first thing in the file.  */
#if defined(__GNUC__)

# if defined(__STRICT_ANSI__)
#  define alloca __builtin_alloca
# endif

#else

# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# elif defined(_AIX)
 #pragma alloca
# elif !defined(alloca) /* predefined by HP cc +Olibcalls */
char *alloca ();
# endif

#endif


static int
genrand_dev(unsigned char *buffer, int buf_len)
{
  int fd;

  fd = open("/dev/random", O_RDONLY);
  if(fd < 0)
    return 0;

  if(read(fd, buffer, buf_len) < buf_len)
    {
      close(fd);
      return 0;
    }

  close(fd);

  return 1;
}

static volatile int received_alarm = 0;

static void
handle_alarm(int signum)
{
  received_alarm = 1;
}

static unsigned char
hashlong(long val)
{
  unsigned char retval = 0, *ptr;
  int i;

  for(ptr = (unsigned char *)&val, i = 0; i < sizeof(val); i++)
    retval ^= ptr[i];

  return retval;
}

static int
genrand_unix(unsigned char *buffer, int buf_len)
{
  struct sigaction sa, oldsa;
  struct itimerval it, oldit;
  int i;
  long min, max;
  long *counts;
  double diff;
  long *uninit;

  counts = alloca(buf_len * sizeof(long));

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_alarm;
  sigaction(SIGALRM, &sa, &oldsa);
  memset(&it, 0, sizeof(it));
  it.it_value.tv_usec = 1;
  getitimer(ITIMER_REAL, &oldit);

  for(i = 0, min = LONG_MAX, max = 0; i < buf_len; i++)
    {
      received_alarm = 0;
      setitimer(ITIMER_REAL, &it, NULL);
      for(counts[i] = 0; !received_alarm; counts[i]++);

      max = MAX(counts[i], max);
      min = MIN(counts[i], min);
    }

  if(!(max - min))
    return 0;

  diff = max - min;

  uninit = alloca(buf_len * sizeof(long)); /* Purposely not initialized */
  for(i = 0; i < buf_len; i++)
    {
      long diffval;
      diffval = counts[i] - min;

      buffer[i] ^= (unsigned char)( ((double) (diffval*256)  / diff )) ^ hashlong(uninit[i]);
    }

  setitimer(ITIMER_REAL, &oldit, NULL);
  sigaction(SIGALRM, &oldsa, NULL);

  return 1;
}

void
esound_genrand(unsigned char *buffer, int buf_len)
{
  if(genrand_dev(buffer, buf_len))
    return;
  else if(genrand_unix(buffer, buf_len))
    return;
  else
    abort();
}

