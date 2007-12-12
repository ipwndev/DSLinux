/* ncftpget.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 * A non-interactive utility to grab files from a remote FTP server.
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

static void
Usage(void)
{
	FILE *fp;

	fp = OpenPager();
	(void) fprintf(fp, "NcFTPGet %.5s\n\n", gVersion + 11);
	(void) fprintf(fp, "Usages:\n");
	(void) fprintf(fp, "  ncftpget [flags] remote-host local-dir remote-path-names...      (mode 1)\n");
	(void) fprintf(fp, "  ncftpget -f login.cfg [flags] local-dir remote-path-names...     (mode 2)\n");
	(void) fprintf(fp, "  ncftpget [flags] ftp://url.style.host/path/name                  (mode 3)\n");
	(void) fprintf(fp, "  ncftpget -c [flags] remote-host remote-path-name > stdout        (mode 4)\n");
	(void) fprintf(fp, "  ncftpget -C [flags] remote-host remote-path-name local-path-name (mode 5)\n");
	(void) fprintf(fp, "  ncftpget -c [flags] ftp://url.style.host/path/name > stdout      (mode 6)\n");
	(void) fprintf(fp, "\nFlags:\n\
  -u XX  Use username XX instead of anonymous.\n\
  -p XX  Use password XX with the username.\n\
  -P XX  Use port number XX instead of the default FTP service port (21).\n\
  -d XX  Use the file XX for debug logging.\n\
  -a     Use ASCII transfer type instead of binary.\n");
	(void) fprintf(fp, "\
  -t XX  Timeout after XX seconds.\n\
  -v/-V  Do (do not) use progress meters.\n\
  -f XX  Read the file XX for host, user, and password information.\n\
  -h XX  Connect to host XX.  Useful for overriding host in -f config.file.\n\
  -c     Read from remote host and write locally to stdout.\n\
  -C     Read from remote host and write locally to specified file.\n\
  -A     Append to local files, instead of overwriting them.\n");
	(void) fprintf(fp, "\
  -z/-Z  Do (do not) try to resume downloads (default: -z).\n\
  -E     Use regular (PORT) data connections.\n\
  -F     Use passive (PASV) data connections (default).\n\
  -DD    Delete remote file after successfully downloading it.\n\
  -b     Run in background (submit job to \"ncftpbatch\" and run).\n\
  -bb    Same as \"-b\" but queue only (do not run \"ncftpbatch\").\n");
	(void) fprintf(fp, "\
  -B XX  Try setting the SO_RCVBUF size to XX.\n\
  -r XX  Redial XX times until connected.\n\
  -o XX  Specify miscellaneous options (see documentation).\n\
  -W XX  Send raw FTP command XX after logging in.\n\
  -X XX  Send raw FTP command XX after each file transferred.\n\
  -Y XX  Send raw FTP command XX before logging out.\n\
  -R     Recursive mode; copy whole directory trees.\n\
  -T     Do not try to use TAR mode with Recursive mode.\n");
	(void) fprintf(fp, "\nExamples:\n\
  ncftpget ftp.freebsd.org . /pub/FreeBSD/README.TXT /pub/FreeBSD/index.html\n\
  ncftpget ftp.gnu.org /tmp '/pub/gnu/README.*'\n\
  ncftpget ftp://ftp.freebsd.org/pub/FreeBSD/README.TXT\n\
  ncftpget -R ftp.ncftp.com /tmp /ncftp  (ncftp is a directory)\n\
  ncftpget -u gleason -p my.password Bozo.probe.net . '/home/mjg/.*rc'\n\
  ncftpget -u gleason Bozo.probe.net . /home/mjg/foo.txt  (prompt for password)\n\
  ncftpget -f Bozo.cfg '/home/mjg/.*rc'\n\
  ncftpget -c ftp.freebsd.org /pub/FreeBSD/README.TXT | /usr/bin/more\n\
  ncftpget -c ftp://ftp.freebsd.org/pub/FreeBSD/README.TXT | /usr/bin/more\n\
  ncftpget -a -d /tmp/debug.log -t 60 ftp.wustl.edu . '/pub/README*'\n");

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




static int 
Copy(FTPCIPtr cip, char *dstdir, const char **files, int rflag, int xtype, int resumeflag, int appendflag, int deleteflag, int tarflag, const char *const perfilecmd)
{
	int i;
	int result;
	const char *file;
	int rc = 0;

	for (i=0; ; i++) {
		file = files[i];
		if (file == NULL)
			break;
		result = FTPGetFiles3(cip, file, dstdir, rflag, kGlobYes, xtype, resumeflag, appendflag, deleteflag, tarflag, kNoFTPConfirmResumeDownloadProc, 0);
		if (result != 0) {
			FTPPerror(cip, result, kErrCouldNotStartDataTransfer, "ncftpget", file);
			if (result != kErrLocalSameAsRemote) {
				/* Display the warning, but don't consider it an error. */
				rc = result;
			}
		} else {
			(void) AdditionalCmd(cip, perfilecmd, file);
		}
	}
	return (rc);
}	/* Copy */




