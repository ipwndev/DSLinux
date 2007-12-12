#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef HAVE_RECVMSG
int
SRecvmsg(int UNUSED(sfd), void *const UNUSED(msg), int UNUSED(fl), int UNUSED(tlen))
{
	LIBSIO_USE_VAR(sfd);
	LIBSIO_USE_VAR(msg);
	LIBSIO_USE_VAR(fl);
	LIBSIO_USE_VAR(tlen);
#	ifdef ENOSYS
	errno = ENOSYS;
#	endif
	return (-1);
}


#else

int
SRecvmsg(int sfd, void *const msg, int fl, int tlen)
{
	recv_return_t nread;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;
	DECL_SIGPIPE_VARS
	
	if (msg == NULL) {
		errno = EINVAL;
		return (-1);
	}
	
	if (tlen <= 0) {
		errno = 0;
		for (;;) {
			nread = recvmsg(sfd, (struct msghdr *) msg, fl);
			if ((nread >= 0) || (errno != EINTR))
				return ((int) nread);
		}
	}

	time(&now);
	done = now + tlen;
	tleft = (done > now) ? ((int) (done - now)) : 0;
	forever {
				
		for (;;) {
			errno = 0;
			MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
			MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
			tv.tv_sec = (tv_sec_t) tleft;
			tv.tv_usec = 0;
			result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, SELECT_TYPE_ARG5 &tv);
			if (result == 1) {
				/* ready */
				break;
			} else if (result == 0) {
				/* timeout */
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			} else if (errno != EINTR) {
				return (-1);
			}
		}

		IGNORE_SIGPIPE
		nread = recvmsg(sfd, (struct msghdr *) msg, fl);
		RESTORE_SIGPIPE

		if (nread >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (done > now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nread = kTimeoutErr;
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			break;
		}
	}

	return ((int) nread);
}	/* SRecvmsg */

#endif
