/*
 * Copyright (c) 2000, 2001 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <sys/types.h>
#if defined(TIME_WITH_SYS_TIME) || defined(HAVE_SYS_TIME_H)
# include <sys/time.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if defined(TIME_WITH_SYS_TIME) || !defined(HAVE_SYS_TIME_H)
# include <time.h>
#endif

#include <extern.h>

#ifdef HAVE_RANDOM
# define RAND	random
# define SRAND	srandom
# define SEED_T	unsigned int
#elif defined (HAVE_LRAND48)
# define RAND	lrand48
# define SRAND	srand48
# define SEED_T	long
#else
# define RAND	rand
# define SRAND	srand
# define SEED_T	unsigned int
#endif

#ifndef lint
static const char rcsid[] = "$Id: random.c,v 1.4 2001/10/01 00:21:34 millert Exp $";
#endif /* not lint */

#ifdef HAVE_SRANDOMDEV
void
seed_random()
{
	srandomdev();
}
#else !HAVE_SRANDOMDEV
#ifdef _PATH_RANDOM
static ssize_t
read_loop(fd, buf, count)
	int fd;
	void *buf;
	size_t count;
{
	char *buffer = (char *) buf;
	ssize_t offset, block;

	offset = 0;
	while (count > 0) {
		block = read(fd, &buffer[offset], count);

		if (block < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return(block);
		} else if (block == 0)
			return(offset);

		offset += block;
		count -= block;
	}

	return(offset);
}
#endif /* _PATH_RANDOM */
void
seed_random()
{
	SEED_T seed;
	struct timeval tv;

#ifdef _PATH_RANDOM
	int fd;

	if ((fd = open(_PATH_RANDOM, O_RDONLY)) != -1 &&
	    read_loop(fd, &seed, sizeof(seed)) == sizeof(seed)) {
		close(fd);
		SRAND(seed);
		return;
	}
#endif /* _PATH_RANDOM */

	/* Don't have /dev/random */
	/* XXX - check for gettimeofday() and use time() if none? */
	gettimeofday(&tv, NULL);
	/* Use time of day and process id  multiplied by small primes */
	seed = (tv.tv_sec % 10000) * 523 + tv.tv_usec * 13 + (getpid() % 1000) * 983;
	SRAND(seed);
}
#endif /* !HAVE_SRANDOMDEV */

long
get_random()
{
	static int initialized;

	if (!initialized) {
		seed_random();
		initialized = 1;
	}

	return(RAND());
}
