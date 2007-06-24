/*
 * This file contains main() and the startup code for Mathomatic.
 * Leave this file out when compiling for the symbolic math library.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 *
 * Output to stderr is only done in this file.  The rest of the code
 * should not output to stderr.
 *
 * This program only supports binary and unary operators.
 * Unary operators are implemented as a binary operation with a dummy operand.
 *
 * In the storage format, each level of parentheses is indicated
 * by a level number (origin 1).  The deeper the level, the
 * higher the level number.
 *
 * The storage format for expressions is a fixed size array of elements
 * "token_type", which may be a CONSTANT, VARIABLE, or OPERATOR.
 * The array always alternates between operand (CONSTANT or VARIABLE)
 * and OPERATOR.  There is a separate integer for each array which
 * contains the current length of the expression stored in the array.
 * This length is always odd and must never exceed "n_tokens".
 *
 * Only one POWER operator is allowed per level in the storage format,
 * and no other operators may be on that level.  Same with the FACTORIAL
 * and MODULUS operators.
 *
 * Any number of TIMES and DIVIDE operators may be on the same
 * level, because they are simple multiplicative class operators.
 * The same for PLUS and MINUS, because they are additive class operators.
 *
 * See the file "am.h" to start understanding the Mathomatic code and
 * to adjust memory usage.
 */

#if	!LIBRARY
#include "includes.h"
#include <sys/ioctl.h>
#include <termios.h>

static void usage();
static int set_signals();

int
main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind;

	int		i;
	char		*cp;
	double		numerator, denominator;
	double		multiplier;
	int		coption = false;

#if	UNIX
	prog_name = strdup(basename(argv[0]));		/* set prog_name to this executable's filename */
#endif
#if	CYGWIN
	dir_path = strdup(dirname_win(argv[0]));	/* set dir_path to this executable's directory */
#endif
	/* initialize the global variables */
	init_gvars();
	gfp = stdout;

	/* process command line options */
	while ((i = getopt(argc, argv, "qrtchuvm:")) >= 0) {
		switch (i) {
		case 'c':
			coption++;
			break;
		case 'h':
			readline_enabled = false;
			html_flag = true;
			break;
		case 'm':
			multiplier = strtod(optarg, &cp) * DEFAULT_N_TOKENS;
			if (*cp || multiplier < 100 || multiplier >= (INT_MAX / 3)) {
				fprintf(stderr, _("%s: Invalid memory size multiplier specified.\n"), prog_name);
				exit(1);
			}
			n_tokens = (int) multiplier;
			break;
		case 'q':
			quiet_mode = true;
			break;
		case 'r':
			readline_enabled = false;
			break;
		case 't':
			readline_enabled = false;
			test_mode = true;
			break;
		case 'u':
			setbuf(stdout, NULL);	/* make standard output unbuffered */
			break;
		case 'v':
			version_report();
			exit(0);
		default:
			usage();
		}
	}
	if (test_mode || html_flag) {
		screen_columns = 0;
		screen_rows = 0;
	} else {
		get_screen_size();
	}
	if (!init_mem()) {
		fprintf(stderr, _("%s: Not enough memory.\n"), prog_name);
		exit(2);
	}
	if (html_flag) {
		printf("<pre>\n");
	}
	if (!test_mode && !quiet_mode) {
#if	SECURE
		printf(_("Secure "));
#endif
		printf(_("Mathomatic version %s (www.mathomatic.org)\n"), VERSION);
		printf(_("Copyright (C) 1987-2007 George Gesslein II.\n"));
		printf(_("This is free software; see the source for copying conditions.  There is NO\n"));
		printf(_("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"));
		printf(_("%d equation spaces available, %lld kilobytes per equation space.\n"),
		    N_EQUATIONS, (long long) n_tokens * sizeof(token_type) * 2LL / 1000LL);
	}
#if	!SECURE
	/* read the user options initialization file */
	if (!test_mode && !load_rc()) {
		fprintf(stderr, _("Error loading set options from \"%s\".\n"), RC_FILE);
	}
#endif
	if (test_mode) {
		color_flag = false;
	}
	if (coption & 1) {
		color_flag = !color_flag;
	}
	if (!test_mode && !quiet_mode && color_flag) {
		printf(_("%s color mode enabled.\n"), html_flag ? "HTML" : "ANSI");
	}
	if ((i = setjmp(jmp_save)) != 0) {
		/* for error handling */
		clean_up();
		switch (i) {
		case 13:
			printf(_("Operation abruptly aborted.\n"));
			break;
		case 14:
			error(_("Expression too large."));
		default:
			printf(_("Operation aborted.\n"));
		}
	} else {
		if (!set_signals()) {
			fprintf(stderr, _("signal(2) setting failed.\n"));
		}
		if (!f_to_fraction(0.5, &numerator, &denominator)
		    || !f_to_fraction(1.0/3.0, &numerator, &denominator)) {
			fprintf(stderr, _("Cannot convert floating point values to fractions.\n"));
		}
#if	!SECURE
		/* read in files on the command line, exit if error */
		for (i = optind; i < argc; i++) {
			if (!read_cmd(argv[i])) {
				exit_program(1);
			}
		}
#endif
	}
	/* main input/output loop */
	for (;;) {
		default_color();
		snprintf(prompt_str, sizeof(prompt_str), "%d%s", cur_equation + 1, html_flag ? HTML_PROMPT : PROMPT);
		if ((cp = get_string((char *) tlhs, n_tokens * sizeof(token_type))) == NULL)
			break;
		process(cp);
	}
	exit_program(0);
}

