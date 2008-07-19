#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
SAccept(int sfd, struct sockaddr_in *const addr, int tlen)
{
	int result;
	fd_set ss;
	struct timeval tv;
	sockaddr_size_t size;
	DECL_SIGPIPE_VARS
	
	if (addr == NULL) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE

	if (tlen <= 0) {
		errno = 0;
		for (;;) {
			size = (sockaddr_size_t) sizeof(struct sockaddr_in);
			result = accept(sfd, (struct sockaddr *) addr, &size);
			if ((result >= 0) || (errno != EINTR)) {
				RESTORE_SIGPIPE
				return (result);
			}
		}
	}

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
		tv.tv_sec = (tv_sec_t) tlen;
		tv.tv_usec = 0;
		result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, SELECT_TYPE_ARG5 &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			RESTORE_SIGPIPE
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			RESTORE_SIGPIPE
			return (-1);
		}
	}

	do {
		size = (sockaddr_size_t) sizeof(struct sockaddr_in);
		result = accept(sfd, (struct sockaddr *) addr, &size);
	} while ((result < 0) && (errno == EINTR));

	RESTORE_SIGPIPE
	return (result);
}	/* SAccept */
