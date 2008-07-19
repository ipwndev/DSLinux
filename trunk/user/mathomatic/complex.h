/*
 * Include file for floating point complex number arithmetic in C.
 * Goes with "complex_lib.c".
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

typedef struct complexs {	/* complex number structure */
	double	re;		/* real part */
	double	im;		/* imaginary part */
} complexs;

/*
 * Complex arithmetic function prototypes
 */
int complex_fixup(complexs *ap);
complexs complex_add(complexs a, complexs b);
complexs complex_negate(complexs a);
complexs complex_mult(complexs a, complexs b);
complexs complex_div(complexs a, complexs b);
complexs complex_log(complexs a);
complexs complex_exp(complexs a);
complexs complex_pow(complexs a, complexs b);
