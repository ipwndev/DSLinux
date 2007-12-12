/* main.c
 *
 * Copyright (c) 1992-2005 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif
#include "ls.h"
#include "bookmark.h"
#include "shell.h"
#include "cmds.h"
#include "main.h"
#include "progress.h"
#include "pref.h"
#include "readln.h"
#include "trace.h"
#include "log.h"
#include "spool.h"
#include "util.h"

int gStartupUrlParameterGiven = 0;
int gIsTTY, gIsTTYr;
int gScreenColumns;

FTPLibraryInfo gLib;
FTPConnectionInfo gConn;
FTPLineList gStartupURLCdList;
char *gXBuf = NULL;
size_t gXBufSize = 0;
int gTransferTypeInitialized = 0;
int gTransferType;
int gURLMode = 0;
extern int gUnprocessedJobs;
char gLocalCWD[512], gPrevLocalCWD[512];

extern char gRemoteCWD[512], gPrevRemoteCWD[512];
extern Bookmark gBm;
extern int gLoadedBm;
extern int gFirewallType;
extern char gAutoAscii[];
extern char gFirewallHost[64];
extern char gFirewallUser[32];
extern char gFirewallPass[32];
extern char gFirewallExceptionList[];
extern char gCopyright[], gVersion[];
extern unsigned int gFirewallPort;
extern int gConnTimeout, gXferTimeout, gCtrlTimeout;
extern int gDataPortMode, gRedialDelay;
extern int gDebug;
extern int gNumProgramRuns, gDoNotDisplayAds;
extern int gSOBufsize;
extern FTPProgressMeterProc gProgressMeter;
extern char gOurHostName[64];
extern int gGetOurHostNameResult;
extern char gStartDir[];

static void
Usage(void)
{
	FILE *fp;
#ifdef UNAME
	char s[80];
#endif

	fp = stderr;
	(void) fprintf(fp, "\nUsage:  ncftp [flags] [<host> | <directory URL to browse>]\n");
	(void) fprintf(fp, "\nFlags:\n\
  -u XX  Use username XX instead of anonymous.\n\
  -p XX  Use password XX with the username.\n\
  -P XX  Use port number XX instead of the default FTP service port (21).\n\
  -j XX  Use account XX with the username (rarely needed).\n\
  -F     Dump a sample $HOME/.ncftp/firewall prefs file to stdout and exit.\n");

	(void) fprintf(fp, "\nProgram version:  %s\nLibrary version:  %s\n", gVersion + 5, gLibNcFTPVersion + 5);
#ifdef UNAME
	AbbrevStr(s, UNAME, 60, 1);
	(void) fprintf(fp, "System:           %s\n", s);
#endif
	(void) fprintf(fp, "\nThis is a freeware program by Mike Gleason (http://www.NcFTP.com).\n");
	(void) fprintf(fp, "A directory URL ends in a slash, i.e. ftp://ftp.freebsd.org/pub/FreeBSD/\n");
	(void) fprintf(fp, "Use ncftpget and ncftpput for command-line FTP and file URLs.\n\n");
	exit(2);
}	/* Usage */




static void
DumpFirewallPrefsTemplate(void)
{
	WriteDefaultFirewallPrefs(stdout);
}	/* DumpFirewallPrefsTemplate */




/* This resets our state information whenever we are ready to open a new
 * host.
 */
