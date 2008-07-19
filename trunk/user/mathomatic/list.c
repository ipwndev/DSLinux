/*
 * Mathomatic expression and equation display routines.
 * Color mode routines, too.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

#define	EQUATE_STRING	" = "	/* string displayed between the LHS and RHS of equations */

#define	APPEND(str)	{ if (string) { strcpy(&string[len], str); } len += strlen(str); }

static int	flist();
static int	flist_recurse();

/* ANSI color code array */
static int	carray[] = {
	32,	/* green (default color) */
	33,	/* yellow */
	31,	/* red */
	35,	/* magenta */
	34,	/* blue */
	36,	/* cyan */
	37	/* white */
};

/* HTML color array */
static char	*html_carray[] = {
	"#00FF00",	/* bright green (default color) */
	"#FFFF00",
	"#FF9000",
	"#FF0000",
	"#FF00FF",
	"#00FFFF",
	"#0000FF"
};

/* global variables for flist() */
int	cur_line;	/* current line */
int	cur_pos;	/* current position in the current line on the screen */

/*
 * Turn color off if color mode is on.
 */
reset_attr()
{
	cur_color = -1;
	if (color_flag) {
		if (html_flag) {
			printf("</font>");
		} else {
			printf("\033[0m");
		}
	}
	fflush(NULL);	/* flush all output */
}

/*
 * Set the current color on the display.
 * Range is 0 to INT_MAX.
 */
set_color(color)
int	color;
{
	if (gfp != stdout)
		return;
	if (cur_color == color)
		return;
	if (color_flag) {
		if (html_flag) {
			if (cur_color >= 0) {
				printf("</font>");
			}
			printf("<font color=\"%s\">", html_carray[color%ARR_CNT(html_carray)]);
		} else {
			if (bold_colors) {
				printf("\033[1;%dm", carray[color%ARR_CNT(carray)]);
			} else {
				printf("\033[0;%dm", carray[color%ARR_CNT(carray)]);
			}
		}
		cur_color = color;
	}
}

/*
 * Set the normal text color.
 */
default_color()
{
	set_color(0);
}

/*
 * Display the expression or equation stored in equation space "n" in single-line format.
 * The equation space is not modified.
 *
 * Return true if successful.
 */
int
list1_sub(n, export_flag)
int	n;		/* equation space number */
int	export_flag;	/* true for exportable format (readable by other math programs) */
			/* 1 for Maxima, 2 for other */
{
	if (n_lhs[n] <= 0)
		return false;
	if (!export_flag && !high_prec) {
		fprintf(gfp, "#%d: ", n + 1);
	}
	list_proc(lhs[n], n_lhs[n], export_flag);
	if (n_rhs[n]) {
		fprintf(gfp, EQUATE_STRING);
		list_proc(rhs[n], n_rhs[n], export_flag);
	}
	if (export_flag == 1) {
		fprintf(gfp, ";");
	}
	if (high_prec) {
#if	CYGWIN
		fprintf(gfp, "\r\n");
#else
		fprintf(gfp, "\n");
#endif
	} else {
		fprintf(gfp, "\n\n");
	}
	return true;
}

/*
 * Display the expression or equation stored in equation space "n".
 *
 * Return true if successful.
 */
int
list_sub(n)
int	n;	/* equation space number */
{
	if (n_lhs[n] <= 0)
		return false;
	if (display2d) {
		/* display in fraction format */
		make_fractions_and_group(n);
		if (factor_int_flag) {
			factor_int_sub(n);
		}
		return flist_sub(n);
	} else {
		/* display in single-line format */
		if (factor_int_flag) {
			factor_int_sub(n);
		}
		return list1_sub(n, false);
	}
}

#if	!SILENT
list_debug(level, p1, n1, p2, n2)
int		level;
token_type	*p1;
int		n1;
token_type	*p2;
int		n2;
{
	if (debug_level >= level) {
		printf(_("level %d: "), level);
		list_proc(p1, n1, false);
		if (p2 && n2 > 0) {
			printf(EQUATE_STRING);
			list_proc(p2, n2, false);
		}
		printf("\n");
	}
}
#endif

/*
 * Convert variable "v" to an ASCII variable name.
 * The ASCII variable name is stored in global "var_str[]".
 *
 * Return length of variable name (number of ASCII characters).
 *
 * If "lang_code == 0", use standard Mathomatic format.
 * If "lang_code == 1", make variable name C language compatible.
 * If "lang_code == 2", assume Java.
 * If "lang_code == 3", assume Python.
 * If "lang_code" is negative, create an exportable variable name: -1 for Maxima, -2 for other.
 */
