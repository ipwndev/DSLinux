/* c_rmdir.c
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
FTPRmdir(const FTPCIPtr cip, const char *const pattern, const int recurse, const int doGlob)
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
		onceResult = FTPCmd(cip, "RMD %s", file); 	
		if (onceResult < 0) {
			batchResult = onceResult;
			break;
		}
		if (onceResult != 2) {
			if (recurse == kRecursiveYes) {
				onceResult = FTPRmdirRecursive(cip, file);
				if (onceResult < 0) {
					batchResult = kErrRMDFailed;
					cip->errNo = kErrRMDFailed;
				}
			} else {
				batchResult = kErrRMDFailed;
				cip->errNo = kErrRMDFailed;
			}
		}
	}
	DisposeLineListContents(&fileList);
	return (batchResult);
}	/* FTPRmdir */
