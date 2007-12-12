/* io_util.c
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

#ifndef O_BINARY
	/* Needed for platforms using different EOLN sequence (i.e. DOS) */
#	ifdef _O_BINARY
#		define O_BINARY _O_BINARY
#	else
#		define O_BINARY 0
#	endif
#endif

double
FTPDuration(struct timeval *t0)
{
	struct timeval t1;
	double sec;

	(void) gettimeofday(&t1, NULL);
	if (t0->tv_usec > t1.tv_usec) {
		t1.tv_usec += 1000000;
		t1.tv_sec--;
	}
	sec = ((double) (t1.tv_usec - t0->tv_usec) * 0.000001)
		+ (t1.tv_sec - t0->tv_sec);

	return (sec);
}	/* FTPDuration */




void
FTPInitIOTimer(const FTPCIPtr cip)
{
	cip->bytesTransferred = (longest_int) 0;
	cip->expectedSize = kSizeUnknown;
	cip->mdtm = kModTimeUnknown;
	cip->rname = NULL;
	cip->lname = NULL;
	cip->kBytesPerSec = -1.0;
	cip->percentCompleted = -1.0;
	cip->sec = -1.0;
	cip->secLeft = -1.0;
	cip->nextProgressUpdate = 0;
	cip->stalled = 0;
	cip->dataTimedOut = 0;
	cip->useProgressMeter = 1;
	(void) gettimeofday(&cip->t0, NULL);
}	/* FTPInitIOTimer */




void
FTPStartIOTimer(const FTPCIPtr cip)
{
	(void) gettimeofday(&cip->t0, NULL);
	if (cip->progress != (FTPProgressMeterProc) 0)
		(*cip->progress)(cip, kPrInitMsg);
}	/* FTPStartIOTimer */




void
FTPUpdateIOTimer(const FTPCIPtr cip)
{
	double sec;
	struct timeval *t0, t1;
	time_t now;

	(void) time(&now);
	if ((now < cip->nextProgressUpdate) && (cip->canceling == 0))
		return;
	now += 1;
	cip->nextProgressUpdate = now;

	(void) gettimeofday(&t1, NULL);
	t0 = &cip->t0;

	if (t0->tv_usec > t1.tv_usec) {
		t1.tv_usec += 1000000;
		t1.tv_sec--;
	}
	sec = ((double) (t1.tv_usec - t0->tv_usec) * 0.000001)
		+ (t1.tv_sec - t0->tv_sec);
	if (sec > 0.0) {
		cip->kBytesPerSec = ((double) cip->bytesTransferred) / (1024.0 * sec);
	} else {
		cip->kBytesPerSec = -1.0;
	}
	if (cip->expectedSize == kSizeUnknown) {
		cip->percentCompleted = -1.0;
		cip->secLeft = -1.0;
	} else if (cip->expectedSize <= 0) {
		cip->percentCompleted = 100.0;
		cip->secLeft = 0.0;
	} else {
		cip->percentCompleted = ((double) (100.0 * (cip->bytesTransferred + cip->startPoint))) / ((double) cip->expectedSize);
		if (cip->percentCompleted >= 100.0) {
			cip->percentCompleted = 100.0;
			cip->secLeft = 0.0;
		} else if (cip->percentCompleted <= 0.0) {
			cip->secLeft = 999.0;
		}
		if (cip->kBytesPerSec > 0.0) {
			cip->secLeft = ((cip->expectedSize - cip->bytesTransferred - cip->startPoint) / 1024.0) / cip->kBytesPerSec;
			if (cip->secLeft < 0.0)
				cip->secLeft = 0.0;
		}
	}
	cip->sec = sec;
	if ((cip->progress != (FTPProgressMeterProc) 0) && (cip->useProgressMeter != 0))
		(*cip->progress)(cip, kPrUpdateMsg);
}	/* FTPUpdateIOTimer */




void
FTPStopIOTimer(const FTPCIPtr cip)
{
	cip->nextProgressUpdate = 0;	/* force last update */
	FTPUpdateIOTimer(cip);
	if (cip->progress != (FTPProgressMeterProc) 0)
		(*cip->progress)(cip, kPrEndMsg);
}	/* FTPStopIOTimer */




/* The purpose of this is to provide updates for the progress meters
 * during lags.  Return zero if the operation timed-out.
 */
