/*
 * Algebraic manipulator commands.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

#include "includes.h"

#define	OPT_MIN_SIZE	7	/* Minimum size of repeated expressions to find in optimize command. */

static int	sum_product();
static int	find_more();
static int	opt_es();
static int	complex_func();
static int	elim_sub();
static int	edit_sub();

/* Global variables for the optimize command. */
static int	opt_en[N_EQUATIONS];
static int	last_temp_var = 0;

#if	!LIBRARY
/*
 * The version command.
 */
int
version_cmd(cp)
char	*cp;	/* the command line argument */
{
	if (extra_garbage(cp)) {
		/* if anything on the command line, fail */
		return false;
	}
	return version_report();
}

/*
 * Report version number and compile flags.
 */
int
version_report()
{
	fprintf(gfp, _("Mathomatic version %s\nCompile flags used: "), VERSION);
#if	__STDC__
	fprintf(gfp, "__STDC__ ");
#endif
#if	UNIX
	fprintf(gfp, "UNIX ");
#endif
#if	CYGWIN
	fprintf(gfp, "CYGWIN ");
#endif
#if	READLINE
	fprintf(gfp, "READLINE ");
#endif
#if	BASICS
	fprintf(gfp, "BASICS ");
#endif
#if	SILENT
	fprintf(gfp, "SILENT ");
#endif
#if	SECURE
	fprintf(gfp, "SECURE ");
#endif
#if	TIMEOUT_SECONDS
	fprintf(gfp, "TIMEOUT_SECONDS=%d ", TIMEOUT_SECONDS);
#endif
#if	I18N
	fprintf(gfp, "I18N ");
#endif
	fprintf(gfp, "\n");
	return true;
}
#endif

/*
 * The sum command.
 */
int
sum_cmd(cp)
char	*cp;
{
	return sum_product(cp, false);
}

/*
 * The product command.
 */
int
product_cmd(cp)
char	*cp;
{
	return sum_product(cp, true);
}

/*
 * Common function for the sum and product commands.
 */
static int
sum_product(cp, product_flag)
char	*cp;		/* the command line */
int	product_flag;	/* true for product, otherwise sum */
{
	int		i, j;
	long		v = 0;
	double		start, end, step = 1.0;
	int		result_en;
	int		n, ns;
	token_type	*dest, *source;
	int		count_down;		/* if true, count down, otherwise count up */
	char		*cp1, buf[MAX_CMD_LEN];

	if (current_not_defined()) {
		return false;
	}
	result_en = next_espace();
	if (n_rhs[cur_equation]) {
		ns = n_rhs[cur_equation];
		source = rhs[cur_equation];
		dest = rhs[result_en];
	} else {
		ns = n_lhs[cur_equation];
		source = lhs[cur_equation];
		dest = lhs[result_en];
	}
	if (*cp == '\0') {
		for (j = 0; j < ns; j += 2) {
			if (source[j].kind == VARIABLE) {
				if ((source[j].token.variable & VAR_MASK) <= SIGN)
					continue;
				if (v) {
					if (v != source[j].token.variable) {
						v = 0;
						break;
					}
				} else {
					v = source[j].token.variable;
				}
			}
		}
		if (v == 0) {
			if (!prompt_var(&v))
				return false;
		}
	} else {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	}
	if (!found_var(source, ns, v)) {
		error(_("Variable not found."));
		return false;
	}
	if (*cp) {
		cp1 = cp;
	} else {
		list_var(v, 0);
		snprintf(prompt_str, sizeof(prompt_str), "%s = ", var_str);
		if ((cp1 = get_string(buf, sizeof(buf))) == NULL)
			return false;
	}
	start = strtod(cp1, &cp);
	if (cp1 == cp || fabs(start) >= MAX_K_INTEGER) {
		error(_("Invalid number."));
		return false;
	}
	cp = skip_space(cp);
	if (*cp) {
		cp1 = cp;
	} else {
		my_strlcpy(prompt_str, _("To: "), sizeof(prompt_str));
		if ((cp1 = get_string(buf, sizeof(buf))) == NULL)
			return false;
	}
	end = strtod(cp1, &cp);
	if (cp1 == cp || fabs(end) >= MAX_K_INTEGER) {
		error(_("Invalid number."));
		return false;
	}
	cp = skip_space(cp);
	if (*cp) {
		cp1 = cp;
		step = fabs(strtod(cp1, &cp));
		if (step <= 0.0 || step >= MAX_K_INTEGER || cp1 == cp) {
			error(_("Invalid step."));
			return false;
		}
	}
	if (extra_garbage(cp))
		return false;
	count_down = (end < start);
	if (fmod(fabs(start - end) / step, 1.0)) {
		printf(_("Warning: end value not reached.\n"));
	}
	if (product_flag) {
		dest[0] = one_token;
	} else {
		dest[0] = zero_token;
	}
	n = 1;
	for (; count_down ? (start >= end) : (start <= end); count_down ? (start -= step) : (start += step)) {
		if (n + 1 + ns > n_tokens) {
			error_huge();
		}
		blt(tlhs, source, ns * sizeof(token_type));
		n_tlhs = ns;
		for (i = 0; i < n_tlhs; i += 2) {
			if (tlhs[i].kind == VARIABLE && tlhs[i].token.variable == v) {
				tlhs[i].kind = CONSTANT;
				tlhs[i].token.constant = start;
			}
		}
		for (i = 0; i < n_tlhs; i++) {
			tlhs[i].level++;
		}
		for (i = 0; i < n; i++) {
			dest[i].level++;
		}
		dest[n].kind = OPERATOR;
		dest[n].level = 1;
		if (product_flag) {
			dest[n].token.operatr = TIMES;
		} else {
			dest[n].token.operatr = PLUS;
		}
		n++;
		blt(&dest[n], tlhs, n_tlhs * sizeof(token_type));
		n += n_tlhs;
		calc_simp(dest, &n);
	}
	if (n_rhs[cur_equation]) {
		n_rhs[result_en] = n;
		blt(lhs[result_en], lhs[cur_equation], n_lhs[cur_equation] * sizeof(token_type));
		n_lhs[result_en] = n_lhs[cur_equation];
	} else {
		n_lhs[result_en] = n;
	}
	return_result(result_en);
	return true;
}

#if	!BASICS && !LIBRARY
/*
 * This function is for the "optimize" command.
 * It finds and substitutes all occurrences of "en"
 * in "equation".
 * It should be called repeatedly until it returns false.
 */
