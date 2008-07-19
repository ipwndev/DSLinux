/* cmds.c
 *
 * Copyright (c) 1992-2006 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif
#include "shell.h"
#include "util.h"
#include "ls.h"
#include "bookmark.h"
#include "cmds.h"
#include "main.h"
#include "trace.h"
#include "log.h"
#include "pref.h"
#include "spool.h"
#include "gl_getline.h"
#include "readln.h"

/* This was the directory path when the user logged in.  For anonymous users,
 * this should be "/", but for real users, it should be their home directory.
 * This variable is used later when we want to calculate a relative path
 * from the starting directory.
 */
char gStartDir[512];

/* The pathname to the current working directory.  This always needs to be
 * current, so whenever a directory change is done this should be changed.
 */
char gRemoteCWD[512];

/* Same, but the previous directory the user was in, or empty string if
 * there is none.
 */
char gPrevRemoteCWD[512];

/* Another buffer we use just temporarily when switching directories. */
char gScratchCWD[512];

/* If true, the remote server is most likely Windows/DOS.
 */
int gServerUsesMSDOSPaths;

/* The only reason we do this is to get gcc/lint to shut up
 * about unused parameters.
 */
int gUnusedArg;
#define ARGSUSED(x) x = (argc != 0) || (argv != 0) || (cmdp != 0) || (aip != 0)

/* Used temporarily, but put it here because it's big. */
FTPConnectionInfo gTmpURLConn;

/* If the user doesn't want to be prompted for a batch of files,
 * they can tell us to answer this answer for each item in the batch.
 */
int gResumeAnswerAll;

/* Did the user stupidly type passwords, etc., on our command line? */
int gUserTypedSensitiveInfoAtShellSoDoNotSaveItToDisk = 0;

typedef struct SpoolCmdInfo {
	int xtype;
	int deleteFlag;
	int recurseFlag;
	int renameMode;
	time_t when;
	const char *lcwd;
	const char *rcwd;
	int base;
} SpoolCmdInfo;

extern FTPConnectionInfo gConn;
extern char gOurDirectoryPath[];
extern Command gCommands[];
extern const char gVersion[];
extern size_t gNumCommands;
extern int gDebug, gDoneApplication, gIsTTYr;
extern int gGotSig;
extern unsigned int gFirewallPort;
extern int gFirewallType;
extern char gFirewallHost[64];
extern char gFirewallUser[32];
extern char gFirewallPass[32];
extern char gFirewallExceptionList[];
extern char gPager[], gHome[], gShell[];
extern char gOS[];
extern int gAutoResume;
extern int gAutoSaveChangesToExistingBookmarks;
extern Bookmark gBm;
extern int gLoadedBm, gConfirmClose, gSavePasswords, gScreenColumns;
extern char gLocalCWD[512], gPrevLocalCWD[512];
extern int gMayCancelJmp;
extern char gOurHostName[64];
extern time_t gCmdStart;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
extern char gOurInstallationPath[];
#elif defined(HAVE_SIGSETJMP)
extern sigjmp_buf gCancelJmp;
#else	/* HAVE_SIGSETJMP */
extern jmp_buf gCancelJmp;
#endif	/* HAVE_SIGSETJMP */

static void
OpenMsg(const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 1, 2)))
#endif
;

#if (defined(__GNUC__)) && (__GNUC__ == 2)
#	define DATE_SPEC "%a %b %d %H:%M:%S %Z %Y"
#else
	/* For GCC 3, we can disable the Y2K warnings. */
#	define DATE_SPEC "%c"
#endif

/* Open the users $PAGER, or just return stdout.  Make sure to use
 * ClosePager(), and not fclose/pclose directly.
 */
static FILE *
OpenPager(void)
{
	FILE *fp;
	char *pprog;

	(void) fflush(stdout);
	pprog = gPager;
	fp = popen((pprog[0] == '\0') ? "more" : pprog, "w");
	if (fp == NULL)
		return (stdout);
	return (fp);
}	/* OpenPager */




/* Close (maybe) a file previously created by OpenPager. */
static void
ClosePager(FILE *pagerfp)
{
#ifdef SIGPIPE
	sigproc_t osigpipe;
#endif

	if ((pagerfp != NULL) && (pagerfp != stdout)) {
#ifdef SIGPIPE
		osigpipe = (sigproc_t) NcSignal(SIGPIPE, SIG_IGN);
#endif
		(void) pclose(pagerfp);
#ifdef SIGPIPE
		(void) NcSignal(SIGPIPE, osigpipe);
#endif
	}
}	/* ClosePager */




/* Fills in the bookmarkName field of the Bookmark. */
int
PromptForBookmarkName(BookmarkPtr bmp)
{
	char dfltname[64];
	char bmname[64];

	DefaultBookmarkName(dfltname, sizeof(dfltname), gConn.host);
	if (dfltname[0] == '\0') {
		(void) printf("Enter a name for this bookmark: ");
	} else {
		(void) printf("Enter a name for this bookmark, or hit enter for \"%s\": ", dfltname);
	}
	fflush(stdin);
	(void) FGets(bmname, sizeof(bmname), stdin);
	if (bmname[0] != '\0') {
		(void) STRNCPY(bmp->bookmarkName, bmname);
		return (0);
	} else if (dfltname[0] != '\0') {
		(void) STRNCPY(bmp->bookmarkName, dfltname);
		return (0);
	}
	return (-1);
}	/* PromptForBookmarkName */



void
CurrentURL(char *dst, size_t dsize, int showpass)
{
	Bookmark bm;
	char dir[160];

	memset(&bm, 0, sizeof(bm));
	(void) STRNCPY(bm.name, gConn.host);
	if ((gConn.user[0] != '\0') && (! STREQ(gConn.user, "anonymous")) && (! STREQ(gConn.user, "ftp"))) {
		(void) STRNCPY(bm.user, gConn.user);
		(void) STRNCPY(bm.pass, (showpass == 0) ? "PASSWORD" : gConn.pass);
		(void) STRNCPY(bm.acct, gConn.acct);
	}

	bm.port = gConn.port;

	/* We now save relative paths, because the pathname in URLs are
	 * relative by nature.  This makes non-anonymous FTP URLs shorter
	 * because it doesn't have to include the pathname of their
	 * home directory.
	 */
	(void) STRNCPY(dir, gRemoteCWD);
	AbsoluteToRelative(bm.dir, sizeof(bm.dir), dir, gStartDir, strlen(gStartDir));

	BookmarkToURL(&bm, dst, dsize);
}	/* CurrentURL */




/*VARARGS*/
int
AskYesNoQuestion(int defaultAnswer, const char *const fmt, ...)
{
	va_list ap;
	char ans[128], buf[512];

	va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
	(void) vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	buf[sizeof(buf) - 1] = '\0';
#else
	(void) vsprintf(buf, fmt, ap);
#endif
	va_end(ap);

	for (;;) {
		(void) fprintf(stdout, "%s", buf);
		(void) fflush(stdout);
		(void) fflush(stdin);
		(void) memset(ans, 0, sizeof(ans));
		(void) fgets(ans, sizeof(ans) - 1, stdin);
		Trace(0, "%s%s", buf, ans);
		switch ((int) ans[0]) {
			case 'y':
			case 'Y':
				return (1);
				/*NOTREACHED*/
			case 'n':
			case 'N':
				return (0);
				/*NOTREACHED*/
			default:
				if (defaultAnswer >= 0)
					return (defaultAnswer);
		
		}
	}
}	/* AskYesNoQuestion */




/* Fills in the fields of the Bookmark structure, based on the FTP current
 * session.
 */
void
FillBookmarkInfo(BookmarkPtr bmp)
{
	char dir[160];

	(void) STRNCPY(bmp->name, gConn.host);
	if ((STREQ(gConn.user, "anonymous")) || (STREQ(gConn.user, "ftp"))) {
		bmp->user[0] = '\0';
		bmp->pass[0] = '\0';
		bmp->acct[0] = '\0';
	} else {
		(void) STRNCPY(bmp->user, gConn.user);
		(void) STRNCPY(bmp->pass, gConn.pass);
		(void) STRNCPY(bmp->acct, gConn.acct);
	}

	/* We now save relative paths, because the pathname in URLs are
	 * relative by nature.  This makes non-anonymous FTP URLs shorter
	 * because it doesn't have to include the pathname of their
	 * home directory.
	 */
	(void) STRNCPY(dir, gRemoteCWD);
	AbsoluteToRelative(bmp->dir, sizeof(bmp->dir), dir, gStartDir, strlen(gStartDir));
	bmp->port = gConn.port;
	(void) time(&bmp->lastCall);
	bmp->hasSIZE = gConn.hasSIZE;
	bmp->hasMDTM = gConn.hasMDTM;
	bmp->hasPASV = gConn.hasPASV;
	bmp->hasUTIME = gConn.hasSITE_UTIME;
	if (gFirewallType == kFirewallNotInUse)
		(void) STRNCPY(bmp->lastIP, gConn.ip);
}	/* FillBookmarkInfo */




/* Saves the current FTP session settings as a bookmark. */
void
SaveCurrentAsBookmark(void)
{
	int saveBmPassword;

	/* gBm.bookmarkName must already be set. */
	FillBookmarkInfo(&gBm);

	saveBmPassword = gSavePasswords;
	if ((saveBmPassword < 0) && (gBm.pass[0] != '\0')) {
	 	if (gLoadedBm != 0) {
	 		saveBmPassword = 1;
	 	} else if (gIsTTYr == 0) {
			saveBmPassword = 0;
		} else {
			(void) printf("\n\nYou logged into this site using a password.\nWould you like to save the password with this bookmark?\n\n");
			if ((saveBmPassword = AskYesNoQuestion(0, "Save? [no] ")) == 0)
				(void) printf("\nNot saving the password.\n");
		}
	}
	if (PutBookmark(&gBm, saveBmPassword) < 0) {
		(void) fprintf(stderr, "Could not save bookmark.\n");
	} else {
		/* Also marks whether we saved it. */
		gLoadedBm = 1;
		(void) printf("Bookmark \"%s\" saved.\n", gBm.bookmarkName);

		ReCacheBookmarks();
	}
}	/* SaveCurrentAsBookmark */




/* If the user did not explicitly bookmark this site already, ask
 * the user if they want to save one.
 */
void
SaveUnsavedBookmark(void)
{
	char url[256];

	if (gIsTTYr == 0) {
		return;
	} else if ((gConfirmClose != 0) && (gLoadedBm == 0) && (gOurDirectoryPath[0] != '\0')) {
		FillBookmarkInfo(&gBm);
		BookmarkToURL(&gBm, url, sizeof(url));
		(void) printf("\n\nYou have not saved a bookmark for this site.\n");
		(void) sleep(1);
		(void) printf("\nWould you like to save a bookmark to:\n\t%s\n\n", url);
		if (AskYesNoQuestion(0, "Save? (yes/no) ") == 0) {
			(void) printf("Not saved.  (If you don't want to be asked this, \"set confirm-close no\")\n\n\n");
		} else if (PromptForBookmarkName(&gBm) < 0) {
			(void) printf("Nevermind.\n");
		} else {
			SaveCurrentAsBookmark();
		}
	} else if ((gLoadedBm == 1) && (gOurDirectoryPath[0] != '\0') && (strcmp(gOurDirectoryPath, gBm.dir) != 0)) {
		/* Bookmark has changed. */
		if (gAutoSaveChangesToExistingBookmarks != 0) {
			SaveCurrentAsBookmark();
		}
	}
}	/* SaveUnsavedBookmark */



/* Save the current host session settings for later as a "bookmark", which
 * will be referred to by a bookmark abbreviation name.
 */
void
BookmarkCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	/* The only reason we do this is to get gcc/lint to shut up
	 * about unused parameters.
	 */
	ARGSUSED(gUnusedArg);

	if (gOurDirectoryPath[0] == '\0') {
		(void) printf("Sorry, configuration information is not saved for this user.\n");
	} else if ((argc <= 1) || (argv[1][0] == '\0')) {
		/* No name specified on the command line. */
		if (gBm.bookmarkName[0] == '\0') {
			/* Not previously bookmarked. */
			if (PromptForBookmarkName(&gBm) < 0) {
				(void) printf("Nevermind.\n");
			} else {
				SaveCurrentAsBookmark();
			}
		} else {
			/* User wants to update an existing bookmark. */
			SaveCurrentAsBookmark();
		}
	} else {
		(void) STRNCPY(gBm.bookmarkName, argv[1]);
		SaveCurrentAsBookmark();
	}
}	/* BookmarkCmd */




/* Dump a remote file to the screen. */
void
CatCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	int i;

	ARGSUSED(gUnusedArg);
	for (i=1; i<argc; i++) {
		result = FTPGetOneFile2(&gConn, argv[i], NULL, kTypeAscii, STDOUT_FILENO, kResumeNo, kAppendNo);
		FTPPerror(&gConn, result, kErrCouldNotStartDataTransfer, "cat", argv[i]);
	}
}	/* CatCmd */



static void
NcFTPCdResponseProc(const FTPCIPtr cipUNUSED, ResponsePtr rp)
{
	FTPLinePtr lp;
	FTPLineListPtr llp;

	gUnusedArg = (cipUNUSED != NULL);
	if ((rp->printMode & kResponseNoPrint) != 0)
		return;
	llp = &rp->msg;
	for (lp = llp->first; lp != NULL; lp = lp->next) {
		if ((lp == llp->first) && (rp->codeType == 2)) {
			if (ISTRNCMP(lp->line, "CWD command", 11) == 0)
				continue;
			if (lp->line[0] == '"')
				continue;	/* "/pub/foo" is... */
		}
		(void) printf("%s\n", lp->line);
	}
}	/* NcFTPCdResponseProc */




/* Manually print a response obtained from the remote FTP user. */
void
PrintResp(FTPLineListPtr llp)
{
	FTPLinePtr lp;
	
	if (llp != NULL) {
		for (lp = llp->first; lp != NULL; lp = lp->next) {
			if ((lp == llp->first) && (ISTRNCMP(lp->line, "CWD command", 11) == 0))
				continue;
			(void) printf("%s\n", lp->line);
		}
	}
}	/* PrintResp */




/* Do a chdir, and update our notion of the current directory.
 * Some servers return it back as part of the CWD response,
 * otherwise do a CWD command followed by a PWD.
 */
int
nFTPChdirAndGetCWD(const FTPCIPtr cip, const char *cdCwd, const int quietMode)
{
	ResponsePtr rp;
	int result;
#ifdef USE_WHAT_SERVER_SAYS_IS_CWD
	int foundcwd;
	char *l, *r;
#endif

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((cdCwd == NULL) || (cdCwd[0] == '\0')) {
		result = kErrInvalidDirParam;
		cip->errNo = kErrInvalidDirParam;
	} else {
		rp = InitResponse();
		if (rp == NULL) {
			result = kErrMallocFailed;
			cip->errNo = kErrMallocFailed;
			/* Error(cip, kDontPerror, "Malloc failed.\n"); */
		} else {
			if (strcmp(cdCwd, "..") == 0) {
				result = RCmd(cip, rp, "CDUP"); 	
			} else {
				result = RCmd(cip, rp, "CWD %s", cdCwd);
			}
			if (result == 2) {
#ifdef USE_WHAT_SERVER_SAYS_IS_CWD
				(void) STRNCPY(gScratchCWD, gRemoteCWD);
				foundcwd = 0;
				if ((r = strrchr(rp->msg.first->line, '"')) != NULL) {
					/* "xxxx" is current directory.
					 * Strip out just the xxxx to copy into the remote cwd.
					 *
					 * This is nice because we didn't have to do a PWD.
					 */
					l = strchr(rp->msg.first->line, '"');
					if ((l != NULL) && (l != r) && (l == rp->msg.first->line)) {
						*r = '\0';
						++l;
						(void) Strncpy(gRemoteCWD, l, sizeof(gRemoteCWD));
						*r = '"';	/* Restore, so response prints correctly. */
						foundcwd = 1;
						result = kNoErr;
					}
				}
				if (quietMode)
					rp->printMode |= kResponseNoPrint;
				NcFTPCdResponseProc(cip, rp);
				DoneWithResponse(cip, rp);
				if (foundcwd == 0) {
					result = FTPGetCWD(cip, gRemoteCWD, sizeof(gRemoteCWD));
					if (result != kNoErr) {
						PathCat(gRemoteCWD, sizeof(gRemoteCWD), gScratchCWD, cdCwd, gServerUsesMSDOSPaths);
						result = kNoErr;
					}
				}
#else /* USE_CLIENT_SIDE_CALCULATED_CWD */
				if (quietMode)
					rp->printMode |= kResponseNoPrint;
				NcFTPCdResponseProc(cip, rp);
				DoneWithResponse(cip, rp);
				(void) STRNCPY(gScratchCWD, gRemoteCWD);
				PathCat(gRemoteCWD, sizeof(gRemoteCWD), gScratchCWD, cdCwd, gServerUsesMSDOSPaths);
				result = kNoErr;
#endif
			} else if (result > 0) {
				result = kErrCWDFailed;
				cip->errNo = kErrCWDFailed;
				DoneWithResponse(cip, rp);
			} else {
				DoneWithResponse(cip, rp);
			}
		}
	}
	return (result);
}	/* nFTPChdirAndGetCWD */




