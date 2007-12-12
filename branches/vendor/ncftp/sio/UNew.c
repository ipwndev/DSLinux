#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
MakeSockAddrUn(struct sockaddr_un *uaddr, const char *const sockfile)
{
	int ualen;
	
	if ((uaddr == NULL) || (sockfile == NULL)) {
		errno = EINVAL;
		return (-1);
	}

	memset(uaddr, 0, sizeof(struct sockaddr_un));
	uaddr->sun_family = AF_UNIX;
	strncpy(uaddr->sun_path, sockfile, sizeof(uaddr->sun_path) - 1);
#ifdef HAVE_SOCKADDR_UN_SUN_LEN
        /* 4.3bsd-reno */
	ualen = (int) sizeof(uaddr->sun_len) + (int) sizeof(uaddr->sun_family) + (int) strlen(uaddr->sun_path) + 1;
	uaddr->sun_len = ualen;
#else
	ualen = (int) sizeof(uaddr->sun_family) + (int) strlen(uaddr->sun_path) + 1;
#endif
	return (ualen);
}	/* MakeSockAddrUn */




int
UNewStreamClient(void)
{
	int sfd;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd < 0)
		return kUNewFailed;

	return (sfd);
}	/* UNewStreamClient */




int
UNewDatagramClient(void)
{
	int sfd;

	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd < 0)
		return kUNewFailed;

	return (sfd);
}	/* UNewDatagramClient */




int
UNewStreamServer(const char *const astr, const int nTries, const int reuseFlag, int listenQueueSize)
{
	int oerrno;
	int sfd;
	
	if ((astr == NULL) || (astr[0] == '\0')) {
		errno = EINVAL;
		return (-1);
	}

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd < 0)
		return kUNewFailed;

	if (UBind(sfd, astr, nTries, reuseFlag) < 0) {
		oerrno = errno;
		(void) close(sfd);
		errno = oerrno;
		return kUBindFailed;
	}

	if (UListen(sfd, listenQueueSize) < 0) {
		oerrno = errno;
		(void) close(sfd);
		errno = oerrno;
		return kUListenFailed;
	}

	return (sfd);
}	/* UNewStreamServer */




int
UNewDatagramServer(const char *const astr, const int nTries, const int reuseFlag)
{
	int oerrno;
	int sfd;
	
	if ((astr == NULL) || (astr[0] == '\0')) {
		errno = EINVAL;
		return (-1);
	}

	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd < 0)
		return kUNewFailed;

	if (UBind(sfd, astr, nTries, reuseFlag) < 0) {
		oerrno = errno;
		(void) close(sfd);
		errno = oerrno;
		return kUBindFailed;
	}

	return (sfd);
}	/* UNewDatagramServer */
