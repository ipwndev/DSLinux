/*
 * Mathomatic unfactorizing (expanding) routines.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

static int	unf_sub();

/*
 * Totally unfactor equation side and simplify.
 */
uf_simp(equation, np)
token_type	*equation;	/* pointer to beginning of equation side */
int		*np;		/* pointer to length of equation side */
{
	if (*np == 0)
		return;
	uf_tsimp(equation, np);
	uf_power(equation, np);
	uf_repeat(equation, np);
	uf_tsimp(equation, np);
}

/*
 * Unfactor equation side and simplify.
 * Don't call uf_repeat().
 */
uf_simp_no_repeat(equation, np)
token_type	*equation;
int		*np;
{
	uf_power(equation, np);
	uf_tsimp(equation, np);
}

/*
 * Unfactor times and divide only and simplify.
 */
int
uf_tsimp(equation, np)
token_type	*equation;
int		*np;
{
	int	rv;

	rv = uf_times(equation, np);
	simp_loop(equation, np);
	if (uf_times(equation, np)) {
		rv = true;
		simp_loop(equation, np);
	}
	return rv;
}

/*
 * Totally unfactor equation side with no simplification.
 */
int
ufactor(equation, np)
token_type	*equation;
int		*np;
{
	int	rv;

	uf_repeat(equation, np);
	rv = uf_times(equation, np);
	uf_allpower(equation, np);
	return rv;
}

/*
 * Unfactor power only.
 * (a * b)^c -> a^c * b^c
 * Return true if equation side is modified.
 */
int
uf_power(equation, np)
token_type	*equation;
int		*np;
{
	int	rv;

	organize(equation, np);
	rv = sub_ufactor(equation, np, 2);
	if (rv) {
		organize(equation, np);
	}
	return rv;
}

/*
 * Unfactor power only.
 * a^(b + c) -> a^b * a^c
 * Return true if equation side is modified.
 */
int
uf_pplus(equation, np)
token_type	*equation;
int		*np;
{
	int	rv;

	organize(equation, np);
	rv = sub_ufactor(equation, np, 4);
	if (rv) {
		organize(equation, np);
	}
	return rv;
}

/*
 * Unfactor all power operators.  Same as a call to uf_pplus()
 * and uf_power(), only faster.
 */
uf_allpower(equation, np)
token_type	*equation;
int		*np;
{
	do {
		organize(equation, np);
	} while (sub_ufactor(equation, np, 0));
}

/*
 * Unfactor power only if it will help with expansion.
 * (a + 1)^2 -> (a + 1)*(a + 1)
 *
 * uf_times() is usually called after this.
 */
uf_repeat(equation, np)
token_type	*equation;
int		*np;
{
	organize(equation, np);
	if (sub_ufactor(equation, np, 6)) {
		organize(equation, np);
	}
	patch_root_div(equation, np);
}

/*
 * Unfactor power only.
 * a^2 -> a*a
 * Useful for removing all integer powers.
 */
uf_repeat_always(equation, np)
token_type	*equation;
int		*np;
{
	organize(equation, np);
	if (sub_ufactor(equation, np, 8)) {
		organize(equation, np);
	}
}

static inline void
no_divide(equation, np)
token_type	*equation;
int		*np;
{
	int	i, j;
	int	level;
	int	plus_flag;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == DIVIDE) {
			level = equation[i].level;
			if (partial_flag != 2) {
				plus_flag = false;
				for (j = i + 2; j < *np; j += 2) {
					if (equation[j].level <= level)
						break;
					switch (equation[j].token.operatr) {
					case PLUS:
					case MINUS:
						plus_flag = true;
						break;
					}
				}
				if (!plus_flag)
					continue;
			}
			for (j = i - 1; j >= 0; j--) {
				if (equation[j].level < level)
					break;
				equation[j].level += 2;
			}
		}
	}
}

/*
 * Unfactor times and divide only.
 * (a + b)*c -> a*c + b*c
 * If "partial_flag", don't unfactor (a+b)/(c+d).
 *
 * Return true if equation side was modified.
 */
int
uf_times(equation, np)
token_type	*equation;
int		*np;
{
	int	i;
	int	rv = false;

	do {
		organize(equation, np);
		if (partial_flag) {
			reorder(equation, np);
		}
		group_proc(equation, np);
		if (partial_flag) {
			no_divide(equation, np);
		}
		i = sub_ufactor(equation, np, 1);
		rv |= i;
	} while (i);
	organize(equation, np);
	return rv;
}