static int
find_more(equation, np, en)
token_type	*equation;
int		*np;		/* length of "equation" */
int		en;		/* equation number */
{
	int	i, j, k;
	int	level;
	int	diff_sign;
	int	found_se;
	long	v;

	found_se = true;
	for (level = 1; found_se; level++) {
		for (i = 1, found_se = false; i < *np; i = j + 2) {
			for (j = i; j < *np && equation[j].level > level; j += 2)
				;
			if (j == i) {
				continue;
			}
			found_se = true;
			k = i - 1;
			if ((j - k) >= OPT_MIN_SIZE && se_compare(&equation[k], j - k, rhs[en], n_rhs[en], &diff_sign)) {
				v = lhs[en][0].token.variable;
				if (diff_sign) {
					blt(&equation[i+2], &equation[j], (*np - j) * sizeof(token_type));
					*np -= (j - (i + 2));
					level++;
					equation[k].level = level;
					equation[k].kind = CONSTANT;
					equation[k].token.constant = -1.0;
					k++;
					equation[k].level = level;
					equation[k].kind = OPERATOR;
					equation[k].token.operatr = TIMES;
					k++;
				} else {
					blt(&equation[i], &equation[j], (*np - j) * sizeof(token_type));
					*np -= (j - i);
				}
				equation[k].level = level;
				equation[k].kind = VARIABLE;
				equation[k].token.variable = v;
				return true;
			}
		}
	}
	return false;
}

/*
 * This function is for the "optimize" command.
 * It finds and replaces all repeated expressions in
 * "equation" with temporary variables.
 * It also creates a new equation for each temporary variable.
 * It should be called repeatedly until it returns false.
 */
static int
opt_es(equation, np)
token_type	*equation;
int		*np;
{
	int	i, j, k;
	int	i1, i2, jj1, k1;
	int	level, level1;
	int	diff_sign;
	int	found_se, found_se1;
	long	v;

	found_se = true;
	for (level = 1; found_se; level++) {
		for (i = 1, found_se = false; i < *np; i = j + 2) {
			for (j = i; j < *np && equation[j].level > level; j += 2)
				;
			if (j == i) {
				continue;
			}
			found_se = true;
			k = i - 1;
			if ((j - k) < OPT_MIN_SIZE) {
				continue;
			}
			found_se1 = true;
			for (level1 = 1; found_se1; level1++) {
				for (i1 = 1, found_se1 = false; i1 < *np; i1 = jj1 + 2) {
					for (jj1 = i1; jj1 < *np && equation[jj1].level > level1; jj1 += 2) {
					}
					if (jj1 == i1) {
						continue;
					}
					found_se1 = true;
					if (i1 <= j)
						continue;
					k1 = i1 - 1;
					if ((jj1 - k1) >= OPT_MIN_SIZE
					    && se_compare(&equation[k], j - k, &equation[k1], jj1 - k1, &diff_sign)) {
						if (last_temp_var > MAX_SUBSCRIPT) {
							last_temp_var = 0;
						}
						v = V_TEMP + (((long) last_temp_var) << VAR_SHIFT);
						last_temp_var++;
						i2 = next_espace();
						lhs[i2][0].level = 1;
						lhs[i2][0].kind = VARIABLE;
						lhs[i2][0].token.variable = v;
						n_lhs[i2] = 1;
						blt(rhs[i2], &equation[k], (j - k) * sizeof(token_type));
						n_rhs[i2] = j - k;
						if (diff_sign) {
							blt(&equation[i1+2], &equation[jj1], (*np - jj1) * sizeof(token_type));
							*np -= (jj1 - (i1 + 2));
							level1++;
							equation[k1].level = level1;
							equation[k1].kind = CONSTANT;
							equation[k1].token.constant = -1.0;
							k1++;
							equation[k1].level = level1;
							equation[k1].kind = OPERATOR;
							equation[k1].token.operatr = TIMES;
							k1++;
						} else {
							blt(&equation[i1], &equation[jj1], (*np - jj1) * sizeof(token_type));
							*np -= (jj1 - i1);
						}
						equation[k1].level = level1;
						equation[k1].kind = VARIABLE;
						equation[k1].token.variable = v;
						blt(&equation[i], &equation[j], (*np - j) * sizeof(token_type));
						*np -= j - i;
						equation[k].level = level;
						equation[k].kind = VARIABLE;
						equation[k].token.variable = v;
						while (find_more(equation, np, i2))
							;
						simp_loop(rhs[i2], &n_rhs[i2]);
						simp_loop(equation, np);
						for (i = 0; opt_en[i] >= 0; i++)
							;
						opt_en[i] = i2;
						opt_en[i+1] = -1;
						return true;
					}
				}
			}
		}
	}
	return false;
}

/*
 * The optimize command.
 */
int
optimize_cmd(cp)
char	*cp;
{
	int	i, j, k;
	int	start, stop;
	int	i1;
	int	rv = false;
	int	flag;

	if (!get_range_eol(&cp, &start, &stop)) {
		return false;
	}
	for (i = start; i <= stop; i++) {
		if (n_lhs[i] == 0)
			continue;
		opt_en[0] = -1;
		simp_sub(i);
		flag = false;
		while (opt_es(lhs[i], &n_lhs[i])) {
			flag = true;
		}
		while (opt_es(rhs[i], &n_rhs[i])) {
			flag = true;
		}
		if (flag) {
			rv = true;
			for (i1 = 0; opt_en[i1] >= 0; i1++) {
				for (j = 0; opt_en[j] >= 0; j++) {
					for (k = j + 1; opt_en[k] >= 0; k++) {
						while (find_more(rhs[opt_en[k]], &n_rhs[opt_en[k]], opt_en[j]))
							;
						while (find_more(rhs[opt_en[j]], &n_rhs[opt_en[j]], opt_en[k]))
							;
					}
				}
				while (opt_es(rhs[opt_en[i1]], &n_rhs[opt_en[i1]]))
					;
			}
			for (j = 0; opt_en[j] >= 0; j++) {
				list_sub(opt_en[j]);
			}
			list_sub(i);
		}
	}
	if (!rv) {
		error(_("Unable to find any repeated expressions."));
	}
	return rv;
}
#endif

#if	READLINE
/*
 * The push command.
 */
int
push_cmd(cp)
char	*cp;
{
	int	i, j;

	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		push_en(i);
	}
	debug_string(0, _("Expression(s) pushed.  Press the UP key to access."));
	return true;
}

/*
 * Push an equation space into the readline history.
 */
push_en(en)
int	en;	/* equation space number */
{
	high_prec = true;
	add_history(list_equation(en, false));
	high_prec = false;
}
#endif

/*
 * The set command.
 */
int
set_cmd(cp)
char	*cp;
{
	char	buf[MAX_CMD_LEN];

	if (*cp == '\0') {
		fprintf(gfp, _("Options are set as follows:\n\n"));

		output_options();

		fprintf(gfp, "columns = %d\n", screen_columns);

#if	!SECURE
		if (getcwd(buf, sizeof(buf))) {
			fprintf(gfp, "directory = %s\n", buf);
		}
#endif
		return true;
	}
	return set_options(cp);
}

/*
 * Output the current set options in a format suitable for RC_FILE.
 */
output_options()
{
#if	!SILENT
	fprintf(gfp, "debug_level = %d\n", debug_level);
#endif

	if (!case_sensitive_flag) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "case_sensitive\n");

	if (!color_flag) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "color\n");

	if (!bold_colors) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "bold_colors\n");

	if (!groupall) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "display2d\n");

	if (!preserve_roots) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "preserve_roots\n");

	if (!true_modulus) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "true_modulus\n");

	if (!finance_option) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "finance\n");

	if (!factor_int_flag) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "factor_integers\n");
}

