/*
 * Mathomatic simplifying routines.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

#define	MAX_COMPARE_TERMS	30	/* Max no. of terms on same level that can be compared, increases stack usage. */
					/* MAX_COMPARE_TERMS should be approximately (sqrt(DEFAULT_N_TOKENS) * 3) */

static int	org_recurse();
static int	const_recurse();
static int	compare_recurse();
static int	order_recurse();

/*
 * Quickly and very basically simplify an equation space.
 * No factoring is done.
 */
simp_sub(n)
int	n;	/* equation space number to simplify */
{
	if (n_lhs[n] <= 0)
		return;
	simp_loop(lhs[n], &n_lhs[n]);
	if (n_rhs[n]) {
		simp_loop(rhs[n], &n_rhs[n]);
	}
}

/*
 * For quick, mid-range simplification of an expression.
 */
simp_side(equation, np)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of equation side */
{
	simp_ssub(equation, np, 0L, 0.0, true, true, 1);
}

/*
 * This function is the mid-range simplifier used by the solver.
 */
simps_side(equation, np, zsolve)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of equation side */
int		zsolve;		/* true if we are solving for zero */
{
	do {
		simp_ssub(equation, np, 0L, 0.0, !zsolve, true, 2);
	} while (super_factor(equation, np, 0));
}

/*
 * This function is used by the "factor" command.
 */
simpv_side(equation, np, v)
token_type	*equation;	/* pointer to the beginning of equation side to factor */
int		*np;		/* pointer to length of equation side */
long		v;		/* variable to factor */
{
	if (*np == 0)
		return;
	simp_ssub(equation, np, v, 0.0, v == 0, true, 2);
}

/*
 * Convert expression with any algebraic fractions into a single fraction.
 */
frac_side(equation, np)
token_type	*equation;	/* pointer to the beginning of equation side */
int		*np;		/* pointer to length of equation side */
{
	if (*np == 0)
		return;
	do {
		simpb_side(equation, np, false, 2);
	} while (super_factor(equation, np, 2));
}

/*
 * Simplify and approximate for the calculate command.
 * Includes complex number simplification.
 */
calc_simp(equation, np)
token_type	*equation;
int		*np;
{
	approximate_roots = true;
	subst_constants(equation, np);
	simp_side(equation, np);
	factorv(equation, np, IMAGINARY);
	ufactor(equation, np);
	factorv(equation, np, IMAGINARY);
	ufactor(equation, np);
	factorv(equation, np, IMAGINARY);
	simp_side(equation, np);
	uf_simp(equation, np);
	approximate_roots = false;
}

/*
 * Try to eliminate "i#" from an equation side by converting "i#*(b^.5)" to "(-1*b)^.5".
 */
simp_i(equation, np)
token_type	*equation;
int		*np;
{
	int	i;
	int	level;

	simp_loop(equation, np);
	for (i = 0; i < *np; i += 2) {
		if (equation[i].kind == VARIABLE && equation[i].token.variable == IMAGINARY) {
			if (*np + 2 > n_tokens) {
				error_huge();
			}
			level = equation[i].level + 1;
			blt(&equation[i+2], &equation[i], (*np - i) * sizeof(token_type));
			*np += 2;
			equation[i].level = level;
			equation[i].kind = CONSTANT;
			equation[i].token.constant = -1.0;
			i++;
			equation[i].level = level;
			equation[i].kind = OPERATOR;
			equation[i].token.operatr = POWER;
			i++;
			equation[i].level = level;
			equation[i].kind = CONSTANT;
			equation[i].token.constant = 0.5;
		}
	}
	do {
		do {
			do {
				do {
					organize(equation, np);
					combine_constants(equation, np, false);
				} while (elim_k(equation, np));
			} while (simp_pp(equation, np));
		} while (factor_power(equation, np));
	} while (factor_times(equation, np));
	simp_loop(equation, np);
}

/*
 * Main simplify code used by the simplify command.
 * "tlhs" and "trhs" are wiped out.
 * This is the slowest and most thorough simplify of all.
 */
simpa_side(equation, np, quick_flag)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of the equation side */
int		quick_flag;	/* "simplify quick" option */
{
	int		flag, poly_flag = true;
	jmp_buf		save_save;

	if (*np <= 1)	/* no need to simplify a constant or a variable */
		return;
	partial_flag = 2;
	simp_loop(equation, np);
	poly_factor(equation, np);
	simpb_side(equation, np, true, 1);
	simp_ssub(equation, np, 0L, 0.0, true, true, 1);
	rationalize(equation, np);
	unsimp_power(equation, np);
	uf_times(equation, np);
	simp_loop(equation, np);
	uf_repeat(equation, np);
	do {
		elim_loop(equation, np);
	} while (mod_simp(equation, np));
	/* Here we try to simplify out unnecessary negative constants and imaginary numbers */
	simp_i(equation, np);
	unsimp_power(equation, np);
	uf_times(equation, np);
	simp_ssub(equation, np, 0L, 1.0, true, true, 1);
	unsimp_power(equation, np);
	uf_neg_help(equation, np);
	uf_times(equation, np);
	do {
		do {
			do {
				simp_ssub(equation, np, 0L, 1.0, false, true, 1);
			} while (poly_gcd_simp(equation, np));
		} while (uf_power(equation, np));
	} while (super_factor(equation, np, 2));
	unsimp_power(equation, np);
	uf_times(equation, np);
	factorv(equation, np, IMAGINARY);
	uf_pplus(equation, np);
	simp_ssub(equation, np, 0L, 1.0, true, false, 1);
	uf_times(equation, np);
	uf_pplus(equation, np);
	factorv(equation, np, IMAGINARY);
	uf_power(equation, np);
	do {
		do {
			simp_ssub(equation, np, 0L, 1.0, false, true, 1);
		} while (uf_power(equation, np));
	} while (super_factor(equation, np, 2));
	if (quick_flag) {
do_quick:
		uf_tsimp(equation, np);
	} else {
		n_tlhs = *np;
		blt(tlhs, equation, n_tlhs * sizeof(token_type));
		blt(save_save, jmp_save, sizeof(jmp_save));
		if (setjmp(jmp_save) != 0) {
			blt(jmp_save, save_save, sizeof(jmp_save));
			/* restore the original expression */
			*np = n_tlhs;
			blt(equation, tlhs, n_tlhs * sizeof(token_type));
			goto do_quick;
		}
		/* expand powers of 2 and higher, might result in error_huge() trap */
		do {
			uf_repeat(equation, np);
		} while (uf_tsimp(equation, np));
		blt(jmp_save, save_save, sizeof(jmp_save));
	}
	partial_flag = false;
	simpb_side(equation, np, true, 1);
	debug_string(1, "Simplify result before applying polynomial operations:");
	side_debug(1, equation, *np);
	for (flag = false;;) {
		/* divide top and bottom of fractions by any polynomial GCD found */
		if (poly_gcd_simp(equation, np)) {
			flag = false;
			simpb_side(equation, np, true, 1);
		}
		if (!flag) {
			/* factor polynomials */
			if (poly_factor(equation, np)) {
				flag = true;
				simpb_side(equation, np, true, 1);
				continue;
			}
		}
		/* simplify algebraic fractions with polynomial and smart division */
		if (div_remainder(equation, np, poly_flag, quick_flag)) {
			flag = false;
			simpb_side(equation, np, true, 1);
			continue;
		}
		break;
	}
	partial_flag = true;
	simp_constant_power(equation, np);
	simp_ssub(equation, np, 0L, 1.0, true, true, 1);
	unsimp_power(equation, np);
	make_fractions(equation, np);
	factor_power(equation, np);
	uf_tsimp(equation, np);
	make_fractions(equation, np);
	uf_power(equation, np);
	integer_root_simp(equation, np);
	simpb_side(equation, np, true, 3);
	while (div_remainder(equation, np, poly_flag, quick_flag)) {
		simpb_side(equation, np, true, 3);
	}
	poly_factor(equation, np);
	simpb_side(equation, np, true, 2);
	debug_string(1, "Result of complete simplify:");
	side_debug(1, equation, *np);
}

