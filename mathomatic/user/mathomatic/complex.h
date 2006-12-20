/*
 * Include file for complex number arithmetic in C.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

typedef struct complexs {	/* complex number structure */
	double	re;		/* real part */
	double	im;		/* imaginary part */
} complexs;

/* complex arithmetic functions */
int complex_fixup(complexs *ap);
complexs complex_add(complexs a, complexs b);
complexs complex_negate(complexs a);
complexs complex_mult(complexs a, complexs b);
complexs complex_div(complexs a, complexs b);
complexs complex_log(complexs a);
complexs complex_exp(complexs a);
complexs complex_pow(complexs a, complexs b);