main_void_return_t
main(int argc, char **argv)
{
	int result, c;
	int rflag = 0;
	int xtype = kTypeBinary;
	int appendflag = kAppendNo;
	int resumeflag = kResumeYes;
	int deleteflag = kDeleteNo;
	int tarflag = kTarYes;
	int progmeters;
	char *dstdir = NULL;
	char *dstlfile = NULL;
	const char **flist;
	ExitStatus es;
	char url[512];
	char urlfile[256];
	char urldir[512];
	int urlxtype;
	FTPLineList cdlist;
	FTPLinePtr lp;
	int rc;
	int nD = 0;
	int batchmode = 0;
	int spooled = 0;
	int ftpcat = 0;
	int i;
	char *urlfilep;
	const char *urldirp;
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
		(void) fprintf(stderr, "ncftpget: init library error %d (%s).\n", result, FTPStrError(result));
		exit(kExitInitLibraryFailed);
	}
	result = FTPInitConnectionInfo(&gLib, &fi, kDefaultFTPBufSize);
	if (result < 0) {
		(void) fprintf(stderr, "ncftpget: init connection info error %d (%s).\n", result, FTPStrError(result));
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
	progmeters = GetDefaultProgressMeterSetting();
	urlfile[0] = '\0';
	InitLineList(&cdlist);
	precmd[0] = '\0';
	postcmd[0] = '\0';
	perfilecmd[0] = '\0';

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:t:aRTr:vVf:ADzZEFbcCB:W:X:Y:")) > 0) {
		if (c == 'b') {
			batchmode++;
		}
	}

	if (batchmode > 0) {
		GetoptReset(&opt);
		while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:U:t:mar:RvVf:o:AT:S:EFcCyZzDbB:W:X:Y:")) > 0) switch(c) {
			case 'v': case 'V': case 'A': case 'B': case 'T':
			case 'd': case 'e': case 't': case 'r': case 'c':
			case 'z': case 'Z': case 'C':
				(void) fprintf(stderr, "The \"-%c\" option is not valid when used with conjunction with \"-%c\".\n", c, 'b');
				exit(kExitUsage);
				break;
		}
	}

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:t:aRTr:vVf:o:ADzZEFbcCB:W:X:Y:")) > 0) switch(c) {
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
			fi.leavePass = 1;
			if (fi.pass[0] == '\0')
				fi.passIsEmpty = 1;
			memset(opt.arg, 0, strlen(fi.pass));
			opt.arg[0] = '?';
			break;
		case 'h':
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
				fi.errLog = fopen(opt.arg, FOPEN_APPEND_TEXT);
			break;
		case 'D':
			/* Require two -D's in case they typo. */
			nD++;
			break;
		case 'd':
			if (strcmp(opt.arg, "stdout") == 0)
				fi.debugLog = stdout;
			else if (opt.arg[0] == '-')
				fi.debugLog = stdout;
			else if (strcmp(opt.arg, "stderr") == 0)
				fi.debugLog = stderr;
			else
				fi.debugLog = fopen(opt.arg, FOPEN_APPEND_TEXT);
			break;
		case 't':
			SetTimeouts(&fi, opt.arg);
			break;
		case 'a':
			xtype = kTypeAscii;
			break;
		case 'r':
			SetRedial(&fi, opt.arg);
			break;
		case 'R':
			rflag = 1;
			break;
		case 'T':
			tarflag = 0;
			break;
		case 'v':
			progmeters = 1;
			break;
		case 'V':
			progmeters = 0;
			break;
		case 'f':
			ReadConfigFile(opt.arg, &fi);
			break;
		case 'o':
			fi.manualOverrideFeatures = opt.arg;
			break;
		case 'A':
			appendflag = kAppendYes;
			break;
		case 'z':
			resumeflag = kResumeYes;
			break;
		case 'Z':
			resumeflag = kResumeNo;
			break;
		case 'E':
			fi.dataPortMode = kSendPortMode;
			break;
		case 'F':
			fi.dataPortMode = kPassiveMode;
			break;
		case 'b':
			/* handled above */
			break;
		case 'B':
			fi.dataSocketRBufSize = (size_t) atol(opt.arg);	
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
		case 'c':
			ftpcat = 1;
			break;
		case 'C':
			ftpcat = 2;
			break;
		default:
			Usage();
	}
	if (opt.ind > argc - 1)
		Usage();

	if (progmeters != 0)
		fi.progress = PrStatBar;

	if ((rflag != 0) && (ftpcat != 0)) {
		fprintf(stderr, "The -R flag is not supported with -c/-C.\n\n");
		Usage();
	}

	if ((perfilecmd[0] != '\0') && (rflag != 0))
		(void) fprintf(stderr, "Warning: your -X command is only applied once per command-line parameter, and not for each file in the directory.\n");

	if (fi.host[0] == '\0') {
		(void) STRNCPY(url, argv[opt.ind]);
		rc = FTPDecodeURL(&fi, url, &cdlist, urlfile, sizeof(urlfile), (int *) &urlxtype, NULL);
		if (rc == kMalformedURL) {
			(void) fprintf(stderr, "Malformed URL: %s\n", url);
			exit(kExitMalformedURL);
		} else if (rc == kNotURL) {
			/* This is what should happen most of the time. */
			if (ftpcat == 0) {
				if (opt.ind > argc - 3)
					Usage();
				(void) STRNCPY(fi.host, argv[opt.ind]);
				dstdir = StrDup(argv[opt.ind + 1]);
				if (dstdir == NULL) {
					(void) fprintf(stderr, "Out of memory?\n");
					exit(kExitNoMemory);
				}
				StrRemoveTrailingLocalPathDelim(dstdir);
				flist = (const char **) argv + opt.ind + 2;
			} else if (ftpcat == 2) {
				if (opt.ind > argc - 3)
					Usage();
				(void) STRNCPY(fi.host, argv[opt.ind]);
				dstdir = NULL;
				flist = (const char **) argv + opt.ind + 1;
				dstlfile = argv[opt.ind + 2];
			} else {
				if (opt.ind > argc - 2)
					Usage();
				(void) STRNCPY(fi.host, argv[opt.ind]);
				dstdir = NULL;
				flist = (const char **) argv + opt.ind + 1;
			}
		} else {
			/* URL okay */
			flist = NULL;
			if ((urlfile[0] == '\0') && (rflag == 0)) {
				/* It was obviously a directory, and they didn't say -R. */
				(void) fprintf(stderr, "ncftpget: Use -R if you want the whole directory tree.\n");
				es = kExitUsage;
				exit((int) es);
			}

			if (ftpcat == 2) {
				if (opt.ind > argc - 2)
					Usage();
				dstlfile = argv[opt.ind + 1];
			}

			/* Allow "-a" flag to use ASCII mode
			 * with the URL, since most people
			 * don't know there is way to specify
			 * ASCII in the URL itself with ";a".
			 */
			if (xtype != kTypeAscii)
				xtype = urlxtype;
		}
	} else {
		/* login.cfg being used, so no remote-host argument. */
		if (ftpcat == 0) {
			if (opt.ind > argc - 2)
				Usage();
			dstdir = StrDup(argv[opt.ind + 0]);
			if (dstdir == NULL) {
				(void) fprintf(stderr, "Out of memory?\n");
				exit(kExitNoMemory);
			}
			StrRemoveTrailingLocalPathDelim(dstdir);
			flist = (const char **) argv + opt.ind + 1;
		} else if (ftpcat == 2) {
			if (opt.ind > argc - 3)
				Usage();
			dstdir = StrDup(argv[opt.ind + 0]);
			if (dstdir == NULL) {
				(void) fprintf(stderr, "Out of memory?\n");
				exit(kExitNoMemory);
			}
			StrRemoveTrailingLocalPathDelim(dstdir);
			flist = (const char **) argv + opt.ind + 1;
			dstlfile = argv[opt.ind + 2];
		} else {
			if (opt.ind > argc - 1)
				Usage();
			dstdir = NULL;
			flist = (const char **) argv + opt.ind + 0;
		}
	}

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

	if (nD >= 2)
		deleteflag = kDeleteYes;

	if (batchmode != 0) {
		if (rflag != 0) {
			(void) fprintf(stderr, "Warning: -R flag may not work reliably for background jobs.\n");
		}
		if (flist == NULL) {
			/* URL mode */

			urldir[0] = '\0';
			for (lp = cdlist.first; lp != NULL; lp = lp->next) {
				if (urldir[0] != '\0')
					STRNCAT(urldir, "/");
				STRNCAT(urldir, lp->line);
			}

			rc = SpoolX(
				(batchmode < 3) ? NULL : stdout,
				NULL,
				"get",
				urlfile, 	/* Remote file */
				urldir,		/* Remote CWD */
				urlfile, 	/* Local file */
				".",		/* Local CWD */
				fi.host,
				fi.ip,
				fi.port,
				fi.user,
				fi.pass,
				fi.acct,
				xtype,
				rflag,
				deleteflag,
				fi.dataPortMode,
				precmd,
				perfilecmd,
				postcmd,
				NULL,
				NULL,
				(time_t) 0,	/* when: now */
				0,
				fi.manualOverrideFeatures,
				0
			);
			if ((rc == 0) && (batchmode < 3)) {
				fprintf(stdout, "  + Spooled; writing locally as %s/%s.\n", ".", urlfile);
				spooled++;
			}
		} else {
			/* List of files specified */
			for (i=0; flist[i] != NULL; i++) {
				STRNCPY(urlfile, flist[i]);
				urlfilep = StrRFindLocalPathDelim(urlfile);
				if (urlfilep == NULL) {
					urldirp = ".";
					urlfilep = urlfile;
				} else {
					urldirp = urlfile;
					*urlfilep++ = '\0';
				}

				rc = SpoolX(
					(batchmode < 3) ? NULL : stdout,
					NULL,
					"get",
					urlfilep, 	/* Remote file */
					urldirp,	/* Remote CWD */
					urlfilep, 	/* Local file */
					dstdir,		/* Local CWD */
					fi.host,
					fi.ip,
					fi.port,
					fi.user,
					fi.pass,
					fi.acct,
					xtype,
					rflag,
					deleteflag,
					fi.dataPortMode,
					precmd,
					perfilecmd,
					postcmd,
					NULL,
					NULL,
					(time_t) 0,	/* when: now */
					0,
					fi.manualOverrideFeatures,
					0
				);
				if ((rc == 0) && (batchmode < 3)) {
					fprintf(stdout, "  + Spooled; writing locally as %s/%s.\n", urldirp, urlfilep);
					spooled++;
				}
			}
		}
		if ((spooled > 0) || (batchmode >= 3)) {
			if (batchmode == 1) {
				RunBatch();
			}
			DisposeWinsock();
			exit(kExitSuccess);
		}
		DisposeWinsock();
		exit(kExitSpoolFailed);
	}
	
	es = kExitOpenTimedOut;
	if ((result = FTPOpenHost(&fi)) < 0) {
		(void) fprintf(stderr, "ncftpget: cannot open %s: %s.\n", fi.host, FTPStrError(result));
		es = kExitOpenFailed;
		DisposeWinsock();
		exit((int) es);
	}
	if (fi.hasCLNT != kCommandNotAvailable)
		(void) FTPCmd(&fi, "CLNT NcFTPGet %.5s %s", gVersion + 11, gOS);

	(void) AdditionalCmd(&fi, precmd, NULL);

	if (flist == NULL) {
		/* URL mode */
		es = kExitChdirTimedOut;
		if ((rc = FTPChdirList(&fi, &cdlist, NULL, 0, (kChdirFullPath|kChdirOneSubdirAtATime))) != 0) {
			FTPPerror(&fi, rc, kErrCWDFailed, "ncftpget: Could not change directory", NULL);
			(void) FTPCloseHost(&fi);
			es = kExitChdirFailed;
			DisposeWinsock();
			exit((int) es);
		}
		
		es = kExitXferTimedOut;
		(void) signal(SIGINT, Abort);
		if (ftpcat == 2) {
			if (FTPGetOneFile3(&fi, urlfile, dstlfile, xtype, (-1), resumeflag, appendflag, deleteflag, kNoFTPConfirmResumeDownloadProc, 0) == kNoErr) {
				es = kExitSuccess;
			} else {
				FTPPerror(&fi, rc, kErrCouldNotStartDataTransfer, "ncftpget", NULL);
				es = kExitXferFailed;
			}
		} else if (ftpcat != 0) {
			if (FTPGetOneFile3(&fi, urlfile, NULL, xtype, STDOUT_FILENO, resumeflag, kAppendNo, deleteflag, kNoFTPConfirmResumeDownloadProc, 0) == kNoErr) {
				es = kExitSuccess;
			} else {
				FTPPerror(&fi, rc, kErrCouldNotStartDataTransfer, "ncftpget", NULL);
				es = kExitXferFailed;
			}
		} else {
			if ((rc = FTPGetFiles3(&fi, urlfile, ".", rflag, kGlobYes, xtype, resumeflag, appendflag, deleteflag, tarflag, kNoFTPConfirmResumeDownloadProc, 0)) < 0) {
				if (rc == kErrLocalSameAsRemote) {
					/* Show the message, but do not err-out. */
					FTPPerror(&fi, rc, kErrCouldNotStartDataTransfer, "ncftpget", NULL);
					es = kExitSuccess;
				} else {
					FTPPerror(&fi, rc, kErrCouldNotStartDataTransfer, "ncftpget", NULL);
					es = kExitXferFailed;
				}
			} else {
				es = kExitSuccess;

				(void) AdditionalCmd(&fi, perfilecmd, urlfile);
			}
		}
	} else {
		es = kExitXferTimedOut;
		(void) signal(SIGINT, Abort);
		if (ftpcat == 2) {
			if (FTPGetOneFile3(&fi, flist[0], dstlfile, xtype, (-1), resumeflag, appendflag, deleteflag, kNoFTPConfirmResumeDownloadProc, 0) == kNoErr)
				es = kExitSuccess;
			else
				es = kExitXferFailed;
		} else if (ftpcat != 0) {
			if (FTPGetOneFile3(&fi, flist[0], NULL, xtype, STDOUT_FILENO, resumeflag, kAppendNo, deleteflag, kNoFTPConfirmResumeDownloadProc, 0) == kNoErr)
				es = kExitSuccess;
			else
				es = kExitXferFailed;
		} else {
			if (Copy(&fi, dstdir, flist, rflag, xtype, resumeflag, appendflag, deleteflag, tarflag, perfilecmd) == 0)
				es = kExitSuccess;
			else
				es = kExitXferFailed;
		}
	}

	(void) AdditionalCmd(&fi, postcmd, NULL);
	
	(void) FTPCloseHost(&fi);
	DisposeWinsock();
	
	exit((int) es);
}	/* main */
