/* c_chmod.c
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
FTPChmod(const FTPCIPtr cip, const char *const pattern, const char *const mode, const int doGlob)
{
	FTPLineList fileList;
	FTPLinePtr filePtr;
	char *file;
	int onceResult, batchResult;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	batchResult = FTPRemoteGlob(cip, &fileList, pattern, doGlob);
	if (batchResult != kNoErr)
		return (batchResult);

	for (batchResult = kNoErr, filePtr = fileList.first;
		filePtr != NULL;
		filePtr = filePtr->next)
	{
		file = filePtr->line;
		if (file == NULL) {
			batchResult = kErrBadLineList;
			cip->errNo = kErrBadLineList;
			break;
		}
		onceResult = FTPCmd(cip, "SITE CHMOD %s %s", mode, file); 	
		if (onceResult < 0) {
			batchResult = onceResult;
			break;
		}
		if (onceResult != 2) {
			batchResult = kErrChmodFailed;
			cip->errNo = kErrChmodFailed;
		}
	}
	DisposeLineListContents(&fileList);
	return (batchResult);
}	/* FTPChmod */
