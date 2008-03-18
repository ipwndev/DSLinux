/* esdplay.c - part of esdplay
 * Copyright (C) 1998 Simon Kågedal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License in the file COPYING for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <audiofile.h>
#include <esd.h>
#include "config.h"

static char *program_name = NULL;

static int
play_file (const char *filename)
{
    esd_play_file( program_name, filename, 1 );
	return 0;
}

static void
usage_exit (int ret_code)
{
  printf ("Usage: %s [OPTION] FILE\n"
	  "Plays the FILE on EsounD.\n\n"
	  "  -s, --server=HOSTNAME\tset EsounD server\n"
	  "  -h, --help\tdisplay this help and exit\n"
	  "  -v, --version\toutput version information and exit\n",
	  program_name);
  if(ret_code)
	  exit(ret_code);
  else 
  	exit (0);
}

int 
main (int argc, char *argv[])
{
  int option_index = 0, c = 0;

  struct option opts[] = {
    { "server", required_argument, NULL, 's' },
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };

  program_name = argv [0];

  /* parse options
   */

  while (1)
    {
      c = getopt_long(argc, argv, "s:hv", opts, &option_index);

      if (c == -1)
	break;

      switch (c)
	{
	case 's':
#ifdef HAVE_SETENV
	  setenv("ESPEAKER", optarg, 1);
#else
#ifdef HAVE_PUTENV
	  {
	    /* The following malloc is correct, and does take into
               account the trailing \0 too.  */
	    char *espeaker_env = malloc (strlen (optarg) + sizeof "ESPEAKER=");
	    if (espeaker_env)
	      {
		strcpy (espeaker_env, "ESPEAKER=");
		strcat (espeaker_env, optarg);
		putenv (espeaker_env);
	      }
	  }
#else
#error "How am I supposed to set an environment variable?"
#endif
#endif
	  break;

	case 'h':
	  usage_exit (0);
	  
	case 'v':
	  /* fputs ("esdplay " VERSION "\n", stdout); */
	  fputs ("esdplay\n", stdout);
	  exit (0);

	case '?':
	  /* `getopt_long' already printed an error message. */
	  fprintf(stderr,"Try `%s --help' for more information.\n", 
		  program_name);
	  exit (1);

	default:
	  abort();
	}
    }

  if (optind != argc-1)
    usage_exit (1);

  return play_file (argv[optind]);
}