/*
 * Display invocation usage info and exit with error.
 */
static void
usage()
{
	fprintf(stderr, _("Mathomatic version %s - automatic algebraic manipulator\n\n"), VERSION);
	fprintf(stderr, _("Usage: %s [ options ] [ input_files ]\n\n"), prog_name);
	fprintf(stderr, _("Options:\n"));
	fprintf(stderr, _("  -c               Toggle color mode.\n"));
	fprintf(stderr, _("  -h               Enable HTML output mode and disable readline.\n"));
	fprintf(stderr, _("  -m number        Specify a memory size multiplier.\n"));
	fprintf(stderr, _("  -q               Quiet mode (don't display prompts).\n"));
	fprintf(stderr, _("  -r               Disable readline.\n"));
	fprintf(stderr, _("  -t               Used when testing.\n"));
	fprintf(stderr, _("  -u               Set unbuffered output.\n"));
	fprintf(stderr, _("  -v               Display version number, compile flags, and memory usage.\n"));
	exit(1);
}

/*
 * All signal(2) initialization goes here.
 *
 * Return true on success.
 */
static int
set_signals()
{
	int	rv = true;

	if (signal(SIGFPE, fphandler) == SIG_ERR)
		rv = false;
	if (signal(SIGINT, inthandler) == SIG_ERR)
		rv = false;
	if (signal(SIGWINCH, resizehandler) == SIG_ERR)
		rv = false;
#if	TIMEOUT_SECONDS
	if (signal(SIGALRM, alarmhandler) == SIG_ERR)
		rv = false;
	alarm(TIMEOUT_SECONDS);
#endif
	return rv;
}

#if	CYGWIN
/*
 * dirname(3) function for Windows.
 */
char	*
dirname_win(cp)
char	*cp;
{
	int	i;

	i = strlen(cp);
	while (i >= 0 && cp[i] != '\\' && cp[i] != '/')
		i--;
	if (i < 0)
		return(".");
	cp[i] = '\0';
	return(cp);
}
#endif

#if	!SECURE
/*
 * Load set options from "~/.mathomaticrc".
 *
 * Return false if error encountered.
 */
int
load_rc()
{
	FILE	*fp;
	char	buf[MAX_CMD_LEN];
	char	*cp;
	int	rv = true;

#if	CYGWIN
	snprintf(buf, sizeof(buf), "%s/%s", dir_path, RC_FILE);
	fp = fopen(buf, "r");
	if (fp == NULL) {
		return true;
	}
#else
	cp = getenv("HOME");
	if (cp == NULL)
		return true;
	snprintf(buf, sizeof(buf), "%s/%s", cp, RC_FILE);
	fp = fopen(buf, "r");
	if (fp == NULL)
		return true;
#endif
	while ((cp = fgets(buf, sizeof(buf), fp)) != NULL) {
		set_error_level(cp);
		if (!set_options(cp))
			rv = false;
	}
	fclose(fp);
	return rv;
}
#endif

/*
 * Floating point exception handler.
 * Usually doesn't work in most operating systems.
 */
void
fphandler(sig)
int	sig;
{
	error(_("Floating point exception."));
}

/*
 * Control-C (interrupt) handler.
 * Abruptly quit this program.
 */
void
inthandler(sig)
int	sig;
{
	printf(_("\nControl-C pressed, quitting...\n"));
	exit_program(1);
}

#if	TIMEOUT_SECONDS
/*
 * Alarm signal handler.
 */
void
alarmhandler(sig)
int	sig;
{
	printf(_("\nTimeout, quitting...\n"));
	exit_program(1);
}
#endif

/*
 * Get the screen (window) width and height.
 */
get_screen_size()
{
	struct winsize	ws;

	ws.ws_col = 0;
	ws.ws_row = 0;
	if (ioctl(1, TIOCGWINSZ, &ws) >= 0) {
		if (ws.ws_col > 0 && screen_columns) {
			screen_columns = ws.ws_col;
		}
		if (ws.ws_row > 0 && screen_rows) {
			screen_rows = ws.ws_row;
		}
	}
}

/*
 * Window resize handler.
 */
void
resizehandler(sig)
int	sig;
{
	get_screen_size();
}

/*
 * Exit this program and return to the Operating System.
 */
void
exit_program(exit_value)
int	exit_value;	/* zero if OK, non-zero indicates error return */
{
	reset_attr();
	printf("\n");
	if (html_flag) {
		printf("</pre>\n");
	}
	exit(exit_value);
}
#endif
