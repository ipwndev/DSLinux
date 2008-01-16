/* ncftpbatch.c
 * 
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	INITCOMMONCONTROLSEX gComCtls;
	HINSTANCE ghInstance = 0;
	HWND gMainWnd = 0;
	HWND gStaticCtrl = 0;
#	include "..\ncftp\util.h"
#	include "..\ncftp\pref.h"
#	include "..\ncftp\spool.h"
#	include "resource.h"
#	include "gpshare.h"
#else
#	define YieldUI(a)
#	include "../ncftp/util.h"
#	include "../ncftp/pref.h"
#	include "../ncftp/spool.h"
#	include "gpshare.h"
#endif

#ifdef HAVE_LONG_FILE_NAMES

#define kSpoolDir "spool"
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kSpoolLog "log.txt"
#else
#	define kSpoolLog "log"
#	define kGlobalSpoolDir "/var/spool/ncftp"
#endif
#define NEW_SLEEP_VAL(s) ((unsigned int) (((0.1 * (rand() % 15)) + 1.2) * (s))) 

int gQuitRequested = 0;
int gGlobalSpooler = 0;
long gMaxLogSize = 200000L;
unsigned int gDelayBetweenPasses = 0;
int gGotSig = 0;
FTPLibraryInfo gLib;
FTPConnectionInfo gConn;
int gIsTTY;
int gSpooled = 0;
char gSpoolDir[256];
char gLogFileName[256];
struct dirent *gDirentBuf = NULL;
size_t gDirentBufSize = 0;
extern int gFirewallType;
extern char gFirewallHost[64];
extern char gFirewallUser[32];
extern char gFirewallPass[32];
extern char gFirewallExceptionList[256];
extern unsigned int gFirewallPort;
extern int gFwDataPortMode;
int gItemInUse = 0;
char gItemPath[256];
char *gItemContents = NULL;
size_t gItemContentsAllocSize = 0;
size_t gItemContentsSize = 0;
char gMyItemPath[256];
int gOperation;
char gOperationStr[16];
unsigned int gDelaySinceLastFailure;
char gHost[64];
char gLastHost[64];
char gHostIP[32];
unsigned int gPort;
char gRUser[128];
char gRPass[128];
char gRAcct[128];
char gManualOverrideFeatures[256];
char gPreFTPCommand[128];
char gPerFileFTPCommand[128];
char gPostFTPCommand[128];
char gPreShellCommand[256];
char gPostShellCommand[256];
int gXtype;
int gRecursive;
int gDelete;
int gPassive;
char gRDir[256];
char gLDir[256];
char gRFile[256];
char gLFile[256];
char gRStartDir[256];

/* Writes logging data to a ~/.ncftp/spool/log file.
 * This is nice for me when I need to diagnose problems.
 */
FILE *gLogFile = NULL;
time_t gLogTime;
char gLogLBuf[256];
unsigned int gMyPID;
const char *gLogOpenMode = FOPEN_APPEND_TEXT;
int gUnused;
int gMayCancelJmp = 0;
int gMaySigExit = 1;

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
char gStatusText[512];
#else
#ifdef HAVE_SIGSETJMP
sigjmp_buf gCancelJmp;
#else	/* HAVE_SIGSETJMP */
jmp_buf gCancelJmp;
#endif	/* HAVE_SIGSETJMP */
#endif

extern const char gOS[], gVersion[];

extern struct dirent *Readdir(DIR *const dir, struct dirent *const dp, const size_t sz);
static void ErrBox(const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 1, 2)))
#endif
;

static void
Log(
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		int uiShow,
#else
		int UNUSED(uiShow),
#endif
		const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;

static void LogPerror(const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 1, 2)))
#endif
;

static void LogEndItemResult(int uiShow, const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;



#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
static void YieldUI(int redraw)
{
	MSG msg;

	if (redraw)
		InvalidateRect(gMainWnd, NULL, (redraw > 1));

	while (PeekMessage (&msg, gMainWnd, 0, 0, PM_REMOVE)) {
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}	// YieldUI
#endif



static void ErrBox(const char *const fmt, ...)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	char buf[512];
	va_list ap;

	ZeroMemory(buf, sizeof(buf));
	va_start(ap, fmt);
	wvsprintf(buf, fmt, ap);
	va_end(ap);

	MessageBox((gMainWnd != NULL) ? gMainWnd : GetDesktopWindow(),
		buf, "Error", MB_OK | MB_ICONINFORMATION);
#else
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
#endif
}	/* ErrBox */




static void PerrorBox(const char *const whatFailed)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	char errMsg[256];

	(void) FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errMsg,
		sizeof(errMsg),
		NULL
	);

	(void) ErrBox("%s: %s\n", whatFailed, errMsg);
#else
	perror(whatFailed);
#endif
}	/* PerrorBox */



/*VARARGS*/
static void
Log(
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		int uiShow,
#else
		int UNUSED(uiShow),
#endif
		const char *const fmt, ...)
{
	va_list ap;
	struct tm lt;
	char tstr[128];
	
	if (gLogFile != NULL) {
		strftime(tstr, sizeof(tstr), "%Y-%m-%d %H:%M:%S %Z", Gmtime(time(&gLogTime), &lt));
		(void) fprintf(gLogFile,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			"%s [$%08x] | ",
#else
			"%s [%06u] | ",
#endif
			tstr,
			gMyPID
		);
		va_start(ap, fmt);
		(void) vfprintf(gLogFile, fmt, ap);
		va_end(ap);
	}
	if (gIsTTY != 0) {
		va_start(ap, fmt);
		(void) vfprintf(stdout, fmt, ap);
		va_end(ap);
	}
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (uiShow) {
		char *cp;

		va_start(ap, fmt);
		(void) vsprintf(gStatusText, fmt, ap);
		va_end(ap);
		cp = gStatusText + strlen(gStatusText) - 1;
		while (iscntrl(*cp)) {
			*cp-- = '\0';
		}
		YieldUI(2);
	}
#else
	LIBNCFTP_USE_VAR(uiShow);
#endif
}	/* Log */



/*VARARGS*/
static void
LogPerror(const char *const fmt, ...)
{
	va_list ap;
	struct tm lt;
	int oerrno;
	char tstr[128];
	
	oerrno = errno;
	if (gLogFile != NULL) {
		strftime(tstr, sizeof(tstr), "%Y-%m-%d %H:%M:%S %Z", Gmtime(time(&gLogTime), &lt));
		(void) fprintf(gLogFile,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			"%s [$%08x] | ",
#else
			"%s [%06u] | ",
#endif
			tstr,
			gMyPID
		);
		va_start(ap, fmt);
		(void) vfprintf(gLogFile, fmt, ap);
		va_end(ap);
#ifdef HAVE_STRERROR
		(void) fprintf(gLogFile, ": %s\n", strerror(oerrno));
#else
		(void) fprintf(gLogFile, ": errno=%d\n", (oerrno));
#endif
	}
	if (gIsTTY != 0) {
		va_start(ap, fmt);
		(void) vfprintf(stdout, fmt, ap);
		va_end(ap);
#ifdef HAVE_STRERROR
		(void) fprintf(stdout, ": %s\n", strerror(oerrno));
#else
		(void) fprintf(stdout, ": errno=%d\n", (oerrno));
#endif
	}
}	/* LogPerror */



#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
void
PrWinStatBar(const FTPCIPtr cip, int mode)
{
	switch (mode) {
		case kPrInitMsg:
			if (gLogFile != NULL)
				(void) fflush(gLogFile);
			YieldUI(2);
			break;

		case kPrUpdateMsg:
			YieldUI(1);
			break;

		case kPrEndMsg:
			if (gLogFile != NULL)
				(void) fflush(gLogFile);
			YieldUI(1);
			break;
	}
}	/* PrWinStatBar */
#endif



static void
DebugHook(const FTPCIPtr cipUnused, char *msg)
{
	gUnused = cipUnused != 0;			/* shut up gcc */
	Log(0, "  %s", msg);
}	/* DebugHook */




static void
CloseLog(void)
{
	if (gLogFile != NULL) {
		(void) fclose(gLogFile);
		gLogFile = NULL;
	}
}	/* CloseLog */




