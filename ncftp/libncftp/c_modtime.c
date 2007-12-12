/* c_modtime.c
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
FTPFileModificationTime(const FTPCIPtr cip, const char *const file, time_t *const mdtm)
{
	int result;
	ResponsePtr rp;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((mdtm == NULL) || (file == NULL))
		return (kErrBadParameter);
	*mdtm = kModTimeUnknown;

	if (cip->hasMDTM == kCommandNotAvailable) {
		cip->errNo = kErrMDTMNotAvailable;
		result = kErrMDTMNotAvailable;
	} else {
		rp = InitResponse();
		if (rp == NULL) {
			result = kErrMallocFailed;
			cip->errNo = kErrMallocFailed;
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		} else {
			result = RCmd(cip, rp, "MDTM %s", file);
			if (result < 0) {
				DoneWithResponse(cip, rp);
				return (result);
			}
			if (result == 2) {
				if (strncmp(rp->msg.first->line, "1910", 4) == 0) {
					/* Year was printed as "19100" rather than
					 * "2000" ...
					 */
					FTPLogError(cip, kDontPerror, "Warning: Server has Y2K Bug in \"MDTM\" command.\n");
				}
				*mdtm = UnMDTMDate(rp->msg.first->line);
				cip->hasMDTM = kCommandAvailable;
				result = kNoErr;
			} else if (FTP_UNIMPLEMENTED_CMD(rp->code)) {
				cip->hasMDTM = kCommandNotAvailable;
				cip->hasMDTM_set = kCommandNotAvailable;
				cip->errNo = kErrMDTMNotAvailable;
				result = kErrMDTMNotAvailable;
			} else {
				cip->errNo = kErrMDTMFailed;
				result = kErrMDTMFailed;
			}
			DoneWithResponse(cip, rp);
		}
	}
	return (result);
}	/* FTPFileModificationTime */
