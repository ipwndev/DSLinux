#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
USendtoByName(int sfd, const char *const buf, size_t size, int fl, const char *const toAddrStr, int tlen)
{
	struct sockaddr_un toAddr;
	int ualen;
	int result;
	
	if ((toAddrStr == NULL) || (toAddrStr[0] == '\0') || (size == 0) || (buf == NULL)) {
		errno = EINVAL;
		return (-1);
	}

	ualen = (int) MakeSockAddrUn(&toAddr, toAddrStr);
	result = USendto(sfd, buf, size, fl, &toAddr, ualen, tlen);
	return (result);
}	/* USendtoByName */