int
list_var(v, lang_code)
long	v;		/* variable to convert */
int	lang_code;	/* language code */
{
	int		j;
	char		*cp, buf[100];

	cp = NULL;
	switch (v & VAR_MASK) {
	case V_NULL:
		cp = "null";
		break;
	case SIGN:
		cp = "sign";
		break;
	case IMAGINARY:
		switch (lang_code) {
		case -2:
			cp = "i";
			break;
		case -1:
			cp = "%i";
			break;
		case 0:
			cp = "i#";
			break;
		case 3:
			cp = "1j";
			break;
		default:
			cp = "I";
			break;
		}
		break;
	case V_E:
		switch (lang_code) {
		case -1:
			cp = "%e";
			break;
		case 0:
			cp = "e#";
			break;
		case 1:
			cp ="M_E";
			break;
		case 2:
			cp = "Math.E";
			break;
		case 3:
			cp = "math.e";
			break;
		default:
			cp = "e";
			break;
		}
		break;
	case V_PI:
		switch (lang_code) {
		case -1:
			cp = "%pi";
			break;
		case 0:
			cp = "pi#";
			break;
		case 1:
			cp = "M_PI";
			break;
		case 2:
			cp = "Math.PI";
			break;
		case 3:
			cp = "math.pi";
			break;
		default:
			cp = "pi";
			break;
		}
		break;
	case MATCH_ANY:
		cp = "all";
		break;
	default:
		j = (v & VAR_MASK) - VAR_OFFSET;
		if (j >= 0) {
			cp = var_names[j];
		}
		break;
	}
	if (cp) {
		my_strlcpy(var_str, cp, sizeof(var_str));
	} else {
		my_strlcpy(var_str, _("bad_variable"), sizeof(var_str));
	}
	j = (v >> VAR_SHIFT) & SUBSCRIPT_MASK;
	if (j) {
		snprintf(buf, sizeof(buf), "%d", j - 1);
		strcat(var_str, buf);
	}
	return(strlen(var_str));
}

/*
 * Display an expression in single line format.
 * Use color if available.
 * The expression is not modified.
 *
 * Return number of characters output (excluding escape sequences).
 */
int
list_proc(equation, n, export_flag)
token_type	*equation;	/* expression pointer */
int		n;		/* length of expression */
int		export_flag;	/* flag for exportable format (usually false) */
				/* 1 for Maxima, 2 for other */
{
	int	i, j, k, i1;
	int	min1;
	int	cur_level;
	char	*cp;
	int	len = 0;

	cur_level = min1 = min_level(equation, n);
	for (i = 0; i < n; i++) {
		j = cur_level - equation[i].level;
		k = abs(j);
		for (i1 = 1; i1 <= k; i1++) {
			if (j > 0) {
				cur_level--;
				len += fprintf(gfp, ")");
				set_color(cur_level-min1);
			} else {
				cur_level++;
				set_color(cur_level-min1);
				len += fprintf(gfp, "(");
			}
		}
		switch (equation[i].kind) {
		case CONSTANT:
			if (equation[i].token.constant == 0.0) {
				equation[i].token.constant = 0.0; /* fix -0 */
			}
			if (high_prec || export_flag) {
				if (equation[i].token.constant < 0.0) {
					len += fprintf(gfp, "(%.20g)", equation[i].token.constant);
				} else {
					len += fprintf(gfp, "%.20g", equation[i].token.constant);
				}
			} else if (finance_option) {
				len += fprintf(gfp, "%.2f", equation[i].token.constant);
			} else {
				len += fprintf(gfp, "%.*g", precision, equation[i].token.constant);
			}
			break;
		case VARIABLE:
			list_var(equation[i].token.variable, 0 - export_flag);
			len += fprintf(gfp, "%s", var_str);
			break;
		case OPERATOR:
			cp = _("(unknown operator)");
			switch (equation[i].token.operatr) {
			case PLUS:
				cp = " + ";
				break;
			case MINUS:
				cp = " - ";
				break;
			case TIMES:
				cp = "*";
				break;
			case DIVIDE:
				cp = "/";
				break;
			case MODULUS:
				cp = " % ";
				break;
			case POWER:
				cp = "^";
				break;
			case FACTORIAL:
				cp = "!";
				i++;
				break;
			}
			len += fprintf(gfp, "%s", cp);
			break;
		}
	}
	for (j = cur_level - min1; j > 0;) {
		len += fprintf(gfp, ")");
		j--;
		set_color(j);
	}
	return len;
}

