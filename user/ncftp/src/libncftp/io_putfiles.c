/* io_putfiles.c
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
FTPPutFiles3(
	const FTPCIPtr cip,
	const char *const pattern,
	const char *const dstdir1,
	const int recurse,
	const int doGlob,
	const int xtype,
	int appendflag,
	const char *const tmppfx,
	const char *const tmpsfx,
	const int resumeflag,
	const int deleteflag,
	const FTPConfirmResumeUploadProc resumeProc,
	int UNUSED(reserved))
{
	FTPLineList globList;
	FTPFileInfoList files;
	FTPFileInfoPtr filePtr;
	int batchResult;
	int result;
	const char *dstdir;
	char dstdir2[512];

	LIBNCFTP_USE_VAR(reserved);
	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (dstdir1 == NULL) {
		dstdir = NULL;
	} else {
		dstdir = STRNCPY(dstdir2, dstdir1);
		StrRemoveTrailingLocalPathDelim(dstdir2);
	}

	(void) FTPLocalGlob(cip, &globList, pattern, doGlob);
	if (recurse == kRecursiveYes) {
		appendflag = kAppendNo;
		(void) FTPLocalRecursiveFileList(cip, &globList, &files);
		if (files.first == NULL) {
			cip->errNo = kErrNoValidFilesSpecified;
			return (kErrNoValidFilesSpecified);
		}
		(void) ComputeRNames(&files, dstdir, 0, 1);
	} else {
		(void) LineListToFileInfoList(&globList, &files);
		(void) ComputeLNames(&files, NULL, NULL, 1);
		(void) ComputeRNames(&files, dstdir, 0, 0);
	}
	DisposeLineListContents(&globList);

#if 0
	for (filePtr = files.first; filePtr != NULL; filePtr = filePtr->next) {
		PrintF(cip, "  R=%s, L=%s, 2=%s, size=%lld, mdtm=%u, type=%c\n",
			filePtr->rname,
			filePtr->lname,
			filePtr->rlinkto ? filePtr->rlinkto : "",
			filePtr->size,
			(unsigned int) filePtr->mdtm,
			filePtr->type
		);
	}
#endif

	batchResult = kNoErr;
	for (filePtr = files.first; filePtr != NULL; filePtr = filePtr->next) {
		if (cip->connected == 0) {
			if (batchResult == kNoErr)
				batchResult = kErrRemoteHostClosedConnection;
			break;
		}
		if (filePtr->type == 'd') {
			/* mkdir */
			StrRemoveTrailingLocalPathDelim(filePtr->rname);
			result = FTPMkdir(cip, filePtr->rname, kRecursiveNo);
			if (result != kNoErr)
				batchResult = result;
#ifdef HAVE_SYMLINK
		} else if (filePtr->type == 'l') {
			/* symlink */
			/* no RFC way to create the link, though. */
			if ((filePtr->rlinkto != NULL) && (filePtr->rlinkto[0] != '\0'))
				(void) FTPSymlink(cip, filePtr->rname, filePtr->rlinkto);
#endif
		} else if (recurse != kRecursiveYes) {
			result = FTPPutOneF(cip, filePtr->lname, filePtr->rname, xtype, -1, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, resumeProc);
			if (files.nFileInfos == 1) {
				if (result != kNoErr)
					batchResult = result;
			} else {
				if ((result != kNoErr) && (result != kErrLocalFileNewer) && (result != kErrRemoteFileNewer) && (result != kErrRemoteSameAsLocal))
					batchResult = result;
			}
			if (result == kErrUserCanceled)
				cip->cancelXfer = 1;
			if (cip->cancelXfer > 0)
				break;
		} else {
			result = FTPPutOneF(cip, filePtr->lname, filePtr->rname, xtype, -1, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, resumeProc);
			if (files.nFileInfos == 1) {
				if (result != kNoErr)
					batchResult = result;
			} else {
				if ((result != kNoErr) && (result != kErrLocalFileNewer) && (result != kErrRemoteFileNewer) && (result != kErrRemoteSameAsLocal))
					batchResult = result;
			}
			if (result == kErrUserCanceled)
				cip->cancelXfer = 1;
			if (cip->cancelXfer > 0)
				break;
		}
	}
	DisposeFileInfoListContents(&files);
	if (batchResult < 0)
		cip->errNo = batchResult;
	return (batchResult);
}	/* FTPPutFiles3 */




int
FTPPutFiles(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob)
{
	return (FTPPutFiles3(cip, pattern, dstdir, recurse, doGlob, kTypeBinary, 0, NULL, NULL, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutFiles */




int
FTPPutFiles2(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob, const int xtype, const int appendflag, const char *const tmppfx, const char *const tmpsfx)
{
	return (FTPPutFiles3(cip, pattern, dstdir, recurse, doGlob, xtype, appendflag, tmppfx, tmpsfx, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutFiles2 */




int
FTPPutFilesAscii(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob)
{
	return (FTPPutFiles3(cip, pattern, dstdir, recurse, doGlob, kTypeAscii, 0, NULL, NULL, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutFilesAscii */
