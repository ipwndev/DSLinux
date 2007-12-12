/* c_mkdir.c
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
FTPMkdir2(const FTPCIPtr cip, const char *const newDir, const int recurse, const char *const curDir)
{
	int result, result2;
	char *cp, *newTreeStart, *cp2;
	char dir[512];
	char dir2[512];
	char c;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((newDir == NULL) || (newDir[0] == '\0')) {
		cip->errNo = kErrInvalidDirParam;
		return (kErrInvalidDirParam);
	}

	/* Preserve old working directory. */
	if ((curDir == NULL) || (curDir[0] == '\0')) {
		/* This hack is nice so you can eliminate an
		 * unnecessary "PWD" command on the server,
		 * since if you already knew what directory
		 * you're in.  We want to minimize the number
		 * of client-server exchanges when feasible.
		 */
		(void) FTPGetCWD(cip, cip->buf, cip->bufSize);
	}

	result = FTPChdir(cip, newDir);
	if (result == kNoErr) {
		/* Directory already exists -- but we
		 * must now change back to where we were.
		 */
		result2 = FTPChdir(cip, ((curDir == NULL) || (curDir[0] == '\0')) ? cip->buf : curDir);
		if (result2 < 0) {
			result = kErrCannotGoToPrevDir;
			cip->errNo = kErrCannotGoToPrevDir;
			return (result);
		}

		/* Don't need to create it. */
		return (kNoErr);
	}

	if (recurse == kRecursiveNo) {
		result = FTPCmd(cip, "MKD %s", newDir);
		if (result > 0) {
			if (result != 2) {
				FTPLogError(cip, kDontPerror, "MKD %s failed; [%s]\n", newDir, cip->lastFTPCmdResultStr);
				result = kErrMKDFailed;
				cip->errNo = kErrMKDFailed;
				return (result);
			} else {
				result = kNoErr;
			}
		}
	} else {
		(void) STRNCPY(dir, newDir);

		/* Strip trailing slashes. */
		cp = dir + strlen(dir) - 1;
		for (;;) {
			if (cp <= dir) {
				if ((newDir == NULL) || (newDir[0] == '\0')) {
					cip->errNo = kErrInvalidDirParam;
					result = kErrInvalidDirParam;
					return (result);
				}
			}
			if ((*cp != '/') && (*cp != '\\')) {
				cp[1] = '\0';
				break;
			}
			--cp;
		}
		(void) STRNCPY(dir2, dir);

		if ((strrchr(dir, '/') == dir) || (strrchr(dir, '\\') == dir)) {
			/* Special case "mkdir /subdir" */
			result = FTPCmd(cip, "MKD %s", dir);
			if (result < 0) {
				return (result);
			}
			if (result != 2) {
				FTPLogError(cip, kDontPerror, "MKD %s failed; [%s]\n", dir, cip->lastFTPCmdResultStr);
				result = kErrMKDFailed;
				cip->errNo = kErrMKDFailed;
				return (result);
			}
			/* Haven't chdir'ed, don't need to goto goback. */
			return (kNoErr);
		}

		for (;;) {
			cp = strrchr(dir, '/');
			if (cp == NULL)
				cp = strrchr(dir, '\\');
			if (cp == NULL) {
				cp = dir + strlen(dir) - 1;
				if (dir[0] == '\0') {
					result = kErrMKDFailed;
					cip->errNo = kErrMKDFailed;
					return (result);
				}
				/* Note: below we will refer to cp + 1
				 * which is why we set cp to point to
				 * the byte before the array begins!
				 */
				cp = dir;
				--cp;
				break;
			}
			if (cp == dir) {
				result = kErrMKDFailed;
				cip->errNo = kErrMKDFailed;
				return (result);
			}
			*cp = '\0';
			result = FTPChdir(cip, dir);
			if (result == 0) {
				break;	/* Found a valid parent dir. */
				/* from this point, we need to preserve old dir. */
			}
		}

		newTreeStart = dir2 + ((cp + 1) - dir);
		for (cp = newTreeStart; ; ) {
			cp2 = cp;
			cp = strchr(cp2, '/');
			c = '/';
			if (cp == NULL)
				cp = strchr(cp2, '\\');
			if (cp != NULL) {
				c = *cp;
				*cp = '\0';
				if (cp[1] == '\0') {
					/* Done, if they did "mkdir /tmp/dir/" */
					break;
				}
			}
			result = FTPCmd(cip, "MKD %s", newTreeStart);
			if (result < 0) {
				return (result);
			}
			if (result != 2) {
				FTPLogError(cip, kDontPerror, "Cwd=%s; MKD %s failed; [%s]\n", cip->buf, newTreeStart, cip->lastFTPCmdResultStr);
				result = kErrMKDFailed;
				cip->errNo = kErrMKDFailed;
				goto goback;
			}
			if (cp == NULL)
				break;	/* No more to make, done. */
			*cp++ = c;
		}
		result = kNoErr;

goback:
		result2 = FTPChdir(cip, ((curDir == NULL) || (curDir[0] == '\0')) ? cip->buf : curDir);
		if ((result == 0) && (result2 < 0)) {
			result = kErrCannotGoToPrevDir;
			cip->errNo = kErrCannotGoToPrevDir;
		}
	}
	return (result);
}	/* FTPMkdir2 */



int
FTPMkdir(const FTPCIPtr cip, const char *const newDir, const int recurse)
{
	return (FTPMkdir2(cip, newDir, recurse, NULL));
}	/* FTPMkdir */




int
FTPMkParentDir(const FTPCIPtr cip, const char *const path, const int recurse, const char *const curDir)
{
	char newDir[512];
	char *cp;

	if ((path == NULL) || (path[0] == '\0')) {
		cip->errNo = kErrInvalidDirParam;
		return (kErrInvalidDirParam);
	}

	STRNCPY(newDir, path);
	if ((newDir[sizeof(newDir) - 2] != '\0') && (path[sizeof(newDir) - 1] != '\0')) {
		/* Path too long to make a copy of it. */
		cip->errNo = kErrInvalidDirParam;
		return (kErrInvalidDirParam);
	}

	StrRemoveTrailingSlashes(newDir);
	cp = StrRFindLocalPathDelim(newDir);
	if (cp == newDir) {
		/* File is in the root directory, which is already made. */
		return (kNoErr);
	} else if (cp == NULL) {
		/* File is in the current directory; the parent directory
		 * is already made.
		 */
		return (kNoErr);
	}
	*cp = '\0';

	return (FTPMkdir2(cip, newDir, recurse, curDir));
}	/* FTPMkParentDir */

