/* io_list.c
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

/* This isn't too useful -- it mostly serves as an example so you can write
 * your own function to do what you need to do with the listing.
 */
int
FTPList(const FTPCIPtr cip, const int outfd, const int longMode, const char *const lsflag)
{
	const char *cmd;
	char line[512];
	char secondaryBuf[768];
#ifndef NO_SIGNALS
	char *secBufPtr, *secBufLimit;
	int nread;
	volatile int result;
#else	/* NO_SIGNALS */
	SReadlineInfo lsSrl;
	int result;
#endif	/* NO_SIGNALS */

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	cmd = (longMode != 0) ? "LIST" : "NLST";
	if ((lsflag == NULL) || (lsflag[0] == '\0')) {
		result = FTPStartDataCmd(cip, kNetReading, kTypeAscii, (longest_int) 0, "%s", cmd);
	} else {
		result = FTPStartDataCmd(cip, kNetReading, kTypeAscii, (longest_int) 0, "%s %s", cmd, lsflag);
	}


#ifdef NO_SIGNALS

	if (result == 0) {
		if (InitSReadlineInfo(&lsSrl, cip->dataSocket, secondaryBuf, sizeof(secondaryBuf), (int) cip->xferTimeout, 1) < 0) {
			/* Not really fdopen, but close in what we're trying to do. */
			result = kErrFdopenR;
			cip->errNo = kErrFdopenR;
			FTPLogError(cip, kDoPerror, "Could not fdopen.\n");
			return (result);
		}
		
		for (;;) {
			result = SReadline(&lsSrl, line, sizeof(line) - 2);
			if (result == kTimeoutErr) {
				/* timeout */
				FTPLogError(cip, kDontPerror, "Could not directory listing data -- timed out.\n");
				cip->errNo = kErrDataTimedOut;
				return (cip->errNo);
			} else if (result == 0) {
				/* end of listing -- done */
				cip->numListings++;
				break;
			} else if (result < 0) {
				/* error */
				FTPLogError(cip, kDoPerror, "Could not read directory listing data");
				result = kErrLISTFailed;
				cip->errNo = kErrLISTFailed;
				break;
			}

			(void) write(outfd, line, (write_size_t) strlen(line));
		}

		DisposeSReadlineInfo(&lsSrl);
		if (FTPEndDataCmd(cip, 1) < 0) {
			result = kErrLISTFailed;
			cip->errNo = kErrLISTFailed;
		}
	} else if (result == kErrGeneric) {
		result = kErrLISTFailed;
		cip->errNo = kErrLISTFailed;
	}


#else	/* NO_SIGNALS */
	
	if (result == 0) {
		/* This line sets the buffer pointer so that the first thing
		 * BufferGets will do is reset and fill the buffer using
		 * real I/O.
		 */
		secBufPtr = secondaryBuf + sizeof(secondaryBuf);
		secBufLimit = (char *) 0;

		for (;;) {
			if (cip->xferTimeout > 0)
				(void) alarm(cip->xferTimeout);
			nread = BufferGets(line, sizeof(line), cip->dataSocket, secondaryBuf, &secBufPtr, &secBufLimit, sizeof(secondaryBuf));
			if (nread <= 0) {
				if (nread < 0)
					break;
			} else {
				cip->bytesTransferred += (longest_int) nread;
				(void) STRNCAT(line, "\n");
				(void) write(outfd, line, (write_size_t) strlen(line));
			}
		}
		if (cip->xferTimeout > 0)
			(void) alarm(0);
		result = FTPEndDataCmd(cip, 1);
		if (result < 0) {
			result = kErrLISTFailed;
			cip->errNo = kErrLISTFailed;
		}
		result = kNoErr;
		cip->numListings++;
	} else if (result == kErrGeneric) {
		result = kErrLISTFailed;
		cip->errNo = kErrLISTFailed;
	}
#endif	/* NO_SIGNALS */
	return (result);
}	/* FTPList */
