/*
 * Mathomatic differentiation routines and commands.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

static int	d_recurse();

/*
 * Compute the derivative of an equation side, with respect to variable "v",
 * using the fast, rule based transform method.
 * The result must be simplified by the caller.
 *
 * Return true if successful.
 */
int
differentiate(equation, np, v)
token_type	*equation;	/* pointer to source and destination equation side */
int		*np;		/* pointer to the length of the equation side */
long		v;		/* differentiation variable */
{
	int	i;

	organize(equation, np);
/* First put every times and divide on a level by itself. */
	for (i = 1; i < *np; i += 2) {
		switch (equation[i].token.operatr) {
		case TIMES:
		case DIVIDE:
			binary_parenthesize(equation, *np, i);
		}
	}
	return d_recurse(equation, np, 0, 1, v);
}

/*
 * Recursive differentiation routine.
 *
 * Symbolically differentiate expression in "equation"
 * (which is a standard equation side) starting at "loc".
 * The current level of parentheses is "level" and
 * do the differentation with respect to variable "v".
 *
 * Return "true" if successful, "false" if it is beyond this program's
 * capabilities or an error was encountered.
 */
static int
d_recurse(equation, np, loc, level, v)
token_type	*equation;
int		*np, loc, level;
long		v;
{
	int		i, j;
	int		n;
	int		op;
	int		oploc, endloc;
	complexs	c;

	if (equation[loc].level < level) {
/* First differentiate if it is a single variable or constant. */
/* If it is the specified variable, change it to the constant 1, */
/* otherwise change it to the constant 0. */
		if (equation[loc].kind == VARIABLE
		    && ((v == MATCH_ANY && (equation[loc].token.variable & VAR_MASK) > SIGN)
		    || equation[loc].token.variable == v)) {
			equation[loc].kind = CONSTANT;
			equation[loc].token.constant = 1.0;
		} else {
			equation[loc].kind = CONSTANT;
			equation[loc].token.constant = 0.0;
		}
		return true;
	}
	for (op = 0, oploc = endloc = loc + 1; endloc < *np && equation[endloc].level >= level; endloc += 2) {
		if (equation[endloc].level == level) {
			switch (op) {
			case 0:
			case PLUS:
			case MINUS:
				break;
			default:
/* Oops.  More than one operator on the same level in this expression. */
				error("Error in d_recurse().");
				return false;
			}
			op = equation[endloc].token.operatr;
			oploc = endloc;
		}
	}
	switch (op) {
	case 0:
	case PLUS:
	case MINUS:
		break;
	case TIMES:
		goto d_times;
	case DIVIDE:
		goto d_divide;
	case POWER:
		goto d_power;
	default:
/* Differentiate an unsupported operator. */
/* This is possible if the expression doesn't contain the specified variable. */
/* In that case, the expression is replaced with "0", otherwise return false. */
		for (i = loc; i < endloc; i += 2) {
			if (equation[i].kind == VARIABLE
			    && ((v == MATCH_ANY && (equation[i].token.variable & VAR_MASK) > SIGN)
			    || equation[i].token.variable == v)) {
				return false;
			}
		}
		blt(&equation[loc+1], &equation[endloc], (*np - endloc) * sizeof(token_type));
		*np -= (endloc - (loc + 1));
		equation[loc].level = level;
		equation[loc].kind = CONSTANT;
		equation[loc].token.constant = 0.0;
		return true;
	}
/* Differentiate PLUS and MINUS operators. */
/* Use addition rule: d(u+v) = d(u) + d(v), */
/* where "d()" is the derivative function */
/* and "u" and "v" are expressions. */
	for (i = loc; i < *np && equation[i].level >= level;) {
		if (equation[i].kind != OPERATOR) {
			if (!d_recurse(equation, np, i, level + 1, v))
				return false;
			i++;
			for (; i < *np && equation[i].level > level; i += 2)
				;
			continue;
		}
		i++;
	}
	return true;
d_times:
/* Differentiate TIMES operator. */
/* Use product rule: d(u*v) = u*d(v) + v*d(u). */
	if (*np + 1 + (endloc - loc) > n_tokens) {
		error_huge();
	}
	for (i = loc; i < endloc; i++)
		equation[i].level++;
	blt(&equation[endloc+1], &equation[loc], (*np - loc) * sizeof(token_type));
	*np += 1 + (endloc - loc);
	equation[endloc].level = level;
	equation[endloc].kind = OPERATOR;
	equation[endloc].token.operatr = PLUS;
	if (!d_recurse(equation, np, endloc + (oploc - loc) + 2, level + 2, v))
		return false;
	return(d_recurse(equation, np, loc, level + 2, v));
d_divide:
/* Differentiate DIVIDE operator. */
/* Use quotient rule: d(u/v) = (v*d(u) - u*d(v))/v^2. */
	if (*np + 3 + (endloc - loc) + (endloc - oploc) > n_tokens) {
		error_huge();
	}
	for (i = loc; i < endloc; i++)
		equation[i].level += 2;
	equation[oploc].token.operatr = TIMES;
	j = 1 + (endloc - loc);
	blt(&equation[endloc+1], &equation[loc], (*np - loc) * sizeof(token_type));
	*np += j;
	equation[endloc].level = level + 1;
	equation[endloc].kind = OPERATOR;
	equation[endloc].token.operatr = MINUS;
	j += endloc;
	blt(&equation[j+2+(endloc-oploc)], &equation[j], (*np - j) * sizeof(token_type));
	*np += 2 + (endloc - oploc);
	equation[j].level = level;
	equation[j].kind = OPERATOR;
	equation[j].token.operatr = DIVIDE;
	blt(&equation[j+1], &equation[oploc+1], (endloc - (oploc + 1)) * sizeof(token_type));
	j += endloc - oploc;
	equation[j].level = level + 1;
	equation[j].kind = OPERATOR;
	equation[j].token.operatr = POWER;
	j++;
	equation[j].level = level + 1;
	equation[j].kind = CONSTANT;
	equation[j].token.constant = 2.0;
	if (!d_recurse(equation, np, endloc + (oploc - loc) + 2, level + 3, v))
		return false;
	return(d_recurse(equation, np, loc, level + 3, v));
d_power:
/* Differentiate POWER operator. */
/* Since we don't have symbolic logarithms, do all we can without them. */
	for (i = oploc; i < endloc; i++) {
		if (equation[i].kind == VARIABLE
		    && ((v == MATCH_ANY && (equation[i].token.variable & VAR_MASK) > SIGN)
		    || equation[i].token.variable == v)) {
			if (parse_complex(&equation[loc], oploc - loc, &c)) {
				c = complex_log(c);
				n = (endloc - oploc) + 6;
				if (*np + n > n_tokens) {
					error_huge();
				}
				blt(&equation[endloc+n], &equation[endloc], (*np - endloc) * sizeof(token_type));
				*np += n;
				n = endloc;
				equation[n].level = level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
				equation[n].level = level + 1;
				equation[n].kind = CONSTANT;
				equation[n].token.constant = c.re;
				n++;
				equation[n].level = level + 1;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = PLUS;
				n++;
				equation[n].level = level + 2;
				equation[n].kind = CONSTANT;
				equation[n].token.constant = c.im;
				n++;
				equation[n].level = level + 2;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
				equation[n].level = level + 2;
				equation[n].kind = VARIABLE;
				equation[n].token.variable = IMAGINARY;
				n++;
				equation[n].level = level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
				blt(&equation[n], &equation[oploc+1], (endloc - (oploc + 1)) * sizeof(token_type));
				for (i = loc; i < endloc; i++) {
					equation[i].level++;
				}
				return(d_recurse(equation, np, n, level + 1, v));
			}
			return false;
		}
	}
	blt(scratch, &equation[oploc+1], (endloc - (oploc + 1)) * sizeof(token_type));
	n = endloc - (oploc + 1);
	scratch[n].level = level;
	scratch[n].kind = OPERATOR;
	scratch[n].token.operatr = TIMES;
	n++;
	if (n + (endloc - loc) + 2 > n_tokens) {
		error_huge();
	}
	blt(&scratch[n], &equation[loc], (endloc - loc) * sizeof(token_type));
	i = n;
	n += oploc + 1 - loc;
	for (; i < n; i++)
		scratch[i].level++;
	n += endloc - (oploc + 1);
	for (; i < n; i++)
		scratch[i].level += 2;
	scratch[n].level = level + 2;
	scratch[n].kind = OPERATOR;
	scratch[n].token.operatr = MINUS;
	n++;
	scratch[n].level = level + 2;
	scratch[n].kind = CONSTANT;
	scratch[n].token.constant = 1.0;
	n++;
	if (n + (oploc - loc) + 1 > n_tokens) {
		error_huge();
	}
	scratch[n].level = level;
	scratch[n].kind = OPERATOR;
	scratch[n].token.operatr = TIMES;
	n++;
	j = n;
	blt(&scratch[n], &equation[loc], (oploc - loc) * sizeof(token_type));
	n += oploc - loc;
	if (*np - (endloc - loc) + n > n_tokens) {
		error_huge();
	}
	blt(&equation[loc+n], &equation[endloc], (*np - endloc) * sizeof(token_type));
	*np += loc + n - endloc;
	blt(&equation[loc], scratch, n * sizeof(token_type));
	return(d_recurse(equation, np, loc + j, level + 1, v));
}

