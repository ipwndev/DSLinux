#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Read up to "size" bytes on sfd.
 *
 * If "retry" is on, after a successful read of less than "size"
 * bytes, it will attempt to read more, upto "size."
 *
 * Although "retry" would seem to indicate you may want to always
 * read "size" bytes or else it is an error, even with that on you
 * may get back a value < size.  Set "retry" to 0 when you want to
 * return as soon as there is a chunk of data whose size is <= "size".
 */

int
PRead(int sfd, char *const buf0, size_t size, int retry)
{
	read_return_t nread;
	read_size_t nleft;
	char *buf = buf0;
	DECL_SIGPIPE_VARS
	
	if ((buf == NULL) || (size == 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	IGNORE_SIGPIPE
	errno = 0;
	nleft = (read_size_t) size;
	forever {
		nread = read(sfd, buf, nleft);
		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
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
		if ((nleft == 0) || (retry == 0))
			break;
		buf += nread;
	}
	nread = (read_return_t) size - (read_return_t) nleft;

done:
	RESTORE_SIGPIPE
	return ((int) nread);
}	/* PRead */
