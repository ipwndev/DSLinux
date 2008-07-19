/* rftw.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Internal to rftw.c */
typedef struct FtwSubDirList *FtwSubDirListPtr;
typedef struct FtwSubDirList {
	FtwSubDirListPtr next;
	struct Stat st;
	size_t fnLen;
	char name[1];
} FtwSubDirList;

/* This mess is essentially the local version of Ftw with FTP
 * grafted onto it, so see that and grok that before studying this.
 */
static int
FTPFtwTraverse(const FtwInfoPtr ftwip, size_t dirPathLen, int depth)
{
	char *cp;
	size_t fnLen;
	mode_t m;
	char *filename;
	char *newBuf;
	char *path = ftwip->curPath;
	int nSubdirs;
	FtwSubDirListPtr head = NULL, tail = NULL, sdp, nextsdp;
	int rc = (-1);
	int lsl, mls, unlsrc;
	FTPCIPtr cip = (FTPCIPtr) ftwip->cip;
	FTPLineList ll;
	FTPFileInfoList fil;
	FTPLinePtr filePtr;
	FTPFileInfoPtr fip;
	int result;
	int isRootDir;
	longest_int sz;

	isRootDir = ((dirPathLen == 1) && ((path[0] == '/') || (path[0] == '\\'))) ? 1 : 0;
	filePtr = NULL;
	fip = NULL;
	mls = 0;
	lsl = 0;

	if (cip->hasMLSD == kCommandAvailable) {
		mls = 1;
		if (((result = FTPListToMemory2(cip, dirPathLen ? path : ".", &ll, "-a", 0, &mls)) < 0) || (ll.first == NULL)) {
			/* Not an error unless the first directory could not be opened. */
			DisposeLineListContents(&ll);
			return (0);
		}

		/* "MLSD" succeeded */
		unlsrc = UnMlsD(cip, &fil, &ll);
		if (unlsrc < 0) {
			DisposeLineListContents(&ll);
			return (cip->errNo = kErrInvalidMLSTResponse);
		} else if (unlsrc == 0) {
			/* empty */
			DisposeLineListContents(&ll);
			return (0);
		}
		fip = fil.first;
		DisposeLineListContents(&ll);
	} else {
		if (((result = FTPListToMemory2(cip, dirPathLen ? path : ".", &ll, "-la", 0, &mls)) < 0) || (ll.first == NULL)) {
			DisposeLineListContents(&ll);
			if (((result = FTPListToMemory2(cip, dirPathLen ? path : ".", &ll, (cip->hasNLST_a == kCommandNotAvailable) ? "" : "-a", 0, &mls)) < 0) || (ll.first == NULL)) {
				DisposeLineListContents(&ll);
				return (0);
			} else {
				/* "NLST -a" succeeded */
				RemoteGlobCollapse(cip, path, &ll);
				filePtr = ll.first;
			}
		} else {
			/* "LIST -a" succeeded */
			lsl = 1;
			unlsrc = UnLslR(cip, &fil, &ll, cip->serverType);
			if (unlsrc < 0) {
				DisposeLineListContents(&ll);
				return (cip->errNo = kErrInvalidMLSTResponse);
			} else if (unlsrc == 0) {
				/* empty */
				DisposeLineListContents(&ll);
				return (0);
			}
			fip = fil.first;
			DisposeLineListContents(&ll);
		}
	}

	nSubdirs = 0;
	++ftwip->numDirs;
	ftwip->depth = depth;
	if (ftwip->maxDepth < ftwip->depth) {
		ftwip->maxDepth = ftwip->depth;
	}
	filename = path + dirPathLen;
	if (isRootDir == 0) {	/* Root directory is a separator. */
		*filename++ = (char) ftwip->dirSeparator;
		dirPathLen++;
	}
	*filename = '\0';
	/* Path now contains dir/  */

	for (;;) {
		if ((mls != 0) || (lsl != 0)) {
			if (fip == NULL)
				break;
			cp = fip->relname;
		} else {
			if (filePtr == NULL)
				break;
			cp = filePtr->line;
		}
		if ((cp[0] == '.') && ((cp[1] == '\0') || ((cp[1] == '.') && (cp[2] == '\0'))))
			goto nxt;	/* Skip . and .. */

		ftwip->rlinkto = NULL;
		*filename = '\0';
		fnLen = strlen(cp) + 1	/* include \0 */;
		if ((fnLen + dirPathLen) > ftwip->curPathAllocSize) {
			if (ftwip->autoGrow == kFtwNoAutoGrowAndFail) {
				goto panic;
			} else if (ftwip->autoGrow == kFtwNoAutoGrowButContinue) {
				goto nxt;
			}
			newBuf = (char *) realloc(ftwip->curPath, fnLen + dirPathLen + 30 + 2 /* room for / and \0 */);
			if (newBuf == NULL)
				goto panic;
			ftwip->curPath = newBuf;
			ftwip->curPathAllocSize = fnLen + dirPathLen + 30;
			path = ftwip->curPath;
			filename = path + dirPathLen;
			if (isRootDir == 0)	/* Root directory is a separator. */
				*filename++ = (char) ftwip->dirSeparator;
			*filename = '\0';
		}
		memcpy(filename, cp, fnLen);
		ftwip->curPathLen = dirPathLen + fnLen - 1;
		ftwip->curFile = filename;
		ftwip->curFileLen = fnLen - 1;

		memset(&ftwip->curStat, 0, sizeof(ftwip->curStat));
		if (mls != 0) {
			ftwip->curType = fip->type;
			if (fip->type == 'd') {
				ftwip->curStat.st_mode = S_IFDIR;
				ftwip->curStat.st_size = (longest_int) -1;
#ifdef S_IFLNK
			} else if (fip->type == 'l') {
				ftwip->curStat.st_mode = S_IFLNK;
				ftwip->rlinkto = fip->rlinkto;
#endif
			} else if (fip->type == '-') {
				ftwip->curStat.st_mode = S_IFREG;
				ftwip->curStat.st_size = fip->size;
			} else {
				/* unknown type, skip */
				goto nxt;
			}
			if (fip->mode != (-1))
				ftwip->curStat.st_mode |= (fip->mode & 00777);
			ftwip->curStat.st_mtime = fip->mdtm;
		} else if (lsl != 0) {
			ftwip->curType = fip->type;
			if (fip->type == 'd') {
				ftwip->curStat.st_mode = S_IFDIR;
				ftwip->curStat.st_size = (longest_int) -1;
#ifdef S_IFLNK
			} else if (fip->type == 'l') {
				ftwip->curStat.st_mode = S_IFLNK;
				ftwip->rlinkto = fip->rlinkto;
#endif
			} else if (fip->type == '-') {
				ftwip->curStat.st_mode = S_IFREG;
				ftwip->curStat.st_size = fip->size;
			} else {
				/* unknown type, skip */
				goto nxt;
			}
			if (fip->mode != (-1))
				ftwip->curStat.st_mode |= (fip->mode & 00777);
			ftwip->curStat.st_mtime = fip->mdtm;

			/* Override local times in LS output! */
			result = FTPFileModificationTime(cip, path, &fip->mdtm);
			if (fip->mdtm != kModTimeUnknown) {
				ftwip->curStat.st_mtime = fip->mdtm;
			}
		} else {
			result = FTPIsDir(cip, path);
			if (result < 0) {
				/* error */
				/* could be just a stat error, so continue */
				goto nxt;
			} else if (result == 1) {
				/* directory */
				ftwip->curType = 'd';
				ftwip->curStat.st_mode = S_IFDIR | 00755;
				result = FTPFileModificationTime(cip, path, &ftwip->curStat.st_mtime);
			} else {
				/* file */
				ftwip->curType = '-';
				ftwip->curStat.st_mode = S_IFREG | 00644;
				result = FTPFileSizeAndModificationTime(cip, path, &sz, kTypeBinary, &ftwip->curStat.st_mtime);
#if defined(TRU64UNIX) || defined(DIGITAL_UNIX)
				ftwip->curStat.st_size = (off_t) sz;
#else
				ftwip->curStat.st_size = sz;
#endif
			}
		}

		{
			m = ftwip->curStat.st_mode;
			if (S_ISREG(m)) {
				++ftwip->numFiles;
				ftwip->curType = '-';
				if ((*ftwip->proc)(ftwip) < 0) {
					goto panic;
				}
			} else if (S_ISLNK(m)) {
				++ftwip->numLinks;
				ftwip->curType = 'l';
				if ((*ftwip->proc)(ftwip) < 0) {
					goto panic;
				}
			} else if (S_ISDIR(m)) {
				/* We delay entering the subdirectories
				 * until we have closed this directory.
				 * This will conserve file descriptors
				 * and also have the effect of having
				 * the files processed first.
				 */
				sdp = (FtwSubDirListPtr) malloc(sizeof(FtwSubDirList) + fnLen);
				if (sdp == NULL)
					goto panic;
				memcpy(&sdp->st, &ftwip->curStat, sizeof(sdp->st));
				memcpy(sdp->name, cp, fnLen);
				sdp->fnLen = fnLen;
				sdp->next = NULL;
				if (head == NULL) {
					head = tail = sdp;
				} else {
					tail->next = sdp;
					tail = sdp;
				}
				nSubdirs++;
			}
		}
nxt:
		if ((mls != 0) || (lsl != 0)) {
			fip = fip->next;
		} else {
			filePtr = filePtr->next;
		}
	}

	if ((mls != 0) || (lsl != 0)) {
		DisposeFileInfoListContents(&fil);
	} else {
		DisposeLineListContents(&ll);
	}

	/* Now enter each subdirectory. */
	for (sdp = head; sdp != NULL; sdp = nextsdp) {
		nextsdp = sdp->next;
		memcpy(&ftwip->curStat, &sdp->st, sizeof(ftwip->curStat));
		fnLen = sdp->fnLen;
		memcpy(filename, sdp->name, fnLen);
		ftwip->curFile = filename;
		ftwip->curFileLen = fnLen - 1;
		ftwip->curPathLen = dirPathLen + fnLen - 1;
		head = nextsdp;
		free(sdp);

		ftwip->curType = 'd';
		if ((*ftwip->proc)(ftwip) < 0) {
			goto panic;
		}
		if (FTPFtwTraverse(ftwip, dirPathLen + fnLen - 1, depth + 1) < 0)
			goto panic;

		/* Reset these, since buffer could have
		 * been reallocated.
		 */
		path = ftwip->curPath;
		filename = path + dirPathLen;
		*filename = '\0';
	}
	head = NULL;
	rc = 0;

panic:
	if (mls != 0) {
		DisposeFileInfoListContents(&fil);
	} else {
		DisposeLineListContents(&ll);
	}

	for (sdp = head; sdp != NULL; sdp = nextsdp) {
		nextsdp = sdp->next;
		free(sdp);
	}

	return (rc);
}	/* FTPFtwTraverse */


