#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
UAccept(int sfd, struct sockaddr_un *const addr, int *ualen, int tlen)
{
	int result;
	fd_set ss;
	struct timeval tv;
	sockaddr_size_t ualen2;
	DECL_SIGPIPE_VARS
	
	if (addr == NULL) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE

	if (tlen <= 0) {
		errno = 0;
		for (;;) {
			ualen2 = (sockaddr_size_t) sizeof(struct sockaddr_un);
			result = accept(sfd, (struct sockaddr *) addr, &ualen2);
			if (ualen != NULL)
				*ualen = (int) ualen2;
			if ((result >= 0) || (errno != EINTR)) {
				RESTORE_SIGPIPE
				return (result);
			}
		}
	}

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
		tv.tv_sec = (tv_sec_t) tlen;
		tv.tv_usec = 0;
		result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */
			errno = ETIMEDOUT;
			RESTORE_SIGPIPE
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			RESTORE_SIGPIPE
			return (-1);
		}
	}

	do {
		ualen2 = (sockaddr_size_t) sizeof(struct sockaddr_un);
		result = accept(sfd, (struct sockaddr *) addr, &ualen2);
		if (ualen != NULL)
			*ualen = (int) ualen2;
	} while ((result < 0) && (errno == EINTR));

	RESTORE_SIGPIPE
	return (result);
}	/* UAccept */
