#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SSendto(int sfd, const char *const buf, size_t size, int fl, const struct sockaddr_in *const toAddr, int tlen)
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
			(sockaddr_size_t) sizeof(struct sockaddr_in));
		RESTORE_SIGPIPE

		if (nwrote >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
	}

	return ((int) nwrote);
}	/* SSendto */





int
Sendto(int sfd, const char *const buf, size_t size, const struct sockaddr_in *const toAddr)
{
	int result;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (toAddr == NULL)) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE
	do {
		result = (int) sendto(sfd, buf, (send_size_t) size, 0,
				(const struct sockaddr *) toAddr,
				(sockaddr_size_t) sizeof(struct sockaddr_in));
	} while ((result < 0) && (errno == EINTR));
	RESTORE_SIGPIPE
	return (result);
}	/* Sendto */
