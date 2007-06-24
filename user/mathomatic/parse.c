/*
 * Expression parsing routines for Mathomatic.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

#define	INFINITY_NAME	"inf"	/* set this to the name of the infinity constant as displayed by printf(3) */

int	point_flag;		/* point to location of parse error if true */

/*
 * Return true if character is a valid starting variable character.
 */
int
isvarchar(ch)
int	ch;
{
	return(ch == '_' || (special_variable_character && ch == special_variable_character) || isalpha(ch));
}

/*
 * Parse an equation string into equation space "n".
 *
 * Returns the new string position or NULL on error.
 */
char	*
parse_equation(n, cp)
int	n;	/* equation space number */
char	*cp;	/* pointer to the beginning of the equation character string */
{
	if (!case_sensitive_flag) {
		str_tolower(cp);
	}
	if ((cp = parse_section(lhs[n], &n_lhs[n], cp)) != NULL) {
		if ((cp = parse_section(rhs[n], &n_rhs[n], cp)) != NULL) {
			if (extra_characters(cp))
				return NULL;
			return cp;
		}
	}
	return NULL;
}

/*
 * This is a simple, non-recursive mathematical expression parser.
 * To parse, the character string is sequentially read and stored
 * in the internal storage format.
 * Any syntax errors are carefully reported with a NULL return.
 *
 * Returns the new string position or NULL on error.
 */
