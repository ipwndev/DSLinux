/*
 * Main program entry point - read the command line options, then perform
 * the appropriate actions.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#include "options.h"
#include "pv.h"

#define _GNU_SOURCE 1
#include <limits.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define RATE_GRANULARITY	100000	    /* usec between -L rate chunks */

extern struct timeval pv_sig_toffset;
extern sig_atomic_t pv_sig_newsize;
extern sig_atomic_t pv_sig_abort;


/*
 * Add the given number of microseconds (which may be negative) to the given
 * timeval.
 */
static void pv_timeval_add_usec(struct timeval *val, long usec)
{
	val->tv_usec += usec;
	while (val->tv_usec < 0) {
		val->tv_sec--;
		val->tv_usec += 1000000;
	}
	while (val->tv_usec >= 1000000) {
		val->tv_sec++;
		val->tv_usec -= 1000000;
	}
}


/*
 * Pipe data from a list of files to standard output, giving information
 * about the transfer on standard error according to the given options.
 *
 * Returns nonzero on error.
 */
int pv_main_loop(opts_t opts)
{
	long written, lineswritten;
	long long total_written, since_last, cansend, donealready, target;
	int eof_in, eof_out, final_update;
	struct timeval start_time, next_update, next_reset, cur_time;
	struct timeval init_time;
	long double elapsed;
	struct stat64 sb;
	int fd, n;

	/*
	 * "written" is ALWAYS bytes written by the last transfer.
	 *
	 * "lineswritten" is the lines written by the last transfer,
	 * but is only updated in line mode.
	 *
	 * "total_written" is the total bytes written since the start,
	 * or in line mode, the total lines written since the start.
	 *
	 * "since_last" is the bytes written since the last display,
	 * or in line mode, the lines written since the last display.
	 *
	 * The remaining variables are all unchanged by linemode.
	 */

	fd = -1;

	pv_crs_init(opts);

	eof_in = 0;
	eof_out = 0;
	total_written = 0;
	since_last = 0;

	gettimeofday(&start_time, NULL);
	gettimeofday(&cur_time, NULL);

	next_update.tv_sec = start_time.tv_sec;
	next_update.tv_usec = start_time.tv_usec;
	pv_timeval_add_usec(&next_update,
			    (long) (1000000.0 * opts->interval));

	next_reset.tv_sec = start_time.tv_sec;
	next_reset.tv_usec = start_time.tv_usec;
	pv_timeval_add_usec(&next_reset, RATE_GRANULARITY);

	cansend = 0;
	donealready = 0;
	final_update = 0;
	n = 0;

	fd = pv_next_file(opts, n, -1);
	if (fd < 0) {
		return 1;
	}

	if (fstat64(fd, &sb) == 0) {
		pv_set_buffer_size(sb.st_blksize * 32, 0);
	}

	if (opts->buffer_size > 0) {
		pv_set_buffer_size(opts->buffer_size, 1);
	}

	while ((!(eof_in && eof_out)) || (!final_update)) {

		if (pv_sig_abort)
			break;

		if (opts->rate_limit > 0) {
			target =
			    ((long double) (opts->rate_limit)) /
			    (long double) (1000000 / RATE_GRANULARITY);
			cansend = target - donealready;
			if (target < donealready)
				cansend = 0;
		}

		written =
		    pv_transfer(opts, fd, &eof_in, &eof_out, cansend,
				&lineswritten);
		if (written < 0)
			return 1;

		if (opts->linemode) {
			since_last += lineswritten;
			total_written += lineswritten;
		} else {
			since_last += written;
			total_written += written;
		}
		if (opts->rate_limit > 0)
			donealready += written;

		if (eof_in && eof_out && n < (opts->argc - 1)) {
			n++;
			fd = pv_next_file(opts, n, fd);
			if (fd < 0)
				return 1;
			eof_in = 0;
			eof_out = 0;
		}

		gettimeofday(&cur_time, NULL);

		if (eof_in && eof_out) {
			final_update = 1;
			next_update.tv_sec = cur_time.tv_sec - 1;
		}

		if ((cur_time.tv_sec > next_reset.tv_sec)
		    || (cur_time.tv_sec == next_reset.tv_sec
			&& cur_time.tv_usec >= next_reset.tv_usec)) {
			pv_timeval_add_usec(&next_reset, RATE_GRANULARITY);
			if (next_reset.tv_sec < cur_time.tv_sec)
				next_reset.tv_sec = cur_time.tv_sec;
			donealready = 0;
		}

		if (opts->no_op)
			continue;

		/*
		 * If -W was given, we don't output anything until we have
		 * written a byte (or line, in line mode), at which point
		 * we then count time as if we started when the first byte
		 * was received.
		 */
		if (opts->wait) {
			if (opts->linemode) {
				if (lineswritten < 1)
					continue;
			} else {
				if (written < 1)
					continue;
			}

			opts->wait = 0;

			/*
			 * Reset the timer offset counter now that data
			 * transfer has begun, otherwise if we had been
			 * stopped and started (with ^Z / SIGTSTOP)
			 * previously (while waiting for data), the timers
			 * will be wrongly offset.
			 *
			 * While we reset the offset counter we must disable
			 * SIGTSTOP so things don't mess up.
			 */
			pv_sig_nopause();
			gettimeofday(&start_time, NULL);
			pv_sig_toffset.tv_sec = 0;
			pv_sig_toffset.tv_usec = 0;
			pv_sig_allowpause();

			next_update.tv_sec = start_time.tv_sec;
			next_update.tv_usec = start_time.tv_usec;
			pv_timeval_add_usec(&next_update,
					    (long) (1000000.0 *
						    opts->interval));
		}

		if ((cur_time.tv_sec < next_update.tv_sec)
		    || (cur_time.tv_sec == next_update.tv_sec
			&& cur_time.tv_usec < next_update.tv_usec)) {
			continue;
		}

		pv_timeval_add_usec(&next_update,
				    (long) (1000000.0 * opts->interval));

		if (next_update.tv_sec < cur_time.tv_sec) {
			next_update.tv_sec = cur_time.tv_sec;
			next_update.tv_usec = cur_time.tv_usec;
		} else if (next_update.tv_sec == cur_time.tv_sec
			   && next_update.tv_usec < cur_time.tv_usec) {
			next_update.tv_usec = cur_time.tv_usec;
		}

		init_time.tv_sec =
		    start_time.tv_sec + pv_sig_toffset.tv_sec;
		init_time.tv_usec =
		    start_time.tv_usec + pv_sig_toffset.tv_usec;
		if (init_time.tv_usec >= 1000000) {
			init_time.tv_sec++;
			init_time.tv_usec -= 1000000;
		}
		if (init_time.tv_usec < 0) {
			init_time.tv_sec--;
			init_time.tv_usec += 1000000;
		}

		elapsed = cur_time.tv_sec - init_time.tv_sec;
		elapsed +=
		    (cur_time.tv_usec - init_time.tv_usec) / 1000000.0;

		if (final_update)
			since_last = -1;

		if (pv_sig_newsize) {
			pv_sig_newsize = 0;
			pv_screensize(opts);
		}

		pv_display(opts, elapsed, since_last, total_written);

		since_last = 0;
	}

	if (opts->cursor) {
		pv_crs_fini(opts);
	} else {
		if ((!opts->numeric) && (!opts->no_op))
			write(STDERR_FILENO, "\n", 1);
	}

	/*
	 * Free up the buffers used by the display and data transfer
	 * routines.
	 */
	pv_display(0, 0, 0, 0);
	pv_transfer(0, -1, 0, 0, 0, NULL);

	return 0;
}

/* EOF */
