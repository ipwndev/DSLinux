/* c_rmdirr.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

static int
FTPRmdirRecursiveL2(const FTPCIPtr cip)
{
	FTPLineList fileList;
	FTPLinePtr filePtr;
	char *file;
	int result;

	result = FTPRemoteGlob(cip, &fileList, "**", kGlobYes);
	if (result != kNoErr) {
		return (result);
	}

	for (filePtr = fileList.first;
		filePtr != NULL;
		filePtr = filePtr->next)
	{
		file = filePtr->line;
		if (file == NULL) {
			cip->errNo = kErrBadLineList;
			break;
		}

		if ((file[0] == '.') && ((file[1] == '\0') || ((file[1] == '.') && (file[2] == '\0'))))
			continue;	/* Skip . and .. */

		if (FTPChdir(cip, file) == kNoErr) {
			/* It was a directory.
			 * Go in and wax it.
			 */
			result = FTPRmdirRecursiveL2(cip);

			if (FTPChdir(cip, "..") != kNoErr) {
				/* Panic -- we can no longer
				 * cd back to the directory
				 * we were in before.
				 */
				result = kErrCannotGoToPrevDir;
				cip->errNo = kErrCannotGoToPrevDir;
				return (result);
			}

			if ((result < 0) && (result != kErrGlobNoMatch))
				return (result);

			result = FTPRmdir(cip, file, kRecursiveNo, kGlobNo);
			if (result != kNoErr) {
				/* Well, we couldn't remove the empty
				 * directory.  Perhaps we screwed up
				 * and the directory wasn't empty.
				 */
				return (result);
			}
		} else {
			/* Assume it was a file -- remove it. */
			result = FTPDelete(cip, file, kRecursiveNo, kGlobNo);
			/* Try continuing to remove the rest,
			 * even if this failed.
			 */
		}
	}
	DisposeLineListContents(&fileList);

	return (result);
} 	/* FTPRmdirRecursiveL2 */



int
FTPRmdirRecursive(const FTPCIPtr cip, const char *const dir)
{
	int result, result2;

	/* Preserve old working directory. */
	(void) FTPGetCWD(cip, cip->buf, cip->bufSize);

	result = FTPChdir(cip, dir);
	if (result != kNoErr) {
		return (result);
	}

	result = FTPRmdirRecursiveL2(cip);

	if (FTPChdir(cip, cip->buf) != kNoErr) {
		/* Could not cd back to the original user directory -- bad. */
		if (result != kNoErr) {
			result = kErrCannotGoToPrevDir;
			cip->errNo = kErrCannotGoToPrevDir;
		}
		return (result);
	}

	/* Now rmdir the last node, the root of the tree
	 * we just went through.
	 */
	result2 = FTPRmdir(cip, dir, kRecursiveNo, kGlobNo);
	if ((result2 != kNoErr) && (result == kNoErr))
		result = result2;

	return (result);
}	/* FTPRmdirRecursive */
