/*
 * disalloc.c - aeb - 940501 - Disallocate virtual terminal(s)
 * Renamed deallocvt.
 */
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <getopt.h>
#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

char *progname;
int fd;
int exit_status = EX_OK;

static void usage(char *progname)
{
  printf(_("Usage: %s [N1 N2 ...]\n"
	   "Deallocate virtual terminal(s)\n"), progname);
  OPTIONS_ARE();
  OPT("-t --vterm=v      ", _("virtual terminal"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}


void dealloc_vt(char *vt)
{
  int num;

  num = atoi(vt);
  
  if (num == 0)
    {
      fprintf(stderr, _("%s: 0: illegal VT number\n"), progname);
      
      /* set unconditionnaly: worse problem */
      exit_status = EX_DATAERR;
    }
#if 0
  else if (num == 1)
    {
      fprintf(stderr,
	      _("%s: VT 1 is the console and cannot be deallocated\n"),
	      progname);
      
      /* do not override DATAERR */
      if (exit_status != EX_DATAERR)
	exit_status = EX_UNAVAILABLE;
    }
#endif
  else if (ioctl(fd,VT_DISALLOCATE,num)) 
    {
      perror("VT_DISALLOCATE");
      fprintf(stderr, _("%s: could not deallocate console %d\n"),
	      progname, num);
      
      /* do not return TEMPFAIL if something worse already occured */
      if (!exit_status)
	exit_status = EX_TEMPFAIL;
    }
}



int main(int argc, char *argv[]) 
{
  int c,  done = 0;
  const struct option long_opts[] =
  {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { "vterm", 1, NULL, 't' },
    { NULL, 0, NULL, 0 }
  };

  setuplocale();
  
  if (argc < 1) 		/* unlikely */
    exit(1);
  progname = strip_path(argv[0]);

  while ( (c = getopt_long (argc, argv, "Vh", long_opts, NULL)) != EOF) {
    switch (c)
      {
      case 'h':
	usage(progname);
	exit(0);
      case 'V':
	version(progname);
	exit(0);
      case 't':
	dealloc_vt (optarg);
	done = 1;
	break;
      case '?':
	usage(progname);
	exit(1);
      }
  }
      
  if (-1 == (fd = get_console_fd(NULL))) 
      exit (EX_OSERR);

  if (optind < argc)
    {
      /* More left to process */
      while (optind < argc) 
	dealloc_vt( argv [ optind++]);
    }
  else
    if (done == 0)
      {
	/* deallocate all unused consoles */
	if (ioctl(fd,VT_DISALLOCATE,0)) 
	  {
	    perror("VT_DISALLOCATE");
	    fprintf(stderr,
		    _("%s: deallocating all unused consoles failed\n"),
		    progname);
	    exit(EX_OSERR);
	  }
      }

  exit (exit_status);
}



