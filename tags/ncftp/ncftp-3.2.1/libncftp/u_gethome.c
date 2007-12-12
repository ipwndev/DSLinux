/* u_gethome.c
 *
 * Copyright (c) 1996-2006 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
extern void GetSpecialDir(char *dst, size_t size, int whichDir);
#endif

void
GetHomeDir(char *const dst, const size_t size)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	const char *homedrive, *homepath;
	
	homepath = getenv("USERPROFILE");	/* Windows XP */
	if (homepath != NULL) {
		(void) Strncpy(dst, homepath, size);
		return;
	}

	homedrive = getenv("HOMEDRIVE");
	homepath = getenv("HOMEPATH");
	if ((homedrive != NULL) && (homepath != NULL)) {
		(void) Strncpy(dst, homedrive, size);
		(void) Strncat(dst, homepath, size);
		return;
	}

	GetSpecialDir(dst, size, CSIDL_PERSONAL	/* "My Documents" */);
	if (dst[0] != '\0')
		return;

	dst[0] = '\0';
	if (GetWindowsDirectory(dst, size - 1) < 1)
		(void) Strncpy(dst, ".", size);
	else if (dst[1] == ':') {
		dst[2] = '\\';
		dst[3] = '\0';
	}
#else
	struct passwd pw;
	char pwbuf[256];

	if (GetMyPwEnt(&pw, pwbuf, sizeof(pwbuf)) == 0) {
		(void) Strncpy(dst, pw.pw_dir, size);
	} else {
		(void) Strncpy(dst, ".", size);
	}
#endif
}	/* GetHomeDir */




void
GetTmpDir(char *const dst, const size_t size)
{
	static const char *envvars[] = {"TMPDIR", "TMP", "TEMP", NULL};
	const char *tdir;
	int i;
	struct Stat st;

	memset(dst, 0, size);

	for (i = 0; envvars[i] != NULL; i++) {
		tdir = getenv(envvars[i]);
		if ((tdir == NULL) || (tdir[0] == '\0'))
			continue;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		if (tdir[0] != '/')
			continue;
#endif
		if ((Stat(tdir, &st) >= 0) && (S_ISDIR(st.st_mode))) {
			(void) Strncpy(dst, tdir, size);
			return;
		}
	}

	/* No suitable environment variable found. */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	ifdef CSIDL_WINDOWS
	memset(dst, 0, size);
	GetSpecialDir(dst, size, CSIDL_WINDOWS /* "C:\WINDOWS" */);
	if (dst[0] != '\0') {
		(void) Strncat(dst, "\\TEMP", size);
		if ((Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) {
			return;
		}
	}
#	else
	(void) Strncpy(dst, "C:\\WINDOWS\\TEMP", size);
	if ((Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) 
		return;
	(void) Strncpy(dst, "C:\\WINNT\\TEMP", size);
	if ((Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) 
		return;
#	endif

#	ifdef CSIDL_INTERNET_CACHE
	memset(dst, 0, size);
	GetSpecialDir(dst, size, CSIDL_INTERNET_CACHE /* "Temporary Internet Files" */);
	if ((dst[0] != '\0') && (Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) 
		return;
#	endif

	(void) Strncpy(dst, "\\TEMP", size);
	if ((Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) 
		return;
#else
	(void) Strncpy(dst, "/tmp", size);
	if ((Stat(dst, &st) >= 0) && (S_ISDIR(st.st_mode))) 
		return;
#endif
	memset(dst, 0, size);	/* return empty string */
}	/* GetTmpDir */
