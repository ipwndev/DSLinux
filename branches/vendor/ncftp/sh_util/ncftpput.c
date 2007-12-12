/* ncftpput.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 * A simple, non-interactive utility to send files to a remote FTP server.
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
	(void) fprintf(fp, "NcFTPPut %.5s\n\n", gVersion + 11);
	(void) fprintf(fp, "Usages:\n");
	(void) fprintf(fp, "  ncftpput [flags] remote-host remote-dir local-files...   (mode 1)\n");
	(void) fprintf(fp, "  ncftpput -f login.cfg [flags] remote-dir local-files...  (mode 2)\n");
	(void) fprintf(fp, "  ncftpput -c remote-host remote-path-name < stdin         (mode 3)\n");
	(void) fprintf(fp, "  ncftpput -C remote-host local-path-name remote-path-name (mode 4)\n");
	(void) fprintf(fp, "\nFlags:\n\
  -u XX  Use username XX instead of anonymous.\n\
  -p XX  Use password XX with the username.\n\
  -P XX  Use port number XX instead of the default FTP service port (21).\n\
  -j XX  Use account XX with the account (deprecated).\n\
  -d XX  Use the file XX for debug logging.\n\
  -e XX  Use the file XX for error logging.\n\
  -U XX  Use value XX for the umask.\n\
  -t XX  Timeout after XX seconds.\n");
	(void) fprintf(fp, "\
  -a     Use ASCII transfer type instead of binary.\n\
  -m     Attempt to mkdir the dstdir before copying.\n\
  -v/-V  Do (do not) use progress meters.\n\
  -f XX  Read the file XX for host, user, and password information.\n");
	(void) fprintf(fp, "\
  -h XX  Connect to host XX.  Useful for overriding host in -f config.file.\n\
  -c     Read locally from stdin and write remotely to specified pathname.\n\
  -C     Similar to -c, except a local pathname is specified.\n\
  -A     Append to remote files instead of overwriting them.\n\
  -z/-Z  Do (do not) try to resume uploads (default: -Z).\n\
  -T XX  Upload into temporary files prefixed by XX.\n");
	(void) fprintf(fp, "\
  -S XX  Upload into temporary files suffixed by XX.\n\
  -DD    Delete local file after successfully uploading it.\n\
  -b     Run in background (submit job to \"ncftpbatch\" and run).\n\
  -bb    Same as \"-b\" but queue only (do not run \"ncftpbatch\").\n\
  -E     Use regular (PORT) data connections.\n\
  -F     Use passive (PASV) data connections (default).\n\
  -y     Try using \"SITE UTIME\" to preserve timestamps on remote host.\n");
	(void) fprintf(fp, "\
  -B XX  Try setting the SO_SNDBUF size to XX.\n\
  -r XX  Redial XX times until connected.\n\
  -o XX  Specify miscellaneous options (see documentation).\n\
  -W XX  Send raw FTP command XX after logging in.\n\
  -X XX  Send raw FTP command XX after each file transferred.\n\
  -Y XX  Send raw FTP command XX before logging out.\n\
  -R     Recursive mode; copy whole directory trees.\n");
	(void) fprintf(fp, "\nExamples:\n\
  ncftpput -u gleason -p my.password Elwood.probe.net /home/gleason stuff.txt\n\
  ncftpput -u gleason Elwood.probe.net /home/gleason a.txt (prompt for pass)\n\
  ncftpput -a -u gleason -p my.password -m -U 007 Bozo.probe.net /tmp/tmpdir a.txt\n\
  tar cvf - /home | ncftpput -u operator -c Server.probe.net /backups/monday.tar\n");
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
Copy(FTPCIPtr cip, const char *const dstdir, char **const files, const int rflag, const int xtype, const int appendflag, const char *const tmppfx, const char *const tmpsfx, const int resumeflag, const int deleteflag, const char *const perfilecmd)
{
	int i;
	int result;
	const char *file;
	int rc = 0;

	for (i=0; ; i++) {
		file = files[i];
		if (file == NULL)
			break;
		result = FTPPutFiles3(cip, file, dstdir, rflag,
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			kGlobYes,
#else
			kGlobNo,	/* Shell does the glob for you */
