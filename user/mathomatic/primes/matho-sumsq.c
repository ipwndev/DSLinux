/*
 * Find and display the minimum sum of the squares for integers.
 *
 * Copyright (C) 2007 George Gesslein II.
 */

/*
 * Usage: matho-sumsq [numbers]
 *
 * This program finds the minimum number of positive integers squared
 * and summed to equal a given positive integer.  If the number of squared
 * integers is 2, all combinations with 2 squares are displayed,
 * otherwise only the first solution found is displayed.
 *
 * If nothing is specified on the command line, the program starts at 0 and
 * counts up.
 *
 * This file is mostly useful for testing the long integer square root function lsqrt().
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#define	true	1
#define	false	0

long	squares[4];

long	lsqrt(long y);

/*
 * Find the sum of "n" squares that equal "d1" and display if found.
 *
 * Return false if not found.
 */
int
sumsq(long d1, int n)
{
	int	i = 0;
	long	d2;
	long	save = 0;
	int	found_one = false;

	d2 = d1;
try_again:
	for (;;) {
		if (i == 2) {
			save = d2;
		}
		squares[i] = lsqrt(d2);
		if (squares[i] < 0) {
			exit(1);
		}
		d2 -= (squares[i] * squares[i]);
		i++;
		if (i >= n) {
			break;
		}
	}
	if (d2 == 0) {
		for (i = 0; i < n; i++) {
			d2 += (squares[i] * squares[i]);
		}
		if (d2 != d1) {
			fprintf(stderr, "Result doesn't compare identical to original number!\n");
			exit(1);
		}
		for (i = 0; i < (n - 1); i++) {
			if (squares[i] < squares[i+1]) {
				goto skip_print;
			}
		}
		found_one = true;
		printf("%ld = %ld^2", d1, squares[0]);
		for (i = 1; i < n; i++) {
			if (squares[i] != 0)
				printf(" + %ld^2", squares[i]);
		}
		printf("\n");
skip_print:
		if (!found_one) {
			fprintf(stderr, "Found bug!\n");
			exit(1);
		}
		if (n != 2) {
			return found_one;
		}
	}
	switch (n) {
	case 4:
		if (squares[2] > squares[n-1]) {
			squares[2] -= 1;
			d2 = save - (squares[2] * squares[2]);
			i = 3;
			goto try_again;
		}
	case 3:
		if (squares[1] > squares[n-1]) {
			squares[1] -= 1;
			d2 = d1 - (squares[0] * squares[0]) - (squares[1] * squares[1]);
			i = 2;
			goto try_again;
		}
	case 2:
		if (squares[0] > squares[n-1]) {
			squares[0] -= 1;
			d2 = d1 - (squares[0] * squares[0]);
			i = 1;
			goto try_again;
		}
	}
	return found_one;
}

/*
 * Display the shortest sum of the squares for long integer "d1".
 *
 * Return number of squares.
 */
int
findsq(long d1)
{
	if (sumsq(d1, 1))
		return 1;
	if (sumsq(d1, 2))
		return 2;
	if (sumsq(d1, 3))
		return 3;
	if (!sumsq(d1, 4)) {
		fprintf(stderr, "Whoops!  Can't find the sum of four squares that equal %ld.\n", d1);
		exit(1);
	}
	return 4;
}

int
main(int argc, char *argv[])
{
	int	i;
	long	d1 = 0;
	char	*cp;

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			errno = 0;
			d1 = strtol(argv[i], &cp, 10);
			if (errno || d1 <= 0) {
				fprintf(stderr, "Invalid number.\n");
				exit(1);
			}
			findsq(d1);
		}
	} else {
		for (;; d1 += 1) {
			findsq(d1);
		}
	}
	exit(0);
}