static int
OpenLog(void)
{
	FILE *fp;
	struct Stat st;
	const char *openMode;

	CloseLog();
	if ((gLogFileName[0] == '\0') || (strcasecmp(gLogFileName, "/dev/null") == 0))
		return (0);

	openMode = gLogOpenMode;
	if ((Stat(gLogFileName, &st) == 0) && (st.st_size > gMaxLogSize)) {
		/* Prevent the log file from growing forever. */
		openMode = FOPEN_WRITE_TEXT;
	}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	fp = _fsopen(gLogFileName, openMode, _SH_DENYNO);
#else
	fp = fopen(gLogFileName, openMode);
#endif

	if (fp != NULL) {
#ifdef HAVE_SETVBUF
		/* Note: On Win32, _IOLBF is the same as _IOFBF.  Bleeeeh.  */
		(void) setvbuf(fp, gLogLBuf, _IOLBF, sizeof(gLogLBuf));
#endif	/* HAVE_SETVBUF */
		(void) time(&gLogTime);
		gLogFile = fp;
		gMyPID = (unsigned int) getpid();
		return (0);
	}
	return (-1);
}	/* OpenLog */




static void
ExitStuff(void)
{
	if (gItemInUse > 0) {
		gItemInUse = 0;
		(void) rename(gMyItemPath, gItemPath);
	}
}	/* ExitStuff */



#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#elif 0
static void
SigAlrm(int sigNum)
{
	if (gMayCancelJmp != 0) {
		gUnused = sigNum;
		if (gItemInUse > 0) {
			gItemInUse = 0;
			(void) rename(gMyItemPath, gItemPath);
		}
#ifdef HAVE_SIGSETJMP
		siglongjmp(gCancelJmp, 1);
#else	/* HAVE_SIGSETJMP */
		longjmp(gCancelJmp, 1);
#endif	/* HAVE_SIGSETJMP */
	}
}	/* SigAlrm */
#endif




static void
SigExit(int sigNum)
{
	gQuitRequested = sigNum;
	if (gMaySigExit != 0) {
#ifdef SIGBUS
		if ((sigNum == SIGSEGV) || (sigNum == SIGBUS) || (sigNum == SIGILL)) {
#else
		if ((sigNum == SIGSEGV) || (sigNum == SIGILL)) {
#endif
			ExitStuff();
			Log(0, "-----caught signal %d, aborting-----\n", sigNum);
			DisposeWinsock();

			/* Need to do this, because we may have been
			 * in the root directory which we probably
			 * can't write the core file to.
			 */
			(void) chdir("/tmp");
			abort();
		} else {

			ExitStuff();
			Log(0, "-----caught signal %d, exiting-----\n", sigNum);
			DisposeWinsock();
			exit(0);
		}
	}
}	/* SigExit */





static void
FTPInit(void)
{
	int result;

	InitWinsock();
	result = FTPInitLibrary(&gLib);
	if (result < 0) {
		ErrBox("ncftpbatch: init library error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(1);
	}

	result = FTPInitConnectionInfo(&gLib, &gConn, kDefaultFTPBufSize);
	if (result < 0) {
		ErrBox("ncftpbatch: init connection info error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(1);
	}
}	/* FTPInit */




/* These things are done first, before we even parse the command-line
 * options.
 */
static void
PreInit(const char *const prog)
{
	const char *cp;

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	gIsTTY = 0;
	ZeroMemory(gStatusText, sizeof(gStatusText));
#else
	gIsTTY = ((isatty(2) != 0) && (getppid() > 1)) ? 1 : 0;
	umask(077);
#endif
#ifdef SIGPOLL
	NcSignal(SIGPOLL, (FTPSigProc) SIG_IGN);
#endif
	InitUserInfo();

	FTPInit();
	LoadFirewallPrefs(0);
	srand((unsigned int) getpid());
	gLogFileName[0] = '\0';

	cp = strrchr(prog, '/');
	if (cp == NULL)
		cp = strrchr(prog, '\\');
	if (cp == NULL)
		cp = prog;
	else
		cp++;
	if (strncasecmp(cp, "ncftpspool", 10) == 0)
		gGlobalSpooler = 1;

	(void) signal(SIGINT, SigExit);
	(void) signal(SIGTERM, SigExit);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	(void) signal(SIGSEGV, SigExit);
	(void) signal(SIGBUS, SigExit);
	(void) signal(SIGFPE, SigExit);
	(void) signal(SIGILL, SigExit);
#if defined(SIGIOT) && (SIGIOT != SIGABRT)
	(void) signal(SIGIOT, SigExit);
#endif
#ifdef SIGEMT
	(void) signal(SIGEMT, SigExit);
#endif
#ifdef SIGSYS
	(void) signal(SIGSYS, SigExit);
#endif
#ifdef SIGSTKFLT
	(void) signal(SIGSTKFLT, SigExit);
#endif
#endif
}	/* PreInit */



static void
PostInit(void)
{
	struct dirent *direntbuf;
	size_t debufsize;
#ifdef HAVE_PATHCONF
	long nmx;
#endif
	/* These things are done after parsing the command-line options. */

	if (gGlobalSpooler != 0) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		if (gSpoolDir[0] == '\0')
			STRNCPY(gSpoolDir, "/var/spool/ncftp");
		if ((chdir(gSpoolDir) < 0) && (mkdir(gSpoolDir, 00775) < 0)) {
			perror(gSpoolDir);
			exit(1);
		}
		(void) chdir("/");
#endif
		if (gDelayBetweenPasses == 0)
			gDelayBetweenPasses = 120;
	} else {
		if (gSpoolDir[0] == '\0')
			(void) OurDirectoryPath(gSpoolDir, sizeof(gSpoolDir), kSpoolDir);
	}

	if (gLogFileName[0] == '\0')
		(void) Path(gLogFileName, sizeof(gLogFileName), gSpoolDir, kSpoolLog);
	debufsize = 512;
#ifdef HAVE_PATHCONF
	nmx = pathconf(gLogFileName, _PC_NAME_MAX);
	if (nmx >= 512)
		debufsize = nmx;
#endif
	debufsize += sizeof(struct dirent) + 8;
	direntbuf = (struct dirent *) calloc(debufsize, (size_t) 1);
	if (direntbuf == NULL) {
		PerrorBox("malloc failed for dirent buffer");
		exit(1);
	}
	gDirentBuf = direntbuf;
	gDirentBufSize = debufsize;
}	/* PostInit */




/* These things are just before the program exits. */
static void
PostShell(void)
{
	CloseLog();
}	/* PostShell */




static int
LoadCurrentSpoolFileContents(int logErrors)
{
	FILE *fp;
	char line[256];
	char *tok1, *tok2;
	char *cp, *lim;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	struct stat st;
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	/* gItemContents is not used on Win32 */
	if ((fp = _fsopen(gMyItemPath, FOPEN_READ_BINARY, _SH_DENYNO)) == NULL) {
		/* Could have been renamed already. */
		if (logErrors != 0)
			LogPerror("%s", gMyItemPath);
		return (-1);
	}
#else
	if ((stat(gMyItemPath, &st) < 0) || ((fp = fopen(gMyItemPath, FOPEN_READ_BINARY)) == NULL)) {
		/* Could have been renamed already. */
		if (logErrors != 0)
			LogPerror("%s", gMyItemPath);
		return (-1);
	}

	/* Make sure our global buffer to contain the contents of
	 * the current spool file is ready to use.
	 */
	if ((size_t) st.st_size > gItemContentsAllocSize) {
		gItemContentsAllocSize = (size_t) (st.st_size + (4096 - (st.st_size % 4096)));
		if (gItemContentsAllocSize == 0) {
			cp = malloc(gItemContentsAllocSize);
		} else {
			cp = realloc(gItemContents, gItemContentsAllocSize);
		}
		if (cp == NULL) {
			if (logErrors != 0)
				LogPerror("malloc");
			return (-1);
		}
		memset(cp, 0, gItemContentsAllocSize);
		gItemContents = cp;
	} else {
		cp = gItemContents;
	}

	/* Load the entire image of the spool file. */
	gItemContentsSize = (size_t) st.st_size;
	if (fread(cp, (size_t) 1, gItemContentsSize, fp) != gItemContentsSize) {
		LogPerror("fread from %s", gMyItemPath);
		fclose(fp);
		return (-1);
	}
	memset(cp + gItemContentsSize, 0, gItemContentsAllocSize - gItemContentsSize);

	/* Rewind it since we have to re-read it for parsing. */
	if (fseek(fp, 0L, SEEK_SET) != 0L) {
		LogPerror("rewind %s", gMyItemPath);
		fclose(fp);
		return (-1);
	}
#endif /* UNIX */

	gOperation = '?';
	STRNCPY(gOperationStr, "?");
	gHost[0] = '\0';
	gHostIP[0] = '\0';
	gPort = kDefaultFTPPort;
	gRUser[0] = '\0';
	gRPass[0] = '\0';
	gRAcct[0] = '\0';
	gXtype = 'I';
	gRecursive = 0;
	gDelete = 0;
	gPassive = 2;
	if (gFwDataPortMode >= 0)
		gPassive = gFwDataPortMode;
	gRDir[0] = '\0';
	gLDir[0] = '\0';
	gRFile[0] = '\0';
	gLFile[0] = '\0';
	gManualOverrideFeatures[0] = '\0';
	gPreFTPCommand[0] = '\0';
	gPerFileFTPCommand[0] = '\0';
	gPostFTPCommand[0] = '\0';
	gPreShellCommand[0] = '\0';
	gPostShellCommand[0] = '\0';
	gDelaySinceLastFailure = 0;

	line[sizeof(line) - 1] = '\0';
	while (fgets(line, sizeof(line) - 1, fp) != NULL) {
		tok1 = strtok(line, " =\t\r\n");
		if ((tok1 == NULL) || (tok1[0] == '#'))
			continue;
		tok2 = strtok(NULL, "\r\n");
		if (tok2 == NULL)
			continue;
		if (strcmp(tok1, "op") == 0) {
			gOperation = tok2[0];
			STRNCPY(gOperationStr, tok2);
		} else if (strcmp(tok1, "delay-since-last-failure") == 0) {
			gDelaySinceLastFailure = (unsigned int) atoi(tok2);
		} else if (strcmp(tok1, "hostname") == 0) {
			(void) STRNCPY(gHost, tok2);
		} else if (strcmp(tok1, "host-ip") == 0) {
			/* Don't really use this anymore, it is
			 * only used if the host is not set.
			 */
			(void) STRNCPY(gHostIP, tok2);
		} else if (strcmp(tok1, "port") == 0) {
			gPort = atoi(tok2);
		} else if (strcmp(tok1, "passive") == 0) {
			if (isdigit((int) tok2[0]))
				gPassive = atoi(tok2);
			else
				gPassive = StrToBool(tok2);
		} else if (strncmp(tok1, "user", 4) == 0) {
			(void) STRNCPY(gRUser, tok2);
		} else if (strncmp(tok1, "pass", 4) == 0) {
			(void) STRNCPY(gRPass, tok2);
		} else if (strcmp(tok1, "anon-pass") == 0) {
			(void) STRNCPY(gLib.defaultAnonPassword, tok2);
		} else if (strncmp(tok1, "acc", 3) == 0) {
			(void) STRNCPY(gRAcct, tok2);
		} else if (strcmp(tok1, "xtype") == 0) {
			gXtype = tok2[0];
		} else if (strcmp(tok1, "recursive") == 0) {
			gRecursive = StrToBool(tok2);
		} else if (strcmp(tok1, "delete") == 0) {
			gDelete = StrToBool(tok2);
		} else if (strcmp(tok1, "remote-dir") == 0) {
			(void) STRNCPY(gRDir, tok2);
		} else if (strcmp(tok1, "local-dir") == 0) {
			(void) STRNCPY(gLDir, tok2);
		} else if (strcmp(tok1, "remote-file") == 0) {
			(void) STRNCPY(gRFile, tok2);
		} else if (strcmp(tok1, "local-file") == 0) {
			(void) STRNCPY(gLFile, tok2);
		} else if (strcmp(tok1, "manual-override-features") == 0) {
			(void) STRNCPY(gManualOverrideFeatures, tok2);
		} else if (strcmp(tok1, "pre-ftp-command") == 0) {
			(void) STRNCPY(gPreFTPCommand, tok2);
			cp = gPreFTPCommand;
			lim = cp + sizeof(gPreFTPCommand) - 1;
			goto multi;
		} else if (strcmp(tok1, "per-file-ftp-command") == 0) {
			(void) STRNCPY(gPerFileFTPCommand, tok2);
			cp = gPerFileFTPCommand;
			lim = cp + sizeof(gPerFileFTPCommand) - 1;
			goto multi;
		} else if (strcmp(tok1, "post-ftp-command") == 0) {
			(void) STRNCPY(gPostFTPCommand, tok2);
			cp = gPostFTPCommand;
			lim = cp + sizeof(gPostFTPCommand) - 1;
			goto multi;
		} else if (strcmp(tok1, "pre-shell-command") == 0) {
			(void) STRNCPY(gPreShellCommand, tok2);
			cp = gPreShellCommand;
			lim = cp + sizeof(gPreShellCommand) - 1;
			goto multi;
		} else if (strcmp(tok1, "post-shell-command") == 0) {
			(void) STRNCPY(gPostShellCommand, tok2);
			cp = gPostShellCommand;
			lim = cp + sizeof(gPostShellCommand) - 1;
		multi:
			cp += strlen(cp) - 1;
			while ((*cp == '\\') && (cp < lim)) {
				*cp++ = '\n';
				(void) fgets(cp, (int) (lim - cp), fp);
				cp += strlen(cp) - 1;
				if (*cp == '\n')
					*cp-- = '\0';
			}
		} else if (strcmp(tok1, "job-name") == 0) {
			/* ignore */
		} else {
			/* else, unknown option which is OK. */
			if (logErrors != 0) {
				Log(0, "Ignoring unknown parameter \"%s\" in %s.\n", tok1, gMyItemPath);
			}
		}
	}
	(void) fclose(fp);

	if (islower(gOperation))
		gOperation = toupper(gOperation);

	if (gHost[0] == '\0') {
		if (gHostIP[0] != '\0') {
			(void) STRNCPY(gHost, gHostIP);
		} else {
			if (logErrors != 0)
				Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "host");
			return (-1);
		}
	}

	if (gOperation == 'G') {
		if (gRecursive != 0) {
			if (gRFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "remote-file");
				return (-1);
			}
			if (gLDir[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "local-dir");
				return (-1);
			}
		} else {
			if (gRFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "remote-file");
				return (-1);
			}
			if (gLFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "local-file");
				return (-1);
			}
		}
	} else if (gOperation == 'P') {
		if (gRecursive != 0) {
			if (gLFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "local-file");
				return (-1);
			}
			if (gRDir[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "remote-dir");
				return (-1);
			}
		} else {
			if (gLFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "local-file");
				return (-1);
			}
			if (gRFile[0] == '\0') {
				if (logErrors != 0)
					Log(0, "Spool file %s missing required parameter: %s.\n", gMyItemPath, "remote-file");
				return (-1);
			}
		}
	} else {
		if (logErrors != 0)
			Log(0, "Invalid spool file operation: %c.\n", gOperation);
		return (-1);
	}

	if (gRUser[0] == '\0')
		(void) STRNCPY(gRUser, "anonymous");
	if ((gRPass[0] != '\0') && (strcmp(gRUser, "anonymous") == 0))
		(void) STRNCPY(gLib.defaultAnonPassword, gRPass);

	return (0);
}	/* LoadCurrentSpoolFileContents */




