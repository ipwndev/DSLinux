/*
 * Mathomatic floating point constant factorizing routines.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

static void	try_factor();
static int	fc_recurse();

/* The following data is used to factor integers: */
static double nn, vv;
static double skip_multiples[] = {	/* Additive array that skips over multiples of 2, 3, 5, and 7. */
	10, 2, 4, 2, 4, 6, 2, 6,
	 4, 2, 4, 6, 6, 2, 6, 4,
	 2, 6, 4, 6, 8, 4, 2, 4,
	 2, 4, 8, 6, 4, 6, 2, 4,
	 6, 2, 6, 6, 4, 2, 4, 6,
	 2, 6, 4, 2, 4, 2,10, 2
};	/* sum of all numbers = 210 = (2*3*5*7) */

/*
 * Factor the integer in "start".
 * Store the prime factors in the unique[] array.
 *
 * Return true if successful.
 */
int
factor_one(start)
double	start;
{
	int	i;
	double	d;

	uno = 0;
	nn = start;
	if (nn == 0.0) {
		return false;
	}
	if (fabs(nn) >= MAX_K_INTEGER) {
		/* too large to factor */
		return false;
	}
	if (fmod(nn, 1.0) != 0.0) {
		/* not an integer */
		return false;
	}
	vv = 1.0 + sqrt(fabs(nn));
	try_factor(2.0);
	try_factor(3.0);
	try_factor(5.0);
	try_factor(7.0);
	d = 1.0;
	while (d <= vv) {
		for (i = 0; i < ARR_CNT(skip_multiples); i++) {
			d += skip_multiples[i];
			try_factor(d);
		}
	}
	if (nn != 1.0) {
		try_factor(nn);
	}
	if (start != multiply_out_unique()) {
		error("Internal error factoring integers.");
		return false;
	}
	return true;
}

/*
 * See if "arg" is one or more factors of "nn".
 * If so, save it and remove it from "nn".
 */
static void
try_factor(arg)
double	arg;
{
	while (fmod(nn, arg) == 0.0) {
		if (uno > 0 && unique[uno-1] == arg) {
			ucnt[uno-1]++;
		} else {
			unique[uno] = arg;
			ucnt[uno] = 1;
			uno++;
		}
		nn /= arg;
		vv = 1.0 + sqrt(fabs(nn));
		if (nn <= 1.0 && nn >= -1.0)
			break;
	}
}

/*
 * Convert unique[] back into an integer.
 * Return the double integer.
 */
double
multiply_out_unique()
{
	int	i, j;
	double	d;

	d = 1.0;
	for (i = 0; i < uno; i++) {
		for (j = 0; j < ucnt[i]; j++) {
			d *= unique[i];
		}
	}
	return d;
}

/*
 * Display the prime factors in the unique[] array.
 */
display_unique()
{
	int	i;

	fprintf(gfp, "%.0f = ", multiply_out_unique());
	for (i = 0; i < uno;) {
		fprintf(gfp, "%.0f", unique[i]);
		if (ucnt[i] > 1) {
			fprintf(gfp, "^%d", ucnt[i]);
		}
		i++;
		if (i < uno) {
			fprintf(gfp, " * ");
		}
	}
	fprintf(gfp, "\n");
}

/*
 * Factor integers in an equation side.
 *
 * Return true if equation side was modified.
 */
int
factor_int(equation, np)
token_type	*equation;
int		*np;
{
	int	i, j;
	int	xsize;
	int	level;
	int	modified = false;

	for (i = 0; i < *np; i += 2) {
		if (equation[i].kind == CONSTANT && factor_one(equation[i].token.constant) && uno > 0) {
			if (uno == 1 && ucnt[0] <= 1)
				continue;	/* prime number */
			level = equation[i].level;
			if (uno > 1 && *np > 1)
				level++;
			xsize = -2;
			for (j = 0; j < uno; j++) {
				if (ucnt[j] > 1)
					xsize += 4;
				else
					xsize += 2;
			}
			if (*np + xsize > n_tokens) {
				error_huge();
			}
			for (j = 0; j < uno; j++) {
				if (ucnt[j] > 1)
					xsize = 4;
				else
					xsize = 2;
				if (j == 0)
					xsize -= 2;
				if (xsize > 0) {
					blt(&equation[i+xsize], &equation[i], (*np - i) * sizeof(token_type));
					*np += xsize;
					if (j > 0) {
						i++;
						equation[i].kind = OPERATOR;
						equation[i].level = level;
						equation[i].token.operatr = TIMES;
						i++;
					}
				}
				equation[i].kind = CONSTANT;
				equation[i].level = level;
				equation[i].token.constant = unique[j];
				if (ucnt[j] > 1) {
					equation[i].level = level + 1;
					i++;
					equation[i].kind = OPERATOR;
					equation[i].level = level + 1;
					equation[i].token.operatr = POWER;
					i++;
					equation[i].level = level + 1;
					equation[i].kind = CONSTANT;
					equation[i].token.constant = ucnt[j];
				}
			}
			modified = true;
		}
	}
	return modified;
}

/*
 * Factor integers in an equation space.
 */
factor_int_sub(n)
int	n;	/* equation space number */
{
	if (n_lhs[n] <= 0)
		return;
	factor_int(lhs[n], &n_lhs[n]);
	factor_int(rhs[n], &n_rhs[n]);
}