int
Chdirs(FTPCIPtr cip, const char *const cdCwd)
{
	char *cp, *startcp;
	int result;
	int lastSubDir;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cdCwd == NULL) {
		result = kErrInvalidDirParam;
		cip->errNo = kErrInvalidDirParam;
		return result;
	}
	
	if ((cdCwd[0] == '\0') || (strcmp(cdCwd, ".") == 0))
		return (kNoErr);

	cp = cip->buf;
	cp[cip->bufSize - 2] = '\0';
	if ((cdCwd[0] == '.') && (cdCwd[1] == '.') && ((cdCwd[2] == '\0') || IsLocalPathDelim(cdCwd[2]))) {
		PathCat(cip->buf, cip->bufSize, gRemoteCWD, cdCwd, gServerUsesMSDOSPaths);
	} else {
		(void) Strncpy(cip->buf, cdCwd, cip->bufSize);
	}
	if (cp[cip->bufSize - 2] != '\0')
		return (kErrBadParameter);

	StrRemoveTrailingLocalPathDelim(cp);

	/* Try doing the whole path at once. */
	result = nFTPChdirAndGetCWD(cip, cp, 0);
	if (result == kNoErr)
		return (result);

	/* Try doing the path one subdir at a time. */
	do {
		startcp = cp;
		cp = StrFindLocalPathDelim(cp + 0);
		if (cp != NULL) {
			*cp++ = '\0';
		}
		lastSubDir = (cp == NULL);
		result = nFTPChdirAndGetCWD(cip, (*startcp != '\0') ? startcp : "/", lastSubDir ? 0 : 1);
		if (result < 0) {
			cip->errNo = result;
		}
	} while ((!lastSubDir) && (result == kNoErr));

	return (result);
}	/* Chdirs */




/* Remote change of working directory command. */
void
ChdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);

	if (argc <= 1) {
		if (gStartDir[0] != '\0') {
			(void) STRNCPY(gPrevRemoteCWD, gRemoteCWD);
			result = Chdirs(&gConn, gStartDir);
			if (result != kNoErr) {
				/* State is incoherent if this happens! */
				FTPPerror(&gConn, result, kErrCWDFailed, "Could not chdir to", gStartDir);
			}
		} else {
			PrintCmdUsage(cmdp);
		}
	} else {
		InitLineList(&ll);
		result = FTPRemoteGlob(&gConn, &ll, argv[1], (aip->noglobargv[1] != 0) ? kGlobNo: kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, argv[0], argv[1]);
		} else {
			lp = ll.first;
			if ((lp != NULL) && (lp->line != NULL)) {
				if ((strcmp(lp->line, "-") == 0) && (gPrevRemoteCWD[0] != '\0')) {
					free(lp->line);
					lp->line = StrDup(gPrevRemoteCWD);
					if (lp->line == NULL) {
						result = kErrMallocFailed;
						gConn.errNo = kErrMallocFailed;
					} else {
						(void) STRNCPY(gPrevRemoteCWD, gRemoteCWD);
						result = Chdirs(&gConn, lp->line);
					}
				} else {
					(void) STRNCPY(gPrevRemoteCWD, gRemoteCWD);
					result = Chdirs(&gConn, lp->line);
				}
				if (result != kNoErr)
					FTPPerror(&gConn, result, kErrCWDFailed, "Could not chdir to", lp->line);
			}
		}
		DisposeLineListContents(&ll);
	}
}	/* ChdirCmd */




/* Chmod files on the remote host, if it supports it. */
void
ChmodCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i, result;

	ARGSUSED(gUnusedArg);
	for (i=2; i<argc; i++) {
		result = FTPChmod(
				&gConn, argv[i], argv[1],
				(aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes
			);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrChmodFailed, "chmod", argv[i]);
			/* but continue */
		}
	}

	/* Really should just flush only the modified directories... */
	FlushLsCache();
}	/* ChmodCmd */




/* Close the current session to a remote FTP server. */
void
CloseCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	if (gConn.connected == 0)
		(void) printf("Already closed.\n");
	else
		CloseHost();
}	/* CloseCmd */



/* User interface to the program's debug-mode setting. */
void
DebugCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	if (argc > 1)
		SetDebug(atoi(argv[1]));
	else
		SetDebug(!gDebug);
}	/* DebugCmd */




/* Delete files on the remote host. */
void
DeleteCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	int i, c;
	int recursive = kRecursiveNo;
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "rf")) > 0) switch(c) {
		case 'r':
			recursive = kRecursiveYes;
			break;
		case 'f':
			/* ignore */
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}

	for (i=opt.ind; i<argc; i++) {
		result = FTPDelete(
				&gConn, argv[i], recursive,
				(aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes
			);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrDELEFailed, "delete", argv[i]);
			/* but continue */
		}
	}

	/* Really should just flush only the modified directories... */
	FlushLsCache();
}	/* DeleteCmd */




/* Command shell echo command.  This is mostly useful for testing the command
 * shell, as a sample command which prints some output.
 */
void
EchoCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i;
	int result;
	int np = 0;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);
	for (i=1; i<argc; i++) {
		InitLineList(&ll);
		result = FTPLocalGlob(&gConn, &ll, argv[i], (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, "local glob", argv[i]);
		} else {
			for (lp = ll.first; lp != NULL; lp = lp->next) {
				if (lp->line != NULL) {
					if (np > 0)
						(void) printf(" ");
					(void) printf("%s", lp->line);
					np++;	
				}
			}
		}
		DisposeLineListContents(&ll);
	}
	(void) printf("\n");
}	/* EchoCmd */




#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else

static int
AddToListOfFilesToEdit(FTPLineListPtr rfiles, FTPLineListPtr lfiles, char* remoteFile, int createIt)
{
	char tname[260];
	int fd;

	GetTmpDir(tname, sizeof(tname));
	if (tname[0] == '\0')
		return (-1);

	(void) STRNCAT(tname, LOCAL_PATH_DELIM_STR);
	(void) STRNCAT(tname, "ncftp_editcmd.XXXXXX");
	if ((fd = mkstemp(tname)) < 0)
		return (-1);
	(void) close(fd);	/* Want the value in tname for later */

	if ((tname[0] == '\0') || (AddLine(lfiles, tname) == NULL))
		return (-1);

	if ((remoteFile == NULL) || (remoteFile[0] == '\0'))
		return (-1);

	if (createIt != 0) {
		/* Use a special prefix on the remote path to denote that we want to
		 * create it.  The prefix should be a path that is preferably an
		 * illegal pathname, or at least something that would probably
		 * not come up in everyday use.
		 */
		STRNCPY(tname, "//|||//");
		STRNCAT(tname, remoteFile);
		if (AddLine(rfiles, tname) == NULL)
			return (-1);
		return (0);
	}

	if (AddLine(rfiles, remoteFile) == NULL)
		return (-1);

	return (0);
}	/* AddToListOfFilesToEdit */




void
EditCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i, n, n2, result;
	char cmdbuf[2048];
	char modstr[80];
	char *rpath;
	const char *envEDITOR;
	FTPLineList globlist;
	FTPLineList rfiles, lfiles, modstrs;
	FTPLinePtr lp, rlp, llp, mlp;
	FILE *fp;
	struct Stat st;

	ARGSUSED(gUnusedArg);
	envEDITOR = getenv("EDITOR");
	if ((envEDITOR == NULL) || (envEDITOR[0] == '\0')) {
		/* Make them set it in the environment, since EDITOR
		 * is a well-established environment variable, unlike
		 * PAGER.
		 */
		Trace(-1, "Set your EDITOR environment variable prior to running ncftp.\nExample for /bin/sh:\n\n\tEDITOR=\"/usr/bin/vi\" ; export EDITOR\n\n");
		return;
	}
		
	InitLineList(&rfiles);
	InitLineList(&lfiles);

	for (i = 1, n = 0; i < argc; i++) {
		InitLineList(&globlist);
		result = FTPRemoteGlob(&gConn, &globlist, argv[i], kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, "remote glob", argv[i]);
			continue;
		}
		for (lp = globlist.first; lp != NULL; lp = lp->next) {
			result = FTPIsRegularFile(&gConn, lp->line);
			switch (result) {
				case 1:
					if (AddToListOfFilesToEdit(&rfiles, &lfiles, lp->line, 0) < 0) {
						Trace(-1, "failed to AddToListOfFilesToEdit %s\n", lp->line);
						DisposeLineListContents(&globlist);
						DisposeLineListContents(&rfiles);
						DisposeLineListContents(&lfiles);
						time(&gCmdStart);	/* Don't beep when this finishes. */
						return;
					}
					++n;
					break;
				case 0:
					Trace(-1, "Cannot edit directory - ignoring \"%s\"\n", lp->line);
					break;
				default:
					if (AskYesNoQuestion(1, "The file \"%s\" does not exist!  Do you want to create it? (Yes/no) ", lp->line) > 0) {
						if (AddToListOfFilesToEdit(&rfiles, &lfiles, lp->line, 1) < 0) {
							Trace(-1, "failed to AddToListOfFilesToEdit %s\n", lp->line);
							DisposeLineListContents(&globlist);
							DisposeLineListContents(&rfiles);
							DisposeLineListContents(&lfiles);
							time(&gCmdStart);	/* Don't beep when this finishes. */
							return;
						}
						++n;
					}
			}
		}
		DisposeLineListContents(&globlist);
	}

	/* Download the remote files into temporary local files for local editing. */
	InitLineList(&modstrs);
	for (rlp = rfiles.first, llp = lfiles.first, n2 = n; ((rlp != NULL) && (llp != NULL)); rlp = rlp->next, llp = llp->next) {
		/* get the files */
		rpath = rlp->line;
		if (strncmp(rpath, "//|||//", strlen("//|||//")) == 0) {
			rpath += strlen("//|||//");
			if ((fp = fopen(llp->line, "w")) == NULL) {
				Trace(0, "Could not create temporary file \"%s\" : %s\n", llp->line, strerror(errno));
				DisposeLineListContents(&modstrs);
				DisposeLineListContents(&rfiles);
				DisposeLineListContents(&lfiles);
				time(&gCmdStart);	/* Don't beep when this finishes. */
				return;
			}
			(void) fprintf(fp, "This will be the remote file \"%s\" after you finish editing this file.\n", rpath);
			(void) fclose(fp);	/* Just want a new zero-byte file for now. */
			result = kNoErr;
			/* Don't download it, we're creating from scratch. */
		} else {
			if (n2 > 0)
				(void) fprintf(stdout, "\n");
			Trace(-1, "Please wait while %s is downloaded for editing.\n", rpath);
			result = FTPGetOneFile3(&gConn, rpath, llp->line, kTypeBinary, -1, kResumeNo, kAppendNo, kDeleteNo, kNoFTPConfirmResumeDownloadProc, 0);
		}
		if (result < 0) {
			FTPPerror(&gConn, result, kErrCouldNotStartDataTransfer, "get for editing", rpath);
			rlp->line[0] = '\0';	/* Mark it as an error */
			--n;
			memset(&st, 0, sizeof(st));
		} else if (Stat(llp->line, &st) < 0) {
			/* Huh? It was deleted already. */
			Trace(-1, "Cannot edit \"%s\", as the local file \"%s\" was deleted after it was downloaded.\n", rpath, llp->line);
			rlp->line[0] = '\0';	/* Mark it as an error */
			llp->line[0] = '\0';	/* Mark it so we do not delete it. */
			--n;
			memset(&st, 0, sizeof(st));
		}
#ifdef PRINTF_ULONG_LONG
		(void) sprintf(modstr, "%u " PRINTF_ULONG_LONG, (unsigned int) st.st_mtime, (longest_uint) st.st_size);
#else
		(void) sprintf(modstr, "%u %lu" , (unsigned int) st.st_mtime,  (unsigned long) st.st_size);
#endif
		if (AddLine(&modstrs, modstr) == NULL) {
			DisposeLineListContents(&modstrs);
			DisposeLineListContents(&rfiles);
			DisposeLineListContents(&lfiles);
			time(&gCmdStart);	/* Don't beep when this finishes. */
			return;
		}
	}

	if (n <= 0) {
		Trace(-1, "Nothing to edit!\n");
		DisposeLineListContents(&modstrs);
		DisposeLineListContents(&rfiles);
		DisposeLineListContents(&lfiles);
		time(&gCmdStart);	/* Don't beep when this finishes. */
		return;
	}

	/* Edit the files in $EDITOR. */
	(void) STRNCPY(cmdbuf, "exec ");	/* use exec to avoid having a shell hanging around */
	(void) STRNCAT(cmdbuf, envEDITOR);
	for (rlp = rfiles.first, llp = lfiles.first; ((rlp != NULL) && (llp != NULL)); rlp = rlp->next, llp = llp->next) {
		if ((rlp->line[0] == '\0') || (llp->line[0] == '\0'))
			continue;
		(void) STRNCAT(cmdbuf, " \'");
		(void) STRNCAT(cmdbuf, llp->line);
		(void) STRNCAT(cmdbuf, "\'");
	}

	if (n2 > 1) {
		(void) fprintf(stdout, "\n");
		(void) AskYesNoQuestion(1, "All files downloaded.  Press ENTER to start editing. ");
	}

	result = system(cmdbuf);
	(void) fprintf(stdout, "\n");
#if ((defined(WIFEXITED)) && (defined(WEXITSTATUS)))
	if ((result == 0) || WIFEXITED(result)) {
		Trace(0, "%.480s (es=%d)\n", cmdbuf, WEXITSTATUS(result));
	} else {
		Trace(0, "%.480s (es=0x%08x)\n", cmdbuf, result);
	}
#else
	Trace(0, "%.480s (es=0x%08x)\n", cmdbuf, result);
#endif
	/* Upload changed files. */
	for (rlp = rfiles.first, llp = lfiles.first, mlp = modstrs.first; ((rlp != NULL) && (llp != NULL) && (mlp != NULL)); rlp = rlp->next, llp = llp->next, mlp = mlp->next) {
		if ((rlp->line[0] == '\0') || (llp->line[0] == '\0'))
			continue;
		rpath = rlp->line;
		if (strncmp(rpath, "//|||//", strlen("//|||//")) == 0)
			rpath += strlen("//|||//");	/* Skip over the Create-from-scratch magic cookie. */
		if (Stat(llp->line, &st) < 0) {
			Trace(-1, "Cannot upload edited file \"%s\", as the local file \"%s\" was deleted after it was downloaded.\n", rpath, llp->line);
			rlp->line[0] = '\0';	/* Mark it as an error */
			llp->line[0] = '\0';	/* Mark it so we do not delete it. */
			--n;
			(void) fprintf(stdout, "\n");
			continue;
		}
#ifdef PRINTF_ULONG_LONG
		(void) sprintf(modstr, "%u " PRINTF_ULONG_LONG, (unsigned int) st.st_mtime, (longest_uint) st.st_size);
#else
		(void) sprintf(modstr, "%u %lu", (unsigned int) st.st_mtime, (unsigned long) st.st_size);
#endif
		if (strcmp(modstr, mlp->line) == 0) {
			Trace(-1, "No changes made to \"%s\".\n", rpath);
			continue;
		}
		if (AskYesNoQuestion(-1, "Put edited file \"%s\" ? (yes/no) ", rpath) > 0) {
			result = FTPPutOneFile3(&gConn, llp->line, rpath, kTypeBinary, -1, kAppendNo, NULL, NULL, kResumeNo, kDeleteNo, kNoFTPConfirmResumeUploadProc, 0);
			if (result < 0) {
				FTPPerror(&gConn, result, kErrCouldNotStartDataTransfer, "put", rpath);
				Trace(-1, "Local changes to \"%s\" can be recovered from \"%s\".\n", rpath, llp->line);
				llp->line[0] = '\0';	/* Mark it so we do not delete it. */
				(void) fprintf(stdout, "\n");
			}
		}
	}

	for (llp = lfiles.first; llp != NULL; llp = llp->next) {
		if (llp->line[0] != '\0') {
			if (remove(llp->line) < 0)
				Trace(0, "Could not remove temporary file \"%s\" : %s\n", llp->line, strerror(errno));
		}
	}

	DisposeLineListContents(&modstrs);
	DisposeLineListContents(&rfiles);
	DisposeLineListContents(&lfiles);
	FlushLsCache();
	time(&gCmdStart);	/* Don't beep when this finishes. */
}	/* EditCmd */

#endif	/* ! WINDOWS */