/*
 * Return true if "cp" points to a negative word.
 */
int
is_no(cp)
char	*cp;
{
	if (strcmp_tospace(cp, "no") == 0
	    || strcmp_tospace(cp, "off") == 0
	    || strcmp_tospace(cp, "false") == 0) {
		return true;
	}
	return false;
}

/*
 * Handle parsing of options for the set command.
 *
 * Return false if error.
 */
int
set_options(cp)
char	*cp;
{
	int	negate;

next_option:
	cp = skip_space(cp);
	if (*cp == '\0') {
		return true;
	}
	negate = is_no(cp);
	if (negate) {
		cp = skip_param(cp);
	}
#if	!SECURE
	if (strncasecmp(cp, "dir", 3) == 0) {
		cp = skip_param(cp);
		if (chdir(cp)) {
			error(_("Error changing directory."));
			return false;
		}
		return true;
	}
#endif
#if	!SILENT
	if (strncasecmp(cp, "debug", 5) == 0) {
		cp = skip_param(cp);
		if (negate) {
			debug_level = 0;
		} else {
			if (*cp == '\0') {
				error(_("Please specify a debug level number."));
				return false;
			}
			debug_level = decstrtol(cp, &cp);
		}
		goto next_option;
	}
#endif
	if (strncasecmp(cp, "columns", 7) == 0) {
		cp = skip_param(cp);
		if (negate) {
			screen_columns = 0;
		} else {
			if (*cp == '\0') {
				error(_("Please specify how wide the screen is in columns."));
				return false;
			}
			screen_columns = decstrtol(cp, &cp);
		}
		goto next_option;
	}
	if (strncasecmp(cp, "case", 4) == 0) {
		cp = skip_param(cp);
		case_sensitive_flag = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "display2d", 9) == 0) {
		cp = skip_param(cp);
		groupall = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "preserve", 8) == 0) {
		cp = skip_param(cp);
		preserve_roots = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "true", 4) == 0) {
		cp = skip_param(cp);
		true_modulus = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "color", 5) == 0) {
		cp = skip_param(cp);
		color_flag = !negate;
		cur_color = -1;
		goto next_option;
	}
	if (strncasecmp(cp, "bold", 4) == 0) {
		cp = skip_param(cp);
		bold_colors = !negate;
		cur_color = -1;
		goto next_option;
	}
	if (strncasecmp(cp, "finance", 7) == 0) {
		cp = skip_param(cp);
		finance_option = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "factor", 6) == 0) {
		cp = skip_param(cp);
		factor_int_flag = !negate;
		goto next_option;
	}
	error(_("Unknown set option."));
	return false;
}

/*
 * The pause command.
 */
int
pause_cmd(cp)
char	*cp;
{
	char	*cp1;
	char	buf[MAX_CMD_LEN];

	if (*cp == '\0') {
		cp = _("Please Press Enter");
	}
	snprintf(prompt_str, sizeof(prompt_str), " ==== %s ==== ", cp);
	if ((cp1 = get_string(buf, sizeof(buf))) == NULL) {
		return false;
	}
	if (strncasecmp(cp1, "quit", 4) == 0) {
		return false;
	}
	return true;
}

/*
 * The copy command.
 */
int
copy_cmd(cp)
char	*cp;
{
	int	i, j, k;
	int	i1;
	char	exists[N_EQUATIONS];

	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (i1 = 0; i1 < N_EQUATIONS; i1++) {
		exists[i1] = false;
	}
	for (i1 = i; i1 <= j; i1++) {
		if (n_lhs[i1] > 0) {
			exists[i1] = true;
		}
	}
	for (i1 = i; i1 <= j; i1++) {
		if (exists[i1]) {
			k = next_espace();
			blt(lhs[k], lhs[i1], n_lhs[i1] * sizeof(token_type));
			n_lhs[k] = n_lhs[i1];
			blt(rhs[k], rhs[i1], n_rhs[i1] * sizeof(token_type));
			n_rhs[k] = n_rhs[i1];
			return_result(k);
		}
	}
	return true;
}

/*
 * Common function for the imaginary and real commands.
 */
static int
complex_func(cp, imag_flag)
char	*cp;		/* the command line */
int	imag_flag;	/* if true, copy the imaginary part, otherwise copy the real part */
{
	int		i, j, k;
	int		beg;
	int		found_imag;
	int		has_imag, has_real;
	token_type	*source, *dest;
	int		n1, *nps, *np;
	long		v;

	if (current_not_defined()) {
		return false;
	}
	i = cur_equation;
	j = next_espace();
	if (n_rhs[i]) {
		source = rhs[i];
		nps = &n_rhs[i];
		dest = rhs[j];
		np = &n_rhs[j];
	} else {
		source = lhs[i];
		nps = &n_lhs[i];
		dest = lhs[j];
		np = &n_lhs[j];
	}
	v = IMAGINARY;
	if (*cp) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
		if (extra_garbage(cp))
			return false;
	}
	simp_loop(source, nps);
	uf_simp(source, nps);
	factorv(source, nps, v);
	partial_flag = false;
	uf_simp(source, nps);
	partial_flag = true;
	n1 = 1;
	dest[0] = zero_token;
	has_imag = false;
	has_real = false;
	k = 0;
	for (beg = k; beg < *nps; beg = k, k++) {
		found_imag = false;
		for (;; k++) {
			if (k >= *nps)
				break;
			if (source[k].level == 1 && source[k].kind == OPERATOR
			    && (source[k].token.operatr == PLUS || source[k].token.operatr == MINUS)) {
				break;
			}
			if (source[k].kind == VARIABLE && source[k].token.variable == v) {
				found_imag = true;
			}
		}
		if (found_imag)
			has_imag = true;
		else
			has_real = true;
		if (found_imag == imag_flag) {
			if (beg == 0) {
				n1 = 0;
			}
			blt(&dest[n1], &source[beg], (k - beg) * sizeof(token_type));
			n1 += (k - beg);
		}
	}
	if (!has_imag || !has_real) {
		error(_("Failed, expression is not a mix."));
		return false;
	}
	simp_divide(dest, &n1);
	if (n_rhs[i]) {
		blt(lhs[j], lhs[i], n_lhs[i] * sizeof(token_type));
		n_lhs[j] = n_lhs[i];
	}
	*np = n1;
	cur_equation = j;
	return_result(cur_equation);
	return true;
}

/*
 * The real command.
 */
int
real_cmd(cp)
char	*cp;
{
	return complex_func(cp, false);
}

/*
 * The imaginary command.
 */
int
imaginary_cmd(cp)
char	*cp;
{
	return complex_func(cp, true);
}

#if	!LIBRARY
/*
 * The tally command.
 */
