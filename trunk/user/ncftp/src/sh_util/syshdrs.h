/* syshdrs.h
 *
 * Copyright (c) 1992-1999 by Mike Gleason.
 * All rights reserved.
 * 
 */

#if defined(WIN32) || defined(_WINDOWS)
#	pragma once
#	pragma warning(disable : 4127)	// warning C4127: conditional expression is constant
#	pragma warning(disable : 4100)	// warning C4100: 'lpReserved' : unreferenced formal parameter
#	pragma warning(disable : 4514)	// warning C4514: unreferenced inline function has been removed
#	pragma warning(disable : 4115)	// warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#	pragma warning(disable : 4201)	// warning C4201: nonstandard extension used : nameless struct/union
#	pragma warning(disable : 4214)	// warning C4214: nonstandard extension used : bit field types other than int
#	pragma warning(disable : 4115)	// warning C4115: 'IRpcStubBuffer' : named type definition in parentheses
#	pragma warning(disable : 4711)	// warning C4711: function selected for automatic inline expansion
#	define SELECT_TYPE_ARG1 int
#	define SELECT_TYPE_ARG234 (fd_set *)
#	define SELECT_TYPE_ARG5 (struct timeval *)
#	define STDC_HEADERS 1
#	define HAVE_GETHOSTNAME 1
#	define HAVE_MKTIME 1
#	define HAVE_SOCKET 1
#	define HAVE_STRSTR 1
#	define HAVE_MEMMOVE 1
#	define HAVE_LONG_FILE_NAMES 1
#	ifndef WINVER
#		define WINVER 0x0400
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#	endif
#	include <windows.h>		/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	include <shlobj.h>
#	include <tchar.h>
#	include <process.h>
#	include <commctrl.h>
#	include <io.h>
#	include <direct.h>
#	include <share.h>
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <stdarg.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <time.h>
#	include <fcntl.h>
#	include <signal.h>
#	include <assert.h>
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define sleep WinSleep
#	ifndef S_ISREG
#		define S_ISREG(m)      (((m) & _S_IFMT) == _S_IFREG)
#		define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
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
#		define chdir _chdir
#	endif
#	ifndef unlink
#		define unlink remove
#	endif
#	define HAVE_GETHOSTNAME 1
#	define HAVE_MKTIME 1
#	define HAVE_SETVBUF 1
#	define HAVE_SNPRINTF 1
#	define HAVE_VSNPRINTF 1
#	define HAVE_STRSTR 1
#	ifndef vsnprintf
#		define vsnprintf _vsnprintf
#	endif
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#	define uid_t int
#	define NO_SIGNALS 1
#	define USE_SIO 1
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
#	define main_void_return_t void
#	/* define alarm_time_t unsigned int */ /* leave undefined */
#	define gethost_addrptr_t const char *
#	define gethostname_size_t int
#	define listen_backlog_t int
#	define read_return_t int
#	define read_size_t unsigned int
#	define sockaddr_size_t int
#	define sockopt_size_t int
#	define write_return_t int
#	define write_size_t unsigned int
#	ifndef __T
#		define __T(x)      x
#	endif
#else	/* UNIX */
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
#	include <sys/socket.h>
#	include <sys/time.h>
#	include <sys/wait.h>
#	ifdef CAN_USE_SYS_SELECT_H
#		include <sys/select.h>
#	endif
#	if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
#		include <sys/utsname.h>
#	endif
#	include <netinet/in.h>
#	include <arpa/inet.h>
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
#	include <assert.h>
#	include <time.h>
#	include <pwd.h>
#	include <fcntl.h>
#	include <dirent.h>
#	ifdef HAVE_LOCALE_H
#		include <locale.h>
#	endif
#	ifdef NEED_GETOPT_H
#		include <getopt.h>
#	elif defined(NEED_GETOPT_EXTERN_DECLS)
		extern int optind, opterr, optopt;
		extern char *optarg;
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
#endif	/* UNIX */

#ifndef STDIN_FILENO
#	define STDIN_FILENO    0
#	define STDOUT_FILENO   1
#	define STDERR_FILENO   2
#endif

#define NDEBUG 1			/* For assertions. */

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

#if defined(HAVE_LONG_LONG) && defined(HAVE_LSEEK64)
#	define Lseek(a,b,c) lseek64(a, (longest_int) b, c)
#elif defined(HAVE_LONG_LONG) && defined(HAVE_LLSEEK)
#	if 1
#		if defined(LINUX) && (LINUX <= 23000)
#			define Lseek(a,b,c) lseek(a, (off_t) b, c)
#		else
#			define Lseek(a,b,c) llseek(a, (longest_int) b, c)
#		endif
#	else
#		define Lseek(a,b,c) lseek(a, (off_t) b, c)
#	endif
#else
#	define Lseek(a,b,c) lseek(a, (off_t) b, c)
#endif

#if (defined(SOCKS)) && (SOCKS >= 5)
#	ifdef HAVE_SOCKS_H
#		ifdef HAVE_SOCKS5P_H
#			define INCLUDE_PROTOTYPES 1
#		endif
#		include <socks.h>
#	endif
#endif	/* SOCKS */

#include <Strn.h>			/* Library header. */
#include <sio.h>			/* Library header. */
#include <ncftp.h>			/* Library header. */