/*
 * The derivative command.
 */
int
derivative_cmd(cp)
char	*cp;
{
	int		i;
	long		v = 0;
	long		l1, order = 1;
	token_type	*source, *dest;
	int		n1, *nps, *np;

	if (current_not_defined()) {
		return false;
	}
	i = next_espace();
	if (n_rhs[cur_equation]) {
		source = rhs[cur_equation];
		nps = &n_rhs[cur_equation];
		dest = rhs[i];
		np = &n_rhs[i];
	} else {
		source = lhs[cur_equation];
		nps = &n_lhs[cur_equation];
		dest = lhs[i];
		np = &n_lhs[i];
	}
/* parse the command line or prompt: */
	if (*cp) {
		if (is_all(cp)) {
			cp = skip_param(cp);
			v = MATCH_ANY;
		} else {
			if (!isdigit(*cp)) {
				cp = parse_var2(&v, cp);
				if (cp == NULL) {
					return false;
				}
			}
		}
		if (*cp) {
			order = decstrtol(cp, &cp);
		}
		if (*cp || order <= 0) {
			error(_("The order must be a positive integer."));
			return false;
		}
	}
	if (no_vars(source, *nps, &v)) {
		error(_("Current expression contains no variables, result would be zero."));
		return false;
	}
	if (v == 0) {
		if (!prompt_var(&v)) {
			return false;
		}
	}
	if (v != MATCH_ANY && !found_var(source, *nps, v)) {
		error(_("Variable not found, result would be zero."));
		return false;
	}
#if	!SILENT
	list_var(v, 0);
	printf(_("Differentiating with respect to (%s) and simplifying"), var_str);
	if (n_rhs[cur_equation]) {
		printf(_(" the RHS...\n"));
	} else {
		printf("...\n");
	}
#endif
	blt(dest, source, *nps * sizeof(token_type));
	n1 = *nps;
/* do the actual differentiating and simplifying: */
	for (l1 = 0; l1 < order; l1++) {
		if (!differentiate(dest, &n1, v)) {
			error(_("Differentiation failed."));
			return false;
		}
		simpa_side(dest, &n1, true);
	}
	if (n_rhs[cur_equation]) {
		blt(lhs[i], lhs[cur_equation], n_lhs[cur_equation] * sizeof(token_type));
		n_lhs[i] = n_lhs[cur_equation];
	}
	*np = n1;
	cur_equation = i;
	return return_result(cur_equation);
}