/*
 * Compare function for qsort(3) within simpb_side().
 */
static int
vcmp(p1, p2)
sort_type	*p1, *p2;
{
	if (((p1->v & VAR_MASK) == SIGN) == ((p2->v & VAR_MASK) == SIGN)) {
		if (p2->count == p1->count) {
			if (p1->v < p2->v)
				return -1;
			if (p1->v == p2->v)
				return 0;
			return 1;
		}
		return(p2->count - p1->count);
	} else {
		if ((p1->v & VAR_MASK) == SIGN) {
			return -1;
		} else {
			return 1;
		}
	}
}

/*
 * Beauty simplifier.
 * Neat simplify and factor routine.
 * Factors variables in order: "sign" variables first, then by frequency.
 */
simpb_side(equation, np, power_flag, fc_level)
token_type	*equation;	/* pointer to the beginning of equation side */
int		*np;		/* pointer to length of equation side */
int		power_flag;	/* factor_power() flag */
int		fc_level;	/* factor constants code, passed to factor_constants() */
{
	int		i;
	int		vc, cnt;
	long		v1, last_v;
	sort_type	va[MAX_VARS];

	simp_loop(equation, np);
	uf_allpower(equation, np);
	last_v = 0;
	for (vc = 0; vc < ARR_CNT(va); vc++) {
		for (i = 0, cnt = 0, v1 = -1; i < *np; i += 2) {
			if (equation[i].kind == VARIABLE && equation[i].token.variable > last_v) {
				if (v1 == -1 || equation[i].token.variable < v1) {
					v1 = equation[i].token.variable;
					cnt = 1;
				} else if (equation[i].token.variable == v1) {
					cnt++;
				}
			}
		}
		if (v1 == -1)
			break;
		last_v = v1;
		va[vc].v = v1;
		va[vc].count = cnt;
	}
	if (vc) {
		qsort((char *) va, vc, sizeof(*va), vcmp);
		simp2_divide(equation, np, va[0].v, fc_level);
		for (i = 1; i < vc; i++) {
			if (factor_divide(equation, np, va[i].v, 0.0))
				simp2_divide(equation, np, va[i].v, fc_level);
		}
		simp2_divide(equation, np, 0L, fc_level);
		for (i = 0; i < vc; i++) {
			while (factor_plus(equation, np, va[i].v, 0.0)) {
				simp2_divide(equation, np, 0L, fc_level);
			}
		}
		while (factor_plus(equation, np, MATCH_ANY, 0.0)) {
			simp2_divide(equation, np, 0L, fc_level);
		}
	}
	simp_ssub(equation, np, MATCH_ANY, 0.0, power_flag, true, fc_level);
}

/*
 * The most basic simplification loop.
 * Just does constant simplification.
 */
elim_loop(equation, np)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of equation side */
{
	side_debug(6, equation, *np);
	do {
		do {
			do {
				organize(equation, np);
			} while (combine_constants(equation, np, true));
		} while (elim_k(equation, np));
	} while (simp_pp(equation, np));
	if (reorder(equation, np)) {
		do {
			organize(equation, np);
		} while (elim_k(equation, np));
	}
	side_debug(5, equation, *np);
}

/*
 * Configurable high level simplify routine.
 */
simp_ssub(equation, np, v, d, power_flag, times_flag, fc_level)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of equation side */
long		v;		/* variable to factor, 0L or MATCH_ANY to factor all variables */
double		d;		/* factor expressions raised to the power of this if v */
int		power_flag;	/* factor_power() flag */
int		times_flag;	/* factor_times() flag */
int		fc_level;	/* factor constants code, passed to factor_constants() */
{
	do {
		do {
			do {
				do {
					do {
						do {
							do {
								do {
									elim_loop(equation, np);
								} while (simp2_power(equation, np));
							} while (times_flag && factor_times(equation, np));
						} while (elim_sign(equation, np));
					} while (subtract_itself(equation, np));
				} while (factor_constants(equation, np, fc_level));
			} while (factor_divide(equation, np, v, d));
		} while (factor_plus(equation, np, v, d));
	} while (power_flag && factor_power(equation, np));
}

/*
 * Commonly used quick simplify routine that doesn't factor.
 *
 * Return true if factor_times() simplified something.
 */
int
simp_loop(equation, np)
token_type	*equation;	/* pointer to the beginning of equation side to simplify */
int		*np;		/* pointer to length of equation side */
{
	int	i;
	int	rv = false;

	do {
		do {
			do {
				do {
					elim_loop(equation, np);
				} while (simp2_power(equation, np));
				i = factor_times(equation, np);
				if (i)
					rv = true;
			} while (i);
		} while (elim_sign(equation, np));
	} while (subtract_itself(equation, np));
	return rv;
}

/*
 * Factor only the specified variable "v".
 * If v is IMAGINARY, do complex number simplification.
 */
factorv(equation, np, v)
token_type	*equation;	/* pointer to the beginning of equation side to factor */
int		*np;		/* pointer to equation side length */
long		v;		/* variable to factor */
{
	if (v == IMAGINARY) {
		do {
			elim_loop(equation, np);
		} while (complex_root_simp(equation, np));
	}
	do {
		do {
			simp_loop(equation, np);
		} while (factor_plus(equation, np, v, 0.0));
	} while (v == IMAGINARY && div_imaginary(equation, np));
}

/*
 * Combine all like denominators.
 */
simp_divide(equation, np)
token_type	*equation;
int		*np;
{
	do {
		do {
			simp_loop(equation, np);
		} while (factor_constants(equation, np, 1));
	} while (factor_divide(equation, np, 0L, 0.0));
}

/*
 * Combine all like denominators containing variable "v".
 * Don't call factor_times().
 */
simp2_divide(equation, np, v, fc_level)
token_type	*equation;
int		*np;
long		v;
int		fc_level;
{
	do {
		do {
			do {
				do {
					do {
						elim_loop(equation, np);
					} while (simp2_power(equation, np));
				} while (elim_sign(equation, np));
			} while (subtract_itself(equation, np));
		} while (factor_constants(equation, np, fc_level));
	} while (factor_divide(equation, np, v, 0.0));
}

/*
 * Fix up levels of parentheses in an equation side.
 * Put similar operators on the same level.
 *
 * This is the inner-most loop in Mathomatic, make it fast.
 */
organize(equation, np)
token_type	*equation;	/* equation side pointer */
int		*np;		/* pointer to length of equation side */
{
	if (*np <= 0 || (*np & 1) != 1) {
		error("Internal error: Organize called with bad expression size.");
		longjmp(jmp_save, 13);
	}
	if (*np > n_tokens) {
		error("Internal error: Expression array overflow detected.");
		longjmp(jmp_save, 13);
	}
	org_recurse(equation, np, 0, 1, NULL);
}

static inline void
org_up_level(bp, ep, level, invert)
token_type	*bp, *ep;
int		level, invert;
{
	if (invert) {
		for (; bp <= ep; bp++) {
			bp->level--;
			if (bp->level == level && bp->kind == OPERATOR) {
				switch (bp->token.operatr) {
				case PLUS:
					bp->token.operatr = MINUS;
					break;
				case MINUS:
					bp->token.operatr = PLUS;
					break;
				case TIMES:
					bp->token.operatr = DIVIDE;
					break;
				case DIVIDE:
					bp->token.operatr = TIMES;
					break;
				}
			}
		}
	} else {
		for (; bp <= ep; bp++) {
			bp->level--;
		}
	}
}

/*
 * Recurse through every sub-expression in "equation", starting at "loc",
 * moving up levels to "level" of parentheses.
 */
