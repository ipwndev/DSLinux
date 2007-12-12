#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
USendto(int sfd, const char *const buf, size_t size, int fl, const struct sockaddr_un *const toAddr, int ualen, int tlen)
{
	send_return_t nwrote;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (toAddr == NULL) || (tlen <= 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	time(&now);
	done = now + tlen;
	nwrote = 0;
	forever {
		forever {
			if (now >= done) {
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			}
			tleft = (done > now) ? ((int) (done - now)) : 0;
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
			result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG5 &tv);
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
			time(&now);
		}

		IGNORE_SIGPIPE
		nwrote = sendto(sfd, buf, (send_size_t) size, fl,
			(const struct sockaddr *) toAddr,
			(sockaddr_size_t) ualen);
		RESTORE_SIGPIPE

		if (nwrote >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
	}

	return ((int) nwrote);
}	/* USendto */
