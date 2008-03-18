/* Evil evil evil hack to get OSS apps to cooperate with esd
 * Copyright (C) 1998, 1999 Manish Singh <yosh@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* #define DSP_DEBUG */

/* This lets you run multiple instances of x11amp by setting the X11AMPNUM
   environment variable. Only works on glibc2.
 */
/* #define MULTIPLE_X11AMP */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)

#ifdef DSP_DEBUG
#define DPRINTF(format, args...)	printf(format, ## args)
#else
#define DPRINTF(format, args...)
#endif

#include "config.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_MACHINE_SOUNDCARD_H
#  include <machine/soundcard.h>
#else
#  ifdef HAVE_SOUNDCARD_H
#    include <soundcard.h>
#  else
#    include <sys/soundcard.h>
#  endif
#endif

#include "esd.h"

/* BSDI has this functionality, but not define :() */
#if defined(RTLD_NEXT)
#define REAL_LIBC RTLD_NEXT
#else
#define REAL_LIBC ((void *) -1L)
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__bsdi__)
typedef unsigned long request_t;
#else
typedef int request_t;
#endif

static int sndfd = -1, mixfd = -1;
static int settings = 0, done = 0;

static char *ident = NULL, *mixer = NULL;
static int use_mixer = 0;

/*
 * memory mapping emulation
 */
static int mmapemu = 0;
static count_info mmapemu_ocount;
static char *mmapemu_obuffer = NULL;
static int mmapemu_osize;
static long long mmapemu_bytes_per_sec;
struct timeval mmapemu_last_flush;

#define MMAPEMU_FRAGSTOTAL 16
#define MMAPEMU_FRAGSIZE ESD_BUF_SIZE

#define OSS_VOLUME_BASE 50

#define ESD_VOL_TO_OSS(left, right) (short int)			\
    (((OSS_VOLUME_BASE * (right) / ESD_VOLUME_BASE) << 8) |	\
      (OSS_VOLUME_BASE * (left)  / ESD_VOLUME_BASE))

#define OSS_VOL_TO_ESD_LEFT(vol) 				\
    (ESD_VOLUME_BASE * (vol & 0xff) / OSS_VOLUME_BASE)
#define OSS_VOL_TO_ESD_RIGHT(vol) 				\
    (ESD_VOLUME_BASE * ((vol >> 8) & 0xff) / OSS_VOLUME_BASE)

static void
get_volume (int *left, int *right)
{
  int vol;

  if (read (mixfd, &vol, sizeof (vol)) != sizeof (vol))
    *left = *right = ESD_VOLUME_BASE;
  else
    {
      *left  = OSS_VOL_TO_ESD_LEFT  (vol);
      *right = OSS_VOL_TO_ESD_RIGHT (vol);
    }
}

static void
set_volume (int left, int right)
{
  int vol = ESD_VOL_TO_OSS (left, right);

  write (mixfd, &vol, sizeof (vol));
}


static void
dsp_init (void)
{
  if (!ident)
    {
      const char *str;
     
      str = getenv ("ESDDSP_NAME");
      ident = malloc (ESD_NAME_MAX);
      strncpy (ident, (str ? str : "esddsp"), ESD_NAME_MAX);

      str = getenv("ESDDSP_MMAP");
      mmapemu = str && !strcmp(str,"1");

      if (getenv ("ESDDSP_MIXER"))
	{
	  use_mixer = 1;

	  str = getenv ("HOME");
	  if (str)
 	    {
	      mixer = malloc (strlen (str) + strlen (ident) + 10);
	      sprintf (mixer, "%s/.esddsp_%s", str, ident);
	    }
	  else
	    {
	      fprintf (stderr, "esddsp: can't get home directory\n");
	      exit (1);
	    }

	  DPRINTF ("mixer settings file: %s\n", mixer);
	}
    }
}

static void
mix_init (int *esd, int *player)
{
  esd_info_t *all_info;
  esd_player_info_t *player_info;

  if (*esd < 0 && (*esd = esd_open_sound (NULL)) < 0)
    return;
    
  if (*player < 0)
    {
      all_info = esd_get_all_info (*esd);
      if (all_info)
	{
	  for (player_info = all_info->player_list; player_info;
	       player_info = player_info->next)
	    if (!strcmp(player_info->name, ident))
	      {
		*player = player_info->source_id;
		break;
	      }

	  esd_free_all_info (all_info);
	}
    }
}

