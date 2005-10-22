/*	$NetBSD: input.c,v 1.9 2003/08/07 09:37:47 agc Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek and Darren F. Provine.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)input.c	8.1 (Berkeley) 5/31/93
 */

/*
 * Tetris input.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <errno.h>
#include <unistd.h>

#include "input.h"
#include "tetris.h"

/* return true iff the given timeval is positive */
#define	TV_POS(tv) \
	((tv)->tv_sec > 0 || ((tv)->tv_sec == 0 && (tv)->tv_usec > 0))

/* subtract timeval `sub' from `res' */
#define	TV_SUB(res, sub) \
	(res)->tv_sec -= (sub)->tv_sec; \
	(res)->tv_usec -= (sub)->tv_usec; \
	if ((res)->tv_usec < 0) { \
		(res)->tv_usec += 1000000; \
		(res)->tv_sec--; \
	}

/*
 * Do a `read wait': poll for reading from stdin, with timeout *tvp.
 * On return, modify *tvp to reflect the amount of time spent waiting.
 * It will be positive only if input appeared before the time ran out;
 * otherwise it will be zero or perhaps negative.
 *
 * If tvp is nil, wait forever, but return if poll is interrupted.
 *
 * Return 0 => no input, 1 => can read() from stdin
 */
int
rwait(tvp)
	struct timeval *tvp;
{
	struct pollfd set[1];
	struct timeval starttv, endtv;
	int timeout;
#define	NILTZ ((struct timezone *)0)

	if (tvp) {
		(void) gettimeofday(&starttv, NILTZ);
		endtv = *tvp;
		timeout = tvp->tv_sec * 1000 + tvp->tv_usec / 1000;
	} else
		timeout = INFTIM;
again:
	set[0].fd = STDIN_FILENO;
	set[0].events = POLLIN;
	switch (poll(set, 1, timeout)) {

	case -1:
		if (tvp == 0)
			return (-1);
		if (errno == EINTR)
			goto again;
		stop("poll failed, help");
		/* NOTREACHED */

	case 0:	/* timed out */
		tvp->tv_sec = 0;
		tvp->tv_usec = 0;
		return (0);
	}
	if (tvp) {
		/* since there is input, we may not have timed out */
		(void) gettimeofday(&endtv, NILTZ);
		TV_SUB(&endtv, &starttv);
		TV_SUB(tvp, &endtv);	/* adjust *tvp by elapsed time */
	}
	return (1);
}

/*
 * `sleep' for the current turn time.
 * Eat any input that might be available.
 */
void
tsleep()
{
	struct timeval tv;
	char c;

	tv.tv_sec = 0;
	tv.tv_usec = fallrate;
	while (TV_POS(&tv))
		if (rwait(&tv) && read(0, &c, 1) != 1)
			break;
}

/*
 * getchar with timeout.
 */
int
tgetchar()
{
	static struct timeval timeleft;
	char c;

	/*
	 * Reset timeleft to fallrate whenever it is not positive.
	 * In any case, wait to see if there is any input.  If so,
	 * take it, and update timeleft so that the next call to
	 * tgetchar() will not wait as long.  If there is no input,
	 * make timeleft zero or negative, and return -1.
	 *
	 * Most of the hard work is done by rwait().
	 */
	if (!TV_POS(&timeleft)) {
		faster();	/* go faster */
		timeleft.tv_sec = 0;
		timeleft.tv_usec = fallrate;
	}
	if (!rwait(&timeleft))
		return (-1);
	if (read(0, &c, 1) != 1)
		stop("end of file, help");
	return ((int)(unsigned char)c);
}
