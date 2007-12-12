/* c_exists.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* We only use STAT to see if files or directories exist.
 * But since it is so rarely used in the wild, we need to
 * make sure the server supports the use where we pass
 * a pathname as a parameter.
 */
int
FTPFileExistsStat(const FTPCIPtr cip, const char *const file)
{
	int result;
	ResponsePtr rp;
	FTPLineList fileList;
	char savedCwd[512];

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (file == NULL)
		return (kErrBadParameter);

	if (cip->STATfileParamWorks == kCommandNotAvailable) {
		cip->errNo = result = kErrSTATwithFileNotAvailable;
		return (result);
	}

	if (cip->STATfileParamWorks == kCommandAvailabilityUnknown) {
		rp = InitResponse();
		if (rp == NULL) {
			result = kErrMallocFailed;
			cip->errNo = kErrMallocFailed;
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			return (result);

		}

		/* First, make sure that when we STAT a pathname
		 * that does not exist, that we get an error back.
		 *
		 * We also assume that a valid STAT response has
		 * at least 3 lines of response text, typically
		 * a "start" line, intermediate data, and then
		 * a trailing line.
		 *
		 * We also can see a one-line case.
		 */
		result = RCmd(cip, rp, "STAT %s", "NoSuchFile");
		if ((result == 2) && ((rp->msg.nLines >= 3) || (rp->msg.nLines == 1))) {
			/* Hmmm.... it gave back a positive
			 * response.  So STAT <file> does not
			 * work correctly.
			 */
			if (
				(rp->msg.first->next != NULL) &&
				(rp->msg.first->next->line != NULL) &&
				(
					(strstr(rp->msg.first->next->line, "o such file") != NULL) ||
					(strstr(rp->msg.first->next->line, "ot found") != NULL)
				)
			) {
				/* OK, while we didn't want a 200
				 * level response, some servers,
				 * like wu-ftpd print an error
				 * message "No such file or
				 * directory" which we can special
				 * case.
				 */
				result = kNoErr;
			} else {
				cip->STATfileParamWorks = kCommandNotAvailable;
				cip->errNo = result = kErrSTATwithFileNotAvailable;
				DoneWithResponse(cip, rp);
				return (result);
			}
		}
		DoneWithResponse(cip, rp);

		/* We can't assume that we can simply say STAT rootdir/firstfile,
		 * since the remote host may not be using / as a directory
		 * delimiter.  So we have to change to the root directory
		 * and then do the STAT on that file.
		 */
		if (
			(FTPGetCWD(cip, savedCwd, sizeof(savedCwd)) != kNoErr) ||
			(FTPChdir(cip, cip->startingWorkingDirectory) != kNoErr)
		) {
			return (cip->errNo);
		}

		/* OK, we get an error when we stat
		 * a non-existant file, but now we need to
		 * see if we get a positive reply when
		 * we stat a file that does exist.
		 *
		 * To do this, we list the root directory,
		 * which we assume has one or more items.
		 * If it doesn't, the user can't do anything
		 * anyway.  Then we stat the first item
		 * we found to see if STAT says it exists.
		 */
		if (
			((result = FTPListToMemory2(cip, "", &fileList, "", 0, (int *) 0)) < 0) ||
			(fileList.last == NULL) ||
			(fileList.last->line == NULL)
		) {
			/* Hmmm... well, in any case we can't use STAT. */
			cip->STATfileParamWorks = kCommandNotAvailable;
			cip->errNo = result = kErrSTATwithFileNotAvailable;
			DisposeLineListContents(&fileList);
			(void) FTPChdir(cip, savedCwd);
			return (result);
		}

		rp = InitResponse();
		if (rp == NULL) {
			result = kErrMallocFailed;
			cip->errNo = kErrMallocFailed;
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			DisposeLineListContents(&fileList);
			(void) FTPChdir(cip, savedCwd);
			return (result);

		}

		result = RCmd(cip, rp, "STAT %s", fileList.last->line);
		DisposeLineListContents(&fileList);

		if ((result != 2) || (rp->msg.nLines == 2)) {
			/* Hmmm.... it gave back a negative
			 * response.  So STAT <file> does not
			 * work correctly.
			 */
			cip->STATfileParamWorks = kCommandNotAvailable;
			cip->errNo = result = kErrSTATwithFileNotAvailable;
			DoneWithResponse(cip, rp);
			(void) FTPChdir(cip, savedCwd);
			return (result);
		} else if (
				(rp->msg.first->next != NULL) &&
				(rp->msg.first->next->line != NULL) &&
				(
					(strstr(rp->msg.first->next->line, "o such file") != NULL) ||
					(strstr(rp->msg.first->next->line, "ot found") != NULL)
				)
		) {
			/* Same special-case of the second line of STAT response. */
			cip->STATfileParamWorks = kCommandNotAvailable;
			cip->errNo = result = kErrSTATwithFileNotAvailable;
			DoneWithResponse(cip, rp);
			(void) FTPChdir(cip, savedCwd);
			return (result);
		}
		DoneWithResponse(cip, rp);
		cip->STATfileParamWorks = kCommandAvailable;

		/* Don't forget to change back to the original directory. */
		(void) FTPChdir(cip, savedCwd);
	}

	rp = InitResponse();
	if (rp == NULL) {
		result = kErrMallocFailed;
		cip->errNo = kErrMallocFailed;
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		return (result);
	}

	result = RCmd(cip, rp, "STAT %s", file);
	if (result == 2) {
		result = kNoErr;
		if (((rp->msg.nLines >= 3) || (rp->msg.nLines == 1))) {
			if (
				(rp->msg.first->next != NULL) &&
				(rp->msg.first->next->line != NULL) &&
				(
					(strstr(rp->msg.first->next->line, "o such file") != NULL) ||
					(strstr(rp->msg.first->next->line, "ot found") != NULL)
				)
			) {
				cip->errNo = kErrSTATFailed;
				result = kErrSTATFailed;
			} else {
				result = kNoErr;
			}
		} else if (rp->msg.nLines == 2) {
			cip->errNo = kErrSTATFailed;
			result = kErrSTATFailed;
		} else {
			result = kNoErr;
		}
	} else {
		cip->errNo = kErrSTATFailed;
		result = kErrSTATFailed;
	}
	DoneWithResponse(cip, rp);
	return (result);
}	/* FTPFileExistsStat */




