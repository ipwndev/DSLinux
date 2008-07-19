#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
UBind(int sockfd, const char *const astr, const int nTries, const int reuseFlag)
{
	unsigned int i;
	int on;
	sockopt_size_t onsize;
	struct sockaddr_un localAddr;
	sockaddr_size_t ualen;
	
	if ((astr == NULL) || (astr[0] == '\0'))  {
		errno = EINVAL;
		return (-1);
	}

	ualen = (sockaddr_size_t) MakeSockAddrUn(&localAddr, astr);
	(void) unlink(localAddr.sun_path);

	if (reuseFlag != kReUseAddrNo) {
		/* This is mostly so you can quit the server and re-run it
		 * again right away.  If you don't do this, the OS may complain
		 * that the address is still in use.
		 */
		on = 1;
		onsize = (sockopt_size_t) sizeof(on);
		(void) setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			&on, onsize);
	}

	for (i=1; ; i++) {
		/* Try binding a few times, in case we get Address in Use
		 * errors.
		 */
		if (bind(sockfd, (struct sockaddr *) &localAddr, ualen) == 0) {
			break;
		}
		if (i == (unsigned int) nTries) {
			return (-1);
		}
		/* Give the OS time to clean up the old socket,
		 * and then try again.
		 */
		sleep(i * 3);
	}

	return (0);
}	/* UBind */




int
UListen(int sfd, int backlog)
{
	return (listen(sfd, (listen_backlog_t) backlog));
}	/* UListen */
