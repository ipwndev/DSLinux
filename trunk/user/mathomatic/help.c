/*
 * Mathomatic help command and parsing routines.
 *
 * Everything that depends on the command table goes here.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

#include "includes.h"

/*
 * The following structure is used for each Mathomatic command.
 */
typedef	struct {
	char	*name;			/* command name to be typed by user */
	char	*secondary_name;	/* another name for this command */
	int	(*func)();		/* function that handles this command */
					/* function is passed a char pointer and returns true if successful */
	char	*usage;			/* command syntax text */
	char	*info;			/* one line description of command */
} com_type;

/*
 * The Mathomatic command table follows.  It should be in alphabetical order.
 */
static com_type com_list[] = {
/*	command name,	alternate name,		function,	usage,							information */
#if	!LIBRARY
{	"calculate",	NULL,			calculate_cmd,	"[variable number-of-iterations]",			"Temporarily plug in values for variables and approximate." },
#endif
{	"clear",	NULL,			clear_cmd,	"[equation-numbers-or-ranges]",				"Delete expressions or equations so equation spaces can be reused." },
#if	!LIBRARY
{	"code",		NULL,			code_cmd,	"[\"c\" or \"java\" or \"python\" or \"integer\"] [equation-number-range]",	"Output C, Java, or Python code for the specified equations." },
#endif
{	"compare",	NULL,			compare_cmd,	"equation-number [\"with\" equation-number]",		"Compare two equation spaces to see if they are the same." },
{	"copy",		NULL,			copy_cmd,	"[equation-number-range]",				"Duplicate the specified equation spaces." },
{	"derivative",	"differentiate",	derivative_cmd,	"[variable or \"all\"] [order]",			"Symbolically differentiate and simplify, order times." },
{	"display",	"flist",		display_cmd,	"[\"factor\"] [equation-number-range]",			"Display stored expressions in multi-line fraction format." },
#if	!LIBRARY
{	"divide",	NULL,			divide_cmd,	"[variable]",						"Prompt for 2 polynomials/numbers and divide.  Display result and GCD." },
#endif
{	"echo",		NULL,			echo_cmd,	"[text]",						"Output a line of text, followed by a newline." },
#if	!LIBRARY && (UNIX || CYGWIN) && !SECURE
{	"edit",		NULL,			edit_cmd,	"[file-name]",						"Edit all equation spaces or an input file." },
#endif
{	"eliminate",	NULL,			eliminate_cmd,	"variables or \"all\" [\"using\" equation-number]",	"Substitute the specified variables with solved equations." },
{	"extrema",	"stationary",		extrema_cmd,	"[variable] [order]",					"Find possible local minimums and maximums of the current expression." },
{	"factor",	NULL,			factor_cmd,	"[\"number\" [integers]] or [equation-number-range] [variables]",	"Trivially factor integers or stored expressions." },
{	"fraction",	NULL,			fraction_cmd,	"[equation-number-range]",				"Convert expressions with any algebraic fractions into a single fraction." },
{	"help",		NULL,			help_cmd,	"[topic or command-name]",				"Short, built-in help." },
{	"imaginary",	NULL,			imaginary_cmd,	"[variable]",						"Copy the imaginary part of an expression (see the \"real\" command)." },
{	"integrate",	"integral",		integrate_cmd,	"[\"definite\"] variable [order]",			"Symbolically integrate polynomials, order times." },
{	"laplace",	NULL,			laplace_cmd,	"[\"inverse\"] variable",				"Compute the Laplace or inverse Laplace transform of polynomials." },
{	"limit",	NULL,			limit_cmd,	"variable expression",					"Take the limit of the current expression as variable goes to expression." },
{	"list",		NULL,			list_cmd,	"[\"export\" or \"maxima\"] [equation-number-range]",	"Display stored expressions in single line format." },
#if	!LIBRARY
{	"nintegrate",	NULL,			nintegrate_cmd,	"[\"trapezoid\"] variable [partitions]",		"Approximate the definite integral using Simpson's rule."},
{	"optimize",	NULL,			optimize_cmd,	"[equation-number-range]",				"Split up an equation into smaller multiple equations." },
#endif
#if	!LIBRARY
{	"pause",	NULL,			pause_cmd,	"[text]",						"Wait for user to press the Enter key.  Optionally display a message." },
#endif
{	"product",	NULL,			product_cmd,	"variable start end [step]",				"Compute the product as variable goes from start to end." },
#if	READLINE
{	"push",		NULL,			push_cmd,	"[equation-number-range]",				"Push stored expressions into the readline history." },
#endif
#if	!LIBRARY
{	"quit",		"exit",			quit_cmd,	"",							"Terminate this program without saving." },
#endif
#if	!SECURE
{	"read",		NULL,			read_cmd,	"file-name",						"Read in a text file as if it was typed in." },
#endif
{	"real",		NULL,			real_cmd,	"[variable]",						"Copy the real part of an expression (see the \"imaginary\" command)." },
{	"replace",	NULL,			replace_cmd,	"[\"constants\"] or [variables [\"with\" expression]]",	"Substitute variables in the current equation with expressions." },
#if	!LIBRARY
{	"roots",	NULL,			roots_cmd,	"root real-part imaginary-part",			"Display all the roots of a complex number." },
#endif
#if	!SECURE
{	"save",		NULL,			save_cmd,	"file-name",						"Save all stored expressions and equations in a text file." },
#endif
{	"set",		NULL,			set_cmd,	"[[\"no\"] option]",					"Set or display various session options." },
{	"simplify",	NULL,			simplify_cmd,	"[\"symbolic\"] [\"quick\"] [equation-number-range]",	"Completely simplify stored expressions." },
{	"solve",	NULL,			solve_cmd,	"variable or \"0\"",					"Solve the current equation for a variable or for 0." },
{	"sum",		NULL,			sum_cmd,	"variable start end [step]",				"Compute the summation as variable goes from start to end." },
#if	!LIBRARY
{	"tally",	NULL,			tally_cmd,	"",							"Prompt for and add numerical entries, show running total and average." },
#endif
{	"taylor",	NULL,			taylor_cmd,	"[variable] [order] [point]",				"Compute the Taylor series expansion of the current expression." },
{	"unfactor",	"expand",		unfactor_cmd,	"[\"fully\"] [equation-number-range]",			"Algebraically expand (multiply out) stored expressions." },
{	"version",	NULL,			version_cmd,	"",							"Display version number, compile flags used, and maximum memory usage." }
};