static int
org_recurse(equation, np, loc, level, elocp)
token_type	*equation;	/* equation side pointer */
int		*np;		/* pointer to length of equation side */
int		loc, level, *elocp;
{
	token_type	*p1, *bp, *ep;
	int		op, sub_op;
	int		i;
	int		eloc;
	int		sub_eloc;
	int		min1;
	int		invert;

	bp = &equation[loc];
	ep = &equation[*np];
	min1 = bp->level;
	for (p1 = bp + 1; p1 < ep; p1 += 2) {
		if (p1->level < min1) {
			if (p1->level < level)
				break;
			min1 = p1->level;
		}
	}
	ep = p1;
	eloc = (ep - equation) - 1;
	if (elocp)
		*elocp = eloc;
	if (eloc == loc) {
		bp->level = max(level - 1, 1);
		return 0;
	}
	if (min1 > level) {
		for (p1 = bp; p1 < ep; p1++)
			p1->level -= min1 - level;
	}
	op = 0;
	for (p1 = bp + 1; p1 < ep; p1 += 2) {
		if (p1->level == level) {
			op = p1->token.operatr;
			break;
		}
	}
	for (i = loc; i <= eloc; i += 2) {
		if (equation[i].level > level) {
			sub_op = org_recurse(equation, np, i, level + 1, &sub_eloc);
			switch (sub_op) {
			case PLUS:
			case MINUS:
				if (op != PLUS && op != MINUS)
					break;
				invert = (i - 1 >= loc && equation[i-1].token.operatr == MINUS);
				org_up_level(&equation[i], &equation[sub_eloc], level, invert);
				break;
			case TIMES:
			case DIVIDE:
				if (op != TIMES && op != DIVIDE)
					break;
				invert = (i - 1 >= loc && equation[i-1].token.operatr == DIVIDE);
				org_up_level(&equation[i], &equation[sub_eloc], level, invert);
				break;
			}
			i = sub_eloc;
		}
	}
	return op;
}

/*
 * Convert (x^n)^m to x^(n*m).
 *
 * Return true if expression was modified.
 */
int
simp_pp(equation, np)
token_type	*equation;
int		*np;
{
	int		i, j, k;
	int		level;
	int		left_level;
	int		diff;
	int		modified = false;
	double		numerator, denominator;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			level = equation[i].level;
			left_level = 1;
			for (j = i - 2; j >= 0; j -= 2) {
				if (equation[j].level < level) {
					left_level = equation[j].level;
					break;
				}
			}
			for (j = i + 2; j < *np; j += 2) {
				if (equation[j].level <= level) {
					if (left_level <= equation[j].level && equation[j].token.operatr == POWER) {
						if (!symb_flag && (i + 2) == j && equation[j-1].kind == CONSTANT) {
							f_to_fraction(equation[j-1].token.constant, &numerator, &denominator);
							if (fmod(numerator, 2.0) == 0.0) {
								if (equation[j].level == equation[j+1].level
								    && equation[j+1].kind == CONSTANT) {
									f_to_fraction(equation[j+1].token.constant, &numerator, &denominator);
									if (fmod(denominator, 2.0) == 0.0) {
										break;
									}
								}
							}
						}
						equation[j].token.operatr = TIMES;
						for (k = i + 1; k < j; k++)
							equation[k].level++;
						diff = (level - equation[j].level) + 1;
						level = equation[j].level;
						for (k = j; k < *np && equation[k].level >= level; k++) {
							equation[k].level += diff;
						}
						modified = true;
					}
					break;
				}
			}
		}
	}
	return modified;
}

/*
 * Simplify things like 12^(1/2) to 2*3^(1/2).
 *
 * Return true if expression was modified.
 */
int
integer_root_simp(equation, np)
token_type	*equation;
int		*np;
{
	int	modified = false;
	int	i, j, k;
	int	level;
	double	d1, d2;
	int	root;

	for (i = 1; i < (*np - 2); i += 2) {
		if (equation[i].token.operatr == POWER) {
			level = equation[i].level;
			if (equation[i-1].level == level
			    && equation[i+2].level == level + 1
			    && equation[i+2].token.operatr == DIVIDE
			    && equation[i+3].level == level + 1
			    && equation[i-1].kind == CONSTANT
			    && equation[i+1].kind == CONSTANT
			    && equation[i+3].kind == CONSTANT) {
				if (i + 4 < *np && equation[i+4].level >= level)
					continue;
				d2 = equation[i+1].token.constant;
				if (d2 < 1.0 || d2 > 50.0 || fmod(d2, 1.0) != 0.0)
					continue;
				errno = 0;
				d2 = pow(equation[i-1].token.constant, d2);
				if (errno) {
					continue;
				}
				if (equation[i+3].token.constant > 50.0 || equation[i+3].token.constant < 2.0)
					continue;
				root = equation[i+3].token.constant;
				if (root != equation[i+3].token.constant || root < 2)
					continue;
				if (!factor_one(d2))
					continue;
				d1 = 1.0;
				for (j = 0; j < uno; j++) {
					while (ucnt[j] >= root) {
						d1 *= unique[j];
						ucnt[j] -= root;
					}
				}
				if (d1 == 1.0)
					continue;
				if (*np + 2 > n_tokens) {
					error_huge();
				}
				equation[i+1].token.constant = 1.0;
				equation[i-1].token.constant = multiply_out_unique();
				k = i - 1;
				j = i + 4;
				for (; k < j; k++) {
					equation[k].level++;
				}
				blt(&equation[i+1], &equation[i-1], (*np - (i - 1)) * sizeof(token_type));
				*np += 2;
				equation[i].level = level;
				equation[i].kind = OPERATOR;
				equation[i].token.operatr = TIMES;
				equation[i-1].level = level;
				equation[i-1].kind = CONSTANT;
				equation[i-1].token.constant = d1;
				modified = true;
				i += 2;
			}
		}
	}
	return modified;
}

static inline int
sp2_sub(equation, np, i)
token_type	*equation;
int		*np, i;
{
	int	j;
	int	level;

	level = equation[i].level;
	if (equation[i-1].level != level || equation[i-1].kind != CONSTANT)
		return false;
	if (equation[i+1].level != level + 1 || equation[i+1].kind != CONSTANT
	    || equation[i+1].token.constant == 1.0)
		return false;
	j = i + 2;
	if (j >= *np || equation[j].level != level + 1)
		return false;
	switch (equation[j].token.operatr) {
	case TIMES:
		break;
	case DIVIDE:
		if (*np + 2 > n_tokens) {
			error_huge();
		}
		blt(&equation[j+2], &equation[j], (*np - j) * sizeof(token_type));
		*np += 2;
		equation[j+1].level = level + 1;
		equation[j+1].kind = CONSTANT;
		equation[j+1].token.constant = 1.0;
		break;
	default:
		return false;
	}
	equation[j].level = level;
	equation[j].token.operatr = POWER;
	equation[i-1].level++;
	equation[i].level++;
	return true;
}

/*
 * Simplify constant^(constant*x).
 *
 * Return true if equation side was modified.
 */
int
simp_constant_power(equation, np)
token_type	*equation;
int		*np;
{
	int	modified = false;
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			modified |= sp2_sub(equation, np, i);
		}
	}
	return modified;
}

static inline int
sp_sub(equation, np, i)
token_type	*equation;
int		*np, i;
{
	int	j, k;
	int	level;
	int	op;

	level = equation[i].level;
	op = 0;
	k = -1;
	for (j = i + 1; j < *np && equation[j].level >= level; j++) {
		if (equation[j].level == level + 1) {
			if (equation[j].kind == OPERATOR) {
				op = equation[j].token.operatr;
			} else if (equation[j].kind == CONSTANT && equation[j].token.constant < 0.0) {
				k = j;
			}
		}
	}
	if (j - i <= 2 && equation[i+1].kind == CONSTANT && equation[i+1].token.constant < 0.0) {
		k = i + 1;
	} else if (k < 0)
		return false;
	switch (op) {
	case 0:
	case TIMES:
	case DIVIDE:
		if (*np + 2 > n_tokens) {
			error_huge();
		}
		equation[k].token.constant = -equation[k].token.constant;
		for (k = i - 2;; k--) {
			if (k < 0 || equation[k].level < level)
				break;
		}
		k++;
		for (i = k; i < j; i++)
			equation[i].level++;
		blt(&equation[k+2], &equation[k], (*np - k) * sizeof(token_type));
		*np += 2;
		equation[k].level = level;
		equation[k].kind = CONSTANT;
		equation[k].token.constant = 1.0;
		k++;
		equation[k].level = level;
		equation[k].kind = OPERATOR;
		equation[k].token.operatr = DIVIDE;
		return true;
	}
	return false;
}

