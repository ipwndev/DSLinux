/*
 * Nail - a mail user agent derived from Berkeley Mail.
 *
 * Copyright (c) 2000-2004 Gunnar Ritter, Freiburg i. Br., Germany.
 */
/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
 */

#ifndef lint
#ifdef	DOSCCS
static char sccsid[] = "@(#)v7.local.c	2.9 (gritter) 10/2/04";
#endif
#endif /* not lint */

/*
 * Mail -- a mail program
 *
 * Version 7
 *
 * Local routines that are installation dependent.
 */

#include "rcv.h"
#include "extern.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).
 */
void
findmail(char *user, int force, char *buf, int size)
{
	char *mbox, *cp;

	if (strcmp(user, myname) == 0 && !force &&
			(cp = value("folder")) != NULL &&
			which_protocol(cp) == PROTO_IMAP) {
		snprintf(buf, size, "%s/INBOX", protbase(cp));
	} else if (force || (mbox = value("MAIL")) == NULL) {
		snprintf(buf, size, "%s/%s", MAILSPOOL, user);
	} else {
		strncpy(buf, mbox, size);
		buf[size-1]='\0';
	}
}

/*
 * Get rid of the queued mail.
 */
void 
demail(void)
{

	if (value("keep") != NULL || rm(mailname) < 0)
		close(creat(mailname, 0600));
}

/*
 * Discover user login name.
 */
char *
username(void)
{
	char *np;
	uid_t uid;

	if ((np = getenv("USER")) != NULL)
		return np;
	if ((np = getname(uid = getuid())) != NULL)
		return np;
	printf(catgets(catd, CATSET, 201,
		"Cannot associate a name with uid %d\n"), (int)uid);
	return NULL;
}
