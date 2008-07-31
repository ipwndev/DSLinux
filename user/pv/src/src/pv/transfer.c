/*
 * Functions for transferring between file descriptors.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#include "options.h"

#define BUFFER_SIZE	409600
#define BUFFER_SIZE_MAX	524288

#define MAXIMISE_BUFFER_FILL	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static unsigned long long pv__bufsize = BUFFER_SIZE;


/*
 * Set the buffer size for transfers.
 */
void pv_set_buffer_size(unsigned long long sz, int force)
{
	if ((sz > BUFFER_SIZE_MAX) && (!force))
		sz = BUFFER_SIZE_MAX;
	pv__bufsize = sz;
}


/*
 * Transfer some data from "fd" to standard output, timing out after 9/100
 * of a second. If opts->rate_limit is >0, only up to "allowed" bytes can
 * be written. The variables that "eof_in" and "eof_out" point to are used
 * to flag that we've finished reading and writing respectively.
 *
 * Returns the number of bytes written, or negative on error. In line mode,
 * the number of lines written will be put into *lineswritten.
 *
 * If "opts" is NULL, then the transfer buffer is freed, and zero is
 * returned.
 */
long pv_transfer(opts_t opts, int fd, int *eof_in, int *eof_out,
		 unsigned long long allowed, long *lineswritten)
{
	static unsigned char *buf = NULL;
	static unsigned long long buf_alloced = 0;
	static unsigned long in_buffer = 0;
	static unsigned long bytes_written = 0;
	struct timeval tv;
	fd_set readfds;
	fd_set writefds;
	int max_fd;
	long to_write, written;
	ssize_t r, w;
	int n;

	if (opts == NULL) {
		if (buf)
			free(buf);
		buf = NULL;
		in_buffer = 0;
		bytes_written = 0;
		return 0;
	}

	if (buf == NULL) {
		buf_alloced = pv__bufsize;
		buf = (unsigned char *) malloc(pv__bufsize + 32);
		if (buf == NULL) {
			fprintf(stderr, "%s: %s: %s\n",
				opts->program_name,
				_("buffer allocation failed"),
				strerror(errno));
			return -1;
		}
	}

	/*
	 * Reallocate the buffer if the buffer size has changed mid-transfer.
	 */
	if (buf_alloced < pv__bufsize) {
		unsigned char *newptr;
		newptr =
		    realloc( /* RATS: ignore */ buf, pv__bufsize + 32);
		if (newptr == NULL) {
			pv__bufsize = buf_alloced;
		} else {
			buf = newptr;
			buf_alloced = pv__bufsize;
		}
	}

	if ((opts->linemode) && (lineswritten != NULL))
		*lineswritten = 0;

	tv.tv_sec = 0;
	tv.tv_usec = 90000;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	max_fd = 0;

	if ((!(*eof_in)) && (in_buffer < pv__bufsize)) {
		FD_SET(fd, &readfds);
		if (fd > max_fd)
			max_fd = fd;
	}

	to_write = in_buffer - bytes_written;
	if (opts->rate_limit > 0) {
		if (to_write > allowed) {
			to_write = allowed;
		}
	}

	if ((!(*eof_out)) && (to_write > 0)) {
		FD_SET(STDOUT_FILENO, &writefds);
		if (STDOUT_FILENO > max_fd)
			max_fd = STDOUT_FILENO;
	}

	if ((*eof_in) && (*eof_out))
		return 0;

	n = select(max_fd + 1, &readfds, &writefds, NULL, &tv);

	if (n < 0) {
		if (errno == EINTR)
			return 0;
		fprintf(stderr, "%s: %s: %s: %d: %s\n",
			opts->program_name, opts->current_file,
			_("select call failed"), n, strerror(errno));
		return -1;
	}

	written = 0;

	if (FD_ISSET(fd, &readfds)) {
		r = read( /* RATS: ignore (checked OK) */ fd,
			 buf + in_buffer, pv__bufsize - in_buffer);
		if (r < 0) {
			/*
			 * If a read error occurred but it was EINTR or
			 * EAGAIN, just wait a bit and then return zero,
			 * since this was a transient error.
			 */
			if ((errno == EINTR) || (errno == EAGAIN)) {
				tv.tv_sec = 0;
				tv.tv_usec = 10000;
				select(0, NULL, NULL, NULL, &tv);
				return 0;
			}
			fprintf(stderr, "%s: %s: %s: %s\n",
				opts->program_name,
				opts->current_file,
				_("read failed"), strerror(errno));
			*eof_in = 1;
			if (bytes_written >= in_buffer)
				*eof_out = 1;
		} else if (r == 0) {
			*eof_in = 1;
			if (bytes_written >= in_buffer)
				*eof_out = 1;
		} else {
			in_buffer += r;
		}
	}

	/*
	 * In line mode, only write up to and including the first newline,
	 * so that we're writing output line-by-line.
	 */
	if (opts->linemode) {
		int ckidx;
		for (ckidx = 1; ckidx < to_write; ckidx++) {
			if (buf[bytes_written + ckidx - 1] == '\n')
				to_write = ckidx;
		}
	}

	if (FD_ISSET(STDOUT_FILENO, &writefds)
	    && (in_buffer > bytes_written)
	    && (to_write > 0)) {

		signal(SIGALRM, SIG_IGN);   /* RATS: ignore */
		alarm(1);

		w = write(STDOUT_FILENO, buf + bytes_written, to_write);

		alarm(0);

		if (w < 0) {
			/*
			 * If a write error occurred but it was EINTR or
			 * EAGAIN, just wait a bit and then return zero,
			 * since this was a transient error.
			 */
			if ((errno == EINTR) || (errno == EAGAIN)) {
				tv.tv_sec = 0;
				tv.tv_usec = 10000;
				select(0, NULL, NULL, NULL, &tv);
				return 0;
			}
			/*
			 * SIGPIPE means we've finished. Don't output an
			 * error because it's not really our error to report. 
			 */
			if (errno == EPIPE) {
				*eof_in = 1;
				*eof_out = 1;
				return 0;
			}
			fprintf(stderr, "%s: %s: %s\n",
				opts->program_name,
				_("write failed"), strerror(errno));
			*eof_out = 1;
			written = -1;
		} else if (w == 0) {
			*eof_out = 1;
		} else {
			if (opts->linemode) {
				int nlidx;
				for (nlidx = 0; nlidx < w; nlidx++) {
					if (buf[bytes_written + nlidx] ==
					    '\n') {
						*lineswritten =
						    1 + (*lineswritten);
					}
				}
			}
			bytes_written += w;
			written += w;
			if (bytes_written >= in_buffer) {
				bytes_written = 0;
				in_buffer = 0;
				if (*eof_in)
					*eof_out = 1;
			}
		}
	}
#ifdef MAXIMISE_BUFFER_FILL
	/*
	 * Rotate the written bytes out of the buffer so that it can be
	 * filled up completely by the next read.
	 */
	if (bytes_written > 0) {
		if (bytes_written < in_buffer) {
			memmove(buf, buf + bytes_written,
				in_buffer - bytes_written);
			in_buffer -= bytes_written;
			bytes_written = 0;
		} else {
			bytes_written = 0;
			in_buffer = 0;
		}
	}
#endif				/* MAXIMISE_BUFFER_FILL */

	return written;
}

/* EOF */