/*
 * General equation side algebraic expansion routine.
 * Expands everything.
 * The type of expansion to be done is indicated by the code in "ii".
 *
 * Return true if equation side was modified.
 */
int
sub_ufactor(equation, np, ii)
token_type	*equation;
int		*np;
int		ii;
{
	int	modified = false;
	int	i;
	int	b1, e1;
	int	level;

	for (i = 1; i < *np; i += 2) {
		switch (equation[i].token.operatr) {
		case TIMES:
		case DIVIDE:
			if (ii == 1)
				break;
			else
				continue;
		case POWER:
			if ((ii & 1) == 0)	/* even number codes only */
				break;
		default:
			continue;
		}
		level = equation[i].level;
		for (b1 = i - 2; b1 >= 0; b1 -= 2)
			if (equation[b1].level < level)
				break;
		b1++;
		for (e1 = i + 2; e1 < *np; e1 += 2) {
			if (equation[e1].level < level)
				break;
		}
		if (unf_sub(equation, np, b1, i, e1, level, ii)) {
			modified = true;
			i = b1 - 1;
			continue;
		}
	}
	return modified;
}

static int
unf_sub(equation, np, b1, loc, e1, level, ii)
token_type	*equation;
int		*np;
int		b1, loc, e1, level;
int		ii;
{
	int		i, j, k;
	int		b2, eb1, be1;
	int		len;
	double		d1, d2;

	switch (equation[loc].token.operatr) {
	case TIMES:
	case DIVIDE:
		if (ii != 1)
			break;
		for (i = b1 + 1; i < e1; i += 2) {
			if (equation[i].level == level + 1) {
				switch (equation[i].token.operatr) {
				case PLUS:
				case MINUS:
					break;
				default:
					continue;
				}
				for (b2 = i - 2; b2 >= b1; b2 -= 2)
					if (equation[b2].level <= level)
						break;
				b2++;
				eb1 = b2;
				for (be1 = i + 2; be1 < e1; be1 += 2)
					if (equation[be1].level <= level)
						break;
				if (eb1 > b1 && equation[eb1-1].token.operatr == DIVIDE) {
					i = be1 - 2;
					continue;
				}
				len = 0;
u_again:
				if (len + (eb1 - b1) + (i - b2) + (e1 - be1) + 1 > n_tokens) {
					error_huge();
				}
				blt(&scratch[len], &equation[b1], (eb1 - b1) * sizeof(token_type));
				j = len;
				len += (eb1 - b1);
				for (; j < len; j++)
					scratch[j].level++;
				blt(&scratch[len], &equation[b2], (i - b2) * sizeof(token_type));
				len += (i - b2);
				blt(&scratch[len], &equation[be1], (e1 - be1) * sizeof(token_type));
				j = len;
				len += (e1 - be1);
				for (; j < len; j++)
					scratch[j].level++;
				if (i < be1) {
					scratch[len] = equation[i];
					scratch[len].level--;
					len++;
					b2 = i + 1;
					for (i += 2; i < be1; i += 2) {
						if (equation[i].level == (level + 1))
							break;
					}
					goto u_again;
				} else {
					if (*np - (e1 - b1) + len > n_tokens) {
						error_huge();
					}
					blt(&equation[b1+len], &equation[e1], (*np - e1) * sizeof(token_type));
					*np += len - (e1 - b1);
					blt(&equation[b1], scratch, len * sizeof(token_type));
					return true;
				}
			}
		}
		break;
	case POWER:
		if (ii == 2 || ii == 0) {
			for (i = b1 + 1; i < loc; i += 2) {
				if (equation[i].level != level + 1)
					continue;
				switch (equation[i].token.operatr) {
				case TIMES:
				case DIVIDE:
					break;
				default:
					goto do_pplus;
				}
				b2 = b1;
				len = 0;
u1_again:
				if (len + (i - b2) + (e1 - loc) + 1 > n_tokens) {
					error_huge();
				}
				blt(&scratch[len], &equation[b2], (i - b2) * sizeof(token_type));
				len += (i - b2);
				blt(&scratch[len], &equation[loc], (e1 - loc) * sizeof(token_type));
				j = len;
				len += (e1 - loc);
				for (; j < len; j++)
					scratch[j].level++;
				if (i < loc) {
					scratch[len] = equation[i];
					scratch[len].level--;
					len++;
					b2 = i + 1;
					for (i += 2; i < loc; i += 2) {
						if (equation[i].level == (level + 1))
							break;
					}
					goto u1_again;
				} else {
					if (*np - (e1 - b1) + len > n_tokens) {
						error_huge();
					}
					blt(&equation[b1+len], &equation[e1], (*np - e1) * sizeof(token_type));
					*np += len - (e1 - b1);
					blt(&equation[b1], scratch, len * sizeof(token_type));
					return true;
				}
			}
		}
do_pplus:
		if (ii == 4 || ii == 0) {
			for (i = loc + 2; i < e1; i += 2) {
				if (equation[i].level != level + 1)
					continue;
				switch (equation[i].token.operatr) {
				case PLUS:
				case MINUS:
					break;
				default:
					goto do_repeat;
				}
				b2 = loc + 1;
				len = 0;
u2_again:
				if (len + (loc - b1) + (i - b2) + 2 > n_tokens) {
					error_huge();
				}
				j = len;
				blt(&scratch[len], &equation[b1], (loc + 1 - b1) * sizeof(token_type));
				len += (loc + 1 - b1);
				for (; j < len; j++)
					scratch[j].level++;
				blt(&scratch[len], &equation[b2], (i - b2) * sizeof(token_type));
				len += (i - b2);
				if (i < e1) {
					scratch[len].level = level;
					scratch[len].kind = OPERATOR;
					if (equation[i].token.operatr == PLUS)
						scratch[len].token.operatr = TIMES;
					else
						scratch[len].token.operatr = DIVIDE;
					len++;
					b2 = i + 1;
					for (i += 2; i < e1; i += 2) {
						if (equation[i].level == (level + 1))
							break;
					}
					goto u2_again;
				} else {
					if (*np - (e1 - b1) + len > n_tokens) {
						error_huge();
					}
					blt(&equation[b1+len], &equation[e1], (*np - e1) * sizeof(token_type));
					*np += len - (e1 - b1);
					blt(&equation[b1], scratch, len * sizeof(token_type));
					return true;
				}
			}
		}
do_repeat:
		if (ii != 6 && ii != 8)
			break;
		i = loc;
		if (equation[i+1].level != level || equation[i+1].kind != CONSTANT)
			break;
		d1 = equation[i+1].token.constant;
		if (!isfinite(d1) || d1 <= 1.0)
			break;
		if (ii != 8) {
			if ((i - b1) > 1 && d1 > 2.0 && fmod(d1, 1.0) != 0.0)
				break;
			if ((i - b1) == 1 && equation[b1].kind != CONSTANT)
				break;
		}
		d1 = ceil(d1) - 1.0;
		d2 = d1 * ((i - b1) + 1.0);
		if ((*np + d2) > (n_tokens - 10)) {
			break;	/* too big to expand, do nothing */
		}
		j = d1;
		k = d2;
		blt(&equation[e1+k], &equation[e1], (*np - e1) * sizeof(token_type));
		*np += k;
		equation[i+1].token.constant -= d1;
		k = e1;
		while (j-- > 0) {
			equation[k].level = level;
			equation[k].kind = OPERATOR;
			equation[k].token.operatr = TIMES;
			blt(&equation[k+1], &equation[b1], (i - b1) * sizeof(token_type));
			k += (i - b1) + 1;
		}
		if (equation[i+1].token.constant == 1.0) {
			blt(&equation[i], &equation[e1], (*np - e1) * sizeof(token_type));
			*np -= (e1 - i);
		} else {
			for (j = b1; j < e1; j++)
				equation[j].level++;
		}
		return true;
	}
	return false;
}