char	*
parse_section(equation, np, cp)
token_type	*equation;	/* where the parsed expression is stored (equation side) */
int		*np;		/* pointer to the returned parsed expression length */
char		*cp;		/* string to parse */
{
	int		n = 0;		/* position in equation[] */
	int		cur_level = 1;	/* current level of parentheses */
	int		operand;	/* flip-flop between operand and operator */
	char		*cp_start, *cp1;
	double		d;
	int		abs_count = 0;
	int		abs_array[10];

	if (cp == NULL)
		return(NULL);
	cp_start = cp;
	operand = false;
	for (;; cp++) {
		switch (*cp) {
		case ' ':
		case '\t':
			continue;
		case '(':
		case '[':
		case '{':
			cur_level++;
			if (operand) {
				goto syntax_error;
			}
			continue;
		case ')':
		case ']':
		case '}':
			cur_level--;
			if (cur_level <= 0 || (abs_count > 0 && cur_level < abs_array[abs_count-1])) {
				put_up_arrow((int) (cp - cp_start), _("Too many right parentheses."));
				return(NULL);
			}
			if (!operand) {
				goto syntax_error;
			}
			continue;
		case '=':
		case ';':
		case 0:
		case '\n':
			goto p_out;
		}
		if (n > (n_tokens - 6)) {
			error_huge();
		}
		operand = !operand;
		switch (*cp) {
		case '|':
			if (operand) {
				if (abs_count >= ARR_CNT(abs_array)) {
					error(_("Too many nested absolute values."));
					return(NULL);
				}
				cur_level += 2;
				abs_array[abs_count++] = cur_level;
			} else {
				if (abs_count <= 0 || cur_level != abs_array[--abs_count]) {
					goto syntax_error;
				}
				cur_level--;
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = POWER;
				n++;
				equation[n].level = cur_level;
				equation[n].kind = CONSTANT;
				equation[n].token.constant = 2.0;
				n++;
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = POWER;
				n++;
				equation[n].level = cur_level;
				equation[n].kind = CONSTANT;
				equation[n].token.constant = 0.5;
				n++;
				cur_level--;
			}
			operand = !operand;
			break;
		case '!':
			if (operand) {
				goto syntax_error;
			}
			if (cp[1] == '!') {
				put_up_arrow((int) (cp - cp_start), _("Multifactorial not supported yet."));
				return(NULL);
			}
			equation[n].level = cur_level;
			equation[n].kind = OPERATOR;
			equation[n].token.operatr = FACTORIAL;
			n++;
			equation[n].level = cur_level;
			equation[n].kind = CONSTANT;
			equation[n].token.constant = 1.0;
			n++;
			operand = true;
			break;
		case '^':
parse_power:
			if (operand) {
				goto syntax_error;
			}
			equation[n].level = cur_level;
			equation[n].kind = OPERATOR;
			equation[n].token.operatr = POWER;
			n++;
			break;
		case '*':
			if (cp[1] == '*') {	/* for "**" */
				cp++;
				goto parse_power;
			}
			if (operand) {
				goto syntax_error;
			}
			equation[n].level = cur_level;
			equation[n].kind = OPERATOR;
			equation[n].token.operatr = TIMES;
			n++;
			break;
		case '/':
			if (operand) {
				goto syntax_error;
			}
			equation[n].level = cur_level;
			equation[n].kind = OPERATOR;
			equation[n].token.operatr = DIVIDE;
			n++;
			break;
		case '%':
			if (operand) {
				goto syntax_error;
			}
			equation[n].level = cur_level;
			equation[n].kind = OPERATOR;
			equation[n].token.operatr = MODULUS;
			n++;
			break;
		case '+':
		case '-':
			if (!operand) {
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = ((*cp == '+') ? PLUS : MINUS);
				n++;
			}
			if (strncmp(cp, "+/-", 3) == 0) {
				equation[n].level = cur_level;
				equation[n].kind = VARIABLE;
				next_sign(&equation[n].token.variable);
				n++;
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
				cp += 2;
				operand = false;
				break;
			}
			if (!operand) {
				break;
			}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			if (!operand) {
				operand = true;
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
			}
			if (*cp == '-' && !(isdigit(cp[1]) || cp[1] == '.')) {
				equation[n].kind = CONSTANT;
				equation[n].token.constant = -1.0;
				equation[n].level = cur_level;
				n++;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = NEGATE;
				equation[n].level = cur_level;
				n++;
				operand = false;
				continue;
			}
			cp1 = cp;
			errno = 0;
			d = strtod(cp1, &cp);
			if (errno) {
				put_up_arrow((int) (cp1 - cp_start), _("Constant out of range."));
				return(NULL);
			}
			if (cp == cp1) {
				put_up_arrow((int) (cp1 - cp_start), _("Error parsing constant."));
				return(NULL);
			}
			equation[n].kind = CONSTANT;
			equation[n].token.constant = d;
			equation[n].level = cur_level;
			n++;
			cp--;
			break;
		default:
			if (!isvarchar(*cp)) {
				put_up_arrow((int) (cp - cp_start), _("Unrecognized character."));
				return(NULL);
			}
			if (!operand) {
				operand = true;
				equation[n].level = cur_level;
				equation[n].kind = OPERATOR;
				equation[n].token.operatr = TIMES;
				n++;
			}
			if (strncmp(cp, INFINITY_NAME, strlen(INFINITY_NAME)) == 0
			    && !isvarchar(cp[strlen(INFINITY_NAME)])) {
				equation[n].kind = CONSTANT;
				equation[n].token.constant = HUGE_VAL;	/* the infinity constant */
				cp += strlen(INFINITY_NAME);
			} else {
				equation[n].kind = VARIABLE;
				cp1 = cp;
				cp = parse_var(&equation[n].token.variable, cp);
				if (cp == NULL) {
					return(NULL);
				}
			}
			cp--;
			equation[n].level = cur_level;
			n++;
			break;
		}
	}
p_out:
	if (abs_count != 0 || (n && !operand)) {
		goto syntax_error;
	}
	if (cur_level != 1) {
		put_up_arrow((int) (cp - cp_start), _("Missing right parenthesis."));
		return(NULL);
	}
	if (*cp == '=')
		cp++;
	*np = n;
	if (n) {
		handle_negate(equation, np);
		prior_sub(equation, np);
		organize(equation, np);
	}
	input_column += (cp - cp_start);
	return cp;

syntax_error:
	put_up_arrow((int) (cp - cp_start), _("Syntax error."));
	return(NULL);
}

/*
 * Parse variable name string pointed to by "cp".
 * The variable name is converted to Mathomatic format and stored in "*vp".
 *
 * If the variable is not special and never existed before, it is created.
 *
 * Return new string position if successful.
 * Display error message and return NULL on failure.
 */
