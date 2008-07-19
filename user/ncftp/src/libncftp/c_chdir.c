/* c_chdir.c
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
FTPChdir(const FTPCIPtr cip, const char *const cdCwd)
{
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cdCwd == NULL) {
		result = kErrInvalidDirParam;
		cip->errNo = kErrInvalidDirParam;
	} else {
		if (cdCwd[0] == '\0')	/* But allow FTPChdir(cip, ".") to go through. */
			result = 2;	
		else if (strcmp(cdCwd, "..") == 0)
			result = FTPCmd(cip, "CDUP");
		else
			result = FTPCmd(cip, "CWD %s", cdCwd);
		if (result >= 0) {
			if (result == 2) {
				result = kNoErr;
			} else {
				result = kErrCWDFailed;
				cip->errNo = kErrCWDFailed;
			}
		}
	}
	return (result);
}	/* FTPChdir */