static int
NcFTPConfirmResumeDownloadProc(
	const FTPCIPtr cipUNUSED,
	const char **localpath,
	longest_int localsize,
	time_t localmtime,
	const char *remotepath,
	longest_int remotesize,
	time_t remotemtime,
	longest_int *startPoint)
{
	int zaction = kConfirmResumeProcSaidBestGuess;
	char tstr[80], ans[32];
	static char newname[128];	/* arrggh... static. */
	struct tm t;

	gUnusedArg = (cipUNUSED != NULL);
	if (gResumeAnswerAll != kConfirmResumeProcNotUsed) {
		if (gResumeAnswerAll == kConfirmResumeProcSaidResume) {
			if ((gConn.hasREST != kCommandAvailable) || (remotesize == kSizeUnknown)) {
				/* Resume is not available on this server. */
				return (zaction);
			} else if (remotesize < localsize) {
				/* Cannot resume when local file is already larger than the remote file. */
				return (zaction);
			} else if (remotesize <= localsize) {
				/* Local file is already the same size as the remote file. */
				return (zaction);
			}
			*startPoint = localsize;
		}
		return (gResumeAnswerAll);
	}

	if ((gAutoResume != 0) || (gIsTTYr == 0))
		return (kConfirmResumeProcSaidBestGuess);

	tstr[sizeof(tstr) - 1] = '\0';
	(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(localmtime, &t));
	(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
		"\nThe local file \"%s\" already exists.\n\tLocal:  %12lld bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
		"\nThe local file \"%s\" already exists.\n\tLocal:  %12qd bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
		"\nThe local file \"%s\" already exists.\n\tLocal:  %12I64d bytes, dated %s.\n",
#else
		"\nThe local file \"%s\" already exists.\n\tLocal:  %12ld bytes, dated %s.\n",
#endif
		*localpath,
		localsize,
		tstr
	);

	if ((remotemtime != kModTimeUnknown) && (remotesize != kSizeUnknown)) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(remotemtime, &t));
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tRemote: %12lld bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tRemote: %12qd bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tRemote: %12I64d bytes, dated %s.\n",
#else
			"\tRemote: %12ld bytes, dated %s.\n",
#endif
			remotesize,
			tstr
		);
		if ((remotemtime == localmtime) && (remotesize == localsize)) {
			(void) printf("\t(Files are identical, skipped)\n\n");
			return (kConfirmResumeProcSaidSkip);
		}
	} else if (remotesize != kSizeUnknown) {
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tRemote: %12lld bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tRemote: %12qd bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tRemote: %12I64d bytes, date unknown.\n",
#else
			"\tRemote: %12ld bytes, date unknown.\n",
#endif
			remotesize
		);
	} else if (remotemtime != kModTimeUnknown) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(remotemtime, &t));
		(void) printf(
			"\tRemote: size unknown, dated %s.\n",
			tstr
		);
	}

	printf("\n");
	(void) memset(ans, 0, sizeof(ans));
	for (;;) {
		(void) printf("\t[O]verwrite?");
		if ((gConn.hasREST == kCommandAvailable) && (remotesize != kSizeUnknown) && (remotesize > localsize))
			printf("  [R]esume?");
		printf("  [A]ppend to?  [S]kip?  [N]ew Name?\n");
		(void) printf("\t[O!]verwrite all?");
		if ((gConn.hasREST == kCommandAvailable) && (remotesize != kSizeUnknown) && (remotesize > localsize))
			printf("  [R!]esume all?");
		printf("  [S!]kip all?  [C]ancel  > ");
		fflush(stdin);
		(void) fgets(ans, sizeof(ans) - 1, stdin);
		switch ((int) ans[0]) {
			case 'c':
			case 'C':
				ans[0] = 'C';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidCancel;
				break;
			case 'o':
			case 'O':
				ans[0] = 'O';
				zaction = kConfirmResumeProcSaidOverwrite;
				break;
			case 'r':
			case 'R':
				if ((gConn.hasREST != kCommandAvailable) || (remotesize == kSizeUnknown)) {
					printf("\tResume is not available on this server.\n\n");
					ans[0] = '\0';
					break;
				} else if (remotesize < localsize) {
					printf("\tCannot resume when local file is already larger than the remote file.\n\n");
					ans[0] = '\0';
					break;
				} else if (remotesize <= localsize) {
					printf("\tLocal file is already the same size as the remote file.\n\n");
					ans[0] = '\0';
					break;
				}
				ans[0] = 'R';
				*startPoint = localsize;
				zaction = kConfirmResumeProcSaidResume;
				if (OneTimeMessage("auto-resume") != 0) {
					printf("\n\tNOTE: If you want NcFTP to guess automatically, \"set auto-resume yes\"\n\n");
				}
				break;
			case 's':
			case 'S':
				ans[0] = 'S';
				zaction = kConfirmResumeProcSaidSkip;
				break;
			case 'n':
			case 'N':
				ans[0] = 'N';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidOverwrite;
				break;
			case 'a':
			case 'A':
				ans[0] = 'A';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidAppend;
				break;
			case 'g':
			case 'G':
				ans[0] = 'G';
				zaction = kConfirmResumeProcSaidBestGuess;
				break;
			default:
				ans[0] = '\0';
		}
		if (ans[0] != '\0')
			break;
	}

	if (ans[0] == 'N') {
		(void) memset(newname, 0, sizeof(newname));
		printf("\tSave as:  ");
		fflush(stdin);
		(void) fgets(newname, sizeof(newname) - 1, stdin);
		newname[strlen(newname) - 1] = '\0';
		if (newname[0] == '\0') {
			/* Nevermind. */
			printf("Skipped %s.\n", remotepath);
			zaction = kConfirmResumeProcSaidSkip;
		} else {
			*localpath = newname;
		}
	}

	if (ans[1] == '!')
		gResumeAnswerAll = zaction;
	return (zaction);
}	/* NcFTPConfirmResumeDownloadProc */




/* Download files from the remote system. */
void
GetCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int optrc;
	int renameMode = 0;
	int recurseFlag = kRecursiveNo;
	int appendFlag = kAppendNo;
	int resumeFlag = kResumeYes;
	int tarflag = kTarYes;
	const char *dstdir = NULL;
	int rc;
	int i;
	int doGlob;
	int xtype = gBm.xferType;
	int nD = 0;
	int deleteFlag = kDeleteNo;
	char pattern[256];
	vsigproc_t osigint;
	FTPConfirmResumeDownloadProc confirmProc;
	GetoptInfo opt;

	confirmProc = NcFTPConfirmResumeDownloadProc;
	gResumeAnswerAll = kConfirmResumeProcNotUsed;	/* Ask at least once each time */
	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "aAzfrRTD")) >= 0) switch (optrc) {
		case 'a':
			xtype = kTypeAscii;
			break;
		case 'A':
			/* Append to local files, instead of truncating
			 * them first.
			 */
			appendFlag = kAppendYes;
			break;
		case 'f':
		case 'Z':
			/* Do not try to resume a download, even if it
			 * appeared that some of the file was transferred
			 * already.
			 */
			resumeFlag = kResumeNo;
			confirmProc = kNoFTPConfirmResumeDownloadProc;
			break;
		case 'z':
			/* Special flag that lets you specify the
			 * destination file.  Normally a "get" will
			 * write the file by the same name as the
			 * remote file's basename.
			 */
			renameMode = 1;
			break;
		case 'r':
		case 'R':
			/* If the item is a directory, get the
			 * directory and all its contents.
			 */
			recurseFlag = kRecursiveYes;
			break;
		case 'T':
			/* If they said "-R", they may want to
			 * turn off TAR mode if they are trying
			 * to resume the whole directory.
			 * The disadvantage to TAR mode is that
			 * it always downloads the whole thing,
			 * which is why there is a flag to
			 * disable this.
			 */
			tarflag = kTarNo;
			break;
		case 'D':
			/* You can delete the remote file after
			 * you downloaded it successfully by using
			 * the -DD option.  It requires two -D's
			 * to minimize the odds of accidentally
			 * using a single -D.
			 */
			nD++;
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}

	if (nD >= 2)
		deleteFlag = kDeleteYes;

	if (renameMode != 0) {
		if (opt.ind > argc - 2) {
			PrintCmdUsage(cmdp);
			(void) fprintf(stderr, "\nFor get with rename, try \"get -z remote-path-name local-path-name\".\n");
			return;
		}
		osigint = NcSignal(SIGINT, XferCanceller);
		rc = FTPGetOneFile3(&gConn, argv[opt.ind], argv[opt.ind + 1], xtype, (-1), resumeFlag, appendFlag, deleteFlag, kNoFTPConfirmResumeDownloadProc, 0);
		if (rc < 0)
			FTPPerror(&gConn, rc, kErrCouldNotStartDataTransfer, "get", argv[opt.ind]);
	} else {
		osigint = NcSignal(SIGINT, XferCanceller);
		for (i=opt.ind; i<argc; i++) {
			doGlob = (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes;
			STRNCPY(pattern, argv[i]);
			StrRemoveTrailingSlashes(pattern);
			rc = FTPGetFiles3(&gConn, pattern, dstdir, recurseFlag, doGlob, xtype, resumeFlag, appendFlag, deleteFlag, tarflag, confirmProc, 0);
			if (rc < 0)
				FTPPerror(&gConn, rc, kErrCouldNotStartDataTransfer, "get", argv[i]);
		}
	}
	(void) NcSignal(SIGINT, osigint);
	(void) fflush(stdin);

	if (deleteFlag == kDeleteYes) {
		/* Directory is now out of date */
		FlushLsCache();
	}
}	/* GetCmd */




/* Display some brief help for specified commands, or a list of commands. */
void
HelpCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	CommandPtr c;
	int showall = 0, helpall = 0;
	int i, j, k, n;
	int nRows, nCols;
	int nCmds2Print;
	int screenColumns;
	int len, widestName;
	char *cp, spec[16];
	const char *cmdnames[64];

	ARGSUSED(gUnusedArg);
	assert(gNumCommands < (sizeof(cmdnames) / sizeof(char *)));
	if (argc == 2) {
		showall = (strcmp(argv[1], "showall") == 0);
		helpall = (strcmp(argv[1], "helpall") == 0);
	}
	if (argc == 1 || showall) {
		(void) printf("\
Commands may be abbreviated.  'help showall' shows hidden and unsupported \n\
commands.  'help <command>' gives a brief description of <command>.\n\n");

		/* First, see how many commands we will be printing to the screen.
		 * Unless 'showall' was given, we won't be printing the hidden
		 * (i.e. not very useful to the end-user) commands.
		 */
		c = gCommands;
		nCmds2Print = 0;
		for (n = 0; n < (int) gNumCommands; c++, n++)
			if ((!iscntrl((int) c->name[0])) && (!(c->flags & kCmdHidden) || showall))
				nCmds2Print++;

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		(void) memset((void *) cmdnames, 0, sizeof(cmdnames));
#else
		(void) memset(cmdnames, 0, sizeof(cmdnames));
#endif
		/* Now form the list we'll be printing, and noting what the maximum
		 * length of a command name was, so we can use that when determining
		 * how to print in columns.
		 */
		c = gCommands;
		i = 0;
		widestName = 0;
		for (n = 0; n < (int) gNumCommands; c++, n++) {
			if ((!iscntrl((int) c->name[0])) && (!(c->flags & kCmdHidden) || showall)) {
				cmdnames[i++] = c->name;
				len = (int) strlen(c->name);
				if (len > widestName)
					widestName = len;
			}
		}

		if ((cp = (char *) getenv("COLUMNS")) == NULL)
			screenColumns = 80;
		else
			screenColumns = atoi(cp);

		/* Leave an extra bit of whitespace for the margins between columns. */
		widestName += 2;
		
		nCols = (screenColumns + 0) / widestName;
		nRows = nCmds2Print / nCols;
		if ((nCmds2Print % nCols) > 0)
			nRows++;

		for (i = 0; i < nRows; i++) {
			for (j = 0; j < nCols; j++) {
				k = nRows * j + i;
				if (k < nCmds2Print) {
					(void) sprintf(spec, "%%-%ds",
						(j < nCols - 1) ? widestName : widestName - 2
					);
					(void) printf(spec, cmdnames[k]);
				}
			}
			(void) printf("\n");
		}
	} else if (helpall) {
		/* Really intended for me, so I can debug the help strings. */
		for (c = gCommands, n = 0; n < (int) gNumCommands; c++, n++) {
			PrintCmdHelp(c);
			PrintCmdUsage(c);
		}
	} else {
		/* For each command name specified, print its help stuff. */
		for (i=1; i<argc; i++) {
			c = GetCommandByName(argv[i], 0);
			if (c == kAmbiguousCommand) {
				(void) printf("%s: ambiguous command name.\n", argv[i]);
			} else if (c == kNoCommand) {
				(void) printf("%s: no such command.\n", argv[i]);
			} else {
				if (i > 1)
					(void) printf("\n");
				PrintCmdHelp(c);
				PrintCmdUsage(c);
			}
		}
	}
	printf("\nFor details, please see the manual (\"man ncftp\" at your regular shell prompt\nor online at http://www.NcFTP.com/ncftp/doc/ncftp.html).\n");
}	/* HelpCmd */




/* Displays the list of saved bookmarks, so that the user can then choose
 * one by name.
 */
static int
PrintHosts(void)
{
	FILE *bookmarks;
	FILE *pager;
	int nbm = 0;
	Bookmark bm;
	char url[128];
#ifdef SIGPIPE
	sigproc_t osigpipe;
#endif

	bookmarks = OpenBookmarkFile(NULL);
	if (bookmarks == NULL)
		return (0);

#ifdef SIGPIPE
	osigpipe = (sigproc_t) NcSignal(SIGPIPE, SIG_IGN);
#endif
	pager = OpenPager();

	while (GetNextBookmark(bookmarks, &bm) == 0) {
		nbm++;
		if (nbm == 1) {
			/* header */
			(void) fprintf(pager, "--BOOKMARK----------URL--------------------------------------------------------\n");
		}
		BookmarkToURL(&bm, url, sizeof(url));
		(void) fprintf(pager, "  %-16s  %s\n", bm.bookmarkName, url);
	}

	ClosePager(pager);
	CloseBookmarkFile(bookmarks);

#ifdef SIGPIPE
	(void) NcSignal(SIGPIPE, osigpipe);
#endif
	return (nbm);
}	/* PrintHosts */





static int
RunBookmarkEditor(char *selectedBmName, size_t dsize)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	char ncftpbookmarks[260];
	const char *prog;
	int winExecResult;
	HANDLE hMailSlot;
	char msg[kNcFTPBookmarksMailslotMsgSize];
	DWORD dwRead;
	BOOL rc;

	if (selectedBmName != NULL)
		memset(selectedBmName, 0, dsize);
	if (gOurInstallationPath[0] == '\0') {
		(void) fprintf(stderr, "Cannot find path to %s.  Please re-run Setup.\n", "ncftpbookmarks.exe");
		return (-1);
	}
	prog = ncftpbookmarks;
	OurInstallationPath(ncftpbookmarks, sizeof(ncftpbookmarks), "ncftpbookmarks.exe");


	hMailSlot = CreateMailslot(kNcFTPBookmarksMailslot, kNcFTPBookmarksMailslotMsgSize, MAILSLOT_WAIT_FOREVER, NULL); 

	if (hMailSlot == INVALID_HANDLE_VALUE) {
		SysPerror("CreateMailslot");
		(void) fprintf(stderr, "Could not create communication channel with %s.\n", "ncftpbookmarks.exe");
		(void) fprintf(stderr, "%s", "This means if you select a bookmark to connect to that NcFTP\n");
		(void) fprintf(stderr, "%s", "will not get the message from %s.\n", "ncftpbookmarks.exe");
	}
 
	winExecResult = WinExec(prog, SW_SHOWNORMAL);
	if (winExecResult <= 31) switch (winExecResult) {
		case ERROR_BAD_FORMAT:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The .EXE file is invalid");
			return (-1);
		case ERROR_FILE_NOT_FOUND:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The specified file was not found.");
			return (-1);
		case ERROR_PATH_NOT_FOUND:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The specified path was not found.");
			return (-1);
		default:
			fprintf(stderr, "Could not run %s: Unknown error #%d.\n", prog, winExecResult);
			return (-1);
	}

	if (hMailSlot != INVALID_HANDLE_VALUE) {
		fprintf(stdout, "Waiting for %s to exit...\n", "ncftpbookmarks.exe");
		ZeroMemory(msg, sizeof(msg));
		dwRead = 0;
		rc = ReadFile(
			hMailSlot,
			msg,
			sizeof(msg),
			&dwRead,
			NULL
			);
		
		if (!rc) {
			SysPerror("ReadFile");
		} else {
			msg[sizeof(msg) - 1] = '\0';
			Strncpy(selectedBmName, msg, dsize);
			Trace(0, "Selected bookmark from editor: [%s]\n", selectedBmName);
		}
		CloseHandle(hMailSlot);
	}
	return (0);

#else	/* ! Windows */
	char ncftpbookmarks[256];
	char *av[8];
	int pid;
	int status;
	char bmSelectionFile[256];
	char pidStr[32];
	FILE *fp;

	if (selectedBmName != NULL)
		memset(selectedBmName, 0, dsize);
#ifdef BINDIR
	STRNCPY(ncftpbookmarks, BINDIR);
	STRNCAT(ncftpbookmarks, "/");
	STRNCAT(ncftpbookmarks, "ncftpbookmarks");
