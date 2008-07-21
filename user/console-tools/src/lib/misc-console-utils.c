#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#include <lct/local.h>
#include <lct/console.h>

int is_a_console(int fd) 
{
  char arg;
  
  arg = 0;
  return (ioctl(fd, KDGKBTYPE, &arg) == 0
	  && ((arg == KB_101) || (arg == KB_84)));
}

static int open_a_console(char *fnam) 
{
  int fd;
  
  /* try read-only */
  fd = open(fnam, O_RDWR);
  
  /* if failed, try read-only */
  if (fd < 0 && errno == EACCES)
      fd = open(fnam, O_RDONLY);
  
  /* if failed, try write-only */
  if (fd < 0 && errno == EACCES)
      fd = open(fnam, O_WRONLY);
  
  /* if failed, fail */
  if (fd < 0)
      return -1;
  
  /* if not a console, fail */
  if (! is_a_console(fd))
    {
      close(fd);
      return -1;
    }
  
  /* success */
  return fd;
}

/*
 * Get an fd for use with kbd/console ioctls.
 * We try several things because opening /dev/console will fail
 * if someone else used X (which does a chown on /dev/console).
 *
 * if tty_name is non-NULL, try this one instead.
 */

int get_console_fd(char* tty_name) 
{
  int fd;

  if (tty_name)
    {
      if (-1 == (fd = open_a_console(tty_name)))
	return -1;
      else
	return fd;
    }
  
  fd = open_a_console("/dev/tty");
  if (fd >= 0)
    return fd;
  
  fd = open_a_console("/dev/tty0");
  if (fd >= 0)
    return fd;
  
  fd = open_a_console("/dev/console");
  if (fd >= 0)
    return fd;
  
  for (fd = 0; fd < 3; fd++)
    if (is_a_console(fd))
      return fd;
  
  fprintf(stderr,
	  _("Couldnt get a file descriptor referring to the console\n"));
  return -1;		/* total failure */
}


/* FIXME: Only on the current console? On all allocated consoles?
 * A newly allocated console has NORM_MAP by default -
 * probably it should copy the default from the current console?
 * But what if we want a new one because the current one is messed up? 
 * For the moment: only the current console
 * g_set specifies eight G0 or G1 using values 0 and 1 respectively.
 */
int acm_activate(int tty_fd, int g_set)
{
  if (is_a_console(tty_fd))
    if (g_set == 0)
      write(tty_fd, "\033(K\017", 4);
    else
      write(tty_fd, "\033)K\016", 4);
  else 
    {
      errno = ENODEV;
      return -1;
    }
  
  return 0;
}
