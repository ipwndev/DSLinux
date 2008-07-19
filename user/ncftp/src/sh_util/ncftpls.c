/* ncftpls.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 * A non-interactive utility to list directories on a remote FTP server.
 * Very useful in shell scripts!
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	include "..\ncftp\util.h"
#	include "..\ncftp\spool.h"
#	include "..\ncftp\pref.h"
#	include "..\ncftp\gl_getline.h"
#else
#	include "../ncftp/util.h"
#	include "../ncftp/spool.h"
#	include "../ncftp/pref.h"
#	include "../ncftp/gl_getline.h"
#endif

#include "gpshare.h"

FTPLibraryInfo gLib;
FTPConnectionInfo fi;

extern int gFirewallType;
extern char gFirewallHost[64];
extern char gFirewallUser[32];
extern char gFirewallPass[32];
extern unsigned int gFirewallPort;
extern char gFirewallExceptionList[256];
extern int gFwDataPortMode;
extern const char gOS[], gVersion[];

static int FTPRemoteRecursiveMList(FTPCIPtr cip, const char *const rdir, /* FTPFileInfoListPtr files, */ FTPLineListPtr lines);

static void
FTPRemoteRecursiveMListSubdir(FTPCIPtr cip, char *const parentdir, const size_t pdsize, const size_t pdlen, const char *const subdir, FTPLineListPtr lines)
{
	size_t sdlen = strlen(subdir);
	size_t newlen;
	char *relpath = parentdir;
	int mls = 1;
	int unlsrc;
	FTPFileInfoPtr fip;
	FTPLineList ll;
	FTPLinePtr lp;
	FTPFileInfoList fil;
	MLstItem mli;
	char *cp;
	char *newl;

	if (pdlen + sdlen + /* '/' */ 1 + /* '\0' */ 1 > pdsize) {
		return;
	}

	if (pdlen == 0) {
		memcpy(relpath + 0, subdir, sdlen + /* '\0' */ 1);
		newlen = sdlen;
	} else {
		relpath[pdlen] = '/';
		memcpy(relpath + pdlen + 1, subdir, sdlen + /* '\0' */ 1);
		newlen = pdlen + sdlen + 1;
	}

	(void) AddLine(lines, "");
	if (Dynscpy(&newl, relpath, ":", 0) != NULL) {
		(void) AddLine(lines, newl);
		free(newl);
	}

	/* Paths collected must be relative. */
	if (((FTPListToMemory2(cip, relpath, &ll, "-a", 0, &mls)) < 0) || (ll.first == NULL)) {
		/* Not an error unless the first directory could not be opened. */
		DisposeLineListContents(&ll);
		goto done;
	}

	/* "MLSD" succeeded */
	unlsrc = UnMlsD(cip, &fil, &ll);
	if (unlsrc < 0) {
		/* Return the lines as is -- even though it's invalid. */
		goto done;
	} else if (unlsrc == 0) {
		/* empty */
		goto done;
	}
	fip = fil.first;

	/* Concat the raw MLST data. */
	for (lp = ll.first; lp != NULL ; lp=lp->next) {
		if (lp->line != NULL) {
			if ((UnMlsT(cip, lp->line, &mli) == 0) && ((mli.ftype == '-') || (mli.ftype == 'd')) && (strchr(mli.fname, '/') == NULL) && ((cp = strchr(lp->line, ' ')) != NULL)) {
				/* The server returned just a simple filename
				 * for this item.  Try to prepend the relative
				 * pathname to it.
				 */
				newl = NULL;
				*cp++ = '\0';
				if (Dynscpy(&newl, lp->line, " ", relpath, "/", cp, 0) != NULL) {
					free(lp->line);
					lp->line = newl;
				}
			}
			(void) AddLine(lines, lp->line);
		}
	}
	DisposeLineListContents(&ll);

	/* Iterate through any subdirectories present. */
	for (fip = fil.first; fip != NULL; fip = fip->next) {
		if (fip->type != 'd')
			continue;
		FTPRemoteRecursiveMListSubdir(cip, parentdir, pdsize, newlen, fip->relname, lines);
	}
	DisposeFileInfoListContents(&fil);

done:
	relpath[pdlen] = '\0';
}	/* FTPRemoteRecursiveMListSubdir */