/*
 * Convert x^-y to 1/(x^y).
 *
 * Return true if equation side was modified.
 */
int
simp2_power(equation, np)
token_type	*equation;
int		*np;
{
	int	modified = false;
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			modified |= sp_sub(equation, np, i);
		}
	}
	return modified;
}

/*
 * Combine two or more constants on the same level of parentheses.
 * If "iflag" is false, don't produce imaginary numbers.
 *
 * Return true if equation side was modified.
 */
int
combine_constants(equation, np, iflag)
token_type	*equation;	/* pointer to the beginning of equation side */
int		*np;		/* pointer to length of equation side */
int		iflag;		/* produce imaginary numbers flag */
{
	return const_recurse(equation, np, 0, 1, iflag);
}

/*
 * Do the floating point arithmetic for Mathomatic.
 *
 * Return true if successful.
 */
static inline int
calc(op1p, k1p, op2, k2)
int	*op1p;	/* pointer to operator 1, sometimes modified */
double	*k1p;	/* pointer to operand 1 and where to store the result */
int	op2;	/* operator 2 */
double	k2;	/* operand 2 */
{
	int	op1;
	double	d, d1, d2;

	domain_check = false;
	errno = 0;
	op1 = *op1p;
	switch (op2) {
	case PLUS:
	case MINUS:
		if (op1 == MINUS)
			d = -(*k1p);
		else
			d = *k1p;
		d1 = fabs(d) * epsilon;
		if (op2 == PLUS) {
			d += k2;
		} else {
			d -= k2;
		}
		if (fabs(d) < d1)
			d = 0.0;
		if (op1 == 0) {
			*k1p = d;
		} else {
			if (d >= 0.0) {
				*op1p = PLUS;
				*k1p = d;
			} else {
				*op1p = MINUS;
				*k1p = -d;
			}
		}
		break;
	case TIMES:
	case DIVIDE:
		if (op1 == 0)
			op1 = TIMES;
		if (op1 == op2) {
			*k1p *= k2;
		} else {
			if (op1 == DIVIDE) {
				*k1p = k2 / *k1p;
				*op1p = TIMES;
			} else if (op2 == DIVIDE) {
				*k1p = *k1p / k2;
			}
		}
		break;
	case MODULUS:
		if (k2 == 0) {
			debug_string(0, _("Warning: modulo 0 encountered, might be considered undefined."));
		}
#if	false	/* If fmod() always works.  It doesn't. */
		*k1p = fmod(*k1p, k2);
#else
		*k1p = modf(*k1p / k2, &d) * k2;
#endif
		if (true_modulus && *k1p < 0.0) {
			*k1p += fabs(k2);	/* make positive */
		}
		break;
	case POWER:
		if (*k1p < 0.0 && fmod(k2, 1.0) != 0.0) {
			break;
		}
		if (*k1p == 0.0 && k2 < 0.0) {
			error(_("Divide by zero error."));
			longjmp(jmp_save, 2);
		}
		domain_check = true;
		if (*k1p == 0.0 && k2 == 0.0) {
			debug_string(0, _("Warning: 0^0 encountered, might be considered undefined."));
			d = 1.0;	/* some people don't know 0^0 */
		} else {
			d = pow(*k1p, k2);
			if (preserve_roots && !approximate_roots) {
				if (isfinite(k2) && fmod(k2, 1.0) != 0.0 && f_to_fraction(*k1p, &d1, &d2)) {
					if (!f_to_fraction(d, &d1, &d2)) {
						domain_check = false;
						return false;
					}
				}
			}
		}
		check_err();
		if (domain_check)
			*k1p = d;
		break;
	case FACTORIAL:
#if	true	/* set this to false if lgamma() doesn't exist */
		d = exp(lgamma(*k1p + 1.0)) * signgam;
		if (errno) {	/* don't evaluate if overflow */
			return false;
		}
#else
		if (*k1p > 170.0 || *k1p < 0.0 || fmod(*k1p, 1.0) != 0.0) {
			return false;
		}
		d = 1.0;
		for (d1 = 2.0; d1 <= *k1p; d1 += 1.0) {
			d *= d1;
		}
#endif
		*k1p = d;
		break;
	default:
		return false;
	}
	return true;
}

static int
const_recurse(equation, np, loc, level, iflag)
token_type	*equation;
int		*np, loc, level, iflag;
{
	int		loc1, old_loc;
	int		const_count = 0;
	int		op, zero = 0;
	int		modified = false;
	double		d, d1, d2, numerator, denominator;
	complexs	cv, p;

	loc1 = old_loc = loc;
	for (;; loc++) {
beginning:
		if (loc >= *np || equation[loc].level < level) {
			if (loc - old_loc == 1)	/* decrement the level of parentheses if only one constant left */
				equation[old_loc].level = max(level - 1, 1);
			return modified;
		}
		if (equation[loc].level > level) {
			modified |= const_recurse(equation, np, loc, level + 1, iflag);
			for (; loc < *np && equation[loc].level > level; loc++)
				;
			goto beginning;
		}
		if (equation[loc].kind == CONSTANT) {
			if (const_count == 0) {
				loc1 = loc;
				const_count++;
				continue;
			}
			op = equation[loc-1].token.operatr;
			d = equation[loc1].token.constant;
			d2 = equation[loc].token.constant;
			if (calc((loc1 <= old_loc) ? &zero : &equation[loc1-1].token.operatr, &d, op, d2)) {
				if (op == POWER && !domain_check) {
					if (!f_to_fraction(d2, &numerator, &denominator)) {	/* if irrational power */
						if (!iflag || (preserve_roots && !approximate_roots))
							return modified;
						cv.re = d;
						cv.im = 0.0;
						p.re = d2;
						p.im = 0.0;
						cv = complex_pow(cv, p);
						if (*np + 2 > n_tokens) {
							error_huge();
						}
						blt(&equation[loc1+2], &equation[loc1], (*np - loc1) * sizeof(token_type));
						*np += 2;
						equation[loc1].level = level;
						equation[loc1].kind = CONSTANT;
						equation[loc1].token.constant = cv.re;
						loc1++;
						equation[loc1].level = level;
						equation[loc1].kind = OPERATOR;
						equation[loc1].token.operatr = PLUS;
						level++;
						equation[loc].level = level;
						equation[loc].kind = VARIABLE;
						equation[loc].token.variable = IMAGINARY;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = OPERATOR;
						equation[loc].token.operatr = TIMES;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = CONSTANT;
						equation[loc].token.constant = cv.im;
						return true;
					}
					errno = 0;
					d1 = pow(-d, d2);
					check_err();
					if (!always_positive(denominator)) {
						if (!always_positive(numerator)) {
							d1 = -d1;
						}
						d = d1;
						goto not_imaginary;
					}
					if (!iflag)
						return modified;
					if (*np + 2 > n_tokens) {
						error_huge();
					}
					blt(&equation[loc1+2], &equation[loc1], (*np - loc1) * sizeof(token_type));
					*np += 2;
					if (d2 == 0.5) {
						equation[loc1].level = level + 1;
						equation[loc1].kind = CONSTANT;
						equation[loc1].token.constant = -d;
						loc1++;
						equation[loc1].level = level + 1;
						equation[loc1].kind = OPERATOR;
						equation[loc1].token.operatr = POWER;
						equation[loc].level = level + 1;
						equation[loc].kind = CONSTANT;
						equation[loc].token.constant = d2;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = OPERATOR;
						equation[loc].token.operatr = TIMES;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = VARIABLE;
						equation[loc].token.variable = IMAGINARY;
					} else {
						equation[loc1].level = level;
						equation[loc1].kind = CONSTANT;
						equation[loc1].token.constant = d1;
						loc1++;
						equation[loc1].level = level;
						equation[loc1].kind = OPERATOR;
						equation[loc1].token.operatr = TIMES;
						level++;
						equation[loc].level = level;
						equation[loc].kind = VARIABLE;
						equation[loc].token.variable = IMAGINARY;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = OPERATOR;
						equation[loc].token.operatr = POWER;
						loc++;
						equation[loc].level = level;
						equation[loc].kind = CONSTANT;
						equation[loc].token.constant = d2 * 2.0;
					}
					return true;
				} else {
not_imaginary:
					equation[loc1].token.constant = d;
					modified = true;
					domain_check = false;
					blt(&equation[loc-1], &equation[loc+1], (*np - (loc + 1)) * sizeof(token_type));
					*np -= 2;
					loc -= 2;
				}
			}
		}
	}
}

