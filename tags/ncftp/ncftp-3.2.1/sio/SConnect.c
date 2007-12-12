#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int _SConnect(const int sfd, const struct sockaddr_in *const addr, const size_t saddrsiz, const int tlen);

int
SConnect(int sfd, const struct sockaddr_in *const addr, int tlen)
{
	int result;
	
	result = _SConnect(sfd, addr, (size_t) sizeof(struct sockaddr_in), tlen);
	return (result);
}	/* SConnect */



#ifdef FIONBIO
/*
 * All this crud just to get various compilers to shut up about non-problems.
 * Just ioctl() already!
 */
static int
SSetFIONBIO(
	int sfd,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	unsigned long onoff
#else
	int onoff
#endif
)
{
	int rc;
#if (defined(HPUX))
#	undef FIONBIO 
#	define FIONBIO 0x8004667e
#endif
#if ((defined(TRU64UNIX)) || (defined(DIGITAL_UNIX)) || (defined(HPUX)))
	unsigned int ui_fionbio = FIONBIO;
#endif

#if ((defined(TRU64UNIX)) || (defined(DIGITAL_UNIX)) || (defined(HPUX)))
	rc = ioctlsocket(sfd, ui_fionbio, &onoff);
#else
	rc = ioctlsocket(sfd, FIONBIO, &onoff);
#endif

	return (rc);
}	/* SSetFIONBIO */
#endif	/* FIONBIO */




int
_SConnect(const int sfd, const struct sockaddr_in *const addr, const size_t saddrsiz, const int tlen)
{
	fd_set ss, xx;
	struct timeval tv;
	int result;
	int cErrno;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	int wsaErrno;
	int soerr;
	sockopt_size_t soerrsize;
#else
#ifndef FIONBIO
	int fcntl_opt;
#endif
	int optval;
	sockopt_size_t optlen;
#endif
	DECL_SIGPIPE_VARS
	
	if (addr == NULL) {
		errno = EINVAL;
		return (-1);
	}
	
	errno = 0;
	if (tlen <= 0) {
		do {
			IGNORE_SIGPIPE
			result = connect(sfd, (const struct sockaddr *) addr,
				(sockaddr_size_t) saddrsiz);
			SIOSETERRNO
			RESTORE_SIGPIPE
		} while ((result < 0) && (errno == EINTR));
		return (result);
	}

#ifdef FIONBIO
	if (SSetFIONBIO(sfd, 1) < 0) {
		SIOSETERRNO
		return (-1);
	}
#else
	if ((fcntl_opt = fcntl(sfd, F_GETFL, 0)) < 0) {
		SIOSETERRNO
		return (-1);
	} else if (fcntl(sfd, F_SETFL, fcntl_opt | O_NONBLOCK) < 0) {
		SIOSETERRNO
		return (-1);
	}
#endif

	errno = 0;
	IGNORE_SIGPIPE
	result = connect(sfd, (const struct sockaddr *) addr,
			(sockaddr_size_t) saddrsiz);
	RESTORE_SIGPIPE
	if (result == 0) 
		goto connected;	/* Already?!? */

	if ((result < 0) 
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		&& ((wsaErrno = WSAGetLastError()) != WSAEWOULDBLOCK)
		&& (wsaErrno != WSAEINPROGRESS)
#else
		&& (errno != EWOULDBLOCK) && (errno != EINPROGRESS)
#endif
		) {
		cErrno = errno;
		SIOSETERRNO
		shutdown(sfd, 2);
		errno = cErrno;
		return (-1);
	}
	cErrno = errno;

	forever {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		WSASetLastError(0);
#endif
		MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#	pragma message save
#	pragma message disable trunclongint
#endif
		MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#	pragma message restore
#endif
		xx = ss;
		tv.tv_sec = (tv_sec_t) tlen;
		tv.tv_usec = 0;
		result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &xx, SELECT_TYPE_ARG5 &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */		
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			/* Don't bother turning off FIONBIO */
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			/* Don't bother turning off FIONBIO */
			SIOSETERRNO
			return (-1);
		}
	}

	/* Supposedly once the select() returns with a writable
	 * descriptor, it is connected and we don't need to
	 * recall connect().  When select() returns an exception,
	 * the connection failed -- we can get the connect error
	 * doing a write on the socket which will err out.
	 */

#if defined(__DECC) || defined(__DECCXX)
#	pragma message save
#	pragma message disable trunclongint
#endif
	if (MY_FD_ISSET(sfd, &xx)) {
#if defined(__DECC) || defined(__DECCXX)
#	pragma message restore
#endif
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		errno = 0;
		soerr = 0;
		soerrsize = (sockopt_size_t) sizeof(soerr);
		result = getsockopt(sfd, SOL_SOCKET, SO_ERROR, (char *) &soerr, &soerrsize);
		if ((result >= 0) && (soerr != 0)) {
			errno = soerr;
		} else {
			errno = 0;
			(void) send(sfd, "\0", 1, 0);
			SIOSETERRNO
		}
#else
		errno = 0;
		(void) send(sfd, "\0", 1, 0);
#endif
		result = errno;
		shutdown(sfd, 2);
		errno = result;
		return (-1);
	}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	if (cErrno == EINPROGRESS) {
		/*
		 * [from Linux connect(2) page]
		 *
		 * EINPROGRESS
		 *
		 * The socket is non-blocking and the connection can­
		 * not  be  completed immediately.  It is possible to
		 * select(2) or poll(2) for completion  by  selecting
		 * the  socket  for  writing.  After select indicates
		 * writability,  use  getsockopt(2)   to   read   the
		 * SO_ERROR  option  at level SOL_SOCKET to determine
		 * whether connect completed  successfully  (SO_ERROR
		 * is zero) or unsuccessfully (SO_ERROR is one of the
		 * usual error codes  listed  above,  explaining  the
		 * reason for the failure).
	         */
		optval = 0;
		optlen = (sockopt_size_t) sizeof(optval);
		if (getsockopt(sfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0) {
			errno = optval;
			if (errno != 0)
				return (-1);
		}
	}
#endif


connected:

#ifdef FIONBIO
	if (SSetFIONBIO(sfd, 0) < 0) {
		SIOSETERRNO
		shutdown(sfd, 2);
		return (-1);
	}
#else
	if (fcntl(sfd, F_SETFL, fcntl_opt) < 0) {
		SIOSETERRNO
		shutdown(sfd, 2);
		return (-1);
	}
#endif

	return (0);
}	/* _SConnect */
