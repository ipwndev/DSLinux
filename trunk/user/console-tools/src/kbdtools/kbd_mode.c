/*
 * kbd_mode: report and set keyboard mode - aeb, 940406
 * 
 * If you make \215A\201 an alias for "kbd_mode -a", and you are
 * in raw mode, then hitting F7 = (two keys) will return you to sanity.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/kd.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

char* progname;

static void usage(char *progname)
{
  printf(_("Usage: %s [option]\n"
	   "Report and set keyboard mode\n") ,progname);
  OPTIONS_ARE();
  OPT("-h --help     ", _("print this help information and exit"));
  OPT("-V --version  ", _("report version and exit"));
  OPT("-a --8bit     ", _("ASCII or 8bit mode (XLATE)"));	  
  OPT("-k --keycode  ", _("keycode mode (MEDIUMRAW)"));
  OPT("-u --unicode  ", _("UTF-8 mode (UNICODE)"));
  OPT("-s --scancode ", _("scancode mode (RAW)"));
  OPT("   --mode={8bit,keycode,unicode,scancode} ", _("set mode"));

  OPT("-h --help     ", HELPDESC);
  OPT("-V --version  ", VERSIONDESC);
}

static int parse_cmdline (int argc, char *argv[])
{
    char *progname = strip_path (argv[0]);
    int mode = -1;
    const struct option long_opts[] = {
      { "help"     , no_argument, NULL, 'h' },
      { "version"  , no_argument, NULL, 'V' },
      { "8bit"    , no_argument, NULL, 'a' },
      { "ascii"   , no_argument, NULL, 'a' },
      { "keycode"  , no_argument, NULL, 'k' },
      { "mode"     , required_argument, NULL, 'm' },
      { "scancode" , no_argument, NULL, 's' },
      { "unicode"  , no_argument, NULL, 'u' },
      { NULL, 0, NULL, 0 }
    };
    int c;

    while ( (c = getopt_long (argc, argv, "Vhaksu", long_opts, NULL)) != EOF) 
      switch (c) {
      case 'h':
	usage (progname);
	exit(0);
      case 'V':
	version (progname);
	exit(0);
      case 'a':
	mode = K_XLATE;
	break;
      case 'u':
	mode = K_UNICODE;
	break;
      case 's':
	mode = K_RAW;
	break;
      case 'k':
	mode = K_MEDIUMRAW;
	break;
      case 'm':
	if (!strcmp(optarg,"8bit") || !strcmp(optarg,"ascii"))	  
	  mode = K_XLATE;
	else if (!strcmp (optarg, "unicode"))
	  mode = K_UNICODE;
	else if (!strcmp (optarg, "scancode"))
	  mode = K_RAW;
	else if (!strcmp (optarg, "keycode"))
	  mode = K_MEDIUMRAW;
	else {
	  fprintf (stderr,_("%s: unknown mode: %s\n"),
		   progname, optarg);
	  exit(1);
	}
      }
    return mode;
}
	

int main(int argc, char *argv[])
{
  int fd, mode;

  setuplocale();
  
  mode = parse_cmdline (argc, argv);
  
  if (-1 == (fd = get_console_fd(NULL)))
      exit (1);
  
  if (mode == -1)
    {
      /* report mode */
      if (ioctl(fd, KDGKBMODE, &mode)) 
	{
	  fprintf(stderr, progname);
	  perror(_(": error reading keyboard mode\n"));
	  exit(1);
	}
      switch(mode) 
	{
	case K_RAW:
	  printf(_("The keyboard is in raw (scancode) mode\n"));
	  break;
	case K_MEDIUMRAW:
	  printf(_("The keyboard is in mediumraw (keycode) mode\n"));
	  break;
	case K_XLATE:
	  printf(_("The keyboard is in the default (ASCII) mode\n"));
	  break;
	case K_UNICODE:
	  printf(_("The keyboard is in Unicode (UTF-8) mode\n"));
	  break;
	default:
	  printf(_("The keyboard is in some unknown mode\n"));
	}
      exit(0);
    }
  
  if (ioctl(fd, KDSKBMODE, mode))
    {
      fprintf(stderr, progname);
      perror(_(": error setting keyboard mode\n"));
      exit(1);
    }
  exit(0);
}