static int
RunShellCommandWithSpoolItemData(const char *const cmd, const char *const addstr)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	return (-1);
#else	/* UNIX */
	int pfd[2];
	char *argv[8];
	pid_t pid = 0;
	int oerrno;

	if ((cmd == NULL) || (cmd[0] == '\0'))
		return (-1);

	if (access(cmd, X_OK) < 0) {
		LogPerror("Cannot run program \"%s\"", cmd);
		return (-1);
	}

	if (pipe(pfd) < 0) {
		LogPerror("pipe");
	}

	pid = vfork();
	if (pid < 0) {
		(void) close(pfd[0]);
		(void) close(pfd[1]);
		LogPerror("fork");
	} else if (pid == 0) {
		(void) close(pfd[1]);	/* Child closes write end. */
		if (pfd[0] != 0) {
			(void) dup2(pfd[0], 0);
			(void) close(pfd[0]);
		}
		argv[0] = strdup(cmd);
		argv[1] = NULL;

		/* Avoid sharing other resources with the shell
		 * command, such as our FTP session descriptors.
		 */
		FTPCloseControlConnection(&gConn);
		gMyPID = getpid();
		CloseLog();

		(void) execv(cmd, argv);
		oerrno = errno;
		(void) OpenLog();
		errno = oerrno;
		LogPerror("Could not run program \"%s\"", cmd);
		exit(1);
	}
	(void) close(pfd[0]);	/* Parent closes read end. */
	(void) PWrite(pfd[1], (const char *) gItemContents, gItemContentsSize);
	if ((addstr != NULL) && (addstr[0] != '\0'))
		(void) PWrite(pfd[1], (const char *) addstr, strlen(addstr));
	(void) close(pfd[1]);	/* Parent closes write end. */

	if (pid > 1) {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif	/* HAVE_WAITPID */
	}
	return (0);
#endif	/* UNIX */
}	/* RunShellCommandWithSpoolItemData */