/*
 * Process equation and expression input in Mathomatic, with no solving
 * and no automatic calculation.
 *
 * Parse the equation or expression text in "cp" and place in equation space "n".
 *
 * Return true if successful.
 */
int
parse(n, cp)
int	n;
char	*cp;
{
	if ((cp = parse_equation(n, cp)) != NULL) {
		if (n_lhs[n] == 0 && n_rhs[n] == 0)
			return true;
		if (n_lhs[n] == 0) {
			/* RHS expression only, set equal to zero */
			n_lhs[n] = 1;
			lhs[n][0] = zero_token;
		}
		cur_equation = n;
		return return_result(cur_equation);
	}
	n_lhs[n] = 0;
	n_rhs[n] = 0;
	return false;
}

/*
 * Process equation and expression input in Mathomatic.
 *
 * Parse the expression text in "cp" and solve the current equation for it
 * or place it in equation space "n" if it is not a solve variable.
 *
 * Return true if successful.
 */
int
process_parse(n, cp)
int	n;
char	*cp;
{
#if	!LIBRARY
	int	i;
#endif

	if ((cp = parse_equation(n, cp)) != NULL) {
		if (n_lhs[n] == 0 && n_rhs[n] == 0)
			return true;
		if (n_lhs[n] == 0 || n_rhs[n] == 0) {
			if (autosolve) {
				if ((n_lhs[n] == 1 && ((lhs[n][0].kind == CONSTANT && lhs[n][0].token.constant == 0.0)
				    || (lhs[n][0].kind == VARIABLE && (lhs[n][0].token.variable & VAR_MASK) > SIGN)))
				    || (n_rhs[n] == 1 && ((rhs[n][0].kind == CONSTANT && rhs[n][0].token.constant == 0.0)
				    || (rhs[n][0].kind == VARIABLE && (rhs[n][0].token.variable & VAR_MASK) > SIGN)))) {
					if (solve(n, cur_equation)) {
						return return_result(cur_equation);
					}
					return false;
				}
			}
			if (n_rhs[n]) {
				/* RHS expression only, set equal to zero */
				n_lhs[n] = 1;
				lhs[n][0] = zero_token;
				goto return_ok;
			}
			if (n_lhs[n] == 1 && lhs[n][0].kind == CONSTANT && fmod(lhs[n][0].token.constant, 1.0) == 0.0
			    && lhs[n][0].token.constant > 0.0 && lhs[n][0].token.constant <= n_equations) {
				/* easy selecting of equation spaces */
				n_lhs[n] = 0;
				n = lhs[n][0].token.constant - 1;
				goto return_ok;
			}
#if	!LIBRARY
			if (autocalc) {
				/* the numerical input calculation */
				for (i = 0; i < n_lhs[n]; i += 2) {
					if (lhs[n][i].kind == VARIABLE && (lhs[n][i].token.variable & VAR_MASK) > SIGN) {
						goto return_ok;		/* not numerical (contains a variable) */
					}
				}
				/* make the expression an equation */
				blt(rhs[n], lhs[n], n_lhs[n] * sizeof(token_type));
				n_rhs[n] = n_lhs[n];
				lhs[n][0].level = 1;
				lhs[n][0].kind = VARIABLE;
				parse_var(&lhs[n][0].token.variable, "answer");
				n_lhs[n] = 1;
				i = cur_equation;
				/* make the current equation and run the calculate command on it */
				cur_equation = n;
				calculate_cmd("");
				cur_equation = i;
				/* delete from memory */
				n_lhs[n] = 0;
				n_rhs[n] = 0;
				return true;
			}
#endif
		}
return_ok:
		cur_equation = n;
		return return_result(cur_equation);
	}
	n_lhs[n] = 0;
	n_rhs[n] = 0;
	return false;
}