/*
 * The extrema command.
 */
int
extrema_cmd(cp)
char	*cp;
{
	int		i;
	long		v = 0;
	long		l1, order = 1;
	token_type	want;
	token_type	*source;
	int		n;

	if (current_not_defined()) {
		return false;
	}
	if (n_rhs[cur_equation]) {
		if (!solved_equation(cur_equation)) {
			error(_("The current equation is not solved for a variable."));
			return false;
		}
		source = rhs[cur_equation];
		n = n_rhs[cur_equation];
	} else {
		source = lhs[cur_equation];
		n = n_lhs[cur_equation];
	}
	if (*cp) {
		if (!isdigit(*cp)) {
			cp = parse_var2(&v, cp);
			if (cp == NULL) {
				return false;
			}
		}
		if (*cp) {
			order = decstrtol(cp, &cp);
		}
		if (*cp || order <= 0) {
			error(_("The order must be a positive integer."));
			return false;
		}
	}
	if (no_vars(source, n, &v)) {
		error(_("Current expression contains no variables, there are no extrema."));
		return false;
	}
	if (v == 0) {
		if (!prompt_var(&v)) {
			return false;
		}
	}
	if (!found_var(source, n, v)) {
		error(_("Variable not found."));
		return false;
	}
	i = next_espace();
	blt(rhs[i], source, n * sizeof(token_type));
/* take derivatives with respect to the specified variable and simplify: */
	for (l1 = 0; l1 < order; l1++) {
		if (!differentiate(rhs[i], &n, v)) {
			error(_("Differentiation failed."));
			return false;
		}
		simpa_side(rhs[i], &n, true);
	}
	if (!found_var(rhs[i], n, v)) {
		error(_("There are no solutions."));
		return false;
	}
	n_rhs[i] = n;
/* set equal to zero: */
	n_lhs[i] = 1;
	lhs[i][0] = zero_token;
	cur_equation = i;
/* lastly, solve for the specified variable and simplify: */
	want.level = 1;
	want.kind = VARIABLE;
	want.token.variable = v;
	if (solve_sub(&want, 1, lhs[i], &n_lhs[i], rhs[i], &n_rhs[i]) <= 0) {
		error(_("Solve failed."));
		return false;
	}
	simpa_side(rhs[i], &n_rhs[i], false);
	return return_result(cur_equation);
}

