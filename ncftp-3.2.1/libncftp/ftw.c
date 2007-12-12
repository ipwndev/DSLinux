/* Ftw.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#if defined(SOLARIS) && (SOLARIS >= 250)
#	define _POSIX_PTHREAD_SEMANTICS 1
#endif

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Internal to ftw.c */
typedef struct FtwSubDirList *FtwSubDirListPtr;
typedef struct FtwSubDirList {
	FtwSubDirListPtr next;
	struct Stat st;
	size_t fnLen;
	char name[1];
} FtwSubDirList;

struct dirent *Readdir(DIR *const dir, struct dirent *const dp, const size_t sz);



#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
DIR *opendir(const char *const path)
{
	DIR *p;
	char *dirpath;
	size_t len;

	p = (DIR *) malloc(sizeof(DIR));
	if (p == NULL)
		return NULL;
	memset(p, 0, sizeof(DIR));

	len = strlen(path);
	dirpath = (char *) malloc(len + 5);
	if (dirpath == NULL)
		return NULL;
	p->dirpath = dirpath;

	memcpy(dirpath, path, len + 1);
	if (IsLocalPathDelim(dirpath[len - 1])) {
		--len;
		dirpath[len] = '\0';
	}
	memcpy(dirpath + len, "\\*.*", (size_t) 5);

	p->searchHandle = FindFirstFile(dirpath, &p->ffd);
	if (p->searchHandle == INVALID_HANDLE_VALUE) {
		memset(&p->ffd, 0, sizeof(p->ffd));
	}
	return (p);
}	/* opendir */



struct dirent *readdir(DIR *dir)
{
	memcpy(dir->dent.d_name, dir->ffd.cFileName, (size_t) sizeof(dir->dent.d_name));
	if (dir->searchHandle != INVALID_HANDLE_VALUE) {
		if (!FindNextFile(dir->searchHandle, &dir->ffd)) {
			/* no more items, or an error we don't care about */
			FindClose(dir->searchHandle);
			dir->searchHandle = INVALID_HANDLE_VALUE;
			memset(&dir->ffd, 0, sizeof(dir->ffd));
		}
	}
	if (dir->dent.d_name[0] == '\0')
		return NULL;
	return (&dir->dent);
}	/* readdir */



void closedir(DIR *dir)
{
	/* The searchHandle is already closed, but we
	 * need to dealloc the structures.
	 */
	if ((dir != NULL) && (dir->dirpath != NULL)) {
		free(dir->dirpath);
		memset(dir, 0, sizeof(DIR));
		free(dir);
	}
}	/* closedir */
#endif	/* WIN32 */




/*
 * Warning: be sure to use a custom-sized struct dirent, since the
 * real struct dirent on some platforms is just a stub structure
 * with room for only one byte in the filename.
 */
struct dirent *
Readdir(DIR *const dir, struct dirent *const dp, const size_t sz)
{
#if defined(MACOSX)
	struct dirent *p;
	p = readdir(dir);
	if (p != NULL) {
		memcpy(dp, p, sz);
		return (dp);
	}
#elif defined(HAVE_READDIR_R) && ( (defined(SOLARIS) && (SOLARIS < 250)) || (defined(SCO)) || (defined(IRIX) && (IRIX < 6)) )
	struct dirent *p;
	p = readdir_r(dir, dp);
	if (p != NULL)
		return (dp);
#elif defined(HAVE_READDIR_R) && (defined(HPUX) && (HPUX < 1100))
	struct dirent *p;
	if (readdir_r(dir, dp) >= 0)
		return (dp);
#elif defined(HAVE_READDIR_R)
	struct dirent *p;
	p = NULL;
	if ((readdir_r(dir, dp, &p) == 0) && (p != NULL))
		return (dp);
#else
	struct dirent *p;
	p = readdir(dir);
	if (p != NULL) {
#	if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		memset(dp, 0, sz);
		memcpy(dp, p, ((sz < sizeof(struct dirent)) ? sz : sizeof(struct dirent)));
#	else
		memcpy(dp, p, sz);
#	endif
		return (dp);
	}
#endif

	memset(dp, 0, sz);
	return (NULL);
}	/* Readdir */



