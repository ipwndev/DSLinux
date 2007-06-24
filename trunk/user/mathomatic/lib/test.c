/*
 * This file contains a test for the Mathomatic symbolic math library.
 * Refer to this, if you are going to use the Mathomatic code in other projects.
 *
 * To run this, compile and link with the Mathomatic symbolic math library
 * and create an executable.
 *
 * Mathomatic Copyright (C) 1987-2007 George Gesslein II.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mathomatic.h"

int
main(int argc, char **argv)
{
	char	*cp, *ocp;
	int	rv;
	char	buf[1000];

	printf("Mathomatic library test program.\n");
	if (!matho_init()) {
		fprintf(stderr, "Not enough memory.\n");
		exit(1);
	}
	/* Mathomatic is ready for use. */
	/* This is an input/output loop for testing. */
	for (;;) {
		printf("%d-> ", cur_equation + 1);
		if ((cp = fgets(buf, sizeof(buf), stdin)) == NULL)
			break;
		rv = matho_process(cp, &ocp);
		if (ocp) {
			printf("%s\n", ocp);
			if (rv) {
				free(ocp);
			}
		}
	}
	exit(0);
}
