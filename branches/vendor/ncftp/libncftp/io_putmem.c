/* io_putmem.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
#	define NO_SIGNALS 1
#endif

int
FTPPutFileFromMemory(
	const FTPCIPtr cip,
	const char *volatile dstfile,
	const char *volatile src,
	const size_t srcLen,
	const int appendflag)
{
	const char *cp;
	const char *cmd;
	int tmpResult, result;
	read_return_t nread;
	write_return_t nwrote;
	size_t bufSize;
	const char *srcLim;
	const char *volatile srcp;
#if !defined(NO_SIGNALS)
	int sj;
	volatile FTPSigProc osigpipe;
	volatile FTPCIPtr vcip;
#endif	/* NO_SIGNALS */

	if (cip->buf == NULL) {
		FTPLogError(cip, kDoPerror, "Transfer buffer not allocated.\n");
		cip->errNo = kErrNoBuf;
		return (cip->errNo);
	}

	cip->usingTAR = 0;

	/* For Put, we can't recover very well if it turns out restart
	 * didn't work, so check beforehand.
	 */
	FTPCheckForRestartModeAvailability(cip); 

	FTPSetUploadSocketBufferSize(cip);

#ifdef NO_SIGNALS
#else	/* NO_SIGNALS */
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
		vcip->errNo = kErrRemoteHostClosedConnection;
		return(vcip->errNo);
	}
	gCanBrokenDataJmp = 1;
#endif	/* NO_SIGNALS */

	cmd = appendflag == kAppendYes ? "APPE" : "STOR";
	tmpResult = FTPStartDataCmd(
		cip,
		kNetWriting,
		kTypeBinary,
		(longest_int) 0,
		"%s %s",
		cmd,
		dstfile
	);

	if (tmpResult < 0) {
		cip->errNo = tmpResult;
#if !defined(NO_SIGNALS)
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
		return (cip->errNo);
	}

	result = kNoErr;
	bufSize = cip->bufSize;

	FTPInitIOTimer(cip);
	cip->expectedSize = (longest_int) srcLen;
	cip->lname = NULL;	/* could be NULL */
	cip->rname = dstfile;
	srcp = src;
	srcLim = src + srcLen;
	FTPStartIOTimer(cip);
	{
		/* binary */
		for (;;) {
#if !defined(NO_SIGNALS)
			gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */
			nread = (read_return_t) bufSize;
			if ((size_t) (srcLim - srcp) < bufSize) {
				nread = (read_return_t) (srcLim - srcp);
				if (nread == 0) {
					result = kNoErr;
					break;
				}
			}
			cip->bytesTransferred += (longest_int) nread;
			cp = srcp;
			srcp += nread;

#if !defined(NO_SIGNALS)
			gCanBrokenDataJmp = 1;
			if (cip->xferTimeout > 0)
				(void) alarm(cip->xferTimeout);
#endif	/* NO_SIGNALS */
			do {
				if (! WaitForRemoteOutput(cip)) {	/* could set cancelXfer */
					cip->errNo = result = kErrDataTimedOut;
					FTPLogError(cip, kDontPerror, "Remote write timed out.\n");
					goto brk;
				}
				if (cip->cancelXfer > 0) {
					FTPAbortDataTransfer(cip);
					result = cip->errNo = kErrDataTransferAborted;
					goto brk;
				}

#ifdef NO_SIGNALS
				nwrote = (write_return_t) SWrite(cip->dataSocket, cp, (size_t) nread, (int) cip->xferTimeout, kNoFirstSelect);
				if (nwrote < 0) {
					if (nwrote == kTimeoutErr) {
						cip->errNo = result = kErrDataTimedOut;
						FTPLogError(cip, kDontPerror, "Remote write timed out.\n");
					} else if (errno == EPIPE) {
						cip->errNo = result = kErrSocketWriteFailed;
						errno = EPIPE;
						FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
					} else if (errno == EINTR) {
						continue;
					} else {
						cip->errNo = result = kErrSocketWriteFailed;
						FTPLogError(cip, kDoPerror, "Remote write failed.\n");
					}
					(void) shutdown(cip->dataSocket, 2);
					goto brk;
				}
#else	/* NO_SIGNALS */
				nwrote = write(cip->dataSocket, cp, (write_size_t) nread);
				if (nwrote < 0) {
					if ((gGotBrokenData != 0) || (errno == EPIPE)) {
						cip->errNo = result = kErrSocketWriteFailed;
						errno = EPIPE;
						FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
					} else if (errno == EINTR) {
						continue;
					} else {
						cip->errNo = result = kErrSocketWriteFailed;
						FTPLogError(cip, kDoPerror, "Remote write failed.\n");
					}
					(void) shutdown(cip->dataSocket, 2);
					goto brk;
				}
#endif	/* NO_SIGNALS */
				cp += nwrote;
				nread -= nwrote;
			} while (nread > 0);
			FTPUpdateIOTimer(cip);
		}
	}
brk:

	/* This looks very bizarre, since
	 * we will be checking the socket
	 * for readability here!
	 *
	 * The reason for this is that we
	 * want to be able to timeout a
	 * small put.  So, we close the
	 * write end of the socket first,
	 * which tells the server we're
	 * done writing.  We then wait
	 * for the server to close down
	 * the whole socket (we know this
	 * when the socket is ready for
	 * reading an EOF), which tells
	 * us that the file was completed.
	 */
	(void) shutdown(cip->dataSocket, 1);
	(void) WaitForRemoteInput(cip);

#if !defined(NO_SIGNALS)
	gCanBrokenDataJmp = 0;
	if (cip->xferTimeout > 0)
		(void) alarm(0);
#endif	/* NO_SIGNALS */
	tmpResult = FTPEndDataCmd(cip, 1);
	if ((tmpResult < 0) && (result == kNoErr)) {
		cip->errNo = result = kErrSTORFailed;
	}
	FTPStopIOTimer(cip);

	if (result == kNoErr) {
		/* The store succeeded;  If we were
		 * uploading to a temporary file,
		 * move the new file to the new name.
		 */
		cip->numUploads++;
	}

#if !defined(NO_SIGNALS)
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
	return (result);
}	/* FTPPutFileFromMemory */