void
InitConnectionInfo(void)
{
	int result;

	result = FTPInitConnectionInfo2(&gLib, &gConn, gXBuf, gXBufSize);
	if (result < 0) {
		(void) fprintf(stderr, "ncftp: init connection info error %d (%s).\n", result, FTPStrError(result));
		exit(1);
	}

	gConn.debugLog = NULL;
	gConn.errLog = stderr;
	SetDebug(gDebug);
	UseTrace();
	(void) STRNCPY(gConn.user, "anonymous");
	gConn.host[0] = '\0';
	gConn.progress = gProgressMeter;
	gTransferTypeInitialized = 0;
	gTransferType = kTypeBinary;
	gConn.leavePass = 1;		/* Don't let the lib zap it. */
	gConn.connTimeout = gConnTimeout;
	gConn.xferTimeout = gXferTimeout;
	gConn.ctrlTimeout = gCtrlTimeout;
	gConn.dataPortMode = gDataPortMode;
	gConn.maxDials = (-1);	/* Dial forever, until they hit ^C. */
	gUnprocessedJobs = 0;
	gPrevRemoteCWD[0] = '\0';
	gStartDir[0] = '\0';
	gConn.dataSocketRBufSize = gConn.dataSocketSBufSize = gSOBufsize;
	if (gRedialDelay >= 10)
		gConn.redialDelay = gRedialDelay;
	if ((gAutoAscii[0] == '\0') || (ISTREQ(gAutoAscii, "no")) || (ISTREQ(gAutoAscii, "off")) || (ISTREQ(gAutoAscii, "false"))) {
		gConn.asciiFilenameExtensions = NULL;
	} else {
		gConn.asciiFilenameExtensions = gAutoAscii;
	}
}	/* InitConnectionInfo */




/* This lets us do things with our state information just before the
 * host is closed.
 */
void
CloseHost(void)
{
	if (gConn.connected != 0) {
		if (gConn.loggedIn != 0) {
			SaveUnsavedBookmark();
		}
		RunBatchIfNeeded(&gConn);
	}
	gConn.ctrlTimeout = 3;
	(void) FTPCloseHost(&gConn);
	gPrevRemoteCWD[0] = '\0';
	gStartDir[0] = '\0';
}	/* CloseHost */




/* If the user specified a URL on the command-line, this initializes
 * our state information based upon it.
 */
static void
SetStartupURL(const char *const urlgiven)
{
	int rc;
	char url[256];
	char urlfile[128];

	gLoadedBm = 0;
	(void) STRNCPY(url, urlgiven);

	rc = DecodeDirectoryURL(&gConn, url, &gStartupURLCdList, urlfile, sizeof(urlfile));
	if (rc == kMalformedURL) {
		(void) fprintf(stderr, "Malformed URL: %s\n", url);
		exit(1);
	} else if (rc == kNotURL) {
		/* This is what should happen most of the time. */
		(void) STRNCPY(gConn.host, urlgiven);
		gURLMode = 2;
		if (GetBookmark(gConn.host, &gBm) >= 0) {
			gLoadedBm = 1;
			(void) STRNCPY(gConn.host, gBm.name);
			(void) STRNCPY(gConn.user, gBm.user);
			(void) STRNCPY(gConn.pass, gBm.pass);
			(void) STRNCPY(gConn.acct, gBm.acct);
			gConn.hasSIZE = gBm.hasSIZE;
			gConn.hasMDTM = gBm.hasMDTM;
			gConn.hasPASV = gBm.hasPASV;
			gConn.hasSITE_UTIME = gBm.hasUTIME;
			gConn.port = gBm.port;
		} else {
			SetBookmarkDefaults(&gBm);
		}

		if (MayUseFirewall(gConn.host, gFirewallType, gFirewallExceptionList) != 0) {
			gConn.firewallType = gFirewallType; 
			(void) STRNCPY(gConn.firewallHost, gFirewallHost);
			(void) STRNCPY(gConn.firewallUser, gFirewallUser);
			(void) STRNCPY(gConn.firewallPass, gFirewallPass);
			gConn.firewallPort = gFirewallPort;
		}
	} else {
		/* URL okay */
		if (urlfile[0] != '\0') {
			/* It was obviously not a directory */
			(void) fprintf(stderr, "Use ncftpget or ncftpput to handle file URLs.\n");
			exit(1);
		}
		gURLMode = 1;
		if (MayUseFirewall(gConn.host, gFirewallType, gFirewallExceptionList) != 0) {
			gConn.firewallType = gFirewallType; 
			(void) STRNCPY(gConn.firewallHost, gFirewallHost);
			(void) STRNCPY(gConn.firewallUser, gFirewallUser);
			(void) STRNCPY(gConn.firewallPass, gFirewallPass);
			gConn.firewallPort = gFirewallPort;
		}
	}
}	/* SetStartupURL */




