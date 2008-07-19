//          _       _________ ______   ______   _        _______  _______  _
//         ( (    /|\__   __/(  ___ \ (  ___ \ ( \      (  ____ \(  ____ \( )
//  ______ |  \  ( |   ) (   | (   ) )| (   ) )| (      | (    \/| (    \/| |
// (  ___ \|   \ | |   | |   | (__/ / | (__/ / | |      | (__    | (_____ | |
// | (   \/| (\ \) |   | |   |  __ (  |  __ (  | |      |  __)   (_____  )| |
// | |     | | \   |   | |   | (  \ \ | (  \ \ | |      | (            ) |(_)
// | (___/\| )  \  |___) (___| )___) )| )___) )| (____/\| (____/\/\____) | _
// (______/|/    )_)\_______/|/ \___/ |/ \___/ (_______/(_______/\_______)(_)
//                                                              Version 2.0.0
//  
// Copyright (C), Daniel Aarno <macbishop@users.sf.net> - All rights reserved 
//         Licensed under the Academic Free License version 1.2 

// cNibbles is a curses based version of the old nibbles game (also known as 
// snake). Your object is to control the worm and help it eat apples 
// distributed on the playing area. 
//
// Use the arrow keys (h,j,k,l alternatively) to move the worm. The m and z 
// keys can be used for relative movement. The q key quits the game and the p 
// key pauses the game. 


#include <string.h>

#include "options.h"
#include "misc.h"

#define GROWS_WITH_TIME  1

Options g_opts;

static const char *g_args[] = { "--no-color", "-n", "",
     "--help", "-h", "",
     "--version", "-v", "",
     NULL, NULL, NULL
};

void
print_about ()
{
     fprintf (stderr, "\ncNibbles %s %s\n%s\n\n",
	      _TEXT_VERSION_,
	      ", Copyright (C) 2003, Daniel Aarno - All rights reserved",
	      "\tLicensed under the Academic Free License version 1.2");
}

void
print_help ()
{
     int i;
     const char *description = "";
     //     const char *extras = "";

     print_about ();

     fprintf (stderr, "%s %s %s %s\n\n",
	      "cNibbles is a curses based version of the old nibbles game",
	      "(also known as snake).",
	      "Your object is to control the worm and help it eat apples",
	      "distributed on the playing area.");

     fprintf (stderr, "%s %s %s\n",
	      "Use the arrow keys (i,j,k,l alternatively) to move tha worm.",
	      "The m and z keys can be used for relative movement.",
	      "The q key quits the game and the p key pauses the game.");
     //Print help for args

     fprintf (stderr, "\n\tUsage: cNibbles ");
     for (i = 0; g_args[i] != NULL; i += 3) {
	  fprintf (stderr, "[%s, %s", g_args[i], g_args[i + 1]);
	  if (strlen (g_args[i + 2]) > 0)
	       fprintf (stderr, " %s] ", g_args[i + 2]);
	  else
	       fprintf (stderr, "] ");
     }

     fprintf (stderr, "\n");

     for (i = 1; g_args[i] != NULL; i += 3) {
	  switch (g_args[i][1]) {


	  case 'n':
	       description =
		    "Don't use color, even if available. Normally color\n\t\
is autodetected and used if available. Specifly this flag if you don't\n\t\
which to use color.";
	       break;

	  case 'h':
	       description = "Print this help message and exit.";
	       break;

	  case 'v':
	       description = "Display version information and exit";
	       break;

	  default:
	       fprintf (stderr,
			"Something is seriously wrong in print_help()!\n");
	       exit (1);
	       break;
	  }

	  fprintf (stderr, "\n%s, %s %s\n", g_args[i], g_args[i - 1],
		   g_args[i + 1]);
	  fprintf (stderr, "\t%s\n", description);

     }
}

void
print_version (void)
{
     print_about ();
     fprintf (stderr, "Built on %s @ %s\n", __DATE__, __TIME__);
}

void
set_opt_defaults (void)
{
     g_opts.useColor = 1;
     g_opts.growsWithTime = GROWS_WITH_TIME;
     g_opts.speed = 2;
}

void
parse_options (int argc, const char *argv[])
{
     char currentArg;
     const char **args;
     long i;

     set_opt_defaults ();

     if (argc <= 1)
	  return;

     argc--;
     args = &argv[1];

     for (i = 0; i < argc; i++) {
	  if (args[i][0] != '-') {	//make sure it's an option
	       print_help ();
	       exit (0);
	  }

	  if (args[i][1] == '-') {	//long opt, convert to short
	       //convert long opt to short equivalent
	       currentArg = long_to_short_opt (args[i]);
	  } else {
	       if (strlen (args[i]) != 2) {	//long opt with only on -
		    print_help ();
		    exit (1);
	       }

	       currentArg = args[i][1];
	  }

	  switch (currentArg) {	//act on this arg
	  case 'n':		//No color
	       g_opts.useColor = 0;
	       break;

	  case 't':		//Inc. worm length with time
	       fprintf (stderr, "Warning the -t option is deprecated!\n");
	       g_opts.growsWithTime = 1;
	       break;

	  case 's':		//speed
	       fprintf (stderr, "Warning, the -s options is deprecated!\n");
	       i++;
	       errno = 0;
	       g_opts.speed = strtol (args[i], NULL, 10);	//get speed value
	       if (g_opts.speed > 4 && errno == 0)
		    errno = ERANGE;	//not in range
	       if (errno) {	//wrong value for speed
		    perror ("parse_options()");
		    print_help ();
		    exit (errno);
	       }
	       break;

	  case 'v':		//version
	       print_version ();
	       exit (0);
	       break;

	  case 'h':		//help
	  default:
	       print_help ();
	       exit (0);
	       break;
	  }
     }
}

char
long_to_short_opt (const char arg[])
{
     int i;

     for (i = 0; g_args[i] != NULL; i += 3) {
	  if (!strcmp (g_args[i], arg)) {	//match
	       return g_args[i + 1][1];	//return the short opt char
	  }
     }

     return 0;
}
