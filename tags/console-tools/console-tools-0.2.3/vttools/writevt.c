/*
 * sti.c: put text in some tty input buffer - aeb, 951009
 *
 * You may have to be root if the tty is not your controlling tty.
 */
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>

char *progname;

static void usage(char *progname)
{
  printf(_("Usage: %s tty text\n"
	   "Put text into the input buffer of a virtual terminal.\n"), progname);
  OPTIONS_ARE();
  OPT("-t --term=tty     ", _("device name"));
  OPT("-T --text=text    ", _("text to insert"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}


int main(int argc, char **argv) 
{
  int fd, c;
  char *vterm = NULL;
  char *text = NULL;
  const struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "term", 1, NULL, 't' },
    { "text", 1, NULL, 'T' },
    { "version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };

  setuplocale();
  
  progname = strip_path(argv[0]);

  while ( (c = getopt_long (argc, argv, "t:T:Vh", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case 't':
      vterm = optarg;
      break;
    case '?':
      usage(progname);
      exit(1);
    }
  }

  if (vterm == NULL) {
    if (optind < argc) 
      vterm = argv[optind++];
    else {
      fprintf(stderr,_("%s: No tty specified.\n"),progname);
      exit(1);
    }
  }
  
  if (text == NULL) {
    if (optind < argc)
      text = argv[optind++];
    else {
      fprintf(stderr,_("%s: No text specified.\n"),progname);
      exit(1);
    }
  }
  
  if (argc != optind ) {
    fprintf(stderr, _("%s: too many arguments\n"), progname);
    exit(1);
  }

  fd = open(vterm, O_RDONLY);
  if(fd < 0) 
    {
      perror(vterm);
      fprintf(stderr, _("%s: could not open tty\n"), progname);
      exit(1);
    }
  while(*text) 
    {
      if(ioctl(fd, TIOCSTI, text)) 
	{
	  perror("TIOCSTI");
	  fprintf(stderr, _("%s: TIOCSTI ioctl failed\n"), progname);
	  exit(1);
        }
	text++;
    }
  return 0;
}
