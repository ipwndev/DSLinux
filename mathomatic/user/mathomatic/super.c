/*
 * Group and combine denominators.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

#include "includes.h"

static int	sf_recurse();
static int	sf_sub();
static void	group_recurse();

/*
 * Combine denominators in equation side.
 * This means converting "a/b+c/d" to "(a*d+c*b)/b/d".
 *
 * If start_flag is false, only combine denominators to simplify complex fractions.
 *
 * If start_flag is true, always combine denominators.
 *
 * If start_flag is 2, combine denominators if they don't have a GCD in common.
 * Note that this wipes out tlhs and trhs.  This prevents making a more complicated
 * algebraic fraction.  This needs to be improved to combine denominators and remove
 * the GCD.
 *
 * Return true if equation side was modified.
 */
int
super_factor(equation, np, start_flag)
token_type	*equation;
int		*np;
int		start_flag;
{
	int	rv;

	group_proc(equation, np);
	rv = sf_recurse(equation, np, 0, 1, start_flag);
	organize(equation, np);
	return rv;
}

static int
sf_recurse(equation, np, loc, level, start_flag)
token_type	*equation;
int		*np, loc, level, start_flag;
{
	int	modified = false;
	int	i, j, k;
	int	op;
	int	len1, len2;

	if (!start_flag) {
		for (i = loc + 1; i < *np && equation[i].level >= level; i += 2) {
			if (equation[i].level == level
			    && equation[i].token.operatr == DIVIDE) {
				start_flag = true;
				break;
			}
		}
	}
	op = 0;
	for (i = loc; i < *np && equation[i].level >= level;) {
		if (equation[i].level > level) {
			modified |= sf_recurse(equation, np, i, level + 1, start_flag);
			i++;
			for (; i < *np && equation[i].level > level; i += 2)
				;
			continue;
		} else if (equation[i].kind == OPERATOR) {
			op = equation[i].token.operatr;
		}
		i++;
	}
	if (modified || !start_flag)
		return modified;
	switch (op) {
	case PLUS:
	case MINUS:
		break;
	default:
		return modified;
	}
sf_again:
	i = loc;
	for (k = i + 1; k < *np && equation[k].level > level; k += 2)
		;
	len1 = k - i;
	for (j = i + len1 + 1; j < *np && equation[j-1].level >= level; j += len2 + 1) {
		for (k = j + 1; k < *np && equation[k].level > level; k += 2)
			;
		len2 = k - j;
		if (sf_sub(equation, np, loc, i, len1, j, len2, level + 1, start_flag == 2)) {
			modified = true;
			goto sf_again;
		}
	}
	return modified;
}

