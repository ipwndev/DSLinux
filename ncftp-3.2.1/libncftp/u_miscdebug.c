/* u_miscdebug.c
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
FTPGetDateStr(time_t t, const char *fmt, char *const ltstr1, const size_t ltstr1size, char *const gtstr1, const size_t gtstr1size)
{
	char ltstr[64];
	char gtstr[64];
	struct tm *ltp;
	struct tm *gtp;

	if (fmt == NULL) {
		/* Default to RFC 822 date format, with the
		 * addition of the textual timezone,
		 * i.e. RFC822+%Z.
		 */
		fmt = "%a, %d %b %Y %H:%M:%S %z %Z";
	}

	memset(gtstr, 0, sizeof(gtstr));
	memset(ltstr, 0, sizeof(ltstr));
	if ((ltstr1 != NULL) && (ltstr1size != 0))
		memset(ltstr1, 0, ltstr1size);
	if ((gtstr1 != NULL) && (gtstr1size != 0))
		memset(gtstr1, 0, gtstr1size);

	if (t == 0) {
		(void) time(&t);
		if ((t == 0) || (t == (time_t) -1)) {
			/* Should never happen */
			return;
		}
	}

	if ((gtp = gmtime(&t)) != NULL) {
		strftime(gtstr, sizeof(gtstr) - 1, fmt, gtp);
		if ((gtstr1 != NULL) && (gtstr1size != 0))
			(void) Strncpy(gtstr1, gtstr, gtstr1size);
	}

	if ((ltp = localtime(&t)) != NULL) {
		strftime(ltstr, sizeof(ltstr) - 1, fmt, ltp);
		if ((ltstr1 != NULL) && (ltstr1size != 0))
			(void) Strncpy(ltstr1, ltstr, ltstr1size);
	}
}	/* FTPGetDateStr */