#else
	/* Full pathname to ncftpbookmarks is not available.
	 * We'll have to run it without a fully-qualified
	 * pathname and hope the program is installed somewhere
	 * in the user's $PATH.
	 */
	if (geteuid() == 0) {
		/* Don't assume root's $PATH is safe. */
		return (-1);
	}
	STRNCPY(ncftpbookmarks, "ncftpbookmarks");
#endif
	STRNCPY(bmSelectionFile, "view");
	if ((selectedBmName != NULL) && (gOurDirectoryPath[0] != '\0')) {
		sprintf(pidStr, ".%u", (unsigned int) getpid());
		OurDirectoryPath(bmSelectionFile, sizeof(bmSelectionFile), kOpenSelectedBookmarkFileName);
		STRNCAT(bmSelectionFile, pidStr);
	}

#ifdef BINDIR
	if (access(ncftpbookmarks, X_OK) < 0) {
		return (-1);
	}
#endif
	pid = (int) vfork();
	if (pid < 0) {
		return (-1);
	} else if (pid == 0) {
		/* child */

		av[0] = strdup("ncftpbookmarks");
		av[1] = strdup(bmSelectionFile);
		av[2] = NULL;
		execv(ncftpbookmarks, av);
		exit(1);
	} else {
		/* parent NcFTP */
		for (;;) {
#ifdef HAVE_WAITPID
			if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR))
				break;
#else
			if ((wait(&status) < 0) && (errno != EINTR))
				break;
#endif
			if (WIFEXITED(status) || WIFSIGNALED(status))
				break;		/* done */
		}

		if (strcmp(bmSelectionFile, "view") != 0) {
			fp = fopen(bmSelectionFile, FOPEN_READ_TEXT);
			if (fp != NULL) {
				(void) FGets(selectedBmName, dsize, fp);
				(void) fclose(fp);
				(void) unlink(bmSelectionFile);
				Trace(0, "Selected bookmark from editor: [%s]\n", selectedBmName);
			}
		}
		return (0);
	}
	return (-1);
#endif	/* ! Windows */
}	/* RunBookmarkEditor */



/* This just shows the list of saved bookmarks. */
void
HostsCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	char *av[3];
	char bm[128];

	ARGSUSED(gUnusedArg);
	/* Skip visual mode if "-l". */
	if ((argc == 1) && (RunBookmarkEditor(bm, sizeof(bm)) == 0)) {
		if (bm[0] != '\0') {
			av[0] = strdup("open");
			av[1] = strdup(bm);
			av[2] = NULL;
			OpenCmd(2, av, (CommandPtr) 0, (ArgvInfoPtr) 0);
			free(av[0]);
			free(av[1]);
		}
		return;
	}
	if (PrintHosts() <= 0) {
		(void) printf("You haven't bookmarked any FTP sites.\n");
		(void) printf("Before closing a site, you can use the \"bookmark\" command to save the current\nhost and directory for next time.\n");
	} else {
		(void) printf("\nTo use a bookmark, use the \"open\" command with the name of the bookmark.\n");
	}
}	/* HostsCmd */




/* Show active background ncftp (ncftpbatch) jobs. */
void
JobsCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	Jobs();
}	/* JobsCmd */




/* Do a "ls" on the remote system and display the directory contents to our
 * user.  This command handles "dir" (ls -l), "ls", and "nlist" (ls -1).
 */
void
ListCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	volatile int i;
	int j;
	char options[32];
	char option[2];
	volatile int listmode;
	FILE *volatile stream;
	volatile int paging;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	int sj;
	vsigproc_t osigpipe, osigint;
#endif

	ARGSUSED(gUnusedArg);
	stream = stdout;
	paging = 0;

	if (argv[0][0] == 'd') {
		/* dir */
		listmode = 'l';
	} else if (argv[0][0] == 'n') {
		/* nlist */
		listmode = '1';
	} else if (argv[0][0] == 'p') {
		/* pager */
		paging = 1;

		if (argv[0][1] == 'd') {
			/* dir */
			listmode = 'l';
		} else if (argv[0][1] == 'n') {
			/* nlist */
			listmode = '1';
		} else {
			/* ls */
			listmode = 'C';
		}
	} else {
		/* ls */
		listmode = 'C';
	}
	options[0] = '\0';
	option[1] = '\0';

	for (i=1; i<argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (argv[i][1] == '-') {
			if (argv[i][2] == '\0') {
				/* end of options. */
				++i;
				break;
			} else {
				/* GNU-esque long --option? */
				PrintCmdUsage(cmdp);
			}
		} else {
			for (j=1; ; j++) {
				option[0] = argv[i][j];
				if (argv[i][j] == '\0')
					break;
				switch (argv[i][j]) {
					case 'l':
						listmode = 'l';
						break;
					case '1':
						listmode = '1';
						break;
					case 'C':
						listmode = 'C';
						break;
					default:
						(void) STRNCAT(options, option);
						break;
				}
			}
		}
	}


	if (paging != 0) {
		stream = OpenPager();
		if (stream == NULL) {
			return;
		}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#elif defined(HAVE_SIGSETJMP)
		osigpipe = osigint = (sigproc_t) 0;
		sj = sigsetjmp(gCancelJmp, 1);
#else	/* HAVE_SIGSETJMP */
		osigpipe = osigint = (sigproc_t) 0;
		sj = setjmp(gCancelJmp);
#endif	/* HAVE_SIGSETJMP */

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		if (sj != 0) {
			/* Caught a signal. */
			(void) NcSignal(SIGPIPE, (FTPSigProc) SIG_IGN);
			ClosePager(stream);
			(void) NcSignal(SIGPIPE, osigpipe);
			(void) NcSignal(SIGINT, osigint);
			Trace(0, "Canceled because of signal %d.\n", gGotSig);
			(void) fprintf(stderr, "Canceled.\n");
			gMayCancelJmp = 0;
			return;
		} else {
			osigpipe = NcSignal(SIGPIPE, Cancel);
			osigint = NcSignal(SIGINT, Cancel);
			gMayCancelJmp = 1;
		}
#endif
	}

	if (argv[i] == NULL) {
		/* No directory specified, use cwd. */
		Ls(NULL, listmode, options, stream);
	} else {
		/* List each item. */
		for ( ; i<argc; i++) {
			Ls(argv[i], listmode, options, stream);
		}
	}

	if (paging != 0) {
		ClosePager(stream);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		(void) NcSignal(SIGPIPE, osigpipe);
		(void) NcSignal(SIGINT, osigint);
#endif
	}
	gMayCancelJmp = 0;
}	/* ListCmd */




/* Does a change of working directory on the local host. */
void
LocalChdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	const char *dir;
	char buf[512];

	ARGSUSED(gUnusedArg);
	dir = ((argc < 2) ? gHome : argv[1]);
	if ((dir[0] == '-') && (dir[1] == '\0')) {
		if (gPrevLocalCWD[0] == '\0') {
			(void) fprintf(stderr, "No previous local working directory to switch to.\n");
			return;
		} else {
			dir = gPrevLocalCWD;
		}
	} else if (dir[0] == '~') {
		if (dir[1] == '\0') {
			dir = gHome;
		} else if (dir[1] == '/') {
			(void) STRNCPY(buf, gHome);
			dir = STRNCAT(buf, dir + 1);
		}
	}
	result = chdir(dir);
	if (result < 0) {
		perror(dir);
	} else {
		(void) STRNCPY(gPrevLocalCWD, gLocalCWD);
		(void) FTPGetLocalCWD(gLocalCWD, sizeof(gLocalCWD));

	}
}	/* LocalChdirCmd */




/* Does directory listing on the local host. */
void
LocalListCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	volatile int i;
	int j;
	char options[32];
	char option[2];
	volatile int listmode;
	FILE *volatile stream;
	volatile int paging;


	ARGSUSED(gUnusedArg);
	stream = stdout;
	paging = 0;

	if (argv[0][1] == 'd') {
		/* dir */
		listmode = 'l';
	} else if (argv[0][1] == 'n') {
		/* nlist */
		listmode = '1';
	} else {
		/* ls */
		listmode = 'C';
	}
	options[0] = '\0';
	option[1] = '\0';

	for (i=1; i<argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (argv[i][1] == '-') {
			if (argv[i][2] == '\0') {
				/* end of options. */
				++i;
				break;
			} else {
				/* GNU-esque long --option? */
				PrintCmdUsage(cmdp);
			}
		} else {
			for (j=1; ; j++) {
				option[0] = argv[i][j];
				if (argv[i][j] == '\0')
					break;
				switch (argv[i][j]) {
					case 'l':
						listmode = 'l';
						break;
					case '1':
						listmode = '1';
						break;
					case 'C':
						listmode = 'C';
						break;
					default:
						(void) STRNCAT(options, option);
						break;
				}
			}
		}
	}


	if (argv[i] == NULL) {
		/* No directory specified, use cwd. */
		LLs(NULL, listmode, options, stream);
	} else {
		/* List each item. */
		for ( ; i<argc; i++) {
			LLs(argv[i], listmode, options, stream);
		}
	}

#else
	FILE *volatile outfp;
	FILE *volatile infp;
	int i;
	int sj;
	int dashopts;
	char incmd[256];
	char line[256];
	vsigproc_t osigpipe = (sigproc_t) 0, osigint = (sigproc_t) 0;

	ARGSUSED(gUnusedArg);
	(void) fflush(stdin);
	outfp = OpenPager();

	(void) STRNCPY(incmd, "/bin/ls");
	for (i=1, dashopts=0; i<argc; i++) {
		(void) STRNCAT(incmd, " ");
		if (argv[i][0] == '-')
			dashopts++;
		(void) STRNCAT(incmd, argv[i]);
	}

	if (dashopts == 0) {
		(void) STRNCPY(incmd, "/bin/ls -CF");
		for (i=1; i<argc; i++) {
			(void) STRNCAT(incmd, " ");
			(void) STRNCAT(incmd, argv[i]);
		}
	}

	infp = popen(incmd, "r");
	if (infp == NULL) {
		ClosePager(outfp);
		return;
	}


#ifdef HAVE_SIGSETJMP
	sj = sigsetjmp(gCancelJmp, 1);
#else	/* HAVE_SIGSETJMP */
	sj = setjmp(gCancelJmp);
#endif	/* HAVE_SIGSETJMP */

	if (sj != 0) {
		/* Caught a signal. */
		(void) NcSignal(SIGPIPE, (FTPSigProc) SIG_IGN);
		ClosePager(outfp);
		if (infp != NULL)
			(void) pclose(infp);
		(void) NcSignal(SIGPIPE, osigpipe);
		(void) NcSignal(SIGINT, osigint);
		(void) fprintf(stderr, "Canceled.\n");
		Trace(0, "Canceled because of signal %d.\n", gGotSig);
		gMayCancelJmp = 0;
		return;
	} else {
		osigpipe = NcSignal(SIGPIPE, Cancel);
		osigint = NcSignal(SIGINT, Cancel);
		gMayCancelJmp = 1;
	}

	while (fgets(line, sizeof(line) - 1, infp) != NULL)
		(void) fputs(line, outfp);
	(void) fflush(outfp);

	(void) pclose(infp);
	infp = NULL;
	ClosePager(outfp);
	outfp = NULL;

	(void) NcSignal(SIGPIPE, osigpipe);
	(void) NcSignal(SIGINT, osigint);
	gMayCancelJmp = 0;
#endif
}	/* LocalListCmd */




static void
Sys(const int argc, char **const argv, const ArgvInfoPtr aip, const char *syscmd, int noDQuote)
{
	char cmd[256];
	int i;

	(void) STRNCPY(cmd, syscmd);
	for (i = 1; i < argc; i++) {
		if (aip->noglobargv[i] != 0) {
			(void) STRNCAT(cmd, " '");
			(void) STRNCAT(cmd, argv[i]);
			(void) STRNCAT(cmd, "'");
		} else if (noDQuote != 0) {
			(void) STRNCAT(cmd, " ");
			(void) STRNCAT(cmd, argv[i]);
		} else {
			(void) STRNCAT(cmd, " \"");
			(void) STRNCAT(cmd, argv[i]);
			(void) STRNCAT(cmd, "\" ");
		}
	}
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	fprintf(stderr, "Cannot run command: %s\n", cmd);
#else
	Trace(0, "Sys: %s\n", cmd);
	(void) system(cmd);
#endif
}	/* Sys */




#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
void
LocalChmodCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, "/bin/chmod", 1);
}	/* LocalChmodCmd */
#endif



void
LocalMkdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	const char *arg;
	int i;

	for (i = 1; i < argc; i++) {
		arg = argv[i];
		if (MkDirs(arg, 00755) < 0) {
			perror(arg);
		}
	}
#else
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, "/bin/mkdir", 0);
#endif
}	/* LocalMkdirCmd */




#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
void
LocalPageCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, gPager, 0);
}	/* LocalPageCmd */
#endif




void
LocalRenameCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (rename(argv[1], argv[2]) < 0) {
		perror("rename");
	}
#else
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, "/bin/mv", 1);
#endif
}	/* LocalRenameCmd */




void
LocalRmCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	int i;
	int result;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);
	for (i=1; i<argc; i++) {
		InitLineList(&ll);
		result = FTPLocalGlob(&gConn, &ll, argv[i], (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, "local glob", argv[i]);
		} else {
			for (lp = ll.first; lp != NULL; lp = lp->next) {
				if (lp->line != NULL) {
					if (remove(lp->line) < 0)
						perror(lp->line);
				}
			}
		}
		DisposeLineListContents(&ll);
	}
#else
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, "/bin/rm", 1);
#endif
}	/* LocalRmCmd */




void
LocalRmdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	int i;
	int result;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);
	for (i=1; i<argc; i++) {
		InitLineList(&ll);
		result = FTPLocalGlob(&gConn, &ll, argv[i], (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, "local glob", argv[i]);
		} else {
			for (lp = ll.first; lp != NULL; lp = lp->next) {
				if (lp->line != NULL) {
					if (rmdir(lp->line) < 0)
						perror(lp->line);
				}
			}
		}
		DisposeLineListContents(&ll);
	}
#else
	ARGSUSED(gUnusedArg);
	Sys(argc, argv, aip, "/bin/rmdir", 1);
#endif
}	/* LocalRmdirCmd */




/* Displays the current local working directory. */
void
LocalPwdCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	if (FTPGetLocalCWD(gLocalCWD, sizeof(gLocalCWD)) != NULL) {
		Trace(-1, "%s\n", gLocalCWD);
	}
}	/* LocalPwdCmd */




/* This is a simple interface to name service.  I prefer using this instead
 * of nslookup.
 */
void
LookupCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i, j;
	struct hostent hp;
	int hpok;
	const char *host;
	char **cpp;
	struct in_addr ip_address;
	int shortMode, optrc;
	char ipStr[16];
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	shortMode = 0;
	
	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "vqV")) >= 0) {
		if (optrc == 'v') {
			shortMode = 0;
		} else if (optrc == 'V') {
			shortMode = 1;
		} else if (optrc == 'q') {
			shortMode = 1;
		} else {
			PrintCmdUsage(cmdp);
			return;
		}
	}

	for (i=opt.ind; i<argc; i++) {
		hpok = 0;
		if (GetHostEntry(&hp, (host = argv[i]), &ip_address, gConn.buf, gConn.bufSize) == 0)
			hpok = 1;
		if ((i > opt.ind) && (shortMode == 0))
			Trace(-1, "\n");
		if (hpok == 0) {
			Trace(-1, "Unable to get information about site %s.\n", host);
		} else if (shortMode) {
			MyInetAddr(ipStr, sizeof(ipStr), hp.h_addr_list, 0);
			Trace(-1, "%-40s %s\n", hp.h_name, ipStr);
		} else {
			Trace(-1, "%s:\n", host);
			Trace(-1, "    Name:     %s\n", hp.h_name);
			for (cpp = hp.h_aliases; *cpp != NULL; cpp++)
				Trace(-1, "    Alias:    %s\n", *cpp);
			for (j = 0, cpp = hp.h_addr_list; *cpp != NULL; cpp++, ++j) {
				MyInetAddr(ipStr, sizeof(ipStr), hp.h_addr_list, j);
				Trace(-1, "    Address:  %s\n", ipStr);	
			}
		}
	}
}	/* LookupCmd */



/* Directory listing in a machine-readable format;
 * Mostly for debugging, since NcFTP uses MLSD automatically when it needs to.
 */