int
tally_cmd(cp)
char	*cp;
{
	int	i;
	double	count;

	if (extra_garbage(cp))
		return false;
	trhs[0] = zero_token;
	n_trhs = 1;
	for (count = 0.0;; count++) {
		fprintf(gfp, _("Running total = "));
		list_proc(trhs, n_trhs, false);
		if (count > 0.0) {
			/* calculate and display the average */
			blt(tlhs, trhs, n_trhs * sizeof(token_type));
			n_tlhs = n_trhs;
			if ((n_tlhs + 2) > n_tokens) {
				error_huge();
			}
			for (i = 0; i < n_tlhs; i++) {
				tlhs[i].level++;
			}
			tlhs[n_tlhs].kind = OPERATOR;
			tlhs[n_tlhs].level = 1;
			tlhs[n_tlhs].token.operatr = DIVIDE;
			n_tlhs++;
			tlhs[n_tlhs].kind = CONSTANT;
			tlhs[n_tlhs].level = 1;
			tlhs[n_tlhs].token.constant = count;
			n_tlhs++;
			calc_simp(tlhs, &n_tlhs);
			fprintf(gfp, _(", average = "));
			list_proc(tlhs, n_tlhs, false);
		}
		fprintf(gfp, "\n");
		my_strlcpy(prompt_str, _("Enter value: "), sizeof(prompt_str));
		if (!get_expr(tlhs, &n_tlhs)) {
			break;
		}
		if ((n_trhs + 1 + n_tlhs) > n_tokens) {
			error_huge();
		}
		for (i = 0; i < n_tlhs; i++) {
			tlhs[i].level++;
		}
		for (i = 0; i < n_trhs; i++) {
			trhs[i].level++;
		}
		trhs[n_trhs].kind = OPERATOR;
		trhs[n_trhs].level = 1;
		trhs[n_trhs].token.operatr = PLUS;
		n_trhs++;
		blt(&trhs[n_trhs], tlhs, n_tlhs * sizeof(token_type));
		n_trhs += n_tlhs;
		calc_simp(trhs, &n_trhs);
	}
	return true;
}
#endif

#if	!LIBRARY
/*
 * The calculate command.
 *
 * Temporarily plug values into the RHS of the current equation,
 * unless there is no RHS, then use LHS.
 */
int
calculate_cmd(cp)
char	*cp;
{
	int		i, j, k;
	long		v, last_v, it_v = 0;
	long		counter;
	long		counter_max;
	sign_array_type	sa_mark;
	sign_array_type	sa_value;
	int		iterations = 1;
	int		n;
	token_type	*source;

	if (current_not_defined()) {
		return false;
	}
	i = cur_equation;
	if (n_rhs[i]) {
		source = rhs[i];
		n = n_rhs[i];
	} else {
		source = lhs[i];
		n = n_lhs[i];
	}
	if (*cp) {
		cp = parse_var2(&it_v, cp);
		if (cp == NULL) {
			return false;
		}
		iterations = decstrtol(cp, &cp);
		if (*cp || iterations <= 0) {
			error(_("Invalid number of iterations."));
			return false;
		}
	}
	if (it_v) {
		if (!found_var(source, n, it_v)) {
			error(_("Iteration variable not found."));
			return false;
		}
	}
	n_trhs = n;
	blt(trhs, source, n_trhs * sizeof(token_type));
	last_v = 0;
	for (;;) {
		v = -1;
		for (j = 0; j < n; j += 2) {
			if (source[j].kind == VARIABLE) {
				if (source[j].token.variable > last_v
				    && (v == -1 || source[j].token.variable < v))
					v = source[j].token.variable;
			}
		}
		if (v == -1)
			break;
		last_v = v;
		if ((v & VAR_MASK) <= SIGN || v == it_v) {
			continue;
		}
		list_var(v, 0);
		snprintf(prompt_str, sizeof(prompt_str), _("Enter %s: "), var_str);
		if (!get_expr(tlhs, &n_tlhs)) {
			continue;
		}
		for (j = 0; j < n_tlhs; j += 2)
			if (tlhs[j].kind == VARIABLE)
				tlhs[j].token.variable = -tlhs[j].token.variable;
		subst_var_with_exp(trhs, &n_trhs, tlhs, n_tlhs, v);
	}
	for (j = 0; j < n_trhs; j += 2)
		if (trhs[j].kind == VARIABLE && trhs[j].token.variable < 0)
			trhs[j].token.variable = -trhs[j].token.variable;
	if (it_v) {
		list_var(it_v, 0);
		snprintf(prompt_str, sizeof(prompt_str), _("Enter initial %s: "), var_str);
		while (!get_expr(scratch, &n))
			;
		blt(tlhs, trhs, n_trhs * sizeof(token_type));
		n_tlhs = n_trhs;
		for (j = 0; j < iterations; j++) {
			blt(trhs, tlhs, n_tlhs * sizeof(token_type));
			n_trhs = n_tlhs;
			subst_var_with_exp(trhs, &n_trhs, scratch, n, it_v);
			calc_simp(trhs, &n_trhs);
			blt(scratch, trhs, n_trhs * sizeof(token_type));
			n = n_trhs;
		}
	} else {
		simp_side(trhs, &n_trhs);
	}
	for (j = 0; j < ARR_CNT(sa_mark); j++)
		sa_mark[j] = false;
	for (j = 0; j < n_trhs; j += 2) {
		if (trhs[j].kind == VARIABLE && (trhs[j].token.variable & VAR_MASK) == SIGN) {
			sa_mark[(trhs[j].token.variable >> VAR_SHIFT) & SUBSCRIPT_MASK] = true;
		}
	}
	for (j = 0, k = 0; j < ARR_CNT(sa_mark); j++) {
		if (sa_mark[j]) {
			k++;
		}
	}
	counter_max = (1L << k) - 1;
	counter = 0;
	for (; counter <= counter_max; counter++) {
		blt(tlhs, trhs, n_trhs * sizeof(token_type));
		n_tlhs = n_trhs;
		for (j = 0, k = 0; j < ARR_CNT(sa_mark); j++) {
			if (sa_mark[j]) {
				sa_value[j] = (((1L << k) & counter) != 0);
				k++;
			}
		}
		for (j = 0; j < n_tlhs; j += 2) {
			if (tlhs[j].kind == VARIABLE && (tlhs[j].token.variable & VAR_MASK) == SIGN) {
				if (sa_value[(tlhs[j].token.variable >> VAR_SHIFT) & SUBSCRIPT_MASK]) {
					tlhs[j].kind = CONSTANT;
					tlhs[j].token.constant = -1.0;
				} else {
					tlhs[j].kind = CONSTANT;
					tlhs[j].token.constant = 1.0;
				}
			}
		}
		for (j = 0, k = false; j < ARR_CNT(sa_mark); j++) {
			if (sa_mark[j]) {
				if (k) {
					fprintf(gfp, ", ");
				} else {
					fprintf(gfp, _("Solution #%ld with "), counter + 1);
				}
				list_var((long) SIGN + (((long) j) << VAR_SHIFT), 0);
				fprintf(gfp, "%s = ", var_str);
				if (sa_value[j]) {
					fprintf(gfp, "-1");
				} else {
					fprintf(gfp, "1");
				}
				k = true;
			}
		}
		if (k)
			fprintf(gfp, ":\n");
		calc_simp(tlhs, &n_tlhs);
		if (factor_int_flag) {
			factor_int(tlhs, &n_tlhs);
		}
		fprintf(gfp, " ");
		if (n_rhs[i]) {
			list_proc(lhs[i], n_lhs[i], false);
			fprintf(gfp, " = ");
		}
		list_proc(tlhs, n_tlhs, false);
		fprintf(gfp, "\n");
	}
	return true;
}
#endif

