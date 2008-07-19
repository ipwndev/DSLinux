/*
 * Miscellaneous routines for Mathomatic.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

/*
 * Check if a floating point math function flagged an error.
 */
check_err()
{
	switch (errno) {
	case EDOM:
		errno = 0;
		if (domain_check) {
			domain_check = false;
		} else {
			error(_("Domain error in constant."));
			longjmp(jmp_save, 2);
		}
		break;
	case ERANGE:
		errno = 0;
		error(_("Overflow error in constant."));
		longjmp(jmp_save, 2);
		break;
	}
}

/*
 * Erase all equation spaces.
 * Similar to a restart.
 */
clear_all()
{
	int	i;

	cur_equation = 0;
	for (i = 0; i < n_equations; i++) {
		n_lhs[i] = 0;
		n_rhs[i] = 0;
	}
	for (i = 0; var_names[i]; i++) {
		free(var_names[i]);
		var_names[i] = NULL;
	}
	clear_sign_array();
}

/*
 * Allocate the needed temp storage.
 *
 * Return true if successful.
 */
int
init_mem()
{
	if ((scratch = (token_type *) malloc(((n_tokens * 3) / 2) * sizeof(token_type))) == NULL
	    || (tes = (token_type *) malloc(n_tokens * sizeof(token_type))) == NULL
	    || (tlhs = (token_type *) malloc(n_tokens * sizeof(token_type))) == NULL
	    || (trhs = (token_type *) malloc(n_tokens * sizeof(token_type))) == NULL) {
		return false;
	}
	if (alloc_next_espace() < 0) {	/* make sure there is at least 1 equation space */
		return false;
	}
	return true;
}

/*
 * Initialize the global variables.
 */
init_gvars()
{
	domain_check = false;
	high_prec = false;
	partial_flag = true;
	symb_flag = false;
	sign_flag = false;
	approximate_roots = false;

	zero_token.level = 1;
	zero_token.kind = CONSTANT;
	zero_token.token.constant = 0.0;

	one_token.level = 1;
	one_token.kind = CONSTANT;
	one_token.token.constant = 1.0;
}

/*
 * Clean up when processing is unexpectedly
 * interrupted or terminated.
 */
clean_up()
{
	init_gvars();
	if (gfp != stdout) {
#if	!SECURE
		fclose(gfp);
#endif
		gfp = stdout;
	}
}

/*
 * Restart sign array.
 */
clear_sign_array()
{
	int	i;

	for (i = 0; i < ARR_CNT(sign_array); i++) {
		sign_array[i] = false;
	}
}

/*
 * Register all sign variables.
 */
set_sign_array()
{
	int	i, j;

	clear_sign_array();
	for (i = 0; i < n_equations; i++) {
		if (n_lhs[i] > 0) {
			for (j = 0; j < n_lhs[i]; j += 2) {
				if (lhs[i][j].kind == VARIABLE
				    && (lhs[i][j].token.variable & VAR_MASK) == SIGN) {
					sign_array[(lhs[i][j].token.variable >> VAR_SHIFT) & SUBSCRIPT_MASK] = true;
				}
			}
			for (j = 0; j < n_rhs[i]; j += 2) {
				if (rhs[i][j].kind == VARIABLE
				    && (rhs[i][j].token.variable & VAR_MASK) == SIGN) {
					sign_array[(rhs[i][j].token.variable >> VAR_SHIFT) & SUBSCRIPT_MASK] = true;
				}
			}
		}
	}
}

/*
 * Return next unused sign variable in "*vp".
 * Mark it used.
 */
int
next_sign(vp)
long	*vp;
{
	int	i;

	for (i = 2;; i++) {
		if (i >= ARR_CNT(sign_array)) {
			/* out of unique sign variables */
			*vp = SIGN;
			return false;
		}
		if (!sign_array[i]) {
			*vp = SIGN + (((long) i) << VAR_SHIFT);
			sign_array[i] = true;
			break;
		}
	}
	return true;
}

/*
 * Allocate or reuse an empty equation space.
 *
 * Returns new equation space number or -1 on error.
 */
