/* u_misc.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

static int wsaInit = 0;
#if defined(WIN32) || defined(_WINDOWS)
WSADATA wsaData;
#endif



void
InitWinsock(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
#	ifdef _CONSOLE
		fprintf(stderr, "could not initialize winsock\n");
#	endif
		exit(1);
	}
#endif
	wsaInit++;
}	/* InitWinsock */



void
DisposeWinsock(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (wsaInit > 0) { WSACleanup(); wsaInit--; }
#else
	wsaInit--;
#endif
}	/* DisposeWinsock */




#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)


int gettimeofday(struct timeval *const tp, void *junk)
{
	SYSTEMTIME systemTime;

	GetSystemTime(&systemTime);

	/* Create an arbitrary second counter;
	 * Note that this particular one creates
	 * a problem at the end of the month.
	 */
	tp->tv_sec =
		systemTime.wSecond +
		systemTime.wMinute * 60 +
		systemTime.wHour * 3600 +
		systemTime.wDay * 86400;

	tp->tv_usec = systemTime.wMilliseconds * 1000;

	return 0;
}	/* gettimeofday */




void WinSleep(unsigned int seconds)
{
	DWORD now, deadline;
	DWORD milliseconds = seconds * 1000;

	if (milliseconds > 0) {
		now = GetTickCount();
		deadline = now + milliseconds;
		if (now < deadline) {
			/* Typical case */
			do {
				milliseconds = deadline - now;
				Sleep(milliseconds);
				now = GetTickCount();
			} while (now < deadline);
		} else {
			/* Overflow case */
			deadline = now - 1;
			milliseconds -= (0xFFFFFFFF - now);
			do {
				Sleep(0xFFFFFFFF - now);
				now = GetTickCount();
			} while (now > deadline);
			/* Counter has now wrapped around */
			deadline = now + milliseconds;
			do {
				milliseconds = deadline - now;
				Sleep(milliseconds);
				now = GetTickCount();
			} while (now < deadline);
		}
	}
}	/* WinSleep */
#endif



#if defined(WIN32) || defined(_WINDOWS) || defined(__CYGWIN__)
char *
StrFindLocalPathDelim(const char *src) /* TODO: optimize */
{
	const char *first;
	int c;

	first = NULL;
	for (;;) {
		c = *src++;
		if (c == '\0')
			break;
		if (IsLocalPathDelim(c)) {
			first = src - 1;
			break;
		}
	}

	return ((char *) first);
}	/* StrFindLocalPathDelim */



char *
StrRFindLocalPathDelim(const char *src)	/* TODO: optimize */
{
	const char *last;
	int c;

	last = NULL;
	for (;;) {
		c = *src++;
		if (c == '\0')
			break;
		if (IsLocalPathDelim(c))
			last = src - 1;
	}

	return ((char *) last);
}	/* StrRFindLocalPathDelim */




void
StrRemoveTrailingLocalPathDelim(char *dst)
{
	char *cp;

	cp = StrRFindLocalPathDelim(dst);
	if ((cp == NULL) || (cp[1] != '\0'))
		return;

	/* Note: Do not destroy a path of "/" */
	while ((cp > dst) && (IsLocalPathDelim(*cp)))
		*cp-- = '\0';
}	/* StrRemoveTrailingLocalPathDelim */



void
TVFSPathToLocalPath(char *dst)
{
	int c;

	/* Note: Technically we don't need to do this,
	 * since Win32 accepts a / as equivalent to a \
	 * in a pathname.
	 */
	if (dst != NULL) {
		for (;;) {
			c = *dst++;
			if (c == '\0')
				break;
			if (c == '/')
				dst[-1] = LOCAL_PATH_DELIM;
		}
	}
}	/* TVFSPathToLocalPath */


void
LocalPathToTVFSPath(char *dst)
{
	int c;

	if (dst != NULL) {
		for (;;) {
			c = *dst++;
			if (c == '\0')
				break;
			if (c == LOCAL_PATH_DELIM)
				dst[-1] = '/';
		}
	}
}	/* LocalPathToTVFSPath */
#endif