int
FTPRemoteRecursiveMList(FTPCIPtr cip, const char *const rdir, FTPLineListPtr lines)
{
	FTPFileInfoList fil;
	FTPFileInfoPtr fip;
	int result, cwdresult;
	char rcwd[512];
	char startdir[512];
	size_t sdlen;
	int mls = 1;
	int unlsrc;

	rcwd[0] = '\0';
	InitLineList(lines);

	if (cip->hasMLSD != kCommandAvailable) {
		return (kErrMLSDNotAvailable);
	}

	if ((result = FTPGetCWD(cip, rcwd, sizeof(rcwd))) < 0)
		return (result);

	if (rdir == NULL)
		return (-1);

	if (FTPChdir(cip, rdir) < 0) {
		/* Probably not a directory.  */
		return (cip->errNo = kErrNotADirectory);
	}

	STRNCPY(startdir, rdir);
	sdlen = strlen(startdir);

	/* Paths collected must be relative. */
	if (((result = FTPListToMemory2(cip, "", lines, "-a", 0, &mls)) < 0) || (lines->first == NULL)) {
		DisposeLineListContents(lines);
		goto done;
	}

	/* "MLSD" succeeded */
	unlsrc = UnMlsD(cip, &fil, lines);
	if (unlsrc < 0) {
		/* Return the lines as is -- even though it's invalid. */
		result = kErrInvalidMLSTResponse;
		goto done;
	} else if (unlsrc == 0) {
		/* empty */
		result = kNoErr;
		goto done;
	}
	fip = fil.first;

	/* Iterate through any subdirectories present. */
	for (fip = fil.first; fip != NULL; fip = fip->next) {
		if (fip->type != 'd')
			continue;
		FTPRemoteRecursiveMListSubdir(cip, startdir, sizeof(startdir), sdlen, fip->relname, lines);
	}
	DisposeFileInfoListContents(&fil);

done:
	/* Ready to wrap things up -- revert to the state we had been in. */
	if (rcwd[0] != '\0') {
		if ((cwdresult = FTPChdir(cip, rcwd)) < 0) {
			if (result == kNoErr)
				result = cwdresult;
		}
	}

	return (result);
}	/* FTPRemoteRecursiveMList */




static void
Usage(void)
{
	FILE *fp;

	fp = OpenPager();
	(void) fprintf(fp, "NcFTPLs %.5s\n\n", gVersion + 11);
	(void) fprintf(fp, "Usages:\n");
	(void) fprintf(fp, "  ncftpls [FTP flags] [-x \"ls flags\"] ftp://url.style.host/path/name/\n");
	(void) fprintf(fp, "\nls Flags:\n\
  -m     Use machine readable (MLSD) list format, if the server supports it.\n\
  -1     Most basic format, one item per line.\n\
  -l     Long list format.\n\
  -C     Columnized list format (default).\n\
  -R     Long list format, recurse subdirectories if server allows it.\n\
  -g     Recursive and print one path per line; like \"/usr/bin/find . -print\"\n\
  -gg    As above, but append a \"/\" character to directory pathnames.\n\
  -a     Show all files, if server allows it (as in \"/bin/ls -a\").\n\
  -i XX  Filter the listing (if server supports it) with the wildcard XX.\n\
  -x XX  List command flags to use on the remote server.\n");
	(void) fprintf(fp, "\nFTP Flags:\n\
  -u XX  Use username XX instead of anonymous.\n\
  -p XX  Use password XX with the username.\n\
  -P XX  Use port number XX instead of the default FTP service port (21).\n\
  -j XX  Use account XX with the account (deprecated).\n\
  -d XX  Use the file XX for debug logging.\n");
	(void) fprintf(fp, "\
  -t XX  Timeout after XX seconds.\n\
  -f XX  Read the file XX for user and password information.\n\
  -E     Use regular (PORT) data connections.\n\
  -F     Use passive (PASV) data connections (default).\n\
  -K     Show disk usage by attempting SITE DF.\n");
	(void) fprintf(fp, "\
  -o XX  Specify miscellaneous options (see documentation).\n\
  -W XX  Send raw FTP command XX after logging in.\n\
  -X XX  Send raw FTP command XX after each listing.\n\
  -Y XX  Send raw FTP command XX before logging out.\n\
  -r XX  Redial XX times until connected.\n");
	(void) fprintf(fp, "\nExamples:\n\
  ncftpls ftp://ftp.freebsd.org/pub/FreeBSD/\n\
  ncftpls -1 ftp://ftp.freebsd.org/pub/FreeBSD/\n\
  ncftpls -la -i '*.TXT' ftp://ftp.freebsd.org/pub/FreeBSD/\n\
  ncftpls -m ftp://ftp.ncftp.com/ncftpd/\n\
  ncftpls -x \"-lrt\" ftp://ftp.freebsd.org/pub/FreeBSD/\n");

	(void) fprintf(fp, "%s", "\nNote: The standard specifies that URL pathnames are are relative pathnames.\n  For FTP, this means that URLs specify relative pathnames from the start\n  directory, which for user logins, are typically the user's home directory.\n  If you want to use absolute pathnames, you need to include a literal slash,\n  using the \"%2F\" code for a \"/\" character.  Examples:\n\n");

	(void) fprintf(fp, "%s", "\
  ncftpls -u linus ftp://ftp.kernel.org/%2Fusr/src/\n\
  ncftpls ftp://steve@ftp.apple.com/%2Fetc/\n");

	(void) fprintf(fp, "\nLibrary version: %s.\n", gLibNcFTPVersion + 5);
	(void) fprintf(fp, "\nThis is a freeware program by Mike Gleason (http://www.ncftp.com).\n");
	(void) fprintf(fp, "This was built using LibNcFTP (http://www.ncftp.com/libncftp/).\n");

	ClosePager(fp);
	DisposeWinsock();
	exit(kExitUsage);
}	/* Usage */




