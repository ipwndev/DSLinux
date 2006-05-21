/*
 * Main include file for Mathomatic.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

#define	true	1
#define	false	0

#if	LIBRARY		/* define this when using Mathomatic as a symbolic math library */
#define	SILENT	1
#endif

#ifndef	max
#define max(a, b)    (((a) > (b)) ? (a) : (b))	/* return the maximum of two values */
#endif

#ifndef	min
#define min(a, b)    (((a) < (b)) ? (a) : (b))	/* return the minimum of two values */
#endif

#ifndef	isfinite
#define	isfinite(d)	finite(d)
#endif

#undef	PI
#define	PI	3.141592653589793238
#define	E	2.718281828459045235

#define	ARR_CNT(a)	(sizeof(a)/sizeof(a[0]))	/* returns the number of elements in an array */

#define	MAX_K_INTEGER	1.0e14				/* 14 digits for doubles */

#define	blt(dst, src, cnt)	memmove((char *) (dst), (char *) (src), (size_t) (cnt))	/* memory copy function */
#define always_positive(power)	(fmod((double) (power), 2.0) == 0.0)	/* true if all real numbers raised to "power" result in positive, real numbers */

#if	I18N	/* internationalization */
#define _(str)		gettext(str)
#else
#define _(str)		str
#endif

#define	STANDARD_SCREEN_COLUMNS	80			/* default number of columns of characters on the screen */
#define	STANDARD_SCREEN_ROWS	24			/* default number of lines on the screen */

#define	TMP_FILE	"/tmp/mathomatic.XXXXXX"	/* temp file template */

#define	PROMPT		"-> "				/* user interface prompt string */
#define	HTML_PROMPT	"-&gt; "

#define	MAX_CMD_LEN	PATH_MAX			/* maximum command line length (not equations), also max file name length */
#define	MAX_PROMPT_LEN	STANDARD_SCREEN_COLUMNS		/* maximum length of prompts */

#if	CYGWIN
#define RC_FILE		"mathomatic.rc"	/* file of set options in the Mathomatic directory under Windows */
#else
#define RC_FILE		".mathomaticrc"	/* file of set options read at startup from $HOME */
#endif

/*
 * The following defines the maximum number of equation spaces that can be allocated.
 */
#define	N_EQUATIONS	50

/*
 * The following defines the default maximum mathematical expression size.
 * This should be made much smaller for Palmtops or embedded systems.
 */
#define	DEFAULT_N_TOKENS	30000

#define	DIVISOR_SIZE	(DEFAULT_N_TOKENS / 2)	/* A nice maximum divisor size. */

#define	MAX_VAR_NAMES	8000			/* Maximum number of long variable names. Keep this under (VAR_MASK - VAR_OFFSET). */
#define	MAX_VAR_LEN	1000			/* Maximum size of long variable names */

#define	MAX_VARS	500			/* Maximum number of unique variables handled in each equation */

#define	VAR_OFFSET	'A'			/* makes space for predefined variables */
#define	VAR_MASK	0x3fffL			/* mask for bits containing the variable name */
#define	VAR_SHIFT	14			/* number of bits set in VAR_MASK */
#define	SUBSCRIPT_MASK	63			/* mask for variable subscript after shifting VAR_SHIFT */
#define	MAX_SUBSCRIPT	(SUBSCRIPT_MASK - 1)	/* maximum variable subscript */

typedef	char	sign_array_type[MAX_SUBSCRIPT+2];

enum kind_list {		/* kinds of tokens specified in the union below */
	CONSTANT,
	VARIABLE,
	OPERATOR
};

typedef union {
	double	constant;	/* internal storage for mathematical constants */
	long	variable;	/* internal storage for mathematical variables */
/* predefined variables follow (order is important) */
#define	V_NULL		0L	/* null variable (display nothing) */
#define	V_E		1L	/* the constant "e" */
#define	V_PI		2L	/* the constant "pi" */
#define	IMAGINARY	3L	/* the imaginary constant "i#" */
#define	SIGN		4L	/* the "sign" variable */
#define	MATCH_ANY	5L	/* match any variable (wild-card variable) */
#define	V_ANSWER	6L	/* the "answer" variable */
#define V_TEMP		7L	/* the "temp" variable */
#define	V_INTEGER	8L	/* the "integer" variable */
	int	operatr;	/* internal storage for operators */
/* valid operators follow */
#define	PLUS		1	/* a + b */
#define	MINUS		2	/* a - b */
#define	TIMES		3	/* a * b */
#define	DIVIDE		4	/* a / b */
#define	MODULUS		5	/* a % b */
#define	POWER		6	/* a ^ b */
#define	FACTORIAL	7	/* a! */
#define	NEGATE		8	/* special parser operator */
} storage_type;

typedef	struct {		/* storage structure for each token in an expression */
	enum kind_list	kind;	/* kind of token */
	int		level;	/* level of parentheses, origin 1 */
	storage_type	token;	/* the actual token */
} token_type;

typedef struct {		/* variable sort data structure */
	long	v;		/* variable */
	int	count;		/* number of times the variable occurs */
} sort_type;

#if	SILENT
#define	error(str)			{ error_str = str; }	/* must be a constant string, temp strings don't work */
#define	list_tdebug(level)
#define	side_debug(level, p1, n1)
#define debug_string(level, str)
#else
#define	error(str)			{ printf("%s\n", str); }
#define list_tdebug(level)		list_debug(level, tlhs, n_tlhs, trhs, n_trhs)
#define	side_debug(level, p1, n1)	list_debug(level, p1, n1, NULL, 0)
#define debug_string(level, str)	{ if (debug_level >= (level)) printf("%s\n", str); }
#endif
