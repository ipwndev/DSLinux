/* spoolutil.c
 *
 * Copyright (c) 1992-2005 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifdef HAVE_LONG_FILE_NAMES

#include "spool.h"
#include "util.h"

int gSpoolSerial = 0;

extern FTPLibraryInfo gLib;
extern char gOurDirectoryPath[];
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
extern char gOurInstallationPath[];
#endif
#ifdef ncftp
extern int gUnprocessedJobs;
#endif

int
MkSpoolDir(char *sdir, size_t size)
{
	struct stat st;
	*sdir = '\0';

	/* Don't create in root directory. */
	if (gOurDirectoryPath[0] != '\0') { 
		(void) OurDirectoryPath(sdir, size, kSpoolDir);
		if ((stat(sdir, &st) < 0) && (MkDirs(sdir, 00700) < 0)) {
			perror(sdir);
			return (-1);
		} else {
			return (0);
		}
	}
	return (-1);
}	/* MkSpoolDir */



void
SpoolName(char *const sp, const size_t size, const int flag, const int serial, time_t when)
{
	char dstr[32];
	struct tm lt;

	if ((when == (time_t) 0) || (when == (time_t) -1))
		(void) time(&when);
	if (Gmtime(when, &lt) == NULL) {
		/* impossible */
		(void) Strncpy(dstr, "20010101-000000", size);
	} else {
		(void) strftime(dstr, sizeof(dstr), "%Y%m%d-%H%M%S", &lt);
	}
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	(void) sprintf(sp, "%c-%s-%08X-%d",
		flag,
		dstr,
		(unsigned int) getpid(),
		serial
	);
#else
#	ifdef HAVE_SNPRINTF
	(void) snprintf(sp, size - 1,
#	else
	(void) sprintf(sp,
#	endif
		"%c-%s-%06d-%d",
		flag,
		dstr,
		(unsigned int) getpid(),
		serial
	);
#endif
}	/* SpoolName */




static int
WriteSpoolLine(FILE *const ofp, const char *const line)
{
	int c;
	const char *cp;

	c = 0;
	for (cp = line; *cp; cp++) {
		c = (int) *cp;
		if ((c == '\n') && (cp[1] != '\0')) {
			if (putc('\\', ofp) == EOF)
				return (-1);
		}
		if (putc(c, ofp) == EOF)
			return (-1);
	}
	if (c != '\n') {
		c = '\n';
		if (putc(c, ofp) == EOF)
			return (-1);
	}
	return (0);
}	/* WriteSpoolLine */



int
SpoolX(
	FILE *const ofp,
	const char *sdir,
	const char *const op,
	const char *const rfile,
	const char *const rdir,
	const char *const lfile,
	const char *const ldir,
	const char *const host,
	const char *const ip,
	const unsigned int port,
	const char *const user,
	const char *const passclear,
	const char *const xacct,
	const int xtype,
	const int recursive,
	const int deleteflag,
	const int passive,
	const char *const preftpcmd,
	const char *const perfileftpcmd,
	const char *const postftpcmd,
	const char *const preshellcmd,
	const char *const postshellcmd,
	const time_t when,
	const unsigned int delaySinceLastFailure,
	const char *const manualOverrideFeatures,
	const int UNUSED(reserved)
	)
{
	char sdir2[256];
	char pass[160];
	char sname[64], sname2[64];
	char spathname[256];
	char spathname2[256];
	char ldir2[256];
	char *ldir3;
	FILE *fp;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else
	mode_t um;
#endif

	LIBNCFTP_USE_VAR(reserved);
	gSpoolSerial++;
	SpoolName(sname2, sizeof(sname2), op[0], gSpoolSerial, when);

	if (ofp == NULL) {
		if (sdir == NULL) {
			if (MkSpoolDir(sdir2, sizeof(sdir2)) < 0)
				return (-1);
			sdir = sdir2;
		}
		Path(spathname2, sizeof(spathname2), sdir, sname2);
		STRNCPY(sname, sname2);
		sname[0] = 'z';
		Path(spathname, sizeof(spathname), sdir, sname);
	}

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	fp = (ofp == NULL) ? fopen(spathname, FOPEN_WRITE_TEXT) : ofp;
#else
	um = umask(077);
	fp = (ofp == NULL) ? fopen(spathname, FOPEN_WRITE_TEXT) : ofp;
	(void) umask(um);
#endif
	if (fp == NULL)
		return (-1);

	if (fprintf(fp, "# This is a NcFTP spool file entry.\n") < 0)
		goto err;
	if ((ofp == NULL) && (fprintf(fp, "# Run the \"ncftpbatch\" program to process the spool directory.\n#\n") < 0))
		goto err;
	if (fprintf(fp, "job-name=%s\n", sname2) < 0)
		goto err;
	if (fprintf(fp, "op=%s\n", op) < 0)
		goto err;
	if ((delaySinceLastFailure != 0) && (fprintf(fp, "delay-since-last-failure=%u\n", delaySinceLastFailure) < 0))
		goto err;
	if (fprintf(fp, "hostname=%s\n", host) < 0)
		goto err;
	if ((ip != NULL) && (ip[0] != '\0') && (fprintf(fp, "host-ip=%s\n", ip) < 0))
		goto err;
	if ((port != 0) && (port != (unsigned int) kDefaultFTPPort) && (fprintf(fp, "port=%u\n", port) < 0))
		goto err;
	if ((user != NULL) && (user[0] != '\0') && (strcmp(user, "anonymous") != 0) && (fprintf(fp, "user=%s\n", user) < 0))
		goto err;
	if ((strcmp(user, "anonymous") != 0) && (passclear != NULL) && (passclear[0] != '\0')) {
		(void) memcpy(pass, kPasswordMagic, kPasswordMagicLen);
		ToBase64(pass + kPasswordMagicLen, passclear, strlen(passclear), 1);
		if (fprintf(fp, "pass=%s\n", pass) < 0)
			goto err;
	} else if ((strcmp(user, "anonymous") == 0) && (gLib.defaultAnonPassword[0] != '\0')) {
		if (fprintf(fp, "pass=%s\n", gLib.defaultAnonPassword) < 0)
			goto err;
	}
	if ((xacct != NULL) && (xacct[0] != '\0') && (fprintf(fp, "acct=%s\n", xacct) < 0))
		goto err;
	if (fprintf(fp, "xtype=%c\n", xtype) < 0)
		goto err;
	if ((recursive != 0) && (fprintf(fp, "recursive=%s\n", YESNO(recursive)) < 0))
		goto err;
	if ((deleteflag != 0) && (fprintf(fp, "delete=%s\n", YESNO(deleteflag)) < 0))
		goto err;
	if (fprintf(fp, "passive=%d\n", passive) < 0)
		goto err;
	if (fprintf(fp, "remote-dir=%s\n", rdir) < 0)
		goto err;
	if ((ldir == NULL) || (ldir[0] == '\0') || (strcmp(ldir, ".") == 0)) {
		/* Use current process' working directory. */
		FTPGetLocalCWD(ldir2, sizeof(ldir2));
		if (fprintf(fp, "local-dir=%s\n", ldir2) < 0)
			goto err;
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	} else if ((ldir[0] != '/') && (ldir[0] != '\\')) {
		FTPGetLocalCWD(ldir2, sizeof(ldir2));
		if (DPathCat(&ldir3, ldir2, ldir, 1) == 0) {
			if (fprintf(fp, "local-dir=%s\n", ldir3) < 0)
				goto err;
			free(ldir3);
		}
#else
	} else if (ldir[0] != '/') {
		FTPGetLocalCWD(ldir2, sizeof(ldir2));
		if (DPathCat(&ldir3, ldir2, ldir, 0) == 0) {
			if (fprintf(fp, "local-dir=%s\n", ldir3) < 0)
				goto err;
			free(ldir3);
		}
#endif
	} else {
		if (fprintf(fp, "local-dir=%s\n", ldir) < 0)
			goto err;
	}
	if (fprintf(fp, "remote-file=%s\n", rfile) < 0)
		goto err;
	if (fprintf(fp, "local-file=%s\n", lfile) < 0)
		goto err;
	if ((manualOverrideFeatures != NULL) && (manualOverrideFeatures[0] != '\0')) {
		if (fprintf(fp, "manual-override-features=") < 0)
			goto err;
		if (WriteSpoolLine(fp, manualOverrideFeatures) < 0)
			goto err;
	}
	if ((preftpcmd != NULL) && (preftpcmd[0] != '\0')) {
		if (fprintf(fp, "pre-ftp-command=") < 0)
			goto err;
		if (WriteSpoolLine(fp, preftpcmd) < 0)
			goto err;
	}
	if ((perfileftpcmd != NULL) && (perfileftpcmd[0] != '\0')) {
		if (fprintf(fp, "per-file-ftp-command=") < 0)
			goto err;
		if (WriteSpoolLine(fp, perfileftpcmd) < 0)
			goto err;
	}
	if ((postftpcmd != NULL) && (postftpcmd[0] != '\0')) {
		if (fprintf(fp, "post-ftp-command=") < 0)
			goto err;
		if (WriteSpoolLine(fp, postftpcmd) < 0)
			goto err;
	}
	if ((preshellcmd != NULL) && (preshellcmd[0] != '\0')) {
		if (fprintf(fp, "pre-shell-command=") < 0)
			goto err;
		if (WriteSpoolLine(fp, preshellcmd) < 0)
			goto err;
	}
	if ((postshellcmd != NULL) && (postshellcmd[0] != '\0')) {
		if (fprintf(fp, "post-shell-command=") < 0)
			goto err;
		if (WriteSpoolLine(fp, postshellcmd) < 0)
			goto err;
	}

	if ((fp != ofp) && (fclose(fp) < 0))
		goto err2;

	/* Move the spool file into its "live" name. */
	if ((fp != ofp) && (rename(spathname, spathname2) < 0)) {
		perror("rename spoolfile failed");
		goto err3;
	}
#ifdef ncftp
	gUnprocessedJobs++;
#endif
	return (0);

err:
	if (fp != ofp)
		(void) fclose(fp);
err2:
	perror("write to spool file failed");
err3:
	if (fp != ofp)
		(void) unlink(spathname);
	return (-1);
}	/* SpoolX */




void
RunBatch(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	char ncftpbatch[260];
	const char *prog;
	int winExecResult;

	if (gOurInstallationPath[0] == '\0') {
		(void) fprintf(stderr, "Cannot find path to %s.  Please re-run Setup.\n", "ncftpbatch.exe");
		return;
	}
	prog = ncftpbatch;
	OurInstallationPath(ncftpbatch, sizeof(ncftpbatch), "ncftpbatch.exe");
	
	winExecResult = WinExec(prog, SW_SHOWNORMAL);
	if (winExecResult <= 31) switch (winExecResult) {
		case ERROR_BAD_FORMAT:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The .EXE file is invalid");
			return;
		case ERROR_FILE_NOT_FOUND:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The specified file was not found.");
			return;
		case ERROR_PATH_NOT_FOUND:
			fprintf(stderr, "Could not run %s: %s\n", prog, "The specified path was not found.");
			return;
		default:
			fprintf(stderr, "Could not run %s: Unknown error #%d.\n", prog, winExecResult);
			return;
	}
#else
	char *argv[8];
	pid_t pid = 0;
#ifdef BINDIR
	char ncftpbatch[256];

	STRNCPY(ncftpbatch, BINDIR);
	STRNCAT(ncftpbatch, "/");
	STRNCAT(ncftpbatch, "ncftpbatch");
#endif	/* BINDIR */

	pid = vfork();
	if (pid < 0) {
		perror("fork");
	} else if (pid == 0) {
		argv[0] = strdup("ncftpbatch");
		argv[1] = strdup("-d");
		argv[2] = NULL;
#ifdef BINDIR
		(void) execv(ncftpbatch, argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in installed as %s?\n", argv[0], ncftpbatch);
#else	/* BINDIR */
		(void) execvp(argv[0], argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in your $PATH?\n", argv[0]);
#endif	/* BINDIR */
		perror(argv[0]);
		exit(1);
	}

	if (pid > 1) {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif	/* HAVE_WAITPID */
	}
#endif	/* UNIX */
}	/* RunBatch */

#else	/* ! HAVE_LONG_FILE_NAMES */

int
SpoolX(
	FILE *const ofp,
	const char *sdir,
	const char *const op,
	const char *const rfile,
	const char *const rdir,
	const char *const lfile,
	const char *const ldir,
	const char *const host,
	const char *const ip,
	const unsigned int port,
	const char *const user,
	const char *const passclear,
	const char *const xacct,
	const int xtype,
	const int recursive,
	const int deleteflag,
	const int passive,
	const char *const preftpcmd,
	const char *const perfileftpcmd,
	const char *const postftpcmd,
	const char *const preshellcmd,
	const char *const postshellcmd,
	const time_t when,
	const unsigned int delaySinceLastFailure)
{
	return (-1);
}

void
RunBatch(void)
{
	fprintf(stderr, "Background processing not available on this platform.\n");
}	/* RunBatch */

#endif	/* HAVE_LONG_FILE_NAMES */
