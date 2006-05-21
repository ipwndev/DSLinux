#include <math.h>

/*
 * Factorial function in C for double precision floating point.
 * Uses the gamma function, just like Mathomatic does.
 *
 * Return (arg!).
 */
double
fact(arg)
double	arg;
{
        return(exp(lgamma(arg + 1.0)));
}