static inline int
usp_sub(equation, np, i)
token_type	*equation;
int		*np, i;
{
	int	level;
	int	j;

	level = equation[i].level;
	for (j = i - 2;; j -= 2) {
		if (j < 0)
			return false;
		if (equation[j].level < level) {
			if (equation[j].level == (level - 1) && equation[j].token.operatr == DIVIDE) {
				break;
			} else
				return false;
		}
	}
	if ((*np + 2) > n_tokens) {
		error_huge();
	}
	equation[j].token.operatr = TIMES;
	for (j = i + 1;; j++) {
		if (j >= *np || equation[j].level < level)
			break;
		equation[j].level++;
	}
	i++;
	blt(&equation[i+2], &equation[i], (*np - i) * sizeof(token_type));
	*np += 2;
	equation[i].level = level + 1;
	equation[i].kind = CONSTANT;
	equation[i].token.constant = -1.0;
	i++;
	equation[i].level = level + 1;
	equation[i].kind = OPERATOR;
	equation[i].token.operatr = TIMES;
	return true;
}

/*
 * Convert a/(x^y) to a*x^(-1*y).
 * If y is a constant, don't do.
 *
 * Return true if equation side is modified.
 */
int
unsimp_power(equation, np)
token_type	*equation;
int		*np;
{
	int	modified = false;
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			if (equation[i].level == equation[i+1].level
			    && equation[i+1].kind == CONSTANT)
				continue;
			modified |= usp_sub(equation, np, i);
		}
	}
	return modified;
}