/*
 * The clear command.
 */
int
clear_cmd(cp)
char	*cp;
{
	int	i, j;
	char	*cp1;

	do {
		cp1 = cp;
		if (is_all(cp)) {
			clear_all();
			return true;
		} else {
			if (!get_range(&cp, &i, &j)) {
				return false;
			}
			if (*cp && cp == cp1) {
				error(_("Invalid argument."));
				return false;
			}
			for (; i <= j; i++) {
				n_lhs[i] = 0;
				n_rhs[i] = 0;
			}
		}
	} while (*cp);
	return true;
}

static int
compare_rhs(i, j, diff_signp)
int	i, j;
int	*diff_signp;
{
	int	rv;

	rv = se_compare(rhs[i], n_rhs[i], rhs[i], n_rhs[i], diff_signp);
	if (!rv || *diff_signp) {
		error(_("Error in compare function or too many terms to compare!"));
		return false;
	}
	sign_flag = true;
	rv = se_compare(rhs[i], n_rhs[i], rhs[j], n_rhs[j], diff_signp);
	sign_flag = false;
	return rv;
}

static int
compare_lhs(i, j)
int	i, j;
{
	int	rv;
	int	diff_sign;

	rv = se_compare(lhs[i], n_lhs[i], lhs[i], n_lhs[i], &diff_sign);
	if (!rv || diff_sign) {
		error(_("Error in compare function or too many terms to compare!"));
		return false;
	}
	sign_flag = true;
	rv = se_compare(lhs[i], n_lhs[i], lhs[j], n_lhs[j], &diff_sign);
	sign_flag = false;
	return(rv && !diff_sign);
}

/*
 * The compare command.
 */
int
compare_cmd(cp)
char	*cp;
{
	int		i, j;
	int		diff_sign;
	int		already_solved;

	i = decstrtol(cp, &cp) - 1;
	if (not_defined(i)) {
		return false;
	}
	if (strcmp_tospace(cp, "with") == 0) {
		cp = skip_param(cp);
	}
	if ((j = get_default_en(cp)) < 0) {
		return false;
	}
	if (i == j) {
		error(_("Cannot compare an equation with itself."));
		return false;
	}
#if	!SILENT
	fprintf(gfp, _("Comparing #%d with #%d...\n"), i + 1, j + 1);
#endif
	if (n_rhs[i] == 0 || n_rhs[j] == 0) {
		if (n_rhs[i] == 0 && n_rhs[j] == 0) {
			simp_loop(lhs[i], &n_lhs[i]);
			simp_loop(lhs[j], &n_lhs[j]);
			if (compare_lhs(i, j)) {
				fprintf(gfp, _("Expressions are identical.\n"));
				return true;
			}
			debug_string(0, _("Simplifying both expressions..."));
			simpa_side(lhs[i], &n_lhs[i], true);
			simpa_side(lhs[j], &n_lhs[j], true);
#if	!SILENT
			if (debug_level >= 0) {
				list_sub(i);
				list_sub(j);
			}
#endif
			if (compare_lhs(i, j)) {
				fprintf(gfp, _("Expressions are identical.\n"));
				return true;
			}
			uf_simp(lhs[i], &n_lhs[i]);
			uf_simp(lhs[j], &n_lhs[j]);
			if (compare_lhs(i, j)) {
				fprintf(gfp, _("Expressions are identical.\n"));
				return true;
			}
			fprintf(gfp, _("Expressions may differ.\n"));
			return false;
		}
		error(_("Cannot compare an expression with an equation."));
		return false;
	}
	already_solved = (solved_equation(i) && solved_equation(j));
	if (already_solved) {
		simp_loop(rhs[i], &n_rhs[i]);
		simp_loop(rhs[j], &n_rhs[j]);
		if (compare_rhs(i, j, &diff_sign)) {
			goto times_neg1;
		}
		debug_string(0, _("Simplifying both equations..."));
		simpa_side(rhs[i], &n_rhs[i], true);
		simpa_side(rhs[j], &n_rhs[j], true);
#if	!SILENT
		if (debug_level >= 0) {
			list_sub(i);
			list_sub(j);
		}
#endif
		if (compare_rhs(i, j, &diff_sign)) {
			goto times_neg1;
		}
		uf_simp(rhs[i], &n_rhs[i]);
		uf_simp(rhs[j], &n_rhs[j]);
		if (compare_rhs(i, j, &diff_sign)) {
			goto times_neg1;
		}
	}
	debug_string(0, _("Solving both equations for zero and unfactoring..."));
	if (solve_sub(&zero_token, 1, lhs[i], &n_lhs[i], rhs[i], &n_rhs[i]) <= 0
	    || solve_sub(&zero_token, 1, lhs[j], &n_lhs[j], rhs[j], &n_rhs[j]) <= 0) {
		error(_("Can't solve for zero!"));
		return false;
	}
	uf_simp(rhs[i], &n_rhs[i]);
	uf_simp(rhs[j], &n_rhs[j]);
	if (compare_rhs(i, j, &diff_sign)) {
		fprintf(gfp, _("Equations are identical.\n"));
		return true;
	}
	debug_string(0, _("Simplifying both equations..."));
	simpa_side(rhs[i], &n_rhs[i], true);
	simpa_side(rhs[j], &n_rhs[j], true);
	if (compare_rhs(i, j, &diff_sign)) {
		fprintf(gfp, _("Equations are identical.\n"));
		return true;
	}
	if (solve_sub(&zero_token, 1, lhs[i], &n_lhs[i], rhs[i], &n_rhs[i]) <= 0
	    || solve_sub(&zero_token, 1, lhs[j], &n_lhs[j], rhs[j], &n_rhs[j]) <= 0) {
		error(_("Can't solve for zero!"));
		return false;
	}
	uf_simp(rhs[i], &n_rhs[i]);
	uf_simp(rhs[j], &n_rhs[j]);
	if (compare_rhs(i, j, &diff_sign)) {
		fprintf(gfp, _("Equations are identical.\n"));
		return true;
	}
	fprintf(gfp, _("Equations may differ.\n"));
	return false;
times_neg1:
	if (!diff_sign && lhs[i][0].token.variable == lhs[j][0].token.variable) {
		fprintf(gfp, _("Equations are identical.\n"));
		return true;
	}
	fprintf(gfp, _("Variable ("));
	list_proc(lhs[i], n_lhs[i], false);
	fprintf(gfp, _(") in the first equation is equal to ("));
	if (diff_sign) {
		fprintf(gfp, "-");
	}
	list_proc(lhs[j], n_lhs[j], false);
	fprintf(gfp, _(") in the second equation.\n"));
	return(!diff_sign);
}

#if	!LIBRARY
/*
 * The divide command.
 */
