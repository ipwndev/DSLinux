/* syshdrs.h
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	pragma once
#	pragma warning(disable : 4127)	// warning C4127: conditional expression is constant
#	pragma warning(disable : 4100)	// warning C4100: 'lpReserved' : unreferenced formal parameter
#	pragma warning(disable : 4514)	// warning C4514: unreferenced inline function has been removed
#	pragma warning(disable : 4115)	// warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#	pragma warning(disable : 4201)	// warning C4201: nonstandard extension used : nameless struct/union
#	pragma warning(disable : 4214)	// warning C4214: nonstandard extension used : bit field types other than int
#	pragma warning(disable : 4115)	// warning C4115: 'IRpcStubBuffer' : named type definition in parentheses
	/* Include "wincfg.h" in place of "config.h" */
#	include "wincfg.h"
#	ifndef WINVER
#		define WINVER 0x0400
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#	endif
#	include <windows.h>		/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	include <shlobj.h>
#	include <io.h>
#	include <conio.h>
#	include <direct.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <stdarg.h>
#	include <time.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/utime.h>
#	include <fcntl.h>
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define sleep WinSleep
#	ifndef mode_t
#		define mode_t int
#	endif
#	ifndef S_ISREG
#		define S_ISREG(m)      (((m) & _S_IFMT) == _S_IFREG)
#		define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
#		define S_ISLNK(m)      (0)
#	endif
#	ifndef S_IFREG
#		define S_IFREG _S_IFREG
#		define S_IFDIR _S_IFDIR
#	endif
#	ifndef open
#		define open _open
#		define write _write
#		define read _read
#		define close _close
#		define lseek _lseek
#		define stat _stat
#		define lstat _stat
#		define fstat _fstat
#		define dup _dup
#		define utime _utime
#		define utimbuf _utimbuf
#	endif
#	ifndef unlink
#		define unlink remove
#	endif
#	ifndef vsnprintf
#		define vsnprintf _vsnprintf
#	endif
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#	ifndef FOPEN_READ_TEXT
#		define FOPEN_READ_TEXT "rt"
#		define FOPEN_WRITE_TEXT "wt"
#		define FOPEN_APPEND_TEXT "at"
#	endif
#	ifndef FOPEN_READ_BINARY
#		define FOPEN_READ_BINARY "rb"
#		define FOPEN_WRITE_BINARY "wb"
#		define FOPEN_APPEND_BINARY "ab"
#	endif
#	define MY_FD_ZERO FD_ZERO
#	define MY_FD_SET(s,set) FD_SET((SOCKET) (s), set)
#	define MY_FD_CLR(s,set) FD_CLR((SOCKET) (s), set)
#	define MY_FD_ISSET FD_ISSET
#	define NO_SIGNALS 1
#	define USE_SIO 1
#else /* ---------------------------- UNIX ---------------------------- */
#	if defined(HAVE_CONFIG_H)
#		include <config.h>
#	endif
#	if defined(AIX) || defined(_AIX) || defined(__HOS_AIX__)
#		define _ALL_SOURCE 1
#	endif
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <sys/wait.h>
#	if !defined(HAVE_GETCWD) && defined(HAVE_GETWD)
#		include <sys/param.h>
#	endif
#	if defined(HAVE_UNAME) && defined(HAVE_SYS_UTSNAME_H)
#		include <sys/utsname.h>
#	endif
#	ifdef HAVE_SYS_SYSTEMINFO_H
#		include <sys/systeminfo.h>
#	endif
#	ifdef HAVE_GNU_LIBC_VERSION_H
#		include <gnu/libc-version.h>
#	endif

#	include <netinet/in_systm.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <arpa/telnet.h>
#	include <netdb.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <signal.h>
#	include <setjmp.h>
#	include <stdarg.h>
#	include <time.h>
#	include <pwd.h>
#	include <dirent.h>
#	include <fcntl.h>

#	ifdef HAVE_NET_ERRNO_H
#		include <net/errno.h>
#	endif
#	ifdef HAVE_ARPA_NAMESER_H
#		include <arpa/nameser.h>
#	endif
#	ifdef HAVE_NSERVE_H
#		ifdef SCO
#			undef MAXDNAME
#		endif
#		include <nserve.h>
#	endif
#	ifdef HAVE_RESOLV_H
#		include <resolv.h>
#	endif

#	ifdef CAN_USE_SYS_SELECT_H
#		include <sys/select.h>
#	endif
#	define MY_FD_ZERO FD_ZERO
#	define MY_FD_SET FD_SET
#	define MY_FD_CLR FD_CLR
#	define MY_FD_ISSET FD_ISSET

#	ifndef SETSOCKOPT_ARG4
#		define SETSOCKOPT_ARG4
#		define GETSOCKOPT_ARG4
#	endif

#	ifdef HAVE_UTIME_H
#		include <utime.h>
#	elif defined(HAVE_SYS_UTIME_H)
#		include <sys/utime.h>
#	else
		struct utimbuf { time_t actime, modtime; };
#	endif

#	ifdef HAVE_GETCWD
#		ifndef HAVE_UNISTD_H
			extern char *getcwd();
#		endif
#	else
#		ifdef HAVE_GETWD
#			include <sys/param.h>
#			ifndef MAXPATHLEN
#				define MAXPATHLEN 1024
#			endif
			extern char *getwd(char *);