void
FtwInit(FtwInfo *const ftwip)
{
	memset(ftwip, 0, sizeof(FtwInfo));
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	ftwip->dirSeparator = '\\';
	ftwip->rootDir[0] = '\\';
#else
	ftwip->dirSeparator = '/';
	ftwip->rootDir[0] = '/';
#endif
	ftwip->init = kFtwMagic;
}	/* FtwInit */



void
FtwDispose(FtwInfo *const ftwip)
{
	if (ftwip->init != kFtwMagic)
		return;
	if ((ftwip->noAutoMallocAndFree == 0) && (ftwip->curPath != NULL))
		free(ftwip->curPath);
	memset(ftwip, 0, sizeof(FtwInfo));
}	/* FtwDispose */


static int
FtwTraverse(const FtwInfoPtr ftwip, size_t dirPathLen, int depth)
{
	DIR *DIRp;
	char *cp;
	size_t fnLen;
	struct dirent *dentp;
	mode_t m;
	char *filename;
	char *newBuf;
	char *path = ftwip->curPath;
	int nSubdirs;
	FtwSubDirListPtr head = NULL, tail = NULL, sdp, nextsdp;
	int rc = (-1);
	int isRootDir;

	isRootDir = ((dirPathLen == 1) && (IsLocalPathDelim(path[0]))) ? 1 : 0;
	if ((DIRp = opendir(dirPathLen ? path : ".")) == NULL) {
		/* Not an error unless the first directory could not be opened. */
		return (0);
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

	dentp = (struct dirent *) ftwip->direntbuf;
	for (;;) {
		if (Readdir(DIRp, dentp, ftwip->direntbufSize) == NULL)
			break;
		cp = dentp->d_name;
		if ((cp[0] == '.') && ((cp[1] == '\0') || ((cp[1] == '.') && (cp[2] == '\0'))))
			continue;	/* Skip . and .. */

		*filename = '\0';
		fnLen = strlen(cp) + 1	/* include \0 */;
		if ((fnLen + dirPathLen) > ftwip->curPathAllocSize) {
			if (ftwip->autoGrow == kFtwNoAutoGrowAndFail) {
				goto panic;
			} else if (ftwip->autoGrow == kFtwNoAutoGrowButContinue) {
				continue;
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
		if (Lstat(path, &ftwip->curStat) == 0) {
			m = ftwip->curStat.st_mode;
			if (S_ISREG(m)) {
				++ftwip->numFiles;
				ftwip->curType = '-';
				if ((*ftwip->proc)(ftwip) < 0) {
					goto panic;
				}
#ifdef S_ISLNK
			} else if (S_ISLNK(m)) {
				ftwip->curType = 'l';
				++ftwip->numLinks;
				if ((*ftwip->proc)(ftwip) < 0) {
					goto panic;
				}
#endif	/* S_ISLNK */
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
	}
	(void) closedir(DIRp);
	DIRp = NULL;

	/* Now enter each subdirectory. */
	for (sdp = head; sdp != NULL; sdp = nextsdp) {
		nextsdp = sdp->next;
		memcpy(&ftwip->curStat, &sdp->st, sizeof(ftwip->curStat));
		fnLen = sdp->fnLen;
		memcpy(filename, sdp->name, fnLen);
		ftwip->curPathLen = dirPathLen + fnLen - 1;
		ftwip->curFile = filename;
		ftwip->curFileLen = fnLen - 1;
		head = nextsdp;
		free(sdp);

		ftwip->curType = 'd';
		if ((*ftwip->proc)(ftwip) < 0) {
			goto panic;
		}
		if (FtwTraverse(ftwip, dirPathLen + fnLen - 1, depth + 1) < 0)
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
	if (DIRp != NULL)
		(void) closedir(DIRp);

	for (sdp = head; sdp != NULL; sdp = nextsdp) {
		nextsdp = sdp->next;
		free(sdp);
	}

	return (rc);
}	/* FtwTraverse */




int
Ftw(const FtwInfoPtr ftwip, const char *const path, FtwProc proc)
{
	size_t len, alen;
	int rc;
	char *cp, *endp;
	size_t debufsize = 256;
#ifdef HAVE_PATHCONF
	long nmx;
#endif

	if ((ftwip->init != kFtwMagic) || (path == NULL) || (path[0] == '\0') || (proc == (FtwProc) 0)) {
		errno = EINVAL;
		return (-1);
	}

	ftwip->rlinkto = NULL;
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

	ftwip->curType = 'd';
	memset(ftwip->curPath, 0, ftwip->curPathAllocSize);
	memcpy(ftwip->curPath, path, len + 1);
	cp = ftwip->curPath + strlen(ftwip->curPath);
	--cp;
	while ((cp > ftwip->curPath) && IsLocalPathDelim(*cp))
		*cp-- = '\0';
	endp = cp + 1;
	ftwip->curPathLen = ftwip->startPathLen = len = (size_t) (endp - ftwip->curPath);
	while (cp >= ftwip->curPath) {
		if (IsLocalPathDelim(*cp))
			break;
		--cp;
	}
	ftwip->curFile = ++cp;
	ftwip->curFileLen = (size_t) (endp - cp);

	/* Note: we use Stat instead of Lstat here because we allow the
	 * top level node (as specified by path) to be a symlink
	 * to a directory.
	 */
	memset(&ftwip->curStat, 0, sizeof(ftwip->curStat));
	if (Stat(ftwip->curPath, &ftwip->curStat) < 0) {
		return (-1);
	} else if (! S_ISDIR(ftwip->curStat.st_mode)) {
		errno = ENOTDIR;
		return (-1);
	}

#ifdef HAVE_PATHCONF
	nmx = pathconf(ftwip->curPath, _PC_NAME_MAX);
	if (nmx >= 256)
		debufsize = nmx;
#endif
	debufsize += sizeof(struct dirent) + 8;
	ftwip->direntbuf = calloc(debufsize, (size_t) 1);
	if (ftwip->direntbuf == NULL) {
		return (-1);
	}
	ftwip->direntbufSize = debufsize;

	ftwip->proc = proc;
	if ((*proc)(ftwip) < 0) {
		free(ftwip->direntbuf);
		ftwip->direntbuf = NULL;
		return (-1);
	}

	ftwip->depth = ftwip->maxDepth = ftwip->numDirs = ftwip->numFiles = ftwip->numLinks = 0;
	rc = FtwTraverse(ftwip, len, 1);

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
	free(ftwip->direntbuf);
	ftwip->direntbuf = NULL;

	return (rc);
}	/* Ftw */




void
FtwSetBuf(const FtwInfoPtr ftwip, char *const buf, const size_t bufsize, int autogrow)
{
	if (ftwip->init != kFtwMagic)
		return;
	if ((ftwip->noAutoMallocAndFree == 0) && (ftwip->curPath != NULL)) {
		free(ftwip->curPath);
	}
	if (buf == NULL) {
		/* They want us to create it */
		ftwip->noAutoMallocAndFree = 0;
		ftwip->curPath = (char *) malloc(bufsize);
		ftwip->curPathAllocSize = (ftwip->curPath != NULL) ? bufsize : 0;
		ftwip->autoGrow = autogrow;
	} else {
		/* We have been given a buffer to borrow.
		 * Note that we won't autogrow a borrowed buffer.
		 */
		ftwip->noAutoMallocAndFree = 1;
		ftwip->curPath = buf;
		ftwip->curPathAllocSize = bufsize;
		ftwip->autoGrow = (autogrow == kFtwAutoGrow) ? kFtwNoAutoGrowAndFail : autogrow;
	}
}	/* FtwSetBuf */



#ifdef TEST

static int
MyFtwProc(const FtwInfoPtr ftwip)
{
	int m;

	m = ftwip->curStat.st_mode;
	if (S_ISREG(m) != 0) {
		/* file */
		printf("%s\n", ftwip->curPath);
	} else if (S_ISDIR(m)) {
		/* directory */
		printf("%s%c\n", ftwip->curPath, ftwip->dirSeparator);
	} else if (S_ISLNK(m)) {
		/* symbolic link */
		printf("%s@\n", ftwip->curPath);
	}

	return (0);
}	/* MyFtwProc */




int
main(int argc, char *argv[])
{
	int rc;
	FtwInfo ftwi;

	if (argc != 2)
		exit(1);

	FtwInit(&ftwi);
	rc = Ftw(&ftwi, argv[1], MyFtwProc);
	if (rc < 0)
		perror(argv[1]);
	printf("rc=%d depth=%u dircount=%u filecount=%u\n", rc, ftwi.maxDepth, ftwi.numDirs, ftwi.numFiles);
	FtwDispose(&ftwi);
	exit((rc < 0) ? 1 : 0);
}	/* main */

#endif	/* TEST */
