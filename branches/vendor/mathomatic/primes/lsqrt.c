/*
 * Long integer square root function.
 *
 * Copyright (C) 2007 George Gesslein II.
 */

#include <stdio.h>

/*
 * Return the truncated integer square root of "y" using longs.
 * Return -1 on error.
 */
long
lsqrt(long y)
{
	long	x_old, x_new;
	long	testy;
	int	nbits;
	int	i;

	if (y <= 0) {
		if (y != 0) {
			fprintf(stderr, "Domain error in lsqrt().\n");
			return -1L;
		}
		return 0L;
	}
/* select a good starting value using binary logarithms: */
	nbits = (sizeof(y) * 8) - 1;	/* subtract 1 for sign bit */
	for (i = 4, testy = 16L;; i += 2, testy <<= 2L) {
		if (i >= nbits || y <= testy) {
			x_old = (1L << (i / 2L));	/* x_old = sqrt(testy) */
			break;
		}
	}
/* x_old >= sqrt(y) */
/* use the Babylonian method to arrive at the integer square root: */
	for (;;) {
		x_new = (y / x_old + x_old) / 2L;
		if (x_old <= x_new)
			break;
		x_old = x_new;
	}
/* optional test code to make sure that the answer is right: */
	if ((long long) x_old * x_old > y || ((long long) x_old + 1) * ((long long) x_old + 1) <= y) {
		fprintf(stderr, "Error in lsqrt().\n");
		return -1L;
	}
	return x_old;
}
