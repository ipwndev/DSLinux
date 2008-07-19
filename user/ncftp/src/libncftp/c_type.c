/* c_type.c
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
FTPSetTransferType(const FTPCIPtr cip, int type)
{
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->curTransferType != type) {
		switch (type) {
			case kTypeAscii:
			case kTypeBinary:
			case kTypeEbcdic:
				break;
			case 'i':
			case 'b':
			case 'B':
				type = kTypeBinary;
				break;
			case 'e':
				type = kTypeEbcdic;
				break;
			case 'a':
				type = kTypeAscii;
				break;
			default:
				/* Yeah, we don't support Tenex.  Who cares? */
				FTPLogError(cip, kDontPerror, "Bad transfer type [%c].\n", type);
				cip->errNo = kErrBadTransferType;
				return (kErrBadTransferType);
		}
		result = FTPCmd(cip, "TYPE %c", type);
		if (result != 2) {
			result = kErrTYPEFailed;
			cip->errNo = kErrTYPEFailed;
			return (result);
		}
		cip->curTransferType = type;
	}
	return (kNoErr);
}	/* FTPSetTransferType */
