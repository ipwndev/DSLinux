#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
PWrite(int sfd, const char *const buf0, size_t size)
{
	write_return_t nwrote;
	write_size_t nleft;
	const char *buf = buf0;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE
	nleft = (write_size_t) size;
	forever {
		nwrote = write(sfd, buf, nleft);
		if (nwrote < 0) {
			if (errno != EINTR) {
				nwrote = (write_return_t) size - (write_return_t) nleft;
				if (nwrote == 0)
					nwrote = (write_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nwrote = 0;
				/* Try again. */
			}
		}
		nleft -= (write_size_t) nwrote;
		if (nleft == 0)
			break;
		buf += nwrote;
	}
	nwrote = (write_return_t) size - (write_return_t) nleft;

done:
	RESTORE_SIGPIPE
	return ((int) nwrote);
}	/* PWrite */