/*
 * Eliminate operations that involve constants that can be simplified.
 * Fix addition/subtraction of negative or infinity constants.
 * Replace division by a constant with times its reciprocal.
 *
 * Return true if one or more eliminations occurred.
 */
int
elim_k(equation, np)
token_type	*equation;
int		*np;
{
	token_type	*p1, *p2, *p3, *p4;
	token_type	*ep;			/* end pointer */
	int		modified = false;
	int		level;
	double		d, numerator, denominator;
	int		flag;

	for (p1 = equation + 1;;) {
		ep = &equation[*np];
		if (p1 >= ep)
			break;
		if (p1->kind != OPERATOR) {
			p1++;
			continue;
		}
		level = p1->level;
		switch (p1->token.operatr) {
		case PLUS:
		case MINUS:
			p2 = p1 + 1;
			if (p1 + 2 < ep && (p1 + 2)->level == level + 1
			    && ((p1 + 2)->token.operatr == TIMES || (p1 + 2)->token.operatr == DIVIDE)
			    && p2->kind == CONSTANT && p2->token.constant < 0.0) {
				if (p1->token.operatr == PLUS)
					p1->token.operatr = MINUS;
				else
					p1->token.operatr = PLUS;
				p2->token.constant = -(p2->token.constant);
			}
			if (p2->level == level && p2->kind == CONSTANT) {
				if (p2->token.constant < 0.0) {
					if (p1->token.operatr == PLUS)
						p1->token.operatr = MINUS;
					else
						p1->token.operatr = PLUS;
					p2->token.constant = -p2->token.constant;
				}
				if (p2->token.constant == 0.0) {
					blt(p1, p1 + 2, (char *) ep - (char *) (p1 + 2));
					*np -= 2;
					modified = true;
					continue;
				}
			}
			if ((p1 - 1)->level == level && (p1 - 1)->kind == CONSTANT && isinf((p1 - 1)->token.constant)) {
				p2 = p1 - 1;
			}
			if (p2->level == level && p2->kind == CONSTANT && isinf(p2->token.constant)) {
				flag = false;
				for (p3 = p1;; p3--) {
					if (p3->level < level) {
						p3++;
						break;
					}
					if (p3->kind == CONSTANT && p3 != p2
					    && !isfinite(p3->token.constant)) {
						flag = true;
					}
					if (p3 == equation)
						break;
				}
				for (p4 = p1; p4 < ep && p4->level >= level; p4++) {
					if (p4->kind == CONSTANT && p4 != p2
					    && !isfinite(p4->token.constant)) {
						flag = true;
					}
				}
				if (!flag) { /* no other infinities on level */
					if (p2 > p3 && (p2 - 1)->token.operatr == MINUS) {
						p2->token.constant = -(p2->token.constant);
					}
					blt(p2 + 1, p4, (char *) ep - (char *) p4);
					*np -= p4 - (p2 + 1);
					ep = &equation[*np];
					blt(p3, p2, (char *) ep - (char *) p2);
					*np -= p2 - p3;
					return true;
				}
			}
			break;
		}
		switch (p1->token.operatr) {
		case PLUS:
			p2 = p1 - 1;
			if (p2->level == level && p2->kind == CONSTANT
			    && p2->token.constant == 0.0) {
				blt(p2, p1 + 1, (char *) ep - (char *) (p1 + 1));
				*np -= 2;
				modified = true;
				continue;
			}
			break;
		case MINUS:
			p2 = p1 - 1;
			if (p2->level == level && p2->kind == CONSTANT
			    && p2->token.constant == 0.0) {
				if (p2 == equation
				    || (p2 - 1)->level < level) {
					p2->token.constant = -1.0;
					p1->token.operatr = TIMES;
					binary_parenthesize(equation, *np, p1 - equation);
					modified = true;
					continue;
				}
			}
			break;
		case TIMES:
			if ((p1 - 1)->level == level && (p1 - 1)->kind == CONSTANT) {
				d = (p1 - 1)->token.constant;
				if (d == 0.0) {
					for (p2 = p1 + 2; p2 < ep; p2 += 2) {
						if (p2->level < level)
							break;
					}
					blt(p1, p2, (char *) ep - (char *) p2);
					*np -= p2 - p1;
					modified = true;
					continue;
				}
				if (fabs(d - 1.0) <= epsilon) {
					blt(p1 - 1, p1 + 1, (char *) ep - (char *) (p1 + 1));
					*np -= 2;
					modified = true;
					continue;
				}
			}
			if ((p1 + 1)->level == level && (p1 + 1)->kind == CONSTANT) {
				d = (p1 + 1)->token.constant;
				for (p2 = p1 - 1; p2 > equation; p2--) {
					if ((p2 - 1)->level < level)
						break;
				}
				if (p2->level == level && p2->kind == CONSTANT) {
					break;
				}
				blt(p2 + 2, p2, (char *) p1 - (char *) p2);
				p2->level = level;
				p2->kind = CONSTANT;
				p2->token.constant = d;
				(p2 + 1)->level = level;
				(p2 + 1)->kind = OPERATOR;
				(p2 + 1)->token.operatr = TIMES;
				if (p2 > equation) {
					p1 = p2 - 1;
					continue;
				} else {
					p1 = equation + 1;
					continue;
				}
			}
			break;
		case DIVIDE:
			p2 = p1 - 1;
			if (p2->level == level && p2->kind == CONSTANT
			    && p2->token.constant == 0.0) {
				for (p2 = p1 + 2; p2 < ep; p2++) {
					if (p2->level < level)
						break;
				}
				blt(p1, p2, (char *) ep - (char *) p2);
				*np -= p2 - p1;
				modified = true;
				continue;
			}
			p2 = p1 + 1;
			if (p2->level == level && p2->kind == CONSTANT) {
				f_to_fraction(p2->token.constant, &numerator, &denominator);
				p2->token.constant = denominator / numerator;
				p1->token.operatr = TIMES;
				continue;
			}
			if (p2->level == level
			    && p2->kind == VARIABLE && (p2->token.variable & VAR_MASK) == SIGN) {
				p1->token.operatr = TIMES;
				continue;
			}
			break;
		case MODULUS:
			p2 = p1 - 1;
			if (p2->level == level && p2->kind == CONSTANT
			    && p2->token.constant == 0.0) {
				for (p2 = p1 + 2; p2 < ep; p2++) {
					if (p2->level < level)
						break;
				}
				blt(p1, p2, (char *) ep - (char *) p2);
				*np -= p2 - p1;
				modified = true;
				continue;
			}
			break;
		case POWER:
			if ((p1 - 1)->level == level && (p1 - 1)->kind == CONSTANT) {
				if ((p1 - 1)->token.constant == 1.0) {
					for (p2 = p1 + 2; p2 < ep; p2++) {
						if (p2->level <= level)
							break;
					}
					blt(p1, p2, (char *) ep - (char *) p2);
					*np -= p2 - p1;
					modified = true;
					continue;
				}
			}
			if ((p1 + 1)->level == level && (p1 + 1)->kind == CONSTANT) {
				if ((p1 + 1)->token.constant == 0.0) {
					for (p2 = p1 - 1; p2 > equation; p2--) {
						if ((p2 - 1)->level <= level)
							break;
					}
					blt(p2, p1 + 1, (char *) ep - (char *) (p1 + 1));
					*np -= (p1 + 1) - p2;
					p2->token.constant = 1.0;
					p1 = p2 + 1;
					modified = true;
					continue;
				}
				if (fabs((p1 + 1)->token.constant - 1.0) <= epsilon) {
					blt(p1, p1 + 2, (char *) ep - (char *) (p1 + 2));
					*np -= 2;
					modified = true;
					continue;
				}
			}
			break;
		}
		p1 += 2;
	}
	return modified;
}

