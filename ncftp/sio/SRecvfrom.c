#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SRecvfrom(int sfd, char *const buf, size_t size, int fl, struct sockaddr_in *const fromAddr, int tlen)
{
	recv_return_t nread;
	int tleft;
	fd_set ss;
	struct timeval tv;
	int result;
	time_t done, now;
	sockaddr_size_t alen;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (fromAddr == NULL) || (tlen <= 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	time(&now);
	done = now + tlen;
	tleft = (done > now) ? ((int) (done - now)) : 0;
	nread = 0;
	forever {
		alen = (sockaddr_size_t) sizeof(struct sockaddr_in);
				
		forever {
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
		nread = recvfrom(sfd, buf, (recv_size_t) size, fl,
			(struct sockaddr *) fromAddr, &alen);
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
}	/* SRecvfrom */