static int
sf_sub(equation, np, loc, i1, n1, i2, n2, level, check_flag)
token_type	*equation;
int		*np, loc, i1, n1, i2, n2, level, check_flag;
{
	int	i, j, k;
	int	b1, b2;
	int	len;
	int	e1, e2;
	int	op1, op2;
	int	div_flag1 = false, div_flag2 = false;

	e1 = i1 + n1;
	e2 = i2 + n2;
	op2 = equation[i2-1].token.operatr;
	switch (op2) {
	case PLUS:
	case MINUS:
		break;
	default:
		return false;
	}
	if (i1 <= loc) {
		op1 = PLUS;
	} else
		op1 = equation[i1-1].token.operatr;
	for (i = i1 + 1; i < e1; i += 2) {
		if (equation[i].level == level && equation[i].token.operatr == DIVIDE) {
			div_flag1 = true;
			break;
		}
	}
	b1 = i + 1;
	if (div_flag1) {
		for (i += 2; i < e1; i += 2) {
			if (equation[i].level <= level)
				break;
		}
	}
	for (j = i2 + 1; j < e2; j += 2) {
		if (equation[j].level == level && equation[j].token.operatr == DIVIDE) {
			div_flag2 = true;
			break;
		}
	}
	b2 = j + 1;
	if (div_flag2) {
		for (j += 2; j < e2; j += 2) {
			if (equation[j].level <= level)
				break;
		}
	}
	if (!div_flag1 && !div_flag2)
		return false;
	if (check_flag && div_flag1 && div_flag2) {
		if (poly2_gcd(&equation[b1], i - b1, &equation[b2], j - b2, 0L)
		    || poly2_gcd(&equation[b2], j - b2, &equation[b1], i - b1, 0L)) {
			return false;
		}
	}
	if (n1 + n2 + (i - b1) + (j - b2) + 8 > n_tokens) {
		error_huge();
	}
	if (!div_flag1) {
		for (k = i1; k < e1; k++)
			equation[k].level++;
	}
	if (!div_flag2) {
		for (k = i2; k < e2; k++)
			equation[k].level++;
	}
	len = (b1 - 1) - i1;
	blt(scratch, &equation[i1], len * sizeof(*equation));
	if (op1 == MINUS) {
		scratch[len].level = level;
		scratch[len].kind = OPERATOR;
		scratch[len].token.operatr = TIMES;
		len++;
		scratch[len].level = level;
		scratch[len].kind = CONSTANT;
		scratch[len].token.constant = -1.0;
		len++;
	}
	if (div_flag1) {
		blt(&scratch[len], &equation[i], (e1 - i) * sizeof(*equation));
		len += e1 - i;
	}
	if (div_flag2) {
		scratch[len].level = level;
		scratch[len].kind = OPERATOR;
		scratch[len].token.operatr = TIMES;
		len++;
		blt(&scratch[len], &equation[b2], (j - b2) * sizeof(*equation));
		len += j - b2;
	}
	for (k = 0; k < len; k++)
		scratch[k].level += 2;
	scratch[len].level = level + 1;
	scratch[len].kind = OPERATOR;
	scratch[len].token.operatr = op2;
	len++;
	k = len;
	blt(&scratch[len], &equation[i2], (b2 - i2 - 1) * sizeof(*equation));
	len += b2 - i2 - 1;
	if (div_flag2) {
		blt(&scratch[len], &equation[j], (e2 - j) * sizeof(*equation));
		len += e2 - j;
	}
	if (div_flag1) {
		scratch[len].level = level;
		scratch[len].kind = OPERATOR;
		scratch[len].token.operatr = TIMES;
		len++;
		blt(&scratch[len], &equation[b1], (i - b1) * sizeof(*equation));
		len += i - b1;
	}
	for (; k < len; k++)
		scratch[k].level += 2;
	scratch[len].level = level;
	scratch[len].kind = OPERATOR;
	scratch[len].token.operatr = DIVIDE;
	len++;
	k = len;
	if (div_flag1) {
		blt(&scratch[len], &equation[b1], (i - b1) * sizeof(*equation));
		len += i - b1;
	}
	if (div_flag1 && div_flag2) {
		scratch[len].level = level;
		scratch[len].kind = OPERATOR;
		scratch[len].token.operatr = TIMES;
		len++;
	}
	if (div_flag2) {
		blt(&scratch[len], &equation[b2], (j - b2) * sizeof(*equation));
		len += j - b2;
	}
	for (; k < len; k++)
		scratch[k].level++;
	if (*np + len - n1 - (n2 + 1) > n_tokens) {
		error_huge();
	}
	if (op1 == MINUS) {
		equation[i1-1].token.operatr = PLUS;
	}
	blt(&equation[i2-1], &equation[e2], (*np - e2) * sizeof(*equation));
	*np -= n2 + 1;
	blt(&equation[i1+len], &equation[e1], (*np - e1) * sizeof(*equation));
	*np += len - n1;
	blt(&equation[i1], scratch, len * sizeof(*equation));
	return true;
}

/*
 * This function is the guts of the "flist" command.
 */
group_sub(n)
int	n;
{
	if (n_lhs[n] <= 0)
		return;
	elim_loop(lhs[n], &n_lhs[n]);
	make_fractions(lhs[n], &n_lhs[n]);
	group_proc(lhs[n], &n_lhs[n]);

	if (n_rhs[n]) {
		elim_loop(rhs[n], &n_rhs[n]);
		make_fractions(rhs[n], &n_rhs[n]);
		group_proc(rhs[n], &n_rhs[n]);
	}
}

/*
 * Group denominators.
 * Grouping here means converting "a/b/c" to "a/(b*c)".
 */
group_proc(equation, np)
token_type	*equation;
int		*np;
{
	group_recurse(equation, np, 0, 1);
}

static void
group_recurse(equation, np, loc, level)
token_type	*equation;
int		*np, loc, level;
{
	int		i;
	int		len;
	int		di, edi;
	int		grouper = false;
	int		e1;

	for (i = loc; i < *np && equation[i].level >= level;) {
		if (equation[i].level > level) {
			group_recurse(equation, np, i, level + 1);
			i++;
			for (; i < *np && equation[i].level > level; i += 2)
				;
			continue;
		}
		i++;
	}
	e1 = i;
	di = -1;
	edi = e1;
	for (i = loc + 1; i < e1; i += 2) {
		if (equation[i].level == level) {
			if (equation[i].token.operatr == DIVIDE) {
				if (di < 0) {
					di = i;
					continue;
				}
				grouper = true;
				for (len = i + 2; len < e1; len += 2) {
					if (equation[len].level == level
					    && equation[len].token.operatr != DIVIDE)
						break;
				}
				len -= i;
				if (edi == e1) {
					i += len;
					edi = i;
					continue;
				}
				blt(scratch, &equation[i], len * sizeof(*equation));
				blt(&equation[di+len], &equation[di], (i - di) * sizeof(*equation));
				blt(&equation[di], scratch, len * sizeof(*equation));
				edi += len;
				i += len - 2;
			} else {
				if (di >= 0 && edi == e1)
					edi = i;
			}
		}
	}
	if (!grouper) {
		return;
	}
	for (i = di + 1; i < edi; i++) {
		if (equation[i].level == level && equation[i].kind == OPERATOR) {
			equation[i].token.operatr = TIMES;
		}
		equation[i].level++;
	}
}
