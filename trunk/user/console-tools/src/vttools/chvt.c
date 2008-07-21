/*
 * chvt.c - aeb - 940227 - Change virtual terminal
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

static void usage(char *progname)
{
  printf(_("Usage: %s [vt_number]\n"
	   "Change virtual terminal\n"), progname);
  OPTIONS_ARE();
  OPT("-t --vterm=v      ", _("virtual terminal"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

int parse_args (int argc,char **argv) 
{
  char * progname = strip_path(argv[0]);
  const struct option long_opts[] = {
    {"vterm", 1, NULL, 't' },
    {"help"  , no_argument, NULL, 'h' },
    {"version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0}
  };
  int c, cons = -1;
  while ( (c = getopt_long (argc, argv, "Vht:", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case 't':			/* screen */
      cons = atoi(optarg);
      break;
    case '?':
    default:
      usage(progname);
      exit(1);            
    }
  }
  if (cons == -1)
    {
      if (argc == optind+1)
	cons = atoi(argv[optind]);
      else {
	fprintf(stderr,_("%s: Wrong number of args\n"), progname);
	exit(1);
      }
    }
  
  return cons;
}

int main(int argc, char *argv[]) 
{
  int fd, num;

  setuplocale();
  
  num = parse_args (argc, argv);

  if (-1 == (fd = get_console_fd(NULL))) exit (1);
  
  if (ioctl(fd,VT_ACTIVATE,num)) 
    {
      perror("chvt: VT_ACTIVATE");
      exit(1);
    }
  if (ioctl(fd,VT_WAITACTIVE,num)) 
    {
      perror("VT_WAITACTIVE");
      exit(1);
    }
  exit(0);
}