int
WaitForRemoteInput(const FTPCIPtr cip)
{
	fd_set ss, ss2;
	struct timeval tv;
	int result;
	int fd;
	int wsecs;
	int xferTimeout;
	int ocancelXfer;

	xferTimeout = cip->xferTimeout;
	if (xferTimeout < 1)
		return (1);

	fd = cip->dataSocket;
	if (fd < 0)
		return (1);

	if (cip->dataTimedOut > 0) {
		cip->dataTimedOut++;
		return (0);	/* already timed-out */
	}

	ocancelXfer = cip->cancelXfer;
	wsecs = 0;
	cip->stalled = 0;

	while ((xferTimeout <= 0) || (wsecs < xferTimeout)) {
		if ((cip->cancelXfer != 0) && (ocancelXfer == 0)) {
			/* leave cip->stalled -- could have been stalled and then canceled. */
			return (1);
		}
		MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
		MY_FD_SET(fd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
		ss2 = ss;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		result = select(fd + 1, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG234 &ss2, &tv);
		if (result == 1) {
			/* ready */
			cip->stalled = 0;
			return (1);
		} else if (result < 0) {
			if (errno != EINTR) {
				cip->stalled = 0;
				return (1);	/* Ready to read error */
			}
		} else {
			wsecs++;
			cip->stalled = wsecs;
		}
		FTPUpdateIOTimer(cip);
	}

#if !defined(NO_SIGNALS)
	/* Shouldn't get here -- alarm() should have
	 * went off by now.
	 */
	(void) kill(getpid(), SIGALRM);
#endif	/* NO_SIGNALS */

	cip->dataTimedOut++;
	return (0);	/* timed-out */
}	/* WaitForRemoteInput */




/* The purpose of this is to provide updates for the progress meters
 * during lags.  Return zero if the operation timed-out.
 */
int
WaitForRemoteOutput(const FTPCIPtr cip)
{
	fd_set ss, ss2;
	struct timeval tv;
	int result;
	int fd;
	int wsecs;
	int xferTimeout;
	int ocancelXfer;

	xferTimeout = cip->xferTimeout;
	if (xferTimeout < 1)
		return (1);

	fd = cip->dataSocket;
	if (fd < 0)
		return (1);

	if (cip->dataTimedOut > 0) {
		cip->dataTimedOut++;
		return (0);	/* already timed-out */
	}

	ocancelXfer = cip->cancelXfer;
	wsecs = 0;
	cip->stalled = 0;

	while ((xferTimeout <= 0) || (wsecs < xferTimeout)) {
		if ((cip->cancelXfer != 0) && (ocancelXfer == 0)) {
			/* leave cip->stalled -- could have been stalled and then canceled. */
			return (1);
		}
		MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
		MY_FD_SET(fd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
		ss2 = ss;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		result = select(fd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &ss2, &tv);
		if (result == 1) {
			/* ready */
			cip->stalled = 0;
			return (1);
		} else if (result < 0) {
			if (errno != EINTR) {
				cip->stalled = 0;
				return (1);	/* Ready to read error */
			}
		} else {
			wsecs++;
			cip->stalled = wsecs;
		}
		FTPUpdateIOTimer(cip);
	}

#if !defined(NO_SIGNALS)
	/* Shouldn't get here -- alarm() should have
	 * went off by now.
	 */
	(void) kill(getpid(), SIGALRM);
#endif	/* NO_SIGNALS */

	cip->dataTimedOut++;
	return (0);	/* timed-out */
}	/* WaitForRemoteOutput */




void
AutomaticallyUseASCIIModeDependingOnExtension(const FTPCIPtr cip, const char *const pathName, int *const xtype)
{
	if ((*xtype == kTypeBinary) && (cip->asciiFilenameExtensions != NULL)) {
		if (FilenameExtensionIndicatesASCII(pathName, cip->asciiFilenameExtensions)) {
			/* Matched -- send this file in ASCII mode
			 * instead of binary since it's extension
			 * appears to be that of a text file.
			 */
			*xtype = kTypeAscii;
		}
	}
}	/* AutomaticallyUseASCIIModeDependingOnExtension */



void
FTPCheckForRestartModeAvailability(const FTPCIPtr cip)
{
	if (cip->hasREST == kCommandAvailabilityUnknown) {
		(void) FTPSetTransferType(cip, kTypeBinary);
		if (FTPSetStartOffset(cip, (longest_int) 1) == kNoErr) {
			/* Now revert -- we still may not end up
			 * doing it.
			 */
			FTPSetStartOffset(cip, (longest_int) -1);
		}
	}
}	/* FTPCheckForRestartModeAvailability */



void
FTPSetUploadSocketBufferSize(const FTPCIPtr cip)
{
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
	if ((cip->numUploads == 0) && (cip->dataSocketSBufSize != 0)) {
		if (cip->hasSITE_STORBUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE STORBUFSIZE %lu", (unsigned long) cip->dataSocketSBufSize);
		else if (cip->hasSITE_SBUFSIZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE SBUFSIZ %lu", (unsigned long) cip->dataSocketSBufSize);
		else if (cip->hasSITE_SBUFSZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE SBUFSZ %lu", (unsigned long) cip->dataSocketSBufSize);
		/* At least one server implemenation has RBUFSZ but not
		 * SBUFSZ and instead uses RBUFSZ for both.
		 */
		else if ((cip->hasSITE_SBUFSZ != kCommandAvailable) && (cip->hasSITE_RBUFSZ == kCommandAvailable))
			(void) FTPCmd(cip, "SITE RBUFSZ %lu", (unsigned long) cip->dataSocketSBufSize);
		else if (cip->hasSITE_BUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE BUFSIZE %lu", (unsigned long) cip->dataSocketSBufSize);
	}
}	/* FTPSetUploadSocketBufferSize */
