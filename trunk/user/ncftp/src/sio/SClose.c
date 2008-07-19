#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SCloseSocket(int sfd)
{
	int result;
	DECL_SIGPIPE_VARS
	
	IGNORE_SIGPIPE
	result = closesocket(sfd);
	RESTORE_SIGPIPE
	
	return (result);
}	/* SCloseSocket */



#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
#endif

int
SClose(int sfd, int tlen)
{
#ifdef UNIX_SIGNALS
	volatile sio_sigproc_t sigalrm = (sio_sigproc_t) 0;
	volatile sio_sigproc_t sigpipe = (sio_sigproc_t) 0;
	volatile alarm_time_t oalarm = 0;
	int result;
	int oerrno;
	
	if (sfd < 0) {
		errno = EBADF;
		return (-1);
	}

	if (GetSocketLinger(sfd, NULL) <= 0) {
		/* Linger wasn't on, so close shouldn't block.
		 * Take the regular way out.
		 */
		return (SCloseSocket(sfd));
	}
	
	if (tlen < 1) {
		/* Don't time it, shut it down now. */
		if (SetSocketLinger(sfd, 0, 0) == 0) {
			/* Linger disabled, so close()
			 * should not block.
			 */
			return (SCloseSocket(sfd));
		} else {
			/* This may result in a fd leak,
			 * but it's either that or hang forever.
			 */
			(void) shutdown(sfd, 2);
			return (SCloseSocket(sfd));
		}
	}

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		(void) alarm(0);
		(void) SetSocketLinger(sfd, 0, 0);
		errno = 0;
		(void) shutdown(sfd, 2);
		result = closesocket(sfd);
		oerrno = errno;
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		(void) alarm(oalarm);
		errno = oerrno;
		return (result);
	}

	sigalrm = (sio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (sio_sigproc_t) SSignal(SIGPIPE, SIG_IGN);

	oalarm = alarm((alarm_time_t) tlen);
	for (errno = 0;;) {
		result = closesocket(sfd);
		if (result == 0)
			break;
		if (errno != EINTR)
			break;
	}
	oerrno = errno;
	(void) alarm(0);

	if ((result != 0) && (errno != EBADF)) {
		(void) SetSocketLinger(sfd, 0, 0);
		(void) shutdown(sfd, 2);
		result = closesocket(sfd);
		oerrno = errno;
	}
	
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
	(void) alarm(oalarm);
	errno = oerrno;
	
	return (result);
#else	/* ! UNIX_SIGNALS */
	if (sfd < 0) {
		errno = EBADF;
		return (-1);
	}
	
	/* Sorry... it's up to you to make sure you don't block forever
	 * on closesocket() since this platform doesn't have alarm().
	 * Even so, it shouldn't be a problem unless you use linger mode
	 * on the socket, and nobody does that these days.
	 */
	return (SCloseSocket(sfd));
#endif
}	/* SClose */
