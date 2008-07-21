/*
 * call: setkeycode scancode keycode ...
 *  (where scancode is either xx or e0xx, given in hexadecimal,
 *   and keycode is given in decimal)
 *
 * aeb, 941108
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

char *progname;

static void usage() 
{
  printf(_("Usage: %s [options] scancode keycode ...\n"
	   " (where scancode is either xx or e0xx, given in hexadecimal,\n"
	   "  and keycode is given in decimal)\n"), progname);
  OPTIONS_ARE();
  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}


void main(int argc, char **argv) 
{
  char *ep;
  int fd, sc;
  struct kbkeycode a;
  const struct option long_opts[] = {
    { "help"     , no_argument, NULL, 'h' },
    { "version"  , no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
    };
  int c;

  setuplocale();
  
  progname = strip_path(argv[0]);
  
  while ( (c = getopt_long (argc, argv, "Vh", long_opts, NULL)) != EOF) 
      switch (c) {
      case 'h':
	usage (progname);
	exit(0);
      case 'V':
	version (progname);
	exit(0);
      case '?':
	exit(1);
      }

    
    if ( (argc - optind -1)  % 2 != 1)
      badusage(_("even number of arguments expected"));
    if (-1 == (fd = get_console_fd(NULL)))
      exit (1);

  while ( (argc - optind)  > 2) 
    {
      a.scancode = sc = strtol(argv[optind++], &ep, 16);
      a.keycode = atoi(argv[optind++]);

      if (*ep)
	  badusage(_("error reading scancode"));
      if (a.scancode > 127) 
	{
	  a.scancode -= 0xe000;
	  a.scancode += 128;
	}
      if (a.scancode > 255 || a.keycode > 127)
	badusage(_("code outside bounds"));
      if (ioctl(fd,KDSETKEYCODE,&a)) 
	{
	  perror("KDSETKEYCODE");
	  fprintf(stderr, _("failed to set scancode %x to keycode %d\n"),
		  sc, a.keycode);
	  exit(1);
	}
    }
  exit(0);
}