/*
 * Store the specified equation in a text string.
 * String should be freed with free() when done.
 *
 * Return string or NULL if can't malloc().
 */
char *
list_equation(n, export_flag)
int	n;		/* equation space number */
int	export_flag;	/* flag for exportable format (usually false) */
{
	int	len;
	char	*cp;

	len = list_string_sub(lhs[n], n_lhs[n], NULL, export_flag);
	if (n_rhs[n]) {
		len += strlen(EQUATE_STRING);
		len += list_string_sub(rhs[n], n_rhs[n], NULL, export_flag);
	}
	len += 2;
	cp = (char *) malloc(len);
	if (cp == NULL) {
		error(_("Out of memory (can't malloc(3))."));
		return NULL;
	}
	list_string_sub(lhs[n], n_lhs[n], cp, export_flag);
	if (n_rhs[n]) {
		strcat(cp, EQUATE_STRING);
		list_string_sub(rhs[n], n_rhs[n], &cp[strlen(cp)], export_flag);
	}
	if (export_flag == 1) {
		strcat(cp, ";");
	}
	return cp;
}

/*
 * Store an expression in a text string.
 * String should be freed with free() when done.
 *
 * Return string or NULL if can't malloc().
 */
char *
list_expression(equation, n, export_flag)
token_type	*equation;	/* expression pointer */
int		n;		/* length of expression */
int		export_flag;
{
	int	len;
	char	*cp;

	len = list_string_sub(equation, n, NULL, export_flag);
	len++;
	cp = (char *) malloc(len);
	if (cp == NULL) {
		error(_("Out of memory (can't malloc(3))."));
		return NULL;
	}
	list_string_sub(equation, n, cp, export_flag);
	return cp;
}

/*
 * Convert an expression to a text string and store in "string" if
 * "string" is not NULL.  "string" need not be initialized,
 * but must be long enough to contain the expression.
 *
 * Return length (number of characters).
 */
int
list_string_sub(equation, n, string, export_flag)
token_type	*equation;	/* expression pointer */
int		n;		/* length of expression */
char		*string;	/* buffer or NULL pointer */
int		export_flag;
{
	int	i, j, k, i1;
	int	min1;
	int	cur_level;
	char	*cp;
	int	len = 0;
	char	buf[500];

	if (string)
		string[0] = '\0';
	cur_level = min1 = min_level(equation, n);
	for (i = 0; i < n; i++) {
		j = cur_level - equation[i].level;
		k = abs(j);
		for (i1 = 1; i1 <= k; i1++) {
			if (j > 0) {
				cur_level--;
				APPEND(")");
			} else {
				cur_level++;
				APPEND("(");
			}
		}
		switch (equation[i].kind) {
		case CONSTANT:
			if (equation[i].token.constant == 0.0) {
				equation[i].token.constant = 0.0; /* fix -0 */
			}
			if (high_prec || export_flag) {
				if (equation[i].token.constant < 0.0) {
					snprintf(buf, sizeof(buf), "(%.20g)", equation[i].token.constant);
				} else {
					snprintf(buf, sizeof(buf), "%.20g", equation[i].token.constant);
				}
			} else if (finance_option) {
				snprintf(buf, sizeof(buf), "%.2f", equation[i].token.constant);
			} else {
				snprintf(buf, sizeof(buf), "%.*g", precision, equation[i].token.constant);
			}
			APPEND(buf);
			break;
		case VARIABLE:
			list_var(equation[i].token.variable, 0 - export_flag);
			APPEND(var_str);
			break;
		case OPERATOR:
			cp = _("(unknown operator)");
			switch (equation[i].token.operatr) {
			case PLUS:
				cp = " + ";
				break;
			case MINUS:
				cp = " - ";
				break;
			case TIMES:
				cp = "*";
				break;
			case DIVIDE:
				cp = "/";
				break;
			case MODULUS:
				cp = " % ";
				break;
			case POWER:
				cp = "^";
				break;
			case FACTORIAL:
				cp = "!";
				i++;
				break;
			}
			APPEND(cp);
			break;
		}
	}
	for (j = cur_level - min1; j > 0; j--) {
		APPEND(")");
	}
	return len;
}

