/* io_gettar.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define ASCII_TRANSLATION 0
#endif

#ifndef ASCII_TRANSLATION
#	define ASCII_TRANSLATION 1
#endif

#ifndef NO_SIGNALS
#	define NO_SIGNALS 1
#endif

#ifndef O_BINARY
	/* Needed for platforms using different EOLN sequence (i.e. DOS) */
#	ifdef _O_BINARY
#		define O_BINARY _O_BINARY
#	else
#		define O_BINARY 0
#	endif
#endif

/* Nice for UNIX, but not necessary otherwise. */
#ifdef TAR

static int
OpenTar(const FTPCIPtr cip, const char *const dstdir, int *const pid)
{
	int pipe1[2];
	int pid1;
	int i;
	char *argv[8];

	*pid = -1;

	if (access(TAR, X_OK) < 0) {
		/* Path to TAR is invalid. */
		return (-1);
	}

	if (pipe(pipe1) < 0) {
		FTPLogError(cip, kDoPerror, "pipe to Tar failed");
		return (-1);
	}

	pid1 = (int) fork();
	if (pid1 < 0) {
		(void) close(pipe1[0]);
		(void) close(pipe1[1]);
		return (-1);
	} else if (pid1 == 0) {
		/* Child */
		if ((dstdir != NULL) && (dstdir[0] != '\0') && (chdir(dstdir) < 0)) {
			FTPLogError(cip, kDoPerror, "tar chdir to %s failed", dstdir);
			exit(1);
		}
		(void) close(pipe1[1]);		/* close write end */
		(void) dup2(pipe1[0], 0);	/* use read end on stdin */
		(void) close(pipe1[0]);

		for (i=3; i<256; i++)
			(void) close(i);

		argv[0] = strdup("tar");
		argv[1] = strdup("xpf");
		argv[2] = strdup("-");
		argv[3] = NULL;

		(void) execv(TAR, argv);
		exit(1);
	}

	/* Parent */
	*pid = pid1;

	(void) close(pipe1[0]);		/* close read end */
	return (pipe1[1]);		/* use write end */
}	/* OpenTar */




int
FTPGetOneTarF(const FTPCIPtr cip, const char *file, const char *const dstdir)
{
	char *buf;
	size_t bufSize;
	int tmpResult;
	volatile int result;
	read_return_t nread;
	write_return_t nwrote;
	volatile int fd;
	volatile int vfd;
	const char *volatile vfile;
#ifndef NO_SIGNALS
	int sj;
	volatile FTPSigProc osigpipe;
	volatile FTPCIPtr vcip;
#endif
	int pid, status;
	char savedCwd[512];
	char *volatile basecp;

	result = kNoErr;
	cip->usingTAR = 0;

	if ((file[0] == '\0') || ((file[0] == '/') && (file[1] == '\0'))) {
		/* It was "/"
		 * We can't do that, because "get /.tar"
		 * or "get .tar" does not work.
		 */
		result = kErrOpenFailed;
		cip->errNo = kErrOpenFailed;
		return (result);
	}

	if (FTPCmd(cip, "MDTM %s.tar", file) == 2) {
		/* Better not use this method since there is
		 * no way to tell if the server would use the
		 * existing .tar or do a new one on the fly.
		 */
		result = kErrOpenFailed;
		cip->errNo = kErrOpenFailed;
		return (result);
	}

	basecp = strrchr(file, '/');
	if (basecp != NULL)
		basecp = strrchr(file, '\\');
	if (basecp != NULL) {
		/* Need to cd to the parent directory and get it
		 * from there.
		 */
		if (FTPGetCWD(cip, savedCwd, sizeof(savedCwd)) != 0) {
			result = kErrOpenFailed;
			cip->errNo = kErrOpenFailed;
			return (result);
		}
		result = FTPChdir(cip, file);
		if (result != kNoErr) {
			return (result);
		}
		result = FTPChdir(cip, "..");
		if (result != kNoErr) {
			(void) FTPChdir(cip, savedCwd);
			return (result);
		}
		file = basecp + 1;
	}

	fd = OpenTar(cip, dstdir, &pid);
	if (fd < 0) {
		result = kErrOpenFailed;
		cip->errNo = kErrOpenFailed;
		if (basecp != NULL)
			(void) FTPChdir(cip, savedCwd);
		return (result);
	}

	vfd = fd;
	vfile = file;

#ifndef NO_SIGNALS
	vcip = cip;
	osigpipe = (volatile FTPSigProc) signal(SIGPIPE, BrokenData);

	gGotBrokenData = 0;
	gCanBrokenDataJmp = 0;

#ifdef HAVE_SIGSETJMP
	sj = sigsetjmp(gBrokenDataJmp, 1);
#else
	sj = setjmp(gBrokenDataJmp);
#endif	/* HAVE_SIGSETJMP */

	if (sj != 0) {
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
		FTPShutdownHost(vcip);

		(void) signal(SIGPIPE, SIG_IGN);
		(void) close(vfd);
		for (;;) {
#ifdef HAVE_WAITPID
			if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR))
				break;
#else
			if ((wait(&status) < 0) && (errno != EINTR))
				break;
#endif
			if (WIFEXITED(status) || WIFSIGNALED(status))
				break;		/* done */
		}
		if (basecp != NULL)
			(void) FTPChdir(cip, savedCwd);
		vcip->errNo = kErrRemoteHostClosedConnection;
		return(vcip->errNo);
	}
	gCanBrokenDataJmp = 1;