static void
OpenURL(void)
{
	FTPLinePtr lp;
	int result;

	if (gURLMode == 1) {
		SetBookmarkDefaults(&gBm);
		if (DoOpen() >= 0) {
			for (lp = gStartupURLCdList.first; lp != NULL; lp = lp->next) {
				result = FTPChdir(&gConn, lp->line);
				if (result != kNoErr) {
					FTPPerror(&gConn, result, kErrCWDFailed, "Could not chdir to", lp->line);
					break;
				}
			}
			result = FTPGetCWD(&gConn, gRemoteCWD, sizeof(gRemoteCWD));
			if (result != kNoErr) {
				FTPPerror(&gConn, result, kErrPWDFailed, NULL, NULL);
			} else {
				(void) printf("Current remote directory is %s.\n", gRemoteCWD);
			}
		}
	} else if (gURLMode == 2) {
		(void) DoOpen();
	}
}	/* OpenURL */




/* These things are done first, before we even parse the command-line
 * options.
 */
static void
PreInit(void)
{
	int result;

	InitWinsock();
#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	gIsTTY = 1;
	gIsTTYr = 1;
#elif defined(__CYGWIN__)
	gIsTTY = (isatty(2) != 0) ? 1 : 0;
	gIsTTYr = (isatty(0) != 0) ? 1 : 0;
#else
	gIsTTY = ((isatty(2) != 0) && (getppid() > 1)) ? 1 : 0;
	gIsTTYr = ((isatty(0) != 0) && (getppid() > 1)) ? 1 : 0;
#endif
#ifdef SIGPOLL
	(void) NcSignal(SIGPOLL, (FTPSigProc) SIG_IGN);
#endif
	gXBufSize = kDefaultFTPBufSize;
	gXBuf = malloc(gXBufSize);
	if (gXBuf == NULL) {
		perror("malloc");
		exit(1);
	}
	InitUserInfo();
	result = FTPInitLibrary(&gLib);
	if (result < 0) {
		(void) fprintf(stderr, "ncftp: init library error %d (%s).\n", result, FTPStrError(result));
		exit(1);
	}
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	srand((unsigned int) (GetTickCount() & 0x7FFF));
#else
	srand((unsigned int) getpid());
#endif
	InitLineList(&gStartupURLCdList);
	CheckForNewV3User();
	InitLog();
	InitPrefs();
	LoadFirewallPrefs(0);
	LoadPrefs();
	InitConnectionInfo();
	InitCommandList();
	InitLs();
	TruncBatchLog();
	GetScreenColumns();
}	/* PreInit */





/* These things are done at startup, but after we parse the command-line
 * options.
 */
static void
PostInit(void)
{
	PostInitPrefs();
	if (gGetOurHostNameResult == 100)	/* avoid doing again */
		gGetOurHostNameResult = GetOurHostName(gOurHostName, sizeof(gOurHostName));
	OpenTrace();
	InitTermcap();
	InitReadline();
	(void) FTPGetLocalCWD(gLocalCWD, sizeof(gLocalCWD));
	gPrevLocalCWD[0] = '\0';
	PrintStartupBanner();
	if (gNumProgramRuns <= 1)
		(void) printf("\n%s\n", gCopyright + 5);

	Trace(0, "Fw: %s  Type: %d  User: %s  Pass: %s  Port: %u\n", 
		gFirewallHost,
		gFirewallType,
		gFirewallUser,
		(gFirewallPass[0] == '\0') ? "(none)" : "********",
		gFirewallPort
	);
	Trace(0, "FwExceptions: %s\n", gFirewallExceptionList);
	if (strchr(gOurHostName, '.') == NULL) {
		Trace(0, "NOTE:  Your domain name could not be detected.\n");
		if (gConn.firewallType != kFirewallNotInUse) {
			Trace(0, "       Make sure you manually add your domain name to firewall-exception-list.\n");
		}
	}
}	/* PostInit */