void
MlsCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i;
	int optrc;
	FTPLinePtr linePtr, nextLinePtr;
	int result;
	FTPLineList dirContents;
	int mlsd = 1, x;
	const char *item;
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "dt")) >= 0) {
		if ((optrc == 'd') || (optrc == 't')) {
			/* Use MLST instead of MLSD,
			 * which is similar to using "ls -d" instead of "ls".
			 */
			mlsd = 0;
		} else {
			PrintCmdUsage(cmdp);
			return;
		}
	}

	i = opt.ind;
	if (i == argc) {
		/* No args, do current directory. */
		x = 1;
		item = "";
		if ((result = FTPListToMemory2(&gConn, item, &dirContents, (mlsd == 0) ? "-d" : "", 1, &x)) < 0) {
			if (mlsd != 0) {
				FTPPerror(&gConn, result, 0, "Could not MLSD", item);
			} else {
				FTPPerror(&gConn, result, 0, "Could not MLST", item);
			}
		} else {
			for (linePtr = dirContents.first;
				linePtr != NULL;
				linePtr = nextLinePtr)
			{
				nextLinePtr = linePtr->next;
				(void) fprintf(stdout, "%s\n", linePtr->line);	
				Trace(0, "%s\n", linePtr->line);	
			}
		}
	}

	for ( ; i<argc; i++) {
		x = 1;
		item = argv[i];
		if ((result = FTPListToMemory2(&gConn, item, &dirContents, (mlsd == 0) ? "-d" : "", 1, &x)) < 0) {
			if (mlsd != 0) {
				FTPPerror(&gConn, result, 0, "Could not MLSD", item);
			} else {
				FTPPerror(&gConn, result, 0, "Could not MLST", item);
			}
		} else {
			for (linePtr = dirContents.first;
				linePtr != NULL;
				linePtr = nextLinePtr)
			{
				nextLinePtr = linePtr->next;
				(void) fprintf(stdout, "%s\n", linePtr->line);	
				Trace(0, "%s\n", linePtr->line);	
			}
		}
	}
}	/* MlsCmd */




/* Create directories on the remote system. */
void
MkdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i;
	int optrc;
	int result;
	int recurseFlag = kRecursiveNo;
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "p")) >= 0) {
		if (optrc == 'p') {
			/* Try creating intermediate directories if they
			 * don't exist.
			 *
			 * For example if only /pub/stuff existed, and you
			 * do a "mkdir -p /pub/stuff/a/b/c", the "a" and "b"
			 * directories would also be created.
			 */
			recurseFlag = kRecursiveYes;
		} else {
			PrintCmdUsage(cmdp);
			return;
		}
	}

	for (i=opt.ind; i<argc; i++) {
		result = FTPMkdir(&gConn, argv[i], recurseFlag);
		if (result < 0)
			FTPPerror(&gConn, result, kErrMKDFailed, "Could not mkdir", argv[i]);
	}

	/* Really should just flush only the modified directories... */
	FlushLsCache();
}	/* MkdirCmd */



/*VARARGS*/
static void
OpenMsg(const char *const fmt, ...)
{
	va_list ap;
	char buf[512];
	size_t len, padlim;

	padlim = (size_t) gScreenColumns;
	if ((size_t) gScreenColumns > (sizeof(buf) - 1))
		padlim = sizeof(buf) - 1;

	va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
	len = (size_t) vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);
	buf[sizeof(buf) - 1] = '\0';
#else
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
	len = strlen(buf);
#endif
	buf[len] = '\0';
	Trace(9, "%s\n", buf);
	for (; len < padlim; len++)
		buf[len] = ' ';
	buf[len] = '\0';

#if 0
	(void) fprintf(stdout, "\r%s", buf);
#else
	(void) fprintf(stdout, "%s\r", buf);
#endif
	(void) fflush(stdout);
}	/* OpenMsg */




static void
NcFTPOpenPrintResponseProc(const FTPCIPtr cipUNUSED, ResponsePtr rp)
{
	gUnusedArg = (cipUNUSED != NULL);
	if ((rp->printMode & kResponseNoPrint) != 0)
		return;
#if 0
	if (rp->code == 331)	/* Skip: "331 User name okay, need password." */
		return;
#else
	/* This is only used to print errors. */
	if (rp->code < 400)
		return;
#endif
	PrintResp(&rp->msg);
}	/* NcFTPOpenPrintResponseProc */



static void
NcFTPOnConnectMessageProc(const FTPCIPtr cipUNUSED, ResponsePtr rp)
{
	gUnusedArg = (cipUNUSED != NULL);
	(void) printf("\n");
	PrintResp(&rp->msg);
	OpenMsg("Logging in...");
}	/* NcFTPOnConnectMessageProc */



static void
NcFTPOnLoginMessageProc(const FTPCIPtr cipUNUSED, ResponsePtr rp)
{
	gUnusedArg = (cipUNUSED != NULL);
	(void) printf("\n");
	PrintResp(&rp->msg);
	OpenMsg("Logging in...");
}	/* NcFTPOnLoginMessageProc */




static void
NcFTPRedialStatusProc(const FTPCIPtr cipUNUSED, int mode, int val)
{
	gUnusedArg = (cipUNUSED != NULL);
	if (mode == kRedialStatusDialing) {
		if (val > 0) {
			OpenMsg("Redialing (try %d)...", val);
			sleep(1);	/* Give time for message to stay */
		}
	} else if (mode == kRedialStatusSleeping) {
		OpenMsg("Sleeping %d seconds...", val);
	}
}	/* NcFTPRedialStatusProc */




static void
NcFTPGetPassphraseProc(const FTPCIPtr cip, FTPLineListPtr pwPrompt, char *pass, size_t dsize)
{
	FTPLinePtr lp;

	(void) printf("\nPassword requested by %s for user \"%s\".\n\n",
		cip->host,
		cip->user
		);

	for (lp = pwPrompt->first; lp != NULL; lp = lp->next) {
		(void) printf("    %s\n", lp->line);
	}
	(void) printf("\n");
	(void) gl_getpass("Password: ", pass, (int) dsize);
}	/* NcFTPGetPassphraseProc */




/* Attempts to establish a new FTP connection to a remote host. */
int
DoOpen(void)
{
	int result;
	char ipstr[128];
	char ohost[128];
#ifdef SIGALRM
	sigproc_t osigalrm;
#endif
	char prompt[256];
	char *cp;

	if (gConn.firewallType == kFirewallNotInUse) {
		(void) STRNCPY(ohost, gConn.host);
		OpenMsg("Resolving %s...", ohost);
		if ((gLoadedBm != 0) && (gBm.lastIP[0] != '\0')) {
			result = MyGetHostByName(ipstr, sizeof(ipstr), ohost, 3);
			if (result < 0) {
				(void) STRNCPY(ipstr, gBm.lastIP);
				result = 0;
			} else {
				result = MyGetHostByName(ipstr, sizeof(ipstr), ohost, 7);
			}
		} else {
			result = MyGetHostByName(ipstr, sizeof(ipstr), ohost, 10);
		}
		if (result < 0) {
			(void) printf("\n");
			(void) printf("Unknown host \"%s\".\n", ohost);
			return (-1);
		}
		(void) STRNCPY(gConn.host, ipstr);
		OpenMsg("Connecting to %s...", ipstr);
	} else {
		OpenMsg("Connecting to %s via %s...", gConn.host, gConn.firewallHost);
		Trace(0, "Fw: %s  Type: %d  User: %s  Pass: %s  Port: %u\n", 
			gConn.firewallHost,
			gConn.firewallType,
			gConn.firewallUser,
			(gConn.firewallPass[0] == '\0') ? "(none)" : "********",
			gConn.firewallPort
		);
		Trace(0, "FwExceptions: %s\n", gFirewallExceptionList);
		if (strchr(gOurHostName, '.') == NULL) {
			Trace(0, "NOTE:  Your domain name could not be detected.\n");
			if (gConn.firewallType != kFirewallNotInUse) {
				Trace(0, "       Make sure you manually add your domain name to firewall-exception-list.\n");
			}
		}
	}

	if (gConn.firewallPass[0] == '\0') {
		switch (gConn.firewallType) {
			case kFirewallNotInUse:
				break;
			case kFirewallUserAtSite:
			case kFirewallUserAtSitePort:
			case kFirewallUserAtSitePort2:
				break;
			case kFirewallLoginThenUserAtSite:
			case kFirewallSiteSite:
			case kFirewallOpenSite:
			case kFirewallUserAtUserPassAtPass:
			case kFirewallFwuAtSiteFwpUserPass:
			case kFirewallUserAtSiteFwuPassFwp:
				(void) printf("\n");
				(void) STRNCPY(prompt, "Password for firewall user \"");
				(void) STRNCAT(prompt, gConn.firewallUser);
				(void) STRNCAT(prompt, "\" at ");
				(void) STRNCAT(prompt, gConn.firewallHost);
				(void) STRNCAT(prompt, ": ");
				(void) gl_getpass(prompt, gConn.firewallPass, sizeof(gConn.firewallPass));
				break;
		}
	}

	if ((gConn.user[0] != '\0') && (strcmp(gConn.user, "anonymous") != 0) && (strcmp(gConn.user, "ftp") != 0)) {
		gConn.passphraseProc = NcFTPGetPassphraseProc;
	}

	/* Register our callbacks. */
	gConn.printResponseProc = NcFTPOpenPrintResponseProc;
	gConn.onConnectMsgProc = NcFTPOnConnectMessageProc;
	gConn.onLoginMsgProc = NcFTPOnLoginMessageProc;
	gConn.redialStatusProc = NcFTPRedialStatusProc; 

#ifdef SIGALRM
	osigalrm = NcSignal(SIGALRM, (FTPSigProc) SIG_IGN);
	result = FTPOpenHost(&gConn);
	(void) NcSignal(SIGALRM, osigalrm);
#else
	result = FTPOpenHost(&gConn);
#endif
	
	if (gConn.firewallType == kFirewallNotInUse)
		(void) STRNCPY(gConn.host, ohost);		/* Put it back. */
	if (result >= 0) {
		(void) time(&gBm.lastCall);
		LogOpen(gConn.host);
		OpenMsg("Logged in to %s.", gConn.host);
		(void) printf("\n");

		/* Remove callback. */
		gConn.printResponseProc = 0;

		/* Need to note what our "root" was before we change it. */
		gServerUsesMSDOSPaths = 0;
		if (gConn.startingWorkingDirectory == NULL) {
			(void) STRNCPY(gRemoteCWD, "/");	/* Guess! */
		} else {
			cp = gConn.startingWorkingDirectory;
			(void) STRNCPY(gRemoteCWD, cp);
			if ((strlen(cp) >= 2) && (isalpha((int) cp[0])) && (cp[1] == ':'))
				gServerUsesMSDOSPaths = 1;
		}
		(void) STRNCPY(gStartDir, gRemoteCWD);

		/* If the bookmark specified a remote directory, change to it now. */
		if ((gLoadedBm != 0) && (gBm.dir[0] != '\0')) {
			result = Chdirs(&gConn, gBm.dir);
			if (result < 0) {
				FTPPerror(&gConn, result, kErrCWDFailed, "Could not chdir to previous directory", gBm.dir);
			}
			Trace(-1, "Current remote directory is %s.\n", gRemoteCWD);
		}

		/* If the bookmark specified a local directory, change to it now. */
		if ((gLoadedBm != 0) && (gBm.ldir[0] != '\0')) {
			(void) chdir(gBm.ldir);
			(void) STRNCPY(gPrevLocalCWD, gLocalCWD);
			if (FTPGetLocalCWD(gLocalCWD, sizeof(gLocalCWD)) != NULL) {
				Trace(-1, "Current local directory is %s.\n", gLocalCWD);
			}
		}

		/* Identify the FTP client type to the server.  Most don't understand this yet. */
		if (gConn.hasCLNT != kCommandNotAvailable)
			(void) FTPCmd(&gConn, "CLNT NcFTP %.5s %s", gVersion + 11, gOS);
		return (0);
	} else {
		FTPPerror(&gConn, result, 0, "Could not open host", gConn.host);
	}

	/* Remove callback. */
	gConn.printResponseProc = 0;
	(void) printf("\n");
	return (-1);
}	/* Open */




/* Chooses a new remote system to connect to, and attempts to establish
 * a new FTP connection.  This function is in charge of collecting the
 * information needed to do the open, and then doing it.
 */
void
OpenCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int c;
	int opts = 0;
	int uOptInd = 0;
	int n;
	int rc;
	char url[256];
	char urlfile[128];
	int directoryURL = 0;
	FTPLineList cdlist;
	FTPLinePtr lp;
	char prompt[256];
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	FlushLsCache();
	CloseHost();
	gLoadedBm = 0;
	InitConnectionInfo();

	/* Need to find the host argument first. */
	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "aP:u:p:j:rd:g:o:")) > 0) switch(c) {
		case 'u':
			uOptInd = opt.ind + 1;
			opts++;
			break;
		default:
			opts++;
	}
	if (opt.ind < argc) {
		(void) STRNCPY(gConn.host, argv[opt.ind]);
		(void) STRNCPY(url, argv[opt.ind]);
	} else if (uOptInd > argc) {
		/* Special hack for v2.4.2 compatibility */
		(void) STRNCPY(gConn.host, argv[argc - 1]);
		(void) STRNCPY(url, argv[argc - 1]);
	} else {
		/* No host arg */
		if (opts > 0) {
			PrintCmdUsage(cmdp);
		} else if (RunBookmarkEditor(gConn.host, sizeof(gConn.host)) == 0) {
			if (gConn.host[0] != '\0') {
				gLoadedBm = 1;
				/* okay, now fall through */
			} else {
				return;
			}
		} else if (PrintHosts() <= 0) {
			(void) printf("You haven't bookmarked any FTP sites.\n");
			(void) printf("Before closing a site, you can use the \"bookmark\" command to save the current\nhost and directory for next time.\n");
			return;
		} else {
			(void) printf("\nTo use a bookmark, use the \"open\" command with the name of the bookmark.\n");
			return;
		}
	}

	InitLineList(&cdlist);

	if (GetBookmark(gConn.host, &gBm) >= 0) {
		gLoadedBm = 1;
		(void) STRNCPY(gConn.host, gBm.name);
		(void) STRNCPY(gConn.user, gBm.user);
		(void) STRNCPY(gConn.pass, gBm.pass);
		(void) STRNCPY(gConn.acct, gBm.acct);
		/*
		 * We used to save these, but if the server software
		 * has been upgraded then our has* data could be stale.
		 *
		gConn.hasSIZE = gBm.hasSIZE;
		gConn.hasMDTM = gBm.hasMDTM;
		gConn.hasSITE_UTIME = gBm.hasUTIME;
		 *
		 */
		gConn.port = gBm.port;

		/* Note:  Version 3 only goes off of the
		 * global "gDataPortMode" setting instead of
		 * setting the dataPortMode on a per-site
		 * basis.
		 */
		gConn.hasPASV = gBm.hasPASV;
	} else {
		SetBookmarkDefaults(&gBm);

		memcpy(&gTmpURLConn, &gConn, sizeof(gTmpURLConn));
		rc = DecodeDirectoryURL(&gTmpURLConn, url, &cdlist, urlfile, sizeof(urlfile));
		if (gTmpURLConn.pass[0] != '\0') {
			gUserTypedSensitiveInfoAtShellSoDoNotSaveItToDisk++;
		}
		if (rc == kMalformedURL) {
			(void) fprintf(stdout, "Malformed URL: %s\n", url);
			DisposeLineListContents(&cdlist);
			memset(&gTmpURLConn, 0, sizeof(gTmpURLConn));
			return;
		} else if (rc == kNotURL) {
			directoryURL = 0;
		} else {
			/* It was a URL. */
			if (urlfile[0] != '\0') {
				/* It was obviously not a directory */
				(void) fprintf(stdout, "Use ncftpget or ncftpput to handle file URLs.\n");
				DisposeLineListContents(&cdlist);
				return;
			}
			memcpy(&gConn, &gTmpURLConn, sizeof(gConn));
			directoryURL = 1;
		}
		memset(&gTmpURLConn, 0, sizeof(gTmpURLConn));
	}

	if (MayUseFirewall(gConn.host, gFirewallType, gFirewallExceptionList) != 0) {
		gConn.firewallType = gFirewallType; 
		(void) STRNCPY(gConn.firewallHost, gFirewallHost);
		(void) STRNCPY(gConn.firewallUser, gFirewallUser);
		(void) STRNCPY(gConn.firewallPass, gFirewallPass);
		gConn.firewallPort = gFirewallPort;
	}

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "aP:u:p:j:rd:g:o:")) > 0) switch(c) {
		case 'j':
			(void) STRNCPY(gConn.acct, opt.arg);
			break;
		case 'a':
			(void) STRNCPY(gConn.user, "anonymous");
			(void) STRNCPY(gConn.pass, "");
			gConn.passIsEmpty = 0;
			(void) STRNCPY(gConn.acct, "");
			break;
		case 'P':
			gConn.port = atoi(opt.arg);	
			break;
		case 'u':
			if (uOptInd <= argc)
				(void) STRNCPY(gConn.user, opt.arg);
			break;
		case 'p':
			(void) STRNCPY(gConn.pass, opt.arg);	/* Don't recommend doing this! */
			if (gConn.pass[0] == '\0')
				gConn.passIsEmpty = 1;
			memset(opt.arg, '*', strlen(opt.arg));
			/* Since they passed sensitive information
			 * on the command line (their fault!),
			 * make an effort not to compound the
			 * problem by not saving this information
			 * to the trace log and command history.
			 */
			gUserTypedSensitiveInfoAtShellSoDoNotSaveItToDisk++;
			break;
		case 'r':
			/* redial is on by default */
			break;
		case 'g':
			n = atoi(opt.arg);
			gConn.maxDials = n;
			break;
		case 'd':
			n = atoi(opt.arg);
			if (n >= 10)
				gConn.redialDelay = n;
			break;
		case 'o':
			gConn.manualOverrideFeatures = opt.arg;
			break;
		default:
			PrintCmdUsage(cmdp);
			DisposeLineListContents(&cdlist);
			return;
	}

	if (uOptInd > argc) {
		(void) STRNCPY(prompt, "Username at ");
		(void) STRNCAT(prompt, gConn.host);
		(void) STRNCAT(prompt, ": ");
		(void) gl_getpass(prompt, gConn.user, sizeof(gConn.user));
	}

	rc = DoOpen();
	if ((rc >= 0) && (directoryURL != 0)) {
		for (lp = cdlist.first; lp != NULL; lp = lp->next) {
			rc = FTPChdir(&gConn, lp->line);
			if (rc != kNoErr) {
				FTPPerror(&gConn, rc, kErrCWDFailed, "Could not chdir to", lp->line);
				break;
			}
		}
		rc = FTPGetCWD(&gConn, gRemoteCWD, sizeof(gRemoteCWD));
		if (rc != kNoErr) {
			FTPPerror(&gConn, rc, kErrPWDFailed, NULL, NULL);
		} else {
			(void) printf("Current remote directory is %s.\n", gRemoteCWD);
		}
	}
	DisposeLineListContents(&cdlist);
}	/* OpenCmd */




