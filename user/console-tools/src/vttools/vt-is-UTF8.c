/* (c) Ricardas Cepas <rch@pub.osf.lt>. Copying policy: GNU GPL V2. */
/* Modified by ydi for console-tools */
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <lct/console.h>
#include <lct/utils.h>
#include <lct/local.h>

extern int is_in_UTF8_mode (int);
char* progname;

void usage()
{
  printf (_("Usage: %s [option]\n"), progname);
  OPTIONS_ARE();
  OPT("-q  --quiet    ", _("don't print result"));

  OPT("-h --help      ", HELPDESC);
  OPT("-V --version   ", VERSIONDESC);
}

const struct option opts[] = 
{
  /* operations */
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"quiet", no_argument, 0, 'q'},
    {0, 0, 0, 0}
};

void main (int argc, char* argv[])
{
  int x;
  int result;			       /* option handling */
  int an_option;
  int quiet = 0;

  setuplocale();
  
  progname = strip_path(argv[0]);
  
  while (1)
    {
      result = getopt_long(argc, argv, "Vhq", opts, &an_option);

      if (result == EOF)
	  break;
      
      switch (result)
	{
	case 'V':
	  version();
	  exit (0);
	case 'h':
	  usage();
	  exit (0);
	  
	case 'q':
	  quiet = 1;
	}
    }
  
  if (optind < argc)
      badusage ("no non-option arguments are valid");

  x = is_in_UTF8_mode (get_console_fd(NULL));
  switch (x)
    {
    case 1:
      if (quiet)
	  exit (0);
      else
	  fprintf (stderr, "UTF-8 unicode mode.\n");
      break;
      
    case 0:
      if (quiet)
	  exit (1);
      else
	  fprintf (stderr, _("Single-byte char mode.\n"));
      break;
      
    case -1:
      perror("is_in_UTF8_mode");
      if (!quiet) exit (1);
      break;
    }
  
  exit (0);
}