/*
 * The taylor command.
 */
int
taylor_cmd(cp)
char	*cp;
{
	long		v = 0;
	int		i, j, k, i1;
	int		level;
	long		l1, n, order = -1L;
	double		d;
	char		*cp_start, *cp1, buf[MAX_CMD_LEN];
	int		our;
	int		our_nlhs, our_nrhs;
	token_type	*ep, *source, *dest;
	int		n1, *nps, *np;

	cp_start = cp;
	if (current_not_defined()) {
		return false;
	}
	i = next_espace();
	blt(lhs[i], lhs[cur_equation], n_lhs[cur_equation] * sizeof(token_type));
	n_lhs[i] = n_lhs[cur_equation];
	n_rhs[i] = 0;
        our = alloc_next_espace();
	n_lhs[i] = 0;
        if (our < 0) {
                error(_("Out of free equation spaces."));
		return false;
	}
	if (n_rhs[cur_equation]) {
		source = rhs[cur_equation];
		nps = &n_rhs[cur_equation];
		dest = rhs[i];
		np = &n_rhs[i];
	} else {
		source = lhs[cur_equation];
		nps = &n_lhs[cur_equation];
		dest = lhs[i];
		np = &n_lhs[i];
	}
	if (*cp && !isdigit(*cp)) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	}
	if (*cp) {
		order = decstrtol(cp, &cp1);
		if (cp1 != skip_param(cp) || order < 0) {
			error(_("Positive integer required for order."));
			return false;
		}
		cp = cp1;
	}
	if (no_vars(source, *nps, &v)) {
		error(_("Current expression contains no variables."));
		return false;
	}
	if (v == 0) {
		if (!prompt_var(&v)) {
			return false;
		}
	}
	if (!found_var(source, *nps, v)) {
		error(_("Variable not found."));
		return false;
	}
	blt(rhs[our], source, *nps * sizeof(token_type));
	our_nrhs = *nps;
	if (!differentiate(rhs[our], &our_nrhs, v)) {
		error(_("Differentiation failed."));
		return false;
	}
	if (*cp) {
		input_column += (cp - cp_start);
		if (!case_sensitive_flag) {
			str_tolower(cp);
		}
		if ((cp = parse_section(lhs[our], &our_nlhs, cp)) == NULL || our_nlhs <= 0) {
			return false;
		}
		if (extra_characters(cp))
			return false;
	} else {
#if	!SILENT
		list_var(v, 0);
		printf(_("Taylor series expansion about %s = point.\n"), var_str);
#endif
		my_strlcpy(prompt_str, _("Enter point: "), sizeof(prompt_str));
		if (!get_expr(lhs[our], &our_nlhs)) {
			return false;
		}
	}
	if (order < 0) {
		my_strlcpy(prompt_str, _("Enter order (number of derivatives to take): "), sizeof(prompt_str));
		if ((cp1 = get_string(buf, sizeof(buf))) == NULL)
			return false;
		if (*cp1) {
			order = decstrtol(cp1, &cp);
			if (*cp || order < 0) {
				error(_("Positive integer required for order."));
				return false;
			}
		} else {
			printf(_("Derivatives will be taken until they reach zero...\n"));
			order = LONG_MAX - 1L;
		}
	}
	n = 0;
	i1 = 0;
	blt(dest, source, *nps * sizeof(token_type));
	n1 = *nps;
