/*
 * Display functions.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "options.h"
#include "pv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>


/*
 * Fill in opts->width and opts->height with the current terminal size,
 * if possible.
 */
void pv_screensize(opts_t opts)
{
#ifdef TIOCGWINSZ
	struct winsize wsz;

	if (isatty(STDERR_FILENO)) {
		if (ioctl(STDERR_FILENO, TIOCGWINSZ, &wsz) == 0) {
			opts->width = wsz.ws_col;
			opts->height = wsz.ws_row;
		}
	}
#endif
}


/*
 * Calculate the percentage transferred so far and return it.
 */
static long pv__calc_percentage(long long so_far, long long total)
{
	if (total < 1)
		return 0;

	so_far *= 100;
	so_far /= total;

	return (long) so_far;
}


/*
 * Given how many bytes have been transferred, the total byte count to
 * transfer, and how long it's taken so far in seconds, return the estimated
 * number of seconds until completion.
 */
static long pv__calc_eta(long long so_far, long long total, long elapsed)
{
	long long amount_left;

	if (so_far < 1)
		return 0;

	amount_left = total - so_far;
	amount_left *= (long long) elapsed;
	amount_left /= so_far;

	return (long) amount_left;
}


/*
 * Output status information on standard error, where "esec" is the seconds
 * elapsed since the transfer started, "sl" is the number of bytes transferred
 * since the last update, and "tot" is the total number of bytes transferred
 * so far.
 *
 * If "sl" is negative, this is the final update so the rate is given as an
 * an average over the whole transfer; otherwise the current rate is shown.
 *
 * In line mode, "sl" and "tot" are in lines, not bytes.
 *
 * If "opts" is NULL, then free all allocated memory and return.
 */