/*
 * Process a line of input to Mathomatic.
 * It may be a command, an expression, an equation, etc.
 *
 * Return true if successful.
 */
int
process(cp)
char	*cp;
{
	char	*cp1;
	char	*cp_start;
	int	i;
	int	rv;
	char	buf2[MAX_CMD_LEN];
	int	i1;
	char	*filename;
	FILE	*fp;
	int	append_flag;

	if (cp == NULL) {
		return false;
	}
	set_sign_array();
	cp_start = cp;
	cp = skip_space(cp);
	if (*cp == '#') {	/* handle the equation number selector */
		cp++;
		switch (*cp) {
		case '+':
		case '-':
			i = decstrtol(cp, &cp1);
			i = cur_equation + i;
			break;
		default:
			i = decstrtol(cp, &cp1) - 1;
			break;
		}
		if (cp == cp1)
			return true;	/* treat as comment */
		cp = cp1;
		if (i < 0 || i >= n_equations) {
			error(_("Equation not defined."));
			return false;
		}
		if (*cp == ':') {
			cp++;
		}
		cp = skip_space(cp);
		if (*cp) {
			input_column += (cp - cp_start);
			return parse(i, cp);
		}
		cur_equation = i;
		return return_result(cur_equation);
	}
#if	!SECURE
	/* handle shell escape */
	if (*cp == '!') {
		cp = skip_space(cp + 1);
		if (*cp) {
			return(!shell_out(cp));
		}
		cp1 = getenv("SHELL");
		if (cp1) {
			return(!shell_out(cp1));
		}
		error(_("SHELL environment variable not set."));
		return false;
	}
#endif
	/* a quick way to get help */
	if (*cp == '?') {
		cp = skip_space(cp + 1);
		input_column += (cp - cp_start);
		return(help_cmd(cp));
	}
/* See if the string pointed to by "cp" is a command. */
/* If so, execute it. */
	cp1 = cp;
	while (*cp1 && !isspace(*cp1))
		cp1++;
	for (i = 0; i < ARR_CNT(com_list); i++) {
		if (((cp1 - cp) >= min(4, strlen(com_list[i].name)) && strncasecmp(cp, com_list[i].name, cp1 - cp) == 0)
		    || (com_list[i].secondary_name && (cp1 - cp) >= min(4, strlen(com_list[i].secondary_name)) && strncasecmp(cp, com_list[i].secondary_name, cp1 - cp) == 0)) {
			cp1 = skip_space(cp1);
			input_column += (cp1 - cp_start);
			if (my_strlcpy(buf2, cp1, sizeof(buf2)) >= sizeof(buf2)) {
				error(_("Command line too long."));
				return false;
			}
			fp = NULL;
#if	!SECURE
			/* handle output redirection */
			append_flag = false;
			filename = NULL;
			for (i1 = strlen(buf2) - 1; i1 >= 0; i1--) {
				if (buf2[i1] == '>') {
					filename = skip_space(&buf2[i1+1]);
					if (i1 && buf2[i1-1] == '>') {
						i1--;
						append_flag = true;
					}
					buf2[i1] = '\0';
					break;
				}
			}
			if (filename) {
				if (append_flag) {
					fp = fopen(filename, "a");
				} else {
					fp = fopen(filename, "w");
				}
				if (fp == NULL) {
#if	!SILENT
					printf(_("Can't open \"%s\" for writing.\n"), filename);
#endif
					return false;
				}
				gfp = fp;
			}
#endif
			/* remove trailing spaces from the command line */
		        i1 = strlen(buf2) - 1;
		        while (i1 >= 0 && isspace(buf2[i1])) {
        		        buf2[i1] = '\0';
                		i1--;
			}
			/* execute the command */
			rv = (*com_list[i].func)(buf2);
#if	!SECURE
			if (fp) {	/* if output redirected, close file */
				if (gfp != stdout)
					fclose(gfp);
				else
					fclose(fp);
				gfp = stdout;
			}
#endif
			return rv;
		}
	}
/* "cp" is not a command, so parse the expression or equation. */
	i = next_espace();
	input_column += (cp - cp_start);
	return process_parse(i, cp);
}