/*
 * Compare two sub-expressions for equality.
 * This is quick and low level and does not simplify or modify the expressions.
 *
 * If equal, return true with *diff_signp = 0.
 * if p1 * -1 == p2, return true with *diff_signp = 1.
 * Otherwise return false.
 */
int
se_compare(p1, n1, p2, n2, diff_signp)
token_type	*p1;		/* first sub-expression pointer */
int		n1;		/* first sub-expression length */
token_type	*p2;		/* second sub-expression pointer */
int		n2;		/* second sub-expression length */
int		*diff_signp;	/* different sign flag pointer */
{
	int	l1, l2;

	/* First, find the proper ground levels of parentheses for the two sub-expressions. */
	l1 = min_level(p1, n1);
	l2 = min_level(p2, n2);
	return compare_recurse(p1, n1, l1, p2, n2, l2, diff_signp);
}

/*
 * Recursively compare each parenthesized sub-expression.
 */
static int
compare_recurse(p1, n1, l1, p2, n2, l2, diff_signp)
token_type	*p1;
int		n1, l1;
token_type	*p2;
int		n2, l2, *diff_signp;
{
	token_type	*pv1, *ep1, *ep2;
	int		i, j;
	int		len;
	int		first;
	int		oc2;				/* operand count 2 */
	token_type	*opa2[MAX_COMPARE_TERMS];	/* operand pointer array 2 */
	char		used[MAX_COMPARE_TERMS];	/* operand used flag array 2 */
	int		last_op1, op1, op2;
	int		diff_op;
	double		d1, c1, c2;
	double		compare_epsilon = epsilon;

	*diff_signp = false;
	if (n1 == 1 && n2 == 1) {
		if (p1->kind != p2->kind) {
			return false;
		}
		switch (p1->kind) {
		case VARIABLE:
			if (sign_flag && (p1->token.variable & VAR_MASK) == SIGN) {
				return((p2->token.variable & VAR_MASK) == SIGN);
			} else {
				return(p1->token.variable == p2->token.variable);
			}
			break;
		case CONSTANT:
			c1 = p1->token.constant;
			c2 = p2->token.constant;
			if (c1 == c2) {
				return true;
			} else if (c1 == -c2) {
				*diff_signp = true;
				return true;
			}
			d1 = fabs(c1) * compare_epsilon;
			if (fabs(c1 - c2) < d1) {
				return true;
			}
			if (fabs(c1 + c2) < d1) {
				*diff_signp = true;
				return true;
			}
			break;
		}
		return false;
	}
	ep1 = &p1[n1];
	ep2 = &p2[n2];
	op1 = 0;
	for (pv1 = p1 + 1; pv1 < ep1; pv1 += 2) {
		if (pv1->level == l1) {
			op1 = pv1->token.operatr;
			break;
		}
	}
	op2 = 0;
	for (pv1 = p2 + 1; pv1 < ep2; pv1 += 2) {
		if (pv1->level == l2) {
			op2 = pv1->token.operatr;
			break;
		}
	}
	diff_op = false;
	if (op2 == 0) {
		if (op1 != TIMES && op1 != DIVIDE)
			return false;
		goto no_op2;
	}
	switch (op1) {
	case PLUS:
	case MINUS:
		if (op2 != PLUS && op2 != MINUS)
			diff_op = true;
		break;
	case 0:
		if (op2 != TIMES && op2 != DIVIDE)
			return false;
		break;
	case TIMES:
	case DIVIDE:
		if (op2 != TIMES && op2 != DIVIDE)
			diff_op = true;
		break;
	default:
		if (op2 != op1)
			diff_op = true;
		break;
	}
	if (diff_op) {
		if (p1->kind == CONSTANT && p1->level == l1 && op1 == TIMES) {
			if (fabs(fabs(p1->token.constant) - 1.0) <= compare_epsilon) {
				if (!compare_recurse(p1 + 2, n1 - 2, min_level(p1 + 2, n1 - 2), p2, n2, l2, diff_signp)) {
					return false;
				}
				if (p1->token.constant < 0.0) {
					*diff_signp ^= true;
				}
				return true;
			}
		}
		if (p2->kind == CONSTANT && p2->level == l2 && op2 == TIMES) {
			if (fabs(fabs(p2->token.constant) - 1.0) <= compare_epsilon) {
				if (!compare_recurse(p1, n1, l1, p2 + 2, n2 - 2, min_level(p2 + 2, n2 - 2), diff_signp)) {
					return false;
				}
				if (p2->token.constant < 0.0) {
					*diff_signp ^= true;
				}
				return true;
			}
		}
		return false;
	}
no_op2:
	opa2[0] = p2;
	used[0] = false;
	oc2 = 1;
	for (pv1 = p2 + 1; pv1 < ep2; pv1 += 2) {
		if (pv1->level == l2) {
			opa2[oc2] = pv1 + 1;
			used[oc2] = false;
			if (++oc2 >= ARR_CNT(opa2)) {
				return false;	/* expression too big to compare */
			}
		}
	}
	opa2[oc2] = pv1 + 1;
	last_op1 = 0;
	first = true;
	for (pv1 = p1;;) {
		for (len = 1; &pv1[len] < ep1; len += 2)
			if (pv1[len].level <= l1)
				break;
		for (i = 0;; i++) {
			if (i >= oc2) {
				if ((op1 == TIMES || op1 == DIVIDE) && pv1->level == l1 && pv1->kind == CONSTANT) {
					if (fabs(fabs(pv1->token.constant) - 1.0) <= compare_epsilon) {
						if (pv1->token.constant < 0.0) {
							*diff_signp ^= true;
						}
						break;
					}
				}
				return false;
			}
			if (used[i]) {
				continue;
			}
			switch (op1) {
			case PLUS:
			case MINUS:
				break;
			case 0:
			case TIMES:
			case DIVIDE:
				if ((last_op1 == DIVIDE) != ((i != 0)
				    && ((opa2[i] - 1)->token.operatr == DIVIDE)))
					continue;
				break;
			default:
				if ((last_op1 == 0) != (i == 0))
					return false;
				break;
			}
			if (compare_recurse(pv1, len, (pv1->level <= l1) ? l1 : (l1 + 1),
			    opa2[i], (opa2[i+1] - opa2[i]) - 1, (opa2[i]->level <= l2) ? l2 : (l2 + 1), &j)) {
				switch (op1) {
				case 0:
				case TIMES:
				case DIVIDE:
					*diff_signp ^= j;
					break;
				case PLUS:
				case MINUS:
					if (last_op1 == MINUS)
						j = !j;
					if (i != 0 && (opa2[i] - 1)->token.operatr == MINUS)
						j = !j;
					if (!first) {
						if (*diff_signp != j)
							continue;
					} else {
						*diff_signp = j;
						first = false;
					}
					break;
				default:
					if (j)
						continue;
					break;
				}
				used[i] = true;
				break;
			}
		}
		pv1 += len;
		if (pv1 >= ep1)
			break;
		last_op1 = pv1->token.operatr;
		pv1++;
	}
	for (i = 0; i < oc2; i++) {
		if (!used[i]) {
			if ((op2 == TIMES || op2 == DIVIDE) && opa2[i]->level == l2 && opa2[i]->kind == CONSTANT) {
				if (fabs(fabs(opa2[i]->token.constant) - 1.0) <= compare_epsilon) {
					if (opa2[i]->token.constant < 0.0) {
						*diff_signp ^= true;
					}
					continue;
				}
			}
			return false;
		}
	}
	return true;
}