/* View a remote file through the users $PAGER. */
void
PageCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	int i;
	FILE *volatile stream;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	int sj;
	vsigproc_t osigpipe, osigint;
#endif

	ARGSUSED(gUnusedArg);
	stream = OpenPager();
	if (stream == NULL) {
		return;
	}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else

#ifdef HAVE_SIGSETJMP
	osigpipe = osigint = (sigproc_t) 0;
	sj = sigsetjmp(gCancelJmp, 1);
#else	/* HAVE_SIGSETJMP */
	osigpipe = osigint = (sigproc_t) 0;
	sj = setjmp(gCancelJmp);
#endif	/* HAVE_SIGSETJMP */

	if (sj != 0) {
		/* Caught a signal. */
		(void) NcSignal(SIGPIPE, (FTPSigProc) SIG_IGN);
		ClosePager(stream);
		(void) NcSignal(SIGPIPE, osigpipe);
		(void) NcSignal(SIGINT, osigint);
		(void) fprintf(stderr, "Canceled.\n");
		Trace(0, "Canceled because of signal %d.\n", gGotSig);
		gMayCancelJmp = 0;
		return;
	} else {
		osigpipe = NcSignal(SIGPIPE, Cancel);
		osigint = NcSignal(SIGINT, Cancel);
		gMayCancelJmp = 1;
	}

#endif

	for (i=1; i<argc; i++) {
		result = FTPGetOneFile2(&gConn, argv[i], NULL, kTypeAscii, fileno(stream), kResumeNo, kAppendNo);
		if (result < 0) {
			if (errno != EPIPE) {
				ClosePager(stream);
				stream = NULL;
				FTPPerror(&gConn, result, kErrCouldNotStartDataTransfer, argv[0], argv[i]);
			}
			break;
		}
	}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	ClosePager(stream);
#else
	(void) NcSignal(SIGPIPE, (FTPSigProc) SIG_IGN);
	ClosePager(stream);
	(void) NcSignal(SIGPIPE, osigpipe);
	(void) NcSignal(SIGINT, osigint);
#endif
	gMayCancelJmp = 0;
}	/* PageCmd */




/* Toggles passive mode.  For compatibility only; they should be doing
 * "set passive on" instead.
 */
void
PassiveCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	Set("passive", (gConn.dataPortMode == kPassiveMode) ? "off" : "on");
	Set("passive", NULL);	/* Show the new setting */
}	/* PassiveCmd */




static int
NcFTPConfirmResumeUploadProc(
	const FTPCIPtr cipUNUSED,
	const char *localpath,
	longest_int localsize,
	time_t localmtime,
	const char **remotepath,
	longest_int remotesize,
	time_t remotemtime,
	longest_int *startPoint)
{
	int zaction = kConfirmResumeProcSaidBestGuess;
	char tstr[80], ans[32];
	static char newname[128];	/* arrggh... static. */
	struct tm t;

	gUnusedArg = (cipUNUSED != NULL);
	if (gResumeAnswerAll != kConfirmResumeProcNotUsed) {
		if (gResumeAnswerAll == kConfirmResumeProcSaidResume) {
			if (gConn.hasREST != kCommandAvailable) {
				/* Resume is not available on this server. */
				return (zaction);
			} else if (remotesize > localsize) {
				/* Cannot resume when remote file is already larger than the local file. */
				return (zaction);
			} else if (remotesize == localsize) {
				/* Remote file is already the same size as the local file. */
				return (zaction);
			}
			*startPoint = remotesize;
		}
		return (gResumeAnswerAll);
	}

	if ((gAutoResume != 0) || (gIsTTYr == 0))
		return (kConfirmResumeProcSaidBestGuess);

	printf("\nThe remote file \"%s\" already exists.\n", *remotepath);

	if ((localmtime != kModTimeUnknown) && (localsize != kSizeUnknown)) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(localmtime, &t));
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tLocal:  %12lld bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tLocal:  %12qd bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tLocal:  %12I64d bytes, dated %s.\n",
#else
			"\tLocal:  %12ld bytes, dated %s.\n",
#endif
			localsize,
			tstr
		);
		if ((remotemtime == localmtime) && (remotesize == localsize)) {
			(void) printf("\t(Files are identical, skipped)\n\n");
			return (kConfirmResumeProcSaidSkip);
		}
	} else if (localsize != kSizeUnknown) {
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tLocal:  %12lld bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tLocal:  %12qd bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tLocal:  %12I64d bytes, date unknown.\n",
#else
			"\tLocal:  %12ld bytes, date unknown.\n",
#endif
			localsize
		);
	} else if (localmtime != kModTimeUnknown) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(localmtime, &t));
		(void) printf(
			"\tLocal:  size unknown, dated %s.\n",
			tstr
		);
	}

	tstr[sizeof(tstr) - 1] = '\0';
	if ((remotemtime != kModTimeUnknown) && (remotesize != kSizeUnknown)) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(remotemtime, &t));
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tRemote: %12lld bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tRemote: %12qd bytes, dated %s.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tRemote: %12I64d bytes, dated %s.\n",
#else
			"\tRemote: %12ld bytes, dated %s.\n",
#endif
			remotesize,
			tstr
		);
	} else if (remotesize != kSizeUnknown) {
		(void) fprintf(stdout,
#if defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_LLD)
			"\tRemote: %12lld bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_QD)
			"\tRemote: %12qd bytes, date unknown.\n",
#elif defined(HAVE_LONG_LONG) && defined(PRINTF_LONG_LONG_I64D)
			"\tRemote: %12I64d bytes, date unknown.\n",
#else
			"\tRemote: %12ld bytes, date unknown.\n",
#endif
			remotesize
		);
	} else if (remotemtime != kModTimeUnknown) {
		(void) strftime(tstr, sizeof(tstr) - 1, DATE_SPEC, Localtime(remotemtime, &t));
		(void) printf(
			"\tRemote: size unknown, dated %s.\n",
			tstr
		);
	}

	printf("\n");
	fflush(stdin);
	(void) memset(ans, 0, sizeof(ans));
	for (;;) {
		(void) printf("\t[O]verwrite?");
		/* if ((gConn.hasREST == kCommandAvailable) && (remotesize < localsize)) */
		if (remotesize < localsize)
			printf("  [R]esume?");
		printf("  [A]ppend to?  [S]kip?  [N]ew Name?\n");
		(void) printf("\t[O!]verwrite all?");
		/* if ((gConn.hasREST == kCommandAvailable) && (remotesize < localsize)) */
		if (remotesize < localsize)
			printf("  [R!]esume all?");
		printf("  [S!]kip all?  [C]ancel  > ");
		(void) fgets(ans, sizeof(ans) - 1, stdin);
		switch ((int) ans[0]) {
			case 'c':
			case 'C':
				ans[0] = 'C';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidCancel;
				break;
			case 'o':
			case 'O':
				ans[0] = 'O';
				zaction = kConfirmResumeProcSaidOverwrite;
				break;
			case 'r':
			case 'R':
				/* (We will try APPE if needed) if (gConn.hasREST != kCommandAvailable) {
					printf("\tResume is not available on this server.\n\n");
					ans[0] = '\0';
					break;
				} else */ if (remotesize > localsize) {
					printf("\tCannot resume when remote file is already larger than the local file.\n\n");
					ans[0] = '\0';
					break;
				} else if (remotesize == localsize) {
					printf("\tRemote file is already the same size as the local file.\n\n");
					ans[0] = '\0';
					break;
				}
				ans[0] = 'R';
				*startPoint = remotesize;
				zaction = kConfirmResumeProcSaidResume;
				if (OneTimeMessage("auto-resume") != 0) {
					printf("\n\tNOTE: If you want NcFTP to guess automatically, \"set auto-resume yes\"\n\n");
				}
				break;
			case 's':
			case 'S':
				ans[0] = 'S';
				zaction = kConfirmResumeProcSaidSkip;
				break;
			case 'n':
			case 'N':
				ans[0] = 'N';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidOverwrite;
				break;
			case 'a':
			case 'A':
				ans[0] = 'A';
				ans[1] = '\0';
				zaction = kConfirmResumeProcSaidAppend;
				break;
			case 'g':
			case 'G':
				ans[0] = 'G';
				zaction = kConfirmResumeProcSaidBestGuess;
				break;
			default:
				ans[0] = '\0';
		}
		if (ans[0] != '\0')
			break;
	}

	if (ans[0] == 'N') {
		(void) memset(newname, 0, sizeof(newname));
		printf("\tSave as:  ");
		fflush(stdin);
		(void) fgets(newname, sizeof(newname) - 1, stdin);
		newname[strlen(newname) - 1] = '\0';
		if (newname[0] == '\0') {
			/* Nevermind. */
			printf("Skipped %s.\n", localpath);
			zaction = kConfirmResumeProcSaidSkip;
		} else {
			*remotepath = newname;
		}
	}

	if (ans[1] == '!')
		gResumeAnswerAll = zaction;
	return (zaction);
}	/* NcFTPConfirmResumeUploadProc */




/* Upload files to the remote system, permissions permitting. */
void
PutCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int optrc;
	int renameMode = 0;
	int recurseFlag = kRecursiveNo;
	int appendFlag = kAppendNo;
	const char *dstdir = NULL;
	int rc;
	int i;
	int doGlob;
	int xtype = gBm.xferType;
	int nD = 0;
	int deleteFlag = kDeleteNo;
	int resumeFlag = kResumeYes;
	char pattern[256];
	vsigproc_t osigint;
	FTPConfirmResumeUploadProc confirmProc;
	GetoptInfo opt;

	confirmProc = NcFTPConfirmResumeUploadProc;
	gResumeAnswerAll = kConfirmResumeProcNotUsed;	/* Ask at least once each time */
	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "AafZzrRD")) >= 0) switch (optrc) {
		case 'a':
			xtype = kTypeAscii;
			break;
		case 'A':
			/* Append to remote files, instead of truncating
			 * them first.
			 */
			appendFlag = kAppendYes;
			break;
		case 'f':
		case 'Z':
			/* Do not try to resume a download, even if it
			 * appeared that some of the file was transferred
			 * already.
			 */
			resumeFlag = kResumeNo;
			confirmProc = kNoFTPConfirmResumeUploadProc;
			break;
		case 'z':
			/* Special flag that lets you specify the
			 * destination file.  Normally a "put" will
			 * write the file by the same name as the
			 * local file's basename.
			 */
			renameMode = 1;
			break;
		case 'r':
		case 'R':
			recurseFlag = kRecursiveYes;
			/* If the item is a directory, get the
			 * directory and all its contents.
			 */
			recurseFlag = kRecursiveYes;
			break;
		case 'D':
			/* You can delete the local file after
			 * you uploaded it successfully by using
			 * the -DD option.  It requires two -D's
			 * to minimize the odds of accidentally
			 * using a single -D.
			 */
			nD++;
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}

	if (nD >= 2)
		deleteFlag = kDeleteYes;

	if (renameMode != 0) {
		if (opt.ind > (argc - 2)) {
			PrintCmdUsage(cmdp);
			(void) fprintf(stderr, "\nFor put with rename, try \"put -z local-path-name remote-path-name\".\n");
			return;
		}
		osigint = NcSignal(SIGINT, XferCanceller);
		rc = FTPPutOneFile3(&gConn, argv[opt.ind], argv[opt.ind + 1], xtype, (-1), appendFlag, NULL, NULL, resumeFlag, deleteFlag, confirmProc, 0);
		if (rc < 0)
			FTPPerror(&gConn, rc, kErrCouldNotStartDataTransfer, "put", argv[opt.ind + 1]);
	} else {
		osigint = NcSignal(SIGINT, XferCanceller);
		for (i=opt.ind; i<argc; i++) {
			doGlob = (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes;
			STRNCPY(pattern, argv[i]);
			StrRemoveTrailingSlashes(pattern);
			rc = FTPPutFiles3(&gConn, pattern, dstdir, recurseFlag, doGlob, xtype, appendFlag, NULL, NULL, resumeFlag, deleteFlag, confirmProc, 0);
			if (rc < 0)
				FTPPerror(&gConn, rc, kErrCouldNotStartDataTransfer, "put", argv[i]);
		}
	}

	/* Really should just flush only the modified directories... */
	FlushLsCache();

	(void) NcSignal(SIGINT, osigint);
	(void) fflush(stdin);
}	/* PutCmd */




/* Displays the current remote working directory path. */
void
PwdCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	char url[256];
	char olddir[256];

	ARGSUSED(gUnusedArg);
#ifdef USE_WHAT_SERVER_SAYS_IS_CWD
	result = FTPGetCWD(&gConn, gRemoteCWD, sizeof(gRemoteCWD));
	CurrentURL(url, sizeof(url), 0);
	if (result < 0) {
		FTPPerror(&gConn, result, kErrPWDFailed, "Could not get remote working directory", NULL);
	} else {
		Trace(-1, "%s\n", url);
	}
#else /* USE_CLIENT_SIDE_CALCULATED_CWD */

	/* Display the current working directory, as
	 * maintained by us.
	 */
	CurrentURL(url, sizeof(url), 0);
	Trace(-1, "  %s\n", url);
	olddir[sizeof(olddir) - 2] = '\0';
	STRNCPY(olddir, gRemoteCWD);

	/* Now see what the server reports as the CWD.
	 * Because of symlinks, it could be different
	 * from what we are using.
	 */
	result = FTPGetCWD(&gConn, gRemoteCWD, sizeof(gRemoteCWD));
	if ((result == kNoErr) && (strcmp(gRemoteCWD, olddir) != 0)) {
		Trace(-1, "This URL is also valid on this server:\n");
		CurrentURL(url, sizeof(url), 0);
		Trace(-1, "  %s\n", url);
		if (olddir[sizeof(olddir) - 2] == '\0') {
			/* Go back to using the non-resolved version. */
			STRNCPY(gRemoteCWD, olddir);
		}
	}
#endif
}	/* PwdCmd */




/* Sets a flag so that the command shell exits. */
void
QuitCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	gDoneApplication = 1;
}	/* QuitCmd */




/* Send a command string to the FTP server, verbatim. */
void
QuoteCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	char cmdbuf[256];
	int i;

	ARGSUSED(gUnusedArg);
	(void) STRNCPY(cmdbuf, argv[1]);
	for (i=2; i<argc; i++) {
		(void) STRNCAT(cmdbuf, " ");
		(void) STRNCAT(cmdbuf, argv[i]);
	}
	(void) FTPCmd(&gConn, "%s", cmdbuf);
	PrintResp(&gConn.lastFTPCmdResultLL);
}	/* QuoteCmd */




/* Expands a remote regex.  Mostly a debugging command. */
void
RGlobCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i;
	int result;
	int np = 0;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);
	for (i=1; i<argc; i++) {
		InitLineList(&ll);
		result = FTPRemoteGlob(&gConn, &ll, argv[i], (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrGlobFailed, "remote glob", argv[i]);
		} else {
			for (lp = ll.first; lp != NULL; lp = lp->next) {
				if (lp->line != NULL) {
					if (np > 0)
						(void) printf(" ");
					(void) printf("%s", lp->line);
					np++;	
				}
			}
		}
		DisposeLineListContents(&ll);
	}
	(void) printf("\n");
}	/* RGlobCmd */




/* Renames a remote file. */
void
RenameCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;

	ARGSUSED(gUnusedArg);
	result = FTPRename(&gConn, argv[1], argv[2]);
	if (result < 0)
		FTPPerror(&gConn, result, kErrRenameFailed, "rename", argv[1]);
	else {
		/* Really should just flush only the modified directories... */
		FlushLsCache();
	}
}	/* RenameCmd */




