/* io_putonefile.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define ASCII_TRANSLATION 0
#endif

#ifndef ASCII_TRANSLATION
#	define ASCII_TRANSLATION 1
#endif

#ifndef NO_SIGNALS
#	define NO_SIGNALS 1
#endif

#ifndef O_BINARY
	/* Needed for platforms using different EOLN sequence (i.e. DOS) */
#	ifdef _O_BINARY
#		define O_BINARY _O_BINARY
#	else
#		define O_BINARY 0
#	endif
#endif

int
FTPPutOneFile3(
	const FTPCIPtr cip,
	const char *const file,
	const char *const dstfile,
	const int xtype,
	const int fdtouse,
	const int appendflag,
	const char *const tmppfx,
	const char *const tmpsfx,
	const int resumeflag,
	const int deleteflag,
	const FTPConfirmResumeUploadProc resumeProc,
	int UNUSED(reserved))
{
	int result;

	LIBNCFTP_USE_VAR(reserved);
	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	
	if ((dstfile == NULL) || (dstfile[0] == '\0'))
		return (kErrBadParameter);
	if (fdtouse < 0) {
		if ((file == NULL) || (file[0] == '\0'))
			return (kErrBadParameter);
	}
	result = FTPPutOneF(cip, file, dstfile, xtype, fdtouse, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, resumeProc);
	return (result);
}	/* FTPPutOneFile3 */




int
FTPPutOneFile(const FTPCIPtr cip, const char *const file, const char *const dstfile)
{
	return (FTPPutOneFile3(cip, file, dstfile, kTypeBinary, -1, 0, NULL, NULL, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutOneFile */




int
FTPPutOneFile2(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int appendflag, const char *const tmppfx, const char *const tmpsfx)
{
	return (FTPPutOneFile3(cip, file, dstfile, xtype, fdtouse, appendflag, tmppfx, tmpsfx, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutOneFile2 */




int
FTPPutOneFileAscii(const FTPCIPtr cip, const char *const file, const char *const dstfile)
{
	return (FTPPutOneFile3(cip, file, dstfile, kTypeAscii, -1, 0, NULL, NULL, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0));
}	/* FTPPutOneFileAscii */