/*
 * Take out meaningless "sign" variables and negative constants.
 * Simplify imaginary numbers raised to the power of some constants,
 * including simple division (1/i# -> -1*i#).
 *
 * Return true if equation side was modified.
 */
int
elim_sign(equation, np)
token_type	*equation;
int		*np;
{
	int		i, j;
	int		level;
	int		modified = false;
	int		op;
	double		d;
	double		numerator, denominator;

	for (i = 1; i < *np; i += 2) {
		level = equation[i].level;
		if (equation[i].token.operatr == DIVIDE) {
			if (equation[i+1].level == level
			    && equation[i+1].kind == VARIABLE
			    && equation[i+1].token.variable == IMAGINARY) {
				if (*np + 2 > n_tokens) {
					error_huge();
				}
				blt(&equation[i+2], &equation[i], (*np - i) * sizeof(token_type));
				*np += 2;
				equation[i].level = level;
				equation[i].kind = OPERATOR;
				equation[i].token.operatr = TIMES;
				i++;
				equation[i].level = level;
				equation[i].kind = CONSTANT;
				equation[i].token.constant = -1.0;
				i++;
				equation[i].level = level;
				equation[i].kind = OPERATOR;
				equation[i].token.operatr = TIMES;
				modified = true;
			}
		}
		if (equation[i].token.operatr == POWER
		    && equation[i+1].level == level
		    && equation[i+1].kind == CONSTANT) {
			f_to_fraction(equation[i+1].token.constant, &numerator, &denominator);
			if (always_positive(numerator)) {
				if (equation[i-1].level == level
				    && equation[i-1].kind == VARIABLE
				    && equation[i-1].token.variable == IMAGINARY) {
					equation[i-1].kind = CONSTANT;
					equation[i-1].token.constant = -1.0;
					equation[i+1].token.constant /= 2.0;
					modified = true;
					continue;
				}
				op = 0;
				for (j = i - 1; (j >= 0) && equation[j].level >= level; j--) {
					if (equation[j].level <= level + 1) {
						if (equation[j].kind == OPERATOR) {
							op = equation[j].token.operatr;
							break;
						}
					}
				}
				switch (op) {
				case 0:
				case TIMES:
				case DIVIDE:
					for (j = i - 1; (j >= 0) && equation[j].level >= level; j--) {
						if (equation[j].level <= level + 1) {
							if (equation[j].kind == VARIABLE
							    && (equation[j].token.variable & VAR_MASK) == SIGN) {
								equation[j].kind = CONSTANT;
								equation[j].token.constant = 1.0;
								modified = true;
							} else if (equation[j].kind == CONSTANT
							    && equation[j].token.constant < 0.0) {
								equation[j].token.constant = -equation[j].token.constant;
								modified = true;
							}
						}
					}
				}
			} else {
				if (equation[i-1].level == level && equation[i-1].kind == VARIABLE) {
					if (equation[i-1].token.variable == IMAGINARY) {
						d = fmod(equation[i+1].token.constant, 4.0);
						if (d == 1.0) {
							equation[i].token.operatr = TIMES;
							equation[i+1].token.constant = 1.0;
							modified = true;
						} else if (d == 3.0) {
							equation[i].token.operatr = TIMES;
							equation[i+1].token.constant = -1.0;
							modified = true;
						}
					} else if ((equation[i-1].token.variable & VAR_MASK) == SIGN) {
						equation[i+1].token.constant = fmod(equation[i+1].token.constant, 2.0);
					}
				}
			}
		}
	}
	return modified;
}

/*
 * Move the imaginary number from the denominator to the numerator.
 * Also does complex number division.
 * This often complicates expressions, so use with care.
 *
 * Return true if equation side was modified.
 */
int
div_imaginary(equation, np)
token_type	*equation;
int		*np;
{
	int		i, j, k;
	int		n;
	int		level;
	int		ilevel;
	int		op;
	int		iloc, biloc, eiloc;
	int		eloc;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == DIVIDE) {
			level = equation[i].level;
			op = 0;
			iloc = biloc = eiloc = -1;
			k = i;
			for (j = i + 1; j < *np && equation[j].level > level; j++) {
				if (equation[j].kind == OPERATOR && equation[j].level == level + 1) {
					op = equation[j].token.operatr;
					k = j;
					if (iloc >= 0 && eiloc < 0) {
						eiloc = j;
					}
				} else if (equation[j].kind == VARIABLE && equation[j].token.variable == IMAGINARY) {
					if (iloc >= 0) {
						op = 0;
						break;
					}
					iloc = j;
					biloc = k + 1;
				}
			}
			eloc = j;
			if (iloc >= 0 && eiloc < 0) {
				eiloc = j;
			}
			if (iloc < 0 || (op != PLUS && op != MINUS))
				continue;
			ilevel = equation[iloc].level;
			if (ilevel != level + 1) {
				if (ilevel != level + 2) {
					continue;
				}
				if (iloc > biloc && equation[iloc-1].token.operatr != TIMES)
					continue;
				if (iloc + 1 < eiloc) {
					switch (equation[iloc+1].token.operatr) {
					case TIMES:
					case DIVIDE:
						break;
					default:
						continue;
					}
				}
			}
			if ((eloc - (i + 1)) + 5 + (eiloc - biloc) + *np + 2 > n_tokens)
				error_huge();
			n = eloc - (i + 1);
			blt(scratch, &equation[i+1], n * sizeof(token_type));
			scratch[iloc-(i+1)].kind = CONSTANT;
			scratch[iloc-(i+1)].token.constant = 0.0;
			for (j = 0; j < n; j++)
				scratch[j].level += 2;
			scratch[n].level = level + 2;
			scratch[n].kind = OPERATOR;
			scratch[n].token.operatr = POWER;
			n++;
			scratch[n].level = level + 2;
			scratch[n].kind = CONSTANT;
			scratch[n].token.constant = 2.0;
			n++;
			scratch[n].level = level + 1;
			scratch[n].kind = OPERATOR;
			scratch[n].token.operatr = PLUS;
			n++;
			blt(&scratch[n], &equation[biloc], (eiloc - biloc) * sizeof(token_type));
			j = n;
			n += (eiloc - biloc);
			for (k = j; k < n; k++)
				scratch[k].level += 2;
			scratch[n].level = level + 2;
			scratch[n].kind = OPERATOR;
			scratch[n].token.operatr = POWER;
			n++;
			scratch[n].level = level + 2;
			scratch[n].kind = CONSTANT;
			scratch[n].token.constant = 2.0;
			n++;
			scratch[j+(iloc-biloc)].kind = CONSTANT;
			scratch[j+(iloc-biloc)].token.constant = 1.0;
			blt(&equation[iloc+2], &equation[iloc], (*np - iloc) * sizeof(token_type));
			*np += 2;
			ilevel++;
			equation[iloc].level = ilevel;
			equation[iloc].kind = CONSTANT;
			equation[iloc].token.constant = -1.0;
			iloc++;
			equation[iloc].level = ilevel;
			equation[iloc].kind = OPERATOR;
			equation[iloc].token.operatr = TIMES;
			iloc++;
			equation[iloc].level = ilevel;
			blt(&equation[i+1+n], &equation[i], (*np - i) * sizeof(token_type));
			*np += n + 1;
			blt(&equation[i+1], scratch, n * sizeof(token_type));
			equation[i+1+n].token.operatr = TIMES;
			return true;
		}
	}
	return false;
}

