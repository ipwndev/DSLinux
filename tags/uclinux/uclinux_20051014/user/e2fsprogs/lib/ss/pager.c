/*
 * Pager: Routines to create a "more" running out of a particular file
 * descriptor.
 *
 * Copyright 1987, 1988 by MIT Student Information Processing Board
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose is hereby granted, provided that
 * the names of M.I.T. and the M.I.T. S.I.P.B. not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  M.I.T. and the
 * M.I.T. S.I.P.B. make no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "ss_internal.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>

static char MORE[] = "more";
extern char *_ss_pager_name;
extern char *getenv();

/*
 * this needs a *lot* of work....
 *
 * run in same process
 * handle SIGINT sensibly
 * allow finer control -- put-page-break-here
 */

#ifndef NO_FORK
int ss_pager_create() 
{
	int filedes[2];
     
	if (pipe(filedes) != 0)
		return(-1);

#ifdef EMBED
	switch(vfork()) {
#else
	switch(fork()) {
#endif
	case -1:
		return(-1);
	case 0:
		/*
		 * Child; dup read half to 0, close all but 0, 1, and 2
		 */
		if (dup2(filedes[0], 0) == -1)
			exit(1);
		ss_page_stdin();
	default:
		/*
		 * Parent:  close "read" side of pipe, return
		 * "write" side.
		 */
		(void) close(filedes[0]);
		return(filedes[1]);
	}
}
#else /* don't fork */
int ss_pager_create()
{
    int fd;
    fd = open("/dev/tty", O_WRONLY, 0);
    return fd;
}
#endif

void ss_page_stdin()
{
	int i;
	for (i = 3; i < 32; i++)
		(void) close(i);
	(void) signal(SIGINT, SIG_DFL);
	{
#ifdef POSIX_SIGNALS
		sigset_t mask;
		
		sigprocmask(SIG_BLOCK, 0, &mask);
		sigdelset(&mask, SIGINT);
		sigprocmask(SIG_SETMASK, &mask, 0);
#else
		int mask = sigblock(0);
		mask &= ~sigmask(SIGINT);
		sigsetmask(mask);
#endif
	}
	if (_ss_pager_name == (char *)NULL) {
		if ((_ss_pager_name = getenv("PAGER")) == (char *)NULL)
			_ss_pager_name = MORE;
	}
	(void) execlp(_ss_pager_name, _ss_pager_name, (char *) NULL);
	{
		/* minimal recovery if pager program isn't found */
		char buf[80];
		register int n;
		while ((n = read(0, buf, 80)) > 0)
			write(1, buf, n);
	}
	exit(errno);
}
