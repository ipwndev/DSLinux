/*
 * Algebraic manipulator globals.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 *
 * All global variables for Mathomatic are defined here and duplicated in "externs.h".
 */

#include "includes.h"

int		n_tokens = DEFAULT_N_TOKENS;	/* maximum size of expressions */
int		n_equations;			/* number of equation spaces allocated */
int		cur_equation;			/* current equation space number (origin 0) */

token_type	*lhs[N_EQUATIONS];		/* The Left Hand Sides of equation spaces */
token_type	*rhs[N_EQUATIONS];		/* The Right Hand Sides of equation spaces */

int		n_lhs[N_EQUATIONS];		/* number of tokens in each lhs[] */
int		n_rhs[N_EQUATIONS];		/* number of tokens in each rhs[] */

token_type	*tlhs;				/* LHS during solve and temporary storage for expressions */
token_type	*trhs;				/* RHS during solve and temporary storage for expressions */

int		n_tlhs;				/* number of tokens in tlhs */
int		n_trhs;				/* number of tokens in trhs */

token_type	*scratch;			/* very temporary storage for expressions */

token_type	zero_token;			/* the constant 0.0 as a token */
token_type	one_token;			/* the constant 1.0 as a token */

/* set options */
int		case_sensitive_flag = true;	/* "set case_sensitive" flag */
int		factor_int_flag;		/* factor integers when displaying expressions */
int		groupall = true;		/* "set display2d" flag */
int		preserve_roots = true;		/* true if we are to preserve 2^.5, etc. */
int		in_calc_cmd;			/* true if in calculate command (force approximations) */
int		true_modulus;			/* true for mathematically correct modulus */
int		screen_columns = STANDARD_SCREEN_COLUMNS;	/* screen width */
int		screen_rows = STANDARD_SCREEN_ROWS;		/* screen height */
int		finance_option;			/* for displaying dollars and cents */
int		starstar;			/* output "**" instead of "^" for the power operator */
#if	!SILENT
int		debug_level;			/* current debug level */
#endif

/* variables having to do with color mode */
int		color_flag = true;		/* true for color mode */
int		bold_colors;			/* true for bold colors */
int		cur_color = -1;			/* current color */

/* epsilon constants */
double		small_epsilon	= 0.000000000000005;	/* for small round-off errors */
double		epsilon		= 0.00000000000005;	/* for larger accumulated round-off errors */

/* string variables */
char		*var_names[MAX_VAR_NAMES];	/* storage for long variable names */
char		var_str[MAX_VAR_LEN+80];	/* temp storage for variable names */
char		prompt_str[MAX_PROMPT_LEN];	/* temp storage for prompt strings */
#if	CYGWIN
char		*dir_path;			/* directory path to the executable */
#endif
char		*prog_name = "mathomatic";	/* name of this program */

/* The following are for integer factoring (filled by factor_one()): */
double		unique[64];		/* storage for the unique factors */
int		ucnt[64];		/* number of times the factor occurs */
int		uno;			/* number of unique factors stored in unique[] */

/* misc. variables */
sign_array_type	sign_array;		/* for keeping track of unique "sign" variables */
FILE		*gfp;			/* global output file pointer */
jmp_buf		jmp_save;		/* for setjmp() when an error is encountered */
int		test_mode;		/* test mode flag (-t) */
int		quiet_mode;		/* quiet mode (don't display prompts) */
int		html_flag;		/* true for HTML mode */
int		partial_flag;		/* true for partial unfactoring of algebraic fractions */
int		symb_flag;		/* true for "simplify symbolic" */
int		high_prec;		/* flag to output constants in higher precision (used when saving equations) */
int		input_column;		/* current column number on the screen at the beginning of a parse */
int		sign_flag;		/* true when all "sign" variables are to compare equal */
int		domain_check;		/* flag to track domain errors in the pow() function */

/* library variables go here */
char		*error_str;		/* last error when SILENT is defined */
char		*result_str;		/* returned result when using as library */