/*
 * Return true if expression is a valid integer expression for
 * list_code().
 */
int
int_expr(equation, n)
token_type	*equation;	/* expression pointer */
int		n;		/* length of expression */
{
	int	i;

	for (i = 0; i < n; i++) {
		if ((equation[i].kind == CONSTANT && fmod(equation[i].token.constant, 1.0))
		    || (equation[i].kind == VARIABLE && equation[i].token.variable <= IMAGINARY)) {
			return false;
		}
	}
	return true;
}

/*
 * Display an equation as C, Java, or Python code.
 */
list_c_equation(en, language, int_flag)
int	en;		/* equation space number */
int	language;
int	int_flag;	/* integer arithmetic flag */
{
	if (n_lhs[en] <= 0)
		return;
	list_code(lhs[en], &n_lhs[en], language, int_flag);
	if (n_rhs[en]) {
		fprintf(gfp, " = ");
		list_code(rhs[en], &n_rhs[en], language, int_flag);
	}
	if (language != 2) {
		fprintf(gfp, ";");
	}
	fprintf(gfp, "\n");
}

/*
 * Output C, Java, or Python code for an expression.
 * Expression might be modified by this function.
 *
 * Return length of output (number of characters).
 */
int
list_code(equation, np, language, int_flag)
token_type	*equation;	/* expression pointer */
int		*np;		/* pointer to length of expression */
int		language;	/* if 0, generate C code; if 1, generate Java code; if 2, generate Python code */
int		int_flag;	/* integer arithmetic flag, should work with any language */
{
	int	i, j, k, i1, i2;
	int	min1;
	int	cur_level;
	char	*cp;
	char	buf[500];
	int	len = 0;

	min1 = min_level(equation, *np);
	if (*np > 1)
		min1--;
	cur_level = min1;
	for (i = 0; i < *np; i++) {
		j = cur_level - equation[i].level;
		k = abs(j);
		for (i1 = 1; i1 <= k; i1++) {
			if (j > 0) {
				cur_level--;
				len += fprintf(gfp, ")");
			} else {
				cur_level++;
				for (i2 = i + 1; i2 < *np && equation[i2].level >= cur_level; i2 += 2) {
					if (equation[i2].level == cur_level) {
						switch (equation[i2].token.operatr) {
						case POWER:
							if (equation[i2-1].level == cur_level
							    && equation[i2+1].level == cur_level
							    && equation[i2+1].kind == CONSTANT
							    && equation[i2+1].token.constant == 2.0) {
								equation[i2].token.operatr = TIMES;
								equation[i2+1] = equation[i2-1];
							} else {
								if (int_flag || language > 1)
									break;
								if (language) {
									cp = "Math.pow";
								} else {
									cp = "pow";
								}
								len += fprintf(gfp, cp);
							}
							break;
						case FACTORIAL:
							cp = "fact";
							len += fprintf(gfp, cp);
							break;
						}
						break;
					}
				}
				len += fprintf(gfp, "(");
			}
		}
		switch (equation[i].kind) {
		case CONSTANT:
			if (equation[i].token.constant == 0.0) {
				equation[i].token.constant = 0.0; /* fix -0 */
			}
			if (int_flag) {
				snprintf(buf, sizeof(buf), "%.0f", equation[i].token.constant);
			} else {
				snprintf(buf, sizeof(buf), "%#.*g", MAX_PRECISION, equation[i].token.constant);
				j = strlen(buf) - 1;
				for (; j >= 0; j--) {
					if (buf[j] == '0')
						continue;
					if (buf[j] == '.') {
						buf[j+2] = '\0';
					} else {
						break;
					}
				}
			}
			len += fprintf(gfp, "%s", buf);
			break;
		case VARIABLE:
			len += list_var(equation[i].token.variable, language + 1);
			fprintf(gfp, "%s", var_str);
			break;
		case OPERATOR:
			cp = _("(unknown operator)");
			switch (equation[i].token.operatr) {
			case PLUS:
				cp = " + ";
				break;
			case MINUS:
				cp = " - ";
				break;
			case TIMES:
				cp = " * ";
				break;
			case DIVIDE:
				cp = " / ";
				break;
			case MODULUS:
				cp = " % ";
				break;
			case POWER:
				if (int_flag || language > 1) {
					cp = " ** ";
				} else {
					cp = ", ";
				}
				break;
			case FACTORIAL:
				cp = "";
				i++;
				break;
			}
			len += fprintf(gfp, "%s", cp);
			break;
		}
	}
	for (j = cur_level - min1; j > 0; j--) {
		len += fprintf(gfp, ")");
	}
	return len;
}