#if	false
/*
 * Convert a/(x^y) to a*(1/x)^y.
 * Return true if equation side is modified.
 */
int
unsimp2_power(equation, np)
token_type	*equation;
int		*np;
{
	int	modified = false;
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			modified |= usp2_sub(equation, np, i);
		}
	}
	return modified;
}

int
usp2_sub(equation, np, i)
token_type	*equation;
int		*np, i;
{
	int	level;
	int	j, k;

	level = equation[i].level;
	if (equation[i+1].level == level && equation[i+1].kind == CONSTANT) {
		return false;
	}
	for (j = i - 2;; j -= 2) {
		if (j < 0)
			return false;
		if (equation[j].level < level) {
			if (equation[j].level == (level - 1) && equation[j].token.operatr == DIVIDE) {
				break;
			} else
				return false;
		}
	}
	if ((*np + 2) > n_tokens) {
		error_huge();
	}
	equation[j].token.operatr = TIMES;
	j++;
	for (k = j; k < i; k++) {
		equation[k].level++;
	}
	blt(&equation[j+2], &equation[j], (*np - j) * sizeof(token_type));
	*np += 2;
	equation[j].level = level + 1;
	equation[j].kind = CONSTANT;
	equation[j].token.constant = 1.0;
	j++;
	equation[j].level = level + 1;
	equation[j].kind = OPERATOR;
	equation[j].token.operatr = DIVIDE;
	return true;
}
#endif

/*
 * Convert anything times a negative constant to a positive constant
 * divided by -1.
 */
uf_neg_help(equation, np)
token_type	*equation;
int		*np;
{
	int	i;
	int	level;

	for (i = 0; i < *np - 1; i += 2) {
		if (equation[i].kind == CONSTANT && equation[i].token.constant < 0.0) {
			level = equation[i].level;
			if (equation[i+1].level == level) {
				switch (equation[i+1].token.operatr) {
				case TIMES:
				case DIVIDE:
					if ((*np + 2) > n_tokens) {
						error_huge();
					}
					blt(&equation[i+3], &equation[i+1], (*np - (i + 1)) * sizeof(token_type));
					*np += 2;
					equation[i].token.constant = -equation[i].token.constant;
					i++;
					equation[i].level = level;
					equation[i].kind = OPERATOR;
					equation[i].token.operatr = DIVIDE;
					i++;
					equation[i].level = level;
					equation[i].kind = CONSTANT;
					equation[i].token.constant = -1.0;
					break;
				}
			}
		}
	}
}

/*
 * Convert 1/(k1^k2) to 1/(k1^(k2-1))/k1 if k1 is integer,
 * otherwise 1*((1/k1)^k2).
 *
 * Return true if equation side was modified.
 */
int
patch_root_div(equation, np)
token_type	*equation;
int		*np;
{
	int	i;
	int	level;
	int	modified = false;

	for (i = 1; i < *np - 2; i += 2) {
		if (equation[i].token.operatr == DIVIDE) {
			level = equation[i].level + 1;
			if (equation[i+2].token.operatr == POWER
			    && equation[i+2].level == level
			    && equation[i+1].kind == CONSTANT
			    && equation[i+3].level == level
			    && equation[i+3].kind == CONSTANT) {
				if (fmod(equation[i+1].token.constant, 1.0) == 0.0) {
					if (!isfinite(equation[i+3].token.constant)
					    || equation[i+3].token.constant <= 0.0
					    || equation[i+3].token.constant >= 1.0) {
						continue;
					}
					if (*np + 2 > n_tokens)
						error_huge();
					equation[i+3].token.constant -= 1.0;
					blt(&equation[i+2], &equation[i], (*np - i) * sizeof(token_type));
					*np += 2;
					i++;
					equation[i].level = level - 1;
					equation[i].kind = CONSTANT;
					equation[i].token.constant = equation[i+2].token.constant;
					i++;
				} else {
					equation[i].token.operatr = TIMES;
					equation[i+1].token.constant = 1.0 / equation[i+1].token.constant;
				}
				modified = true;
			}
		}
	}
	return modified;
}