loop_again:
	for (k = i1; k < n1; k += 2) {
		if (dest[k].kind == VARIABLE && dest[k].token.variable == v) {
			level = dest[k].level;
			if ((n1 + our_nlhs - 1) > n_tokens)
				error_huge();
			blt(&dest[k+our_nlhs], &dest[k+1], (n1 - (k + 1)) * sizeof(token_type));
			n1 += our_nlhs - 1;
			j = k;
			blt(&dest[k], lhs[our], our_nlhs * sizeof(token_type));
			k += our_nlhs;
			for (; j < k; j++)
				dest[j].level += level;
			k--;
		}
	}
	if ((n1 + our_nlhs + 7) > n_tokens)
		error_huge();
	for (k = i1; k < n1; k++)
		dest[k].level++;
	ep = &dest[n1];
	ep->level = 1;
	ep->kind = OPERATOR;
	ep->token.operatr = TIMES;
	ep++;
	ep->level = 3;
	ep->kind = VARIABLE;
	ep->token.variable = v;
	ep++;
	ep->level = 3;
	ep->kind = OPERATOR;
	ep->token.operatr = MINUS;
	n1 += 3;
	j = n1;
	blt(&dest[n1], lhs[our], our_nlhs * sizeof(token_type));
	n1 += our_nlhs;
	for (; j < n1; j++)
		dest[j].level += 3;
	ep = &dest[n1];
	ep->level = 2;
	ep->kind = OPERATOR;
	ep->token.operatr = POWER;
	ep++;
	ep->level = 2;
	ep->kind = CONSTANT;
	ep->token.constant = n;
	ep++;
	ep->level = 1;
	ep->kind = OPERATOR;
	ep->token.operatr = DIVIDE;
	ep++;
	for (d = 1.0, l1 = 2; l1 <= n; l1++)
		d *= l1;
	ep->level = 1;
	ep->kind = CONSTANT;
	ep->token.constant = d;
	n1 += 4;
	for (; i1 < n1; i1++)
		dest[i1].level++;
	simp_loop(dest, &n1);
/*	side_debug(1, dest, n1); */
	if (n < order) {
		if (n > 0) {
			if (!differentiate(rhs[our], &our_nrhs, v)) {
				error(_("Differentiation failed."));
				return false;
			}
		}
		simpa_side(rhs[our], &our_nrhs, true);
		if (our_nrhs != 1 || rhs[our][0].kind != CONSTANT || rhs[our][0].token.constant != 0.0) {
			i1 = n1;
			if ((i1 + 1 + our_nrhs) > n_tokens)
				error_huge();
			for (j = 0; j < i1; j++)
				dest[j].level++;
			dest[i1].level = 1;
			dest[i1].kind = OPERATOR;
			dest[i1].token.operatr = PLUS;
			i1++;
			blt(&dest[i1], rhs[our], our_nrhs * sizeof(token_type));
			n1 = i1 + our_nrhs;
			n++;
			goto loop_again;
		}
	}
#if	!SILENT
	printf(_("%ld derivative%s applied.\n"), n, (n == 1) ? "" : "s");
#endif
	if (n_rhs[cur_equation]) {
		n_lhs[i] = n_lhs[cur_equation];
	}
	*np = n1;
	cur_equation = i;
	return return_result(cur_equation);
}

/*
 * The limit command.
 */
int
limit_cmd(cp)
char	*cp;
{
	int		i, j, k;
	long		v = 0;
	token_type	solved_v, want;
	char		*cp_start;
	int		infinity_flag, found_den, complex_den, num, den;
	int		level;

	cp_start = cp;
	if (current_not_defined()) {
		return false;
	}
	if (n_rhs[cur_equation] == 0) {
/* make expression into an equation: */
		blt(rhs[cur_equation], lhs[cur_equation], n_lhs[cur_equation] * sizeof(token_type));
		n_rhs[cur_equation] = n_lhs[cur_equation];
		n_lhs[cur_equation] = 1;
		lhs[cur_equation][0].level = 1;
		lhs[cur_equation][0].kind = VARIABLE;
		parse_var(&lhs[cur_equation][0].token.variable, "answer");
	}
	if (!solved_equation(cur_equation)) {
		error(_("The current equation is not solved for a variable."));
		return false;
	}
	solved_v = lhs[cur_equation][0];
/* parse the command line or prompt: */
	if (*cp) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	}
	if (no_vars(rhs[cur_equation], n_rhs[cur_equation], &v)) {
		error(_("Current expression contains no variables."));
		return false;
	}
	if (v == 0) {
		if (!prompt_var(&v)) {
			return false;
		}
	}
	if (!found_var(rhs[cur_equation], n_rhs[cur_equation], v)) {
		error(_("Variable not found."));
		return false;
	}
	if (*cp) {
		input_column += (cp - cp_start);
		if (!case_sensitive_flag) {
			str_tolower(cp);
		}
		if ((cp = parse_section(tes, &n_tes, cp)) == NULL || n_tes <= 0) {
			return false;
		}
	} else {
		list_var(v, 0);
		snprintf(prompt_str, sizeof(prompt_str), _("as (%s) goes to: "), var_str);
		if (!get_expr(tes, &n_tes)) {
			return false;
		}
	}
	if (extra_characters(cp))
		return false;
