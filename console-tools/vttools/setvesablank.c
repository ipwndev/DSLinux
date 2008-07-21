/*
 * setvesablank.c - aeb - 941230
 *
 * usage: setvesablank ON|on|off
 */
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include <getopt.h>
#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

char* progname;

static void usage(char *progname)
{
  printf(_("Usage: %s ON|on|off\n\n"
	   "Set VESA blanking on console.\n"), progname);
  OPTIONS_ARE();
  OPT("-b --blanking=on|off  ", _("turn blanking on"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

int blanking (char *s)
{
  if (!strcmp(s, "on"))
      return 1;
  else if (!strcmp(s, "ON"))
      return 2;
  else if (!strcmp(s, "off"))
     return 0;
  else {
    fprintf(stderr, _("%s: argument '%s' should be 'ON', 'on' or 'off'\n"),
	    progname, s);
    exit(1);
  }
}
  
void main(int argc, char *argv[]) 
{
  int fd, c;
  struct { char ten, onoff; } arg;
  const struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { "blanking", 1, NULL, 'b' },
    { NULL, 0, NULL, 0 }
  };

  setuplocale();
  
  progname = strip_path(argv[0]);
  arg.onoff = -1;

  while ( (c = getopt_long (argc, argv, "Vhb:", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case 'b':
      arg.onoff = blanking (optarg);
      break;
    case '?':
      usage(progname);
      exit(1);
    }
  }


  if (arg.onoff == -1 && argc == optind+1)     
    arg.onoff = blanking (argv[optind++]);
   
  if (arg.onoff == -1 || argc != optind) {
    fprintf (stderr, _("%s: Wrong number of args\n"),
	     progname);
    exit(1);
  }
		            
  if (-1 == (fd = get_console_fd(NULL))) exit (1);
  
  arg.ten = 10;
  if (ioctl(fd, TIOCLINUX, &arg)) 
    {
      fprintf(stderr, "%s: ", progname);
      perror("TIOCLINUX");
      exit(1);
    }
  exit(0);
}