int
alloc_next_espace()
{
	int	i, n;

	for (n = cur_equation, i = 1;; n = (n + 1) % n_equations, i++) {
		if (i > n_equations) {
			i = n_equations;
			if (i < N_EQUATIONS) {
				lhs[i] = (token_type *) malloc(n_tokens * sizeof(token_type));
				if (lhs[i]) {
					rhs[i] = (token_type *) malloc(n_tokens * sizeof(token_type));
					if (rhs[i] == NULL) {
						free(lhs[i]);
						lhs[i] = NULL;
					}
				}
			}
			if (i >= N_EQUATIONS || lhs[i] == NULL) {
				return -1;
			}
			n_equations++;
			return i;
		}
		if (n_lhs[n] == 0)
			break;
	}
	n_rhs[n] = 0;
	return n;
}

/*
 * Return the number of the next empty equation space.
 */
int
next_espace()
{
	int	i;

	i = alloc_next_espace();
	if (i < 0) {
		error(_("Out of free equation spaces."));
#if	!SILENT
		printf(_("Please use the clear command on unnecessary equations and try again.\n"));
#endif
		longjmp(jmp_save, 3);
	}
	return i;
}

/*
 * Copy equation number "src" to equation number "dest".
 */
copy_espace(src, dest)
int	src, dest;
{
	blt(lhs[dest], lhs[src], n_lhs[src] * sizeof(token_type));
	n_lhs[dest] = n_lhs[src];
	blt(rhs[dest], rhs[src], n_rhs[src] * sizeof(token_type));
	n_rhs[dest] = n_rhs[src];
}

/*
 * Return true if equation number "i" is solved for a normal variable.
 */
int
solved_equation(i)
int	i;
{
	if (n_rhs[i] <= 0)
		return false;
	if (n_lhs[i] != 1 || lhs[i][0].kind != VARIABLE || (lhs[i][0].token.variable & VAR_MASK) <= SIGN)
		return false;
	if (found_var(rhs[i], n_rhs[i], lhs[i][0].token.variable))
		return false;
	return true;
}

/*
 * Return true if variable "v" exists in expression.
 */
int
found_var(equation, n, v)
token_type	*equation;
int		n;
long		v;
{
	int	j;

	for (j = 0; j < n; j += 2) {
		if (equation[j].kind == VARIABLE && equation[j].token.variable == v) {
			return true;
		}
	}
	return false;
}

/*
 * Return true if variable "v" exists in equation number "i".
 */
int
var_in_equation(i, v)
int	i;
long	v;
{
	if (n_lhs[i] <= 0)
		return false;
	if (found_var(lhs[i], n_lhs[i], v))
		return true;
	if (found_var(rhs[i], n_rhs[i], v))
		return true;
	return false;
}

/*
 * Substitute every instance of "v" in "equation" with "expression".
 */
subst_var_with_exp(equation, np, expression, len, v)
token_type	*equation;	/* equation side pointer */
int		*np;		/* pointer to equation side length */
token_type	*expression;	/* expression pointer */
int		len;		/* expression length */
long		v;		/* variable to substitute with expression */
{
	int	j, k;
	int	level;

	for (j = *np - 1; j >= 0; j--) {
		if (equation[j].kind == VARIABLE && equation[j].token.variable == v) {
			level = equation[j].level;
			if (*np + len - 1 > n_tokens) {
				error_huge();
			}
			blt(&equation[j+len], &equation[j+1], (*np - (j + 1)) * sizeof(token_type));
			*np += len - 1;
			blt(&equation[j], expression, len * sizeof(token_type));
			for (k = j; k < j + len; k++)
				equation[k].level += level;
		}
	}
}

/*
 * Return the minimum level encountered in a sub-expression.
 */
int
min_level(equation, n)
token_type	*equation;	/* expression pointer */
int		n;		/* expression length */
{
	int		min1;
	token_type	*p1, *ep;

	if (n <= 1) {
		if (n <= 0) {
			return 1;
		} else {
			return equation[0].level;
		}
	}
	min1 = equation[1].level;
	ep = &equation[n];
	for (p1 = &equation[3]; p1 < ep; p1 += 2) {
		if (p1->level < min1)
			min1 = p1->level;
	}
	return min1;
}

/*
 * This is called when the maximum expression size has been exceeded.
 *
 * There is no return.
 */
error_huge()
{
	longjmp(jmp_save, 14);
}