#if	!SECURE
/*
 * Execute a system command.
 *
 * Returns exit value of command (0 if no error).
 */
int
shell_out(cp)
char	*cp;
{
	int	rv;

	reset_attr();
	rv = system(cp);
	default_color();
	return rv;
}
#endif

/*
 * Parse a variable name with before and after space character skipping.
 *
 * Return new position in string or NULL if error.
 */
char	*
parse_var2(vp, cp)
long	*vp;	/* pointer to returned variable in Mathomatic internal format */
char	*cp;	/* pointer to variable name string */
{
	cp = skip_space(cp);
	cp = parse_var(vp, cp);
	if (cp == NULL) {
		return NULL;
	}
	return skip_space(cp);
}

#define P(A)	fprintf(gfp, "%s\n", A)

/*
 * Output command info and usage.
 *
 * Return the number of lines output.
 */
int
display_command(i)
int	i;	/* command index */
{
	int	rows = 3;

	fprintf(gfp, "%s - %s\n", com_list[i].name, com_list[i].info);
	fprintf(gfp, "Usage: %s %s\n", com_list[i].name, com_list[i].usage);
	if (com_list[i].secondary_name) {
		fprintf(gfp, "Alternate name for this command: %s\n", com_list[i].secondary_name);
		rows++;
	}
	fprintf(gfp, "\n");
	return rows;
}

/*
 * The help command.
 */