void pv_display(opts_t opts, long double esec, long long sl, long long tot)
{
	static long percentage = 0;
	static long double prev_esec = 0;
	static long double prev_rate = 0;
	static long double prev_trans = 0;
	static char *display = NULL;
	static long dispbufsz = 0;
	long double sincelast, rate, transferred;
	long eta;
	char prefix[128];		 /* RATS: ignore (checked OK) */
	char suffix[128];		 /* RATS: ignore (checked OK) */
	char pct[64];			 /* RATS: ignore (checked OK) */
	char *units;
	int avail, i;

	if (opts == NULL) {
		if (display)
			free(display);
		display = NULL;
		return;
	}

	pv_sig_checkbg();

	if (sl >= 0) {
		sincelast = esec - prev_esec;
		if (sincelast <= 0.01) {
			rate = prev_rate;
			prev_trans += sl;
		} else {
			rate = ((long double) sl + prev_trans) / sincelast;
			prev_esec = esec;
			prev_trans = 0;
		}
	} else {
		if (esec < 0.000001)
			esec = 0.000001;
		rate = ((long double) tot) / (long double) esec;
	}
	prev_rate = rate;

	if (rate > 0)
		percentage += 2;
	if (percentage > 199)
		percentage = 0;
	eta = 0;

	/*
	 * Reallocate display buffer if width changes.
	 */
	if (display != NULL && dispbufsz < (opts->width * 2)) {
		free(display);
		display = NULL;
		dispbufsz = 0;
	}

	/*
	 * Allocate display buffer if there isn't one.
	 */
	if (display == NULL) {
		dispbufsz = opts->width + 80;
		if (opts->name)
			dispbufsz += strlen(opts->name);	/* RATS: ignore */
		display = malloc(dispbufsz + 16);
		if (display == NULL)
			return;
		display[0] = 0;
	}

	if (opts->size > 0)
		percentage = pv__calc_percentage(tot, opts->size);

	if (opts->numeric) {
		if (percentage > 100) {
			sprintf(display, "%ld\n", 200 - percentage);
		} else {
			sprintf(display, "%ld\n", percentage);
		}
		write(STDERR_FILENO, display, strlen(display));	/* RATS: ignore */
		return;
	}

	if (opts->size > 0)
		eta = pv__calc_eta(tot, opts->size, esec);

	display[0] = 0;

	if (opts->name)
		sprintf(display, "%9s: ", opts->name);	/* RATS: ignore (OK) */

	if (opts->bytes) {
		transferred = tot;

		if (opts->linemode) {
			if (transferred >= 1000 * 1000 * 1000) {
				transferred =
				    transferred / (1000 * 1000 * 1000);
				units = _("G");
			} else if (transferred >= 1000 * 1000) {
				transferred = transferred / (1000 * 1000);
				units = _("M");
			} else if (transferred >= 1000) {
				transferred = transferred / 1000;
				units = _("k");
			} else {
				units = "";
			}
		} else {
			if (transferred >= 1000 * 1024 * 1024) {
				transferred =
				    transferred / (1024 * 1024 * 1024);
				units = _("GB");
			} else if (transferred >= 1000 * 1024) {
				transferred = transferred / (1024 * 1024);
				units = _("MB");
			} else if (transferred >= 1000) {
				transferred = transferred / 1024;
				units = _("kB");
			} else {
				units = _("B");
			}
		}

		/*
		 * Bounds check, so we don't overrun the prefix buffer.
		 */
		if (transferred > 100000)
			transferred = 100000;

		sprintf(prefix, "%4.3Lg%.16s ", transferred, units);
		strcat(display, prefix);    /* RATS: ignore (OK) */
	}

	if (opts->timer) {
		/*
		 * Bounds check, so we don't overrun the prefix buffer. This
		 * does mean that the timer will stop at a 100,000 hours,
		 * but since that's 11 years, it shouldn't be a problem.
		 */
		if (esec > (long double) 360000000.0L)
			esec = (long double) 360000000.0L;

		sprintf(prefix, "%ld:%02ld:%02ld ", ((long) esec) / 3600,
			(((long) esec) / 60) % 60, ((long) esec) % 60);
		strcat(display, prefix);    /* RATS: ignore (OK) */
	}

	if (opts->rate) {
		if (opts->linemode) {
			if (rate >= 1000 * 1000 * 1000) {
				rate = rate / (1000 * 1000 * 1000);
				units = _("G/s");
			} else if (rate >= 1000 * 1000) {
				rate = rate / (1000 * 1000);
				units = _("M/s");
			} else if (rate >= 1000) {
				rate = rate / 1000;
				units = _("k/s");
			} else {
				units = _("/s ");
			}
		} else {
			if (rate >= 1000 * 1024 * 1024) {
				rate = rate / (1024 * 1024 * 1024);
				units = _("GB/s");
			} else if (rate >= 1000 * 1024) {
				rate = rate / (1024 * 1024);
				units = _("MB/s");
			} else if (rate >= 1000) {
				rate = rate / 1024;
				units = _("kB/s");
			} else {
				units = _("B/s ");
			}
		}

		/*
		 * Bounds check, so we don't overrun the prefix buffer.
		 * Hopefully 97TB/sec is fast enough.
		 */
		if (rate > 100000)
			rate = 100000;

		sprintf(prefix, "[%4.3Lg%.16s] ", rate, units);
		strcat(display, prefix);    /* RATS: ignore (OK) */
	}

	suffix[0] = 0;
	if (opts->eta && opts->size > 0) {
		if (eta < 0)
			eta = 0;

		/*
		 * Bounds check, so we don't overrun the suffix buffer. This
		 * means the ETA will always be less than 100,000 hours.
		 */
		if (eta > (long) 360000000L)
			eta = (long) 360000000L;

		sprintf(suffix, " %.16s %ld:%02ld:%02ld", _("ETA"),
			eta / 3600, (eta / 60) % 60, eta % 60);

		/*
		 * If this is the final update, show a blank space where the
		 * ETA used to be.
		 */
		if (sl < 0) {
			for (i = 0; i < sizeof(suffix) && suffix[i] != 0;
			     i++) {
				suffix[i] = ' ';
			}
		}
	}

	if (opts->progress) {
		strcat(display, "[");
		if (opts->size > 0) {
			if (percentage < 0)
				percentage = 0;
			if (percentage > 100000)
				percentage = 100000;
			sprintf(pct, "%2ld%%", percentage);
			avail = opts->width - strlen(display)	/* RATS: ignore */
			    -strlen(suffix) /* RATS: ignore */
			    -strlen(pct) - 3;	/* RATS: ignore */
			for (i = 0; i < (avail * percentage) / 100 - 1;
			     i++) {
				if (i < avail)
					strcat(display, "=");
			}
			if (i < avail) {
				strcat(display, ">");
				i++;
			}
			for (; i < avail; i++) {
				strcat(display, " ");
			}
			strcat(display, "] ");
			strcat(display, pct);	/* RATS: ignore (OK) */
		} else {
			int p = percentage;
			avail = opts->width - strlen(display)	/* RATS: ignore */
			    -strlen(suffix) - 5;	/* RATS: ignore */
			if (p > 100)
				p = 200 - p;
			for (i = 0; i < (avail * p) / 100; i++) {
				if (i < avail)
					strcat(display, " ");
			}
			strcat(display, "<=>");
			for (; i < avail; i++) {
				strcat(display, " ");
			}
			strcat(display, "]");
		}
	}

	strcat(display, suffix);	    /* RATS: ignore (OK) */

	if (opts->cursor) {
		pv_crs_update(opts, display);
	} else {
		write(STDERR_FILENO, display, strlen(display));	/* RATS: ignore */
		write(STDERR_FILENO, "\r", 1);
	}
}

/* EOF */
