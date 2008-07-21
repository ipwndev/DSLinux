#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>

void version(char *progname)
{
  /* Same format as GNU utils: dump to stdout, not stderr */
  printf ("%s: (%s) %s\n", progname, PACKAGE, VERSION);
}

char* strip_path (const char* name)
{
  char* progname;
  
  if (NULL == (progname = strrchr(name, '/')))
    progname = (char*)name;
  else
    progname++;					  /* forget last '/' */

  return progname;
}

void simple_options(int argc, char **argv, UsageFunc usage, char *progname)
{
  /* Simple getopt handling for programs that don't have any options.
   * Depends on a locally-defined usage function in each program.
   */
     
  const struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };
  int c;

  while ( (c = getopt_long (argc, argv, "Vh", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case '?':
      usage(progname);
      exit(1);
    }
  }
  if (argc != optind) {
    fprintf(stderr, _("%s: Unexpected arguments.\n"), 
	    progname);
    exit(1);
  }       
}
      
  
