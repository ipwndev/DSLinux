/* spool.h
 *
 * Copyright (c) 1992-2004 by Mike Gleason.
 * All rights reserved.
 * 
 */

#define kSpoolDir "spool"
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kSpoolLog "log.txt"
#else
#	define kSpoolLog "log"
#endif

/* spool.c */
void TruncBatchLog(void);
int MkSpoolDir(char *, size_t);
void SpoolName(char *const sp, const size_t size, const int flag, const int serial, time_t when);
int CanSpool(void);
int HaveSpool(void);
int SpoolX(
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
	const int reserved);
void RunBatch(void);
void RunBatchWithCore(const FTPCIPtr);
void Jobs(void);
void RunBatchIfNeeded(const FTPCIPtr);
