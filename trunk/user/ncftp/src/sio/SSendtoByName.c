#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SSendtoByName(int sfd, const char *const buf, size_t size, int fl, const char *const toAddrStr, int tlen)
{
	send_return_t nwrote;
	int tleft, result;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	struct sockaddr_in toAddr;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (toAddrStr == NULL) || (toAddrStr[0] == '\0') || (tlen <= 0)) {
		errno = EINVAL;
		return (-1);
	}

	if ((result = AddrStrToAddr(toAddrStr, &toAddr, -1)) < 0) {
		return (result);
	}

	time(&now);
	done = now + tlen;
	nwrote = 0;
	forever {
		forever {
			if (now >= done) {
				errno = ETIMEDOUT;
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
				return (kTimeoutErr);
			} else if (errno != EINTR) {
				return (-1);
			}
			time(&now);
		}

		IGNORE_SIGPIPE
		nwrote = sendto(sfd, buf, (send_size_t) size, fl,
			(struct sockaddr *) &toAddr,
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
SendtoByName(int sfd, const char *const buf, size_t size, const char *const toAddrStr)
{
	int result;
	struct sockaddr_in toAddr;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (toAddrStr == NULL)) {
		errno = EINVAL;
		return (-1);
	}
	

	if ((result = AddrStrToAddr(toAddrStr, &toAddr, -1)) < 0) {
		return (result);
	}

	IGNORE_SIGPIPE
	do {
		result = (int) sendto(sfd, buf, (send_size_t) size, 0,
				(struct sockaddr *) &toAddr,
				(sockaddr_size_t) sizeof(struct sockaddr_in));
	} while ((result < 0) && (errno == EINTR));
	RESTORE_SIGPIPE

	return (result);
}	/* SendtoByName */