/*
 * Display an equation space in multi-line fraction format.
 * Use color if available.
 *
 * Return true if successful.
 */
int
flist_sub(n)
int	n;	/* equation space number */
{
	int	sind;
	char	buf[30];
	int	len, len2, len3;
	int	pos;
	int	high, low;
	int	max_line, min_line;
	int	max2_line, min2_line;

	if (n_lhs[n] <= 0)
		return false;
	len = snprintf(buf, sizeof(buf), "#%d: ", n + 1);
	cur_line = 0;
	cur_pos = 0;
	sind = n_rhs[n];
	len += flist(lhs[n], n_lhs[n], false, 0, &max_line, &min_line);
	if (n_rhs[n]) {
		len += strlen(EQUATE_STRING);
make_smaller:
		len2 = flist(rhs[n], sind, false, 0, &high, &low);
		if (screen_columns && gfp == stdout && (len + len2) >= screen_columns && sind > 0) {
			for (sind--; sind > 0; sind--) {
				if (rhs[n][sind].level == 1 && rhs[n][sind].kind == OPERATOR) {
					switch (rhs[n][sind].token.operatr) {
					case PLUS:
					case MINUS:
					case MODULUS:
						goto make_smaller;
					}
				}
			}
			goto make_smaller;
		}
		if (high > max_line)
			max_line = high;
		if (low < min_line)
			min_line = low;
		len3 = flist(&rhs[n][sind], n_rhs[n] - sind, false, 0, &max2_line, &min2_line);
	} else {
		len2 = 0;
		len3 = 0;
	}
	if (screen_columns && gfp == stdout && max(len + len2, len3) >= screen_columns) {
		return list1_sub(n, false);
	}
	fprintf(gfp, "\n");
	for (cur_line = max_line; cur_line >= min_line; cur_line--) {
		pos = cur_pos = 0;
		if (cur_line == 0) {
			cur_pos += fprintf(gfp, "%s", buf);
		}
		pos += strlen(buf);
		pos += flist(lhs[n], n_lhs[n], true, pos, &high, &low);
		if (n_rhs[n]) {
			if (cur_line == 0) {
				cur_pos += fprintf(gfp, "%s", EQUATE_STRING);
			}
			pos += strlen(EQUATE_STRING);
			pos += flist(rhs[n], sind, true, pos, &high, &low);
		}
		fprintf(gfp, "\n");
	}
	if (sind < n_rhs[n]) {
		fprintf(gfp, "\n");
		for (cur_line = max2_line; cur_line >= min2_line; cur_line--) {
			cur_pos = 0;
			flist(&rhs[n][sind], n_rhs[n] - sind, true, 0, &high, &low);
			fprintf(gfp, "\n");
		}
	}
	fprintf(gfp, "\n");
	return true;
}

/*
 * Display a line of an expression if "out_flag" is true.
 * Use color if available.
 *
 * Return the width of the expression (that is, the required number of screen columns).
 */
static int
flist(equation, n, out_flag, pos, highp, lowp)
token_type	*equation;	/* expression pointer */
int		n;		/* length of expression */
int		out_flag;
int		pos;
int		*highp, *lowp;
{
	int	i;

	i = flist_recurse(equation, n, out_flag, 0, pos, 1, highp, lowp);
	if (out_flag) {
		default_color();
	}
	return i;
}

