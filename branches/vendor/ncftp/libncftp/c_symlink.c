/* c_symlink.c
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
FTPSymlink(const FTPCIPtr cip, const char *const lfrom, const char *const lto)
{
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	if ((cip == NULL) || (lfrom == NULL) || (lto == NULL))
		return (kErrBadParameter);
	if ((lfrom[0] == '\0') || (lto[0] == '\0'))
		return (kErrBadParameter);
	if (FTPCmd(cip, "SITE SYMLINK %s %s", lfrom, lto) == 2)
		return (kNoErr);
	cip->errNo = kErrSYMLINKFailed;
	return (kErrSYMLINKFailed);
}	/* FTPSymlink */