/* Removes a directory on the remote host. */
void
RmdirCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;
	int i, c;
	int recursive = kRecursiveNo;
	GetoptInfo opt;

	ARGSUSED(gUnusedArg);
	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "rf")) > 0) switch(c) {
		case 'r':
			recursive = kRecursiveYes;
			break;
		case 'f':
			/* ignore */
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}
	for (i=opt.ind; i<argc; i++) {
		result = FTPRmdir(&gConn, argv[i], recursive, (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (result < 0)
			FTPPerror(&gConn, result, kErrRMDFailed, "rmdir", argv[i]);
	}

	/* Really should just flush only the modified directories... */
	FlushLsCache();
}	/* RmdirCmd */




/* Asks the remote server for help on how to use its server. */
void
RmtHelpCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i, result;
	FTPLineList ll;
	FTPLinePtr lp;

	ARGSUSED(gUnusedArg);
	if (argc == 1) {
		result = FTPRemoteHelp(&gConn, NULL, &ll);
		if (result < 0)
			FTPPerror(&gConn, result, kErrHELPFailed, "HELP failed", NULL);
		else {
			for (lp = ll.first; lp != NULL; lp = lp->next)
				(void) printf("%s\n", lp->line);
		}
		DisposeLineListContents(&ll);
	} else {
		for (i=1; i<argc; i++) {
			result = FTPRemoteHelp(&gConn, argv[i], &ll);
			if (result < 0)
				FTPPerror(&gConn, result, kErrHELPFailed, "HELP failed for", argv[i]);
			else {
				for (lp = ll.first; lp != NULL; lp = lp->next)
					(void) printf("%s\n", lp->line);
			}
			DisposeLineListContents(&ll);
		}
	}
}	/* RmtHelpCmd */




/* Show and/or change customizable program settings. These changes are saved
 * at the end of the program's run.
 */
void
SetCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	if (argc < 2)
		Set(NULL, NULL);
	else if (argc == 2)
		Set(argv[1], NULL);
	else
		Set(argv[1], argv[2]);
}	/* SetCmd */




/* Local shell escape. */
void
ShellCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	const char *cp;
	pid_t pid;
	int status;
	vsigproc_t osigint;
	char **margv;
	int margc;

	osigint = NcSignal(SIGINT, (FTPSigProc) SIG_IGN);
	ARGSUSED(gUnusedArg);
	pid = vfork();
	if (pid < (pid_t) 0) {
		perror("fork");
	} else if (pid == 0) {
		cp = strrchr(gShell, '/');
		if (cp == NULL)
			cp = gShell;	/* bug */
		else
			cp++;
		if (argc == 1) {
			execl(gShell, cp, NULL);
			perror(gShell);
			exit(1);
		} else {
			margv = calloc((size_t) (argc + 1), sizeof(char *));
			if (margv == (char **) 0) {
				perror("malloc");
				exit(1);
			}
			for (margc = 1; ; margc++) {
				if (argv[margc] == NULL)
					break;
				margv[margc - 1] = strdup(argv[margc]);
			}
			execvp(margv[0], margv);
			perror(gShell);
			exit(1);
		}
	} else {
		/* parent */
		for (;;) {
#ifdef HAVE_WAITPID
			if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR))
				break;
#else
			if ((wait(&status) < 0) && (errno != EINTR))
				break;
#endif
			if (WIFEXITED(status) || WIFSIGNALED(status))
				break;		/* done */
		}
	}
	(void) NcSignal(SIGINT, osigint);
#endif
}	/* ShellCmd */




/* Send a command string to the FTP server, verbatim. */
void
SiteCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	char cmdbuf[256];
	int i;

	ARGSUSED(gUnusedArg);
	(void) STRNCPY(cmdbuf, "SITE");
	for (i=1; i<argc; i++) {
		(void) STRNCAT(cmdbuf, " ");
		(void) STRNCAT(cmdbuf, argv[i]);
	}
	(void) FTPCmd(&gConn, "%s", cmdbuf);
	PrintResp(&gConn.lastFTPCmdResultLL);
}	/* SiteCmd */




static time_t
GetStartSpoolDate(const char *s)
{
	char *cp;
	char s2[64];
	time_t now, when;
	int toff, n, c, hr, min;
	struct tm lt, *ltp;

	STRNCPY(s2, s);
	cp = strchr(s2, ':');
	if ((s2[0] == 'n') || (s2[0] == '+')) {
		/* "now + XX hours" or 
		 * "+ XX hours"
		 */
		cp = strchr(s2, '+');
		if (cp == NULL)
			return ((time_t) -1);
		++cp;
		toff = 0;
		n = 0;
		(void) sscanf(cp, "%d%n", &toff, &n);
		if ((n <= 0) || (toff <= 0))
			return ((time_t) -1);
		cp += n;
		while ((*cp != '\0') && (!isalpha((int) *cp)))
			cp++;
		c = *cp;
		if (isupper(c))
			c = tolower(c);
		if (c == 's') {
			/* seconds */
		} else if (c == 'm') {
			/* minutes */
			toff *= 60;
		} else if (c == 'h') {
			/* hours */
			toff *= 3600;
		} else if (c == 'd') {
			/* days */
			toff *= 86400;
		} else {
			/* unrecognized unit */
			return ((time_t) -1);
		}
		time(&now);
		when = now + (time_t) toff;
	} else if (cp != NULL) {
		/* HH:MM, as in "23:38" */
		ltp = Localtime(time(&now), &lt);
		if (ltp == NULL)
			return ((time_t) -1);	/* impossible */
		*cp = ' ';
		hr = -1;
		min = -1;
		(void) sscanf(s2, "%d%d", &hr, &min);
		if ((hr < 0) || (min < 0))
			return ((time_t) -1);
		lt.tm_hour = hr;
		lt.tm_min = min;
		when = mktime(&lt);
		if ((when == (time_t) -1) || (when == (time_t) 0))
			return (when);
		if (when < now)
			when += (time_t) 86400;
	} else {
		when = UnDate(s2);
	}
	return (when);
}	/* GetStartSpoolDate */



static int
SpoolCheck(void)
{
	if (CanSpool() < 0) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		(void) printf("Sorry, spooling isn't allowed until you run Setup.exe.\n");
#else
		(void) printf("Sorry, spooling isn't allowed because this user requires that the NCFTPDIR\nenvironment variable be set to a directory to write datafiles to.\n");
#endif
		return (-1);
	} else if (HaveSpool() == 0) {
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		(void) printf("Sorry, the \"ncftpbatch\" program could not be found.\nPlease re-run Setup to correct this problem.\n");
#else
		(void) printf("Sorry, the \"ncftpbatch\" program could not be found.\nThis program must be installed and in your PATH in order to use this feature.\n");
#endif
		return (-1);
	}
	return (0);
}	/* SpoolCheck */



/* Show and/or change customizable program settings. These changes are saved
 * at the end of the program's run.
 */
void
BGStartCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int i, n;

	ARGSUSED(gUnusedArg);
	if (SpoolCheck() < 0)
		return;

	if ((argc < 2) || ((n = atoi(argv[1])) < 2)) {
		RunBatch();
		(void) printf("Background process started.\n");
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		(void) printf("Watch the \"%s/spool/log\" file to see how it is progressing.\n", gOurDirectoryPath);
#endif
	} else {
		for (i=0; i<n; i++)
			RunBatch();
		(void) printf("Background processes started.\n");
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
		(void) printf("Watch the \"%s/spool/log\" file to see how it is progressing.\n", gOurDirectoryPath);
#endif
	}
}	/* BGStart */



static int
BggetFtwProc(const FtwInfoPtr ftwip)
{
	const char *baserelpath;
	char *curPath;
	char *rdir, *ldir;
	char *rdir1, *ldir1, *cp;
	size_t len;
	char *lpath;
	char dirsep[2];
	int rc;
	int toplevel;
	SpoolCmdInfo *cinfop;

	if (ftwip->depth >= 50) {
		Trace(-1, "Aborting -- recursion depth is %u.\nPerhaps an infinite loop exists on the remote filesystem?", (unsigned int) ftwip->depth);
		return (-1);
	}

	cinfop = (SpoolCmdInfo *) ftwip->userdata;
	/* relpath = ftwip->curPath + ftwip->startPathLen + 1; */
	baserelpath = ftwip->curPath + cinfop->base;
	curPath = ftwip->curPath;
	if (strcmp(curPath, ftwip->rootDir) == 0)
		return (0);	/* ignore root directory */

	lpath = NULL;
	dirsep[0] = ftwip->dirSeparator;
	dirsep[1] = '\0';
	toplevel = 0;
	if ((strrchr(ftwip->curPath, '/') == ftwip->curPath) || (strrchr(ftwip->curPath, '\\') == ftwip->curPath)) {
		/* "get -R /" == "rcp -r remote:/ ." */
		/* Special case files in top-level, i.e. /vmunix */
		toplevel = 1;
		if (Dynscpy(&rdir1, gRemoteCWD, 0) == NULL)
			return (-1);
		if (Dynscpy(&ldir1, gLocalCWD, 0) == NULL)
			return (-1);
	} else if ((ftwip->curPath[0] == '.') && ((ftwip->curPath[1] == '\0') || (((ftwip->curPath[1] == '/') || (ftwip->curPath[1] == '\\')) && (strchr(ftwip->curPath + 2, '/') == NULL) && (strchr(ftwip->curPath + 2, '\\') == NULL)) )) {
		/* "get -R ." == "rcp -r remote:. ." */
		/* Special case files in top- .level, i.e. ./vmunix but not ./bin/ls */
		toplevel = 1;
		if (Dynscpy(&rdir1, gRemoteCWD, 0) == NULL)
			return (-1);
		if (Dynscpy(&ldir1, gLocalCWD, 0) == NULL)
			return (-1);
	} else {
		if (Dynscpy(&rdir1, cinfop->rcwd, "/", curPath, 0) == NULL)
			return (-1);
		if (Dynscpy(&ldir1, cinfop->lcwd, dirsep, baserelpath, 0) == NULL)
			return (-1);
	}

	len = strlen(rdir1);
	rdir = malloc(len + 1);
	if (rdir == NULL)
		return (-1);
	CompressPath(rdir, rdir1, len + 1, 1);
	StrFree(&rdir1);
	cp = strrchr(rdir, '/');
	if ((cp != NULL) && (cp != rdir) && (toplevel == 0))
		*cp = '\0';

	len = strlen(ldir1);
	ldir = malloc(len + 1);
	if (ldir == NULL)
		return (-1);
	CompressPath(ldir, ldir1, len + 1,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		1);
#else
		0);
#endif
	StrFree(&ldir1);
	cp = StrRFindLocalPathDelim(ldir);
	if ((cp != NULL) && (cp != ldir) && (toplevel == 0))
		*cp = '\0';

	if (ftwip->curType != '-') {
		if (Dynscpy(&lpath, ldir, "/", ftwip->curFile, 0) == NULL)
			return (-1);
		TVFSPathToLocalPath(lpath);
		Trace(0, "  // Lpath [%s] ([%s]/%s)\n", lpath, ldir, ftwip->curFile);
	}

	if (ftwip->curType == 'd') {
		/* Directory: Note that batch would mkdir -p if needed.
		 * We do this mostly so "symlink" below will work.
		 */
		if (MkDirs(lpath, 00755) < 0) {
#ifdef HAVE_STRERROR
			Trace(-1, "  - Local Mkdir %s Failed: %s\n", lpath, strerror(errno));
#else
			Trace(-1, "  - Local Mkdir %s Failed: errno %d\n", lpath, (errno));
#endif
		} else if (errno != EEXIST) {
			Trace(0, "  + Mkdir %s\n", lpath);
		} else {
			Trace(0, "  + Mkdir %s (already existed)\n", lpath);
		}
#ifdef HAVE_SYMLINK
	} else if (ftwip->curType == 'l') {
		/* Symlink */
		if (ftwip->rlinkto != NULL) {
			if (symlink(ftwip->rlinkto, lpath) == 0) {
				Trace(0, "  + Linked: %s -> %s\n", lpath, ftwip->rlinkto);
			} else {
#ifdef HAVE_STRERROR
				Trace(-1, "  - Link Failed: %s -> %s (%s)\n", lpath, ftwip->rlinkto, strerror(errno));
#else
				Trace(-1, "  - Link Failed: %s -> %s (errno %d)\n", lpath, ftwip->rlinkto, (errno));
#endif
			}
		}
#endif	/* SYMLINK */
	} else if (ftwip->curType == '-') {
		/* File */
		rc = SpoolX(
			NULL,
			NULL,
			"get",
			ftwip->curFile,
			rdir,
			ftwip->curFile,
			ldir,
			gConn.host,
			gConn.ip,
			gConn.port,
			gConn.user,
			gConn.pass,
			gConn.acct,
			cinfop->xtype,
			kRecursiveNo,
			cinfop->deleteFlag,
			gConn.dataPortMode,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			cinfop->when,
			0,
			gConn.manualOverrideFeatures,
			0
		);
		if (rc == 0) {
			Trace(-1, "  + Spooled: get %s/%s\n", rdir, ftwip->curFile);
			Trace(0, "  + Spooled: get [%s]/%s -> [%s]/%s\n", rdir, ftwip->curFile, ldir, ftwip->curFile);
		}
	}

	StrFree(&rdir);
	StrFree(&ldir);
	StrFree(&lpath);
	return (0);
}	/* BggetFtwProc */