static int
flist_recurse(equation, n, out_flag, line, pos, cur_level, highp, lowp)
token_type	*equation;
int		n;
int		out_flag;
int		line;
int		pos;
int		cur_level;
int		*highp, *lowp;
{
	int	i, j, k, i1;
	int	l1, l2;
	int	ii;
	int	stop_at;
	int	div_loc;
	int	len_div;
	int	level;
	int	start_level;
	int	oflag;
	int	len = 0, len1, len2;
	int	high, low;
	char	buf[500];
	char	*cp;

	start_level = cur_level;
	*highp = line;
	*lowp = line;
	if (n <= 0) {
		return 0;
	}
	oflag = (out_flag && line == cur_line);
	if (oflag) {
		set_color(cur_level-1);
		for (; cur_pos < pos; cur_pos++) {
			fprintf(gfp, " ");
		}
	}
	ii = 0;
check_again:
	stop_at = n;
	div_loc = -1;
	for (i = ii; i < n; i++) {
		if (equation[i].kind == OPERATOR && equation[i].token.operatr == DIVIDE) {
			level = equation[i].level;
			for (j = i - 2; j > 0; j -= 2) {
				if (equation[j].level < level)
					break;
			}
			j++;
			if (div_loc < 0) {
				div_loc = i;
				stop_at = j;
			} else {
				if (j < stop_at) {
					div_loc = i;
					stop_at = j;
				} else if (j == stop_at) {
					if (level < equation[div_loc].level)
						div_loc = i;
				}
			}
		}
	}
	for (i = ii; i < n; i++) {
		if (i == stop_at) {
			j = cur_level - equation[div_loc].level;
			k = abs(j) - 1;
		} else {
			j = cur_level - equation[i].level;
			k = abs(j);
		}
		for (i1 = 1; i1 <= k; i1++) {
			if (j > 0) {
				cur_level--;
				len++;
				if (oflag) {
					fprintf(gfp, ")");
					set_color(cur_level-1);
				}
			} else {
				cur_level++;
				len++;
				if (oflag) {
					set_color(cur_level-1);
					fprintf(gfp, "(");
				}
			}
		}
		if (i == stop_at) {
			level = equation[div_loc].level;
			len1 = flist_recurse(&equation[stop_at], div_loc - stop_at, false, line + 1, pos + len, level, &high, &low);
			l1 = (2 * (line + 1)) - low;
			for (j = div_loc + 2; j < n; j += 2) {
				if (equation[j].level <= level)
					break;
			}
			len2 = flist_recurse(&equation[div_loc+1], j - (div_loc + 1), false, line - 1, pos + len, level, &high, &low);
			l2 = (2 * (line - 1)) - high;
			ii = j;
			len_div = max(len1, len2);
			j = 0;
			if (len1 < len_div) {
				j = (len_div - len1) / 2;
			}
			flist_recurse(&equation[stop_at], div_loc - stop_at, out_flag, l1, pos + len + j, level, &high, &low);
			if (high > *highp)
				*highp = high;
			if (low < *lowp)
				*lowp = low;
			if (oflag) {
				set_color(level-1);
				for (j = 0; j < len_div; j++) {
					if (html_flag) {
						fprintf(gfp, "&mdash;");
					} else {
						fprintf(gfp, "-");
					}
				}
				set_color(cur_level-1);
			}
			j = 0;
			if (len2 < len_div) {
				j = (len_div - len2) / 2;
			}
			flist_recurse(&equation[div_loc+1], ii - (div_loc + 1), out_flag, l2, pos + len + j, level, &high, &low);
			if (high > *highp)
				*highp = high;
			if (low < *lowp)
				*lowp = low;
			len += len_div;
			goto check_again;
		}
		switch (equation[i].kind) {
		case CONSTANT:
			if (equation[i].token.constant == 0.0) {
				equation[i].token.constant = 0.0; /* fix -0 */
			}
			if (finance_option) {
				len += snprintf(buf, sizeof(buf), "%.2f", equation[i].token.constant);
			} else {
				len += snprintf(buf, sizeof(buf), "%.*g", precision, equation[i].token.constant);
			}
			if (oflag)
				fprintf(gfp, "%s", buf);
			break;
		case VARIABLE:
			len += list_var(equation[i].token.variable, 0);
			if (oflag)
				fprintf(gfp, "%s", var_str);
			break;
		case OPERATOR:
			cp = _("(unknown operator)");
			switch (equation[i].token.operatr) {
			case PLUS:
				cp = " + ";
				break;
			case MINUS:
				cp = " - ";
				break;
			case TIMES:
				cp = "*";
				break;
			case DIVIDE:
				cp = "/";
				break;
			case MODULUS:
				cp = " % ";
				break;
			case POWER:
				cp = "^";
				break;
			case FACTORIAL:
				cp = "!";
				i++;
				break;
			}
			if (oflag)
				fprintf(gfp, "%s", cp);
			len += strlen(cp);
			break;
		}
	}
	for (j = cur_level - start_level; j > 0;) {
		cur_level--;
		len++;
		j--;
		if (oflag) {
			fprintf(gfp, ")");
			if (j > 0)
				set_color(cur_level-1);
		}
	}
	if (oflag)
		cur_pos += len;
	return len;
}
