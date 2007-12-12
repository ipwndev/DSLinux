/* c_sizemdtm.c
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
FTPFileSizeAndModificationTime(const FTPCIPtr cip, const char *const file, longest_int *const size, const int type, time_t *const mdtm)
{
	MLstItem mlsInfo;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((mdtm == NULL) || (size == NULL) || (file == NULL))
		return (kErrBadParameter);

	*mdtm = kModTimeUnknown;
	*size = kSizeUnknown;

	result = FTPSetTransferType(cip, type);
	if (result < 0)
		return (result);

	result = FTPMListOneFile(cip, file, &mlsInfo);
	if (result < 0) {
		/* Do it the regular way, where
		 * we do a SIZE and then a MDTM.
		 */
		result = FTPFileSize(cip, file, size, type);
		if (result < 0)
			return (result);
		result = FTPFileModificationTime(cip, file, mdtm);
		return (result);
	} else {
		*mdtm = mlsInfo.ftime;
		*size = mlsInfo.fsize;
	}

	return (result);
}	/* FTPFileSizeAndModificationTime */
