#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef SO_RCVBUF
int
GetSocketBufSize(const int UNUSED(sockfd), size_t *const rsize, size_t *const ssize)
{
	LIBSIO_USE_VAR(sockfd);
	if (ssize != NULL)
		*ssize = 0;
	if (rsize != NULL)
		*rsize = 0;
	return (-1);
}	/* GetSocketBufSize */
#else
int
GetSocketBufSize(const int sockfd, size_t *const rsize, size_t *const ssize)
{
	int rc = -1;
	int opt;
	sockopt_size_t optsize;

	if (ssize != NULL) {
		opt = 0;
		optsize = (sockopt_size_t) sizeof(opt);
		rc = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, GETSOCKOPT_ARG4 &opt, &optsize);
		if (rc == 0)
			*ssize = (size_t) opt;
		else
			*ssize = 0;
	}
	if (rsize != NULL) {
		opt = 0;
		optsize = (sockopt_size_t) sizeof(opt);
		rc = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, GETSOCKOPT_ARG4 &opt, &optsize);
		if (rc == 0)
			*rsize = (size_t) opt;
		else
			*rsize = 0;
	}
	return (rc);
}	/* GetSocketBufSize */
#endif




#ifndef SO_RCVBUF
int
SetSocketBufSize(const int UNUSED(sockfd), const size_t UNUSED(rsize), const size_t UNUSED(ssize))
{
	LIBSIO_USE_VAR(sockfd);
	LIBSIO_USE_VAR(rsize);
	LIBSIO_USE_VAR(ssize);
	return (-1);
}	/* SetSocketBufSize */
#else
int
SetSocketBufSize(const int sockfd, const size_t rsize, const size_t ssize)
{
	int rc = -1;
	int opt;
	sockopt_size_t optsize;

#ifdef TCP_RFC1323
	/* This is an AIX-specific socket option to do RFC1323 large windows */
	if (ssize != 0 || rsize != 0) {
		opt = 1;
		optsize = (sockopt_size_t) sizeof(opt);
		rc = setsockopt(sockfd, IPPROTO_TCP, TCP_RFC1323, &opt, optsize);
	}
#endif

	if (ssize != 0) {
		opt = (int) ssize;
		optsize = (sockopt_size_t) sizeof(opt);
		rc = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, GETSOCKOPT_ARG4 &opt, optsize);
		if (rc < 0)
			return (rc);
	}
	if (rsize != 0) {
		opt = (int) rsize;
		optsize = (sockopt_size_t) sizeof(opt);
		rc = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, GETSOCKOPT_ARG4 &opt, optsize);
		if (rc < 0)
			return (rc);
	}
	return (0);
}	/* SetSocketBufSize */
#endif




#ifndef SO_KEEPALIVE
int
GetSocketKeepAlive(const int UNUSED(fd))
{
	LIBSIO_USE_VAR(fd);
	return (-1);
}	/* GetSocketKeepAlive */
#else
int
GetSocketKeepAlive(const int fd)
{
	sockopt_size_t optsize;
	int opt;

	opt = -2;
	optsize = (sockopt_size_t) sizeof(opt);
	if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &opt, &optsize) < 0)
		return (-1);
	return (opt);
}	/* GetSocketKeepAlive */
#endif	/* SO_KEEPALIVE */





#ifndef SO_KEEPALIVE
int
SetSocketKeepAlive(const int UNUSED(fd), const int UNUSED(onoff))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(onoff);
	return (-1);
}	/* SetSocketKeepAlive */
#else
int
SetSocketKeepAlive(const int fd, const int onoff)
{
	int opt;

	opt = onoff;
	return (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &opt, (sockopt_size_t) sizeof(opt)));
}	/* SetSocketKeepAlive */
#endif	/* SO_KEEPALIVE */




#ifndef TCP_NODELAY
int
GetSocketNagleAlgorithm(const int UNUSED(fd))
{
	LIBSIO_USE_VAR(fd);
	return (-1);
}	/* GetSocketNagleAlgorithm */
#else
int
GetSocketNagleAlgorithm(const int fd)
{
	sockopt_size_t optsize;
	int opt;

	opt = -2;
	optsize = (sockopt_size_t) sizeof(opt);
	if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &opt, &optsize) < 0)
		return (-1);
	return (opt);
}	/* GetSocketNagleAlgorithm */
#endif	/* TCP_NODELAY */





#ifndef TCP_NODELAY
int
SetSocketNagleAlgorithm(const int UNUSED(fd), const int UNUSED(onoff))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(onoff);
	return (-1);
}	/* SetSocketNagleAlgorithm */
#else
int
SetSocketNagleAlgorithm(const int fd, const int onoff)
{
	int opt;

	opt = onoff;
	return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &opt, (sockopt_size_t) sizeof(opt)));
}	/* SetSocketNagleAlgorithm */
#endif	/* TCP_NODELAY */