int
divide_cmd(cp)
char	*cp;
{
	long		v, v_tmp;
	int		i, j;
	int		nl, nr;
	double		d1, d2, d3, d4;
	complexs	c1, c2, c3;

	v = 0;
	if (*cp) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
		if (extra_garbage(cp))
			return false;
	}
	i = next_espace();
	my_strlcpy(prompt_str, _("Enter dividend: "), sizeof(prompt_str));
	if (!get_expr(rhs[i], &nr)) {
		return false;
	}
	my_strlcpy(prompt_str, _("Enter divisor: "), sizeof(prompt_str));
	if (!get_expr(lhs[i], &nl)) {
		return false;
	}
	fprintf(gfp, "\n");
	calc_simp(lhs[i], &nl);
	calc_simp(rhs[i], &nr);
	if (get_constant(rhs[i], nr, &d1) && get_constant(lhs[i], nl, &d2)) {
		d4 = modf(d1 / d2, &d3);
		fprintf(gfp, _("Result of numerical division: %.14g\n"), d1 / d2);
		fprintf(gfp, _("Quotient: %.14g, Remainder: %.14g\n"), d3, d4 * d2);
		d1 = fabs(d1);
		d2 = fabs(d2);
		d3 = gcd(d1, d2);
		if (d3 <= 0.0) {
			fprintf(gfp, _("No GCD found.\n"));
			return true;
		}
		fprintf(gfp, _("Greatest Common Divisor (GCD) = %.14g\n"), d3);
		fprintf(gfp, _("Least Common Multiple (LCM) = %.14g\n"), (d1 * d2) / d3);
		return true;
	}
	if (parse_complex(rhs[i], nr, &c1) && parse_complex(lhs[i], nl, &c2)) {
		c3 = complex_div(c1, c2);
		fprintf(gfp, _("Result of complex division: "));
		if (c3.im == 0.0) {
			fprintf(gfp, "%.12g\n\n", c3.re);
		} else {
			fprintf(gfp, "%.12g%+.12g*i#\n\n", c3.re, c3.im);
		}
		return true;
	}
	v_tmp = v;
	if (poly_div(rhs[i], nr, lhs[i], nl, &v_tmp)) {
		simp_divide(tlhs, &n_tlhs);
		simp_divide(trhs, &n_trhs);
		list_var(v_tmp, 0);
		fprintf(gfp, _("Polynomial division successful using base variable (%s).\n"), var_str);
		fprintf(gfp, _("The quotient is:\n"));
		list_proc(tlhs, n_tlhs, false);
		fprintf(gfp, _("\n\nThe remainder is:\n"));
		list_proc(trhs, n_trhs, false);
		fprintf(gfp, "\n");
	} else {
		fprintf(gfp, _("Polynomial division failed.\n"));
	}
	fprintf(gfp, "\n");
	j = poly_gcd(rhs[i], nr, lhs[i], nl, v);
	if (!j) {
		j = poly_gcd(lhs[i], nl, rhs[i], nr, v);
	}
	if (j) {
		simp_divide(trhs, &n_trhs);
		fprintf(gfp, _("Polynomial Greatest Common Divisor (iterations = %d):\n"), j);
		list_proc(trhs, n_trhs, false);
		fprintf(gfp, "\n");
	} else {
		fprintf(gfp, _("No univariate polynomial GCD found.\n"));
	}
	return true;
}
#endif

/*
 * The eliminate command.
 */
int
eliminate_cmd(cp)
char	*cp;
{
	long	v, last_v, v1, va[MAX_VARS];
	int	i, n;
	int	using_flag, did_something = false;
	char	used[N_EQUATIONS];
	int	vc = 0;

	for (i = 0; i < ARR_CNT(used); i++)
		used[i] = false;
	if (current_not_defined()) {
		return false;
	}
	if (is_all(cp)) {
		cp = skip_param(cp);
		if (extra_garbage(cp))
			return false;
		last_v = 0;
		for (;;) {
			v1 = -1;
			for (i = 0; i < n_lhs[cur_equation]; i += 2) {
				if (lhs[cur_equation][i].kind == VARIABLE
				    && lhs[cur_equation][i].token.variable > last_v) {
					if (v1 == -1 || lhs[cur_equation][i].token.variable < v1) {
						v1 = lhs[cur_equation][i].token.variable;
					}
				}
			}
			for (i = 0; i < n_rhs[cur_equation]; i += 2) {
				if (rhs[cur_equation][i].kind == VARIABLE
				    && rhs[cur_equation][i].token.variable > last_v) {
					if (v1 == -1 || rhs[cur_equation][i].token.variable < v1) {
						v1 = rhs[cur_equation][i].token.variable;
					}
				}
			}
			if (v1 == -1)
				break;
			last_v = v1;
			if ((v1 & VAR_MASK) > SIGN) {
				if (vc >= ARR_CNT(va)) {
					break;
				}
				va[vc++] = v1;
			}
		}
	}
next_var:
	if (*cp) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	} else if (vc) {
		v = va[--vc];
	} else {
		if (did_something) {
			return_result(cur_equation);
		} else {
			error(_("No substitutions made."));
		}
		return did_something;
	}
	if (!var_in_equation(cur_equation, v)) {
#if	!SILENT
		list_var(v, 0);
		printf(_("Variable (%s) not found in current equation.\n"), var_str);
#endif
		goto next_var;
	}
	using_flag = (strcmp_tospace(cp, "using") == 0);
	if (using_flag) {
		cp = skip_param(cp);
		i = decstrtol(cp, &cp) - 1;
		if (not_defined(i)) {
			return false;
		}
		if (i == cur_equation) {
			error(_("Error: source and destination are the same."));
			return false;
		}
		if (!elim_sub(i, v))
			goto next_var;
	} else {
		n = 1;
		i = cur_equation;
		for (;; n++) {
			if (n >= n_equations) {
				goto next_var;
			}
			if (i <= 0)
				i = n_equations - 1;
			else
				i--;
			if (used[i])
				continue;
			if (n_lhs[i] && n_rhs[i] && var_in_equation(i, v)) {
				if (elim_sub(i, v))
					break;
			}
		}
	}
	did_something = true;
	simp_sub(cur_equation);
	used[i] = true;
	goto next_var;
}

static int
elim_sub(i, v)
int	i;
long	v;
{
	token_type	want;

#if	!SILENT
	if (debug_level >= 0) {
		list_var(v, 0);
		printf(_("Solving equation #%d for (%s)...\n"), i + 1, var_str);
	}
#endif
	want.level = 1;
	want.kind = VARIABLE;
	want.token.variable = v;
	if (solve_sub(&want, 1, lhs[i], &n_lhs[i], rhs[i], &n_rhs[i]) <= 0) {
		debug_string(0, _("Solve failed."));
		return false;
	}
	subst_var_with_exp(rhs[cur_equation], &n_rhs[cur_equation], rhs[i], n_rhs[i], v);
	subst_var_with_exp(lhs[cur_equation], &n_lhs[cur_equation], rhs[i], n_rhs[i], v);
	return true;
}

/*
 * The flist command.
 *
 * Displays equations in fraction format.
 */
