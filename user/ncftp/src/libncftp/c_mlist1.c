/* c_mlist1.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
FTPRequestMlsOptions(const FTPCIPtr cip)
{
	int f;
	char optstr[128];
	size_t optstrlen;

	if (cip->usedMLS == 0) {
		/* First MLSD/MLST ? */
		cip->usedMLS = 1;

		f = cip->mlsFeatures & kPreferredMlsOpts;
		optstr[0] = '\0';

		/* TYPE */
		if ((f & kMlsOptType) != 0) {
			STRNCAT(optstr, "type;");
		}

		/* SIZE */
		if ((f & kMlsOptSize) != 0) {
			STRNCAT(optstr, "size;");
		}

		/* MODTIME */
		if ((f & kMlsOptModify) != 0) {
			STRNCAT(optstr, "modify;");
		}

		/* MODE */
		if ((f & kMlsOptUNIXmode) != 0) {
			STRNCAT(optstr, "UNIX.mode;");
		}

		/* PERM */
		if ((f & kMlsOptPerm) != 0) {
			STRNCAT(optstr, "perm;");
		}

		/* OWNER */
		if ((f & kMlsOptUNIXowner) != 0) {
			STRNCAT(optstr, "UNIX.owner;");
		}

		/* UID */
		if ((f & kMlsOptUNIXuid) != 0) {
			STRNCAT(optstr, "UNIX.uid;");
		}

		/* GROUP */
		if ((f & kMlsOptUNIXgroup) != 0) {
			STRNCAT(optstr, "UNIX.group;");
		}

		/* GID */
		if ((f & kMlsOptUNIXgid) != 0) {
			STRNCAT(optstr, "UNIX.gid;");
		}

		/* UNIQUE */
		if ((f & kMlsOptUnique) != 0) {
			STRNCAT(optstr, "unique;");
		}

		/* Tell the server what we prefer. */
		optstrlen = strlen(optstr);
		if (optstrlen != 0)
			(void) FTPCmd(cip, "OPTS MLST %s", optstr);
	}
}	/* FTPRequestMlsOptions */




int
FTPMListOneFile(const FTPCIPtr cip, const char *const file, const MLstItemPtr mlip)
{
	int result;
	ResponsePtr rp;

	/* We do a special check for older versions of NcFTPd which
	 * are based off of an incompatible previous version of IETF
	 * extensions.
	 *
	 * Roxen also seems to be way outdated, where MLST was on the
	 * data connection among other things.
	 *
	 */
	if (
		(cip->hasMLST == kCommandNotAvailable) ||
		((cip->serverType == kServerTypeNcFTPd) && (cip->ietfCompatLevel < 19981201)) ||
		(cip->serverType == kServerTypeRoxen)
	) {
		cip->errNo = kErrMLSTNotAvailable;
		return (cip->errNo);
	}

	rp = InitResponse();
	if (rp == NULL) {
		result = cip->errNo = kErrMallocFailed;
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
	} else {
		FTPRequestMlsOptions(cip);
		result = RCmd(cip, rp, "MLST %s", file);
		if (
			(result == 2) &&
			(rp->msg.first->line != NULL) &&
			(rp->msg.first->next != NULL) &&
			(rp->msg.first->next->line != NULL)
		) {
			result = UnMlsT(cip, rp->msg.first->next->line, mlip);
			if (result < 0) {
				cip->errNo = result = kErrInvalidMLSTResponse;
			}
		} else if (FTP_UNIMPLEMENTED_CMD(rp->code)) {
			cip->hasMLST = kCommandNotAvailable;
			cip->errNo = kErrMLSTNotAvailable;
			result = kErrMLSTNotAvailable;
		} else {
			cip->errNo = kErrMLSTFailed;
			result = kErrMLSTFailed;
		}
		DoneWithResponse(cip, rp);
	}

	return (result);
}	/* FTPMListOneFile */