static void
Abort(int sigNum)
{
	signal(sigNum, Abort);

	/* Hopefully the I/O operation in progress
	 * will complete, and we'll abort before
	 * it starts a new block.
	 */
	fi.cancelXfer++;

	/* If the user appears to be getting impatient,
	 * restore the default signal handler so the
	 * next ^C abends the program.
	 */
	if (fi.cancelXfer >= 2)
		signal(sigNum, SIG_DFL);
}	/* Abort */




main_void_return_t
main(int argc, char **argv)
{
	int result, c;
	FTPConnectionInfo savedfi;
	FTPConnectionInfo startfi;
	ExitStatus es;
	char url[256];
	char urlfile[128];
	char rootcwd[256];
	char curcwd[256];
	int i;
	FTPLineList cdlist, dirlisting;
	FTPLinePtr lp, lp2;
	int rc;
	int ndirs;
	int dfmode = 0;
	int tryMLSD = 0;
	const char *pattern = "";
	const char *patterntouse = NULL;
	const char *userflags = NULL, *lsflagstouse;
	int lslong = 0, lsrecursive = 0, lsall = 0, lsone = 0, lscolumned = -1, lslikefind = 0, lsF = 0;
	char lsflags[32];
	char *curdir, *coloncp, *slashcp, *lslinecp;
	const char *tailcp;
	MLstItem mli;
	ResponsePtr rp;
	FILE *ofp;
	char precmd[320], postcmd[320], perfilecmd[320];
	GetoptInfo opt;

	InitWinsock();
#if (defined(SOCKS)) && (SOCKS >= 5)
	SOCKSinit(argv[0]);
#endif	/* SOCKS */
#ifdef SIGPOLL
	NcSignal(SIGPOLL, (FTPSigProc) SIG_IGN);
#endif
	result = FTPInitLibrary(&gLib);
	if (result < 0) {
		(void) fprintf(stderr, "ncftpls: init library error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(kExitInitLibraryFailed);
	}
	result = FTPInitConnectionInfo(&gLib, &fi, kDefaultFTPBufSize);
	if (result < 0) {
		(void) fprintf(stderr, "ncftpls: init connection info error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(kExitInitConnInfoFailed);
	}

	InitUserInfo();
	fi.dataPortMode = kFallBackToSendPortMode;
	LoadFirewallPrefs(0);
	if (gFwDataPortMode >= 0)
		fi.dataPortMode = gFwDataPortMode;
	fi.debugLog = NULL;
	fi.errLog = stderr;
	fi.xferTimeout = 60 * 60;
	fi.connTimeout = 30;
	fi.ctrlTimeout = 135;
	(void) STRNCPY(fi.user, "anonymous");
	fi.host[0] = '\0';
	urlfile[0] = '\0';
	InitLineList(&cdlist);
	precmd[0] = '\0';
	postcmd[0] = '\0';
	perfilecmd[0] = '\0';
	es = kExitSuccess;

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "1lRx:P:u:j:p:h:e:d:t:r:f:o:EFKW:X:Y:maCi:g")) > 0) switch(c) {
		case 'P':
			fi.port = atoi(opt.arg);	
			break;
		case 'u':
			(void) STRNCPY(fi.user, opt.arg);
			memset(opt.arg, 0, strlen(fi.user));
			opt.arg[0] = '?';
			break;
		case 'j':
			(void) STRNCPY(fi.acct, opt.arg);
			memset(opt.arg, 0, strlen(fi.acct));
			opt.arg[0] = '?';
			break;
		case 'p':
			(void) STRNCPY(fi.pass, opt.arg);	/* Don't recommend doing this! */
			if (fi.pass[0] == '\0')
				fi.passIsEmpty = 1;
			memset(opt.arg, 0, strlen(fi.pass));
			opt.arg[0] = '?';
			break;
		case 'h':
			/* Doesn't make sense really, but implement for
			 * compatibility with the other utilities.
			 */
			(void) STRNCPY(fi.host, opt.arg);
			memset(opt.arg, 0, strlen(fi.user));
			opt.arg[0] = '?';
			break;
		case 'e':
			if (strcmp(opt.arg, "stdout") == 0)
				fi.errLog = stdout;
			else if (opt.arg[0] == '-')
				fi.errLog = stdout;
			else if (strcmp(opt.arg, "stderr") == 0)
				fi.errLog = stderr;
			else
				fi.errLog = fopen(opt.arg, "a");
			break;
		case 'd':
			if (strcmp(opt.arg, "stdout") == 0)
				fi.debugLog = stdout;
			else if (opt.arg[0] == '-')
				fi.debugLog = stdout;
			else if (strcmp(opt.arg, "stderr") == 0)
				fi.debugLog = stderr;
			else
				fi.debugLog = fopen(opt.arg, "a");
			break;
		case 't':
			SetTimeouts(&fi, opt.arg);
			break;
		case 'r':
			SetRedial(&fi, opt.arg);
			break;
		case 'f':
			ReadConfigFile(opt.arg, &fi);
			break;
		case 'o':
			fi.manualOverrideFeatures = opt.arg;
			break;
		case 'E':
			fi.dataPortMode = kSendPortMode;
			break;
		case 'F':
			fi.dataPortMode = kPassiveMode;
			break;
		case 'i':
			pattern = opt.arg;
			break;
		case 'a':
			lsall = 1;
			break;
		case 'l':
			lslong = 1;
			break;
		case '1':
			lsone = 1;
			break;
		case 'C':
			lscolumned = 1;
			break;
		case 'R':
			lsrecursive = 1;
			break;
		case 'g':
			lslikefind++;
			lsone = 1;
			lsrecursive = 1;
			lslong = lscolumned = 0;
			if (lslikefind > 1)
				lsF = 1;
			break;
		case 'x':
			userflags = opt.arg;
			break;
		case 'm':
			tryMLSD = 1;
			break;
		case 'K':
			dfmode++;
			break;
		case 'W':
			STRNCAT(precmd, opt.arg);
			STRNCAT(precmd, "\n");
			break;
		case 'X':
			STRNCAT(perfilecmd, opt.arg);
			STRNCAT(perfilecmd, "\n");
			break;
		case 'Y':
			STRNCAT(postcmd, opt.arg);
			STRNCAT(postcmd, "\n");
			break;
		default:
			Usage();
	}
	if (opt.ind > argc - 1)
		Usage();

	STRNCPY(lsflags, "-CF");
	if (lslong != 0) {
		STRNCPY(lsflags, "-l");
	} else if (lsone != 0) {
		STRNCPY(lsflags, "-1");
	}
	if (lsrecursive != 0) {
		if ((lsone == 0) && (lscolumned != 1)) {
			/* Maintain backwards compatibility */
			STRNCPY(lsflags, "-lR");
		} else {
			STRNCAT(lsflags, "R");
		}
	}
	if (lsF != 0) {
		STRNCAT(lsflags, "F");
	}
	if (lsall != 0) {
		STRNCAT(lsflags, "a");
	}
	lsflagstouse = lsflags;
	if (userflags != NULL)
		lsflagstouse = userflags;

	InitOurDirectory();

	startfi = fi;
	memset(&savedfi, 0, sizeof(savedfi));
	ndirs = argc - opt.ind;
	for (i=opt.ind; i<argc; i++) {
		fi = startfi;
		(void) STRNCPY(url, argv[i]);
		patterntouse = pattern;
		rc = FTPDecodeURL(&fi, url, &cdlist, urlfile, sizeof(urlfile), (int *) 0, NULL);
		(void) STRNCPY(url, argv[i]);
		if (rc == kMalformedURL) {
			(void) fprintf(stderr, "Malformed URL: %s\n", url);
			DisposeWinsock();
			exit(kExitMalformedURL);
		} else if (rc == kNotURL) {
			(void) fprintf(stderr, "Not a URL: %s\n", url);
			DisposeWinsock();
			exit(kExitMalformedURL);
		} else if (urlfile[0] != '\0') {
			patterntouse = urlfile;
		}
		
		if ((strcmp(fi.host, savedfi.host) == 0) && (strcmp(fi.user, savedfi.user) == 0)) {
			fi = savedfi;
			
			/* This host is currently open, so keep using it. */
			if (FTPChdir(&fi, rootcwd) < 0) {
				FTPPerror(&fi, fi.errNo, kErrCWDFailed, "ncftpls: Could not chdir to", rootcwd);
				es = kExitChdirFailed;
				DisposeWinsock();
				exit((int) es);
			}
		} else {
			if (savedfi.connected != 0) {
				(void) AdditionalCmd(&fi, postcmd, NULL);

				(void) FTPCloseHost(&savedfi);
			}
			memset(&savedfi, 0, sizeof(savedfi));
			
			if (strcmp(fi.user, "anonymous") && strcmp(fi.user, "ftp")) {
				if ((fi.pass[0] == '\0') && (fi.passIsEmpty == 0)) {
					(void) gl_getpass("Password: ", fi.pass, sizeof(fi.pass));
				}
			}
			
			if (MayUseFirewall(fi.host, gFirewallType, gFirewallExceptionList) != 0) {
				fi.firewallType = gFirewallType; 
				(void) STRNCPY(fi.firewallHost, gFirewallHost);
				(void) STRNCPY(fi.firewallUser, gFirewallUser);
				(void) STRNCPY(fi.firewallPass, gFirewallPass);
				fi.firewallPort = gFirewallPort;
			}
			
			es = kExitOpenTimedOut;
			if ((result = FTPOpenHost(&fi)) < 0) {
				(void) fprintf(stderr, "ncftpls: cannot open %s: %s.\n", fi.host, FTPStrError(result));
				es = kExitOpenFailed;
				DisposeWinsock();
				exit((int) es);
			}

			if (fi.hasCLNT != kCommandNotAvailable)
				(void) FTPCmd(&fi, "CLNT NcFTPLs %.5s %s", gVersion + 11, gOS);

			(void) AdditionalCmd(&fi, precmd, NULL);
			
			if (FTPGetCWD(&fi, rootcwd, sizeof(rootcwd)) < 0) {
				FTPPerror(&fi, fi.errNo, kErrPWDFailed, "ncftpls", "could not get current remote working directory");
				es = kExitChdirFailed;
				DisposeWinsock();
				exit((int) es);
			}
		}
		
		es = kExitChdirTimedOut;
		if ((FTPChdirList(&fi, &cdlist, NULL, 0, (kChdirFullPath|kChdirOneSubdirAtATime))) != 0) {
			FTPPerror(&fi, fi.errNo, kErrCWDFailed, "ncftpls: Could not change directory", NULL);
			es = kExitChdirFailed;
			DisposeWinsock();
			exit((int) es);
		}
		
		if (ndirs > 1) {
			fprintf(stdout, "%s%s\n\n",
				(i > opt.ind) ? "\n\n\n" : "", url);
		}
		fflush(stdout);
	
		if (dfmode != 0) {
			if (FTPGetCWD(&fi, curcwd, sizeof(curcwd)) < 0) {
				FTPPerror(&fi, fi.errNo, kErrPWDFailed, "ncftpls", "could not get current remote working directory from remote host");
				es = kExitChdirFailed;
				DisposeWinsock();
				exit((int) es);
			}

			rp = InitResponse();
			if (rp != NULL) {
				result = RCmd(&fi, rp, "SITE DF %s", curcwd);
				ofp = fi.debugLog;
				fi.debugLog = stdout;
				PrintResponse(&fi, &rp->msg);
				fi.debugLog = ofp;
				DoneWithResponse(&fi, rp);
			}
			if (dfmode == 1)
				continue;	/* Don't bother with the listing unless -KK. */
		}

		es = kExitXferTimedOut;
		(void) signal(SIGINT, Abort);

		PrintF(&fi, "ncftpls DIRLIST: directory, file, or wildcard = \"%s\";  lsflags = \"%s\";  tryMLS = %d.\n", patterntouse, lsflagstouse, tryMLSD);
		if (
			(lsrecursive && tryMLSD && FTPRemoteRecursiveMList(&fi, patterntouse, &dirlisting) >= 0) ||
			(FTPListToMemory2(&fi, patterntouse, &dirlisting, lsflagstouse, /* allow blank lines from server? yes */ 1, &tryMLSD) >= 0)
		) {
			es = kExitSuccess;
			(void) AdditionalCmd(&fi, perfilecmd, curcwd);
			savedfi = fi;

			if (lslikefind > 0) {
				curdir = NULL;
				for (lp = dirlisting.first; lp != NULL; ) {
					lp2 = lp;
					lp = lp->next;
					tailcp = "";
					if (tryMLSD != 0) {
						if (UnMlsT(&fi, lp2->line, &mli) < 0)
							continue;
						if (mli.ftype == 'd') {
							tailcp = "/";
						} else {
							tailcp = "";
						}
						lslinecp = strchr(lp2->line, ' ');
						if ((lslinecp != NULL) && (lslinecp != lp2->line) && (lslinecp[-1] == ';')) {
							/* Valid MLSD format; skip ahead one byte to the pathname. */
							lslinecp++;
						} else {
							lslinecp = lp2->line;
						}
					} else {
						lslinecp = lp2->line;
					}
					if (lslinecp != NULL) {
						if ((lslinecp[0] == '.') && ((lslinecp[1] == '/') || (lslinecp[1] == '\\')))
							lslinecp += 2;
						coloncp = strrchr(lslinecp, ':');
						if ((coloncp != NULL) && (coloncp[1] == '\0')) {
							*coloncp = '\0';
							curdir = lslinecp;
							if (strcmp(curdir, ".") == 0)
								curdir = NULL;
						} else if ((tryMLSD == 0) && (coloncp != NULL) && (strlen(coloncp) > 4) && (strncmp(coloncp + 3, "ermission denied", strlen("ermission denied")) == 0)) {
							continue;
						} else if ((lslinecp[0] != '\0') && (lslinecp[0] != '\n') && (lslinecp[0] != '\r')) {
							slashcp = strchr(lslinecp, '/');
							if (slashcp == NULL)
								slashcp = strchr(lslinecp, '\\');
							if ((slashcp == NULL) || (slashcp[1] == '\0') || (slashcp[1] == '\n') || (slashcp[1] == '\r')) {
								if (curdir == NULL) {
									/* Could print "./" then file */
									(void) fprintf(stdout, "%s%s\n", lslinecp, tailcp);
								} else {
									(void) fprintf(stdout, "%s/%s%s\n", curdir, lslinecp, tailcp);
								}
							} else {
								(void) fprintf(stdout, "%s%s\n", lslinecp, tailcp);
							}
						}
					}
				}
			} else {
				/* Print the completed listing. */
				for (lp = dirlisting.first; lp != NULL; ) {
					lp2 = lp;
					lp = lp->next;
					if (lp2->line != NULL) {
						(void) fprintf(stdout, "%s\n", lp2->line);
					}
				}
			}
		} else {
			es = kExitXferFailed;
		}
		(void) signal(SIGINT, SIG_DFL);
	}

	(void) AdditionalCmd(&fi, postcmd, NULL);
	
	(void) FTPCloseHost(&fi);

	DisposeWinsock();
	exit((int) es);
}	/* main */
