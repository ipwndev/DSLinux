/*
 * Small reimplementation of getopt().
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HAVE_GETOPT

char *minioptarg = NULL;
int minioptind = 0;
int miniopterr = 1;
int minioptopt = 0;


/*
 * Minimalist getopt() clone, which handles short options only and doesn't
 * permute argv[].
 */
int minigetopt(int argc, char **argv, char *optstring)
{
	static int nextchar = 0;
	int optchar;
	int i;

	if ((minioptind == 0) && (argc > 0))
		minioptind++;

	if ((nextchar > 0) && (argv[minioptind][nextchar] == 0)) {
		minioptind++;
		nextchar = 0;
	}

	if (minioptind >= argc)
		return -1;

	/*
	 * End of options if arg doesn't start with "-"
	 */
	if (argv[minioptind][0] != '-')
		return -1;

	/*
	 * End of options if arg is just "-"
	 */
	if (argv[minioptind][1] == 0)
		return -1;

	/*
	 * End of options if arg is "--", but don't include the "--" in the
	 * non-option arguments
	 */
	if ((argv[minioptind][1] == '-') && (argv[minioptind][2] == 0)) {
		minioptind++;
		return -1;
	}

	if (nextchar == 0)
		nextchar = 1;

	optchar = argv[minioptind][nextchar++];

	for (i = 0; optstring[i] != 0 && optstring[i] != optchar; i++) {
	}

	if (optstring[i] == 0) {
		minioptopt = optchar;
		if (miniopterr)
			fprintf(stderr, "%s: invalid option -- %c\n",
				argv[0], optchar);
		return '?';
	}

	if (optstring[i + 1] != ':') {
		minioptarg = NULL;
		return optchar;
	}

	/*
	 * At this point we've got an option that takes an argument.
	 */

	/*
	 * Next character isn't 0, so the argument is within this array
	 * element (i.e. "-dFOO").
	 */
	if (argv[minioptind][nextchar] != 0) {
		minioptarg = &(argv[minioptind][nextchar]);
		nextchar = 0;
		minioptind++;
		return optchar;
	}

	/*
	 * Argument is in the next array element (i.e. "-d FOO").
	 */
	nextchar = 0;
	minioptind++;
	if (minioptind >= argc) {
		fprintf(stderr, "%s: option `-%c' requires an argument\n",
			argv[0], optchar);
		return ':';
	}
	minioptarg = argv[minioptind++];

	return optchar;
}

#endif				/* HAVE_GETOPT */

/* EOF */