/* We only use STAT to see if files or directories exist.
 * But since it is so rarely used in the wild, we need to
 * make sure the server supports the use where we pass
 * a pathname as a parameter.
 */
int
FTPFileExistsNlst(const FTPCIPtr cip, const char *const file)
{
	int result;
	FTPLineList fileList, rootFileList;
	char savedCwd[512];
	int createdTempFile;
#define kFTPFileExistsNlstTestMessage "This file was created by an FTP client program using the LibNcFTP toolkit.  A temporary file needed to be created to ensure that this directory was not empty, so that the directory could be listed with the guarantee of at least one file in the listing.\r\n\r\nYou may delete this file manually if your FTP client program does not delete it for you.\r\n"
	const char *testFileMessage = kFTPFileExistsNlstTestMessage; 
#undef kFTPFileExistsNlstTestMessage
	const char *testFileName = "testfile.ftp";

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (file == NULL)
		return (kErrBadParameter);

	if (cip->NLSTfileParamWorks == kCommandNotAvailable) {
		cip->errNo = result = kErrNLSTwithFileNotAvailable;
		return (result);
	}

	if (cip->NLSTfileParamWorks == kCommandAvailabilityUnknown) {
		/* First, make sure that when we NLST a pathname
		 * that does not exist, that we get an error back.
		 *
		 * We also assume that a valid NLST response has
		 * at least 3 lines of response text, typically
		 * a "start" line, intermediate data, and then
		 * a trailing line.
		 *
		 * We also can see a one-line case.
		 */
		if (
			((FTPListToMemory2(cip, "NoSuchFile", &fileList, "", 0, (int *) 0)) == kNoErr) &&
			(fileList.nLines >= 1) &&
			(strstr(fileList.last->line, "o such file") == NULL) &&
			(strstr(fileList.last->line, "ot found") == NULL) &&
			(strstr(fileList.last->line, "o Such File") == NULL) &&
			(strstr(fileList.last->line, "ot Found") == NULL)

		) {
			cip->NLSTfileParamWorks = kCommandNotAvailable;
			cip->errNo = result = kErrNLSTwithFileNotAvailable;
			DisposeLineListContents(&fileList);
			return (result);
		}
		DisposeLineListContents(&fileList);

		/* We can't assume that we can simply say NLST rootdir/firstfile,
		 * since the remote host may not be using / as a directory
		 * delimiter.  So we have to change to the root directory
		 * and then do the NLST on that file.
		 */
		if (
			(FTPGetCWD(cip, savedCwd, sizeof(savedCwd)) != kNoErr) ||
			(FTPChdir(cip, cip->startingWorkingDirectory) != kNoErr)
		) {
			return (cip->errNo);
		}

		/* OK, we get an error when we list
		 * a non-existant file, but now we need to
		 * see if we get a positive reply when
		 * we stat a file that does exist.
		 *
		 * To do this, we list the root directory,
		 * which we assume has one or more items.
		 * If it doesn't, the user can't do anything
		 * anyway.  Then we do the first item
		 * we found to see if NLST says it exists.
		 */
		createdTempFile = 0;
		if (
			((result = FTPListToMemory2(cip, "", &rootFileList, "", 0, (int *) 0)) < 0) ||
			(rootFileList.last == NULL) ||
			(rootFileList.last->line == NULL)
		) {
			if (AddLine(&rootFileList, testFileName) != NULL) {
				/* Hate to do this, but if the directory
				 * is empty but writable (i.e. a $HOME)
				 * then we can still continue.
				 */
				if (FTPPutFileFromMemory(cip, testFileName, testFileMessage, strlen(testFileMessage), kAppendNo) == kNoErr) {
					createdTempFile = 1;
				}
			}
			if (createdTempFile == 0) {
				/* Hmmm... well, in any case we can't use NLST. */
				cip->NLSTfileParamWorks = kCommandNotAvailable;
				cip->errNo = result = kErrNLSTwithFileNotAvailable;
				DisposeLineListContents(&rootFileList);
				(void) FTPChdir(cip, savedCwd);
				return (result);
			}
		}

		if (
			((FTPListToMemory2(cip, rootFileList.last->line, &fileList, "", 0, (int *) 0)) == kNoErr) &&
			(fileList.nLines >= 1) &&
			(strstr(fileList.last->line, "o such file") == NULL) &&
			(strstr(fileList.last->line, "ot found") == NULL) &&
			(strstr(fileList.last->line, "o Such File") == NULL) &&
			(strstr(fileList.last->line, "ot Found") == NULL)

		) {
			/* Good.  We listed the item. */
			if (createdTempFile != 0)
				(void) FTPDelete(cip, testFileName, kRecursiveNo, kGlobNo);
			DisposeLineListContents(&fileList);
			DisposeLineListContents(&rootFileList);
			cip->NLSTfileParamWorks = kCommandAvailable;

			/* Don't forget to change back to the original directory. */
			(void) FTPChdir(cip, savedCwd);
		} else {
			if (createdTempFile != 0)
				(void) FTPDelete(cip, testFileName, kRecursiveNo, kGlobNo);
			cip->NLSTfileParamWorks = kCommandNotAvailable;
			cip->errNo = result = kErrNLSTwithFileNotAvailable;
			DisposeLineListContents(&fileList);
			DisposeLineListContents(&rootFileList);
			(void) FTPChdir(cip, savedCwd);
			return (result);
		}
	}

	/* Check the requested item. */
	InitLineList(&fileList);
	if (
		((FTPListToMemory2(cip, file, &fileList, "", 0, (int *) 0)) == kNoErr) &&
		(fileList.nLines >= 1) &&
		(strstr(fileList.last->line, "o such file") == NULL) &&
		(strstr(fileList.last->line, "ot found") == NULL) &&
		(strstr(fileList.last->line, "o Such File") == NULL) &&
		(strstr(fileList.last->line, "ot Found") == NULL)

	) {
		/* The item existed. */
		result = kNoErr;
	} else {
		cip->errNo = kErrNLSTFailed;
		result = kErrNLSTFailed;
	}

	DisposeLineListContents(&fileList);
	return (result);
}	/* FTPFileExistsNlst*/




