/* ftp.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

const char gLibNcFTPVersion[] = kLibraryVersion;

#ifdef NO_SIGNALS
static const char gNoSignalsMarker[] = "@(#) LibNcFTP - NO_SIGNALS";
#else

static int gGotSig = 0;
#ifdef HAVE_SIGSETJMP
static sigjmp_buf gCancelConnectJmp;
#else
static jmp_buf gCancelConnectJmp;
#endif	/* HAVE_SIGSETJMP */

#endif	/* NO_SIGNALS */


#ifndef lint
static const char gCopyright[] = "@(#) LibNcFTP Copyright 1995-2001, by Mike Gleason.  All rights reserved.";
#endif

#ifdef HAVE_LIBSOCKS5
#	define SOCKS 5
#	include <socks.h>
#else
#	ifdef HAVE_LIBSOCKS
#		define accept		Raccept
#		define connect		Rconnect
#		define getsockname	Rgetsockname
#		define listen		Rlisten
#	endif
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define DisposeSocket(a) shutdown(a, 2); closesocket(a)
#else
#	define DisposeSocket(a) close(a)
#endif

static const char *gPrivateNetworks[] = {
	"192.168.",
	"10.",
	"172.16.",
	"172.17.",
	"172.18.",
	"172.19.",
	"172.20.",
	"172.21.",
	"172.22.",
	"172.23.",
	"172.24.",
	"172.25.",
	"172.26.",
	"172.27.",
	"172.28.",
	"172.29.",
	"172.30.",
	"172.31.",
	NULL
};




void
FTPCloseControlConnection(const FTPCIPtr cip)
{
	/* This will close each file, if it was open. */
#ifdef NO_SIGNALS
	SClose(cip->ctrlSocketR, 3);
	cip->ctrlSocketR = kClosedFileDescriptor;
	cip->ctrlSocketW = kClosedFileDescriptor;
	DisposeSReadlineInfo(&cip->ctrlSrl);
#else	/* NO_SIGNALS */
	if (cip->ctrlTimeout > 0)
		(void) alarm(cip->ctrlTimeout);
	CloseFile(&cip->cin);
	CloseFile(&cip->cout);
	cip->ctrlSocketR = kClosedFileDescriptor;
	cip->ctrlSocketW = kClosedFileDescriptor;
	if (cip->ctrlTimeout > 0)
		(void) alarm(0);
#endif	/* NO_SIGNALS */
	cip->connected = 0;
	cip->loggedIn = 0;
}	/* FTPCloseControlConnection */



static int
GetSocketAddress(const FTPCIPtr cip, int sockfd, struct sockaddr_in *saddr)
{
	sockaddr_size_t len = (sockaddr_size_t) sizeof (struct sockaddr_in);
	int result = 0;

	if (getsockname(sockfd, (struct sockaddr *)saddr, &len) < 0) {
		FTPLogError(cip, kDoPerror, "Could not get socket name.\n");
		cip->errNo = kErrGetSockName;
		result = kErrGetSockName;
	}
	return (result);
}	/* GetSocketAddress */




#ifndef NO_SIGNALS

static void
CancelConnect(int signum)
{
	gGotSig = signum;
#ifdef HAVE_SIGSETJMP
	siglongjmp(gCancelConnectJmp, 1);
#else
	longjmp(gCancelConnectJmp, 1);
#endif	/* HAVE_SIGSETJMP */
}	/* CancelConnect */

#endif	/* NO_SIGNALS */