int
flist_cmd(cp)
char	*cp;
{
	int	i, j;
	int	factor_flag;

	if (factor_flag = (strcmp_tospace(cp, "factor") == 0)) {
		cp = skip_param(cp);
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		if (n_lhs[i] > 0) {
			group_sub(i);
			if (factor_flag || factor_int_flag) {
				factor_int(lhs[i], &n_lhs[i]);
				factor_int(rhs[i], &n_rhs[i]);
			}
#if	LIBRARY
			if (gfp == stdout) {
				result_str = list_equation(i, false);
				return true;
			}
#endif
			flist_sub(i);
		}
	}
	return true;
}

/*
 * The list command.
 */
int
list_cmd(cp)
char	*cp;
{
	int	i, j;
	int	export_flag = false;

	if (strcmp_tospace(cp, "export") == 0) {
		export_flag = 2;
		cp = skip_param(cp);
	} else if (strcmp_tospace(cp, "maxima") == 0) {
		export_flag = 1;
		cp = skip_param(cp);
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
#if	LIBRARY
		if (gfp == stdout) {
			result_str = list_equation(i, export_flag);
			return true;
		}
#endif
		list1_sub(i, export_flag);
	}
	return true;
}

#if	!BASICS && !LIBRARY
/*
 * The code command.
 */
int
code_cmd(cp)
char	*cp;
{
	int	i, j;
	int	java_flag = 0;
	int	int_flag = false;

	if (strcmp_tospace(cp, "c") == 0) {
		cp = skip_param(cp);
	} else if (strcmp_tospace(cp, "java") == 0) {
		cp = skip_param(cp);
		java_flag = 1;
	} else if (strcmp_tospace(cp, "python") == 0) {
		cp = skip_param(cp);
		java_flag = 2;
	} else if (strcmp_tospace(cp, "int") == 0 || strcmp_tospace(cp, "integer") == 0) {
		cp = skip_param(cp);
		int_flag = true;
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		if (n_lhs[i] > 0) {
			if (n_rhs[i] == 0 || n_lhs[i] != 1 || lhs[i][0].kind != VARIABLE) {
#if	!SILENT
				printf(_("#%d is not a solved equation.\n"), i + 1);
#endif
				continue;
			}
			in_calc_cmd = true;
			if (int_flag) {
				elim_loop(rhs[i], &n_rhs[i]);
				uf_repeat_always(rhs[i], &n_rhs[i]);
			}
			group_sub(i);
			in_calc_cmd = false;
			if (int_flag) {
				if (!int_expr(rhs[i], n_rhs[i])) {
#if	!SILENT
					printf(_("#%d is not an integer equation.\n"), i + 1);
#endif
					continue;
				}
			}
			list_c_equation(i, java_flag, int_flag);
		}
	}
	return true;
}
#endif

/*
 * The replace command.
 */
int
replace_cmd(cp)
char	*cp;
{
	int	i, j;
	int	n;
	long	last_v, v;
	char	*cp_start;
	long	va[MAX_VARS];
	int	vc;
	int	found;
	char	*cp1;

	cp_start = cp;
	if (current_not_defined()) {
		return false;
	}
	i = cur_equation;
	if (strcmp_tospace(cp, "constants") == 0) {
		cp = skip_param(cp);
		if (extra_garbage(cp))
			return false;
		subst_constants(lhs[i], &n_lhs[i]);
		subst_constants(rhs[i], &n_rhs[i]);
		simp_sub(i);
		return_result(i);
		return true;
	}
	for (vc = 0; *cp; vc++) {
		if (strcmp_tospace(cp, "with") == 0) {
			if (vc)
				break;
			error(_("No variables specified."));
			return false;
		}
		if (vc >= ARR_CNT(va)) {
			error(_("Too many variables specified."));
			return false;
		}
		cp = parse_var2(&va[vc], cp);
		if (cp == NULL) {
			return false;
		}
		if (!var_in_equation(i, va[vc])) {
			error(_("Variable not found."));
			return false;
		}
	}
	n_tlhs = n_lhs[i];
	blt(tlhs, lhs[i], n_tlhs * sizeof(token_type));
	n_trhs = n_rhs[i];
	blt(trhs, rhs[i], n_trhs * sizeof(token_type));
	last_v = 0;
	for (;;) {
		v = -1;
		for (j = 0; j < n_lhs[i]; j++) {
			if (lhs[i][j].kind == VARIABLE) {
				if (lhs[i][j].token.variable > last_v
				    && (v == -1 || lhs[i][j].token.variable < v))
					v = lhs[i][j].token.variable;
			}
		}
		for (j = 0; j < n_rhs[i]; j++) {
			if (rhs[i][j].kind == VARIABLE) {
				if (rhs[i][j].token.variable > last_v
				    && (v == -1 || rhs[i][j].token.variable < v))
					v = rhs[i][j].token.variable;
			}
		}
		if (v == -1) {
			break;
		}
		last_v = v;
		if (vc) {
			found = false;
			for (j = 0; j < vc; j++) {
				if (v == va[j])
					found = true;
			}
			if (!found)
				continue;
			if (*cp) {
				if (strcmp_tospace(cp, "with") != 0) {
					return false;
				}
				cp1 = skip_param(cp);
				input_column += (cp1 - cp_start);
				if (!case_sensitive_flag) {
					str_tolower(cp1);
				}
				if ((cp1 = parse_section(scratch, &n, cp1)) == NULL
				    || n <= 0) {
					return false;
				}
				if (extra_garbage(cp1))
					return false;
				goto do_this;
			}
		}
		list_var(v, 0);
		snprintf(prompt_str, sizeof(prompt_str), _("Enter %s: "), var_str);
		if (!get_expr(scratch, &n)) {
			continue;
		}
do_this:
		for (j = 0; j < n; j += 2) {
			if (scratch[j].kind == VARIABLE) {
				scratch[j].token.variable = -scratch[j].token.variable;
			}
		}
		subst_var_with_exp(tlhs, &n_tlhs, scratch, n, v);
		subst_var_with_exp(trhs, &n_trhs, scratch, n, v);
	}
	for (j = 0; j < n_tlhs; j += 2)
		if (tlhs[j].kind == VARIABLE && tlhs[j].token.variable < 0)
			tlhs[j].token.variable = -tlhs[j].token.variable;
	for (j = 0; j < n_trhs; j += 2)
		if (trhs[j].kind == VARIABLE && trhs[j].token.variable < 0)
			trhs[j].token.variable = -trhs[j].token.variable;
	n_lhs[i] = n_tlhs;
	blt(lhs[i], tlhs, n_tlhs * sizeof(token_type));
	n_rhs[i] = n_trhs;
	blt(rhs[i], trhs, n_trhs * sizeof(token_type));
	simp_sub(i);
	return_result(i);
	return true;
}

/*
 * The simplify command.
 */
