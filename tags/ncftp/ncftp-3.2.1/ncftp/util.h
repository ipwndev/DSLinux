/* util.h
 *
 * Copyright (c) 1992-2006 by Mike Gleason.
 * All rights reserved.
 * 
 */

typedef int (*qsort_proc_t)(const void *, const void *);
typedef int (*bsearch_proc_t)(const void *, const void *);
typedef void (*sigproc_t)(int);
typedef volatile sigproc_t vsigproc_t;

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
#define YESNO(i) ((i == 0) ? "no" : "yes")
#define ONOFF(i) ((i == 0) ? "off" : "on")
#define TRUEFALSE(i) ((i == 0) ? "false" : "true")

#ifndef HAVE_STRCOLL
#	ifndef strcoll
#		define strcoll strcmp
#	endif
#	ifndef strncoll
#		define strncoll strncmp
#	endif
#endif

#ifndef F_OK
#	define F_OK 0
#endif


#define kPasswordMagic "*encoded*"
#define kPasswordMagicLen 9

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kOurDirectoryName	"ncftp"
#	define kNcFTPBookmarksMailslot "\\\\.\\mailslot\\ncftpbm.slt"
#	define kNcFTPBookmarksMailslotMsgSize 128
#else
#	define kOurDirectoryName	".ncftp"
#endif

extern int gNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings;

#if (defined(__APPLE_CC__)) && (__APPLE_CC__ < 10000)
#	define NCFTP_USE_VAR(a) a = 0; gNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#elif (defined(__GNUC__)) && (__GNUC__ >= 3)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((__unused__))
#	endif
#	define NCFTP_USE_VAR(a)
#elif (defined(__GNUC__)) && (__GNUC__ == 2)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((unused))
#	endif
#	define NCFTP_USE_VAR(a)
#else
#	define NCFTP_USE_VAR(a) a = 0; gNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#endif

/* util.c */
void ToBase64(void *, const void *, size_t, int);
void FromBase64(void *, const void *, size_t, int);
void OutOfMemory(void);
void *Realloc(void *, size_t);
char *GetCWD(char *, size_t);
void MyInetAddr(char *, size_t, void *, int);
char *FileToURL(char *url, size_t urlsize, const char *const fn, const char *const rcwd, const char *const startdir, const char *const user, const char *const pass, const char *const hname, const unsigned int port);
char *TolowerStr(char *dst);
void AbbrevStr(char *, const char *, size_t, int);
char *Path(char *const dst, const size_t siz, const char *const parent, const char *const fname);
char *OurDirectoryPath(char *const dst, const size_t siz, const char *const fname);
void InitOurDirectory(void);
void InitUserInfo(void);
int MayUseFirewall(const char *const, int, const char *const);
int StrToBool(const char *const);
void AbsoluteToRelative(char *const, const size_t, const char *const, const char *const, const size_t);
int MyGetHostByName(char *const volatile, size_t, const char *const, int);
time_t UnDate(char *dstr);
int DecodeDirectoryURL(const FTPCIPtr, char *, FTPLineListPtr, char *, size_t);
char *OurInstallationPath(char *const dst, const size_t siz, const char *const fname);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
void SysPerror(const char *const errMsg);
#endif

#if defined(HAVE_STRCOLL) && !defined(HAVE_STRNCOLL)
int strncoll(const char *a, const char *b, size_t n);
#endif