int
OpenControlConnection(const FTPCIPtr cip, char *host, unsigned int port)
{
	struct in_addr ip_address;
	int err = 0;
	int result;
	int oerrno;
	volatile int sockfd = -1;
	volatile int sock2fd = -1;
	ResponsePtr rp = NULL;
	char **volatile curaddr;
	int hpok;
	struct hostent hp;
	char *volatile fhost;
	unsigned int fport;
#ifndef NO_SIGNALS
	volatile FTPSigProc osigint;
	volatile FTPSigProc osigalrm;
	volatile FTPCIPtr vcip;
	int sj;
#endif	/* NO_SIGNALS */
	const char *firstLine, *secondLine, *srvr;

	LIBNCFTP_USE_VAR(gLibNcFTPVersion);
	LIBNCFTP_USE_VAR(gCopyright);
#ifdef NO_SIGNALS
	LIBNCFTP_USE_VAR(gNoSignalsMarker);
#endif	/* NO_SIGNALS */

	if (cip->firewallType == kFirewallNotInUse) {
		fhost = host;
		fport = port;
	} else {
		fhost = cip->firewallHost;
		fport = cip->firewallPort;
	}
	if (fport == 0)
		fport = cip->lip->defaultPort;

	/* Since we're the client, we just have to get a socket() and
	 * connect() it.
	 */
	(void) ZERO(cip->servCtlAddr);
	cip->cin = NULL;
	cip->cout = NULL;

	/* Make sure we use network byte-order. */
	fport = (unsigned int) htons((unsigned short) fport);

	cip->servCtlAddr.sin_port = (unsigned short) fport;

	if (GetHostEntry(&hp, fhost, &ip_address, cip->buf, cip->bufSize) != 0) {
		hpok = 0;
		/* Okay, no Host entry, but maybe we have a numeric address
		 * in ip_address we can try.
		 */
		if (ip_address.s_addr == INADDR_NONE) {
			FTPLogError(cip, kDontPerror, "%s: unknown host.\n", fhost);
			cip->errNo = kErrHostUnknown;
			return (kErrHostUnknown);
		}
		cip->servCtlAddr.sin_family = AF_INET;
		cip->servCtlAddr.sin_addr.s_addr = ip_address.s_addr;
	} else {
		hpok = 1;
		cip->servCtlAddr.sin_family = hp.h_addrtype;
		/* We'll fill in the rest of the structure below. */
	}
	
	/* After obtaining a socket, try to connect it to a remote
	 * address.  If we didn't get a host entry, we will only have
	 * one thing to try (ip_address);  if we do have one, we can try
	 * every address in the list from the host entry.
	 */

	if (hpok == 0) {
		/* Since we're given a single raw address, and not a host entry,
		 * we can only try this one address and not any other addresses
		 * that could be present for a site with a host entry.
		 */

		if ((sockfd = socket(cip->servCtlAddr.sin_family, SOCK_STREAM, 0)) < 0) {
			FTPLogError(cip, kDoPerror, "Could not get a socket.\n");
			cip->errNo = kErrNewStreamSocket;
			return (kErrNewStreamSocket);
		}

		/* This doesn't do anything if you left these
		 * at their defaults (zero).  Otherwise it
		 * tries to set the buffer size to the
		 * size specified.
		 */
		(void) SetSocketBufSize(sockfd, cip->ctrlSocketRBufSize, cip->ctrlSocketSBufSize);

#ifdef NO_SIGNALS
		err = SConnect(sockfd, &cip->servCtlAddr, (int) cip->connTimeout);

		if (err < 0) {
			oerrno = errno;
			(void) SClose(sockfd, 3);
			errno = oerrno;
			sockfd = -1;
		}
#else	/* NO_SIGNALS */
		osigint = (volatile FTPSigProc) signal(SIGINT, CancelConnect);
		if (cip->connTimeout > 0) {
			osigalrm = (volatile FTPSigProc) signal(SIGALRM, CancelConnect);
			(void) alarm(cip->connTimeout);
		}

		vcip = cip;

#ifdef HAVE_SIGSETJMP
		sj = sigsetjmp(gCancelConnectJmp, 1);
#else
		sj = setjmp(gCancelConnectJmp);
#endif	/* HAVE_SIGSETJMP */

		if (sj != 0) {
			/* Interrupted by a signal. */
			(void) DisposeSocket(sockfd);
			(void) signal(SIGINT, (FTPSigProc) osigint);
			if (vcip->connTimeout > 0) {
				(void) alarm(0);
				(void) signal(SIGALRM, (FTPSigProc) osigalrm);
			}
			if (gGotSig == SIGINT) {
				result = vcip->errNo = kErrConnectMiscErr;
				Error(vcip, kDontPerror, "Connection attempt canceled.\n");
				(void) kill(getpid(), SIGINT);
			} else if (gGotSig == SIGALRM) {
				result = vcip->errNo = kErrConnectRetryableErr;
				Error(vcip, kDontPerror, "Connection attempt timed-out.\n");
				(void) kill(getpid(), SIGALRM);
			} else {
				result = vcip->errNo = kErrConnectMiscErr;
				Error(vcip, kDontPerror, "Connection attempt failed due to an unexpected signal (%d).\n", gGotSig);
			}
			return (result);
		} else  {
			err = connect(sockfd, (struct sockaddr *) &cip->servCtlAddr,
				      (int) sizeof (cip->servCtlAddr));
			if (cip->connTimeout > 0) {
				(void) alarm(0);
				(void) signal(SIGALRM, (FTPSigProc) osigalrm);
			}
			(void) signal(SIGINT, (FTPSigProc) osigint);
		}

		if (err < 0) {
			oerrno = errno;
			(void) DisposeSocket(sockfd);
			errno = oerrno;
			sockfd = -1;
		}
#endif	/* NO_SIGNALS */
	} else {
		/* We can try each address in the list.  We'll quit when we
		 * run out of addresses to try or get a successful connection.
		 */
		for (curaddr = hp.h_addr_list; *curaddr != NULL; curaddr++) {
			if ((sockfd = socket(cip->servCtlAddr.sin_family, SOCK_STREAM, 0)) < 0) {
				FTPLogError(cip, kDoPerror, "Could not get a socket.\n");
				cip->errNo = kErrNewStreamSocket;
				return (kErrNewStreamSocket);
			}
			/* This could overwrite the address field in the structure,
			 * but this is okay because the structure has a junk field
			 * just for this purpose.
			 */
			(void) memcpy(&cip->servCtlAddr.sin_addr, *curaddr, (size_t) hp.h_length);

			/* This doesn't do anything if you left these
			 * at their defaults (zero).  Otherwise it
			 * tries to set the buffer size to the
			 * size specified.
			 */
			(void) SetSocketBufSize(sockfd, cip->ctrlSocketRBufSize, cip->ctrlSocketSBufSize);

#ifdef NO_SIGNALS
			err = SConnect(sockfd, &cip->servCtlAddr, (int) cip->connTimeout);

			if (err == 0)
				break;
			oerrno = errno;
			(void) SClose(sockfd, 3);
			errno = oerrno;
			sockfd = -1;
#else	/* NO_SIGNALS */

			osigint = (volatile FTPSigProc) signal(SIGINT, CancelConnect);
			if (cip->connTimeout > 0) {
				osigalrm = (volatile FTPSigProc) signal(SIGALRM, CancelConnect);
				(void) alarm(cip->connTimeout);
			}

			vcip = cip;
#ifdef HAVE_SIGSETJMP
			sj = sigsetjmp(gCancelConnectJmp, 1);
#else
			sj = setjmp(gCancelConnectJmp);
#endif	/* HAVE_SIGSETJMP */

			if (sj != 0) {
				/* Interrupted by a signal. */
				(void) DisposeSocket(sockfd);
				(void) signal(SIGINT, (FTPSigProc) osigint);
				if (vcip->connTimeout > 0) {
					(void) alarm(0);
					(void) signal(SIGALRM, (FTPSigProc) osigalrm);
				}
				if (gGotSig == SIGINT) {
					result = vcip->errNo = kErrConnectMiscErr;
					Error(vcip, kDontPerror, "Connection attempt canceled.\n");
					(void) kill(getpid(), SIGINT);
				} else if (gGotSig == SIGALRM) {
					result = vcip->errNo = kErrConnectRetryableErr;
					Error(vcip, kDontPerror, "Connection attempt timed-out.\n");
					(void) kill(getpid(), SIGALRM);
				} else {
					result = vcip->errNo = kErrConnectMiscErr;
					Error(vcip, kDontPerror, "Connection attempt failed due to an unexpected signal (%d).\n", gGotSig);
				}
				return (result);
			} else {
				err = connect(sockfd, (struct sockaddr *) &cip->servCtlAddr,
					      (int) sizeof (cip->servCtlAddr));
				if (cip->connTimeout > 0) {
					(void) alarm(0);
					(void) signal(SIGALRM, (FTPSigProc) osigalrm);
				}
				(void) signal(SIGINT, (FTPSigProc) osigint);
			}

			if (err == 0)
				break;
			oerrno = errno;
			(void) DisposeSocket(sockfd);
			errno = oerrno;
			sockfd = -1;
#endif /* NO_SIGNALS */
		}
	}
	
	if (err < 0) {
		/* Could not connect.  Close up shop and go home. */

		/* If possible, tell the caller if they should bother
		 * calling back later.
		 */
		switch (errno) {
#ifdef ENETDOWN
			case ENETDOWN:
#elif defined(WSAENETDOWN)
			case WSAENETDOWN:
#endif
#ifdef ENETUNREACH
			case ENETUNREACH:
#elif defined(WSAENETUNREACH)
			case WSAENETUNREACH:
#endif
#ifdef ECONNABORTED
			case ECONNABORTED:
#elif defined(WSAECONNABORTED)
			case WSAECONNABORTED:
#endif
#ifdef ETIMEDOUT
			case ETIMEDOUT:
#elif defined(WSAETIMEDOUT)
			case WSAETIMEDOUT:
#endif
#ifdef EHOSTDOWN
			case EHOSTDOWN:
#elif defined(WSAEHOSTDOWN)
			case WSAEHOSTDOWN:
#endif
#ifdef ECONNRESET
			case ECONNRESET:
#elif defined(WSAECONNRESET)
			case WSAECONNRESET:
#endif
				FTPLogError(cip, kDoPerror, "Could not connect to %s -- try again later.\n", fhost);
				result = cip->errNo = kErrConnectRetryableErr;
				break;
#ifdef ECONNREFUSED
			case ECONNREFUSED:
#elif defined(WSAECONNREFUSED)
			case WSAECONNREFUSED:
#endif
				FTPLogError(cip, kDoPerror, "Could not connect to %s.\n", fhost);
				result = cip->errNo = kErrConnectRefused;
				break;
			default:
				FTPLogError(cip, kDoPerror, "Could not connect to %s.\n", fhost);
				result = cip->errNo = kErrConnectMiscErr;
		}
		goto fatal;
	}

	/* Get our end of the socket address for later use. */
	if ((result = GetSocketAddress(cip, sockfd, &cip->ourCtlAddr)) < 0)
		goto fatal;

	/* We want Out-of-band data to appear in the regular stream,
	 * since we can handle TELNET.
	 */
	(void) SetSocketInlineOutOfBandData(sockfd, 1);
	(void) SetSocketKeepAlive(sockfd, 1);
	(void) SetSocketLinger(sockfd, 0, 0);	/* Don't need it for ctrl. */

	/* Control connection is somewhat interactive, so quick response
	 * is desired.
	 */
	(void) SetSocketTypeOfService(sockfd, IPTOS_LOWDELAY);

#ifdef NO_SIGNALS
	cip->ctrlSocketR = sockfd;
	cip->ctrlSocketW = sockfd;
	cip->cout = NULL;
	cip->cin = NULL;
	sock2fd = kClosedFileDescriptor;

	if (InitSReadlineInfo(&cip->ctrlSrl, sockfd, cip->srlBuf, sizeof(cip->srlBuf), (int) cip->ctrlTimeout, 1) < 0) {
		result = kErrFdopenW;
		cip->errNo = kErrFdopenW;
		FTPLogError(cip, kDoPerror, "Could not fdopen.\n");
		goto fatal;
	}
#else	/* NO_SIGNALS */
	if ((sock2fd = dup(sockfd)) < 0) {
		result = kErrDupSocket;
		cip->errNo = kErrDupSocket;
		FTPLogError(cip, kDoPerror, "Could not duplicate a file descriptor.\n");
		goto fatal;
	}

	/* Now setup the FILE pointers for use with the Std I/O library
	 * routines.
	 */
	if ((cip->cin = fdopen(sockfd, "r")) == NULL) {
		result = kErrFdopenR;
		cip->errNo = kErrFdopenR;
		FTPLogError(cip, kDoPerror, "Could not fdopen.\n");
		goto fatal;
	}

	if ((cip->cout = fdopen(sock2fd, "w")) == NULL) {
		result = kErrFdopenW;
		cip->errNo = kErrFdopenW;
		FTPLogError(cip, kDoPerror, "Could not fdopen.\n");
		CloseFile(&cip->cin);
		sockfd = kClosedFileDescriptor;
		goto fatal;
	}

	cip->ctrlSocketR = sockfd;
	cip->ctrlSocketW = sockfd;

	/* We'll be reading and writing lines, so use line buffering.  This
	 * is necessary since the stdio library will use full buffering
	 * for all streams not associated with the tty.
	 */
#ifdef HAVE_SETLINEBUF
	setlinebuf(cip->cin);
	setlinebuf(cip->cout);
#else
	(void) SETVBUF(cip->cin, NULL, _IOLBF, (size_t) BUFSIZ);
	(void) SETVBUF(cip->cout, NULL, _IOLBF, (size_t) BUFSIZ);
#endif
#endif	/* NO_SIGNALS */

	InetNtoA(cip->ip, &cip->servCtlAddr.sin_addr, sizeof(cip->ip));
	if ((hpok == 0) || (hp.h_name == NULL))
		(void) STRNCPY(cip->actualHost, fhost);
	else
		(void) STRNCPY(cip->actualHost, (char *) hp.h_name);

	/* Read the startup message from the server. */	
	rp = InitResponse();
	if (rp == NULL) {
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		cip->errNo = kErrMallocFailed;
		result = cip->errNo;
		goto fatal;
	}

	result = GetResponse(cip, rp);
	if ((result < 0) && (rp->msg.first == NULL)) {
		goto fatal;
	}
	if (rp->msg.first != NULL) {
		cip->serverType = kServerTypeUnknown;
		srvr = NULL;
		firstLine = rp->msg.first->line;
		secondLine = NULL;
		if (rp->msg.first->next != NULL)
			secondLine = rp->msg.first->next->line;
		
		if (strstr(firstLine, "Version wu-") != NULL) {
			cip->serverType = kServerTypeWuFTPd;
			srvr = "wu-ftpd";
		} else if (strstr(firstLine, "NcFTPd") != NULL) {
			cip->serverType = kServerTypeNcFTPd;
			srvr = "NcFTPd Server";
		} else if (STRNEQ("ProFTPD", firstLine, 7)) {
			cip->serverType = kServerTypeProFTPD;
			srvr = "ProFTPD";
		} else if (strstr(firstLine, "Microsoft FTP Service") != NULL) {
			cip->serverType = kServerTypeMicrosoftFTP;
			srvr = "Microsoft FTP Service";
		} else if (strstr(firstLine, "(NetWare ") != NULL) {
			cip->serverType = kServerTypeNetWareFTP;
			srvr = "NetWare FTP Service";
		} else if (strstr(firstLine, "(DG/UX ") != NULL) {
			cip->serverType = kServerTypeDguxFTP;
			srvr = "DG/UX FTP Service";
		} else if (strstr(firstLine, "IBM FTP CS ") != NULL) {
			cip->serverType = kServerTypeIBMFTPCS;
			srvr = "IBM FTP CS Server";
		} else if (strstr(firstLine, "DC/OSx") != NULL) {
			cip->serverType = kServerTypePyramid;
			srvr = "Pyramid DC/OSx FTP Service";
		} else if (STRNEQ("WFTPD", firstLine, 5)) {
			cip->serverType = kServerTypeWFTPD;
			srvr = "WFTPD";
		} else if (STRNEQ("Serv-U FTP", firstLine, 10)) {
			cip->serverType = kServerTypeServ_U;
			srvr = "Serv-U FTP-Server";
		} else if (strstr(firstLine, "VFTPD") != NULL) {
			cip->serverType = kServerTypeVFTPD;
			srvr = "VFTPD";
		} else if (STRNEQ("FTP-Max", firstLine, 7)) {
			cip->serverType = kServerTypeFTP_Max;
			srvr = "FTP-Max";
		} else if (strstr(firstLine, "Roxen") != NULL) {
			cip->serverType = kServerTypeRoxen;
			srvr = "Roxen";
		} else if (strstr(firstLine, "WS_FTP") != NULL) {
			cip->serverType = kServerTypeWS_FTP;
			srvr = "WS_FTP Server";
		} else if ((secondLine != NULL) && (strstr(secondLine, "WarFTP") != NULL)) {
			cip->serverType = kServerTypeWarFTPd;
			srvr = "WarFTPd";
		}

		if (srvr != NULL)
			PrintF(cip, "Remote server is running %s.\n", srvr);

		/* Do the application's connect message callback, if present. */
		if ((cip->onConnectMsgProc != 0) && (rp->codeType < 4))
			(*cip->onConnectMsgProc)(cip, rp);
	}

	if (rp->codeType >= 4) {
		/* They probably hung up on us right away.  That's too bad,
		 * but we can tell the caller that they can call back later
		 * and try again.
		 */
		result = cip->errNo = kErrConnectRetryableErr;
		FTPLogError(cip, kDontPerror, "Server hungup immediately after connect.\n");
		goto fatal;
	}
	if (result < 0)		/* Some other error occurred during connect message */
		goto fatal;
	cip->connected = 1;
	DoneWithResponse(cip, rp);
	return (kNoErr);
	
fatal:
	if (rp != NULL)
		DoneWithResponse(cip, rp);
	if (sockfd > 0)
		(void) DisposeSocket(sockfd);
	if (sock2fd > 0)
		(void) DisposeSocket(sock2fd);		
	CloseFile(&cip->cin);
	CloseFile(&cip->cout);
	cip->ctrlSocketR = kClosedFileDescriptor;
	cip->ctrlSocketW = kClosedFileDescriptor;
	return (result);
}	/* OpenControlConnection */




void
CloseDataConnection(const FTPCIPtr cip)
{
	if (cip->dataSocket != kClosedFileDescriptor) {
#ifdef NO_SIGNALS
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		if (cip->dataSocketConnected != 0) {
			/* This could block if we were uploading, but only if
			 * linger mode was set.
			 */
			SClose(cip->dataSocket, cip->xferTimeout);
		} else {
			DisposeSocket(cip->dataSocket);
		}
#else
		/* This could block, but only if
		 * linger mode was set.
		 */
		DisposeSocket(cip->dataSocket);
#endif
#else	/* NO_SIGNALS */
		if (cip->xferTimeout > 0)
			(void) alarm(cip->xferTimeout);
		(void) DisposeSocket(cip->dataSocket);
		if (cip->xferTimeout > 0)
			(void) alarm(0);
#endif	/* NO_SIGNALS */
		cip->dataSocket = kClosedFileDescriptor;
	}
	cip->dataSocketConnected = 0;
	memset(&cip->ourDataAddr, 0, sizeof(cip->ourDataAddr));
	memset(&cip->servDataAddr, 0, sizeof(cip->servDataAddr));
}	/* CloseDataConnection */





int
FTPSetStartOffset(const FTPCIPtr cip, longest_int restartPt)
{
	ResponsePtr rp;
	int result;

	if (restartPt != (longest_int) 0) {
		rp = InitResponse();
		if (rp == NULL) {
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			cip->errNo = kErrMallocFailed;
			return (cip->errNo);
		}

		/* Force reset to offset zero. */
		if (restartPt == (longest_int) -1)
			restartPt = (longest_int) 0;
#ifdef PRINTF_LONG_LONG
		result = RCmd(cip, rp,
		"REST " PRINTF_LONG_LONG,
		restartPt);
#else
		result = RCmd(cip, rp, "REST %ld", (long) restartPt);
#endif

		if (result < 0) {
			DoneWithResponse(cip, rp);
			return (result);
		} else if (result == 3) {
			/* Success */
			cip->hasREST = kCommandAvailable;
		} else if (FTP_UNIMPLEMENTED_CMD(rp->code)) {
			cip->hasREST = kCommandNotAvailable;
			DoneWithResponse(cip, rp);
			cip->errNo = kErrSetStartPoint;
			return (kErrSetStartPoint);
		} else {
			DoneWithResponse(cip, rp);
			cip->errNo = kErrSetStartPoint;
			return (kErrSetStartPoint);
		}
		DoneWithResponse(cip, rp);
	}
	return (0);
}	/* FTPSetStartOffset */



