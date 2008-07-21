/*
 * clrunimap.c
 *
 * Note: nowadays this kills kernel console output!
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/console.h>

static void usage(char *progname)
{
  printf(_("Usage: %s\n"
	   "Clears the Unicode map from the console.\n"
	   "Note: nowadays this kills kernel console output!\n"), progname);
  OPTIONS_ARE();
  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

void main(int argc, char **argv)
{
  struct unimapinit advice;
  int fd;

  setuplocale();
  
  simple_options(argc, argv, usage, strip_path(argv[0]));
  
  if (-1 == (fd = get_console_fd(NULL)))
      exit (1);
  
  advice.advised_hashsize = 0;
  advice.advised_hashstep = 0;
  advice.advised_hashlevel = 0;
  
  if(ioctl(fd, PIO_UNIMAPCLR, &advice)) 
    {
      perror("PIO_UNIMAPCLR");
      exit(1);
    }
  
  exit(0);
}
