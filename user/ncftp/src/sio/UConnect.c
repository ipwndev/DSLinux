#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

extern int _SConnect(const int sfd, const struct sockaddr_in *const addr, const size_t saddrsiz, const int tlen);

int
UConnect(int sfd, const struct sockaddr_un *const addr, int ualen, int tlen)
{
	int result;
	
	if ((addr == NULL) || (ualen == 0)) {
		errno = EINVAL;
		return (-1);
	}
	
	result = _SConnect(sfd, (const struct sockaddr_in *) addr, (size_t) ualen, tlen);
	return (result);
}	/* UConnect */
