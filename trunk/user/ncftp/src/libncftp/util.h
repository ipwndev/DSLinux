/* Util.h
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#ifndef _util_h_
#define _util_h_ 1

typedef char string[160], str16[16], str32[32], str64[64];
typedef char longstring[512];
typedef char pathname[512];

#ifndef PTRZERO
#	define PTRZERO(p,siz)  (void) memset(p, 0, (size_t) (siz))
#endif

#define ZERO(a)	PTRZERO(&(a), sizeof(a))
#define STREQ(a,b) (strcmp(a,b) == 0)
#define STRNEQ(a,b,s) (strncmp(a,b,(size_t)(s)) == 0)

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define ISTRCMP stricmp
#	define ISTRNCMP strnicmp
#endif

#ifndef ISTRCMP
#	ifdef HAVE_STRCASECMP
#		define ISTRCMP strcasecmp
#		define ISTRNCMP strncasecmp
#	else
#		define ISTRCMP strcmp
#		define ISTRNCMP strncmp
#	endif
#endif

#define ISTREQ(a,b) (ISTRCMP(a,b) == 0)
#define ISTRNEQ(a,b,s) (ISTRNCMP(a,b,(size_t)(s)) == 0)

typedef int (*cmp_t)(const void *, const void *);
#define QSORT(base,n,s,cmp) \
	qsort(base, (size_t)(n), (size_t)(s), (cmp_t)(cmp))

#define BSEARCH(key,base,n,s,cmp) \
	bsearch(key, base, (size_t)(n), (size_t)(s), (cmp_t)(cmp))

/* For FTPLogError(): */
#define kDoPerror		1
#define kDontPerror		0

#define kClosedFileDescriptor (-1)

#define SZ(a) ((size_t) (a))

#ifndef F_OK
#	define F_OK 0
#endif

#ifdef HAVE_REMOVE
#	define UNLINK remove
#else
#	define UNLINK unlink
#endif

#ifndef SEEK_SET
#	define SEEK_SET    0
#	define SEEK_CUR    1
#	define SEEK_END    2
#endif  /* SEEK_SET */

#ifdef SETVBUF_REVERSED
#	define SETVBUF(a,b,c,d) setvbuf(a,c,b,d)
#else
#	define SETVBUF setvbuf
#endif


/* Util.c */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
int GetMyPwEnt(struct passwd *pwp, char *const pwbuf, size_t pwbufsize);
int GetPwUid(struct passwd *pwp, const uid_t uid, char *const pwbuf, size_t pwbufsize);
int GetPwNam(struct passwd *pwp, const char *const nam, char *const pwbuf, size_t pwbufsize);
#endif
void CloseFile(FILE **);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
int gettimeofday(struct timeval *const tp, void *junk);
#endif

/* io_util.c */
void AutomaticallyUseASCIIModeDependingOnExtension(const FTPCIPtr cip, const char *const pathName, int *const xtype);
void FTPCheckForRestartModeAvailability(const FTPCIPtr cip);
int WaitForRemoteInput(const FTPCIPtr cip);
int WaitForRemoteOutput(const FTPCIPtr cip);
void FTPSetUploadSocketBufferSize(const FTPCIPtr cip);

/* io_get.c, io_put.c */
int
FTPGetOneF(
	const FTPCIPtr cip,
	const char *const file,
	const char *dstfile,
	int xtype,
	const int fdtouse,
	longest_int expectedSize,
	time_t mdtm,
	const int resumeflag,
	const int appendflag,
	const int deleteflag,
	const FTPConfirmResumeDownloadProc resumeProc);

int
FTPPutOneF(
	const FTPCIPtr cip,
	const char *const file,
	const char *volatile dstfile,
	int xtype,
	const int fdtouse,
	const int appendflag,
	const char *volatile tmppfx,
	const char *volatile tmpsfx,
	const int resumeflag,
	const int deleteflag,
	const FTPConfirmResumeUploadProc resumeProc);

int FTPGetOneTarF(const FTPCIPtr cip, const char *file, const char *const dstdir);

/* open.c */
void FTPDeallocateHost(const FTPCIPtr cip);
int FTPAllocateHost(const FTPCIPtr cip);

/* cmds misc */
int FTPRmdirRecursive(const FTPCIPtr cip, const char *const dir);
void FTPRequestMlsOptions(const FTPCIPtr cip);
void RemoteGlobCollapse(const FTPCIPtr, const char *pattern, FTPLineListPtr fileList);
int PathContainsIntermediateDotDotSubDir(const char *s);

#endif	/* _util_h_ */
