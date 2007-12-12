/* io_getfiles.c
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
FTPGetFiles3(
	const FTPCIPtr cip,
	const char *pattern1,
	const char *const dstdir1,
	const int recurse,
	int doGlob,
	const int xtype,
	const int resumeflag,
	int appendflag,
	const int deleteflag,
	const int tarflag,
	const FTPConfirmResumeDownloadProc resumeProc,
	int UNUSED(reserved))
{
	FTPLineList globList;
	FTPLinePtr itemPtr;
	FTPFileInfoList files;
	FTPFileInfoPtr filePtr;
	int batchResult;
	int result;
	char *ldir;
	char *cp;
	const char *dstdir;
	const char *pattern;
	char *pattern2, *dstdir2;
	char c;
	int recurse1;
	int errRc;

	LIBNCFTP_USE_VAR(reserved);
	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	if (pattern1 == NULL)
		return (kErrBadParameter);

	dstdir2 = NULL;
	pattern2 = NULL;

	if (dstdir1 == NULL) {
		dstdir = NULL;
	} else {
		dstdir2 = StrDup(dstdir1);
		if (dstdir2 == NULL) {
			errRc = kErrMallocFailed;
			goto return_err;
		}
		StrRemoveTrailingLocalPathDelim(dstdir2);
		dstdir = dstdir2;
	}

	pattern2 = StrDup(pattern1);
	if (pattern2 == NULL) {
		errRc = kErrMallocFailed;
		goto return_err;
	}
	StrRemoveTrailingSlashes(pattern2);
	pattern = pattern2;

	if (pattern[0] == '\0') {
		if (recurse == kRecursiveNo) {
			errRc = kErrBadParameter;
			goto return_err;
		}
		pattern = ".";
		doGlob = kGlobNo;
	} else if (strcmp(pattern, ".") == 0) {
		if (recurse == kRecursiveNo) {
			errRc = kErrBadParameter;
			goto return_err;
		}
		doGlob = kGlobNo;
	} else if ((strcmp(pattern, "/") == 0) && ((strcmp(dstdir, ".") == 0) || (dstdir[0] == '\0'))) {
		/* Ick... but works around a problem we need to fix */
		free(pattern2);
		pattern2 = StrDup("/.");
		pattern = pattern2;
		/*
		errRc = kErrKnownBug;
		goto return_err;
		*/
	}
	if (recurse == kRecursiveYes)
		appendflag = kAppendNo;

	batchResult = FTPRemoteGlob(cip, &globList, pattern, doGlob);
	if (batchResult != kNoErr) {
		errRc = batchResult;
		goto return_err;
	}

	cip->cancelXfer = 0;	/* should already be zero */

	for (itemPtr = globList.first; itemPtr != NULL; itemPtr = itemPtr->next) {
		if ((recurse == kRecursiveYes) && (FTPIsDir(cip, itemPtr->line) > 0)) {
#ifdef TAR
			if ((tarflag == kTarYes) && (xtype == kTypeBinary) && (appendflag == kAppendNo) && (deleteflag == kDeleteNo) && (FTPGetOneTarF(cip, itemPtr->line, dstdir) == kNoErr)) {
				/* Great! */
				continue;
			}
#endif	/* TAR */
			(void) FTPRemoteRecursiveFileList1(cip, itemPtr->line, &files);
			(void) ComputeLNames(&files, itemPtr->line, dstdir, 1);
			recurse1 = recurse;
		} else {
			recurse1 = kRecursiveNo;
			(void) LineToFileInfoList(itemPtr, &files);
			(void) ComputeRNames(&files, ".", 0, 1);
			(void) ComputeLNames(&files, NULL, dstdir, 0);
		}
		if (cip->cancelXfer > 0) {
			DisposeFileInfoListContents(&files);
			break;
		}

#if 0
		for (filePtr = files.first; filePtr != NULL; filePtr = filePtr->next) {
			PrintF(cip, "  R=%s, L=%s, 2=%s, size=%lld, mdtm=%u, type=%c\n",
				filePtr->rname,
				filePtr->lname,
				filePtr->rlinkto ? filePtr->rlinkto : "",
				(longest_int) filePtr->size,
				(unsigned int) filePtr->mdtm,
				filePtr->type
			);
		}
#endif


		for (filePtr = files.first; filePtr != NULL; filePtr = filePtr->next) {
			if (cip->connected == 0) {
				if (batchResult == kNoErr)
					batchResult = kErrRemoteHostClosedConnection;
				break;
			}
			if (filePtr->type == 'd') {
				(void) MkDirs(filePtr->lname, 00777);
			} else if (filePtr->type == 'l') {
				/* skip it -- we do that next pass. */
			} else if (recurse1 != kRecursiveYes) {
				result = FTPGetOneF(cip, filePtr->rname, filePtr->lname, xtype, -1, filePtr->size, filePtr->mdtm, resumeflag, appendflag, deleteflag, resumeProc);
				if (files.nFileInfos == 1) {
					if (result != kNoErr)
						batchResult = result;
				} else {
					if ((result != kNoErr) && (result != kErrLocalFileNewer) && (result != kErrRemoteFileNewer) && (result != kErrLocalSameAsRemote))
						batchResult = result;
				}
				if (result == kErrUserCanceled)
					cip->cancelXfer = 1;
				if (cip->cancelXfer > 0)
					break;
			} else {
				ldir = filePtr->lname;
				cp = StrRFindLocalPathDelim(ldir);
				if (cp != NULL) {
					while (cp >= ldir) {
						if (! IsLocalPathDelim(*cp)) {
							++cp;
							break;
						}
						--cp;
					}
					if (cp > ldir) {
						c = *cp;
						*cp = '\0';
						if (MkDirs(ldir, 00777) < 0) {
							FTPLogError(cip, kDoPerror, "Could not create local directory \"%s\"\n", ldir);
							batchResult = kErrGeneric;
							*cp = c;
							continue;
						}
						*cp = c;
					}
				}
				if (xtype == kTypeAscii) {
					/* Make sure we got the SIZE from
					 * a SIZE command, and not a
					 * directory listing, which may
					 * not have taken into the account
					 * the required end-of-line format
					 * for text files sent over FTP.
					 */
					if ((resumeflag == kResumeYes) || (resumeProc != kNoFTPConfirmResumeDownloadProc)) {
						FTPCheckForRestartModeAvailability(cip); 
					}
					result = FTPSetTransferType(cip, xtype);
					if (result < 0)
						return (result);
					(void) FTPFileSize(cip, filePtr->rname, &filePtr->size, xtype);
				}
				result = FTPGetOneF(cip, filePtr->rname, filePtr->lname, xtype, -1, filePtr->size, filePtr->mdtm, resumeflag, appendflag, deleteflag, resumeProc);

				if (files.nFileInfos == 1) {
					if (result != kNoErr)
						batchResult = result;
				} else {
					if ((result != kNoErr) && (result != kErrLocalFileNewer) && (result != kErrRemoteFileNewer) && (result != kErrLocalSameAsRemote))
						batchResult = result;
				}
				if (result == kErrUserCanceled)
					cip->cancelXfer = 1;
				if (cip->cancelXfer > 0)
					break;
			}
		}
		if (cip->cancelXfer > 0) {
			DisposeFileInfoListContents(&files);
			break;
		}

#ifdef HAVE_SYMLINK
		for (filePtr = files.first; filePtr != NULL; filePtr = filePtr->next) {
			if (filePtr->type == 'l') {
				(void) unlink(filePtr->lname);
				if (symlink(filePtr->rlinkto, filePtr->lname) < 0) {
					FTPLogError(cip, kDoPerror, "Could not symlink %s to %s\n", filePtr->rlinkto, filePtr->lname);
					/* Note: not worth setting batchResult */
				}
			}
		}
#endif	/* HAVE_SYMLINK */


		DisposeFileInfoListContents(&files);
	}

	DisposeLineListContents(&globList);
	if (batchResult < 0)
		cip->errNo = batchResult;
	errRc = batchResult;

return_err:
	if (dstdir2 != NULL)
		free(dstdir2);
	if (pattern2 != NULL)
		free(pattern2);
	return (errRc);
}	/* FTPGetFiles3 */




int
FTPGetFiles(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob)
{
	return (FTPGetFiles3(cip, pattern, dstdir, recurse, doGlob, kTypeBinary, kResumeNo, kAppendNo, kDeleteNo, kTarYes, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetFiles */




int
FTPGetFiles2(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob, const int xtype, const int resumeflag, const int appendflag)
{
	return (FTPGetFiles3(cip, pattern, dstdir, recurse, doGlob, xtype, resumeflag, appendflag, kDeleteNo, kTarYes, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetFiles2 */




int
FTPGetFilesAscii(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob)
{
	return (FTPGetFiles3(cip, pattern, dstdir, recurse, doGlob, kTypeAscii, kResumeNo, kAppendNo, kDeleteNo, kTarNo, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetFilesAscii */
