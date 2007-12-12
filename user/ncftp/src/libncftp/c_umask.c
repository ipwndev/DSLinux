/* c_umask.c
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
FTPUmask(const FTPCIPtr cip, const char *const umsk)
{
	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	if ((umsk == NULL) || (umsk[0] == '\0'))
		return (kErrBadParameter);
	if (FTPCmd(cip, "SITE UMASK %s", umsk) == 2)
		return (kNoErr);
	cip->errNo = kErrUmaskFailed;
	return (kErrUmaskFailed);
}	/* FTPUmask */
