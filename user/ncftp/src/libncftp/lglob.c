/* lglob.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* This does "tilde-expansion."  Examples:
 * ~/pub         -->  /usr/gleason/pub
 * ~pdietz/junk  -->  /usr/pdietz/junk
 */
static void
ExpandTilde(char *pattern, size_t siz)
{
	char pat[512];
	char *cp, *rest, *firstent;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	struct passwd pw;
	char pwbuf[256];
#endif
	char hdir[512];

	if ((pattern[0] == '~') &&
	(isalnum((int) pattern[1]) || IsLocalPathDelim(pattern[1]) || (pattern[1] == '\0'))) {
		(void) STRNCPY(pat, pattern);
		if ((cp = StrFindLocalPathDelim(pat)) != NULL) {
			*cp = 0;
			rest = cp + 1;	/* Remember stuff after the ~/ part. */
		} else {
			rest = NULL;	/* Was just a ~ or ~username.  */
		}
		if (pat[1] == '\0') {
			/* Was just a ~ or ~/rest type.  */
			GetHomeDir(hdir, sizeof(hdir));
			firstent = hdir;
		} else {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			return;
#else
			/* Was just a ~username or ~username/rest type.  */
			if (GetPwNam(&pw, pat + 1, pwbuf, sizeof(pwbuf)) == 0)
				firstent = pw.pw_dir;
			else
				return;		/* Bad user -- leave it alone. */
#endif
		}
		
		(void) Strncpy(pattern, firstent, siz);
		if (rest != NULL) {
			(void) Strncat(pattern, LOCAL_PATH_DELIM_STR, siz);
			(void) Strncat(pattern, rest, siz);
		}
	}
}	/* ExpandTilde */





#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)

static int
WinLocalGlob(FTPCIPtr cip, FTPLineListPtr fileList, const char *const srcpat)
{
	char pattern[_MAX_PATH];
	WIN32_FIND_DATA ffd;
	HANDLE searchHandle;
	DWORD dwErr;
	char *cp;
	const char *file;
	int result;

	STRNCPY(pattern, srcpat);

	/* Get rid of trailing slashes. */
	cp = pattern + strlen(pattern) - 1;
	while ((cp >= pattern) && IsLocalPathDelim(*cp))
		*cp-- = '\0';

	memset(&ffd, 0, sizeof(ffd));

	/* "Open" the directory. */
	searchHandle = FindFirstFile(pattern, &ffd);
	if (searchHandle == INVALID_HANDLE_VALUE) {
		dwErr = GetLastError();
		return ((dwErr == 0) ? 0 : -1);
	}

	/* Get rid of basename. */
	cp = StrRFindLocalPathDelim(pattern);
	if (cp == NULL)
		cp = pattern;
	else
		cp++;
	*cp = '\0';

	for (result = 0;;) {
		file = ffd.cFileName;
		if ((file[0] == '.') && ((file[1] == '\0') || ((file[1] == '.') && (file[2] == '\0')))) {
			/* skip */
		} else {
			Strncpy(cp, ffd.cFileName, sizeof(pattern) - (cp - pattern));
			PrintF(cip, "  Lglob [%s]\n", pattern);
			(void) AddLine(fileList, pattern);
		}

		if (!FindNextFile(searchHandle, &ffd)) {
			dwErr = GetLastError();
			if (dwErr != ERROR_NO_MORE_FILES) {
				result = ((dwErr == 0) ? 0 : -1);
			}
			break;
		}
	}

	FindClose(searchHandle);
	return (result);
}	// WinLocalGlob

#else

static int
LazyUnixLocalGlob(FTPCIPtr cip, FTPLineListPtr fileList, const char *const pattern)
{
	char cmd[512];
	longstring gfile;
	FILE *fp;
	FTPSigProc sp;
	
	/* Do it the easy way and have the shell do the dirty
	 * work for us.
	 */
#ifdef HAVE_SNPRINTF
	(void) snprintf(cmd, sizeof(cmd) - 1, "%s -c \"%s %s %s\"", "/bin/sh", "/bin/ls",
		"-d", pattern);
	cmd[sizeof(cmd) - 1] = '\0';
#else
	(void) sprintf(cmd, "%s -c \"%s %s %s\"", "/bin/sh", "/bin/ls",
		"-d", pattern);
#endif
	
	fp = (FILE *) popen(cmd, "r");
	if (fp == NULL) {
		FTPLogError(cip, kDoPerror, "Could not Lglob: [%s]\n", cmd);
		cip->errNo = kErrGlobFailed;
		return (kErrGlobFailed);
	}
	sp = NcSignal(SIGPIPE, (FTPSigProc) SIG_IGN);
	while (FGets(gfile, sizeof(gfile), (FILE *) fp) != NULL) {
		PrintF(cip, "  Lglob [%s]\n", gfile);
		(void) AddLine(fileList, gfile);
	}
	(void) pclose(fp);
	(void) NcSignal(SIGPIPE, sp);
	return (kNoErr);
}	/* LazyUnixLocalGlob */

#endif




int
FTPLocalGlob(FTPCIPtr cip, FTPLineListPtr fileList, const char *pattern, int doGlob)
{
	char pattern2[512];
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

	(void) STRNCPY(pattern2, pattern);	/* Don't nuke the original. */
	
	/* Pre-process for ~'s. */ 
	ExpandTilde(pattern2, sizeof(pattern2));
	InitLineList(fileList);
	result = kNoErr;

	if ((doGlob == 1) && (GLOBCHARSINSTR(pattern2))) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		result = WinLocalGlob(cip, fileList, pattern2);
#else
		result = LazyUnixLocalGlob(cip, fileList, pattern2);
#endif
	} else {
		/* Or, if there were no globbing characters in 'pattern', then
		 * the pattern is really just a single pathname.
		 */
		(void) AddLine(fileList, pattern2);
	}

	return (result);
}	/* FTPLocalGlob */