#		endif
#	endif

#	ifdef __CYGWIN__
#		ifndef FOPEN_READ_TEXT
#			define FOPEN_READ_TEXT "rt"
#			define FOPEN_WRITE_TEXT "wt"
#			define FOPEN_APPEND_TEXT "at"
#		endif
#		ifndef FOPEN_READ_BINARY
#			define FOPEN_READ_BINARY "rb"
#			define FOPEN_WRITE_BINARY "wb"
#			define FOPEN_APPEND_BINARY "ab"
#		endif
#	else
#		ifndef FOPEN_READ_TEXT
#			define FOPEN_READ_TEXT "r"
#			define FOPEN_WRITE_TEXT "w"
#			define FOPEN_APPEND_TEXT "a"
#		endif
#		ifndef FOPEN_READ_BINARY
#			define FOPEN_READ_BINARY "r"
#			define FOPEN_WRITE_BINARY "w"
#			define FOPEN_APPEND_BINARY "a"
#		endif
#	endif

#	if defined(MACOSX) || defined(BSDOS)
#		undef SIG_DFL
#		undef SIG_IGN
#		undef SIG_ERR
#		define SIG_DFL         (void (*)(int))0
#		define SIG_IGN         (void (*)(int))1
#		define SIG_ERR         (void (*)(int))-1
#	endif
#endif /* ---------------------------- UNIX ---------------------------- */


#if defined(HAVE_LONG_LONG) && defined(HAVE_OPEN64)
#	define Open open64
#else
#	define Open open
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define Stat WinStat64
#	define Lstat WinStat64
#	define Fstat WinFStat64
#elif defined(HAVE_LONG_LONG) && defined(HAVE_STAT64) && defined(HAVE_STRUCT_STAT64)
#	define Stat stat64
#	ifdef HAVE_FSTAT64
#		define Fstat fstat64
#	else
#		define Fstat fstat
#	endif
#	ifdef HAVE_LSTAT64
#		define Lstat lstat64
#	else
#		define Lstat lstat
#	endif
#else
#	define Stat stat
#	define Fstat fstat
#	define Lstat lstat
#endif

#ifndef Lseek
#	if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#		define Lseek(a,b,c) _lseeki64(a, (__int64) b, c)
#	elif defined(HAVE_LONG_LONG) && defined(HAVE_LSEEK64)
#		define Lseek(a,b,c) lseek64(a, (longest_int) b, c)
#	elif defined(HAVE_LONG_LONG) && defined(HAVE_LLSEEK)
#		if defined(LINUX) && (LINUX <= 23000)
#			define Lseek(a,b,c) lseek(a, (off_t) b, c)
#		else
#			define Lseek(a,b,c) llseek(a, (longest_int) b, c)
#		endif
#	else
#		define Lseek(a,b,c) lseek(a, (off_t) b, c)
#	endif
#endif

#if (defined(AIX) && (AIX >= 430))
/* AIX 4.3's sys/socket.h doesn't properly prototype these for C */
extern int naccept(int, struct sockaddr *, socklen_t *);
extern int ngetpeername(int, struct sockaddr *, socklen_t *);
extern int ngetsockname(int, struct sockaddr *, socklen_t *);
extern ssize_t nrecvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
extern ssize_t nrecvmsg(int, struct msghdr *, int);
extern ssize_t nsendmsg(int, const struct msghdr *, int);
#endif


#ifndef IAC

/*
 * Definitions for the TELNET protocol.
 */
#define IAC     255             /* interpret as command: */
#define DONT    254             /* you are not to use option */
#define DO      253             /* please, you use option */
#define WONT    252             /* I won't use option */
#define WILL    251             /* I will use option */
#define SB      250             /* interpret as subnegotiation */
#define GA      249             /* you may reverse the line */
#define EL      248             /* erase the current line */
#define EC      247             /* erase the current character */
#define AYT     246             /* are you there */
#define AO      245             /* abort output--but let prog finish */
#define IP      244             /* interrupt process--permanently */
#define BREAK   243             /* break */
#define DM      242             /* data mark--for connect. cleaning */
#define NOP     241             /* nop */
#define SE      240             /* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */
#define ABORT   238             /* Abort process */
#define SUSP    237             /* Suspend process */
#define xEOF    236             /* End of file: EOF is already used... */

#define SYNCH   242             /* for telfunc calls */
#endif

#if (defined(SOCKS)) && (SOCKS >= 5)
#	ifdef HAVE_SOCKS_H
#		ifdef HAVE_SOCKS5P_H
#			define INCLUDE_PROTOTYPES 1
#		endif
#		include <socks.h>
#	endif
#endif	/* SOCKS */

#if 1 /* %config2% -- set by configure script -- do not modify */
#	ifndef USE_SIO
#		define USE_SIO 1
#	endif
#	ifndef NO_SIGNALS
#		define NO_SIGNALS 1
#	endif
#else
#	ifndef USE_SIO
#		define USE_SIO 0
#	endif
	/* #undef NO_SIGNALS */
#endif

#if USE_SIO
#	include <sio.h>			/* Library header. */
#endif

#include <Strn.h>			/* Library header. */
#include "ncftp.h"			/* Library header. */

#include "util.h"
#include "ftp.h"

/* eof */