/*
 * Convert 1/m*(-1*b+y) to (y-b)/m.
 *
 * Returns true if equation side was modified.
 * "elim_k()" should be called after this if it returns true.
 *
 * This routine also checks the validity of the equation side.
 */
int
reorder(equation, np)
token_type	*equation;
int		*np;
{
	return(order_recurse(equation, np, 0, 1));
}

static inline void
swap(equation, np, level, i1, i2)
token_type	*equation;
int		*np;
int		level;
int		i1, i2;
{
	int	e1, e2;
	int	n1, n2;

	for (e1 = i1 + 1;; e1 += 2) {
		if (e1 >= *np || equation[e1].level <= level)
			break;
	}
	for (e2 = i2 + 1;; e2 += 2) {
		if (e2 >= *np || equation[e2].level <= level)
			break;
	}
	n1 = e1 - i1;
	n2 = e2 - i2;
	blt(scratch, &equation[i1], (e2 - i1) * sizeof(token_type));
	blt(&equation[i1+n2], &equation[e1], (i2 - e1) * sizeof(token_type));
	blt(&equation[i1], &scratch[i2-i1], n2 * sizeof(token_type));
	blt(&equation[e2-n1], scratch, n1 * sizeof(token_type));
}

static int
order_recurse(equation, np, loc, level)
token_type	*equation;
int		*np, loc, level;
{
	int	i, j, k, n;
	int	op = 0;
	int	modified = false;

	if ((loc & 1) != 0) {
		goto corrupt;
	}
	for (i = loc; i < *np;) {
		if (equation[i].level < level) {
			if (equation[i].kind != OPERATOR) {
				goto corrupt;
			}
			break;
		}
		if (equation[i].level > level) {
			modified |= order_recurse(equation, np, i, level + 1);
			i++;
			for (; i < *np && equation[i].level > level; i++)
				;
			continue;
		} else {
			if (((i & 1) == 1) != (equation[i].kind == OPERATOR)) {
				goto corrupt;
			}
			if (equation[i].kind == OPERATOR) {
				if (op) {
					switch (equation[i].token.operatr) {
					case PLUS:
					case MINUS:
						if (op == PLUS || op == MINUS)
							break;
						op = 0;
					case TIMES:
					case DIVIDE:
						if (op == TIMES || op == DIVIDE)
							break;
					default:
						goto corrupt;
					}
				} else {
					op = equation[i].token.operatr;
				}
			}
		}
		i++;
	}
	switch (op) {
	case PLUS:
	case MINUS:
		if (equation[loc].kind == CONSTANT && equation[loc].token.constant < 0.0) {
			if (equation[loc].level == level || (equation[loc+1].level == level + 1
			    && (equation[loc+1].token.operatr == TIMES || equation[loc+1].token.operatr == DIVIDE))) {
				for (j = loc + 1; j < i; j += 2) {
					if (equation[j].level == level && equation[j].token.operatr == PLUS) {
						swap(equation, np, level, loc, j + 1);
						modified = true;
						break;
					}
				}
			}
		}
		break;
	case TIMES:
	case DIVIDE:
		for (j = loc + 1;; j += 2) {
			if (j >= i)
				return modified;
			if (equation[j].level == level && equation[j].token.operatr == DIVIDE)
				break;
		}
		for (k = j + 2; k < i;) {
			if (equation[k].level == level && equation[k].token.operatr == TIMES) {
				for (n = k + 2; n < i && equation[n].level > level; n += 2)
					;
				n -= k;
				blt(scratch, &equation[k], n * sizeof(token_type));
				blt(&equation[j+n], &equation[j], (k - j) * sizeof(token_type));
				blt(&equation[j], scratch, n * sizeof(token_type));
				j += n;
				k += n;
				modified = true;
				continue;
			}
			k += 2;
		}
		break;
	}
	return modified;
corrupt:
	error(_("Expression is corrupt!  Please file a bug report."));
	longjmp(jmp_save, 13);
}

/*
 * Rationalize the denominator of algebraic fractions.
 * Only works with square roots.
 *
 * Returns true if something was done.
 * Unfactoring needs to be done immediately, if it returns true.
 */
int
rationalize(equation, np)
token_type	*equation;
int		*np;
{
	int	i, j, k;
	int	i1, k1;
	int	div_level;
	int	end_loc, neg_one_loc = -1;
	int	flag;
	int	modified = false;
	int	count;

	for (i = 1;; i += 2) {
be_thorough:
		if (i >= *np)
			break;
		if (equation[i].token.operatr == DIVIDE) {
			div_level = equation[i].level;
			count = 0;
			j = -1;
			for (end_loc = i + 2; end_loc < *np && equation[end_loc].level > div_level; end_loc += 2) {
				if (equation[end_loc].level == (div_level + 1)) {
					count++;
					if (j < 0) {
						j = end_loc;
					}
				}
			}
			if (j < 0)
				continue;
			switch (equation[j].token.operatr) {
			case PLUS:
			case MINUS:
				break;
			default:
				continue;
			}
			i1 = i;
do_again:
			flag = false;
			for (k = j - 2; k > i1; k -= 2) {
				if (equation[k].level == (div_level + 2)) {
					switch (equation[k].token.operatr) {
					case TIMES:
					case DIVIDE:
						flag = 1;
						break;
					case POWER:
						flag = 2;
						break;
					}
					break;
				}
			}
			if (flag) {
				for (k = j - 2; k > i1; k -= 2) {
					if ((equation[k].level == (div_level + 2)
					    || (equation[k].level == (div_level + 3) && flag == 1))
					    && equation[k].token.operatr == POWER
					    && equation[k].level == equation[k+1].level
					    && equation[k+1].kind == CONSTANT
					    && fmod(equation[k+1].token.constant, 1.0) == 0.5) {
						if (count != 1) {
							for (k1 = i + 2; k1 < end_loc; k1 += 2) {
								if (equation[k1].token.operatr == POWER
								    && equation[k1].level == equation[k1+1].level
								    && equation[k1+1].kind == CONSTANT
								    && fmod(equation[k1+1].token.constant, 1.0) == 0.5
								    && k != k1) {
									i += 2;
									goto be_thorough;
								}
							}
						}
						neg_one_loc = i1 + 1;
						k = i1 - i;
						blt(scratch, &equation[i+1], k * sizeof(token_type));
						scratch[k].level = div_level + 2;
						scratch[k].kind = CONSTANT;
						scratch[k].token.constant = -1.0;
						k++;
						scratch[k].level = div_level + 2;
						scratch[k].kind = OPERATOR;
						scratch[k].token.operatr = TIMES;
						k++;
						blt(&scratch[k], &equation[neg_one_loc], (end_loc - neg_one_loc) * sizeof(token_type));
						for (k1 = 0; k1 < (j - neg_one_loc); k1++, k++)
							scratch[k].level++;
						k = end_loc - (i + 1) + 2;
						if (*np + 2 * (k + 1) > n_tokens) {
							error_huge();
						}
						blt(&equation[end_loc+(2*(k+1))], &equation[end_loc], (*np - end_loc) * sizeof(token_type));
						*np += 2 * (k + 1);
						k1 = end_loc;
						equation[k1].level = div_level;
						equation[k1].kind = OPERATOR;
						equation[k1].token.operatr = TIMES;
						k1++;
						blt(&equation[k1], scratch, k * sizeof(token_type));
						k1 += k;
						equation[k1].level = div_level;
						equation[k1].kind = OPERATOR;
						equation[k1].token.operatr = DIVIDE;
						k1++;
						blt(&equation[k1], scratch, k * sizeof(token_type));
						modified = true;
						i = k1 + k;
						goto be_thorough;
					}
				}
			}
			if (j >= end_loc)
				continue;
			i1 = j;
			for (j += 2; j < end_loc; j += 2) {
				if (equation[j].level == (div_level + 1)) {
					break;
				}
			}
			goto do_again;
		}
	}
	return modified;
}
