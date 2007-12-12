/* c_filetype.c
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
FTPFileType(const FTPCIPtr cip, const char *const file, int *const ftype)
{
	int result;
	MLstItem mlsInfo;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((file == NULL) || (file[0] == '\0')) {
		cip->errNo = kErrBadParameter;
		return (kErrBadParameter);
	}

	if (ftype == NULL) {
		cip->errNo = kErrBadParameter;
		return (kErrBadParameter);
	}

	*ftype = 0;
	result = FTPMListOneFile(cip, file, &mlsInfo);
	if (result == kNoErr) {
		*ftype = mlsInfo.ftype;
		return (kNoErr);
	}

	/* Preserve old working directory. */
	(void) FTPGetCWD(cip, cip->buf, cip->bufSize);

	result = FTPChdir(cip, file);
	if (result == kNoErr) {
		*ftype = 'd';
		/* Yes it was a directory, now go back to
		 * where we were.
		 */
		(void) FTPChdir(cip, cip->buf);

		/* Note:  This improperly assumes that we
		 * will be able to chdir back, which is
		 * not guaranteed.
		 */
		return (kNoErr);
	}

	result = FTPFileExists2(cip, file, 1, 1, 0, 1, 1);
	if (result != kErrNoSuchFileOrDirectory)
		result = kErrFileExistsButCannotDetermineType;

	return (result);
}	/* FTPFileType */




int
FTPIsDir(const FTPCIPtr cip, const char *const dir)
{
	int result, ftype;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((dir == NULL) || (dir[0] == '\0')) {
		cip->errNo = kErrInvalidDirParam;
		return (kErrInvalidDirParam);
	}

	result = FTPFileType(cip, dir, &ftype);
	if ((result == kNoErr) || (result == kErrFileExistsButCannotDetermineType)) {
		result = 0;
		if (ftype == 'd')
			result = 1;
	}
	return (result);
}	/* FTPIsDir */




int
FTPIsRegularFile(const FTPCIPtr cip, const char *const file)
{
	int result, ftype;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((file == NULL) || (file[0] == '\0')) {
		cip->errNo = kErrBadParameter;
		return (kErrBadParameter);
	}

	result = FTPFileType(cip, file, &ftype);
	if ((result == kNoErr) || (result == kErrFileExistsButCannotDetermineType)) {
		result = 1;
		if (ftype == 'd')
			result = 0;
	}
	return (result);
}	/* FTPIsRegularFile */
