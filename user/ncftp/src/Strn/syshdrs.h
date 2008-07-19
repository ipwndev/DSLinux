/* syshdrs.h
 *
 * Copyright (c) 1996-2004 Mike Gleason, NcFTP Software.
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
#	define WINVER 0x0400
#	define _WIN32_WINNT 0x0400
#	include <windows.h>
#	include <sys/types.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <stdarg.h>
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define HAVE_STRDUP 1
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
#endif /* ---------------------------- UNIX ---------------------------- */

#include "DStrInternal.h"
#include "Strn.h"

#ifndef HAVE_STRDUP
extern char *strdup(const char *const src);
#endif

/* eof */