int
simplify_cmd(cp)
char	*cp;
{
	int	i, j;
	int	quick_flag = false, symb = false;

	for (;; cp = skip_param(cp)) {
		if (strncasecmp(cp, "symbolic", 4) == 0) {
			symb = true;
			continue;
		}
		if (strncasecmp(cp, "quick", 4) == 0) {
			quick_flag = true;
			continue;
		}
		break;
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	symb_flag = symb;
	for (; i <= j; i++) {
		if (n_lhs[i]) {
			simpa_side(lhs[i], &n_lhs[i], quick_flag);
			simpa_side(rhs[i], &n_rhs[i], quick_flag);
			return_result(i);
		}
	}
	symb_flag = false;
	return true;
}

/*
 * The factor command.
 */
int
factor_cmd(cp)
char	*cp;
{
	int	i, j;
	int	i1, count;
	long	v;
	double	d;
	char	buf[MAX_CMD_LEN];

	if (strncasecmp(cp, "number", 6) == 0) {
		cp = skip_param(cp);
		if (*cp == '\0') {
			my_strlcpy(prompt_str, _("Enter integers to factor: "), sizeof(prompt_str));
			cp = get_string(buf, sizeof(buf));
			if (cp == NULL)
				return false;
		}
		for (count = 1; *cp; count++) {
			d = strtod(cp, &cp);
			cp = skip_space(cp);
			if (!factor_one(d)) {
				error(_("Not a valid integer."));
				return false;
			}
			display_unique();
		}
		return true;
	}
	v = 0;
	if (!get_range(&cp, &i, &j)) {
		return false;
	}
	do {
		if (*cp) {
			if ((cp = parse_var2(&v, cp)) == NULL) {
				return false;
			}
		}
		for (i1 = i; i1 <= j; i1++) {
			if (n_lhs[i1]) {
				simpv_side(lhs[i1], &n_lhs[i1], v);
				simpv_side(rhs[i1], &n_rhs[i1], v);
			}
		}
	} while (*cp);
	for (i1 = i; i1 <= j; i1++) {
		return_result(i1);
	}
	return true;
}

/*
 * The unfactor command.
 */
int
unfactor_cmd(cp)
char	*cp;
{
	int	i, j;
	int	fully_flag;

	if (fully_flag = (strncasecmp(cp, "fully", 4) == 0)) {
		cp = skip_param(cp);
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	partial_flag = !fully_flag;
	for (; i <= j; i++) {
		if (n_lhs[i]) {
			uf_simp(lhs[i], &n_lhs[i]);
			uf_simp(rhs[i], &n_rhs[i]);
			return_result(i);
		}
	}
	partial_flag = true;
	return true;
}

#if	!LIBRARY
/*
 * The quit command.
 */
int
quit_cmd(cp)
char	*cp;
{
	if (extra_garbage(cp))
		return false;
	exit_program(0);
	return false;
}
#endif

#if	!SECURE && !LIBRARY
/*
 * The read command.
 */
int
read_cmd(cp)
char	*cp;
{
	int	rv;
	FILE	*fp;
	jmp_buf	save_save;
	char	buf[MAX_CMD_LEN];

	if (*cp == '\0') {
		error(_("No file specified."));
		return false;
	}
	fp = NULL;
	snprintf(buf, sizeof(buf), "%s.in", cp);
	fp = fopen(buf, "r");
	if (fp == NULL) {
		fp = fopen(cp, "r");
		if (fp == NULL) {
			error(_("Can't open file."));
			return false;
		}
	}
	blt(save_save, jmp_save, sizeof(jmp_save));
	if ((rv = setjmp(jmp_save)) != 0) {
		clean_up();
		if (rv == 14)
			error(_("Expression too big."));
		printf(_("Read operation aborted.\n"));
		goto end_read;
	}
	while (cp = fgets((char *) tlhs, n_tokens * sizeof(token_type), fp)) {
		default_color();
		input_column = printf("%d%s", cur_equation + 1, html_flag ? HTML_PROMPT : PROMPT);
		printf("%s", cp);
		set_error_level(cp);
		if (!process(cp)) {
			longjmp(jmp_save, 3);
		}
	}
end_read:
	blt(jmp_save, save_save, sizeof(jmp_save));
	fclose(fp);
	return(!rv);
}
#endif

#if	(UNIX || CYGWIN) && !SECURE && !LIBRARY
/*
 * The edit command.
 */
int
edit_cmd(cp)
char	*cp;
{
	FILE	*fp;
	int	fd;
	int	rv;
	char	tmp_file[MAX_CMD_LEN];

	if (*cp == '\0') {
#if	CYGWIN
		my_strlcpy(tmp_file, "mathxxx.tmp", sizeof(tmp_file));
		fp = fopen(tmp_file, "w+");
		if (fp == NULL) {
			error(_("Can't create temporary file."));
			return false;
		}
#else
		my_strlcpy(tmp_file, TMP_FILE, sizeof(tmp_file));
		fd = mkstemp(tmp_file);
		if (fd < 0 || (fp = fdopen(fd, "w+")) == NULL) {
			error(_("Can't create temporary file."));
			return false;
		}
#endif
		gfp = fp;
		high_prec = true;
		list_cmd("all");
		high_prec = false;
		gfp = stdout;
		fclose(fp);
		rv = edit_sub(tmp_file);
		unlink(tmp_file);
		return rv;
	} else {
		if (access(cp, 6)) {
			error(_("You can only edit existing/writable files or all equations."));
			return false;
		}
		return edit_sub(cp);
	}
}

static int
edit_sub(cp)
char	*cp;
{
	char	cl[MAX_CMD_LEN];	/* command line */
	char	*cp1;
	char	*editor_keyword = "EDITOR";

edit_again:
	cp1 = getenv(editor_keyword);
	if (cp1 == NULL) {
#if	CYGWIN
		cp1 = "notepad";
#else
		printf(_("%s environment variable not set.\n"), editor_keyword);
		return false;
#endif
	}
	snprintf(cl, sizeof(cl), "%s %s", cp1, cp);
	if (shell_out(cl)) {
		printf(_("Error executing editor.\n"));
		printf(_("Command line = \"%s\".\n"), cl);
		printf(_("Check %s environment variable.\n"), editor_keyword);
		return false;
	}
	clear_all();
	if (!read_cmd(cp)) {
		if (pause_cmd("Prepare to run the editor")) {
			goto edit_again;
		}
	}
	return true;
}
#endif

#if	!SECURE
/*
 * The save command.
 */
int
save_cmd(cp)
char	*cp;
{
	FILE	*fp;
	int	rv;

	if (*cp == '\0') {
		error(_("No file specified."));
		return false;
	}
	if (access(cp, 0) == 0) {
		snprintf(prompt_str, sizeof(prompt_str), _("\"%s\" exists.  Overwrite (Y/N)? "), cp);
		if (!get_yes_no()) {
			printf(_("Command aborted.\n"));
			return false;
		}
	}
	fp = fopen(cp, "w");
	if (fp == NULL) {
		error(_("Can't create file."));
		return false;
	}
	gfp = fp;
	high_prec = true;
	rv = list_cmd("all");
	high_prec = false;
	gfp = stdout;
	if (fclose(fp))
		rv = false;
	if (rv) {
		printf(_("All equations saved in file: \"%s\".\n"), cp);
	} else {
		error(_("Error encountered while saving equations."));
	}
	return rv;
}
#endif
