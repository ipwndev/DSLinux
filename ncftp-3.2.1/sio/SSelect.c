#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
SelectSetInit(SelectSetPtr const ssp, const double timeout)
{
	double i;
	tv_sec_t l;

	/* Inititalize SelectSet, which will clear the fd_set, the
	 * timeval, and the maxfd and numfds to 0.
	 */
	memset(ssp, 0, sizeof(SelectSet));
	l = (tv_sec_t) timeout;
	i = (double) l;
	ssp->timeout.tv_sec = l;
	ssp->timeout.tv_usec = (tv_usec_t) ((timeout - i) * 1000000.0);
}	/* SelectSetInit */




void
SelectSetAdd(SelectSetPtr const ssp, const int fd)
{
	if (fd >= 0) {
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
		MY_FD_SET(fd, &ssp->fds);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
		if (ssp->maxfd < (fd + 1))
			ssp->maxfd = (fd + 1);
		++ssp->numfds;
	}
}	/* SelectSetAdd */




void
SelectSetRemove(SelectSetPtr const ssp, const int fd)
{
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
	if ((fd >= 0) && (MY_FD_ISSET(fd, &ssp->fds))) {
		MY_FD_CLR(fd, &ssp->fds);
		/* Note that maxfd is left alone, even if maxfd was
		 * this one.  That is okay.
		 */
		--ssp->numfds;
	}
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
}	/* SelectSetRemove */



int
SelectW(SelectSetPtr ssp, SelectSetPtr resultssp)
{
	int rc;

	do {
		memcpy(resultssp, ssp, sizeof(SelectSet));
		rc = select(resultssp->maxfd, NULL, SELECT_TYPE_ARG234 &resultssp->fds, NULL, SELECT_TYPE_ARG5 &resultssp->timeout);
	} while ((rc < 0) && (errno == EINTR));
	return (rc);
}	/* SelectW */



int
SelectR(SelectSetPtr ssp, SelectSetPtr resultssp)
{
	int rc;

	do {
		memcpy(resultssp, ssp, sizeof(SelectSet));
		rc = select(resultssp->maxfd, SELECT_TYPE_ARG234 &resultssp->fds, NULL, NULL, SELECT_TYPE_ARG5 &resultssp->timeout);
	} while ((rc < 0) && (errno == EINTR));
	return (rc);
}	/* SelectR */
