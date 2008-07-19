/* io_listmem.c
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

int
FTPListToMemory2(const FTPCIPtr cip, const char *const pattern, const FTPLineListPtr llines, const char *const lsflags, const int blankLines, int *const tryMLSD)
{
	char secondaryBuf[768];
	char line[512];
	char lsflags1[128];
	const char *command = "NLST";
	const char *scp;
	char *dcp, *lim;
#ifndef NO_SIGNALS
	char *secBufPtr, *secBufLimit;
	volatile FTPSigProc osigpipe;
	volatile FTPCIPtr vcip;
	int sj;
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

	if ((llines == NULL) || (pattern == NULL) || (lsflags == NULL))
		return (kErrBadParameter);

	if ((tryMLSD != (int *) 0) && (*tryMLSD != 0) && (cip->hasMLSD == kCommandAvailable)) {
		command = "MLSD";
		if ((lsflags[0] == '-') && (strchr(lsflags, 'd') != NULL) && (cip->hasMLST == kCommandAvailable))
			command = "MLST";
		lsflags1[0] = '\0';
		FTPRequestMlsOptions(cip);
	} else {
		/* Not using MLSD. */
		if (tryMLSD != (int *) 0)
			*tryMLSD = 0;
		if (lsflags[0] == '-') {
			/* See if we should use LIST instead. */
			if (strstr(lsflags, "--") != NULL) {
				/* You are trying to use one of those
				 * --extended-options type of flags;
				 * assume you want LIST and that you
				 * know what you're doing...
				 */
				command = "LIST";
				(void) STRNCPY(lsflags1, lsflags);
			} else {
				scp = lsflags + 1;
				dcp = lsflags1;
				lim = dcp + sizeof(lsflags1) - 2;
				for (; *scp != '\0'; scp++) {
					if (isspace((int) *scp))
						continue;
					if (*scp == '-')
						continue;
					if (*scp == 'l') {
						/* do not add the 'l' */
						command = "LIST";
					} else if (dcp < lim) {
						if (dcp == lsflags1)
							*dcp++ = '-';
						*dcp++ = *scp;
					}
				}
				*dcp = '\0';
			}
		} else {
			(void) STRNCPY(lsflags1, lsflags);
		}
	}

	InitLineList(llines);

	result = FTPStartDataCmd(
		cip,
		kNetReading,
		kTypeAscii,
		(longest_int) 0,
		"%s%s%s%s%s",
		command,
		(lsflags1[0] == '\0') ? "" : " ",
		lsflags1,
		(pattern[0] == '\0') ? "" : " ",
		pattern
	);

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
			result = SReadline(&lsSrl, line, sizeof(line) - 1);
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

			if (line[result - 1] == '\n')
				line[result - 1] = '\0';

			if ((blankLines == 0) && (result <= 1))
				continue;

			/* Valid directory listing line of output */
#define islsenddelim(c) ((c == '\0') || ((iscntrl((int) c)) && (! isspace((int) c))))
			if ((line[0] == '.') && ((islsenddelim(line[1])) || ((line[1] == '.') && (islsenddelim(line[2])))))
				continue;	/* Skip . and .. */

			(void) AddLine(llines, line);
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

	if (result == 0) {
		/* This line sets the buffer pointer so that the first thing
		 * BufferGets will do is reset and fill the buffer using
		 * real I/O.
		 */
		secBufPtr = secondaryBuf + sizeof(secondaryBuf);
		secBufLimit = (char *) 0;
		memset(secondaryBuf, 0, sizeof(secondaryBuf));

		for (;;) {
			memset(line, 0, sizeof(line));
			if (cip->xferTimeout > 0)
				(void) alarm(cip->xferTimeout);
			nread = BufferGets(line, sizeof(line), cip->dataSocket, secondaryBuf, &secBufPtr, &secBufLimit, sizeof(secondaryBuf));
			if (nread <= 0) {
				if (nread < 0)
					break;
				if (blankLines != 0)
					(void) AddLine(llines, line);
			} else {
				cip->bytesTransferred += (longest_int) nread;

				if ((line[0] == '.') && ((line[1] == '\0') || ((line[1] == '.') && ((line[2] == '\0') || (iscntrl(line[2]))))))
					continue;	/* Skip . and .. */

				(void) AddLine(llines, line);
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
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
	return (result);
}	/* FTPListToMemory2 */



int
FTPListToMemory(const FTPCIPtr cip, const char *const pattern, const FTPLineListPtr llines, const char *const lsflags)
{
	return (FTPListToMemory2(cip, pattern, llines, lsflags, 1, (int *) 0));
}	/* FTPListToMemory */