#endif
			xtype, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, kNoFTPConfirmResumeUploadProc, 0);
		if (result != 0) {
			FTPPerror(cip, result, kErrCouldNotStartDataTransfer, "ncftpput", file);
			if (result != kErrRemoteSameAsLocal) {
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
	int appendflag = kAppendNo;
	int deleteflag = kDeleteNo;
	int resumeflag = kResumeNo;
	const char *tmppfx = "";
	const char *tmpsfx = "";
	int xtype = kTypeBinary;
	ExitStatus es;
	int wantMkdir = 0;
	char *Umask = NULL;
	char *dstdir = NULL;
	char *dstfile = NULL;
	char *lfile = NULL;
	char **files = (char **) 0;
	int progmeters;
	int usingcfg = 0;
	int ftpcat = 0;
	int tryUtime = 0;
	int nD = 0;
	int batchmode = 0;
	int spooled = 0;
	int i;
	int ascii = 0;
	char *ufilep;
	const char *udirp;
	char ufile[256];
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
		(void) fprintf(stderr, "ncftpput: init library error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(kExitInitLibraryFailed);
	}
	result = FTPInitConnectionInfo(&gLib, &fi, kDefaultFTPBufSize);
	if (result < 0) {
		(void) fprintf(stderr, "ncftpput: init connection info error %d (%s).\n", result, FTPStrError(result));
		DisposeWinsock();
		exit(kExitInitConnInfoFailed);
	}

	InitUserInfo();
	fi.dataPortMode = kFallBackToSendPortMode;
	LoadFirewallPrefs(0);
	if (gFwDataPortMode >= 0)
		fi.dataPortMode = gFwDataPortMode;
	fi.xferTimeout = 60 * 60;
	fi.connTimeout = 30;
	fi.ctrlTimeout = 135;
	fi.debugLog = NULL;
	fi.errLog = stderr;
	(void) STRNCPY(fi.user, "anonymous");
	progmeters = GetDefaultProgressMeterSetting();
	precmd[0] = '\0';
	postcmd[0] = '\0';
	perfilecmd[0] = '\0';

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:U:t:mar:RvVf:o:AT:S:EFcCyZzDbB:W:X:Y:")) > 0) {
		if (c == 'b') {
			batchmode++;
		}
	}

	if (batchmode > 0) {
		GetoptReset(&opt);
		while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:U:t:mar:RvVf:AT:S:EFcCyZzDbB:W:X:Y:")) > 0) switch(c) {
			case 'v': case 'V': case 'A': case 'B': case 'S':
			case 'T': case 'd': case 'e': case 'U': case 't':
			case 'm': case 'r': case 'c': case 'y': case 'z':
			case 'Z': case 'C':
				(void) fprintf(stderr, "The \"-%c\" option is not valid when used with conjunction with \"-%c\".\n", c, 'b');
				exit(kExitUsage);
				break;
		}
	}

	GetoptReset(&opt);
	while ((c = Getopt(&opt, argc, argv, "P:u:j:p:h:e:d:U:t:mar:RvVf:o:AT:S:EFcCyZzDbB:W:X:Y:")) > 0) switch(c) {
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
		case 'U':
			Umask = opt.arg;
			break;
		case 't':
			SetTimeouts(&fi, opt.arg);
			break;
		case 'm':
			wantMkdir = 1;
			break;
		case 'a':
			xtype = kTypeAscii;	/* Use ascii. */
			ascii++;
			break;
		case 'r':
			SetRedial(&fi, opt.arg);
			break;
		case 'R':
			rflag = 1;
			break;
		case 'v':
			progmeters = 1;
			break;
		case 'V':
			progmeters = 0;
			break;
		case 'f':
			ReadConfigFile(opt.arg, &fi);
			usingcfg = 1;
			break;
		case 'o':
			fi.manualOverrideFeatures = opt.arg;
			break;
		case 'A':
			appendflag = 1;
			break;
		case 'T':
			tmppfx = opt.arg;
			break;
		case 'S':
			tmpsfx = opt.arg;
			break;
		case 'E':
			fi.dataPortMode = kSendPortMode;
			break;
		case 'F':
			fi.dataPortMode = kPassiveMode;
			break;
		case 'c':
			ftpcat = 1;
			break;
		case 'C':
			ftpcat = 2;
			break;
		case 'y':
			tryUtime = 1;
			break;
		case 'z':
			resumeflag = kResumeYes;
			break;
		case 'Z':
			resumeflag = kResumeNo;
			break;
		case 'b':
			/* handled above */
			break;
		case 'B':
			fi.dataSocketSBufSize = (size_t) atol(opt.arg);	
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

	if ((rflag != 0) && (ftpcat != 0)) {
		fprintf(stderr, "The -R flag is not supported with -c/-C.\n\n");
		Usage();
	}

	if ((perfilecmd[0] != '\0') && (rflag != 0))
		(void) fprintf(stderr, "Warning: your -X command is only applied once per command-line parameter, and not for each file in the directory.\n");

	if (usingcfg != 0) {
		if (ftpcat == 0) {
			if (opt.ind > argc - 2)
				Usage();
			dstdir = argv[opt.ind + 0];
			files = argv + opt.ind + 1;
		} else if (ftpcat == 2) {
			if ((opt.ind + 2) > argc) {
				Usage();
			} else if ((opt.ind + 2) == argc) {
				lfile = argv[opt.ind + 0];
				dstfile = argv[opt.ind + 1];
			} else	/* if ((opt.ind + 2) < argc) */ {
				/* host argument at +0 will be overwritten;
				 * the host needs to be in the config file.
				 */
				(void) STRNCPY(fi.host, argv[opt.ind + 0]);
				lfile = argv[opt.ind + 1];
				dstfile = argv[opt.ind + 2];
			}
		} else /* (ftpcat == 1) */ {
			if ((opt.ind + 1) > argc) {
				Usage();
			} else if ((opt.ind + 1) == argc) {
				dstfile = argv[opt.ind + 0];
			} else	/* if ((opt.ind + 1) < argc) */ {
				/* host argument at +0 will be overwritten;
				 * the host needs to be in the config file.
				 */
				(void) STRNCPY(fi.host, argv[opt.ind + 0]);
				dstfile = argv[opt.ind + 1];
			}
		}
	} else {
		if (ftpcat == 0) {
			if (opt.ind > argc - 3)
				Usage();
			(void) STRNCPY(fi.host, argv[opt.ind]);
			dstdir = argv[opt.ind + 1];
			files = argv + opt.ind + 2;
		} else if (ftpcat == 2) {
			if (opt.ind > argc - 3)
				Usage();
			(void) STRNCPY(fi.host, argv[opt.ind]);
			lfile = argv[opt.ind + 1];
			dstfile = argv[opt.ind + 2];
		} else {
			if (opt.ind > argc - 2)
				Usage();
			(void) STRNCPY(fi.host, argv[opt.ind]);
			dstfile = argv[opt.ind + 1];
		}
	}

	if (strcmp(fi.user, "anonymous") && strcmp(fi.user, "ftp")) {
		if ((fi.pass[0] == '\0') && (fi.passIsEmpty == 0)) {
			(void) gl_getpass("Password: ", fi.pass, sizeof(fi.pass));
		}
	}

	if (progmeters != 0)
		fi.progress = PrStatBar;
	if (tryUtime == 0)
		fi.hasSITE_UTIME = 0;
	if (nD >= 2)
		deleteflag = kDeleteYes;
	if (ascii > 1) {
		/* This is used internally for testing.
		 * Valid values as of this release are -1, 0, 1.
		 */
		fi.asciiTranslationMode = ascii - 3;
	}

	if (MayUseFirewall(fi.host, gFirewallType, gFirewallExceptionList) != 0) {
		fi.firewallType = gFirewallType; 
		(void) STRNCPY(fi.firewallHost, gFirewallHost);
		(void) STRNCPY(fi.firewallUser, gFirewallUser);
		(void) STRNCPY(fi.firewallPass, gFirewallPass);
		fi.firewallPort = gFirewallPort;
	}

	if (batchmode != 0) {
		if (rflag != 0) {
			(void) fprintf(stderr, "Warning: -R flag may not work reliably for background jobs.\n");
		}
		/* List of files specified */
		for (i=0; files[i] != NULL; i++) {
			STRNCPY(ufile, files[i]);
			ufilep = StrRFindLocalPathDelim(ufile);
			if (ufilep == NULL) {
				udirp = ".";
				ufilep = ufile;
			} else {
				udirp = ufile;
				*ufilep++ = '\0';
			}

			result = SpoolX(
				(batchmode < 3) ? NULL : stdout,
				NULL,
				"put",
				ufilep, 	/* Remote file */
				dstdir,		/* Remote CWD */
				ufilep, 	/* Local file */
				udirp,	/* Local CWD */
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
			if ((result == 0) && (batchmode < 3)) {
				(void) fprintf(stdout, "  + Spooled; sending remotely as %s/%s.\n", dstdir, ufilep);
				spooled++;
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
		(void) fprintf(stderr, "ncftpput: cannot open %s: %s.\n", fi.host, FTPStrError(result));
		es = kExitOpenFailed;
		DisposeWinsock();
		exit((int) es);
	}

	if (fi.hasCLNT != kCommandNotAvailable)
		(void) FTPCmd(&fi, "CLNT NcFTPPut %.5s %s", gVersion + 11, gOS);

	if (Umask != NULL) {
		result = FTPUmask(&fi, Umask);
		if (result != 0)
			FTPPerror(&fi, result, kErrUmaskFailed, "ncftpput", "could not set umask");
	}

	(void) AdditionalCmd(&fi, precmd, NULL);

	if (dstdir != NULL) {
		es = kExitChdirTimedOut;
		if (wantMkdir != 0) {
			result = FTPChdir3(&fi, dstdir, NULL, 0, kChdirFullPath|kChdirOneSubdirAtATime|kChdirAndMkdir);
			if (result == kErrMKDFailed) {
				FTPPerror(&fi, result, kErrMKDFailed, "ncftpput: Could not create directory", dstdir);
				(void) FTPCloseHost(&fi);
				es = kExitMkdirFailed;
				DisposeWinsock();
				exit((int) es);
			}
		} else {
			result = FTPChdir3(&fi, dstdir, NULL, 0, kChdirFullPath|kChdirOneSubdirAtATime);
		}
	}

	if (result != 0) {
		FTPPerror(&fi, result, kErrCWDFailed, "ncftpput: Could not change to directory", dstdir);
		(void) FTPCloseHost(&fi);
		es = kExitChdirFailed;
		DisposeWinsock();
		exit((int) es);
	}

	if (result >= 0) {
		es = kExitXferTimedOut;
		(void) signal(SIGINT, Abort);
		if (ftpcat == 0) {
			if (Copy(&fi, "", files, rflag, xtype, appendflag, (const char *) tmppfx, (const char *) tmpsfx, resumeflag, deleteflag, perfilecmd) < 0)
				es = kExitXferFailed;
			else
				es = kExitSuccess;
		} else if (ftpcat == 2) {
			result = kNoErr;
			if (wantMkdir)
				result = FTPMkParentDir(&fi, dstfile, 1, NULL);
			if (result != kNoErr) {
				es = kExitMkdirFailed;
			} else if ((result = FTPPutOneFile3(&fi, lfile, dstfile, xtype, -1, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, kNoFTPConfirmResumeUploadProc, 0)) < 0) {
				FTPPerror(&fi, result, kErrCouldNotStartDataTransfer, "ncftpput", dstfile);
				es = kExitXferFailed;
			} else {
				es = kExitSuccess;
				(void) AdditionalCmd(&fi, perfilecmd, argv[opt.ind + 1]);
			}
		} else {
			fi.progress = (FTPProgressMeterProc) 0;
			result = kNoErr;
			if (wantMkdir)
				result = FTPMkParentDir(&fi, dstfile, 1, NULL);
			if (result != kNoErr) {
				es = kExitMkdirFailed;
			} else if (FTPPutOneFile3(&fi, NULL, dstfile, xtype, STDIN_FILENO, appendflag, tmppfx, tmpsfx, resumeflag, deleteflag, kNoFTPConfirmResumeUploadProc, 0) < 0) {
				es = kExitXferFailed;
			} else {
				es = kExitSuccess;
				(void) AdditionalCmd(&fi, perfilecmd, argv[opt.ind + 1]);
			}
		}
	}

	(void) AdditionalCmd(&fi, postcmd, NULL);
	
	(void) FTPCloseHost(&fi);
	DisposeWinsock();
	exit((int) es);
}	/* main */