/*VARARGS*/
static void
LogEndItemResult(int uiShow, const char *const fmt, ...)
{
	va_list ap;
	char buf[512];

	(void) strcpy(buf, "\nresult=");

	va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
	(void) vsnprintf(buf + 8, sizeof(buf) - 8, fmt, ap);
#else
	(void) vsprintf(buf + 8, fmt, ap);
#endif
	va_end(ap);

	Log(uiShow, "%s", buf + 8);
	(void) RunShellCommandWithSpoolItemData(gPostShellCommand, buf);
}	/* LogEndItemResult */




static int
DoItem(void)
{
	char line[256];
	int needOpen;
	int result;
	int cdflags;

	if (LoadCurrentSpoolFileContents(1) < 0)
		return (0);	/* remove invalid spool file */

	if (RunShellCommandWithSpoolItemData(gPreShellCommand, NULL) == 0) {
		/* Ran the pre-command, now reload the spool file in
		 * case they modified it.
		 */
		if (LoadCurrentSpoolFileContents(1) < 0)
			return (0);	/* remove invalid spool file */
	}

	cdflags = kChdirFullPath|kChdirOneSubdirAtATime;
	if (gOperation == 'P')
		cdflags = kChdirFullPath|kChdirOneSubdirAtATime|kChdirAndMkdir;

	if (gLDir[0] != '\0') {
		if (MkDirs(gLDir, 00755) < 0) {
#ifdef HAVE_STRERROR
			LogEndItemResult(1, "Could not mkdir local-dir=%s: %s\n", gLDir, strerror(errno));
#else
			LogEndItemResult(1, "Could not mkdir local-dir=%s: errno %d\n", gLDir, (errno));
#endif
			return (0);	/* remove spool file */
		} else if (chdir(gLDir) < 0) {
#ifdef HAVE_STRERROR
			LogEndItemResult(1, "Could not cd to local-dir=%s: %s\n", gLDir, strerror(errno));
#else
			LogEndItemResult(1, "Could not cd to local-dir=%s: errno %d\n", gLDir, (errno));
#endif
			return (0);	/* remove spool file */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		} else if ((gOperation == 'G') && (access(gLDir, W_OK) < 0)) {
#ifdef HAVE_STRERROR
			LogEndItemResult(1, "Could not write to local-dir=%s: %s\n", gLDir, strerror(errno));
#else
			LogEndItemResult(1, "Could not write to local-dir=%s: errno %d\n", gLDir, (errno));
#endif
			return (0);	/* remove spool file */
#endif
		}
	}

	if (gRUser[0] == '\0')
		(void) STRNCPY(gRUser, "anonymous");

	/* Decode password, if it was base-64 encoded. */
	if (strncmp(gRPass, kPasswordMagic, kPasswordMagicLen) == 0) {
		FromBase64(line, gRPass + kPasswordMagicLen, strlen(gRPass + kPasswordMagicLen), 1);
		(void) STRNCPY(gRPass, line);
	}

	/* Now see if we need to open a new host.  We try to leave the
	 * host connected, so if they batch multiple files using the
	 * same remote host we don't need to re-open the remote host.
	 */
	needOpen = 0;
	if (gConn.connected == 0) {
		/* Not connected at all. */
		Log(0, "Was not connected originally.\n");
		needOpen = 1;
	} else if (ISTRCMP(gHost, gConn.host) != 0) {
		/* Host is different. */
		needOpen = 1;
		Log(0, "New host (%s), old host was (%s).\n", gHost, gConn.host);
	} else if (strcmp(gRUser, gConn.user) != 0) {
		/* Same host, but new user. */
		needOpen = 1;
		Log(0, "New user (%s), old user was (%s).\n", gRUser, gConn.user);
	}

	if (needOpen != 0) {
		(void) AdditionalCmd(&gConn, gPostFTPCommand, NULL);
		(void) FTPCloseHost(&gConn);
		if (FTPInitConnectionInfo(&gLib, &gConn, kDefaultFTPBufSize) < 0) {
			/* Highly unlikely... */
			LogEndItemResult(1, "init connection info failed!\n");
			ExitStuff();
			DisposeWinsock();
			exit(1);
		}

		gConn.debugLogProc = DebugHook;
		gConn.debugLog = NULL;
		gConn.errLogProc = NULL;
		gConn.errLog = NULL;
		(void) STRNCPY(gConn.host, gHost);
		gConn.port = gPort;
		(void) STRNCPY(gConn.user, gRUser);
		(void) STRNCPY(gConn.pass, gRPass);
		if ((gConn.pass[0] == '\0') && (strcmp(gConn.user, "anonymous")) && (strcmp(gConn.user, "ftp")) && (gConn.user[0] != '\0'))
			gConn.passIsEmpty = 1;
		(void) STRNCPY(gConn.acct, gRAcct);
		gConn.maxDials = 1;
		gConn.dataPortMode = gPassive;
		gConn.manualOverrideFeatures = gManualOverrideFeatures;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		gConn.progress = PrWinStatBar;
#endif

		if (MayUseFirewall(gConn.host, gFirewallType, gFirewallExceptionList) != 0) {
			gConn.firewallType = gFirewallType; 
			(void) STRNCPY(gConn.firewallHost, gFirewallHost);
			(void) STRNCPY(gConn.firewallUser, gFirewallUser);
			(void) STRNCPY(gConn.firewallPass, gFirewallPass);
			gConn.firewallPort = gFirewallPort;
		}
		
		gConn.connTimeout = 30;
		gConn.ctrlTimeout = 135;
		gConn.xferTimeout = 300;
		if (ISTRCMP(gHost, gLastHost) == 0) {
			/* Same host, but last "recent" attempt to connect failed. */
			Log(1, "Skipping same failed host as recent attempt (%s).\n", gLastHost);
			return (-1);	/* Try again next time. */
		}
		Log(1, "Opening %s:%u as user %s...\n", gHost, gPort, gRUser);
		result = FTPOpenHost(&gConn);
		if (result < 0) {
			LogEndItemResult(1, "Couldn't open %s, will try again next time.\n", gHost);
			(void) FTPCloseHost(&gConn);
			(void) STRNCPY(gLastHost, gHost);	/* save failed connection to gHost. */
			return (-1);	/* Try again next time. */
		}
		gLastHost[0] = '\0';	/* have connected - "clear" gLastHost. */
		if (FTPGetCWD(&gConn, gRStartDir, sizeof(gRStartDir)) < 0) {
			LogEndItemResult(1, "Couldn't get start directory on %s, will try again next time.\n", gHost);
			(void) AdditionalCmd(&gConn, gPostFTPCommand, NULL);
			(void) FTPCloseHost(&gConn);
			return (-1);	/* Try again next time. */
		}
		if (gConn.hasCLNT != kCommandNotAvailable) {
			(void) FTPCmd(&gConn, "CLNT %s %.5s %s",
				(gGlobalSpooler != 0) ? "NcFTPSpooler" : "NcFTPBatch",
				gVersion + 11,
				gOS
			);
		}
		(void) AdditionalCmd(&gConn, gPreFTPCommand, NULL);

		if (FTPChdir3(&gConn, gRDir, NULL, 0, cdflags) < 0) {
			LogEndItemResult(1, "Could not remote cd to %s.\n", gRDir);

			/* Leave open, but unspool.
			 *
			 * Odds are that the directory no longer exists,
			 * so it would be pointless to retry.
			 */
			return (0);
		}
	} else {
		/* Same host, but go back to root.
		 * The remote directory path is relative
		 * to root, so go back to it.
		 */
		if (FTPChdir(&gConn, gRStartDir) < 0) {
			LogEndItemResult(1, "Could not remote cd back to %s.\n", gRStartDir);
			return (-1);	/* Try again next time, in case conn dropped. */
		}

		if (FTPChdir3(&gConn, gRDir, NULL, 0, cdflags) < 0) {
			LogEndItemResult(1, "Could not remote cd to %s.\n", gRDir);
			return (-1);	/* Try again next time, in case conn dropped. */
		}
	}

	if (gOperation == 'G') {
		if (gRecursive != 0) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			sprintf(gStatusText, "Downloading %.200s", gRFile);
#endif
			result = FTPGetFiles3(&gConn, gRFile, gLDir, gRecursive, kGlobNo, gXtype, kResumeYes, kAppendNo, gDelete, kTarNo, kNoFTPConfirmResumeDownloadProc, 0);
		} else {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			sprintf(gStatusText, "[0%%] - Downloading %.200s", gRFile);
#endif
			result = FTPGetOneFile3(&gConn, gRFile, gLFile, gXtype, (-1), kResumeYes, kAppendNo, gDelete, kNoFTPConfirmResumeDownloadProc, 0);
		}
		if (result == kErrCouldNotStartDataTransfer) {
			LogEndItemResult(1, "Remote item %s is no longer retrievable.\n", gRFile);
			result = 0;	/* file no longer on host */
		} else if ((result == kErrRemoteSameAsLocal) || (result == kErrLocalSameAsRemote)) {
			LogEndItemResult(1, "Remote item %s is already present locally.\n", gRFile);
			result = 0;
		} else if (result == kErrLocalFileNewer) {
			LogEndItemResult(1, "Remote item %s is already present on remote host and is newer.\n", gRFile);
			result = 0;
		} else if (result == kNoErr) {
			(void) AdditionalCmd(&gConn, gPerFileFTPCommand, gRFile);
			LogEndItemResult(1, "Succeeded downloading %s.\n", gRFile);
		} else {
			LogEndItemResult(1, "Error (%d) occurred on %s: %s\n", result, gRFile, FTPStrError(result));
		}
	} else /* if (gOperation == 'P') */ {
		if (gRecursive != 0) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			sprintf(gStatusText, "Uploading %.200s", gLFile);
#endif
			result = FTPPutFiles3(&gConn, gLFile, gRDir, gRecursive, kGlobNo, gXtype, kAppendNo, NULL, NULL, kResumeYes, gDelete, kNoFTPConfirmResumeUploadProc, 0);
		} else {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			sprintf(gStatusText, "[0%%] - Uploading %.200s", gLFile);
#endif
			result = FTPPutOneFile3(&gConn, gLFile, gRFile, gXtype, (-1), kAppendNo, NULL, NULL, kResumeYes, gDelete, kNoFTPConfirmResumeUploadProc, 0);
		}
		if (result == kErrCouldNotStartDataTransfer) {
			LogEndItemResult(1, "Remote item %s is no longer sendable.  Perhaps permission denied on destination?\n", gRFile);
			result = 0;	/* file no longer on host */
		} else if ((result == kErrRemoteSameAsLocal) || (result == kErrLocalSameAsRemote)) {
			LogEndItemResult(1, "Local item %s is already present on remote host.\n", gLFile);
			result = 0;
		} else if (result == kErrRemoteFileNewer) {
			LogEndItemResult(1, "Local item %s is already present on remote host and is newer.\n", gLFile);
			result = 0;
		} else if (result == kNoErr) {
			(void) AdditionalCmd(&gConn, gPerFileFTPCommand, gLFile);
			LogEndItemResult(1, "Succeeded uploading %s.\n", gLFile);
		} else {
			LogEndItemResult(1, "Error (%d) occurred on %s: %s\n", result, gLFile, FTPStrError(result));
		}
	}
	
	switch (result) {
		case kErrSYMLINKFailed:
		case kErrSYMLINKNotAvailable:
		case kErrLocalDeleteFailed:
		case kErrDELEFailed:
		case kErrMKDFailed:
		case kErrCWDFailed:
		case kErrRMDFailed:
		case kErrRenameFailed:
			/* We logged the error, but do not attempt to
			 * retry this spool entry.
			 */
			result = 0;
			break;
	}
	
	return (result);
}	/* DoItem */




