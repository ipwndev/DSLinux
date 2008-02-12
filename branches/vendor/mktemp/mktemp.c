/*	$OpenBSD: mktemp.c,v 1.6 2001/10/01 17:08:30 millert Exp $	*/

/*
 * Copyright (c) 1996, 2000, 2001 Todd C. Miller <Todd.Miller@courtesan.com>
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

#ifndef lint                                                              
static const char rcsid[] = "$Id: mktemp.c,v 1.9 2001/11/12 20:02:06 millert Exp $";
#endif /* not lint */                                                        

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#  include <memory.h>
# endif
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif /* HAVE_STRINGS_H */
#endif /* HAVE_STRING_H */
#if defined(HAVE_MALLOC_H) && !defined(STDC_HEADERS)
#include <malloc.h>
#endif /* HAVE_MALLOC_H && !STDC_HEADERS */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif /* HAVE_PATHS_H */
#include <errno.h>

#include <extern.h>

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#ifdef HAVE_PROGNAME
extern char *__progname;
#else
char *__progname;
#endif

void usage __P((void)) __attribute__((__noreturn__));

int
main(argc, argv)
	int argc;
	char **argv;
{
	int ch, fd, uflag = 0, quiet = 0, tflag = 0, makedir = 0;
	char *cp, *template, *tempfile, *prefix = _PATH_TMP;
	size_t plen;
	extern char *optarg;
	extern int optind;

#ifndef HAVE_PROGNAME
	__progname = argv[0];
#endif

	while ((ch = getopt(argc, argv, "dp:qtuV")) != -1)
		switch(ch) {
		case 'd':
			makedir = 1;
			break;
		case 'p':
			prefix = optarg;
			tflag = 1;
			break;
		case 'q':
			quiet = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'u':
			uflag = 1;
			break;
		case 'V':
			printf("%s version %s\n", __progname, MKTEMP_VERSION);
			exit(0);
		default:
			usage();
	}

	/* If no template specified use a default one (implies -t mode) */
	switch (argc - optind) {
	case 1:
		template = argv[optind];
		break;
	case 0:
		template = "tmp.XXXXXXXXXX";
		tflag = 1;
		break;
	default:
		usage();
	}

	if (tflag) {
		if (strchr(template, '/')) {
			if (!quiet)
				(void)fprintf(stderr,
				    "%s: template must not contain directory separators in -t mode\n", __progname);
			exit(1);
		}

		cp = getenv("TMPDIR");
		if (cp != NULL && *cp != '\0')
			prefix = cp;
		plen = strlen(prefix);
		while (plen != 0 && prefix[plen - 1] == '/')
			plen--;

		tempfile = (char *)malloc(plen + 1 + strlen(template) + 1);
		if (tempfile == NULL) {
			if (!quiet)
				(void)fprintf(stderr,
				    "%s: cannot allocate memory\n", __progname);
			exit(1);
		}
		(void)memcpy(tempfile, prefix, plen);
		tempfile[plen] = '/';
		(void)strcpy(tempfile + plen + 1, template);	/* SAFE */
	} else {
		if ((tempfile = strdup(template)) == NULL) {
			if (!quiet)
				(void)fprintf(stderr,
				    "%s: cannot allocate memory\n", __progname);
			exit(1);
		}
	}

	if (makedir) {
		if (MKDTEMP(tempfile) == NULL) {
			if (!quiet) {
				(void)fprintf(stderr,
				    "%s: cannot make temp dir %s: %s\n",
				    __progname, tempfile, strerror(errno));
			}
			exit(1);
		}

		if (uflag)
			(void)rmdir(tempfile);
	} else {
		if ((fd = MKSTEMP(tempfile)) < 0) {
			if (!quiet) {
				(void)fprintf(stderr,
				    "%s: cannot create temp file %s: %s\n",
				    __progname, tempfile, strerror(errno));
			}
			exit(1);
		}
		(void)close(fd);

		if (uflag)
			(void)unlink(tempfile);
	}

	(void)puts(tempfile);
	free(tempfile);

	exit(0);
}

void
usage()
{

	(void)fprintf(stderr,
	    "Usage: %s [-V] | [-dqtu] [-p prefix] [template]\n",
	    __progname);
	exit(1);
}
