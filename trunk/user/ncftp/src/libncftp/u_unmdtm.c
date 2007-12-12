/* u_unmdtm.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Converts a MDTM date, like "19930602204445"
 * format to a time_t.
 */
time_t UnMDTMDate(char *dstr)
{
#ifndef HAVE_MKTIME
	return (kModTimeUnknown);
#else
	struct tm ut;
	time_t mt, now;
	time_t result = kModTimeUnknown;
	char y2fix[64];

	if (strncmp(dstr, "1910", 4) == 0) {
		/* Server Y2K bug! */
		memset(y2fix, 0, sizeof(y2fix));
		y2fix[0] = '2';
		y2fix[1] = '0';
		y2fix[2] = dstr[3];
		y2fix[3] = dstr[4];
		strncpy(y2fix + 4, dstr + 5, sizeof(y2fix) - 6);
		dstr = y2fix;
	}

	/* Get a fully populated struct tm.  We do this so we don't have
	 * to set all fields ourselves, i.e., non-standard fields such as
	 * tm_gmtoff, if it exists or not.
	 */
	if (Gmtime(time(&now), &ut) == NULL)
		return result;

	/* The time we get back from the server is (should be) in UTC. */
	if (sscanf(dstr, "%04d%02d%02d%02d%02d%02d",
		&ut.tm_year,
		&ut.tm_mon,
		&ut.tm_mday,
		&ut.tm_hour,
		&ut.tm_min,
		&ut.tm_sec) == 6)
	{	
		--ut.tm_mon;
		ut.tm_year -= 1900;
		ut.tm_isdst = -1;
		mt = mktime(&ut);
		if (mt != (time_t) -1) {
			mt += GetUTCOffset2(ut.tm_year, ut.tm_mon + 1, ut.tm_mday, ut.tm_hour, ut.tm_min);
			result = (time_t) mt;
		}
	}
	return result;
#endif	/* HAVE_MKTIME */
}	/* UnMDTMDate */
