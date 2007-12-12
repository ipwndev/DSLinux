/* c_rhelp.c
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
FTPRemoteHelp(const FTPCIPtr cip, const char *const pattern, const FTPLineListPtr llp)
{
	int result;
	ResponsePtr rp;

	if ((cip == NULL) || (llp == NULL))
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	InitLineList(llp);
	rp = InitResponse();
	if (rp == NULL) {
		result = kErrMallocFailed;
		cip->errNo = kErrMallocFailed;
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
	} else {
		if ((pattern == NULL) || (*pattern == '\0'))
			result = RCmd(cip, rp, "HELP");
		else
			result = RCmd(cip, rp, "HELP %s", pattern);
		if (result < 0) {
			DoneWithResponse(cip, rp);
			return (result);
		} else if (result == 2) {
			if (CopyLineList(llp, &rp->msg) < 0) {
				result = kErrMallocFailed;
				cip->errNo = kErrMallocFailed;
				FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			} else {
				result = kNoErr;
			}
		} else {
			cip->errNo = kErrHELPFailed;
			result = kErrHELPFailed;
		}
		DoneWithResponse(cip, rp);
	}
	return (result);
}	/* FTPRemoteHelp */