static int
DecodeName(const char *const src, int *yyyymmdd, int *hhmmss)
{
	char itemName[64];
	char *tok, *ps;
	int t;
	int valid = -1;

	/* Format is:
	 *
	 * X-YYYYMMDD-hhmmss
	 *
	 * Where X is the entry type (G or P, for Get or Put)
	 *       YYYYMMMDD is the 4-digit year, month, month day
	 *       hhmmss is the hour minute second
	 *
	 * The names are also allowed to have additional fields
	 * appended after the hhmmss field, to make it easier
	 * to create unique spool file names.  For example, NcFTP Client
	 * uses X-YYYYMMDD-hhmmss-PPPPPPPPPP-JJJJ, with the PID
	 * and job sequence number appended.
	 */
	(void) STRNCPY(itemName, src);
	for (t = 0, ps = itemName; ((tok = strtok(ps, "-")) != NULL); ps = NULL) {
		t++;
		switch (t) {
			case 1:
				/* Verify that entry is a G or P */
				if (strchr("GgPp", (int) tok[0]) == NULL)
					goto fail;
				break;
			case 2:
				/* Quick sanity check */
				if (isdigit((int) tok[0]) == 0)
					goto fail;
				*yyyymmdd = atoi(tok);
				break;
			case 3:
				if (isdigit((int) tok[0]) == 0)
					goto fail;
				*hhmmss = atoi(tok);
				valid = 0;
				break;
		}
	}
	if (valid < 0) {
fail:
		*yyyymmdd = 0;
		*hhmmss = 0;
		return (-1);
	}
	return (valid);
}	/* DecodeName */




static void
Now(int *yyyymmdd, int *hhmmss)
{
	struct tm lt;

	if (Gmtime(0, &lt) == NULL) {
		*yyyymmdd = 0;
		*hhmmss = 0;
	} else {
		*yyyymmdd = ((lt.tm_year + 1900) * 10000)
			+ ((lt.tm_mon + 1) * 100)
			+ (lt.tm_mday);
		*hhmmss = (lt.tm_hour * 10000)
			+ (lt.tm_min * 100)
			+ (lt.tm_sec);
	}
}	/* Now */





