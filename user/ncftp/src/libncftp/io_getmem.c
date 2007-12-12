/* io_getmem.c
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
FTPGetFileToMemory(
	const FTPCIPtr cip,
	const char *const file,
	char *memBuf,
	const size_t maxNumberOfBytesToWriteToMemBuf,
	size_t *const numberOfBytesWrittenToMemBuf,
	const longest_int startPoint,
	const int deleteflag
	)
{
	int tmpResult;
	volatile int result;
	int atEOF;
	longest_int expectedSize;
	size_t ntoread;
	read_return_t nread;
	size_t numberOfBytesLeftInMemBuf;
#if !defined(NO_SIGNALS)
	volatile FTPSigProc osigpipe;
	volatile FTPCIPtr vcip;
	int sj;
#endif	/* NO_SIGNALS */

	result = kNoErr;
	atEOF = 1;
	cip->usingTAR = 0;
	numberOfBytesLeftInMemBuf = maxNumberOfBytesToWriteToMemBuf;
	if (numberOfBytesWrittenToMemBuf != NULL)
		*numberOfBytesWrittenToMemBuf = 0;
		
	if ((file == NULL) || (file[0] == '\0') || (memBuf == NULL) || (maxNumberOfBytesToWriteToMemBuf == 0)) {
		return (kErrBadParameter);
	}
	
	FTPCheckForRestartModeAvailability(cip); 
	if ((startPoint != 0) && (cip->hasREST == kCommandNotAvailable)) {
		cip->errNo = kErrRESTNotAvailable;
		return (cip->errNo);
	}

	(void) FTPFileSize(cip, file, &expectedSize, kTypeBinary);
	if ((expectedSize != (longest_int) 0) && (startPoint > expectedSize)) {
		/* Don't go to all the trouble of downloading nothing. */
		if (deleteflag == kDeleteYes)
			(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
		return (kNoErr);
	}

	if ((cip->numDownloads == 0) && (cip->dataSocketRBufSize != 0)) {
		/* If dataSocketSBufSize is non-zero, it means you
		 * want to explicitly try to set the size of the
		 * socket's I/O buffer.
		 *
		 * If it is zero, it means you want to just use the
		 * TCP stack's default value, which is typically
		 * between 8 and 64 kB.
		 *
		 * If you try to set the buffer larger than 64 kB,
		 * the TCP stack should try to use RFC 1323 to
		 * negotiate "TCP Large Windows" which may yield
		 * significant performance gains.
		 */
		if (cip->hasSITE_RETRBUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RETRBUFSIZE %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_RBUFSIZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RBUFSIZ %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_RBUFSZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RBUFSZ %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_BUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE BUFSIZE %lu", (unsigned long) cip->dataSocketSBufSize);
	}

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

	tmpResult = FTPStartDataCmd(cip, kNetReading, kTypeBinary, startPoint, "RETR %s", file);

	if (tmpResult < 0) {
		result = tmpResult;
		if (result == kErrGeneric)
			result = kErrRETRFailed;
		cip->errNo = result;
#if !defined(NO_SIGNALS)
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
		return (result);
	}

	if ((startPoint != 0) && (cip->startPoint == 0)) {
		/* Remote could not or would not set the start offset
		 * to what we wanted.
		 */
		cip->errNo = kErrSetStartPoint;
#if !defined(NO_SIGNALS)
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
		return (cip->errNo);
	}

	FTPInitIOTimer(cip);
	cip->expectedSize = expectedSize;
	cip->lname = NULL;	/* could be NULL */
	cip->rname = file;
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

		ntoread = numberOfBytesLeftInMemBuf;
		if (ntoread > cip->bufSize)
			ntoread = cip->bufSize;	/* Break it up into blocks. */
			
#ifdef NO_SIGNALS
		nread = (read_return_t) SRead(cip->dataSocket, memBuf, ntoread, (int) cip->xferTimeout, kFullBufferNotRequired|kNoFirstSelect);
		if (nread == kTimeoutErr) {
			cip->errNo = result = kErrDataTimedOut;
			FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
			break;
		} else if (nread < 0) {
			if (errno == EPIPE) {
				result = cip->errNo = kErrSocketReadFailed;
				errno = EPIPE;
				FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
			} else if (errno == EINTR) {
				continue;
			} else {
				FTPLogError(cip, kDoPerror, "Remote read failed.\n");
				result = kErrSocketReadFailed;
				cip->errNo = kErrSocketReadFailed;
			}
			break;
		} else if (nread == 0) {
			break;
		}
#else			
		gCanBrokenDataJmp = 1;
		if (cip->xferTimeout > 0)
			(void) alarm(cip->xferTimeout);
		nread = read(cip->dataSocket, memBuf, (read_size_t) ntoread);
		if (nread < 0) {
			if ((gGotBrokenData != 0) || (errno == EPIPE)) {
				result = cip->errNo = kErrSocketReadFailed;
				errno = EPIPE;
				FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
			} else if (errno == EINTR) {
				continue;
			} else {
				result = cip->errNo = kErrSocketReadFailed;
				FTPLogError(cip, kDoPerror, "Remote read failed.\n");
			}
			(void) shutdown(cip->dataSocket, 2);
			break;
		} else if (nread == 0) {
			/* At EOF. */
			break;
		}
		gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

		memBuf += nread;
		if (numberOfBytesWrittenToMemBuf != NULL)
			*numberOfBytesWrittenToMemBuf += (size_t) nread;
		cip->bytesTransferred += (longest_int) nread;
		FTPUpdateIOTimer(cip);
		
		if ((size_t) nread > numberOfBytesLeftInMemBuf) {
			/* assertion failure */
			result = cip->errNo = kErrBugInLibrary;
			break;
		}
		
		numberOfBytesLeftInMemBuf -= nread;
		if (numberOfBytesLeftInMemBuf == 0) {
			/* Done (but maybe not at EOF of remote file). */
			atEOF = 0;
			if ((cip->bytesTransferred + startPoint) == expectedSize)
				atEOF = 1;
			break;
		}
	}

#if !defined(NO_SIGNALS)
	if (cip->xferTimeout > 0)
		(void) alarm(0);
	gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

	/* If there hasn't been an error, and you limited
	* the number of bytes, we need to abort the
	* remaining data.
	*/
	if ((result == kNoErr) && (atEOF == 0)) {
		FTPAbortDataTransfer(cip);
		tmpResult = FTPEndDataCmd(cip, 1);
		if ((tmpResult < 0) && (result == 0) && (tmpResult != kErrDataTransferFailed)) {
			result = kErrRETRFailed;
			cip->errNo = kErrRETRFailed;
		}
	} else {
		tmpResult = FTPEndDataCmd(cip, 1);
		if ((tmpResult < 0) && (result == 0)) {
			result = kErrRETRFailed;
			cip->errNo = kErrRETRFailed;
		}
	}

	FTPStopIOTimer(cip);
#if !defined(NO_SIGNALS)
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */

	if (result == kNoErr) {
		cip->numDownloads++;

		if (deleteflag == kDeleteYes) {
			result = FTPDelete(cip, file, kRecursiveNo, kGlobNo);
		}
	}

	return (result);
}	/* FTPGetOneF */