/* (bgget) */
void
SpoolGetCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int optrc;
	int rc;
	int i;
	int nD = 0;
	char pattern[256];
	char *lname;
	char *base;
	FTPLineList ll;
	FTPLinePtr lp;
	GetoptInfo opt;
	FtwInfo ftwi;
	SpoolCmdInfo cinfo;

	cinfo.xtype = gBm.xferType;
	cinfo.deleteFlag = kDeleteNo;
	cinfo.recurseFlag = kRecursiveNo;
	cinfo.renameMode = 0;
	cinfo.when = 0;
	cinfo.base = 0;

	Trace(0, "Local CWD is: %s\n", gLocalCWD);
	Trace(0, "Remote CWD is: %s\n", gRemoteCWD);

	cinfo.rcwd = gRemoteCWD;
	if (((cinfo.rcwd[0] == '/') || (cinfo.rcwd[0] == '\\')) && (cinfo.rcwd[1] == '\0'))
		cinfo.rcwd++;

	cinfo.lcwd = gLocalCWD;
	if (strcmp(cinfo.lcwd, "/") == 0)
		cinfo.lcwd++;

	ARGSUSED(gUnusedArg);

	if ((gSavePasswords <= 0) && ((strcmp(gConn.user, "anonymous") != 0) && (strcmp(gConn.user, "ftp") != 0))) {
		(void) printf("Sorry, spooling isn't allowed when you're not logged in anonymously, because\nthe spool files would need to save your password.\n\nYou can override this by doing a \"set save-passwords yes\" if you're willing\nto live with the consequences.\n");
		return;
	} else if (SpoolCheck() < 0) {
		return;
	}

	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "@:azfrRD")) >= 0) switch (optrc) {
		case '@':
			cinfo.when = GetStartSpoolDate(opt.arg);
			if ((cinfo.when == (time_t) -1) || (cinfo.when == (time_t) 0)) {
				(void) fprintf(stderr, "Bad date.  It must be expressed as one of the following:\n\tYYYYMMDDHHMMSS\t\n\t\"now + N hours|min|sec|days\"\n\tHH:MM\n\nNote:  Do not forget to quote the entire argument for the offset option.\nExample:  bgget -@ \"now + 15 min\" ...\n");
				return;
			}
			break;
		case 'a':
			cinfo.xtype = kTypeAscii;
			break;
		case 'z':
			/* Special flag that lets you specify the
			 * destination file.  Normally a "get" will
			 * write the file by the same name as the
			 * remote file's basename.
			 */
			cinfo.renameMode = 1;
			break;
		case 'D':
			/* You can delete the remote file after
			 * you downloaded it successfully by using
			 * the -DD option.  It requires two -D's
			 * to minimize the odds of accidentally
			 * using a single -D.
			 */
			nD++;
			break;
		case 'r':
		case 'R':
			/* If the item is a directory, get the
			 * directory and all its contents.
			 */
			cinfo.recurseFlag = kRecursiveYes;
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}

	if (nD >= 2)
		cinfo.deleteFlag = kDeleteYes;

	if (cinfo.renameMode != 0) {
		if (opt.ind > argc - 2) {
			PrintCmdUsage(cmdp);
			return;
		}
		rc = SpoolX(
			NULL,
			NULL,
			"get",
			argv[opt.ind],
			gRemoteCWD,
			argv[opt.ind + 1],
			gLocalCWD,
			gConn.host,
			gConn.ip,
			gConn.port,
			gConn.user,
			gConn.pass,
			gConn.acct,
			cinfo.xtype,
			kRecursiveNo,
			cinfo.deleteFlag,
			gConn.dataPortMode,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			cinfo.when,
			0,
			gConn.manualOverrideFeatures,
			0
		);
		if (rc == 0) {
			Trace(-1, "  + Spooled: get %s as %s\n", argv[opt.ind], argv[opt.ind + 1]);
		}
		return;
	}

	if (cinfo.recurseFlag == kRecursiveYes) {
		FtwInit(&ftwi);
		ftwi.userdata = &cinfo;
	}

	for (i=opt.ind; i<argc; i++) {
		STRNCPY(pattern, argv[i]);
		StrRemoveTrailingSlashes(pattern);
		InitLineList(&ll);
		rc = FTPRemoteGlob(&gConn, &ll, pattern, (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (rc < 0) {
			FTPPerror(&gConn, rc, kErrGlobFailed, argv[0], pattern);
			goto done;
		}
		for (lp = ll.first; ((lp != NULL) && (lp->line != NULL)); lp = lp->next) {
			if (cinfo.recurseFlag == kRecursiveNo) {
				lname = strrchr(lp->line, '/');
				if (lname == NULL)
					lname = lp->line;
				else
					lname++;
				rc = SpoolX(
					NULL,
					NULL,
					"get",
					lp->line,
					gRemoteCWD,
					lname,
					gLocalCWD,
					gConn.host,
					gConn.ip,
					gConn.port,
					gConn.user,
					gConn.pass,
					gConn.acct,
					cinfo.xtype,
					cinfo.recurseFlag,
					cinfo.deleteFlag,
					gConn.dataPortMode,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					cinfo.when,
					0,
					gConn.manualOverrideFeatures,
					0
				);
				if (rc == 0) {
					Trace(-1, "  + Spooled: get %s\n", lp->line);
				}
			} else /* if (cinfo.recurseFlag == kRecursiveYes) */ {
				base = StrRFindLocalPathDelim(lp->line);
				if (base == NULL) {
					cinfo.base = 0;
				} else if (base == lp->line) {
					cinfo.base = 0;
				} else {
					base++;
					cinfo.base = (int) (base - lp->line);
				}
				if ((rc = FTPFtw(&gConn, &ftwi, lp->line, BggetFtwProc)) != 0) {
					FTPPerror(&gConn, gConn.errNo, kErrCWDFailed, lp->line, "Could not traverse directory");
				}
			}
		}
		DisposeLineListContents(&ll);
	}
done:
	if (cinfo.recurseFlag == kRecursiveYes)
		FtwDispose(&ftwi);
}	/* SpoolGetCmd */



static int
BgputFtwProc(const FtwInfoPtr ftwip)
{
	const char *basererpath;
	char *curPath;
	char *rdir, *ldir;
	char *rdir1, *ldir1, *cp;
	size_t len;
	char *rpath;
	char dirsep[2];
	int rc;
	int toplevel;
	SpoolCmdInfo *cinfop;
	char errStr[128];

	cinfop = (SpoolCmdInfo *) ftwip->userdata;
	/* rerpath = ftwip->curPath + ftwip->startPathLen + 1; */
	basererpath = ftwip->curPath + cinfop->base;
	curPath = ftwip->curPath;
	if (strcmp(curPath, ftwip->rootDir) == 0)
		return (0);	/* ignore root directory */

	rpath = NULL;
	dirsep[0] = ftwip->dirSeparator;
	dirsep[1] = '\0';
	toplevel = 0;
	if ((strrchr(ftwip->curPath, '/') == ftwip->curPath) || (strrchr(ftwip->curPath, '\\') == ftwip->curPath)) {
		/* "get -R /" == "rcp -r remote:/ ." */
		/* Special case files in top-level, i.e. /vmunix */
		toplevel = 1;
		if (Dynscpy(&rdir1, gRemoteCWD, 0) == NULL)
			return (-1);
		if (Dynscpy(&ldir1, gLocalCWD, 0) == NULL)
			return (-1);
	} else if ((ftwip->curPath[0] == '.') && ((ftwip->curPath[1] == '\0') || (((ftwip->curPath[1] == '/') || (ftwip->curPath[1] == '\\')) && (strchr(ftwip->curPath + 2, '/') == NULL) && (strchr(ftwip->curPath + 2, '\\') == NULL)) )) {
		/* "get -R ." == "rcp -r remote:. ." */
		/* Special case files in top- .level, i.e. ./vmunix but not ./bin/ls */
		toplevel = 1;
		if (Dynscpy(&rdir1, gRemoteCWD, 0) == NULL)
			return (-1);
		if (Dynscpy(&ldir1, gLocalCWD, 0) == NULL)
			return (-1);
	} else {
		if (IsLocalPathDelim(curPath[0])) {
			if (Dynscpy(&ldir1, curPath, 0) == NULL)
				return (-1);
		} else {
			if (Dynscpy(&ldir1, cinfop->lcwd, "/", curPath, 0) == NULL)
				return (-1);
		}
		if (Dynscpy(&rdir1, cinfop->rcwd, dirsep, basererpath, 0) == NULL)
			return (-1);
	}

	len = strlen(rdir1);
	rdir = malloc(len + 1);
	if (rdir == NULL)
		return (-1);
	CompressPath(rdir, rdir1, len + 1, 1);
	StrFree(&rdir1);
	cp = strrchr(rdir, '/');
	if ((cp != NULL) && (cp != rdir) && (toplevel == 0))
		*cp = '\0';

	len = strlen(ldir1);
	ldir = malloc(len + 1);
	if (ldir == NULL)
		return (-1);
	CompressPath(ldir, ldir1, len + 1,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		1);
#else
		0);
#endif
	StrFree(&ldir1);
	cp = StrRFindLocalPathDelim(ldir);
	if ((cp != NULL) && (cp != ldir) && (toplevel == 0))
		*cp = '\0';

	if (ftwip->curType != '-') {
		if (Dynscpy(&rpath, rdir, "/", ftwip->curFile, 0) == NULL)
			return (-1);
		TVFSPathToLocalPath(rpath);
		Trace(0, "  // Rpath [%s] ([%s]/%s)\n", rpath, rdir, ftwip->curFile);
	}

	if (ftwip->curType == 'd') {
		/* Directory: Note that batch would mkdir -p if needed.
		 * We do this mostly so "symlink" below will work.
		 */
		if ((rc = FTPMkdir(&gConn, rpath, kRecursiveYes)) < 0) {
			Trace(-1, "  - Remote Mkdir %s Failed: %s\n", rpath, FTPStrError2(&gConn, gConn.errNo, errStr, sizeof(errStr), kErrMKDFailed));
			goto done;
		} else {
			Trace(0, "  + Mkdir %s\n", rpath);
		}
#ifdef HAVE_SYMLINK
	} else if (ftwip->curType == 'l') {
		/* Symlink */
		if (ftwip->rlinkto != NULL) {
			if (FTPSymlink(&gConn, ftwip->rlinkto, rpath) == 0) {
				Trace(0, "  + Linked: %s -> %s\n", rpath, ftwip->rlinkto);
			} else {
				Trace(-1, "  - Link Failed: %s -> %s (%s)\n", rpath, ftwip->rlinkto, FTPStrError2(&gConn, gConn.errNo, errStr, sizeof(errStr), kErrSYMLINKFailed));
			}
		}
#endif	/* SYMLINK */
	} else if (ftwip->curType == '-') {
		/* File */
		rc = SpoolX(
			NULL,
			NULL,
			"put",
			ftwip->curFile,
			rdir,
			ftwip->curFile,
			ldir,
			gConn.host,
			gConn.ip,
			gConn.port,
			gConn.user,
			gConn.pass,
			gConn.acct,
			cinfop->xtype,
			kRecursiveNo,
			cinfop->deleteFlag,
			gConn.dataPortMode,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			cinfop->when,
			0,
			gConn.manualOverrideFeatures,
			0
		);
		if (rc == 0) {
			Trace(-1, "  + Spooled: put %s/%s\n", ldir, ftwip->curFile);
			Trace(0, "  + Spooled: put [%s]/%s -> [%s]/%s\n", ldir, ftwip->curFile, rdir, ftwip->curFile);
		}
	}
	rc = 0;
done:
	StrFree(&rdir);
	StrFree(&ldir);
	StrFree(&rpath);
	return (rc);
}	/* BgputFtwProc */




/* (bgput) */
void
SpoolPutCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int optrc;
	int rc;
	int i;
	int nD = 0;
	char pattern[256];
	FTPLineList ll;
	FTPLinePtr lp;
	char *rname;
	char *base;
	GetoptInfo opt;
	FtwInfo ftwi;
	SpoolCmdInfo cinfo;

	cinfo.xtype = gBm.xferType;
	cinfo.deleteFlag = kDeleteNo;
	cinfo.recurseFlag = kRecursiveNo;
	cinfo.renameMode = 0;
	cinfo.when = 0;
	cinfo.base = 0;

	Trace(0, "Local CWD is: %s\n", gLocalCWD);
	Trace(0, "Remote CWD is: %s\n", gRemoteCWD);

	cinfo.rcwd = gRemoteCWD;
	if (((cinfo.rcwd[0] == '/') || (cinfo.rcwd[0] == '\\')) && (cinfo.rcwd[1] == '\0'))
		cinfo.rcwd++;

	cinfo.lcwd = gLocalCWD;
	if (strcmp(cinfo.lcwd, "/") == 0)
		cinfo.lcwd++;

	ARGSUSED(gUnusedArg);

	if ((gSavePasswords <= 0) && ((strcmp(gConn.user, "anonymous") != 0) && (strcmp(gConn.user, "ftp") != 0))) {
		(void) printf("Sorry, spooling isn't allowed when you're not logged in anonymously, because\nthe spool files would need to save your password.\n\nYou can override this by doing a \"set save-passwords yes\" if you're willing\nto live with the consequences.\n");
		return;
	} else if (SpoolCheck() < 0) {
		return;
	}

	GetoptReset(&opt);
	while ((optrc = Getopt(&opt, argc, argv, "@:azrRD")) >= 0) switch (optrc) {
		case '@':
			cinfo.when = GetStartSpoolDate(opt.arg);
			if ((cinfo.when == (time_t) -1) || (cinfo.when == (time_t) 0)) {
				(void) fprintf(stderr, "Bad date.  It must be expressed as one of the following:\n\tYYYYMMDDHHMMSS\t\n\t\"now + N hours|min|sec|days\"\n\tHH:MM\n\nNote:  Do not forget to quote the entire argument for the offset option.\nExample:  bgget -@ \"now + 15 min\" ...\n");
				return;
			}
			break;
		case 'a':
			cinfo.xtype = kTypeAscii;
			break;
		case 'z':
			/* Special flag that lets you specify the
			 * destination file.  Normally a "put" will
			 * write the file by the same name as the
			 * remote file's basename.
			 */
			cinfo.renameMode = 1;
			break;
		case 'D':
			/* You can delete the remote file after
			 * you uploaded it successfully by using
			 * the -DD option.  It requires two -D's
			 * to minimize the odds of accidentally
			 * using a single -D.
			 */
			nD++;
			break;
		case 'r':
		case 'R':
			/* If the item is a directory, get the
			 * directory and all its contents.
			 */
			cinfo.recurseFlag = kRecursiveYes;
			break;
		default:
			PrintCmdUsage(cmdp);
			return;
	}

	if (nD >= 2)
		cinfo.deleteFlag = kDeleteYes;

	if (cinfo.renameMode != 0) {
		if (opt.ind > argc - 2) {
			PrintCmdUsage(cmdp);
			return;
		}
		rc = SpoolX(
			NULL,
			NULL,
			"put",
			argv[opt.ind + 1],
			gRemoteCWD,
			argv[opt.ind + 0],
			gLocalCWD,
			gConn.host,
			gConn.ip,
			gConn.port,
			gConn.user,
			gConn.pass,
			gConn.acct,
			cinfo.xtype,
			kRecursiveNo,
			cinfo.deleteFlag,
			gConn.dataPortMode,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			cinfo.when,
			0,
			gConn.manualOverrideFeatures,
			0
		);
		if (rc == 0) {
			Trace(-1, "  + Spooled: put %s as %s\n", argv[opt.ind], argv[opt.ind + 1]);
		}
		return;
	}

	if (cinfo.recurseFlag == kRecursiveYes) {
		FtwInit(&ftwi);
		ftwi.userdata = &cinfo;
	}

	for (i=opt.ind; i<argc; i++) {
		STRNCPY(pattern, argv[i]);
		StrRemoveTrailingSlashes(pattern);
		InitLineList(&ll);
		rc = FTPLocalGlob(&gConn, &ll, pattern, (aip->noglobargv[i] != 0) ? kGlobNo: kGlobYes);
		if (rc < 0) {
			FTPPerror(&gConn, rc, kErrGlobFailed, "local glob", pattern);
			return;
		}
		for (lp = ll.first; ((lp != NULL) && (lp->line != NULL)); lp = lp->next) {
			if (cinfo.recurseFlag == kRecursiveNo) {
				rname = strrchr(lp->line, '/');
				if (rname == NULL)
					rname = lp->line;
				else
					rname++;
				rc = SpoolX(
					NULL,
					NULL,
					"put",
					rname,
					gRemoteCWD,
					lp->line,
					gLocalCWD,
					gConn.host,
					gConn.ip,
					gConn.port,
					gConn.user,
					gConn.pass,
					gConn.acct,
					cinfo.xtype,
					cinfo.recurseFlag,
					cinfo.deleteFlag,
					gConn.dataPortMode,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					cinfo.when,
					0,
					gConn.manualOverrideFeatures,
					0
				);
				if (rc == 0) {
					Trace(-1, "  + Spooled: put %s\n", lp->line);
				}
			} else {
				base = StrRFindLocalPathDelim(lp->line);
				if (base == NULL) {
					cinfo.base = 0;
				} else if (base == lp->line) {
					cinfo.base = 0;
				} else {
					base++;
					cinfo.base = (int) (base - lp->line);
				}
				if ((rc = Ftw(&ftwi, lp->line, BgputFtwProc)) != 0) {
#ifdef HAVE_STRERROR
					fprintf(stderr, "Could not traverse directory %s: %s\n", lp->line, strerror(errno));
#else
					fprintf(stderr, "Could not traverse directory %s: errno %d\n", lp->line, (errno));
#endif
				}
			}
		}
		DisposeLineListContents(&ll);
	}
}	/* SpoolPutCmd */




/* This commands lets the user create symbolic links, if the server supports it. */
void
SymlinkCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;

	ARGSUSED(gUnusedArg);
	result = FTPSymlink(&gConn, argv[1], argv[2]);
	if (result < 0)
		FTPPerror(&gConn, result, kErrSYMLINKFailed, "symlink", argv[1]);

	/* Really should just flush only the modified directories... */
	FlushLsCache();
}	/* SymlinkCmd */




/* This commands lets the user change the transfer type to use. */
void
TypeCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int c;
	int result;
	const char *cs;

	ARGSUSED(gUnusedArg);
	if (argc < 2) {
		c = argv[0][0];
		if (c == 't') {
			if (gBm.xferType == kTypeAscii) {
				c = kTypeAscii;
				cs = "ASCII";
			} else if (gBm.xferType == kTypeEbcdic) {
				c = kTypeEbcdic;
				cs = "EBCDIC";
			} else {
				c = kTypeBinary;
				cs = "binary/image";
			}
			Trace(-1, "Type is %c (%s).\n", c, cs);
		} else {
			result = FTPSetTransferType(&gConn, c);
			if (result < 0) {
				FTPPerror(&gConn, result, kErrTYPEFailed, "Type", argv[1]);
			} else {
				gBm.xferType = gConn.curTransferType;
			}
		}
	} else {
		c = argv[1][0];
		result = FTPSetTransferType(&gConn, c);
		if (result < 0) {
			FTPPerror(&gConn, result, kErrTYPEFailed, "Type", argv[1]);
		} else {
			gBm.xferType = gConn.curTransferType;
		}
	}
}	/* TypeCmd */




/* This commands lets the user change the umask, if the server supports it. */
void
UmaskCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	int result;

	ARGSUSED(gUnusedArg);
	result = FTPUmask(&gConn, argv[1]);
	if (result < 0)
		FTPPerror(&gConn, result, kErrUmaskFailed, "umask", argv[1]);
}	/* UmaskCmd */




/* Show the version information. */
void
VersionCmd(const int argc, char **const argv, const CommandPtr cmdp, const ArgvInfoPtr aip)
{
	ARGSUSED(gUnusedArg);
	(void) printf("Version:          %s\n", gVersion + 5);
	(void) printf("Author:           Mike Gleason (http://www.NcFTP.com/ncftp/)\n");
#ifndef BETA
	(void) printf("Archived at:      ftp://ftp.NcFTP.com/ncftp/\n");
#endif
	(void) printf("Library Version:  %s\n", gLibNcFTPVersion + 5);
#ifdef __DATE__
	(void) printf("Compile Date:     %s\n", __DATE__);
#endif
	if (gOS[0] != '\0')
		(void) printf("Platform:         %s\n", gOS);
}	/* VersionCmd */

/* vim: set noet sw=8: */
