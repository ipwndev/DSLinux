/* u_getutc.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Cheezy, but somewhat portable way to get GMT offset. */
time_t
GetUTCOffset2(const int year, const int mon, const int mday, const int hour, const int min)
{
#ifdef HAVE_MKTIME
	struct tm local_tm, utc_tm, *utc_tmptr;
	time_t local_t, utc_t, utcOffset;

	ZERO(local_tm);
	ZERO(utc_tm);
	utcOffset = 0;
	
	local_tm.tm_year = year;
	local_tm.tm_mon = mon - 1;
	local_tm.tm_mday = mday;
	local_tm.tm_hour = hour;
	local_tm.tm_min = min;
	local_tm.tm_isdst = -1;
	local_t = mktime(&local_tm);
	
	if (local_t != (time_t) -1) {
		utc_tmptr = Gmtime(local_t, &local_tm);
		utc_tm.tm_year = utc_tmptr->tm_year;
		utc_tm.tm_mon = utc_tmptr->tm_mon;
		utc_tm.tm_mday = utc_tmptr->tm_mday;
		utc_tm.tm_hour = utc_tmptr->tm_hour;
		utc_tm.tm_min = utc_tmptr->tm_min;
		utc_tm.tm_isdst = -1;
		utc_t = mktime(&utc_tm);

		if (utc_t != (time_t) -1)
			utcOffset = (local_t - utc_t);
	}
	return (utcOffset);
#else
	return ((time_t) -1);
#endif	/* HAVE_MKTIME */
}	/* GetUTCOffset2 */




time_t
GetUTCOffset(const int mon, const int mday)
{
	return (GetUTCOffset2(2000, mon, mday, 12, 0));
}	/* GetUTCOffset */
