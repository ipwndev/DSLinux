/* c_chdirlist.c
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
FTPChdirList(FTPCIPtr cip, FTPLineListPtr const cdlist, char *const newCwd, const size_t newCwdSize, int flags)
{
	size_t len;
	char *cdstr;
	FTPLinePtr lp;
	int lastSubDir;
	int mkd, pwd;
	int result;

	/* Retain backwards compatibility with earlier library versions. */
	if (flags == kChdirOnly)
		flags = kChdirFullPath;

	if ((flags & kChdirFullPath) != 0) {
		len = 0;
		for (lp = cdlist->first; lp != NULL; lp = lp->next) {
			len += strlen(lp->line);
			len++;	/* account for delimiting slash */
		}
		cdstr = malloc(len + 1);
		if (cdstr == NULL)
			return (cip->errNo = kErrMallocFailed);
		cdstr[0] = '\0';
		for (lp = cdlist->first; lp != NULL; lp = lp->next) {
			strcat(cdstr, lp->line);
			if (lp->next != NULL)
				strcat(cdstr, "/");
		}
		if (FTPChdir3(cip, cdstr, newCwd, newCwdSize, (flags & ~kChdirOneSubdirAtATime)) == kNoErr) {
			free(cdstr);
			return (kNoErr);
		}
		free(cdstr);
	}

	if ((flags & kChdirOneSubdirAtATime) != 0) {
		mkd = (flags & kChdirAndMkdir);
		pwd = (flags & kChdirAndGetCWD);
		lastSubDir = 0;
		result = kNoErr;

		for (lp = cdlist->first; lp != NULL; lp = lp->next) {
			if (lp->next == NULL)
				lastSubDir = 1;

			if (strcmp(lp->line, ".") == 0) {
				result = 0;
				if ((lastSubDir != 0) && (pwd != 0))
					result = FTPGetCWD(cip, newCwd, newCwdSize);
			} else if ((lastSubDir != 0) && (pwd != 0)) {
				result = FTPChdirAndGetCWD(cip, (*lp->line != '\0') ? lp->line : "/", newCwd, newCwdSize);
			} else {
				result = FTPChdir(cip, (*lp->line != '\0') ? lp->line : "/");
			}
			if (result < 0) {
				if ((mkd != 0) && (*lp->line != '\0')) {
					if (FTPCmd(cip, "MKD %s", lp->line) == 2) {
						result = FTPChdir(cip, lp->line);
					} else {
						/* couldn't change nor create */
						cip->errNo = result;
					}
				} else {
					cip->errNo = result;
				}
			}
			if (result != kNoErr)
				break;
		}
		return (result);
	}

	return (kErrBadParameter);
}	/* FTPChdirList */