/* Try to get the user to evaluate our commercial offerings. */
static void
Plug(void)
{
#if defined(WIN32) || defined(_WINDOWS) || defined(__CYGWIN__)
	/* NcFTPd hasn't been ported to Windows. */
#else
	if (gDoNotDisplayAds == 0) {
		if ((gNumProgramRuns % 4) == 3) {
			if ((rand() % 3) == 2) {
				(void) printf("\n\n\n\tThank you for using NcFTP Client.\n\tAsk your system administrator to try NcFTPd Server!\n\thttp://www.ncftp.com/ncftpd/\n\n\n\n");
			} else {
				(void) printf("\n\n\n\tThank you for using NcFTP Client.\n\tIf you find it useful, please consider making a donation!\n\thttp://www.ncftp.com/ncftp/donate.html\n\n\n\n");
			}
		}
	}
#endif
}	/* Plug */




/* These things are just before the program exits. */
static void
PostShell(void)
{
	SetXtermTitle("RESTORE");
	CloseHost();
	FlushLsCache();
	DisposeReadline();
	CloseTrace();
	SavePrefs();
	DisposeBookmarkTable();
	EndLog();
	Plug();
	if (gXBuf != NULL)
		free(gXBuf);
	DisposeWinsock();
}	/* PostShell */




main_void_return_t
main(int argc, char **const argv)
{
	int c;
	int n;
	GetoptInfo opt;

	PreInit();
	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:p:j:rd:eg:o:FVLD")) > 0) switch(c) {
		case 'P':
		case 'u':
		case 'p':
		case 'j':
			gStartupUrlParameterGiven = 1;
			break;
		case 'r':
		case 'g':
		case 'd':
		case 'e':
		case 'o':
		case 'V':
		case 'L':
		case 'D':
		case 'F':
			break;
		default:
			Usage();
	}

	if (opt.ind < argc) {
		LoadFirewallPrefs(0);
		SetStartupURL(argv[opt.ind]);
	} else if (gStartupUrlParameterGiven != 0) {
		/* One of the flags they specified
		 * requires a URL or hostname to
		 * open.
		 */
		Usage();
	}

	/* Allow command-line parameters to override
	 * bookmark settings.
	 */
	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:p:j:rd:eg:o:FVLD")) > 0) switch(c) {
		case 'P':
			gConn.port = atoi(opt.arg);	
			break;
		case 'u':
			(void) STRNCPY(gConn.user, opt.arg);
			memset(opt.arg, '*', strlen(opt.arg));
			break;
		case 'p':
			(void) STRNCPY(gConn.pass, opt.arg);	/* Don't recommend doing this! */
			memset(opt.arg, '*', strlen(opt.arg));
			break;
		case 'j':
			(void) STRNCPY(gConn.acct, opt.arg);
			memset(opt.arg, '*', strlen(opt.arg));
			break;
		case 'e':
			gGetOurHostNameResult = GetOurHostName(gOurHostName, sizeof(gOurHostName));
			printf("%s\n", gOurHostName);
			exit(0);
			break;
		case 'r':
			/* redial is always on */
			break;
		case 'g':
			gConn.maxDials = atoi(opt.arg);
			break;
		case 'd':
			n = atoi(opt.arg);
			if (n >= 10)
				gConn.redialDelay = n;
			break;
		case 'o':
			gConn.manualOverrideFeatures = opt.arg;
			break;
		case 'F':
			DumpFirewallPrefsTemplate();
			exit(0);
			/*NOTREACHED*/
			break;
		case 'V':
			/*FALLTHROUGH*/
		case 'L':
			/*FALLTHROUGH*/
		case 'D':
			/* silently ignore these legacy options */
			break;
		default:
			Usage();
	}

#if (defined(SOCKS)) && (SOCKS >= 5)
	SOCKSinit(argv[0]);
#endif	/* SOCKS */
	PostInit();
	OpenURL();
	CommandShell();
	PostShell();
	exit(0);
}	/* main */