#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
int
WinFStat64(const int h0, struct WinStat64 *const stp)
{
	HANDLE h;
	__int64 fSize;
	DWORD fSize1, fSize2;
	struct _stat st32;
	DWORD winErr;

	h = (HANDLE) _get_osfhandle(h0);
	if (h == INVALID_HANDLE_VALUE)
		return (-1);

	memset(stp, 0, sizeof(struct WinStat64));
	if (_fstat(h0, &st32) < 0)
		return (-1);
	stp->st_atime = st32.st_atime;
	stp->st_ctime = st32.st_ctime;
	stp->st_dev = st32.st_dev;
	stp->st_gid = st32.st_gid;
	stp->st_ino = st32.st_ino;
	stp->st_mode = st32.st_mode;
	stp->st_mtime = st32.st_mtime;
	stp->st_nlink = st32.st_nlink;
	stp->st_rdev = st32.st_rdev;
	stp->st_uid = st32.st_uid;

	if (S_ISREG(stp->st_mode)) {
		fSize = (_int64)0;
		fSize1 = GetFileSize(h, &fSize2);
		if ((fSize1 == 0xFFFFFFFF) && ((winErr = GetLastError()) != NO_ERROR))
			goto return_err;

		fSize = ((__int64) fSize2 << 32) | (__int64) fSize1;
		stp->st_size = fSize;
	}
	return (0);

return_err:
	stp->st_size = (__int32) st32.st_size;
	if ((winErr = GetLastError()) == ERROR_SHARING_VIOLATION) {
		errno = EBUSY;
	} else if ((winErr == ERROR_PATH_NOT_FOUND) || (winErr == ERROR_FILE_NOT_FOUND)) {
		errno = ENOENT;
	} else if (winErr == ERROR_INVALID_PARAMETER) {
		errno = EINVAL;
	} else {
		errno = 100000 + winErr;
	}
	return (-1);
}	/* WinFStat64 */




int
WinStat64(const char *const path, struct WinStat64 *const stp)
{
	HANDLE h;
	__int64 fSize;
	DWORD fSize1, fSize2;
	struct _stat st32;
	DWORD winErr;

	memset(stp, 0, sizeof(struct WinStat64));
	if (_stat(path, &st32) < 0)
		return (-1);
	stp->st_atime = st32.st_atime;
	stp->st_ctime = st32.st_ctime;
	stp->st_dev = st32.st_dev;
	stp->st_gid = st32.st_gid;
	stp->st_ino = st32.st_ino;
	stp->st_mode = st32.st_mode;
	stp->st_mtime = st32.st_mtime;
	stp->st_nlink = st32.st_nlink;
	stp->st_rdev = st32.st_rdev;
	stp->st_uid = st32.st_uid;

	if (S_ISREG(stp->st_mode)) {
		h = CreateFile(path,
			0,				/* Not GENERIC_READ; use 0 for "query attributes only" mode */
			0,
			NULL,
			OPEN_EXISTING,	/* fails if it doesn't exist */
			0,
			NULL
			);
		
		if (h == INVALID_HANDLE_VALUE)
			goto return_err;

		fSize = (_int64)0;
		fSize1 = GetFileSize(h, &fSize2);
		if ((fSize1 == 0xFFFFFFFF) && ((winErr = GetLastError()) != NO_ERROR))
			goto return_err;

		fSize = ((__int64) fSize2 << 32) | (__int64) fSize1;
		stp->st_size = fSize;
		CloseHandle(h);
	}
	return (0);

return_err:
	stp->st_size = (__int32) st32.st_size;
	if ((winErr = GetLastError()) == ERROR_SHARING_VIOLATION) {
		errno = EBUSY;
	} else if ((winErr == ERROR_PATH_NOT_FOUND) || (winErr == ERROR_FILE_NOT_FOUND)) {
		errno = ENOENT;
	} else if (winErr == ERROR_INVALID_PARAMETER) {
		errno = EINVAL;
	} else {
		errno = 100000 + winErr;
	}

	if (h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
	return (-1);
}	/* WinStat64 */

#endif
