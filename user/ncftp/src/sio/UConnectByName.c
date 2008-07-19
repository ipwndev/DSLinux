#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
UConnectByName(int sfd, const char * const addrStr, const int tlen)
{
	int result;
	struct sockaddr_un remoteAddr;
	int ualen;
	
	if ((addrStr == NULL) || (addrStr[0] == '\0')) {
		errno = EINVAL;
		return (-1);
	}

	ualen = MakeSockAddrUn(&remoteAddr, addrStr);
	result = UConnect(sfd, &remoteAddr, ualen, tlen);
	return (result);
}	/* UConnectByName */
