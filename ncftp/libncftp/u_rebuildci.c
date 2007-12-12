/* u_rebuildci.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
FTPRebuildConnectionInfo(const FTPLIPtr lip, const FTPCIPtr cip)
{
	char *buf;

	cip->lip = lip;
	cip->debugLog = NULL;
	cip->errLog = NULL;
	cip->debugLogProc = NULL;
	cip->errLogProc = NULL;
	cip->buf = NULL;
	cip->cin = NULL;
	cip->cout = NULL;
	cip->errNo = 0;
	cip->progress = NULL;
	cip->rname = NULL;
	cip->lname = NULL;
	cip->onConnectMsgProc = NULL;
	cip->redialStatusProc = NULL;
	cip->printResponseProc = NULL;
	cip->onLoginMsgProc = NULL;
	cip->passphraseProc = NULL;
	cip->startingWorkingDirectory = NULL;
	cip->asciiFilenameExtensions = NULL;

	(void) memset(&cip->lastFTPCmdResultLL, 0, sizeof(FTPLineList));

	/* Allocate a new buffer. */
	buf = (char *) calloc((size_t) 1, cip->bufSize);
	if (buf == NULL) {
		cip->errNo = kErrMallocFailed;
		return (kErrMallocFailed);
	}
	cip->buf = buf;

	/* Reattach the FILE pointers for use with the Std I/O library
	 * routines.
	 */
	if ((cip->cin = fdopen(cip->ctrlSocketR, "r")) == NULL) {
		cip->errNo = kErrFdopenR;
		cip->ctrlSocketR = kClosedFileDescriptor;
		cip->ctrlSocketW = kClosedFileDescriptor;
		return (kErrFdopenR);
	}

	if ((cip->cout = fdopen(cip->ctrlSocketW, "w")) == NULL) {
		CloseFile(&cip->cin);
		cip->errNo = kErrFdopenW;
		cip->ctrlSocketR = kClosedFileDescriptor;
		cip->ctrlSocketW = kClosedFileDescriptor;
		return (kErrFdopenW);
	}

#if USE_SIO
	if (InitSReadlineInfo(&cip->ctrlSrl, cip->ctrlSocketR, cip->srlBuf, sizeof(cip->srlBuf), (int) cip->ctrlTimeout, 1) < 0) {
		cip->errNo = kErrFdopenW;
		CloseFile(&cip->cin);
		cip->errNo = kErrFdopenW;
		cip->ctrlSocketR = kClosedFileDescriptor;
		cip->ctrlSocketW = kClosedFileDescriptor;
		return (kErrFdopenW);
	}
#endif
	return (kNoErr);
}	/* FTPRebuildConnectionInfo */
