#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* 
 * Return zero if the operation timed-out or erred-out, otherwise non-zero.
 */
int
SWaitUntilReadyForReading(const int sfd, const int tlen)
{
	fd_set ss, ss2;
	struct timeval tv;
	int result;
	int tleft;
	time_t now, done;

	if (sfd < 0) {
		errno = EBADF;
		return (0);
	}
	errno = 0;

	if (tlen < 0) {
		forever {
			MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
			MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
			ss2 = ss;
			result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG234 &ss2, NULL);
			if (result == 1) {
				/* ready */
				return (1);
			} else if ((result < 0) && (errno != EINTR)) {
				/* error */
				return (0);
			}
			/* else try again */
		}
		/*NOTREACHED*/
	} else if (tlen == 0) {
		forever {
			MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
			MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
			ss2 = ss;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG234 &ss2, &tv);
			if (result == 1) {
				/* ready */
				return (1);
			} else if (result == 0) {
				/* timed-out */
				errno = ETIMEDOUT;
				return (0);
			} else if ((result < 0) && (errno != EINTR)) {
				/* error */
				return (0);
			}
			/* else try again */
		}
		/*NOTREACHED*/
	}
	
	time(&now);
	done = now + tlen;
	tleft = tlen;

	forever {
		MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
		MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
		ss2 = ss;
		tv.tv_sec = (tv_sec_t) tleft;
		tv.tv_usec = 0;
		result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG234 &ss2, &tv);
		if (result == 1) {
			/* ready */
			return (1);
		} else if (result < 0) {
			if (errno != EINTR) {
				/* error */
				break;
			}
			/* try again */
			time(&now);
			if (now > done) {
				/* timed-out */
				errno = ETIMEDOUT;
				break;
			}
			tleft = (int) (done - now);
		} else {
			/* timed-out */
			errno = ETIMEDOUT;
			break;
		}
	}

	return (0);
}	/* SWaitUntilReadyForReading */




/* 
 * Return zero if the operation timed-out or erred-out, otherwise non-zero.
 */
int
SWaitUntilReadyForWriting(const int sfd, const int tlen)
{
	fd_set ss, ss2;
	struct timeval tv;
	int result;
	int tleft;
	time_t now, done;

	if (sfd < 0) {
		errno = EBADF;
		return (0);
	}
	errno = 0;

	if (tlen < 0) {
		forever {
			MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
			MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
			ss2 = ss;
			result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &ss2, NULL);
			if (result == 1) {
				/* ready */
				return (1);
			} else if ((result < 0) && (errno != EINTR)) {
				/* error */
				return (0);
			}
			/* else try again */
		}
		/*NOTREACHED*/
	} else if (tlen == 0) {
		forever {
			MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
			MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
			ss2 = ss;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &ss2, &tv);
			if (result == 1) {
				/* ready */
				return (1);
			} else if (result == 0) {
				/* timed-out */
				errno = ETIMEDOUT;
				return (0);
			} else if ((result < 0) && (errno != EINTR)) {
				/* error */
				return (0);
			}
			/* else try again */
		}
		/*NOTREACHED*/
	}

	time(&now);
	done = now + tlen;
	tleft = tlen;

	forever {
		MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
		MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
		ss2 = ss;
		tv.tv_sec = (tv_sec_t) tleft;
		tv.tv_usec = 0;
		result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &ss2, &tv);
		if (result == 1) {
			/* ready */
			return (1);
		} else if (result < 0) {
			if (errno != EINTR) {
				/* error */
				break;
			}
			/* try again */
			time(&now);
			if (now > done) {
				/* timed-out */
				errno = ETIMEDOUT;
				break;
			}
			tleft = (int) (done - now);
		} else {
			/* timed-out */
			errno = ETIMEDOUT;
			break;
		}
	}

	return (0);
}	/* SWaitUntilReadyForWriting */