char	*
parse_var(vp, cp)
long	*vp;
char	*cp;
{
	int	i, j;
	long	vtmp;
	char	buf[MAX_VAR_LEN+1];
	char	*cp1;
	int	len;
	int	(*strcmpfunc)();

	*vp = V_NULL;
	if (case_sensitive_flag) {
		strcmpfunc = strcmp;
	} else {
		strcmpfunc = strcasecmp;
	}
	if (!isvarchar(*cp)) {
		error(_("Invalid variable."));
		return(NULL);	/* variable name must start with a valid variable character */
	}
	for (cp1 = cp, i = 0; *cp1;) {
		if (!isvarchar(*cp1)) {
			break;
		}
		if (i >= MAX_VAR_LEN) {
			error(_("Variable name too long."));
			return(NULL);
		}
		buf[i++] = *cp1++;
	}
	buf[i] = '\0';
	if ((*strcmpfunc)(buf, INFINITY_NAME) == 0) {
		error(_("Invalid variable."));
		return(NULL);
	} else if ((*strcmpfunc)(buf, "sign") == 0) {
		vtmp = SIGN;
	} else {
		if (strncasecmp(cp, "i#", 2) == 0) {
			*vp = IMAGINARY;
			return(cp + 2);
		}
		if (strncasecmp(cp, "e#", 2) == 0) {
			*vp = V_E;
			return(cp + 2);
		}
		if (strncasecmp(cp, "pi#", 3) == 0) {
			*vp = V_PI;
			return(cp + 3);
		}
		for (cp1 = cp, i = 0; *cp1;) {
			if (!isvarchar(*cp1) && !isdigit(*cp1)) {
				break;
			}
			if (i >= MAX_VAR_LEN) {
				error(_("Variable name too long."));
				return(NULL);
			}
			buf[i++] = *cp1++;
		}
		buf[i] = '\0';
		if ((*strcmpfunc)(buf, "i") == 0) {
			*vp = IMAGINARY;
			return(cp1);
		}
		if ((*strcmpfunc)(buf, "e") == 0) {
			*vp = V_E;
			return(cp1);
		}
		if ((*strcmpfunc)(buf, "pi") == 0) {
			*vp = V_PI;
			return(cp1);
		}
		vtmp = 0;
		for (i = 0; var_names[i]; i++) {
			if ((*strcmpfunc)(buf, var_names[i]) == 0) {
				vtmp = i + VAR_OFFSET;
				break;
			}
		}
		if (vtmp == 0) {
			if (i >= (MAX_VAR_NAMES - 1)) {
				error(_("Maximum number of variable names reached."));
#if	!SILENT
				printf(_("Please restart or use \"clear all\".\n"));
#endif
				return(NULL);
			}
			len = strlen(buf) + 1;
			var_names[i] = (char *) malloc(len);
			if (var_names[i] == NULL) {
				error(_("Out of memory (can't malloc(3) variable name)."));
				return(NULL);
			}
			blt(var_names[i], buf, len);
			vtmp = i + VAR_OFFSET;
			var_names[i+1] = NULL;
		}
		*vp = vtmp;
		return cp1;
	}
	if (isdigit(*cp1)) {
		j = strtol(cp1, &cp1, 10);
		if (j < 0 || j > MAX_SUBSCRIPT) {
			error(_("Maximum subscript exceeded in special variable name."));
			return(NULL);
		}
		if (vtmp == SIGN) {
			sign_array[j+1] = true;
		}
		vtmp += ((long) (j + 1)) << VAR_SHIFT;
	}
	*vp = vtmp;
	return cp1;
}

/*
 * Set point_flag if pointing to the input error works for the passed string.
 * Truncate string to the actual content.
 */
