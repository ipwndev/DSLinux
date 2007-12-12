/* syshdrs.h
 *
 * Copyright (c) 1992-1999 by Mike Gleason.
 * All rights reserved.
 * 
 */

#if defined(HAVE_CONFIG_H)
#	include <config.h>
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	pragma once
#	pragma warning(disable : 4127)	// warning C4127: conditional expression is constant
#	pragma warning(disable : 4100)	// warning C4100: 'lpReserved' : unreferenced formal parameter
#	pragma warning(disable : 4514)	// warning C4514: unreferenced inline function has been removed
#	pragma warning(disable : 4115)	// warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#	pragma warning(disable : 4201)	// warning C4201: nonstandard extension used : nameless struct/union
#	pragma warning(disable : 4214)	// warning C4214: nonstandard extension used : bit field types other than int
#	pragma warning(disable : 4115)	// warning C4115: 'IRpcStubBuffer' : named type definition in parentheses
#	pragma warning(disable : 4711)	// warning C4711: function selected for automatic inline expansion
#	ifndef WINVER
#		define WINVER 0x0400
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#	endif
#	include <windows.h>		/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	include <process.h>
#	include <direct.h>
#	include <io.h>
#	include <stdio.h>
#	include <string.h>
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <stdarg.h>
#	include <time.h>
#	include <io.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <errno.h>
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define sleep(a) Sleep(a * 1000)
#	ifndef FOPEN_READ_TEXT
#		define FOPEN_READ_TEXT "rt"
#		define FOPEN_WRITE_TEXT "wt"
#		define FOPEN_APPEND_TEXT "at"
#	endif
#	ifndef S_ISREG
#		define S_ISREG(m)      (((m) & _S_IFMT) == _S_IFREG)
#		define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
#	endif
#	define uid_t int
#	define HAVE_SNPRINTF 1
#	define HAVE_VSNPRINTF 1
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
#elif defined(__CYGWIN__)
#	error "This version is for native Windows only."
#else	/* UNIX */
#	error "This version is for Windows only."
#endif	/* UNIX */

#include <Strn.h>			/* Library header. */