#endif	/* NO_SIGNALS */

	tmpResult = FTPStartDataCmd(cip, kNetReading, kTypeBinary, (longest_int) 0, "RETR %s.tar", vfile);

	if (tmpResult < 0) {
		result = tmpResult;
		if (result == kErrGeneric)
			result = kErrRETRFailed;
		cip->errNo = result;

#ifndef NO_SIGNALS
		(void) signal(SIGPIPE, SIG_IGN);
#endif
		(void) close(vfd);
		for (;;) {
#ifdef HAVE_WAITPID
			if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR))
				break;
#else
			if ((wait(&status) < 0) && (errno != EINTR))
				break;
#endif
			if (WIFEXITED(status) || WIFSIGNALED(status))
				break;		/* done */
		}

#ifndef NO_SIGNALS
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif
		if (basecp != NULL)
			(void) FTPChdir(cip, savedCwd);
		return (result);
	}

	cip->usingTAR = 1;
	buf = cip->buf;
	bufSize = cip->bufSize;

	FTPInitIOTimer(cip);
	cip->lname = vfile;	/* could be NULL */
	cip->rname = vfile;
	FTPStartIOTimer(cip);

	/* Binary */
	for (;;) {
		if (! WaitForRemoteInput(cip)) {	/* could set cancelXfer */
			cip->errNo = result = kErrDataTimedOut;
			FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
			break;
		}
		if (cip->cancelXfer > 0) {
			FTPAbortDataTransfer(cip);
			result = cip->errNo = kErrDataTransferAborted;
			break;
		}
#if !defined(NO_SIGNALS)
		gCanBrokenDataJmp = 1;
		if (cip->xferTimeout > 0)
			(void) alarm(cip->xferTimeout);
#endif	/* NO_SIGNALS */
#ifdef NO_SIGNALS
		nread = (read_return_t) SRead(cip->dataSocket, buf, bufSize, (int) cip->xferTimeout, kFullBufferNotRequired|kNoFirstSelect);
		if (nread == kTimeoutErr) {
			cip->errNo = result = kErrDataTimedOut;
			FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
			break;
		} else if (nread < 0) {
			if (errno == EINTR)
				continue;
			FTPLogError(cip, kDoPerror, "Remote read failed.\n");
			result = kErrSocketReadFailed;
			cip->errNo = kErrSocketReadFailed;
			break;
		} else if (nread == 0) {
			break;
		}
#else
		nread = read(cip->dataSocket, buf, (read_size_t) bufSize);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			FTPLogError(cip, kDoPerror, "Remote read failed.\n");
			result = kErrSocketReadFailed;
			cip->errNo = kErrSocketReadFailed;
			break;
		} else if (nread == 0) {
			break;
		}
		gCanBrokenDataJmp = 0;
#endif

		nwrote = write(fd, buf, (write_size_t) nread);
		if (nwrote != nread) {
			if (errno == EPIPE) {
				result = kErrWriteFailed;
				cip->errNo = kErrWriteFailed;
				errno = EPIPE;
			} else {
				FTPLogError(cip, kDoPerror, "Local write failed.\n");
				result = kErrWriteFailed;
				cip->errNo = kErrWriteFailed;
			}
			break;
		}
		cip->bytesTransferred += (longest_int) nread;
		FTPUpdateIOTimer(cip);
	}

#if !defined(NO_SIGNALS)
	if (cip->xferTimeout > 0)
		(void) alarm(0);
	gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

	(void) close(fd);
	for (;;) {
#ifdef HAVE_WAITPID
		if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR))
			break;
#else
		if ((wait(&status) < 0) && (errno != EINTR))
			break;
#endif
		if (WIFEXITED(status) || WIFSIGNALED(status))
			break;		/* done */
	}

	tmpResult = FTPEndDataCmd(cip, 1);
	if ((tmpResult < 0) && (result == 0)) {
		result = kErrRETRFailed;
		cip->errNo = kErrRETRFailed;
	}
	FTPStopIOTimer(cip);
#if !defined(NO_SIGNALS)
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif

	if ((result == 0) && (cip->bytesTransferred == 0)) {
		result = kErrOpenFailed;
		cip->errNo = kErrOpenFailed;
	}
	if (basecp != NULL)
		(void) FTPChdir(cip, savedCwd);
	return (result);
}	/* FTPGetOneTarF */

#endif	/* TAR */