/*
 * Neatly factor out coefficients in additive expressions in an equation side.
 * For example: (2*x + 4*y + 6) becomes 2*(x + 2*y + 3).
 *
 * This routine is often necessary because the expression compare (se_compare())
 * does not return a multiplier (except for +/-1.0).
 * This routine is not used during polynomial operations.
 * Normalization done here is required for simplification of algebraic fractions, etc.
 *
 * If "level_code" is 0, all additive expressions are normalized
 * by making at least one coefficient unity (1) by factoring out
 * the smallest constant.  The absolute value of all other coefficients will be >= 1.
 * If all coefficients are negative, -1 will be factored out, too.
 *
 * If "level_code" is 1, any level 1 additive expression is factored
 * nicely for readability, while all deeper levels are normalized.
 *
 * If "level_code" is 2, nothing is normalized unless it increases
 * readability.
 *
 * If "level_code" is 3, nothing is done.
 *
 * Return true if equation side was modified.
 */
int
factor_constants(equation, np, level_code)
token_type	*equation;	/* pointer to the beginning of equation side */
int		*np;		/* pointer to length of equation side */
int		level_code;	/* see above */
{
	if (level_code > 2)
		return false;
	return fc_recurse(equation, np, 0, 1, level_code);
}

static int
fc_recurse(equation, np, loc, level, level_code)
token_type	*equation;
int		*np, loc, level;
int		level_code;
{
	int	modified = false;
	int	i, j, k;
	int	op;
	int	neg_flag = true;
	double	d, minimum = 1.0;
	int	first = true;
	int	count = 0;

	for (i = loc; i < *np && equation[i].level >= level;) {
		if (equation[i].level > level) {
			modified |= fc_recurse(equation, np, i, level + 1, level_code);
			i++;
			for (; i < *np && equation[i].level > level; i += 2)
				;
			continue;
		}
		i++;
	}
	if (modified)
		return true;
	for (i = loc;;) {
break_cont:
		if (i >= *np || equation[i].level < level)
			break;
		if (equation[i].level == level) {
			switch (equation[i].kind) {
			case CONSTANT:
				if (i == loc && equation[i].token.constant >= 0.0)
					neg_flag = false;
				d = fabs(equation[i].token.constant);
				if (first) {
					minimum = d;
					first = false;
				} else if (minimum > d) {
					minimum = d;
				}
				break;
			case OPERATOR:
				count++;
				switch (equation[i].token.operatr) {
				case PLUS:
					neg_flag = false;
				case MINUS:
					break;
				default:
					return modified;
				}
				break;
			default:
				if (i == loc)
					neg_flag = false;
				if (first) {
					minimum = 1.0;
					first = false;
				} else if (minimum > 1.0) {
					minimum = 1.0;
				}
				break;
			}
		} else {
			op = 0;
			for (j = i + 1; j < *np && equation[j].level > level; j += 2) {
				if (equation[j].level == level + 1) {
					op = equation[j].token.operatr;
				}
			}
			if (op == TIMES || op == DIVIDE) {
				for (k = i; k < j; k++) {
					if (equation[k].level == (level + 1) && equation[k].kind == CONSTANT) {
						if (i == loc && equation[k].token.constant >= 0.0)
							neg_flag = false;
						d = fabs(equation[k].token.constant);
						if (first) {
							minimum = d;
							first = false;
						} else if (d < minimum) {
							minimum = d;
						}
						i = j;
						goto break_cont;
					}
				}
			}
			if (i == loc)
				neg_flag = false;
			if (first) {
				minimum = 1.0;
				first = false;
			} else if (1.0 < minimum)
				minimum = 1.0;
			i = j;
			continue;
		}
		i++;
	}
	if (first || count == 0 || (!neg_flag && minimum == 1.0))
		return modified;
	if (!isfinite(minimum))
		return modified;
	if (level_code > 1 || (level_code && (level == 1))) {
		for (i = loc;;) {
			d = 1.0;
			if (equation[i].kind == CONSTANT) {
				if (equation[i].level == level || ((i + 1) < *np
				    && equation[i].level == (level + 1)
				    && equation[i+1].level == (level + 1)
				    && (equation[i+1].token.operatr == TIMES
				    || equation[i+1].token.operatr == DIVIDE))) {
					d = equation[i].token.constant;
				}
			}
			if ((minimum < 1.0 && fmod(d, 1.0) == 0.0) || (fmod(d / minimum, 1.0) != 0.0)) {
				if (neg_flag) {
					minimum = 1.0;
					break;
				}
				return modified;
			}
			i++;
			for (; i < *np && equation[i].level > level; i += 2)
				;
			if (i >= *np || equation[i].level < level)
				break;
			i++;
		}
	}
	if (neg_flag)
		minimum = -minimum;
	if (*np + ((count + 2) * 2) > n_tokens) {
		error_huge();
	}
	for (i = loc; i < *np && equation[i].level >= level; i++) {
		if (equation[i].kind != OPERATOR) {
			for (j = i;;) {
				equation[j].level++;
				j++;
				if (j >= *np || equation[j].level <= level)
					break;
			}
			blt(&equation[j+2], &equation[j], (*np - j) * sizeof(token_type));
			*np += 2;
			equation[j].level = level + 1;
			equation[j].kind = OPERATOR;
			equation[j].token.operatr = DIVIDE;
			j++;
			equation[j].level = level + 1;
			equation[j].kind = CONSTANT;
			equation[j].token.constant = minimum;
			i = j;
		}
	}
	for (i = loc; i < *np && equation[i].level >= level; i++) {
		equation[i].level++;
	}
	blt(&equation[i+2], &equation[i], (*np - i) * sizeof(token_type));
	*np += 2;
	equation[i].level = level;
	equation[i].kind = OPERATOR;
	equation[i].token.operatr = TIMES;
	i++;
	equation[i].level = level;
	equation[i].kind = CONSTANT;
	equation[i].token.constant = minimum;
	return true;
}