static void
EventShell(volatile unsigned int sleepval)
{
	volatile int nItems;
	int nProcessed, nFinished;
	unsigned int minDSLF;
	struct dirent *dent;
	struct Stat st;
	char *cp;
	char tstr[32];
	time_t tnext;
	struct tm tnext_tm;
	int iType;
	int iyyyymmdd, ihhmmss, nyyyymmdd, nhhmmss;
	DIR *volatile DIRp;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	int passes;
#else
	int sj;
	volatile int passes;
#endif

	DIRp = NULL;
	dent = gDirentBuf;
	(void) OpenLog();
	Log(0, "-----started-----\n");

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
#ifdef HAVE_SIGSETJMP
	sj = sigsetjmp(gCancelJmp, 1);
#else	/* HAVE_SIGSETJMP */
	sj = setjmp(gCancelJmp);
#endif	/* HAVE_SIGSETJMP */

	if (sj != 0) {
		gMayCancelJmp = 0;
		if (DIRp != NULL) {
			(void) closedir(DIRp);
			DIRp = NULL;
		}
		FTPShutdownHost(&gConn);
		Log(0, "Timed-out, starting over.\n");
	}
	gMayCancelJmp = 1;
#endif

	passes = 0;
	nItems = 0;
	nProcessed = 0;
	nFinished = 0;
	minDSLF = 0;

	for ( ; ; ) {
		passes++;
		if ((passes > 1) || ((passes == 1) && (sleepval != 0))) {
			/* Don't wait between passes if we just
			 * processed an item.  We only wait between
			 * passes if we didn't do anything on the
			 * previous pass.
			 */
			if ((nProcessed == 0) || (nItems == nFinished)) {
				if (minDSLF != 0) {
					sleepval = minDSLF + 1;
					if ((gDelayBetweenPasses != 0) && (gDelayBetweenPasses < minDSLF))
						sleepval = gDelayBetweenPasses;
					minDSLF = 0;
				} else if (gDelayBetweenPasses != 0) {
					sleepval = gDelayBetweenPasses;
				} else if (sleepval == 0) {
					sleepval = 3;
				} else if (sleepval > 900) {
					/* If sleep duration got so large it got past 15 minutes,
					 * start over again.
					 */
					sleepval = 60;
				} else {
					sleepval = NEW_SLEEP_VAL(sleepval);
				}

				(void) FTPCloseHost(&gConn);
				Log(0, "Sleeping %u seconds before starting pass %d.\n", sleepval, passes);
				if ((sleepval == 0) || (sleepval > 30000)) {
					Log(0, "Panic: invalid sleep amount %u.\n", sleepval);
					exit(1);
				}
				(void) sleep(sleepval);
			}
			YieldUI(1);

			/* Re-open it, in case they deleted the log
			 * while this process was running.  This also
			 * gives us an opportunity to create a new log
			 * if the existing log grew too large.
			 */
			(void) OpenLog();
		}

		if ((DIRp = opendir(gSpoolDir)) == NULL) {
			PerrorBox(gSpoolDir);
			DisposeWinsock();
			exit(1);
		}

		gLastHost[0] = '\0';	/* clear [failed]LastHost before starting a pass */
		Log(0, "Starting pass %d.\n", passes);
		if (passes >= 1000000) {
			/* This "panic" and the one a few lines above
			 * are here temporarily; I think the bug this
			 * was checking for has been fixed finally.
			 */
			Log(0, "Panic: invalid pass number %d.\n", passes);
			exit(1);
		}

		for (nItems = 0, nProcessed = 0, nFinished = 0; ; ) {
			if (Readdir(DIRp, dent, gDirentBufSize) == NULL)
				break;

			YieldUI(0);

			(void) STRNCPY(gItemPath, gSpoolDir);
			(void) STRNCAT(gItemPath, LOCAL_PATH_DELIM_STR);
			(void) STRNCAT(gItemPath, dent->d_name);
			if ((Stat(gItemPath, &st) < 0) || (S_ISREG(st.st_mode) == 0)) {
				/* Item may have been
				 * deleted by another
				 * process.
				 */
				continue;
			}

			if (DecodeName(dent->d_name, &iyyyymmdd, &ihhmmss) < 0) {
				/* Junk file in the spool directory. */
				continue;
			}

			cp = StrRFindLocalPathDelim(gItemPath);
			if (cp == NULL) {
				/* Impossible */
				continue;
			}
			cp++;

			iType = (int) *cp;
			if (isupper(iType))
				iType = islower(iType);
			if ((iType != 'g') && (iType != 'p')) {
				/* No more items waiting for processing. */
				continue;
			}

			/* Count items waiting for processing. */
			nItems++;

			Now(&nyyyymmdd, &nhhmmss);
			if ((nyyyymmdd < iyyyymmdd) || ((nyyyymmdd == iyyyymmdd) && (nhhmmss < ihhmmss))) {
				/* Process only if the specified start
				 * time has passed.
				 */
				continue;
			}

			(void) STRNCPY(gMyItemPath, gItemPath);
			gMyItemPath[(int) (cp - gItemPath)] = 'x';

			/* Race condition between other ncftpbatches,
			 * but only one of them will rename it
			 * successfully.
			 */
			if (rename(gItemPath, gMyItemPath) == 0) {
				gItemInUse = 1;
				Log(0, "Processing path: %s\n", gItemPath);
				nProcessed++;
				if (DoItem() < 0) {
					if (gDelaySinceLastFailure == 0)
						gDelaySinceLastFailure = 5;
					else {
						gDelaySinceLastFailure = NEW_SLEEP_VAL(gDelaySinceLastFailure);
						if (gDelaySinceLastFailure > 900) {
							/* If sleep duration got so large it got past 15 minutes,
							 * start over again.
							 */
							gDelaySinceLastFailure = 60;
						}
					}
					tnext = time(NULL) + (time_t) gDelaySinceLastFailure;
					strftime(tstr, sizeof(tstr), "%Y-%m-%d %H:%M:%S %Z", Gmtime(tnext, &tnext_tm));

					gMaySigExit = 0;
					if (SpoolX(
						NULL,
						gSpoolDir,
						gOperationStr,
						gRFile,
						gRDir,
						gLFile,
						gLDir,
						gHost,
						gHostIP,
						gPort,
						gRUser,
						gRPass,
						gRAcct,
						gXtype,
						gRecursive,
						gDelete,
						gPassive,
						gPreFTPCommand,
						gPerFileFTPCommand,
						gPostFTPCommand,
						gPreShellCommand,
						gPostShellCommand,
						tnext,
						gDelaySinceLastFailure,
						gManualOverrideFeatures,
						0
					) < 0) {
						/* quit now */
						Log(0, "Could not rename job %s!\n", gMyItemPath);
						return;
					}
					if (unlink(gMyItemPath) != 0) {
						/* quit now */
						Log(0, "Could not delete old copy of job %s!\n", gMyItemPath);
						return;
					}
					Log(0, "Rescheduled %s for %s.\n", gItemPath, tstr);

					gMaySigExit = 1;

					if ((minDSLF == 0) || (minDSLF > gDelaySinceLastFailure))
						minDSLF = gDelaySinceLastFailure;
				} else {
					nFinished++;
					Log(0, "Done with %s.\n", gItemPath);
					if (unlink(gMyItemPath) != 0) {
						/* quit now */
						Log(0, "Could not delete finished job %s!\n", gMyItemPath);
						return;
					}
				}
				(void) chdir(LOCAL_PATH_DELIM_STR);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
				/* Allow time for message to be seen */
				sleep(1);
#endif
			}
			if (gQuitRequested != 0) {
				if (DIRp != NULL) {
					(void) closedir(DIRp);
					DIRp = NULL;
				}
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
				Log(0, "User requested close.\n");
#else
				if (gQuitRequested == 1)
					Log(0, "User requested close.\n");
				else
					Log(0, "-----processing delayed signal %d, exiting-----\n", gQuitRequested);
#endif
				(void) AdditionalCmd(&gConn, gPostFTPCommand, NULL);
				(void) FTPCloseHost(&gConn);
				gMayCancelJmp = 0;
				Log(0, "-----done-----\n");
				return;
			}
		}
		if (DIRp != NULL) {
			(void) closedir(DIRp);
			DIRp = NULL;
		}
		if ((nItems == nFinished) && (nFinished > 0)) {
			Log(0, "The spool directory %s is now empty.\n", gSpoolDir);
		} else if (nItems == 0) {
			Log(0, "The spool directory %s is empty.\n", gSpoolDir);
		}
		if (nItems == nFinished) {
			/* Spool directory is empty, done. */
			if (gGlobalSpooler == 0)
				break;
		}
	}
	(void) AdditionalCmd(&gConn, gPostFTPCommand, NULL);
	(void) FTPCloseHost(&gConn);
	gMayCancelJmp = 0;
	Log(0, "-----done-----\n");
}	/* EventShell */




#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else

static void
ListQueue(void)
{
	int nItems;
	struct Stat st;
	struct dirent *dent;
	DIR *DIRp;
	char *cp;
	int iyyyymmdd, ihhmmss;
	char dstr[64];
	char yyyy[8], mm[4], dd[4];
	char HH[4], MM[4];

	if ((DIRp = opendir(gSpoolDir)) == NULL) {
		PerrorBox(gSpoolDir);
		(void) fprintf(stderr, "This directory is created automatically the first time you do a background\noperation from NcFTP.\n");
		DisposeWinsock();
		exit(1);
	}

	dent = gDirentBuf;
	for (nItems = 0; ; ) {
		if (Readdir(DIRp, dent, gDirentBufSize) == NULL)
			break;
		
		(void) STRNCPY(gItemPath, gSpoolDir);
		(void) STRNCAT(gItemPath, LOCAL_PATH_DELIM_STR);
		(void) STRNCAT(gItemPath, dent->d_name);
		if ((Stat(gItemPath, &st) < 0) || (S_ISREG(st.st_mode) == 0)) {
			/* Item may have been
			 * deleted by another
			 * process.
			 */
			continue;
		}

		if (DecodeName(dent->d_name, &iyyyymmdd, &ihhmmss) < 0) {
			/* Junk file in the spool directory. */
			continue;
		}

		cp = StrRFindLocalPathDelim(gItemPath);
		if (cp == NULL) {
			/* Impossible */
			continue;
		}
		cp++;

		(void) STRNCPY(gMyItemPath, gItemPath);
		if (LoadCurrentSpoolFileContents(0) == 0) {
			nItems++;
			if (nItems == 1) {
				(void) printf("---Scheduled-For-----Host----------------------------Command--------------------\n");
			}
			(void) sprintf(dstr, "%08d", iyyyymmdd);
			(void) memcpy(yyyy, dstr, 4); yyyy[4] = '\0';
			(void) memcpy(mm, dstr + 4, 2); mm[2] = '\0';
			(void) memcpy(dd, dstr + 6, 2); dd[2] = '\0';
			(void) sprintf(dstr, "%06d", ihhmmss);
			(void) memcpy(HH, dstr + 0, 2); HH[2] = '\0';
			(void) memcpy(MM, dstr + 2, 2); MM[2] = '\0';
			(void) printf("%c  %s-%s-%s %s:%s  %-30s  ",
				(gItemPath[0] == 'x') ? '*' : ' ',
				yyyy, mm, dd, HH, MM,
				gHost
			);
			if (gOperation != 'P') {
				(void) printf("GET");
				if (gRecursive != 0) {
					(void) printf(" -R %s", gRFile);
				} else {
					(void) printf(" %s", gRFile);
				}
			} else {
				(void) printf("PUT");
				if (gRecursive != 0) {
					(void) printf(" -R %s", gLFile);
				} else {
					(void) printf(" %s", gLFile);
				}
			}
			(void) printf("\n");
		}
	}
	(void) closedir(DIRp);
	if (nItems == 0) {
		/* Spool directory is empty, done. */
		(void) printf("%s \"%s\" directory is empty.\n",
			(gGlobalSpooler != 0) ? "The" : "Your",
			gSpoolDir);
	}
}	/* ListQueue */

#endif





#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)

