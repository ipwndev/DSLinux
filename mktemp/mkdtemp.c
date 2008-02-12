/*
 * Copyright (c) 2000, 2001 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
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
#include <sys/stat.h>

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif /* HAVE_STRINGS_H */
#endif /* HAVE_STRING_H */
#if defined(HAVE_MALLOC_H) && !defined(STDC_HEADERS)
# include <malloc.h>
#endif /* HAVE_MALLOC_H && !STDC_HEADERS */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include <extern.h>

#ifndef lint
static const char rcsid[] = "$Id: mkdtemp.c,v 1.5 2001/11/12 20:02:06 millert Exp $";
#endif /* lint */

/*
 * Very simple-minded mkdtemp() replacement.
 */
char *
mkdtemp(rtemplate)
	char *rtemplate;
{
	char *template;
	int i, oerrno, error;

	if (!(template = malloc(strlen(rtemplate) + 1)))
		return(NULL);

	for (error = 0, i = 0; i < 1000; i++) {
		(void) strcpy(template, rtemplate);
		if (mktemp(template) == NULL) {
			error = 1;
			break;
		}
		error = mkdir(template, 0700);
		if (!error || errno != EEXIST)
			break;
	}
	oerrno = errno;
	if (*template)
		(void) strcpy(rtemplate, template);
	free(template);
	errno = oerrno;
	return(error ? NULL : rtemplate);
}