set_error_level(cp)
char	*cp;	/* input string */
{
	char	*cp1;

	point_flag = true;
/* handle comments */
	for (cp1 = cp; *cp1; cp1++) {
		if (*cp1 == ';') {
			*cp1 = '\0';
			break;
		}
	}
/* remove trailing newlines, carriage returns, and spaces */
	cp1 = &cp[strlen(cp)];
	while (cp1 > cp) {
		cp1--;
		if (*cp1 == '\n' || *cp1 == '\r' || isspace(*cp1)) {
			*cp1 = '\0';
		} else {
			break;
		}
	}
/* set point_flag to false if non-printable characters encountered */
	for (cp1 = cp; *cp1; cp1++) {
		if (!isprint(*cp1)) {
			point_flag = false;
		}
	}
}

/*
 * Display an up arrow pointing to the error under the input string.
 */
put_up_arrow(cnt, cp)
int	cnt;	/* position of error, relative to "input_column" */
char	*cp;	/* error message */
{
#if	!SILENT
	int	 i;

	if (isatty(0) && point_flag) {
		for (i = 0; i < (input_column + cnt); i++) {
			printf(" ");
		}
		printf("^ ");
	}
#endif
	error(cp);
}

/*
 * Return true if passed variable is a constant.
 * Return value of constant in "*dp".
 */
int
var_is_const(v, dp)
long	v;
double	*dp;
{
	switch (v) {
	case V_E:
		*dp = M_E;
		return true;
	case V_PI:
		*dp = M_PI;
		return true;
	}
	return false;
}

/*
 * Substitute E and PI variables with their respective constants.
 */
int
subst_constants(equation, np)
token_type	*equation;
int		*np;
{
	int	i;
	int	modified = false;
	double	d;

	for (i = 0; i < *np; i += 2) {
		if (equation[i].kind == VARIABLE) {
			if (var_is_const(equation[i].token.variable, &d)) {
				equation[i].kind = CONSTANT;
				equation[i].token.constant = d;
				modified = true;
			}
		}
	}
	return modified;
}

/*
 * Parenthesize an operator.
 */
binary_parenthesize(equation, n, i)
token_type	*equation;	/* pointer to expression */
int		n;		/* length of expression */
int		i;		/* location of operator to parenthesize in expression */
{
	int	j;
	int	level;

	level = equation[i].level++;
	if (equation[i-1].level++ > level) {
		for (j = i - 2; j >= 0; j--) {
			if (equation[j].level <= level)
				break;
			equation[j].level++;
		}
	}
	if (equation[i+1].level++ > level) {
		for (j = i + 2; j < n; j++) {
			if (equation[j].level <= level)
				break;
			equation[j].level++;
		}
	}
}

/*
 * Handle and remove the special NEGATE operator.
 */
handle_negate(equation, np)
token_type	*equation;
int		*np;
{
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == NEGATE) {
			equation[i].token.operatr = TIMES;
			binary_parenthesize(equation, *np, i);
		}
	}
}

/*
 * Give different operators on the same level
 * in an expression the correct priority.
 *
 * organize() should be called after this.
 */
prior_sub(equation, np)
token_type	*equation;
int		*np;
{
	int	i;

	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == FACTORIAL) {
			binary_parenthesize(equation, *np, i);
		}
	}
	for (i = 1; i < *np; i += 2) {
		if (equation[i].token.operatr == POWER) {
			binary_parenthesize(equation, *np, i);
		}
	}
	for (i = 1; i < *np; i += 2) {
		switch (equation[i].token.operatr) {
		case TIMES:
		case DIVIDE:
		case MODULUS:
			binary_parenthesize(equation, *np, i);
			break;
		}
	}
}

/*
 * Convert all alphabetic characters in a string to lower case.
 */
str_tolower(cp)
char	*cp;
{
	for (; *cp; cp++) {
		if (isupper(*cp))
			*cp = tolower(*cp);
	}
}

#ifndef	my_strlcpy
/*
 * A very efficient strlcpy().
 *
 * Copy up to (n - 1) characters from string in src
 * to dest and null-terminate the result.
 *
 * Return length of src.
 */
int
my_strlcpy(dest, src, n)
char	*dest, *src;
int	n;
{
	int	len, len_src;

	len_src = strlen(src);
	len = min(n - 1, len_src);
	memmove(dest, src, len);
	dest[len] = '\0';
	return len_src;
}
#endif
