/* io_getonefile.c
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
FTPGetOneFile3(
	const FTPCIPtr cip,
	const char *const file,
	const char *const dstfile,
	const int xtype,
	const int fdtouse,
	const int resumeflag,
	const int appendflag,
	const int deleteflag,
	const FTPConfirmResumeDownloadProc resumeProc,
	int UNUSED(reserved))
{
	int result;

	LIBNCFTP_USE_VAR(reserved);
	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);
	
	if ((file == NULL) || (file[0] == '\0'))
		return (kErrBadParameter);
	if (fdtouse < 0) {
		if ((dstfile == NULL) || (dstfile[0] == '\0'))
			return (kErrBadParameter);
	}

	result = FTPGetOneF(cip, file, dstfile, xtype, fdtouse, kSizeUnknown, kModTimeUnknown, resumeflag, appendflag, deleteflag, resumeProc);
	return (result);
}	/* FTPGetOneFile3 */



int
FTPGetOneFile(const FTPCIPtr cip, const char *const file, const char *const dstfile)
{
	return (FTPGetOneFile3(cip, file, dstfile, kTypeBinary, -1, kResumeNo, kAppendNo, kDeleteNo, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetOneFile */




int
FTPGetOneFile2(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int resumeflag, const int appendflag)
{
	return (FTPGetOneFile3(cip, file, dstfile, xtype, fdtouse, resumeflag, appendflag, kDeleteNo, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetOneFile2 */




int
FTPGetOneFileAscii(const FTPCIPtr cip, const char *const file, const char *const dstfile)
{
	return (FTPGetOneFile3(cip, file, dstfile, kTypeAscii, -1, kResumeNo, kAppendNo, kDeleteNo, (FTPConfirmResumeDownloadProc) 0, 0));
}	/* FTPGetOneFileAscii */