/* This functions goes to a great deal of trouble to try and determine if the
 * remote file specified exists.  Newer servers support commands that make
 * it relatively inexpensive to find the answer, but older servers do not
 * provide a standard way.  This means we may try a whole bunch of things,
 * but the good news is that the library saves information about which things
 * worked so if you do this again it uses the methods that work.
 */
int
FTPFileExists2(const FTPCIPtr cip, const char *const file, const int tryMDTM, const int trySIZE, const int tryMLST, const int trySTAT, const int tryNLST)
{
	int result;
	time_t mdtm;
	longest_int size;
	MLstItem mlsInfo;

	if (tryMDTM != 0) {
		result = FTPFileModificationTime(cip, file, &mdtm);
		if (result == kNoErr)
			return (kNoErr);
		if (result == kErrMDTMFailed) {
			cip->errNo = kErrNoSuchFileOrDirectory;
			return (kErrNoSuchFileOrDirectory);
		}
		/* else keep going */	
	}

	if (trySIZE != 0) {
		result = FTPFileSize(cip, file, &size, kTypeBinary);
		if (result == kNoErr)
			return (kNoErr);
		/* SIZE could fail if the server does
		 * not support it for directories.
		 *
		 * if (result == kErrSIZEFailed)
		 *	return (kErrNoSuchFileOrDirectory);
		 */
		/* else keep going */	
	}


	if (tryMLST != 0) {
		result = FTPMListOneFile(cip, file, &mlsInfo);
		if (result == kNoErr)
			return (kNoErr);
		if (result == kErrMLSTFailed) {
			cip->errNo = kErrNoSuchFileOrDirectory;
			return (kErrNoSuchFileOrDirectory);
		}
		/* else keep going */	
	}

	if (trySTAT != 0) {
		result = FTPFileExistsStat(cip, file);
		if (result == kNoErr)
			return (kNoErr);
		if (result == kErrSTATFailed) {
			cip->errNo = kErrNoSuchFileOrDirectory;
			return (kErrNoSuchFileOrDirectory);
		}
		/* else keep going */	
	}

	if (tryNLST != 0) {
		result = FTPFileExistsNlst(cip, file);
		if (result == kNoErr)
			return (kNoErr);
		if (result == kErrNLSTFailed) {
			cip->errNo = kErrNoSuchFileOrDirectory;
			return (kErrNoSuchFileOrDirectory);
		}
		/* else keep going */	
	}

	cip->errNo = kErrCantTellIfFileExists;
	return (kErrCantTellIfFileExists);
}	/* FTPFileExists2 */




int
FTPFileExists(const FTPCIPtr cip, const char *const file)
{
	return (FTPFileExists2(cip, file, 1, 1, 1, 1, 1));
}	/* FTPFileExists */