/*
 * Get default equation number from a command parameter string.
 * The equation number must be the only parameter.
 *
 * Return -1 on error.
 */
int
get_default_en(cp)
char	*cp;
{
	int	i;

	if (*cp == '\0') {
		i = cur_equation;
	} else {
		i = decstrtol(cp, &cp) - 1;
		if (extra_characters(cp))
			return -1;
	}
	if (not_defined(i)) {
		return -1;
	}
	return i;
}

/*
 * Parse an expression with equation space pull if the line starts with "#".
 *
 * Returns the new string position or NULL on error.
 */
char	*
parse_expr(equation, np, cp)
token_type	*equation;	/* where the parsed expression is stored (equation side) */
int		*np;		/* pointer to the returned parsed expression length */
char		*cp;		/* string to parse */
{
	int	i;
	char	*cp1, *cp2;

	cp1 = skip_space(cp);
	if (*cp1 == '#') {
		cp1++;
		switch (*cp1) {
		case '+':
		case '-':
			i = decstrtol(cp1, &cp2);
			i = cur_equation + i;
			break;
		default:
			i = decstrtol(cp1, &cp2) - 1;
			break;
		}
		if (cp1 == cp2 || *cp2) {
			error(_("Error parsing equation space number."));
			return NULL;
		}
		if (i < 0 || i >= n_equations || n_lhs[i] <= 0) {
			error(_("Equation not defined."));
			return NULL;
		}
		if (n_rhs[i]) {
			blt(equation, rhs[i], n_rhs[i] * sizeof(token_type));
			*np = n_rhs[i];
		} else {
			blt(equation, lhs[i], n_lhs[i] * sizeof(token_type));
			*np = n_lhs[i];
		}
		list_proc(equation, *np, false);
		fprintf(gfp, "\n");
		return cp2;
	}
	return parse_section(equation, np, cp);
}

/*
 * Get an expression from the user.
 * The prompt must be previously copied into the global prompt_str[].
 *
 * Return true if successful.
 */
int
get_expr(equation, np)
token_type	*equation;	/* where the parsed expression is stored (equation side) */
int		*np;		/* pointer to the returned parsed expression length */
{
	char	buf[DEFAULT_N_TOKENS];
	char	*cp;

	for (;;) {
		if ((cp = get_string(buf, sizeof(buf))) == NULL) {
			return false;
		}
		if (!case_sensitive_flag) {
			str_tolower(cp);
		}
		cp = parse_expr(equation, np, cp);
		if (cp && !extra_characters(cp)) {
			break;
		}
	}
	return(*np > 0);
}

/*
 * Prompt for a variable from the user.
 *
 * Return true if successful.
 */
int
prompt_var(vp)
long	*vp;	/* pointer to the returned variable */
{
	char	buf[MAX_CMD_LEN];
	char	*cp;

	for (;;) {
		my_strlcpy(prompt_str, _("Enter variable: "), sizeof(prompt_str));
		if ((cp = get_string(buf, sizeof(buf))) == NULL) {
			return false;
		}
		if (*cp == '\0') {
			return false;
		}
		cp = parse_var2(vp, cp);
		if (cp == NULL || extra_characters(cp)) {
			continue;
		}
		return true;
	}
}

/*
 * Return true and display a message if equation "i" is undefined.
 */
int
not_defined(i)
int	i;	/* equation space number */
{
	if (i < 0 || i >= n_equations) {
		error(_("Invalid equation number."));
		return true;
	}
	if (n_lhs[i] <= 0) {
		error(_("Equation space empty."));
		return true;
	}
	return false;
}

/*
 * Verify that a current equation or expression is loaded.
 *
 * Return true and display a message if current equation space is empty.
 */
int
current_not_defined()
{
	int	i;

	i = cur_equation;
	if (i < 0 || i >= n_equations || n_lhs[i] <= 0) {
		error(_("No current equation or expression."));
		return true;
	}
	return false;
}

/*
 * Common routine to output the prompt in prompt_str[]
 * and get a line of input from stdin.
 * All Mathomatic input comes from this routine,
 * except for file reading.
 *
 * Returns "string" if successful or NULL on error.
 */
char	*
get_string(string, n)
char	*string;	/* storage for input string */
int	n;		/* maximum size of "string" in bytes */
{
#if	LIBRARY
	error(_("Missing command line argument."));
	return NULL;
#else
#if	READLINE
	char	*cp;
#endif

	if (quiet_mode) {
		prompt_str[0] = '\0';	/* don't display a prompt */
	}
	input_column = strlen(prompt_str);
#if	READLINE
	if (readline_enabled) {
		cp = readline(prompt_str);
		if (cp == NULL)
			exit_program(0);
		my_strlcpy(string, cp, n);
		if (string[0]) {
			add_history(cp);
		} else {
			free(cp);
		}
	} else {
		printf("%s", prompt_str);
		if (fgets(string, n, stdin) == NULL)
			exit_program(0);
	}
#else
	printf("%s", prompt_str);
	if (fgets(string, n, stdin) == NULL)
		exit_program(0);
#endif
	set_error_level(string);
	return string;
#endif
}

/*
 * Display the prompt in prompt_str[] and wait for "y" or "n".
 *
 * Return true if "y".
 */
int
get_yes_no()
{
	char	*cp;
	char	buf[MAX_CMD_LEN];

	for (;;) {
		if ((cp = get_string(buf, sizeof(buf))) == NULL) {
			return false;
		}
		str_tolower(cp);
		switch (*cp) {
		case 'n':
			return false;
		case 'y':
			return true;
		}
	}
}

/*
 * Store or display the result of a command.
 *
 * Return true if successful.
 */
int
return_result(en)
int	en;	/* equation space number */
{
#if	LIBRARY
	if (gfp == stdout) {
		if (display2d) {
			make_fractions_and_group(en);
		}
		if (factor_int_flag) {
			factor_int_sub(en);
		}
		free(result_str);
		result_str = list_equation(en, false);
		return(result_str != NULL);
	}
#endif
	return list_sub(en);
}

/*
 * Return true if the first word in the passed string is "all".
 */
int
is_all(cp)
char	*cp;
{
	return(strcmp_tospace(cp, "all") == 0);
}

/*
 * Process an equation number range given in text string *cpp.
 * Skip past all spaces and update *cpp to point to the next argument.
 *
 * Return true if successful,
 * with the starting equation number in "*ip"
 * and ending equation number in "*jp".
 */
int
get_range(cpp, ip, jp)
char	**cpp;
int	*ip, *jp;
{
	int	i;

	*cpp = skip_space(*cpp);
#if	LIBRARY		/* only allow a single equation number when using as a library */
	if (isdigit(**cpp)) {
		i = decstrtol(*cpp, cpp) - 1;
	} else {
		i = cur_equation;
	}
	if (not_defined(i)) {
		return false;
	}
	*ip = i;
	*jp = i;
	return true;
#else
	if (is_all(*cpp)) {
		*cpp = skip_param(*cpp);
		*ip = 0;
		*jp = n_equations - 1;
	} else {
		if (isdigit(**cpp)) {
			*ip = strtol(*cpp, cpp, 10) - 1;
		} else {
			*ip = cur_equation;
		}
		if (*ip < 0 || *ip >= n_equations) {
			error(_("Invalid equation number."));
			return false;
		}
		if (**cpp != '-') {
			if (not_defined(*ip)) {
				return false;
			}
			*jp = *ip;
			*cpp = skip_space(*cpp);
			return true;
		}
		(*cpp)++;
		if (isdigit(**cpp)) {
			*jp = strtol(*cpp, cpp, 10) - 1;
		} else {
			*jp = cur_equation;
		}
		if (*jp < 0 || *jp >= n_equations) {
			error(_("Invalid equation number."));
			return false;
		}
		if (*jp < *ip) {
			i = *ip;
			*ip = *jp;
			*jp = i;
		}
	}
	*cpp = skip_space(*cpp);
	for (i = *ip; i <= *jp; i++) {
		if (n_lhs[i] > 0) {
			return true;
		}
	}
	error(_("No equations defined in specified range."));
	return false;
#endif
}

/*
 * This function is provided to make sure there is nothing else
 * on a command line.
 *
 * Returns false if OK.
 * Returns true if any non-space characters were encountered
 * and an error message was printed.
 */
int
extra_characters(cp)
char	*cp;	/* command line */
{
	if (cp) {
		cp = skip_space(cp);
		if (*cp) {
			error(_("Extra characters encountered."));
			return true;
		}
	}
	return false;
}

/*
 * get_range() if it is the last possible option on the command line.
 */
int
get_range_eol(cpp, ip, jp)
char	**cpp;
int	*ip, *jp;
{
	if (!get_range(cpp, ip, jp)) {
		return false;
	}
	if (extra_characters(*cpp)) {
		return false;
	}
	return true;
}

/*
 * Skip over space characters.
 */
char	*
skip_space(cp)
char	*cp;	/* character pointer */
{
	while (*cp && isspace(*cp))
		cp++;
	return cp;
}

/*
 * Enhanced decimal strtol().
 * Skips trailing spaces.
 */
long
decstrtol(cp, cpp)
char	*cp, **cpp;
{
	long	l;

	l = strtol(cp, cpp, 10);
	if (cp != *cpp) {
		*cpp = skip_space(*cpp);
	}
	return l;
}

/*
 * Skip over a parameter (command line argument) in a character string.
 * Parameters are separated with spaces.
 *
 * Returns a string pointer to the next parameter.
 */
char	*
skip_param(cp)
char	*cp;	/* character pointer */
{
	while (*cp && (!isascii(*cp) || (!isspace(*cp) && *cp != '='))) {
		cp++;
	}
	cp = skip_space(cp);
	if (*cp == '=') {
		cp = skip_space(cp + 1);
	}
	return(cp);
}

/*
 * Compare strings up to the first space.
 * Must be an exact match.
 * Compare is alphabetic case insensitive.
 *
 * Returns zero on match, non-zero if different.
 */
int
strcmp_tospace(cp1, cp2)
char	*cp1, *cp2;
{
	char	*cp1a, *cp2a;

	for (cp1a = cp1; *cp1a && !isspace(*cp1a); cp1a++)
		;
	for (cp2a = cp2; *cp2a && !isspace(*cp2a); cp2a++)
		;
	return strncasecmp(cp1, cp2, max(cp1a - cp1, cp2a - cp2));
}

/*
 * Return the number of "level" additive type operators.
 */
int
level_plus_count(p1, n1, level)
token_type	*p1;	/* expression pointer */
int		n1;	/* expression length */
int		level;	/* parentheses level number */
{
	int	i;
	int	count;

	count = 0;
	for (i = 1; i < n1; i += 2) {
		if (p1[i].level == level) {
			switch (p1[i].token.operatr) {
			case PLUS:
			case MINUS:
				count++;
			}
		}
	}
	return count;
}

/*
 * Return the number of level 1 additive type operators.
 */
int
level1_plus_count(p1, n1)
token_type	*p1;	/* expression pointer */
int		n1;	/* expression length */
{
	return level_plus_count(p1, n1, min_level(p1, n1));
}

/*
 * Return the count of variables in an expression.
 */
int
var_count(p1, n1)
token_type	*p1;	/* expression pointer */
int		n1;	/* expression length */
{
	int	i;
	int	count;

	count = 0;
	for (i = 0; i < n1; i += 2) {
		if (p1[i].kind == VARIABLE) {
			count++;
		}
	}
	return count;
}

/*
 * Set *vp if single variable expression.
 *
 * Return true if expression contains no variables.
 */
int
no_vars(source, n, vp)
token_type	*source;	/* expression pointer */
int		n;		/* expression length */
long		*vp;		/* variable pointer */
{
	int	j;
	int	found = false;

	if (*vp) {
		return(var_count(source, n) == 0);
	}
	for (j = 0; j < n; j += 2) {
		if (source[j].kind == VARIABLE) {
			if ((source[j].token.variable & VAR_MASK) <= SIGN)
				continue;
			if (*vp) {
				if (*vp != source[j].token.variable) {
					*vp = 0;
					break;
				}
			} else {
				found = true;
				*vp = source[j].token.variable;
			}
		}
	}
	return(!found);
}

/*
 * Return true if expression contains infinity or nan.
 */
int
exp_contains_infinity(p1, n1)
token_type	*p1;	/* expression pointer */
int		n1;	/* expression length */
{
	int	i;

	for (i = 0; i < n1; i += 2) {
		if (p1[i].kind == CONSTANT && !isfinite(p1[i].token.constant)) {
			return true;
		}
	}
	return false;
}
