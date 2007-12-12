/* u_signal.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifdef HAVE_SIGACTION
void (*NcSignal(int signum, void (*handler)(int)))(int)
{
	/* Thanks A.P.U.E. (W.R.S., R.I.P.) */
	struct sigaction sa, osa;

	(void) sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
		sa.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef SA_RESTART
		sa.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signum, &sa, &osa) < 0)
		return ((FTPSigProc) SIG_ERR);
	return (osa.sa_handler);
}
#endif	/* HAVE_SIGACTION */