int
help_cmd(cp)
char	*cp;
{
	int	i, j;
	char	*cp1;
	int	flag;
	int	row;

	cp1 = cp;
	while (*cp1 && !isspace(*cp1))
		cp1++;
	if (cp1 != cp) {
		/* first, see if the argument matches any command names */
		flag = false;
		for (i = 0; i < ARR_CNT(com_list); i++) {
			if (strncasecmp(cp, com_list[i].name, cp1 - cp) == 0
			    || (com_list[i].secondary_name && strncasecmp(cp, com_list[i].secondary_name, cp1 - cp) == 0)) {
				display_command(i);
				flag = true;
			}
		}
		if (flag)
			return true;
		if (strncasecmp(cp, "usage", cp1 - cp) == 0) {
			P("Command Usage Syntax");
			P("--------------------");
			for (i = 0, row = 3; i < ARR_CNT(com_list); i++) {
				fprintf(gfp, "%s %s\n", com_list[i].name, com_list[i].usage);
				row++;
				if (gfp == stdout && screen_rows && row >= (screen_rows - 1)) {
					row = 1;
					if (!pause_cmd(""))
						return false;
				}
			}
			return true;
		}
		if (strncasecmp(cp, "geometry", cp1 - cp) == 0) {
			P("Commonly Used Geometric Formulas");
			P("--------------------------------");
			P("Triangle of base \"b\" and height \"h\":");
			P("    Area = b*h/2");
			P("Rectangle of length \"l\" and width \"w\":");
			P("    Area = l*w                    Perimeter = 2*l + 2*w");
			P("Trapezoid of parallel sides \"a\" and \"b\", and \"d\" distance between them:");
			P("    Area = d*(a + b)/2");
			P("Circle of radius \"r\":");
			P("    Area = pi*r^2                 Perimeter = 2*pi*r");
			P("    Area = Perimeter*r/2");
			P("Rectangular solid of length \"l\", width \"w\", and height \"h\":");
			P("    Volume = l*w*h                Surface area = 2*l*w + 2*l*h + 2*w*h");
			P("Sphere of radius \"r\":");
			P("    Volume = 4/3*pi*r^3           Surface area = 4*pi*r^2");
			P("Right circular cylinder of radius \"r\" and height \"h\":");
			P("    Volume = pi*r^2*h             Surface area = 2*pi*r*(h + r)");
			P("Right circular cone of radius \"r\" and height \"h\":");
			P("    Volume = pi*r^2*h/3");
			P("    Base surface area = pi*r^2    Side surface area = pi*r*(r^2 + h^2)^.5\n");
			P("Convex polygon of \"n\" sides, sum of all interior angles formula:");
			P("    Sum = (n - 2)*180 degrees     Sum = (n - 2)*pi radians");
			return true;
		}
		if (strncasecmp(cp, "expressions", cp1 - cp) == 0 || strncasecmp(cp, "equations", cp1 - cp) == 0) {
			P("To enter an expression or equation, simply type it in at the prompt.");
			P("Operators have precedence decreasing as indicated:\n");
			P("    - negate");
			P("    ! factorial (gamma function)");
			P("    ** or ^ power (exponentiation)");
			P("    * multiply        / divide          % modulus");
			P("    + add             - subtract\n");
			P("Operators in the same precedence level are evaluated left to right.\n");
			P("Variables consist of any combination of letters, digits, and underscores (_).");
			P("Constants always start with a digit (0..9) or a period (.).");
			P("Predefined constants and variables follow:");
			P("    e or e# - the universal constant e (2.7182818284...).");
			P("    pi or pi# - the universal constant pi (3.1415926535...).");
			P("    i or i# - imaginary number (square root of -1).");
			P("The above constants may also be used anywhere variables are required.");
			P("    sign, sign1, sign2, sign3, ... - variable that can only be +1 or -1.");
			P("    integer - variable that may be any integer.");
			P("    inf - infinity constant (a true constant).\n");
			P("Absolute value notation \"|x|\" and \"+/-\" are understood.");
			return true;
		}
		if (is_all(cp)) {
			for (i = 0, row = 1; i < ARR_CNT(com_list); i++) {
				row += display_command(i);
				if (gfp == stdout && screen_rows && row >= (screen_rows - 5)) {
					row = 1;
					if (!pause_cmd(""))
						return false;
					printf("\n");
				}
			}
			P("End of command list.");
			return true;
		}
	}
	/* default help text: */
	P("This help command is provided as a quick reference.");
	P("Type \"help all\" for a summary and syntax of the all commands.");
	P("Type \"help usage\" for syntax of all commands.");
	P("Type \"help expressions\" for help with entering expressions.");
	P("Type \"help geometry\" for some commonly used geometric formulas.");
	P("\"help\" or \"?\" followed by a command name will give info on a command.\n");
	P("Available commands:");
	for (i = 0; i < ARR_CNT(com_list); i++) {
		if ((i % 5) == 0)
			fprintf(gfp, "\n");
		j = 15 - fprintf(gfp, "%s", com_list[i].name);
		for (; j > 0; j--)
			fprintf(gfp, " ");
	}
	P("\n\nTo select an equation in memory, type the equation number at the main prompt.");
	P("To solve the current equation, type the variable name at the main prompt.");
	return true;
}