/* copy to a new equation space and work on the copy: */
	i = next_espace();
	copy_espace(cur_equation, i);
	cur_equation = i;
/* see if the limit expression contains infinity: */
	simp_loop(tes, &n_tes);
	infinity_flag = exp_contains_infinity(tes, n_tes);
/* first fully simplify the current equation and group denominators together: */
	simpa_side(rhs[cur_equation], &n_rhs[cur_equation], false);
	group_proc(rhs[cur_equation], &n_rhs[cur_equation]);
	found_den = complex_den = false;
	for (j = 1; j < n_rhs[cur_equation]; j += 2) {
		switch (rhs[cur_equation][j].token.operatr) {
		case DIVIDE:
		case MODULUS:
			num = den = false;
			level = rhs[cur_equation][j].level;
			for (k = j + 2;; k += 2) {
				if (rhs[cur_equation][k-1].kind == VARIABLE
				    && rhs[cur_equation][k-1].token.variable == v) {
/* the limit variable was found in a denominator */
					den = true;
					found_den = true;
				}
				if (k >= n_rhs[cur_equation] || rhs[cur_equation][k].level <= level) {
					break;
				}
				switch (rhs[cur_equation][k].token.operatr) {
				case DIVIDE:
				case MODULUS:
/* a denominator contains a fraction */
					complex_den = true;
					break;
				}
			}
			for (k = j - 1; k >= 0 && rhs[cur_equation][k].level >= level; k--) {
				if (rhs[cur_equation][k].kind == VARIABLE
				    && rhs[cur_equation][k].token.variable == v) {
/* the limit variable was found in a numerator */
					num = true;
				}
			}
			if (num && den) {
				complex_den = true;
			}
			break;
		}
	}
	if (infinity_flag ? (!complex_den && found_den) : (!found_den)) {
/* in this case, just substitute the limit variable with the limit expression and simplify: */
		subst_var_with_exp(rhs[cur_equation], &n_rhs[cur_equation], tes, n_tes, v);
		simpa_side(rhs[cur_equation], &n_rhs[cur_equation], false);
		return return_result(cur_equation);
	}
	debug_string(1, "Taking the limit by solving...");
	if (n_tes == 1 && tes[0].kind == CONSTANT && tes[0].token.constant == HUGE_VAL) {
/* To take the limit to positive infinity, */
/* replace infinity with zero and replace the limit variable with its reciprocal: */
		n_tes = 1;
		tes[0] = zero_token;
		tlhs[0] = one_token;
		tlhs[1].level = 1;
		tlhs[1].kind = OPERATOR;
		tlhs[1].token.operatr = DIVIDE;
		tlhs[2].level = 1;
		tlhs[2].kind = VARIABLE;
		tlhs[2].token.variable = v;
		n_tlhs = 3;
		subst_var_with_exp(rhs[cur_equation], &n_rhs[cur_equation], tlhs, n_tlhs, v);
	}
/* General limit taking, solve for the limit variable: */
	want.level = 1;
	want.kind = VARIABLE;
	want.token.variable = v;
	if (solve_sub(&want, 1, lhs[cur_equation], &n_lhs[cur_equation], rhs[cur_equation], &n_rhs[cur_equation]) <= 0) {
		error(_("Can't take the limit because solve failed."));
		return false;
	}
/* replace the limit variable (LHS) with the limit expression: */
	blt(lhs[cur_equation], tes, n_tes * sizeof(token_type));
	n_lhs[cur_equation] = n_tes;
/* symbolically simplify the RHS: */
	symb_flag = true;
	simpa_side(rhs[cur_equation], &n_rhs[cur_equation], false);
	symb_flag = false;
/* solve back for the original variable: */
	if (solve_sub(&solved_v, 1, lhs[cur_equation], &n_lhs[cur_equation], rhs[cur_equation], &n_rhs[cur_equation]) <= 0) {
		error(_("Can't take the limit because solve failed."));
		return false;
	}
/* symbolically simplify before returning the result: */
	symb_flag = true;
	simpa_side(rhs[cur_equation], &n_rhs[cur_equation], false);
	symb_flag = false;
	return return_result(cur_equation);
}
