#include <math.h>

/*
 * Factorial function in C for double precision floating point.
 * Uses the lgamma(3) function, just like Mathomatic does.
 *
 * Return (arg!).  Sets "errno" on overflow.
 */
double
fact(double arg)
{
	int	sign;
	double	d;

	d = exp(lgamma_r(arg + 1.0, &sign));
	return(d * sign);
}