int
FTPFtw(const FTPCIPtr cip, const FtwInfoPtr ftwip, const char *const path, FtwProc proc)
{
	size_t len, alen;
	int rc;
	MLstItem mli;
	char *cp, *endp;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((ftwip->init != kFtwMagic) || (path == NULL) || (path[0] == '\0') || (proc == (FtwProc) 0)) {
		cip->errNo = kErrBadParameter;
		errno = EINVAL;
		return (kErrBadParameter);
	}

	ftwip->rlinkto = NULL;
	ftwip->dirSeparator = '/';
	ftwip->rootDir[0] = '/';
	ftwip->startPathLen = 0;

	len = strlen(path);
	if (ftwip->curPath == NULL) {
		/* Call FtwSetBuf before calling Ftw for
		 * the first time, otherwise you get the
		 * default behavior.
		 */
		ftwip->autoGrow = kFtwAutoGrow;
		alen = len + 30 /* room to append filenames */ + 2 /* room for / and \0 */;
		if (alen < 256)
			alen = 256;
		ftwip->curPath = (char *) malloc(alen);
		if (ftwip->curPath == NULL)
			return (-1);
		ftwip->curPathAllocSize = alen - 2;
	}

	ftwip->cip = (void *) cip;

	rc = FTPIsDir(cip, path);
	if (rc < 0) {
		/* error */
		return rc;
	} else if (rc == 0) {
		rc = cip->errNo = kErrNotADirectory;
		errno = ENOTDIR;
		return (rc);
	}
	memset(&ftwip->curStat, 0, sizeof(ftwip->curStat));
	ftwip->curStat.st_mode = (S_IFDIR | 00755);
	ftwip->curType = 'd';
	if (FTPMListOneFile(cip, path, &mli) == 0) {
		ftwip->curStat.st_mtime = mli.ftime;
		if (mli.mode != (-1)) {
			ftwip->curStat.st_mode = S_IFDIR;
			ftwip->curStat.st_mode |= (mli.mode & 00777);
		}
	} else {
		(void) FTPFileModificationTime(cip, path, &ftwip->curStat.st_mtime);
	}
	ftwip->curStat.st_size = (longest_int) -1;

	memset(ftwip->curPath, 0, ftwip->curPathAllocSize);
	memcpy(ftwip->curPath, path, len + 1);
	endp = cp = ftwip->curPath + strlen(ftwip->curPath);
	--cp;
	while ((cp > ftwip->curPath) && ((*cp == '/') || (*cp == '\\')))
		*cp-- = '\0';
	ftwip->curPathLen = ftwip->startPathLen = len = (size_t) (endp - ftwip->curPath);
	while (cp >= ftwip->curPath) {
		if ((*cp == '/') || (*cp == '\\'))
			break;
		--cp;
	}
	ftwip->curFile = ++cp;
	ftwip->curFileLen = (size_t) (endp - cp);
	ftwip->proc = proc;
	if ((*proc)(ftwip) < 0) {
		return (-1);
	}

	ftwip->depth = ftwip->maxDepth = ftwip->numDirs = ftwip->numFiles = ftwip->numLinks = 0;
	rc = FTPFtwTraverse(ftwip, len, 1);

	/* Restore the start path when finished. */
	memset(ftwip->curPath + ftwip->startPathLen, 0, ftwip->curPathAllocSize - ftwip->startPathLen);
	ftwip->curPathLen = ftwip->startPathLen;

	/* Clear these out since you shouldn't be using them
	 * after Ftw returns.
	 */
	memset(&ftwip->curStat, 0, sizeof(ftwip->curStat));
	ftwip->proc = (FtwProc) 0;
	ftwip->curFile = ftwip->curPath;
	ftwip->curFileLen = 0;
	ftwip->cip = 0;
	ftwip->rlinkto = NULL;

	return (rc);
}	/* FTPFtw */