#ifndef SO_OOBINLINE
int
GetSocketInlineOutOfBandData(const int UNUSED(fd))
{
	LIBSIO_USE_VAR(fd);
	return (-1);
}	/* GetSocketInlineOutOfBandData */
#else
int
GetSocketInlineOutOfBandData(const int fd)
{
	sockopt_size_t optsize;
	int opt;

	opt = -2;
	optsize = (sockopt_size_t) sizeof(opt);
	if (getsockopt(fd, SOL_SOCKET, SO_OOBINLINE, (char *) &opt, &optsize) < 0)
		return (-1);
	return (opt);
}	/* GetSocketInlineOutOfBandData */
#endif	/* SO_OOBINLINE */





#ifndef SO_OOBINLINE
int
SetSocketInlineOutOfBandData(const int UNUSED(fd), const int UNUSED(onoff))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(onoff);
	return (-1);
}	/* SetSocketInlineOutOfBandData */
#else
int
SetSocketInlineOutOfBandData(const int fd, const int onoff)
{
	int opt;

	opt = onoff;
	return (setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, (char *) &opt, (sockopt_size_t) sizeof(opt)));
}	/* SetSocketInlineOutOfBandData */
#endif	/* SO_OOBINLINE */




#ifndef IP_TOS
int
GetSocketTypeOfService(const int UNUSED(fd))
{
	LIBSIO_USE_VAR(fd);
	return (-1);
}	/* GetSocketTypeOfService */
#else
int
GetSocketTypeOfService(const int fd)
{
	sockopt_size_t optsize;
	int opt;

	opt = -2;
	optsize = (sockopt_size_t) sizeof(opt);
	if (getsockopt(fd, IPPROTO_IP, IP_TOS, (char *) &opt, &optsize) < 0)
		return (-1);
	return (opt);
}	/* GetSocketTypeOfService */
#endif	/* IP_TOS */





#ifndef IP_TOS
int
SetSocketTypeOfService(const int UNUSED(fd), const int UNUSED(tosType))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(tosType);
	return (-1);
}	/* SetSocketTypeOfService */
#else
int
SetSocketTypeOfService(const int fd, const int tosType)
{
	int opt;

	opt = tosType;
	return (setsockopt(fd, IPPROTO_IP, IP_TOS, (char *) &opt, (sockopt_size_t) sizeof(opt)));
}	/* SetSocketTypeOfService */
#endif	/* IP_TOS */




#ifndef SO_LINGER
int
GetSocketLinger(const int UNUSED(fd), int *const UNUSED(lingertime))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(lingertime);
	return (-1);
}	/* GetSocketLinger */
#else
int
GetSocketLinger(const int fd, int *const lingertime)
{
	sockopt_size_t optsize;
	struct linger opt;

	optsize = (sockopt_size_t) sizeof(opt);
	opt.l_onoff = 0;
	opt.l_linger = 0;
	if (getsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &opt, &optsize) < 0)
		return (-1);
	if (lingertime != NULL)
		*lingertime = opt.l_linger;
	return (opt.l_onoff);
}	/* GetSocketLinger */
#endif	/* SO_LINGER */



#ifndef SO_LINGER
int
SetSocketLinger(const int UNUSED(fd), const int UNUSED(l_onoff), const int UNUSED(l_linger))
{
	LIBSIO_USE_VAR(fd);
	LIBSIO_USE_VAR(l_onoff);
	LIBSIO_USE_VAR(l_linger);
	return (-1);
}	/* SetSocketLinger */
#else
int
SetSocketLinger(const int fd, const int l_onoff, const int l_linger)
{
	struct linger opt;
	sockopt_size_t optsize;
/*
 * From hpux:
 *
 * Structure used for manipulating linger option.
 *
 * if l_onoff == 0:
 *    close(2) returns immediately; any buffered data is sent later
 *    (default)
 * 
 * if l_onoff != 0:
 *    if l_linger == 0, close(2) returns after discarding any unsent data
 *    if l_linger != 0, close(2) does not return until buffered data is sent
 */
#if 0
struct	linger {
	int	l_onoff;		/* 0 = do not wait to send data */
					/* non-0 = see l_linger         */
	int	l_linger;		/* 0 = discard unsent data      */
					/* non-0 = wait to send data    */
};
#endif
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	opt.l_onoff = (unsigned short) l_onoff;
	opt.l_linger = (unsigned short)  l_linger;
#else
	opt.l_onoff = l_onoff;
	opt.l_linger = l_linger;
#endif
	optsize = (sockopt_size_t) sizeof(opt);
	return (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &opt, optsize));
}	/* SetSocketLinger */
#endif	/* SO_LINGER */