static void mmapemu_flush(void)
{
  struct timeval current_time;
  long long usec_since_last_flush;
  long long bytes_to_flush;
  int frags_to_flush;

  gettimeofday(&current_time, NULL);
  usec_since_last_flush = 1000000 * (current_time.tv_sec - mmapemu_last_flush.tv_sec) + (current_time.tv_usec - mmapemu_last_flush.tv_usec);

  bytes_to_flush = (mmapemu_bytes_per_sec * usec_since_last_flush) / 1000000;
  frags_to_flush  = bytes_to_flush / MMAPEMU_FRAGSIZE;

  if (frags_to_flush > MMAPEMU_FRAGSTOTAL)
    frags_to_flush = MMAPEMU_FRAGSTOTAL;

  if (frags_to_flush > 0)
    mmapemu_last_flush = current_time;

  while (frags_to_flush-- > 0)
    {
      write(sndfd, &mmapemu_obuffer[mmapemu_ocount.ptr], MMAPEMU_FRAGSIZE);
      mmapemu_ocount.ptr += MMAPEMU_FRAGSIZE;
      mmapemu_ocount.ptr %= mmapemu_osize;
      mmapemu_ocount.blocks++;
      mmapemu_ocount.bytes += MMAPEMU_FRAGSIZE;
    }
}

