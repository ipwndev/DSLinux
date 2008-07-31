/*
 * Main program entry point - read the command line options, then perform
 * the appropriate actions.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "options.h"
#include "pv.h"

/* #undef MAKE_STDOUT_NONBLOCKING */

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>


int remote_set(opts_t opts);
void remote_sig_init(opts_t opts);


/*
 * Process command-line arguments and set option flags, then call functions
 * to initialise, and finally enter the main loop.
 */
int main(int argc, char **argv)
{
	struct termios t;
	opts_t opts;
	int retcode = 0;

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	opts = opts_parse(argc, argv);
	if (!opts)
		return 1;
	if (opts->do_nothing) {
		opts_free(opts);
		return 0;
	}

	if (opts->remote > 0) {
		if (opts->width < 0)
			opts->width = 80;
		if (opts->height < 0)
			opts->height = 25;
		if (opts->width > 999999)
			opts->width = 999999;
		if (opts->height > 999999)
			opts->height = 999999;
		if ((opts->interval != 0) && (opts->interval < 0.1))
			opts->interval = 0.1;
		if (opts->interval > 600)
			opts->interval = 600;
		retcode = remote_set(opts);
		opts_free(opts);
		return retcode;
	}

	/*
	 * If no files were given, pretend "-" was given (stdin).
	 */
	if (opts->argc == 0) {
		opts->argv[opts->argc++] = "-";
	}

	if (opts->size == 0) {
		pv_calc_total_size(opts);
	}

	if (opts->size < 1)
		opts->eta = 0;

	if ((isatty(STDERR_FILENO) == 0)
	    && (opts->force == 0)
	    && (opts->numeric == 0)) {
		opts->no_op = 1;
	}

	if (opts->width == 0) {
		int tmpheight;
		tmpheight = opts->height;
		pv_screensize(opts);
		if (tmpheight > 0)
			opts->height = tmpheight;
	}

	if (opts->height == 0) {
		int tmpwidth;
		tmpwidth = opts->width;
		pv_screensize(opts);
		if (tmpwidth > 0)
			opts->width = tmpwidth;
	}

	/*
	 * Width and height bounds checking (and defaults).
	 */
	if (opts->width < 1)
		opts->width = 80;

	if (opts->height < 1)
		opts->height = 25;

	if (opts->width > 999999)
		opts->width = 999999;

	if (opts->height > 999999)
		opts->height = 999999;

	/*
	 * Interval must be at least 0.1 second, and at most 10 minutes.
	 */
	if (opts->interval < 0.1)
		opts->interval = 0.1;
	if (opts->interval > 600)
		opts->interval = 600;

#ifdef MAKE_STDOUT_NONBLOCKING
	/*
	 * Try and make standard output use non-blocking I/O.
	 *
	 * Note that this can cause problems with (broken) applications
	 * such as dd.
	 */
	fcntl(STDOUT_FILENO, F_SETFL,
	      O_NONBLOCK | fcntl(STDOUT_FILENO, F_GETFL));
#endif				/* MAKE_STDOUT_NONBLOCKING */

	/*
	 * Set terminal option TOSTOP so we get signal SIGTTOU if we try to
	 * write to the terminal while backgrounded.
	 */
	tcgetattr(STDERR_FILENO, &t);
	t.c_lflag |= TOSTOP;
	tcsetattr(STDERR_FILENO, TCSANOW, &t);

	opts->current_file = "(stdin)";

	pv_sig_init();
	remote_sig_init(opts);

	retcode = pv_main_loop(opts);

	opts_free(opts);
	return retcode;
}

/* EOF */