static void OnDraw(HWND hwnd, HDC hdc)
{
	RECT clientRect, rect, r;
	char str[128];
	struct tm lt;
	BOOL sizeIsUnknown, inProgress;
	int secLeft, minLeft;
	double rate, per;
	const char *rStr;
	int oldBkMode;
	int iper;
	HBRUSH redBrush, bkgndBrush;
	TEXTMETRIC textMetrics;
	static int lineHeight = 0;
	static int lastUpdate = 0;
	COLORREF oldBkColor;
	static HFONT statusTextFont;
	LOGFONT lf;
	char *cp;

	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S %Z", Gmtime(0, &lt));

	sizeIsUnknown = (gConn.expectedSize == kSizeUnknown);
	inProgress = (gConn.bytesTransferred > 0);

	GetClientRect(hwnd, &clientRect);

	if (lineHeight == 0) {
		// First time through.
		//
		ZeroMemory(&lf, (DWORD) sizeof(lf));
		lf.lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		strcpy(lf.lfFaceName, "MS Sans Serif");
		statusTextFont = CreateFontIndirect(&lf);
		if (statusTextFont != NULL)
			SendMessage(gStaticCtrl, WM_SETFONT, (WPARAM) statusTextFont, (LPARAM) 1);

		GetTextMetrics(hdc, &textMetrics);
		lineHeight = textMetrics.tmAscent + textMetrics.tmDescent + textMetrics.tmExternalLeading;

		GetWindowRect(gMainWnd, &r);
		r.bottom = r.top + 30 + lineHeight + lineHeight + lineHeight + 20 - 4;
		MoveWindow(gMainWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
	}

	if (gConn.dataSocket < 0) {
		// Transfer not in progress, show the status text.
		//
		SetWindowText(gStaticCtrl, gStatusText);
		if (lastUpdate == 0) {
			ShowWindow(gStaticCtrl, SW_SHOW);
			SetWindowText(gMainWnd, "NcFTPBatch");
		}
		lastUpdate = 1;
	} else {
		if (lastUpdate == 1)
			ShowWindow(gStaticCtrl, SW_HIDE);
		lastUpdate = 0;

		rect.left = 10;
		rect.top = 10;
		rect.right = clientRect.right - 10;
		rect.bottom = rect.top + lineHeight + 10;
		
		if (!sizeIsUnknown) {
			FrameRect(hdc, &rect, GetStockObject(BLACK_BRUSH));
			
			r.left = rect.left + 1;
			per = gConn.percentCompleted / 100.0;
			if (per < 0.0)
				per = 0.0;
			r.right = r.left + (int) ((double) (rect.right - 1 - r.left) * per);
			r.top = rect.top + 1;
			r.bottom = rect.bottom - 1;
			
			redBrush = CreateSolidBrush(RGB(255,0,0));
			FillRect(hdc, &r, redBrush);
			DeleteObject(redBrush);
			
			r.left = r.right;
			r.right = rect.right - 1;
			if ((r.left + 2) < r.right)
				FillRect(hdc, &r, GetStockObject(WHITE_BRUSH));

			r.left = rect.left + 10;
			r.right = rect.right - 10;
			r.top = rect.top + 2;
			r.bottom = rect.bottom - 2;
			
			oldBkMode = SetBkMode(hdc, TRANSPARENT);
			if (gConn.lname != NULL)
				DrawText(hdc, gConn.lname, -1, &r, DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_VCENTER | DT_CENTER);
			(void) SetBkMode(hdc, oldBkMode);

			cp = strchr(gStatusText, '[');
			if (cp != NULL) {
				iper = (int) (per * 100.0 + 0.5);

				if ((iper > 99) && (cp[2] == '%')) {
					memmove(cp + 2, cp, strlen(cp) + 2);
				} else if ((iper > 99) && (cp[3] == '%')) {
					memmove(cp + 1, cp, strlen(cp) + 1);
				} else if ((iper > 9) && (cp[2] == '%')) {
					memmove(cp + 1, cp, strlen(cp) + 1);
				}
				sprintf(cp, "[%d", iper);
				if (iper > 99)
					cp[4] = '%';
				else if (iper > 9)
					cp[3] = '%';
				else
					cp[2] = '%';
			}
		} else {
			FillRect(hdc, &rect, GetStockObject(WHITE_BRUSH));
			FrameRect(hdc, &rect, GetStockObject(BLACK_BRUSH));
			if (gConn.lname != NULL)
				DrawText(hdc, gConn.lname, -1, &r, DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_VCENTER | DT_CENTER);

			cp = strchr(gStatusText, '[');
			if (cp != NULL) {
				// Get rid of the prefix from [0%] - Down...
				//
				memmove(gStatusText, gStatusText + 7, strlen(gStatusText) + 7);
			}
		}
		SetWindowText(gMainWnd, gStatusText);

		oldBkColor = SetBkColor(hdc, RGB(192,192,192));

		rect.left = 10;
		rect.top = 30 + lineHeight;
		rect.right = clientRect.right - 10;
		rect.bottom = rect.top + lineHeight;
		bkgndBrush = CreateSolidBrush(RGB(192,192,192));
		FillRect(hdc, &rect, bkgndBrush);
		DeleteObject(bkgndBrush);

		rect.right = (clientRect.right / 2) - 10;
		if (sizeIsUnknown) {
			sprintf(str, PRINTF_LONG_LONG " bytes", (gConn.startPoint + gConn.bytesTransferred));
		} else {
			sprintf(str, PRINTF_LONG_LONG " of " PRINTF_LONG_LONG " bytes",
				inProgress ? (gConn.startPoint + gConn.bytesTransferred) : (longest_int) 0,
				gConn.expectedSize
				);
		}
		DrawText(hdc, str, -1, &rect, DT_SINGLELINE | DT_NOCLIP);
		
		if ((!sizeIsUnknown) && (inProgress)) {
			rect.left = (clientRect.right / 2);
			rect.right = (3 * clientRect.right / 4);
			secLeft = (int) (gConn.secLeft + 0.5);
			minLeft = secLeft / 60;
			secLeft = secLeft - (minLeft * 60);
			if (minLeft > 999) {
				minLeft = 999;
				secLeft = 59;
			}
			sprintf(str, "ETA: %d:%02d", minLeft, secLeft);
			DrawText(hdc, str, -1, &rect, DT_SINGLELINE | DT_CENTER);
		}
		
		if (inProgress) {
			rate = FileSize(gConn.kBytesPerSec * 1024.0, &rStr, NULL);
			rect.left = (3 * clientRect.right / 4);
			rect.right = clientRect.right - 10;
			sprintf(str, "%.1f %s/sec", rate, rStr);
			DrawText(hdc, str, -1, &rect, DT_SINGLELINE | DT_RIGHT);
		}

		SetBkColor(hdc, oldBkColor);
	}
}	/* OnDraw */





LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMsg) {
	case WM_USER:
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		OnDraw(hwnd, hdc);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		gQuitRequested = 1;
		gConn.cancelXfer = 1;
		return 0;
	}
	
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}	// WndProc




