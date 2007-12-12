/* u_getusr.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
GetUsrName(char *dst, size_t size)
{
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	DWORD size1;

	size1 = size - 1;
	if (! GetUserName(dst, &size1))
		(void) strncpy(dst, "unknown", size);
	dst[size - 1] = '\0';
#else
	struct passwd pw;
	char pwbuf[256];

	if (GetMyPwEnt(&pw, pwbuf, sizeof(pwbuf)) == 0) {
		(void) Strncpy(dst, pw.pw_name, size);
	} else {
		(void) Strncpy(dst, "UNKNOWN", size);
	}
#endif
}	/* GetUserName */
