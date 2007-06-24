/*
 * Mathomatic commands that don't belong anywhere else.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

#define	OPT_MIN_SIZE	7	/* Minimum size of repeated expressions to find in optimize command. */

static int	sum_product();
static int	complex_func();
static int	elim_sub();
#if	!LIBRARY
static int	edit_sub();
static int	find_more();
static int	opt_es();

/* Global variables for the optimize command. */
static int	opt_en[N_EQUATIONS];
static int	last_temp_var = 0;
#endif

/*
 * The version command.
 */
int
version_cmd(cp)
char	*cp;	/* the command line argument */
{
#if	LIBRARY
	free(result_str);
	result_str = strdup(VERSION);
#endif
	return version_report();
}

/*
 * Report version number, compile flags, and memory usage.
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
#if	SILENT
	fprintf(gfp, "SILENT ");
#endif
#if	LIBRARY
	fprintf(gfp, "LIBRARY ");
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
	fprintf(gfp, _("\nMaximum memory usage: %lld kilobytes\n"), (long long) (N_EQUATIONS + 3LL) * n_tokens * sizeof(token_type) * 2LL / 1000LL);
	return true;
}

/*
 * The solve command.
 */
int
solve_cmd(cp)
char	*cp;
{
	int	i;
	char	buf[MAX_CMD_LEN];

	i = cur_equation;
	if (i < 0 || i >= n_equations || n_lhs[i] <= 0 || n_rhs[i] <= 0) {
		error(_("No current equation."));
		return false;
	}
	if (*cp == '\0') {
		my_strlcpy(prompt_str, _("Enter variable or 0: "), sizeof(prompt_str));
		if ((cp = get_string(buf, sizeof(buf))) == NULL) {
			return false;
		}
		if (!case_sensitive_flag) {
			str_tolower(cp);
		}
	}
	i = next_espace();
	if ((cp = parse_equation(i, cp)) != NULL) {
		if (n_lhs[i] && n_rhs[i]) {
			error(_("Invalid argument."));
		} else if (!extra_characters(cp) && solve(i, cur_equation)) {
			return return_result(cur_equation);
		}
	}
	n_lhs[i] = 0;
	n_rhs[i] = 0;
	return false;
}

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
	int		i;
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
	if (*cp) {
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	}
	if (no_vars(source, ns, &v)) {
		error(_("Current expression contains no variables."));
		return false;
	}
	if (v == 0) {
		if (!prompt_var(&v)) {
			return false;
		}
	}
	if (!found_var(source, ns, v)) {
		error(_("Variable not found."));
		return false;
	}
	if (*cp) {
		if (*cp == '=') {
			cp1 = skip_space(cp + 1);
		} else {
			cp1 = cp;
		}
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
	if (strcmp_tospace(cp, "to") == 0) {
		cp = skip_param(cp);
	}
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
	if (extra_characters(cp))
		return false;
	count_down = (end < start);
	if (fmod(fabs(start - end) / step, 1.0)) {
		error(_("Warning: end value not reached."));
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
	return return_result(result_en);
}

#if	!LIBRARY
/*
 * This function is for the "optimize" command.
 * It finds and substitutes all occurrences of the RHS of "en" in "equation".
 * It should be called repeatedly until it returns false.
 */
static int
find_more(equation, np, en)
token_type	*equation;	/* expression pointer */
int		*np;		/* pointer to length of expression */
int		en;		/* equation space number */
{
	int	i, j, k;
	int	level;
	int	diff_sign;
	int	found_se;	/* found sub-expression flag */

	if (!solved_equation(en)) {
		return false;
	}
	for (level = 1, found_se = true; found_se; level++) {
		for (i = 1, found_se = false; i < *np; i = j + 2) {
			for (j = i; j < *np && equation[j].level >= level; j += 2)
				;
			if (j == i) {
				continue;
			}
			found_se = true;
			k = i - 1;
			if (se_compare(&equation[k], j - k, rhs[en], n_rhs[en], &diff_sign)) {
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
				equation[k].token.variable = lhs[en][0].token.variable;
				return true;
			}
		}
	}
	return false;
}

/*
 * This function is for the "optimize" command.
 * It finds and replaces all repeated expressions in "equation" with temporary variables.
 * It also creates a new equation for each temporary variable.
 * It should be called repeatedly until it returns false.
 */
static int
opt_es(equation, np)
token_type	*equation;
int		*np;
{
	int	i, j, k, i1, i2, jj1, k1;
	int	level, level1;
	int	diff_sign;
	int	found_se, found_se1;	/* found sub-expression flags */
	long	v;
	char	temp_buf[50];

	for (level = 1, found_se = true; found_se; level++) {
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
						snprintf(temp_buf, sizeof(temp_buf), "temp%d", last_temp_var);
						if (parse_var(&v, temp_buf) == NULL) {
							return false;	/* can't create "temp" variable */
						}
						last_temp_var++;
						if (last_temp_var < 0) {
							last_temp_var = 0;
						}
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
	int	i, j, k, i1;
	int	start, stop;
	int	rv = false;
	int	start_en;

	if (!get_range_eol(&cp, &start, &stop)) {
		return false;
	}
	opt_en[0] = -1;
	start_en = 0;
	for (i = start; i <= stop; i++) {
		simp_sub(i);
	}
	for (i = start; i <= stop; i++) {
		for (j = start; j <= stop; j++) {
			if (i != j) {
				while (find_more(rhs[i], &n_rhs[i], j)) {
					rv = true;
				}
			}
		}
	}
	for (i = start; i <= stop; i++) {
		if (n_lhs[i] == 0)
			continue;
		for (j = 0; opt_en[j] >= 0; j++) {
			simp_sub(opt_en[j]);
			while (find_more(lhs[i], &n_lhs[i], opt_en[j]))
				;
			while (find_more(rhs[i], &n_rhs[i], opt_en[j]))
				;
		}
		while (opt_es(lhs[i], &n_lhs[i])) {
			rv = true;
		}
		while (opt_es(rhs[i], &n_rhs[i])) {
			rv = true;
		}
		if (rv) {
			for (i1 = start_en; opt_en[i1] >= 0; i1++) {
				for (j = start_en; opt_en[j] >= 0; j++) {
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
			for (; opt_en[start_en] >= 0; start_en++) {
				list_sub(opt_en[start_en]);
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
		if (!push_en(i)) {
			error(_("Push failed."));
			return false;
		}
	}
	debug_string(0, _("Expression(s) pushed.  Press the UP key to access."));
	return true;
}

/*
 * Push an equation space into the readline history.
 *
 * Return true if successful.
 */
int
push_en(en)
int	en;	/* equation space number */
{
	if (readline_enabled) {
		high_prec = true;
		add_history(list_equation(en, false));
		high_prec = false;
		return true;
	} else {
		return false;
	}
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
	if (!autosolve) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "autosolve\n");

	if (!autocalc) {
		fprintf(gfp, "no ");
	}
	fprintf(gfp, "autocalc\n");

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

	if (!display2d) {
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

	fprintf(gfp, "special_variable_character = %c\n", special_variable_character);
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
	int	i;
	int	negate;
	char	*cp1;

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
			i = decstrtol(cp, &cp1);
			if (cp == cp1) {
				error(_("Please specify the debug level number."));
				return false;
			}
			cp = cp1;
			debug_level = i;
		}
		goto next_option;
	}
#endif
	if (strncasecmp(cp, "special", 7) == 0) {
		cp = skip_param(cp);
		if (negate) {
			special_variable_character = '\0';
		} else {
			if (*cp == '\0' || cp[1]) {
				error(_("Please specify a single character."));
				return false;
			}
			special_variable_character = *cp;
		}
		return true;
	}
	if (strncasecmp(cp, "columns", 7) == 0) {
		cp = skip_param(cp);
		if (negate) {
			screen_columns = 0;
		} else {
			if ((i = decstrtol(cp, &cp1)) < 0 || cp == cp1) {
				error(_("Please specify how wide the screen is in columns."));
				return false;
			}
			cp = cp1;
			screen_columns = i;
		}
		goto next_option;
	}
	if (strncasecmp(cp, "autosolve", 9) == 0) {
		cp = skip_param(cp);
		autosolve = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "autocalc", 8) == 0) {
		cp = skip_param(cp);
		autocalc = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "case", 4) == 0) {
		cp = skip_param(cp);
		case_sensitive_flag = !negate;
		goto next_option;
	}
	if (strncasecmp(cp, "display2d", 9) == 0) {
		cp = skip_param(cp);
		display2d = !negate;
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
 * The echo command.
 */
int
echo_cmd(cp)
char	*cp;
{
	fprintf(gfp, "%s\n", cp);
	return true;
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
		cp = _("Please press Enter");
	}
	snprintf(prompt_str, sizeof(prompt_str), " ==== %s ==== ", cp);
	if ((cp1 = get_string(buf, sizeof(buf))) == NULL) {
		return false;
	}
	if (strncasecmp(cp1, "quit", 4) == 0) {
		return false;
	}
	if (strncasecmp(cp1, "exit", 4) == 0) {
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
			copy_espace(i1, k);
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
		if (extra_characters(cp))
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
	has_imag = has_real = false;
	for (beg = k = 0; beg < *nps; beg = k, k++) {
		found_imag = false;
		for (; k < *nps; k++) {
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
	return return_result(cur_equation);
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

	if (extra_characters(cp))
		return false;
	trhs[0] = zero_token;
	n_trhs = 1;
	for (count = 0.0;; count++) {
		fprintf(gfp, _("Running total = "));
		list_proc(trhs, n_trhs, false);
		fprintf(gfp, "\n");
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
			fprintf(gfp, _("Average = "));
			list_proc(tlhs, n_tlhs, false);
			fprintf(gfp, "\n");
		}
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
	long		counter, counter_max;
	sign_array_type	sa_mark;
	sign_array_type	sa_value;
	long		l, iterations = 1;
	token_type	*source;
	int		n;
	int		diff_sign;

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
		while (!get_expr(tes, &n_tes))
			;
		calc_simp(tes, &n_tes);
		blt(tlhs, trhs, n_trhs * sizeof(token_type));
		n_tlhs = n_trhs;
		for (l = 0; l < iterations; l++) {
			blt(trhs, tlhs, n_tlhs * sizeof(token_type));
			n_trhs = n_tlhs;
			subst_var_with_exp(trhs, &n_trhs, tes, n_tes, it_v);
			calc_simp(trhs, &n_trhs);
			if (se_compare(trhs, n_trhs, tes, n_tes, &diff_sign) && !diff_sign) {
				printf(_("Convergence reached after %ld iterations.\n"), l + 1);
				break;
			}
			blt(tes, trhs, n_trhs * sizeof(token_type));
			n_tes = n_trhs;
		}
	} else {
		calc_simp(trhs, &n_trhs);
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
	counter_max = (1L << k) - 1L;
	for (counter = 0; counter <= counter_max; counter++) {
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
		fprintf(gfp, "\n\n");
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
		error(_("Error in compare function or too many terms to compare."));
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
		error(_("Error in compare function or too many terms to compare."));
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
			fprintf(gfp, _("Expressions differ.\n"));
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
	fprintf(gfp, _("Equations differ.\n"));
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
		if (extra_characters(cp))
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
		fprintf(gfp, _("Result of numerical division: %.*g/%.*g = %.*g\n"), precision, d1, precision, d2, precision, d1 / d2);
		fprintf(gfp, _("Quotient: %.*g, Remainder: %.*g\n"), precision, d3, precision, d4 * d2);
		d1 = fabs(d1);
		d2 = fabs(d2);
		d3 = gcd(d1, d2);
		if (d3 <= 0.0) {
			fprintf(gfp, _("No GCD found.\n"));
			return true;
		}
		fprintf(gfp, _("Greatest Common Divisor (GCD) = %.*g\n"), precision, d3);
		fprintf(gfp, _("Least Common Multiple (LCM) = %.*g\n"), precision, (d1 * d2) / d3);
		return true;
	}
	if (parse_complex(rhs[i], nr, &c1) && parse_complex(lhs[i], nl, &c2)) {
		c3 = complex_div(c1, c2);
		fprintf(gfp, _("Result of complex number division:\n"));
		fprintf(gfp, "%.*g %+.*g*i\n\n", precision, c3.re, precision, c3.im);
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
		fprintf(gfp, _("Polynomial GCD (Euclidean algorithm iterations = %d):\n"), j);
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

	if (*cp == '\0') {
		error(_("Please specify the variables to eliminate or \"all\" for all variables."));
		return false;
	}
	for (i = 0; i < ARR_CNT(used); i++)
		used[i] = false;
	if (current_not_defined()) {
		return false;
	}
next_var:
	if (vc) {
		v = va[--vc];
	} else if (*cp) {
		if (is_all(cp)) {
			cp = skip_param(cp);
			vc = 0;
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
			goto next_var;
		}
		cp = parse_var2(&v, cp);
		if (cp == NULL) {
			return false;
		}
	} else {
		if (did_something) {
			did_something = return_result(cur_equation);
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
		printf(_("Solving equation #%d for (%s) and substituting into the current equation...\n"), i + 1, var_str);
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
	simp_sub(cur_equation);
	return true;
}

/*
 * The display command.
 *
 * Displays equations in multi-line fraction format.
 */
int
display_cmd(cp)
char	*cp;
{
	int	i, j;
	int	factor_flag;

	factor_flag = (strcmp_tospace(cp, "factor") == 0);
	if (factor_flag) {
		cp = skip_param(cp);
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		if (n_lhs[i] > 0) {
			make_fractions_and_group(i);
			if (factor_flag || factor_int_flag) {
				factor_int_sub(i);
			}
#if	LIBRARY
			if (gfp == stdout) {
				free(result_str);
				result_str = list_equation(i, false);
				return(result_str != NULL);
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
	}
	if (strcmp_tospace(cp, "maxima") == 0) {
		export_flag = 1;
		cp = skip_param(cp);
	}
	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		if (factor_int_flag) {
			factor_int_sub(i);
		}
#if	LIBRARY
		if (gfp == stdout) {
			free(result_str);
			result_str = list_equation(i, export_flag);
			return(result_str != NULL);
		}
#endif
		list1_sub(i, export_flag);
	}
	return true;
}

#if	!LIBRARY
/*
 * The code command.
 */
int
code_cmd(cp)
char	*cp;
{
	int	i, j;
	int	language = 0;
	int	int_flag = false, displayed = false;

	if (strcmp_tospace(cp, "c") == 0 || strcmp_tospace(cp, "c++") == 0) {
		cp = skip_param(cp);
	} else if (strcmp_tospace(cp, "java") == 0) {
		cp = skip_param(cp);
		language = 1;
	} else if (strcmp_tospace(cp, "python") == 0) {
		cp = skip_param(cp);
		language = 2;
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
				error(_("Not a solved equation."));
				continue;
			}
			approximate_roots = true;
			simp_i(rhs[i], &n_rhs[i]);
			approximate_roots = false;
			if (int_flag) {
				uf_repeat_always(rhs[i], &n_rhs[i]);
			}
			make_fractions_and_group(i);
			if (int_flag) {
				if (!int_expr(rhs[i], n_rhs[i])) {
					error(_("Not an integer equation."));
					continue;
				}
			}
			list_c_equation(i, language, int_flag);
			displayed = true;
		}
	}
	return displayed;
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
		if (extra_characters(cp))
			return false;
		subst_constants(lhs[i], &n_lhs[i]);
		subst_constants(rhs[i], &n_rhs[i]);
		approximate_roots = true;
		simp_sub(i);
		approximate_roots = false;
		return return_result(i);
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
		for (j = 0; j < n_lhs[i]; j += 2) {
			if (lhs[i][j].kind == VARIABLE) {
				if (lhs[i][j].token.variable > last_v
				    && (v == -1 || lhs[i][j].token.variable < v))
					v = lhs[i][j].token.variable;
			}
		}
		for (j = 0; j < n_rhs[i]; j += 2) {
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
				if ((cp1 = parse_section(scratch, &n, cp1)) == NULL || n <= 0) {
					return false;
				}
				if (extra_characters(cp1))
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
	return return_result(i);
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
				error(_("Not a valid integer or number too large."));
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

	fully_flag = (strncasecmp(cp, "fully", 4) == 0);
	if (fully_flag) {
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

/*
 * The fraction command.
 */
int
fraction_cmd(cp)
char	*cp;
{
	int	i, j;

	if (!get_range_eol(&cp, &i, &j)) {
		return false;
	}
	for (; i <= j; i++) {
		if (n_lhs[i]) {
			frac_side(lhs[i], &n_lhs[i]);
			frac_side(rhs[i], &n_rhs[i]);
			return_result(i);
		}
	}
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
	if (extra_characters(cp))
		return false;
	exit_program(0);
	return false;
}
#endif

#if	!SECURE
/*
 * The read command.
 */
int
read_cmd(cp)
char	*cp;
{
	int	rv;
	FILE	*fp;
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
			error(_("Can't open input file."));
			return false;
		}
	} else {
		cp = buf;
	}
	rv = read_sub(fp);
	if (rv) {
		printf(_("Read operation aborted.\n"));
	} else {
		printf(_("Finished reading file \"%s\".\n"), cp);
	}
	fclose(fp);
	return(!rv);
}

/*
 * Read and process Mathomatic input from a file pointer.
 *
 * Return zero if no error, non-zero if aborted.
 */
int
read_sub(fp)
FILE	*fp;
{
	int	rv;
	jmp_buf	save_save;
	char	*cp;

	blt(save_save, jmp_save, sizeof(jmp_save));
	if ((rv = setjmp(jmp_save)) != 0) {
		clean_up();
		if (rv == 14) {
			error(_("Expression too large."));
		}
	} else {
		while ((cp = fgets((char *) tlhs, n_tokens * sizeof(token_type), fp)) != NULL) {
			default_color();
			input_column = printf("%d%s", cur_equation + 1, html_flag ? HTML_PROMPT : PROMPT);
			printf("%s", cp);
			set_error_level(cp);
			if (!process(cp)) {
				longjmp(jmp_save, 3);
			}
		}
	}
	blt(jmp_save, save_save, sizeof(jmp_save));
	return rv;
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
		my_strlcpy(tmp_file, "math.tmp", sizeof(tmp_file));
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
		if (pause_cmd(_("Prepare to run the editor"))) {
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
#if	!SILENT
	if (access(cp, 0) == 0) {
		snprintf(prompt_str, sizeof(prompt_str), _("\"%s\" exists.  Overwrite (Y/N)? "), cp);
		if (!get_yes_no()) {
			printf(_("Command aborted.\n"));
			return false;
		}
	}
#endif
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
#if	!SILENT
		printf(_("All equations saved in file: \"%s\".\n"), cp);
#endif
	} else {
		error(_("Error encountered while saving equations."));
	}
	return rv;
}
#endif
