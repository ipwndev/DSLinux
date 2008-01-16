/* spool.c
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
#ifdef ncftp
#	include "trace.h"
#endif
#include "util.h"

int gUnprocessedJobs = 0;
int gJobs = 0;
int gHaveSpool = -1;

extern char gOurDirectoryPath[];
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
extern char gOurInstallationPath[];
#endif

void
TruncBatchLog(void)
{
	char f[256];
	struct stat st;
	time_t t;
	int fd;

	if (gOurDirectoryPath[0] != '\0') { 
		time(&t);
		t -= 86400;
		(void) OurDirectoryPath(f, sizeof(f), kSpoolLog);
		if ((stat(f, &st) == 0) && (st.st_mtime < t)) {
			/* Truncate old log file.
			 * Do not remove it, since a process
			 * could still conceivably be going.
			 */
			fd = open(f, O_WRONLY|O_TRUNC, 00600);
			if (fd >= 0)
				close(fd);
		}
	}
}	/* TruncBatchLog */




int
HaveSpool(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	char ncftpbatch[260];

	if (gHaveSpool < 0) {
		gHaveSpool = 0;
		if (gOurInstallationPath[0] != '\0') {
			OurInstallationPath(ncftpbatch, sizeof(ncftpbatch), "ncftpbatch.exe");
			gHaveSpool = (access(ncftpbatch, F_OK) == 0) ? 1 : 0;
		}
	}
#elif defined(BINDIR)
	char ncftpbatch[256];

	if (gHaveSpool < 0) {
		STRNCPY(ncftpbatch, BINDIR);
		STRNCAT(ncftpbatch, "/");
		STRNCAT(ncftpbatch, "ncftpbatch");
		gHaveSpool = (access(ncftpbatch, X_OK) == 0) ? 1 : 0;
	}
#else	/* BINDIR */
	if (gHaveSpool < 0) {
		if (geteuid() == 0) {
			gHaveSpool = (access("/usr/bin/ncftpbatch", X_OK) == 0) ? 1 : 0;
		} else {
			gHaveSpool = (system("ncftpbatch -X") == 0) ? 1 : 0;
		}
	}
#endif /* BINDIR */

	return (gHaveSpool);
}	/* HaveSpool */




int
CanSpool(void)
{
	char sdir[256];

	if (gOurDirectoryPath[0] == '\0') {
		return (-1);
	}
	if (MkSpoolDir(sdir, sizeof(sdir)) < 0)
		return (-1);
	return (0);
}	/* CanSpool */




void
Jobs(void)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	assert(0); // Not supported
#else
	char *argv[8];
	pid_t pid;
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
		argv[1] = strdup("-l");
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
	} else {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif
	}
#endif
}	/* Jobs */




void
RunBatchWithCore(const FTPCIPtr cip)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	RunBatch();
#else	/* UNIX */
	int pfd[2];
	char pfdstr[32];
	char *argv[8];
	pid_t pid = 0;
#ifdef BINDIR
	char ncftpbatch[256];

	STRNCPY(ncftpbatch, BINDIR);
	STRNCAT(ncftpbatch, "/");
	STRNCAT(ncftpbatch, "ncftpbatch");
#endif	/* BINDIR */

	if (pipe(pfd) < 0) {
		perror("pipe");
	}

	(void) sprintf(pfdstr, "%d", pfd[0]);
	pid = vfork();
	if (pid < 0) {
		(void) close(pfd[0]);
		(void) close(pfd[1]);
		perror("fork");
	} else if (pid == 0) {
		(void) close(pfd[1]);	/* Child closes write end. */
		argv[0] = strdup("ncftpbatch");
#ifdef DEBUG_NCFTPBATCH
		argv[1] = strdup("-D");
		argv[2] = strdup("-Z");
		argv[3] = strdup("15");
		argv[4] = strdup("-|");
		argv[5] = strdup(pfdstr);
		argv[6] = NULL;
#else
		argv[1] = strdup("-d");
		argv[2] = strdup("-|");
		argv[3] = strdup(pfdstr);
		argv[4] = NULL;
#endif

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
	(void) close(pfd[0]);	/* Parent closes read end. */
	(void) PWrite(pfd[1], (const char *) cip->lip, sizeof(FTPLibraryInfo));
	(void) PWrite(pfd[1], (const char *) cip, sizeof(FTPConnectionInfo));
	(void) close(pfd[1]);	/* Parent closes read end. */

	/* Close it now, or else this process would send
	 * the server a QUIT message.  This will cause it
	 * to think it already has.
	 */
	FTPCloseControlConnection(cip);

	if (pid > 1) {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif	/* HAVE_WAITPID */
	}
#endif	/* UNIX */
}	/* RunBatchWithCore */



void
RunBatchIfNeeded(const FTPCIPtr cip)
{
	if (gUnprocessedJobs > 0) {
#ifdef ncftp
		Trace(0, "Running ncftp_batch for %d job%s.\n", gUnprocessedJobs, gUnprocessedJobs > 0 ? "s" : "");
		gUnprocessedJobs = 0;
		RunBatchWithCore(cip);
#else
		gUnprocessedJobs = 0;
		RunBatch();
#endif
	}
}	/* RunBatchIfNeeded */

#else	/* ! HAVE_LONG_FILE_NAMES */

int gUnprocessedJobs = 0;
int gJobs = 0;
int gHaveSpool = -1;

void
Jobs(void)
{
	fprintf(stderr, "Background processing not available on this platform.\n");
}

void
RunBatchIfNeeded(const FTPCIPtr cip)
{
}

void
RunBatchWithCore(const FTPCIPtr cip)
{
}

void
TruncBatchLog(void)
{
}

int
HaveSpool(void)
{
	return (0);
}

int
CanSpool(void)
{
	return (-1);
}

#endif	/* HAVE_LONG_FILE_NAMES */
