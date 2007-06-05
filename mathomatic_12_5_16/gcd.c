/*
 * General floating point GCD routines.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

#include "includes.h"

/*
 * Return the Greatest Common Divisor of doubles "d1" and "d2",
 * by using the Euclidean GCD algorithm.
 * Will work with non-integers, but there may be some floating point error.
 * Always works correctly with integers.
 *
 * Return 0.0 on failure, otherwise return the positive GCD.
 */
double
gcd(d1, d2)
double	d1, d2;
{
	int	count;
	double	larger;
	double	divisor;
	double	r1;
	double	d;

	d1 = fabs(d1);
	d2 = fabs(d2);
	if (d1 > d2) {
		larger = d1;
		divisor = d2;
	} else {
		larger = d2;
		divisor = d1;
	}
	if (larger >= MAX_K_INTEGER) {
		return 0.0;
	}
	d = larger * epsilon;
	if (d >= divisor)
		return 0.0;
	for (count = 1; count < 50; count++) {
		r1 = fmod(larger, divisor);
		if (r1 <= d || (divisor - r1) <= d) {
			if (r1 != 0.0 && divisor <= (100 * d))
				return 0.0;
			return divisor;
		}
		larger = divisor;
		divisor = r1;
	}
	return 0.0;
}

/*
 * Round a double to the nearest integer.
 */
double
my_round(d1)
double	d1;	/* value to round */
{
	if (d1 >= 0.0) {
		modf(d1 + 0.5, &d1);
	} else {
		modf(d1 - 0.5, &d1);
	}
	return d1;
}

/*
 * Convert the passed double "d" to a fully reduced fraction.
 * This done by the following simple algorithm:
 *
 * divisor = GCD(d, 1.0)
 * numerator = d / divisor
 * denominator = 1.0 / divisor
 *
 * Return true with integer numerator and denominator
 * if conversion was successful.
 * Otherwise return false with numerator = "d" and denominator = "1.0".
 *
 * True return indicates "d" is rational, otherwise "d" is irrational.
 */
int
f_to_fraction(d, d1p, d2p)
double	d;	/* floating point number to convert */
double	*d1p;	/* returned numerator */
double	*d2p;	/* returned denominator */
{
	double	divisor;
	double	d1, d2;
	double	k3, k4;

	*d1p = d;
	*d2p = 1.0;
	if (fmod(d, 1.0) == 0.0)
		return true;
	if ((divisor = gcd(1.0, d)) > epsilon) {
		d1 = d / divisor;
		d2 = 1.0 / divisor;
		d1 = my_round(d1);
		d2 = my_round(d2);
		if (fabs(d1) >= 1.0e12)
			return false;
		if (d2 >= 1.0e12)
			return false;
		if (d2 < 1.5)
			return false;
		divisor = gcd(d1, d2);	/* make sure the result is a fully reduced fraction */
		if (fmod(divisor, 1.0) != 0.0) {
			error("Error in gcd() function!");
			return false;
		}
		if (divisor > 1.0) {	/* shouldn't happen, but just in case */
			d1 = d1 / divisor;
			d2 = d2 / divisor;
		}
		k3 = (d1 / d2);
		k4 = d;
		if (fabs(k3 - k4) > (small_epsilon * fabs(k3))) {
			return false;	/* result is too inaccurate */
		}
		*d1p = d1;
		*d2p = d2;
		return true;
	}
	return false;
}

/*
 * Convert non-integer constants in an equation side to fractions, when appropriate.
 *
 * Return true if equation side was changed.
 */
int
make_fractions(equation, np)
token_type	*equation;	/* equation side pointer */
int		*np;		/* pointer to length of equation side */
{
	int	i, j, k;
	int	level;
	double	d1, d2;
	int	inc_level;
	int	modified = false;

	for (i = 0; i < *np; i += 2) {
		if (equation[i].kind == CONSTANT) {
			level = equation[i].level;
			if (i > 0 && equation[i-1].level == level && equation[i-1].token.operatr == DIVIDE)
				continue;
			if (!f_to_fraction(equation[i].token.constant, &d1, &d2) || d2 == 1.0)
				continue;
			if ((*np + 2) > n_tokens) {
				error_huge();
			}
			modified = true;
			inc_level = (*np > 1);
			if ((i + 1) < *np && equation[i+1].level == level) {
				switch (equation[i+1].token.operatr) {
				case TIMES:
					for (j = i + 3; j < *np && equation[j].level >= level; j += 2) {
						if (equation[j].level == level && equation[j].token.operatr == DIVIDE) {
							break;
						}
					}
					if (d1 == 1.0) {
						blt(&equation[i], &equation[i+2], (j - (i + 2)) * sizeof(*equation));
						j -= 2;
					} else {
						equation[i].token.constant = d1;
						blt(&equation[j+2], &equation[j], (*np - j) * sizeof(*equation));
						*np += 2;
					}
					equation[j].level = level;
					equation[j].kind = OPERATOR;
					equation[j].token.operatr = DIVIDE;
					j++;
					equation[j].level = level;
					equation[j].kind = CONSTANT;
					equation[j].token.constant = d2;
					if (d1 == 1.0) {
						i -= 2;
					}
					continue;
				case DIVIDE:
					inc_level = false;
					break;
				}
			}
			j = i;
			blt(&equation[i+3], &equation[i+1], (*np - (i + 1)) * sizeof(*equation));
			*np += 2;
			equation[j].token.constant = d1;
			j++;
			equation[j].level = level;
			equation[j].kind = OPERATOR;
			equation[j].token.operatr = DIVIDE;
			j++;
			equation[j].level = level;
			equation[j].kind = CONSTANT;
			equation[j].token.constant = d2;
			if (inc_level) {
				for (k = i; k <= j; k++)
					equation[k].level++;
			}
		}
	}
	return modified;
}
