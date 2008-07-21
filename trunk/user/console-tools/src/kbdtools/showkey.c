#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sysexits.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

int fd;
int oldkbmode;
struct termios old;
char* progname;
int timeout = 10;

/*
 * version 0.81 of showkey would restore kbmode unconditially to XLATE,
 * thus making the console unusable when it was called under X.
 */
void get_mode(void)
{
  char *m;

  if (ioctl(fd, KDGKBMODE, &oldkbmode))
    {
      perror("KDGKBMODE");
      exit(1);
    }
  switch(oldkbmode)
    {
    case K_RAW:
      m = "RAW"; break;
    case K_XLATE:
      m = "XLATE"; break;
    case K_MEDIUMRAW:
      m = "MEDIUMRAW"; break;
    case K_UNICODE:
      m = "UNICODE"; break;
    default:
      m = _("?UNKNOWN?"); break;
    }
  printf(_("kb mode was %s\n"), m);
  if (oldkbmode != K_XLATE)
    {
      printf(_("[ if you are trying this under X, it might not work\n"));
      printf(_("since the X server is also reading /dev/console ]\n"));
    }
  printf("\n");
}

void clean_up(void)
{
  if (ioctl(fd, KDSKBMODE, oldkbmode))
    {
      perror("KDSKBMODE");
      exit(1);
    }
  if (tcsetattr(fd, 0, &old) == -1)
    perror("tcsetattr");
  close(fd);
}

void die(int x)
{
  printf(_("caught signal %d, cleaning up...\n"), x);
  clean_up();
  exit(1);
}

void watch_dog(int x)
{
  clean_up();
  exit(0);
}

void usage()
{
  printf(_("usage: %s [options...] <command>\n"), progname);
  COMMANDS_ARE();
  OPT("-s --scancodes  ", _("display only the raw scan-codes."));
  OPT("-k --keycodes   ", _("display only the interpreted keycodes (default)."));
  OPT("-m --keymap     ", _("display only keymap-translated chars."));
  OPT("-u --unicode    ", _("display unicode-translated chars."));
  OPT("-h --help       ", HELPDESC);
  OPT("-V --version    ", VERSIONDESC);
  OPTIONS_ARE();
  OPT("-t --timeout=N  ", _("set the timeout to N seconds."));
}

int
main (int argc, char *argv[])
{
  const char *short_opts = "hskmut:V";
  const struct option long_opts[] =
  {
    { "help",	no_argument, NULL, 'h' },
    { "scancodes",	no_argument, NULL, 's' },
    { "keycodes",	no_argument, NULL, 'k' },
    { "keymap",	no_argument, NULL, 'm' },
    { "timeout",	required_argument, NULL, 't' },
    { "unicode",   no_argument, NULL, 'u' },
    { "version",	no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };
  int c;
  enum {cmd_none, cmd_scancodes, cmd_keycodes, cmd_keymap, cmd_unicode} command = cmd_none;
  
  struct termios new;
  unsigned char buf[128];	       /* to decently grasp STRINGS with cmd_keymap */
  int i, n;

  setuplocale();
  
  progname = strip_path(argv[0]);
  
#define BAD_CMD() badusage(_("only one commands is allowed"));
  
  while ((c = getopt_long(argc, argv,
			  short_opts, long_opts, NULL)) != -1)
    {
      switch (c)
	{
	case 'm':
	  if (command == cmd_none)
	    command = cmd_keymap;
	  else
	    BAD_CMD();
	  break;
	case 's':
	  if (command == cmd_none)
	    command = cmd_scancodes;
	  else
	    BAD_CMD();
	  break;
	case 'k':
	  if (command == cmd_none)
	    command = cmd_keycodes;
	  else
	    BAD_CMD();
	  break;
	case 'u':
	  if (command == cmd_none)
	    command = cmd_unicode;
	  else
	    BAD_CMD();
	  break;
	case 't':
	  if (optarg && (optarg[0] >= '0') && (optarg[0] <= '9'))
	    timeout = atoi(optarg);
	  else
	    badusage(_("timeout value must be numeric"));
	  break;
	case 'V':
	  version();
	  exit(0);
	case 'h':
	case '?':
	  usage();
	  exit(0);
	}
    }

  if (command == cmd_none)
    command = cmd_keycodes;

  if (optind < argc)
    badusage(_("no non-option arguments allowed"));

  if (command == cmd_unicode)
    fprintf(stderr, _("WARNING: Unicode mode is experimental -- output may be wrong\n"));


  if (-1 == (fd = get_console_fd(NULL)))
    exit (1);

  /* the program terminates when there is no input for <timeout> secs */
  signal(SIGALRM, watch_dog);

  /*
    if we receive a signal, we want to exit nicely, in
    order not to leave the keyboard in an unusable mode
    */
  signal(SIGHUP, die);
  signal(SIGINT, die);
  signal(SIGQUIT, die);
  signal(SIGILL, die);
  signal(SIGTRAP, die);
  signal(SIGABRT, die);
  signal(SIGIOT, die);
  signal(SIGFPE, die);
  signal(SIGKILL, die);
  signal(SIGUSR1, die);
  signal(SIGSEGV, die);
  signal(SIGUSR2, die);
  signal(SIGPIPE, die);
  signal(SIGTERM, die);
#ifdef SIGSTKFLT
  signal(SIGSTKFLT, die);
#endif
  signal(SIGCHLD, die);
  signal(SIGCONT, die);
  signal(SIGSTOP, die);
  signal(SIGTSTP, die);
  signal(SIGTTIN, die);
  signal(SIGTTOU, die);

  get_mode();
  if (tcgetattr(fd, &old) == -1)
    perror("tcgetattr = %d\n");
  if (tcgetattr(fd, &new) == -1)
    perror("tcgetattr = %d\n");

  new.c_lflag &= ~ (ICANON | ECHO | ISIG);
  new.c_iflag = 0;
  new.c_cc[VMIN] = sizeof(buf);
  new.c_cc[VTIME] = 1;	/* 0.1 sec intercharacter timeout */

  if (tcsetattr(fd, TCSAFLUSH, &new) == -1)
    perror("tcsetattr = %d\n");
  if ((command != cmd_keymap) && ioctl(fd, KDSKBMODE,
				       (command == cmd_keycodes) ? K_MEDIUMRAW : 
				       (command == cmd_keymap ? K_XLATE : K_RAW)))
    {
      perror("KDSKBMODE");
      exit(1);
    }

  printf(_("press any key (program terminates after %us of last keypress)...\n"), timeout);
  while (1)
    {
      alarm(timeout);
      n = read(fd, buf, sizeof(buf));
	  
      if (command == cmd_keymap)
	{
	  for (i = 0; i < n; i++)
	    if (buf[i] >= 32)
	      putchar(buf[i]);
	    else
	      putchar(' ');
	  fprintf(stdout, " ( ");
	}
	      
      for (i = 0; i < n; i++) 
	{
	  if (command == cmd_keycodes)
	    printf(_("keycode %3d %s\n"),
		   buf[i] & 0x7f,
		   buf[i] & 0x80 ? _("release") : _("press"));
	  else /* scancode or keymap or unicode */
	    printf("0x%02x ", buf[i]);
	}

      switch (command)
	{
	case cmd_scancodes:
	case cmd_unicode:
	  putchar('\n');
	  break;
	case cmd_keymap:
	  printf(")\n");
	default:
	}
    }

  clean_up();
  exit(0);
}
