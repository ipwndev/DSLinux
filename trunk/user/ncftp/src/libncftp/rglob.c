/* rglob.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif


static void
StripUnneccesaryGlobEntries(const FTPCIPtr cip, FTPLineListPtr fileList)
{
	FTPLinePtr lp, nextLine;
	const char *cp;

	for (lp=fileList->first; lp != NULL; lp = nextLine) {
		nextLine = lp->next;
		cp = strrchr(lp->line, '/');
		if (cp == NULL)
			cp = strrchr(lp->line, '\\');
		if (cp == NULL)
			cp = lp->line;
		else
			cp++;
		if ((strcmp(cp, ".") == 0) || (strcmp(cp, "..") == 0)) {
			PrintF(cip, "  Rglob omitted: [%s] (type 1)\n", lp->line);
			nextLine = RemoveLine(fileList, lp);
		}
	}
}	/* StripUnneccesaryGlobEntries */



int
PathContainsIntermediateDotDotSubDir(const char *s)
{
	int c;
	const char *a;

	if ((s[0] == '.') && (s[1] == '.')) {
		a = s + 2;
		while (*a == '.') a++;
		if (((*a == '/') || (*a == '\\')) || (*a == '\0'))
			return (1);
	}

	while (*s != '\0') {
		c = (int) *s++;
		if (((c == '/') || (c == '\\')) && (s[0] == '.') && (s[1] == '.')) {
			a = s + 2;

			/* Windows also treats '...', '....', etc as '..'
			 * so check for these too.  It doesn't matter on
			 * UNIX, but if those come back someone is up to
			 * no good.
			 */
			while (*a == '.') a++;
			if (((*a == '/') || (*a == '\\')) || (*a == '\0'))
				return (1);
		}
	}

	return (0);
}	/* PathContainsIntermediateDotDotSubDir */




/* We need to use this because using NLST gives us more stuff than
 * we want back sometimes.  For example, say we have:
 *
 * /a		(directory)
 * /a/b		(directory)
 * /a/b/b1
 * /a/b/b2
 * /a/b/b3
 * /a/c		(directory)
 * /a/c/c1
 * /a/c/c2
 * /a/c/c3
 * /a/file
 *
 * If you did an "echo /a/<star>" in a normal unix shell, you would expect
 * to get back /a/b /a/c /a/file.  But NLST gives back:
 *
 * /a/b/b1
 * /a/b/b2
 * /a/b/b3
 * /a/c/c1
 * /a/c/c2
 * /a/c/c3
 * /a/file
 *
 * So we use the following routine to convert into the format I expect.
 */

void
RemoteGlobCollapse(const FTPCIPtr cip, const char *pattern, FTPLineListPtr fileList)
{
	FTPLinePtr lp, nextLine;
	char *patPrefix;
	char *patDir;
	char *cur, *prev;
	char *cp;
	char *newpath;
	size_t plen;

	/* Copy all characters before and including the last path delimiter. */
	patDir = NULL;	
	cp = StrRFindLocalPathDelim(pattern);
	if (cp != NULL) {
		patDir = StrDup(pattern);
		if (patDir == NULL)
			return;
		patDir[(cp - pattern) + 1] = '\0';
	}

	/* Copy all characters before the first glob-char. */
	cp = strpbrk(pattern, kGlobChars);
	patPrefix = StrDup(pattern);
	if (patPrefix == NULL) {
		free(patDir);
		return;
	}
	if (cp != NULL) {
		plen = (size_t) (cp - pattern);
		patPrefix[plen] = '\0';
	} else {
		plen = strlen(patPrefix);
	}

	cur = prev = NULL;
	for (lp=fileList->first; lp != NULL; lp = nextLine) {
		nextLine = lp->next;
		if (ISTRNEQ(lp->line, patPrefix, plen)) {
			if (Dynsrecpy(&cur, lp->line + plen, 0) == NULL)
				goto done;
			cp = strpbrk(cur, "/\\");
			if (cp != NULL)
				*cp = '\0';
			if ((prev != NULL) && (STREQ(cur, prev))) {
				PrintF(cip, "  Rglob omitted: [%s] (type 2)\n", lp->line);
				nextLine = RemoveLine(fileList, lp);
			} else if (PathContainsIntermediateDotDotSubDir(lp->line + plen)) {
				PrintF(cip, "  Rglob omitted: [%s] (type 3)\n", lp->line);
				nextLine = RemoveLine(fileList, lp);
			} else {
				if (Dynsrecpy(&prev, cur, 0) == NULL)
					goto done;

				/* We are playing with a dynamically
				 * allocated string, but since the
				 * following expression is guaranteed
				 * to be the same or shorter, we won't
				 * overwrite the bounds.
				 */
				(void) sprintf(lp->line, "%s%s", patPrefix, cur);
			}
		} else if (strpbrk(lp->line, "/\\") == NULL) {
			if (patDir != NULL) {
				newpath = NULL;
				if (Dynsrecpy(&newpath, patDir, lp->line, 0) == NULL)
					goto done;
				PrintF(cip, "  Rglob changed: [%s] to [%s]\n", lp->line, newpath);
				free(lp->line);
				lp->line = newpath;
			}
		} else {
			PrintF(cip, "  Rglob omitted: [%s] (type 4)\n", lp->line);
			nextLine = RemoveLine(fileList, lp);
		}
	}

done:
	StrFree(&patDir);
	StrFree(&patPrefix);
	StrFree(&cur);
	StrFree(&prev);
}	/* RemoteGlobCollapse */




