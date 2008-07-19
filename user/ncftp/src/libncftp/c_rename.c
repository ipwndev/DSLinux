/* c_rename.c
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
FTPRename(const FTPCIPtr cip, const char *const oldname, const char *const newname)
{
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	if ((oldname == NULL) || (oldname[0] == '\0'))
		return (kErrBadParameter);
	if ((newname == NULL) || (oldname[0] == '\0'))
		return (kErrBadParameter);

	
	result = FTPCmd(cip, "RNFR %s", oldname);
	if (result < 0)
		return (result);
	if (result != 3) {
		cip->errNo = kErrRenameFailed;
		return (cip->errNo);
	}
	
	result = FTPCmd(cip, "RNTO %s", newname);
	if (result < 0)
		return (result);
	if (result != 2) {
		cip->errNo = kErrRenameFailed;
		return (cip->errNo);
	}
	return (kNoErr);
}	/* FTPRename */
