/* u_printf.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/*VARARGS*/
void
PrintF(const FTPCIPtr cip, const char *const fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	if (cip->debugLog != NULL) {
		(void) vfprintf(cip->debugLog, fmt, ap);
		(void) fflush(cip->debugLog);
	}
	if (cip->debugLogProc != NULL) {
#ifdef HAVE_VSNPRINTF
		(void) vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
		buf[sizeof(buf) - 1] = '\0';
#else
		(void) vsprintf(buf, fmt, ap);
#endif
		(*cip->debugLogProc)(cip, buf);
	}
	va_end(ap);
}	/* PrintF */