int
FTPRemoteGlob(FTPCIPtr cip, FTPLineListPtr fileList, const char *pattern, int doGlob)
{
	char *cp;
	const char *lsflags;
	FTPLinePtr lp;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (fileList == NULL)
		return (kErrBadParameter);
	InitLineList(fileList);

	if ((pattern == NULL) || (pattern[0] == '\0'))
		return (kErrBadParameter);

	/* Note that we do attempt to use glob characters even if the remote
	 * host isn't UNIX.  Most non-UNIX remote FTP servers look for UNIX
	 * style wildcards.
	 */
	if ((doGlob == 1) && (GLOBCHARSINSTR(pattern))) {
		/* Use NLST, which lists files one per line. */
		lsflags = "";
		
		/* Optimize for "NLST *" case which is same as "NLST". */
		if (strcmp(pattern, "*") == 0) {
			pattern = "";
			lsflags = (cip->hasNLST_a == kCommandNotAvailable) ? "" : "-a";
		} else if (strcmp(pattern, "**") == 0) {
			/* Hack; Lets you try "NLST -a" if you're daring. */
			/* Need to use "NLST -a" whenever possible,
			 * because wu-ftpd doesn't do NLST right, IMHO.
			 * (It doesn't include directories in the NLST
			 *  if you do "NLST /the/dir" without -a.)
			 */
			pattern = "";
			lsflags = (cip->hasNLST_a == kCommandNotAvailable) ? "" : "-a";
		}

		if ((result = FTPListToMemory2(cip, pattern, fileList, lsflags, 0, (int *) 0)) < 0) {
			if (*lsflags == '\0')
				return (result);
			/* Try again, without "-a" */
			cip->hasNLST_a = kCommandNotAvailable;
			lsflags = "";
			if ((result = FTPListToMemory2(cip, pattern, fileList, lsflags, 0, (int *) 0)) < 0) {
				return (result);
			}
		}
#if 0
		DisposeLineListContents(fileList);
		InitLineList(fileList);
		AddLine(fileList, "../FAKEME1.txt");
		AddLine(fileList, "../../FAKEME2.txt");
		AddLine(fileList, "..\\FAKEME3.txt");
		AddLine(fileList, "..\\..\\FAKEME4.txt");
		AddLine(fileList, "...\\FAKEME5.txt");
		AddLine(fileList, "/tmp/bad/FAKEME6.txt");
		AddLine(fileList, "c:\\temp\\FAKEME7.txt");
		AddLine(fileList, "foo/../FAKEME8.txt");
		AddLine(fileList, "foo\\bar\\...\\FAKEME9.txt");
#endif
		if (fileList->first == NULL) {
			cip->errNo = kErrGlobNoMatch;
			return (kErrGlobNoMatch);
		}
		if (fileList->first == fileList->last) {
#define glberr(a) (ISTRNEQ(cp, a, strlen(a)))
			/* If we have only one item in the list, see if it really was
			 * an error message we would recognize.
			 */
			cp = strchr(fileList->first->line, ':');
			if (cp != NULL) {
				if (glberr(": No such file or directory")) {
					(void) RemoveLine(fileList, fileList->first);
					cip->errNo = kErrGlobFailed;
					return (kErrGlobFailed);
				} else if (glberr(": No match")) {
					cip->errNo = kErrGlobNoMatch;
					return (kErrGlobNoMatch);
				}
			}
		}
		StripUnneccesaryGlobEntries(cip, fileList);
		RemoteGlobCollapse(cip, pattern, fileList);
		for (lp=fileList->first; lp != NULL; lp = lp->next)
			PrintF(cip, "  Rglob [%s]\n", lp->line);
	} else {
		/* Or, if there were no globbing characters in 'pattern', then the
		 * pattern is really just a filename.  So for this case the
		 * file list is really just a single file.
		 */
		fileList->first = fileList->last = NULL;
		(void) AddLine(fileList, pattern);
	}
	return (kNoErr);
}	/* FTPRemoteGlob */
