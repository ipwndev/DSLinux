#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SRecv(int sfd, char *const buf0, size_t size, int fl, int tlen, int retry)
{
	read_return_t nread;
	read_size_t nleft;
	char *buf = buf0;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result, firstRead;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0) || (tlen <= 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE
	errno = 0;

	nleft = (read_size_t) size;
	time(&now);
	done = now + tlen;
	firstRead = 1;

	forever {
		tleft = (done > now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nread = (read_return_t) size - (read_return_t) nleft;
			if ((nread == 0) || ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) != 0)) {
				nread = kTimeoutErr;
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
			}
			goto done;
		}
		
		if (!firstRead || ((retry & kNoFirstSelect) == 0)) {
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
					nread = (read_return_t) size - (read_return_t) nleft;
					if ((nread > 0) && ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast))  == 0)) {
						RESTORE_SIGPIPE
						return ((int) nread);
					}
					errno = ETIMEDOUT;
					SETWSATIMEOUTERR
					RESTORE_SIGPIPE
					return (kTimeoutErr);
				} else if (errno != EINTR) {
					RESTORE_SIGPIPE
					return (-1);
				}
			}
			firstRead = 0;
		}

		nread = recv(sfd, (char *) buf, (recv_size_t) nleft, fl);

		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				if (retry == ((retry & (kFullBufferRequiredExceptLast)) != 0))
					nread = (read_return_t) size - (read_return_t) nleft;
				goto done;
			} else if (errno != EINTR) {
				nread = (read_return_t) size - (read_return_t) nleft;
				if (nread == 0)
					nread = (read_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nread = 0;
				/* Try again. */
			}
		}
		nleft -= (read_size_t) nread;
		if ((nleft == 0) || (((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0) && (nleft != (read_size_t) size)))
			break;
		buf += nread;
		time(&now);
	}
	nread = (read_return_t) size - (read_return_t) nleft;

done:
	RESTORE_SIGPIPE
	return ((int) nread);
}	/* SRecv */