#pragma warning(disable : 4100)		// warning C4100: unreferenced formal parameter
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance_unused, PSTR szCmdLine_unused, int iCmdShow)
{
	WNDCLASSEX wndclass;
	HWND hWnd;
	RECT r;

	ghInstance = hInstance;

	ZeroMemory(&gComCtls, sizeof(gComCtls));
	gComCtls.dwSize = sizeof(gComCtls);
	gComCtls.dwICC = ICC_PROGRESS_CLASS;
	if (! InitCommonControlsEx(&gComCtls)) {
		PerrorBox("InitCommonControlsEx");
		return 0;
	}

	ZeroMemory(&wndclass, sizeof(wndclass));
	wndclass.cbSize        = sizeof (wndclass) ;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = __T("ncftpbatch");
	wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINFRAME));

	if (RegisterClassEx(&wndclass) == (ATOM) 0) {
		PerrorBox("RegisterClassEx");
		return 0;
	}

	// Create the main window, which is
	// never intended to be seen.
	//
	hWnd = CreateWindow (
		wndclass.lpszClassName,		// window class name
		__T("NcFTPBatch"),			// window caption
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,		// window style
		100,						// initial x position
		100,						// initial y position
		450,						// initial x size
		100,						// initial y size
		NULL,						// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL);						// creation parameters

	if (hWnd == NULL) {
		PerrorBox("CreateWindow(main window)");
		return 0;
	}
	gMainWnd = hWnd;

	GetClientRect(gMainWnd, &r);
	r.top = r.left = 10;
	r.right -= 10;
	r.bottom -= 10;

	ZeroMemory(gStatusText, (DWORD) sizeof(gStatusText));
	hWnd = CreateWindow (
		"STATIC",					// window class name
		gStatusText,				// window caption
		SS_LEFT | WS_CHILD | WS_VISIBLE,			// window style
		r.left,						// initial x position
		r.top,						// initial y position
		r.right - r.left,			// initial x size
		r.bottom - r.top,			// initial y size
		gMainWnd,					// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL);						// creation parameters

	if (hWnd == NULL) {
		PerrorBox("CreateWindow(static control)");
		return 0;
	}
	gStaticCtrl = hWnd;

	SendMessage(gMainWnd, WM_USER, (WPARAM) 0, (LPARAM) 0);
	ShowWindow(gMainWnd, SW_SHOWNORMAL);
	// Here we go!
	//
	PreInit("ncftpbatch.exe");
	PostInit();
	EventShell(0);
	PostShell();

	return 0;
}	// WinMain
#pragma warning(default : 4100)		// warning C4100: unreferenced formal parameter


#else	/* UNIX */

static void
ReadCore(int fd)
{
	FTPLibraryInfo tLib;
	FTPConnectionInfo tConn;
	int rc;

	if ((PRead(fd, (char *) &tLib, sizeof(tLib), 1) == sizeof(tLib))
		&& (PRead(fd, (char *) &tConn, sizeof(tConn), 1) == sizeof(tConn))
		&& (strncmp(tConn.magic, gConn.magic, sizeof(tConn.magic)) == 0)
	) {
		(void) memcpy(&gConn, &tConn, sizeof(gConn));
		(void) memcpy(&gLib, &tLib, sizeof(gLib));
		rc = FTPRebuildConnectionInfo(&gLib, &gConn);
		if (rc < 0) {
			FTPInit();
		} else {
			gConn.debugLogProc = DebugHook;
		}
	}
}	/* ReadCore */




static void
Daemon(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	/* Change to root directory so filesystems
	 * can be unmounted, if they could in fact
	 * be unmounted.
	 */
	(void) chdir("\\");
#else
	int i, fd;
	int devnull;
	int pid;

	/* Redirect standard in, out, and err, if they were terminals. */
	devnull = open("/dev/null", O_RDWR, 00666);

	for (i=0; i<3; i++) {
		if (gConn.ctrlSocketR == i)
			continue;
		if (gConn.ctrlSocketW == i)
			continue;

		/* Close standard descriptors and replace
		 * with /dev/null.
		 */
		(void) close(i);
		if (devnull >= 0)
			(void) dup2(devnull, i);
	}

	if (devnull >= 0)
		(void) close(devnull);

	/* Close all unneeded descriptors. */
	for (fd = 3; fd < 256; fd++) {
		if (gConn.ctrlSocketR == fd)
			continue;
		if (gConn.ctrlSocketW == fd)
			continue;
		(void) close(fd);
	}

	pid = vfork();
	if (pid < 0)
		exit(1);
	else if (pid > 0)
		exit(0);	/* parent. */

#ifdef HAVE_SETSID
	/* Become session leader for this "group." */
	(void) setsid();
#endif

	/* Run as "nohup."  Don't want to get hangup signals. */
	(void) NcSignal(SIGHUP, (FTPSigProc) SIG_IGN);

	/* Turn off TTY control signals, just to be sure. */
	(void) NcSignal(SIGINT, (FTPSigProc) SIG_IGN);
	(void) NcSignal(SIGQUIT, (FTPSigProc) SIG_IGN);
#ifdef SIGTSTP
	(void) NcSignal(SIGTSTP, (FTPSigProc) SIG_IGN);
#endif
	
	/* Become our own process group. */
#ifdef HAVE_SETPGID
	(void) setpgid(0, 0);
#elif defined(HAVE_SETPGRP) && defined(SETPGRP_VOID)
	(void) setpgrp();
#elif defined(HAVE_SETPGRP) && !defined(SETPGRP_VOID)
	(void) setpgrp(0, getpid());
#endif

#ifdef TIOCNOTTY
	/* Detach from controlling terminal, so this
	 * process is not associated with any particular
	 * tty.
	 */
	fd = open("/dev/tty", O_RDWR, 0);
	if (fd >= 0) {
		(void) ioctl(fd, TIOCNOTTY, 0);
		(void) close(fd);
	}
#endif

	/* Change to root directory so filesystems
	 * can be unmounted.
	 */
	(void) chdir("/");
#endif
}	/* Daemon */




static void
Usage(void)
{
	(void) fprintf(stderr, "Usages:\n");
	if (gGlobalSpooler == 0) {
		(void) fprintf(stderr, "\tncftpbatch -d | -D\t\t\t(start NcFTP batch processing)\n");
		(void) fprintf(stderr, "\tncftpbatch -l\t\t\t\t(list spooled jobs)\n");
	} else {
		(void) fprintf(stderr, "\tncftpspooler -d [-q spool-dir] [-o log-file] [-s delay]\n");
		(void) fprintf(stderr, "\tncftpspooler -l\t\t\t\t(list spooled jobs)\n");
	}
	(void) fprintf(stderr, "\nLibrary version: %s.\n", gLibNcFTPVersion + 5);
	(void) fprintf(stderr, "This is a freeware program by Mike Gleason (http://www.NcFTP.com).\n");
	DisposeWinsock();
	exit(2);
}	/* Usage */




main_void_return_t
main(int argc, char **const argv)
{
	int c;
	int runAsDaemon = -1;
	unsigned int sleepval = 0;
	int listonly = -1;
	int readcore = -1;
	GetoptInfo opt;

	PreInit(argv[0]);
#if (defined(SOCKS)) && (SOCKS >= 5)
	SOCKSinit(argv[0]);
#endif	/* SOCKS */

	if (gGlobalSpooler != 0) {
		runAsDaemon = -1;
		GetoptReset(&opt);
		while ((c = Getopt(&opt, argc, argv, "Ddls:q:o:")) > 0) switch(c) {
			case 'd':
				runAsDaemon = 1;
				break;
			case 'D':
				runAsDaemon = 0;
				break;
			case 'l':
				listonly = 1;
				break;
			case 's':
				if (atoi(opt.arg) > 0)
					gDelayBetweenPasses = (unsigned int) atoi(opt.arg);
				break;
			case 'q':
				STRNCPY(gSpoolDir, opt.arg);
				break;
			case 'o':
				STRNCPY(gLogFileName, opt.arg);
				break;
			default:
				Usage();
		}

		/* Require that they use the "-d" option,
		 * since otherwise a ncftpspooler process
		 * would be launched into the background
		 * unbeknownst to the user.  It is common
		 * for a user to run a program with no
		 * arguments in hopes of seeing a usage
		 * screen.
		 */
		if ((listonly < 0) && (runAsDaemon < 0)) {
			/* Must specify either -l or -d/-D */
			Usage();
		}
	} else {
		/* User Spooler */
		GetoptReset(&opt);
		while ((c = Getopt(&opt, argc, (char **) argv, "|:XDdlS:Z:s:w")) > 0) switch(c) {
			case 'd':
				runAsDaemon = 1;
				break;
			case 'D':
				runAsDaemon = 0;
				break;
			case 'l':
				listonly = 1;
				break;
			case 'Z':
				if (atoi(opt.arg) > 0)
					sleep((unsigned int) atoi(opt.arg));
				break;
			case 'S':
				if (atoi(opt.arg) > 0)
					sleepval = (unsigned int) atoi(opt.arg);
				break;
			case 's':
				if (atoi(opt.arg) > 0)
					gDelayBetweenPasses = (unsigned int) atoi(opt.arg);
				break;
			case 'w':
				gLogOpenMode = FOPEN_WRITE_TEXT;
				break;
			case 'X':
				/* Yes, I do exist. */
				DisposeWinsock();
				exit(0);
				/*NOTREACHED*/
				break;
			case '|':
				readcore = atoi(opt.arg);
				break;
			default:
				Usage();
		}

		if ((listonly < 0) && (runAsDaemon < 0)) {
			/* Must specify either -l or -d/-D */
			Usage();
		}
	}

	PostInit();
	if (listonly > 0) {
		ListQueue();
	} else {
		if (gGlobalSpooler == 0) {
			if (readcore >= 0) {
				/* Inherit current live FTP session
				 * from ncftp!
				 */
				ReadCore(readcore);
			}
		}
		if (runAsDaemon > 0) {
			if (OpenLog() < 0) {
				perror(gLogFileName);
				exit(1);
			}
			Daemon();
			gIsTTY = 0;
		}

		EventShell(sleepval);
		PostShell();
	}
	DisposeWinsock();
	exit(0);
}	/* main */

#endif	/* UNIX */


#else	/* HAVE_LONG_FILE_NAMES */
main()
{
	fprintf(stderr, "this program needs long filenames, sorry.\n");
	exit(1);
}	/* main */
#endif	/* HAVE_LONG_FILE_NAMES */