static int
open_wrapper (int (*func) (const char *, int, mode_t),
	      const char *pathname, int flags, ...)
{
  va_list args;
  mode_t mode;

  dsp_init ();

  va_start (args, flags);
  mode = va_arg (args, mode_t);
  va_end (args);

  if (!strcmp (pathname, "/dev/dsp"))
    {
      if (!getenv ("ESPEAKER") && !mmapemu)
	{
          int ret;

	  flags |= O_NONBLOCK;
	  if ((ret = (*func) (pathname, flags, mode)) >= 0)
	    return ret;
	}

      DPRINTF ("hijacking /dev/dsp open, and taking it to esd...\n");
      settings = done = 0;
      return (sndfd = esd_open_sound (NULL));
    }
  else if (use_mixer && !strcmp (pathname, "/dev/mixer"))
    {
      DPRINTF ("hijacking /dev/mixer open, and taking it to esd...\n");
      return (mixfd = (*func) (mixer, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    }
  else
    return (*func) (pathname, flags, mode);
}

int
open (const char *pathname, int flags, ...)
{
  static int (*func) (const char *, int, mode_t) = NULL;
  va_list args;
  mode_t mode;

  DPRINTF ("open\n");

  if (!func) 
    {
      func = (int (*) (const char *, int, mode_t)) dlsym (REAL_LIBC, "open");
      if (!func) 
	{
	  fprintf(stderr, "esddsp: error: Cannot find symbol 'open'\n");
	  errno = ENXIO;
	  return -1;
	}
    }

  va_start (args, flags);
  mode = va_arg (args, mode_t);
  va_end (args);

  return open_wrapper(func, pathname, flags, mode);
}

int
open64 (const char *pathname, int flags, ...)
{
  static int (*func) (const char *, int, mode_t) = NULL;
  va_list args;
  mode_t mode;

  DPRINTF ("open64\n");

  if (!func) 
    {
      func = (int (*) (const char *, int, mode_t)) dlsym (REAL_LIBC, "open64");
      if (!func) 
	{
	  fprintf(stderr, "esddsp: error: Cannot find symbol 'open64'\n");
	  errno = ENXIO;
	  return -1;
	}
    }

  va_start (args, flags);
  mode = va_arg (args, mode_t);
  va_end (args);

  return open_wrapper(func, pathname, flags, mode);
}

static FILE *
fopen_wrapper (FILE * (*func) (const char *, const char *),
	       const char *pathname, const char *mode)
{
  dsp_init ();

  if (!strcmp (pathname, "/dev/dsp"))
    {
      if (!getenv ("ESPEAKER") && !mmapemu)
	{
          FILE *ret;

	  if ((ret = (*func) (pathname, mode)) != NULL)
	    return ret;
	}

      DPRINTF ("hijacking /dev/dsp fopen, and taking it to esd...\n");
      settings = done = 0;
      return fdopen((sndfd = esd_open_sound (NULL)), mode);
    }
  else if (use_mixer && !strcmp (pathname, "/dev/mixer"))
    {
      FILE *ret;
      DPRINTF ("hijacking /dev/mixer fopen, and taking it to esd...\n");

      if ((ret = (*func) (pathname, mode)) != NULL)
	{
	  mixfd = fileno(ret);
	  return ret;
	}
      else
        return NULL;
    }
  else
    return (*func) (pathname, mode);
}

FILE *
fopen (const char *path, const char *mode)
{
  static FILE * (*func) (const char *, const char *) = NULL;

  DPRINTF ("fopen\n");

  if (!func) 
    {
      func = (FILE * (*) (const char *, const char *)) dlsym (REAL_LIBC, "fopen");
      if (!func) 
	{
	  fprintf(stderr, "esddsp: error: Cannot find symbol 'fopen'\n");
	  return NULL;
	}
    }

  return fopen_wrapper(func, path, mode);
}

FILE *
fopen64 (const char *path, const char *mode)
{
  static FILE * (*func) (const char *, const char *) = NULL;

  DPRINTF ("fopen64\n");

  if (!func) 
    {
      func = (FILE * (*) (const char *, const char *)) dlsym (REAL_LIBC, "fopen64");
      if (!func) 
	{
	  fprintf(stderr, "esddsp: error: Cannot find symbol 'fopen64'\n");
	  return NULL;
	}
    }

  return fopen_wrapper(func, path, mode);
}

static int
dspctl (int fd, request_t request, void *argp)
{
  static esd_format_t fmt = ESD_STREAM | ESD_PLAY | ESD_MONO;
  static int speed;

  int *arg = (int *) argp;

  DPRINTF ("hijacking /dev/dsp ioctl, and sending it to esd "
	   "(%d : %x - %p)\n", fd, request, argp);
  
  switch (request)
    {
    case SNDCTL_DSP_RESET:
    case SNDCTL_DSP_POST:
      break;

    case SNDCTL_DSP_SETFMT:
      fmt |= (*arg & 0x30) ? ESD_BITS16 : ESD_BITS8;
      settings |= 1;
      break;

    case SNDCTL_DSP_SPEED:
      speed = *arg;
      settings |= 2;
      break;

    case SNDCTL_DSP_STEREO:
      fmt &= ~ESD_MONO;
      fmt |= (*arg) ? ESD_STEREO : ESD_MONO;
      break;

    case SNDCTL_DSP_GETBLKSIZE:
      if (mmapemu)
      	*arg = MMAPEMU_FRAGSIZE;
      else
      	*arg = ESD_BUF_SIZE;
     
      break;

    case SNDCTL_DSP_GETFMTS:
      *arg = 0x38;
      break;

#ifdef SNDCTL_DSP_GETCAPS
    case SNDCTL_DSP_GETCAPS:
      if (mmapemu)
	*arg = DSP_CAP_MMAP | DSP_CAP_TRIGGER | DSP_CAP_REALTIME;
      else
	*arg = 0;
      break;
#endif

    case SNDCTL_DSP_GETOSPACE:
    case SNDCTL_DSP_GETISPACE:
      {
	audio_buf_info *bufinfo = (audio_buf_info *) argp;
	if (mmapemu)
	  {
	    /* Number of fragments allocated for buffering */
	    bufinfo->fragstotal = MMAPEMU_FRAGSTOTAL;
	    /* Size of a fragment in bytes (same as SNDCTL_DSP_GETBLKSIZE) */
	    bufinfo->fragsize = MMAPEMU_FRAGSIZE;
	    /* Number of bytes that can be read or written immediately without blocking */
	    bufinfo->bytes = MMAPEMU_FRAGSIZE;
	    /* Number of full fragments that can be read or written without blocking */
	    bufinfo->fragments = 1;
	  }
	else
	  {
	    /* Number of fragments allocated for buffering */
	    bufinfo->fragstotal = 1;
	    /* Size of a fragment in bytes (same as SNDCTL_DSP_GETBLKSIZE) */
	    bufinfo->fragsize = ESD_BUF_SIZE;
	    /* Number of bytes that can be read or written immediately without blocking */
	    bufinfo->bytes = ESD_BUF_SIZE;
	    /* Number of full fragments that can be read or written without blocking */
	    bufinfo->fragments = 1;
	  }
      }
      break;

    case SNDCTL_DSP_GETOPTR:
      if (mmapemu)
        {
	  DPRINTF("esddsp: mmapemu getoptr\n");
	  mmapemu_flush();
	  *((count_info *)arg) = mmapemu_ocount;
	  mmapemu_ocount.blocks = 0;
	}
      else
        {
	  DPRINTF ("esddsp: SNDCTL_DSP_GETOPTR unsupported\n");
	}
      break;

    default:
      DPRINTF ("unhandled /dev/dsp ioctl (%x - %p)\n", request, argp);
      break;
    }

  if (settings == 3 && !done)
    {
      int proto = ESD_PROTO_STREAM_PLAY;

      done = 1;

      if (write (sndfd, &proto, sizeof (proto)) != sizeof (proto))
        return -1;
      if (write (sndfd, &fmt, sizeof (fmt)) != sizeof (fmt))
        return -1;
      if (write (sndfd, &speed, sizeof (speed)) != sizeof (speed))
        return -1;
      if (write (sndfd, ident, ESD_NAME_MAX) != ESD_NAME_MAX)
        return -1;

      if (mmapemu)
	{
	  mmapemu_ocount.ptr=mmapemu_ocount.blocks=mmapemu_ocount.bytes=0;
	  mmapemu_bytes_per_sec = speed * ((fmt & ESD_BITS16) ? 2 : 1) * ((fmt & ESD_STEREO) ? 2 : 1);
	  gettimeofday(&mmapemu_last_flush, NULL);
	}

      fmt = ESD_STREAM | ESD_PLAY | ESD_MONO;
      speed = 0;

      if (use_mixer)
	{
  	  int esd = -1, player = -1;
	  int left, right;

	  while (player < 0)
	    mix_init (&esd, &player);

	  get_volume (&left, &right);

	  DPRINTF ("panning %d - %d %d\n", player, left, right);
	  esd_set_stream_pan (esd, player, left, right);

	  esd_close(esd);
	}
    }

  return 0;
}

int
mixctl (int fd, request_t request, void *argp)
{
  static int esd = -1, player = -1;
  static int left, right;

  int *arg = (int *) argp;

  DPRINTF ("hijacking /dev/mixer ioctl, and sending it to esd "
	   "(%d : %x - %p)\n", fd, request, argp);

  switch (request)
    {
    case SOUND_MIXER_READ_DEVMASK:
      *arg = 5113;
      break;

    case SOUND_MIXER_READ_PCM:
      mix_init (&esd, &player);

      if (player > 0)
	{
	  esd_info_t *all_info;

	  all_info  = esd_get_all_info (esd);

	  esd_close(esd);

	  if (all_info)
	    {
	      esd_player_info_t *player_info;

	      for (player_info = all_info->player_list; player_info;
		   player_info = player_info->next)
		if (player_info->source_id == player)
		  {
		    *arg = ESD_VOL_TO_OSS (player_info->left_vol_scale,
					   player_info->right_vol_scale);
		  }

	      esd_free_all_info (all_info);
	    }
	  else
	    return -1;
	}
      else
	{
          get_volume (&left, &right);
          esd_close(esd);
	  *arg = ESD_VOL_TO_OSS (left, right);
	}

      break;

    case SOUND_MIXER_WRITE_PCM:
      mix_init (&esd, &player);

      left  = OSS_VOL_TO_ESD_LEFT  (*arg);
      right = OSS_VOL_TO_ESD_RIGHT (*arg);

      set_volume (left, right);

      if (player > 0)
	{
	  DPRINTF ("panning %d - %d %d\n", player, left, right);
	  esd_set_stream_pan (esd, player, left, right);
	}

      esd_close(esd);

      break;

    default:
      DPRINTF ("unhandled /dev/mixer ioctl (%x - %p)\n", request, argp);
      break;
    }

  return 0;
}

int
ioctl (int fd, request_t request, ...)
{
  static int (*func) (int, request_t, void *) = NULL;
  va_list args;
  void *argp;

  if (!func)                                                                    
    func = (int (*) (int, request_t, void *)) dlsym (REAL_LIBC, "ioctl");             
  va_start (args, request);
  argp = va_arg (args, void *);
  va_end (args);

  if (fd == sndfd)
    return dspctl (fd, request, argp);
  else if (fd == mixfd) {
    if(use_mixer)
      return mixctl (fd, request, argp);
  } else { /* (fd != sndfd && fd != mixfd) */
        return (*func) (fd, request, argp); 
      }
  return 0;
}

int
close (int fd)
{
  static int (*func) (int) = NULL;

  if (!func)
    func = (int (*) (int)) dlsym (REAL_LIBC, "close");

  if (fd == sndfd)
    sndfd = -1;
  else if (fd == mixfd)
    mixfd = -1;
 
  return (*func) (fd);
}

void *
mmap (void  *start,  size_t length, int prot, int flags, int fd, off_t offset)
{
  static void *(*func) (void *, size_t, int, int, int, off_t) = NULL;
  
  if (!func)
    func = (void * (*) (void *, size_t, int, int, int, off_t)) dlsym (REAL_LIBC, "mmap");

  if(fd != sndfd || sndfd == -1)
    return (*func)(start,length,prot,flags,fd,offset);
  else
    {
      DPRINTF ("esddsp: mmap - start = %x, length = %d, prot = %d\n",
	       start, length, prot);
      DPRINTF ("      flags = %d, fd = %d, offset = %d\n",flags, fd,offset);
      if(mmapemu)
	{
	  mmapemu_osize = length;
	  mmapemu_obuffer = malloc(length);
	  mmapemu_ocount.ptr = mmapemu_ocount.blocks = mmapemu_ocount.bytes = 0;
	  return mmapemu_obuffer;
	}
      else DPRINTF ("esddsp: /dev/dsp mmap (unsupported, try -m option)...\n");
    }
  return (void *)-1;
}

int
munmap (void *start, size_t length)
{
  static int (*func) (void *, size_t) = NULL;

  if (!func)
    func = (int (*) (void *, size_t)) dlsym (REAL_LIBC, "munmap");

  if(start != mmapemu_obuffer || mmapemu_obuffer == 0)
    return (*func)(start,length);

  DPRINTF ("esddsp: /dev/dsp munmap...\n");
  mmapemu_obuffer = NULL;
  free(start);

  return 0;
}

#ifdef MULTIPLE_X11AMP

#include <socketbits.h>
#include <sys/param.h>
#include <sys/un.h>

#define ENVSET "X11AMPNUM"

int
unlink (const char *filename)
{
  static int (*func) (const char *) = NULL;
  const char *num;

  if (!func)
    func = (int (*) (const char *)) dlsym (REAL_LIBC, "unlink");

  if (!strcmp (filename, "/tmp/X11Amp_CTRL") && (num = getenv (ENVSET)))
    {
      char buf[PATH_MAX] = "/tmp/X11Amp_CTRL";
      strncat (buf, num, (PATH_MAX - strlen(buf) - 1));
      return (*func) (buf); 
    }
  else
    return (*func) (filename);
}

typedef int (*sa_func_t) (int, struct sockaddr *, int);

static int
sockaddr_mangle (sa_func_t func, int fd, struct sockaddr *addr, int len)
{
  const char *num;

  if (!strcmp (((struct sockaddr_un *) addr)->sun_path, "/tmp/X11Amp_CTRL")
      && (num = getenv(ENVSET)))
    {
      int ret;
      char buf[PATH_MAX] = "/tmp/X11Amp_CTRL";

      struct sockaddr_un *new_addr = malloc (len);

      strncat (buf, num, (PATH_MAX - strlen(buf) - 1));
      memcpy (new_addr, addr, len);
      strncpy (new_addr->sun_path, buf, strlen(new_addr->sun_path));

      ret = (*func) (fd, (struct sockaddr *) new_addr, len);

      free (new_addr);
      return ret;
    } 
  else
    return (*func) (fd, addr, len);
}

int
bind (int fd, struct sockaddr *addr, int len)
{
  static sa_func_t func = NULL;

  if (!func)
    func = (sa_func_t) dlsym (REAL_LIBC, "bind");
  return sockaddr_mangle (func, fd, addr, len);
}

int
connect (int fd, struct sockaddr *addr, int len)
{
  static sa_func_t func = NULL;

  if (!func)
    func = (sa_func_t) dlsym (REAL_LIBC, "connect");
  return sockaddr_mangle (func, fd, addr, len);
}

#endif /* MULTIPLE_X11AMP */

#else /* __GNUC__ */

void
nogcc (void)
{
  ident = NULL;
}

#endif