int
FTPSendPort(const FTPCIPtr cip, struct sockaddr_in *saddr)
{
	char *a, *p;
	int result;
	ResponsePtr rp;

	rp = InitResponse();
	if (rp == NULL) {
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		cip->errNo = kErrMallocFailed;
		return (cip->errNo);
	}

	/* These will point to data in network byte order. */
	a = (char *) &saddr->sin_addr;
	p = (char *) &saddr->sin_port;
#define UC(x) (int) (((int) x) & 0xff)

	/* Need to tell the other side which host (the address) and
	 * which process (port) on that host to send data to.
	 */
	result = RCmd(cip, rp, "PORT %d,%d,%d,%d,%d,%d",
		UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));

	DoneWithResponse(cip, rp);
	if (result < 0) {
		return (result);
	} else if (result != 2) {
		/* A 500'ish response code means the PORT command failed. */
		cip->errNo = kErrPORTFailed;
		return (cip->errNo);
	}
	return (kNoErr);
}	/* FTPSendPort */




int
FTPSendPassive(const FTPCIPtr cip, struct sockaddr_in *saddr, int *weird)
{
	ResponsePtr rp;
	int i[6], j;
	unsigned char n[6];
	char *cp;
	int result;

	rp = InitResponse();
	if (rp == NULL) {
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		cip->errNo = kErrMallocFailed;
		return (cip->errNo);
	}

	result = RCmd(cip, rp, "PASV");
	if (result < 0)
		goto done;

	if (rp->codeType != 2) {
		/* Didn't understand or didn't want passive port selection. */
		cip->errNo = result = kErrPASVFailed;
		goto done;
	}

	/* The other side returns a specification in the form of
	 * an internet address as the first four integers (each
	 * integer stands for 8-bits of the real 32-bit address),
	 * and two more integers for the port (16-bit port).
	 *
	 * It should give us something like:
	 * "Entering Passive Mode (129,93,33,1,10,187)", so look for
	 * digits with sscanf() starting 24 characters down the string.
	 */
	for (cp = rp->msg.first->line; ; cp++) {
		if (*cp == '\0') {
			FTPLogError(cip, kDontPerror, "Cannot parse PASV response: %s\n", rp->msg.first->line);
			goto done;
		}
		if (isdigit((int) *cp))
			break;
	}

	if (sscanf(cp, "%d,%d,%d,%d,%d,%d",
			&i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6) {
		FTPLogError(cip, kDontPerror, "Cannot parse PASV response: %s\n", rp->msg.first->line);
		goto done;
	}

	if (weird != (int *) 0)
		*weird = 0;
		
	for (j=0; j<6; j++) {
		/* Some ftp servers return bogus port octets, such as
		 * boombox.micro.umn.edu.  Let the caller know if we got a
		 * weird looking octet.
		 */
		if (((i[j] < 0) || (i[j] > 255)) && (weird != (int *) 0))
			*weird = *weird + 1;
		n[j] = (unsigned char) (i[j] & 0xff);
	}
	
	(void) memcpy(&saddr->sin_addr, &n[0], (size_t) 4);
	(void) memcpy(&saddr->sin_port, &n[4], (size_t) 2);

	result = kNoErr;
done:
	DoneWithResponse(cip, rp);
	return (result);
}	/* FTPSendPassive */




int
FTPFixPrivateAddr(struct sockaddr_in *maybePrivateAddr, struct sockaddr_in *knownNonPrivateAddrToUseIfNeeded)
{
	int i;
	char maybePrivateAddrStr[64];
	char knownNonPrivateAddrToUseIfNeededStr[64];

	AddrToAddrStr(maybePrivateAddrStr, sizeof(maybePrivateAddrStr), maybePrivateAddr, 0, "%h");
	AddrToAddrStr(knownNonPrivateAddrToUseIfNeededStr, sizeof(knownNonPrivateAddrToUseIfNeededStr), knownNonPrivateAddrToUseIfNeeded, 0, "%h");

	if (strcmp(maybePrivateAddrStr, knownNonPrivateAddrToUseIfNeededStr) == 0)
		return (0);		/* Assume if we could reach the Ctl, we can reach Data. */

	for (i=0; gPrivateNetworks[i] != NULL; i++) {
		if (strncmp(maybePrivateAddrStr, gPrivateNetworks[i], strlen(gPrivateNetworks[i])) == 0)
			break;
	}

	if (gPrivateNetworks[i] == NULL)
		return (0);		/* It wasn't a private network. */

	if (strncmp(knownNonPrivateAddrToUseIfNeededStr, gPrivateNetworks[i], strlen(gPrivateNetworks[i])) == 0)
		return (0);		/* Assume we might be able to reach slightly different net */

	memcpy(&maybePrivateAddr->sin_addr, &knownNonPrivateAddrToUseIfNeeded->sin_addr, sizeof(maybePrivateAddr->sin_addr));
	return (1);
}	/* FTPFixPrivateAddr */




void
FTPFixServerDataAddr(const FTPCIPtr cip)
{
	struct sockaddr_in oldServDataAddr;
	char servDataAddrStr[64];
	char newDataAddrStr[64];

	memcpy(&oldServDataAddr, &cip->servDataAddr, sizeof(oldServDataAddr));
	if (FTPFixPrivateAddr(&cip->servDataAddr, &cip->servCtlAddr)) {
		AddrToAddrStr(servDataAddrStr, sizeof(servDataAddrStr), &oldServDataAddr, 0, NULL);
		AddrToAddrStr(newDataAddrStr, sizeof(newDataAddrStr), &cip->servDataAddr, 0, NULL);
		PrintF(cip, "Fixing bogus PASV data address from %s to %s.\n", servDataAddrStr, newDataAddrStr);
	}
}	/* FTPFixServerDataAddr */




void
FTPFixClientDataAddr(const FTPCIPtr cip)
{
	struct sockaddr_in oldClientDataAddr, newClientDataAddr;
	char ourDataAddrStr[64];
	char newDataAddrStr[64];

	if (cip->clientKnownExternalAddr.sin_family == 0)
		return;

	memcpy(&oldClientDataAddr, &cip->ourDataAddr, sizeof(oldClientDataAddr));
	if (FTPFixPrivateAddr(&cip->ourDataAddr, &cip->clientKnownExternalAddr)) {
		memcpy(&newClientDataAddr, &cip->clientKnownExternalAddr, sizeof(newClientDataAddr));
		newClientDataAddr.sin_port = cip->ourDataAddr.sin_port;
		AddrToAddrStr(ourDataAddrStr, sizeof(ourDataAddrStr), &oldClientDataAddr, 0, NULL);
		AddrToAddrStr(newDataAddrStr, sizeof(newDataAddrStr), &newClientDataAddr, 0, NULL);
		PrintF(cip, "Fixing what would have been a bogus PORT data address from %s to %s.\n", ourDataAddrStr, newDataAddrStr);
	}
}	/* FTPFixClientDataAddr */




static int
BindToEphemeralPortNumber(int sockfd, struct sockaddr_in *addrp, int ephemLo, int ephemHi)
{
	int i;
	int result;
	int rangesize;
	unsigned short port;

	addrp->sin_family = AF_INET;
	if (((int) ephemLo == 0) || ((int) ephemLo >= (int) ephemHi)) {
		/* Do it the normal way.  System will
		 * pick one, typically in the range
		 * of 1024-4999.
		 */
		addrp->sin_port = 0;	/* Let system pick one. */

		result = bind(sockfd, (struct sockaddr *) addrp, sizeof(struct sockaddr_in));
	} else {
		rangesize = (int) ((int) ephemHi - (int) ephemLo);
		result = 0;
		for (i=0; i<10; i++) {
			port = (unsigned short) (((int) rand() % rangesize) + (int) ephemLo);
			addrp->sin_port = htons(port);

			result = bind(sockfd, (struct sockaddr *) addrp, sizeof(struct sockaddr_in));
			if (result == 0)
				break;
			if ((errno != 999)
				/* This next line is just fodder to
				 * shut the compiler up about variable
				 * not being used.
				 */
				&& (gCopyright[0] != '\0'))
				break;
		}
	}
	return (result);
}	/* BindToEphemeralPortNumber */




int
OpenDataConnection(const FTPCIPtr cip, int mode)
{
	int dataSocket;
	int weirdPort;
	int result;
	int setsbufs;
	size_t rbs, sbs;

	/* Before we can transfer any data, and before we even ask the
	 * remote server to start transferring via RETR/NLST/etc, we have
	 * to setup the connection.
	 */

tryPort2:
	weirdPort = 0;
	result = 0;
	CloseDataConnection(cip);	/* In case we didn't before... */

	dataSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (dataSocket < 0) {
		FTPLogError(cip, kDoPerror, "Could not get a data socket.\n");
		result = kErrNewStreamSocket;
		cip->errNo = kErrNewStreamSocket;
		return result;
	}

	if ((cip->dataSocketRBufSize != 0) || (cip->dataSocketSBufSize != 0)) {
		(void) SetSocketBufSize(dataSocket, cip->dataSocketRBufSize, cip->dataSocketSBufSize);
	} else if (GetSocketBufSize(dataSocket, &rbs, &sbs) == 0) {
		/* Use maximum size buffers that qualify for TCP Small Windows
		 * for file transfers.
		 */
		setsbufs = 0;
		if (rbs < 65536) {
			rbs = 65536;
			setsbufs++;
		}
		if (sbs < 65536) {
			sbs = 65536;
			setsbufs++;
		}
		if (setsbufs > 0) {
		        (void) SetSocketBufSize(dataSocket, rbs, sbs);
		}
	}

	if ((cip->hasPASV == kCommandNotAvailable) || (mode == kSendPortMode)) {
tryPort:
		cip->ourDataAddr = cip->ourCtlAddr;
		cip->ourDataAddr.sin_family = AF_INET;

#ifdef HAVE_LIBSOCKS
		cip->ourDataAddr.sin_port = 0;
		if (Rbind(dataSocket, (struct sockaddr *) &cip->ourDataAddr,
			(int) sizeof (cip->ourDataAddr),
			cip->servCtlAddr.sin_addr.s_addr) < 0) 
#else
		if (BindToEphemeralPortNumber(dataSocket, &cip->ourDataAddr, (int) cip->ephemLo, (int) cip->ephemHi) < 0)
#endif
		{
			FTPLogError(cip, kDoPerror, "Could not bind the data socket");
			result = kErrBindDataSocket;
			cip->errNo = kErrBindDataSocket;
			goto bad;
		}
	
		/* Need to do this so we can figure out which port the system
		 * gave to us.
		 */
		if ((result = GetSocketAddress(cip, dataSocket, &cip->ourDataAddr)) < 0)
			goto bad;
	
		if (listen(dataSocket, 1) < 0) {
			FTPLogError(cip, kDoPerror, "listen failed");
			result = kErrListenDataSocket;
			cip->errNo = kErrListenDataSocket;
			goto bad;
		}

		FTPFixClientDataAddr(cip);
		if ((result = FTPSendPort(cip, &cip->ourDataAddr)) < 0)
			goto bad;
	
		cip->dataPortMode = kSendPortMode;
	} else {
		/* Passive mode.  Let the other side decide where to send. */
		
		cip->servDataAddr = cip->servCtlAddr;
		cip->servDataAddr.sin_family = AF_INET;
		cip->ourDataAddr = cip->ourCtlAddr;
		cip->ourDataAddr.sin_family = AF_INET;

		if (FTPSendPassive(cip, &cip->servDataAddr, &weirdPort) < 0) {
			FTPLogError(cip, kDontPerror, "Passive mode refused.\n");
			cip->hasPASV = kCommandNotAvailable;
			
			/* We can try using regular PORT commands, which are required
			 * by all FTP protocol compliant programs, if you said so.
			 *
			 * We don't do this automatically, because if your host
			 * is running a firewall you (probably) do not want SendPort
			 * FTP for security reasons.
			 */
			if (mode == kFallBackToSendPortMode)
				goto tryPort;
			result = kErrPassiveModeFailed;
			cip->errNo = kErrPassiveModeFailed;
			goto bad;
		}
		FTPFixServerDataAddr(cip);

#ifdef HAVE_LIBSOCKS
		cip->ourDataAddr.sin_port = 0;
		if (Rbind(dataSocket, (struct sockaddr *) &cip->ourDataAddr,
			(int) sizeof (cip->ourDataAddr),
			cip->servCtlAddr.sin_addr.s_addr) < 0) 
#else
		if (BindToEphemeralPortNumber(dataSocket, &cip->ourDataAddr, (int) cip->ephemLo, (int) cip->ephemHi) < 0)
#endif
		{
			FTPLogError(cip, kDoPerror, "Could not bind the data socket");
			result = kErrBindDataSocket;
			cip->errNo = kErrBindDataSocket;
			goto bad;
		}

#ifdef NO_SIGNALS
		result = SConnect(dataSocket, &cip->servDataAddr, (int) cip->connTimeout);
#else	/* NO_SIGNALS */
		if (cip->connTimeout > 0)
			(void) alarm(cip->connTimeout);

		result = connect(dataSocket, (struct sockaddr *) &cip->servDataAddr, (int) sizeof(cip->servDataAddr));
		if (cip->connTimeout > 0)
			(void) alarm(0);
#endif	/* NO_SIGNALS */

#ifdef NO_SIGNALS
		if (result == kTimeoutErr) {
			if (mode == kFallBackToSendPortMode) {
				FTPLogError(cip, kDontPerror, "Data connection timed out.\n");
				FTPLogError(cip, kDontPerror, "Falling back to PORT instead of PASV mode.\n");
				(void) DisposeSocket(dataSocket);
				cip->hasPASV = kCommandNotAvailable;
				goto tryPort2;
			}
			FTPLogError(cip, kDontPerror, "Data connection timed out.\n");
			result = kErrConnectDataSocket;
			cip->errNo = kErrConnectDataSocket;
		} else
#endif	/* NO_SIGNALS */

		if (result < 0) {
#ifdef ECONNREFUSED
			if ((weirdPort > 0) && (errno == ECONNREFUSED)) {
#elif defined(WSAECONNREFUSED)
			if ((weirdPort > 0) && (errno == WSAECONNREFUSED)) {
#endif
				FTPLogError(cip, kDontPerror, "Server sent back a bogus port number.\nI will fall back to PORT instead of PASV mode.\n");
				if (mode == kFallBackToSendPortMode) {
					(void) DisposeSocket(dataSocket);
					cip->hasPASV = kCommandNotAvailable;
					goto tryPort2;
				}
				result = kErrServerSentBogusPortNumber;
				cip->errNo = kErrServerSentBogusPortNumber;
				goto bad;
			}
			if (mode == kFallBackToSendPortMode) {
				FTPLogError(cip, kDoPerror, "connect failed.\n");
				FTPLogError(cip, kDontPerror, "Falling back to PORT instead of PASV mode.\n");
				(void) DisposeSocket(dataSocket);
				cip->hasPASV = kCommandNotAvailable;
				goto tryPort2;
			}
			FTPLogError(cip, kDoPerror, "connect failed.\n");
			result = kErrConnectDataSocket;
			cip->errNo = kErrConnectDataSocket;
			goto bad;
		}
	
		/* Need to do this so we can figure out which port the system
		 * gave to us.
		 */
		if ((result = GetSocketAddress(cip, dataSocket, &cip->ourDataAddr)) < 0)
			goto bad;

		cip->dataPortMode = kPassiveMode;
		cip->hasPASV = kCommandAvailable;
	}

	(void) SetSocketKeepAlive(dataSocket, 1);

	/* Data connection is a non-interactive data stream, so
	 * high throughput is desired, at the expense of low
	 * response time.
	 */
	(void) SetSocketTypeOfService(dataSocket, IPTOS_THROUGHPUT);

	cip->dataSocket = dataSocket;
	return (0);
bad:
	(void) DisposeSocket(dataSocket);
	return (result);
}	/* OpenDataConnection */




int
AcceptDataConnection(const FTPCIPtr cip)
{
	int newSocket;
#ifndef NO_SIGNALS
	int len;
#endif
	unsigned short remoteDataPort;
	unsigned short remoteCtrlPort;
	char ctrlAddrStr[64], dataAddrStr[64];

	/* If we did a PORT, we have some things to finish up.
	 * If we did a PASV, we're ready to go.
	 */
	if (cip->dataPortMode == kSendPortMode) {
		/* Accept will give us back the server's data address;  at the
		 * moment we don't do anything with it though.
		 */
		memset(&cip->servDataAddr, 0, sizeof(cip->servDataAddr));

#ifdef NO_SIGNALS
		newSocket = SAccept(cip->dataSocket, &cip->servDataAddr, (int) cip->connTimeout);
#else	/* NO_SIGNALS */
		len = (int) sizeof(cip->servDataAddr);
		if (cip->connTimeout > 0)
			(void) alarm(cip->connTimeout);
		newSocket = accept(cip->dataSocket, (struct sockaddr *) &cip->servDataAddr, &len);
		if (cip->connTimeout > 0)
			(void) alarm(0);
#endif	/* NO_SIGNALS */

		(void) DisposeSocket(cip->dataSocket);
		if (newSocket < 0) {
			FTPLogError(cip, kDoPerror, "Could not accept a data connection.\n");
			cip->dataSocket = kClosedFileDescriptor;
			cip->errNo = kErrAcceptDataSocket;
			return (kErrAcceptDataSocket);
		}
	
		if (cip->allowProxyForPORT == 0) {
			if (memcmp(&cip->servDataAddr.sin_addr.s_addr, &cip->servCtlAddr.sin_addr.s_addr, sizeof(cip->servDataAddr.sin_addr.s_addr)) != 0) {
				AddrToAddrStr(ctrlAddrStr, sizeof(ctrlAddrStr), &cip->servCtlAddr, 0, NULL);
				AddrToAddrStr(dataAddrStr, sizeof(dataAddrStr), &cip->servDataAddr, 0, NULL);
				FTPLogError(cip, kDontPerror, "Data connection from %s did not originate from remote server %s!\n", dataAddrStr, ctrlAddrStr);
				(void) DisposeSocket(newSocket);
				cip->dataSocket = kClosedFileDescriptor;
				cip->errNo = kErrProxyDataConnectionsDisabled;
				return (kErrProxyDataConnectionsDisabled);
			}
		}

		if (cip->require20 != 0) {
			remoteDataPort = ntohs(cip->servDataAddr.sin_port);
			remoteCtrlPort = ntohs(cip->servCtlAddr.sin_port);
			if ((int) remoteDataPort != ((int) remoteCtrlPort - 1)) {
				FTPLogError(cip, kDontPerror, "Data connection did not originate on correct port (expecting %d, got %d)!\n", (int) remoteCtrlPort - 1, (int) remoteDataPort);
				(void) DisposeSocket(newSocket);
				cip->dataSocket = kClosedFileDescriptor;
				cip->errNo = kErrDataConnOriginatedFromBadPort;
				return (kErrDataConnOriginatedFromBadPort);
			}
		}
		cip->dataSocket = newSocket;
	}

	return (0);
}	/* AcceptDataConnection */




void
HangupOnServer(const FTPCIPtr cip)
{
	/* Since we want to close both sides of the connection for each
	 * socket, we can just have them closed with close() instead of
	 * using shutdown().
	 */
	FTPCloseControlConnection(cip);
	CloseDataConnection(cip);
}	/* HangupOnServer */




void
SendTelnetInterrupt(const FTPCIPtr cip)
{
	unsigned char msg[4];

	/* 1. User system inserts the Telnet "Interrupt Process" (IP) signal
	 *    in the Telnet stream.
	 */

	if (cip->cout != NULL)
		(void) fflush(cip->cout);
	
	msg[0] = (unsigned char) IAC;
	msg[1] = (unsigned char) IP;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	(void) send(cip->ctrlSocketW, (const char *) msg, 2, 0);
#else
	(void) send(cip->ctrlSocketW, (const char *) msg, 2, 0);
#endif
	/* 2. User system sends the Telnet "Sync" signal. */
	msg[0] = (unsigned char) IAC;
	msg[1] = (unsigned char) DM;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (send(cip->ctrlSocketW, (const char *) msg, 2, MSG_OOB) != 2)
#else
	if (send(cip->ctrlSocketW, (const char *) msg, 2, MSG_OOB) != 2)
#endif
		FTPLogError(cip, kDoPerror, "Could not send an urgent message.\n");
}	/* SendTelnetInterrupt */

/* eof FTP.c */
