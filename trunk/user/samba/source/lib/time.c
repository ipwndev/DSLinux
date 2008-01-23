/* 
   Unix SMB/CIFS implementation.
   time handling functions
   Copyright (C) Andrew Tridgell 		1992-1998
   Copyright (C) Stefan (metze) Metzmacher	2002   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "includes.h"

/*
  This stuff was largely rewritten by Paul Eggert <eggert@twinsun.com>
  in May 1996 
  */


int extra_time_offset = 0;

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef TIME_T_MIN
#define TIME_T_MIN ((time_t)0 < (time_t) -1 ? (time_t) 0 \
		    : ~ (time_t) 0 << (sizeof (time_t) * CHAR_BIT - 1))
#endif
#ifndef TIME_T_MAX
#define TIME_T_MAX (~ (time_t) 0 - TIME_T_MIN)
#endif

void get_nttime_max(NTTIME *t)
{
	/* FIXME: This is incorrect */
	unix_to_nt_time(t, get_time_t_max());
}

/*******************************************************************
 External access to time_t_min and time_t_max.
********************************************************************/

time_t get_time_t_max(void)
{
	return TIME_T_MAX;
}

/*******************************************************************
 A gettimeofday wrapper.
********************************************************************/

void GetTimeOfDay(struct timeval *tval)
{
#ifdef HAVE_GETTIMEOFDAY_TZ
	gettimeofday(tval,NULL);
#else
	gettimeofday(tval);
#endif
}

#define TM_YEAR_BASE 1900

/*******************************************************************
 Yield the difference between *A and *B, in seconds, ignoring leap seconds.
********************************************************************/

static int tm_diff(struct tm *a, struct tm *b)
{
	int ay = a->tm_year + (TM_YEAR_BASE - 1);
	int by = b->tm_year + (TM_YEAR_BASE - 1);
	int intervening_leap_days = (ay/4 - by/4) - (ay/100 - by/100) + (ay/400 - by/400);
	int years = ay - by;
	int days = 365*years + intervening_leap_days + (a->tm_yday - b->tm_yday);
	int hours = 24*days + (a->tm_hour - b->tm_hour);
	int minutes = 60*hours + (a->tm_min - b->tm_min);
	int seconds = 60*minutes + (a->tm_sec - b->tm_sec);

	return seconds;
}

/*******************************************************************
 Return the UTC offset in seconds west of UTC, or 0 if it cannot be determined.
******************************************************************/

static int TimeZone(time_t t)
{
	struct tm *tm = gmtime(&t);
	struct tm tm_utc;
	if (!tm)
		return 0;
	tm_utc = *tm;
	tm = localtime(&t);
	if (!tm)
		return 0;
	return tm_diff(&tm_utc,tm);
}

static BOOL done_serverzone_init;

/*******************************************************************
 Return the smb serverzone value.
******************************************************************/

static int get_serverzone(void)
{
        static int serverzone;

        if (!done_serverzone_init) {
                serverzone = TimeZone(time(NULL));

                if ((serverzone % 60) != 0) {
                        DEBUG(1,("WARNING: Your timezone is not a multiple of 1 minute.\n"));
                }

                DEBUG(4,("Serverzone is %d\n",serverzone));

                done_serverzone_init = True;
        }

        return serverzone;
}

/*******************************************************************
 Re-read the smb serverzone value.
******************************************************************/

static struct timeval start_time_hires;

void TimeInit(void)
{
	done_serverzone_init = False;
	get_serverzone();
	/* Save the start time of this process. */
	if (start_time_hires.tv_sec == 0 && start_time_hires.tv_usec == 0)
		GetTimeOfDay(&start_time_hires);
}

/**********************************************************************
 Return a timeval struct of the uptime of this process. As TimeInit is
 done before a daemon fork then this is the start time from the parent
 daemon start. JRA.
***********************************************************************/

void get_process_uptime(struct timeval *ret_time)
{
	struct timeval time_now_hires;

	GetTimeOfDay(&time_now_hires);
	ret_time->tv_sec = time_now_hires.tv_sec - start_time_hires.tv_sec;
	ret_time->tv_usec = time_now_hires.tv_usec - start_time_hires.tv_usec;
	if (time_now_hires.tv_usec < start_time_hires.tv_usec) {
		ret_time->tv_sec -= 1;
		ret_time->tv_usec = 1000000 + (time_now_hires.tv_usec - start_time_hires.tv_usec);
	} else
		ret_time->tv_usec = time_now_hires.tv_usec - start_time_hires.tv_usec;
}

/*******************************************************************
 Return the same value as TimeZone, but it should be more efficient.

 We keep a table of DST offsets to prevent calling localtime() on each 
 call of this function. This saves a LOT of time on many unixes.

 Updated by Paul Eggert <eggert@twinsun.com>
********************************************************************/

static int TimeZoneFaster(time_t t)
{
	static struct dst_table {time_t start,end; int zone;} *tdt, *dst_table = NULL;
	static int table_size = 0;
	int i;
	int zone = 0;

	if (t == 0)
		t = time(NULL);

	/* Tunis has a 8 day DST region, we need to be careful ... */
#define MAX_DST_WIDTH (365*24*60*60)
#define MAX_DST_SKIP (7*24*60*60)

	for (i=0;i<table_size;i++)
		if (t >= dst_table[i].start && t <= dst_table[i].end)
			break;

	if (i<table_size) {
		zone = dst_table[i].zone;
	} else {
		time_t low,high;

		zone = TimeZone(t);
		tdt = SMB_REALLOC_ARRAY(dst_table, struct dst_table, i+1);
		if (!tdt) {
			DEBUG(0,("TimeZoneFaster: out of memory!\n"));
			SAFE_FREE(dst_table);
			table_size = 0;
		} else {
			dst_table = tdt;
			table_size++;

			dst_table[i].zone = zone; 
			dst_table[i].start = dst_table[i].end = t;
    
			/* no entry will cover more than 6 months */
			low = t - MAX_DST_WIDTH/2;
			if (t < low)
				low = TIME_T_MIN;
      
			high = t + MAX_DST_WIDTH/2;
			if (high < t)
				high = TIME_T_MAX;
      
			/* widen the new entry using two bisection searches */
			while (low+60*60 < dst_table[i].start) {
				if (dst_table[i].start - low > MAX_DST_SKIP*2)
					t = dst_table[i].start - MAX_DST_SKIP;
				else
					t = low + (dst_table[i].start-low)/2;
				if (TimeZone(t) == zone)
					dst_table[i].start = t;
				else
					low = t;
			}

			while (high-60*60 > dst_table[i].end) {
				if (high - dst_table[i].end > MAX_DST_SKIP*2)
					t = dst_table[i].end + MAX_DST_SKIP;
				else
					t = high - (high-dst_table[i].end)/2;
				if (TimeZone(t) == zone)
					dst_table[i].end = t;
				else
					high = t;
			}
#if 0
      DEBUG(1,("Added DST entry from %s ",
	       asctime(localtime(&dst_table[i].start))));
      DEBUG(1,("to %s (%d)\n",asctime(localtime(&dst_table[i].end)),
	       dst_table[i].zone));
#endif
		}
	}
	return zone;
}

/****************************************************************************
 Return the UTC offset in seconds west of UTC, adjusted for extra time offset.
**************************************************************************/

int TimeDiff(time_t t)
{
	return TimeZoneFaster(t) + 60*extra_time_offset;
}

/****************************************************************************
 Return the UTC offset in seconds west of UTC, adjusted for extra time
 offset, for a local time value.  If ut = lt + LocTimeDiff(lt), then
 lt = ut - TimeDiff(ut), but the converse does not necessarily hold near
 daylight savings transitions because some local times are ambiguous.
 LocTimeDiff(t) equals TimeDiff(t) except near daylight savings transitions.
**************************************************************************/

static int LocTimeDiff(time_t lte)
{
	time_t lt = lte - 60*extra_time_offset;
	int d = TimeZoneFaster(lt);
	time_t t = lt + d;

	/* if overflow occurred, ignore all the adjustments so far */
	if (((lte < lt) ^ (extra_time_offset < 0))  |  ((t < lt) ^ (d < 0)))
		t = lte;

	/* now t should be close enough to the true UTC to yield the right answer */
	return TimeDiff(t);
}

/****************************************************************************
 Try to optimise the localtime call, it can be quite expensive on some machines.
****************************************************************************/

struct tm *LocalTime(time_t *t)
{
	time_t t2 = *t;

	t2 -= TimeDiff(t2);

	return(gmtime(&t2));
}

#define TIME_FIXUP_CONSTANT (369.0*365.25*24*60*60-(3.0*24*60*60+6.0*60*60))

/****************************************************************************
 Interpret an 8 byte "filetime" structure to a time_t
 It's originally in "100ns units since jan 1st 1601"

 An 8 byte value of 0xffffffffffffffff will be returned as (time_t)0.

 It appears to be kludge-GMT (at least for file listings). This means
 its the GMT you get by taking a localtime and adding the
 serverzone. This is NOT the same as GMT in some cases. This routine
 converts this to real GMT.
****************************************************************************/

time_t nt_time_to_unix(NTTIME *nt)
{
	double d;
	time_t ret;
	/* The next two lines are a fix needed for the 
		broken SCO compiler. JRA. */
	time_t l_time_min = TIME_T_MIN;
	time_t l_time_max = TIME_T_MAX;

	if (nt->high == 0 || (nt->high == 0xffffffff && nt->low == 0xffffffff))
		return(0);

	d = ((double)nt->high)*4.0*(double)(1<<30);
	d += (nt->low&0xFFF00000);
	d *= 1.0e-7;
 
	/* now adjust by 369 years to make the secs since 1970 */
	d -= TIME_FIXUP_CONSTANT;

	if (d <= l_time_min)
		return (l_time_min);

	if (d >= l_time_max)
		return (l_time_max);

	ret = (time_t)(d+0.5);

	/* this takes us from kludge-GMT to real GMT */
	ret -= get_serverzone();
	ret += LocTimeDiff(ret);

	return(ret);
}

/****************************************************************************
 Convert a NTTIME structure to a time_t.
 It's originally in "100ns units".

 This is an absolute version of the one above.
 By absolute I mean, it doesn't adjust from 1/1/1601 to 1/1/1970
 if the NTTIME was 5 seconds, the time_t is 5 seconds. JFM
****************************************************************************/

time_t nt_time_to_unix_abs(NTTIME *nt)
{
	double d;
	time_t ret;
	/* The next two lines are a fix needed for the 
	   broken SCO compiler. JRA. */
	time_t l_time_min = TIME_T_MIN;
	time_t l_time_max = TIME_T_MAX;

	if (nt->high == 0)
		return(0);

	if (nt->high==0x80000000 && nt->low==0)
		return -1;

	/* reverse the time */
	/* it's a negative value, turn it to positive */
	nt->high=~nt->high;
	nt->low=~nt->low;

	d = ((double)nt->high)*4.0*(double)(1<<30);
	d += (nt->low&0xFFF00000);
	d *= 1.0e-7;
  
	if (!(l_time_min <= d && d <= l_time_max))
		return(0);

	ret = (time_t)(d+0.5);

	return(ret);
}

/****************************************************************************
 Interprets an nt time into a unix time_t.
 Differs from nt_time_to_unix in that an 8 byte value of 0xffffffffffffffff
 will be returned as (time_t)-1, whereas nt_time_to_unix returns 0 in this case.
****************************************************************************/

time_t interpret_long_date(char *p)
{
	NTTIME nt;
	nt.low = IVAL(p,0);
	nt.high = IVAL(p,4);
	if (nt.low == 0xFFFFFFFF && nt.high == 0xFFFFFFFF) {
		return (time_t)-1;
	}
	return nt_time_to_unix(&nt);
}

/****************************************************************************
 Put a 8 byte filetime from a time_t
 This takes real GMT as input and converts to kludge-GMT
****************************************************************************/

void unix_to_nt_time(NTTIME *nt, time_t t)
{
	double d;

	if (t==0) {
		nt->low = 0;
		nt->high = 0;
		return;
	}
	if (t == TIME_T_MAX) {
		nt->low = 0xffffffff;
		nt->high = 0x7fffffff;
		return;
	}		
	if (t == -1) {
		nt->low = 0xffffffff;
		nt->high = 0xffffffff;
		return;
	}		

	/* this converts GMT to kludge-GMT */
	t -= TimeDiff(t) - get_serverzone(); 

	d = (double)(t);
	d += TIME_FIXUP_CONSTANT;
	d *= 1.0e7;

	nt->high = (uint32)(d * (1.0/(4.0*(double)(1<<30))));
	nt->low  = (uint32)(d - ((double)nt->high)*4.0*(double)(1<<30));
}

/****************************************************************************
 Convert a time_t to a NTTIME structure

 This is an absolute version of the one above.
 By absolute I mean, it doesn't adjust from 1/1/1970 to 1/1/1601
 If the nttime_t was 5 seconds, the NTTIME is 5 seconds. JFM
****************************************************************************/

void unix_to_nt_time_abs(NTTIME *nt, time_t t)
{
	double d;

	if (t==0) {
		nt->low = 0;
		nt->high = 0;
		return;
	}

	if (t == TIME_T_MAX) {
		nt->low = 0xffffffff;
		nt->high = 0x7fffffff;
		return;
	}
		
	if (t == -1) {
		/* that's what NT uses for infinite */
		nt->low = 0x0;
		nt->high = 0x80000000;
		return;
	}		

	d = (double)(t);
	d *= 1.0e7;

	nt->high = (uint32)(d * (1.0/(4.0*(double)(1<<30))));
	nt->low  = (uint32)(d - ((double)nt->high)*4.0*(double)(1<<30));

	/* convert to a negative value */
	nt->high=~nt->high;
	nt->low=~nt->low;
}

/****************************************************************************
 Take a Unix time and convert to an NTTIME structure and place in buffer 
 pointed to by p.
****************************************************************************/

void put_long_date(char *p,time_t t)
{
	NTTIME nt;
	unix_to_nt_time(&nt, t);
	SIVAL(p, 0, nt.low);
	SIVAL(p, 4, nt.high);
}

/****************************************************************************
 Check if it's a null mtime.
****************************************************************************/

BOOL null_mtime(time_t mtime)
{
	if (mtime == 0 || mtime == (time_t)0xFFFFFFFF || mtime == (time_t)-1)
		return(True);
	return(False);
}

/*******************************************************************
 Create a 16 bit dos packed date.
********************************************************************/

static uint16 make_dos_date1(struct tm *t)
{
	uint16 ret=0;
	ret = (((unsigned)(t->tm_mon+1)) >> 3) | ((t->tm_year-80) << 1);
	ret = ((ret&0xFF)<<8) | (t->tm_mday | (((t->tm_mon+1) & 0x7) << 5));
	return(ret);
}

/*******************************************************************
 Create a 16 bit dos packed time.
********************************************************************/

static uint16 make_dos_time1(struct tm *t)
{
	uint16 ret=0;
	ret = ((((unsigned)t->tm_min >> 3)&0x7) | (((unsigned)t->tm_hour) << 3));
	ret = ((ret&0xFF)<<8) | ((t->tm_sec/2) | ((t->tm_min & 0x7) << 5));
	return(ret);
}

/*******************************************************************
 Create a 32 bit dos packed date/time from some parameters.
 This takes a GMT time and returns a packed localtime structure.
********************************************************************/

static uint32 make_dos_date(time_t unixdate)
{
	struct tm *t;
	uint32 ret=0;

	t = LocalTime(&unixdate);
	if (!t)
		return 0xFFFFFFFF;

	ret = make_dos_date1(t);
	ret = ((ret&0xFFFF)<<16) | make_dos_time1(t);

	return(ret);
}

/*******************************************************************
 Put a dos date into a buffer (time/date format).
 This takes GMT time and puts local time in the buffer.
********************************************************************/

void put_dos_date(char *buf,int offset,time_t unixdate)
{
	uint32 x = make_dos_date(unixdate);
	SIVAL(buf,offset,x);
}

/*******************************************************************
 Put a dos date into a buffer (date/time format).
 This takes GMT time and puts local time in the buffer.
********************************************************************/

void put_dos_date2(char *buf,int offset,time_t unixdate)
{
	uint32 x = make_dos_date(unixdate);
	x = ((x&0xFFFF)<<16) | ((x&0xFFFF0000)>>16);
	SIVAL(buf,offset,x);
}

/*******************************************************************
 Put a dos 32 bit "unix like" date into a buffer. This routine takes
 GMT and converts it to LOCAL time before putting it (most SMBs assume
 localtime for this sort of date)
********************************************************************/

void put_dos_date3(char *buf,int offset,time_t unixdate)
{
	if (!null_mtime(unixdate))
		unixdate -= TimeDiff(unixdate);
	SIVAL(buf,offset,unixdate);
}

/*******************************************************************
 Interpret a 32 bit dos packed date/time to some parameters.
********************************************************************/

static void interpret_dos_date(uint32 date,int *year,int *month,int *day,int *hour,int *minute,int *second)
{
	uint32 p0,p1,p2,p3;

	p0=date&0xFF; p1=((date&0xFF00)>>8)&0xFF; 
	p2=((date&0xFF0000)>>16)&0xFF; p3=((date&0xFF000000)>>24)&0xFF;

	*second = 2*(p0 & 0x1F);
	*minute = ((p0>>5)&0xFF) + ((p1&0x7)<<3);
	*hour = (p1>>3)&0xFF;
	*day = (p2&0x1F);
	*month = ((p2>>5)&0xFF) + ((p3&0x1)<<3) - 1;
	*year = ((p3>>1)&0xFF) + 80;
}

/*******************************************************************
 Create a unix date (int GMT) from a dos date (which is actually in
 localtime).
********************************************************************/

time_t make_unix_date(void *date_ptr)
{
	uint32 dos_date=0;
	struct tm t;
	time_t ret;

	dos_date = IVAL(date_ptr,0);

	if (dos_date == 0)
		return(0);
  
	interpret_dos_date(dos_date,&t.tm_year,&t.tm_mon,
			&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
	t.tm_isdst = -1;
  
	/* mktime() also does the local to GMT time conversion for us */
	ret = mktime(&t);

	return(ret);
}

/*******************************************************************
 Like make_unix_date() but the words are reversed.
********************************************************************/

time_t make_unix_date2(void *date_ptr)
{
	uint32 x,x2;

	x = IVAL(date_ptr,0);
	x2 = ((x&0xFFFF)<<16) | ((x&0xFFFF0000)>>16);
	SIVAL(&x,0,x2);

	return(make_unix_date((void *)&x));
}

/*******************************************************************
 Create a unix GMT date from a dos date in 32 bit "unix like" format
 these generally arrive as localtimes, with corresponding DST.
******************************************************************/

time_t make_unix_date3(void *date_ptr)
{
	time_t t = (time_t)IVAL(date_ptr,0);
	if (!null_mtime(t))
		t += LocTimeDiff(t);
	return(t);
}

/***************************************************************************
 Return a HTTP/1.0 time string.
***************************************************************************/

char *http_timestring(time_t t)
{
	static fstring buf;
	struct tm *tm = LocalTime(&t);

	if (!tm)
		slprintf(buf,sizeof(buf)-1,"%ld seconds since the Epoch",(long)t);
	else
#ifndef HAVE_STRFTIME
		fstrcpy(buf, asctime(tm));
	if(buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = 0;
#else /* !HAVE_STRFTIME */
		strftime(buf, sizeof(buf)-1, "%a, %d %b %Y %H:%M:%S %Z", tm);
#endif /* !HAVE_STRFTIME */
	return buf;
}

/****************************************************************************
 Return the date and time as a string
****************************************************************************/

char *timestring(BOOL hires)
{
	static fstring TimeBuf;
	struct timeval tp;
	time_t t;
	struct tm *tm;

	if (hires) {
		GetTimeOfDay(&tp);
		t = (time_t)tp.tv_sec;
	} else {
		t = time(NULL);
	}
	tm = LocalTime(&t);
	if (!tm) {
		if (hires) {
			slprintf(TimeBuf,
				 sizeof(TimeBuf)-1,
				 "%ld.%06ld seconds since the Epoch",
				 (long)tp.tv_sec, 
				 (long)tp.tv_usec);
		} else {
			slprintf(TimeBuf,
				 sizeof(TimeBuf)-1,
				 "%ld seconds since the Epoch",
				 (long)t);
		}
	} else {
#ifdef HAVE_STRFTIME
		if (hires) {
			strftime(TimeBuf,sizeof(TimeBuf)-1,"%Y/%m/%d %H:%M:%S",tm);
			slprintf(TimeBuf+strlen(TimeBuf),
				 sizeof(TimeBuf)-1 - strlen(TimeBuf), 
				 ".%06ld", 
				 (long)tp.tv_usec);
		} else {
			strftime(TimeBuf,sizeof(TimeBuf)-1,"%Y/%m/%d %H:%M:%S",tm);
		}
#else
		if (hires) {
			slprintf(TimeBuf, 
				 sizeof(TimeBuf)-1, 
				 "%s.%06ld", 
				 asctime(tm), 
				 (long)tp.tv_usec);
		} else {
			fstrcpy(TimeBuf, asctime(tm));
		}
#endif
	}
	return(TimeBuf);
}

/****************************************************************************
 Return the best approximation to a 'create time' under UNIX from a stat
 structure.
****************************************************************************/

time_t get_create_time(SMB_STRUCT_STAT *st,BOOL fake_dirs)
{
	time_t ret, ret1;

	if(S_ISDIR(st->st_mode) && fake_dirs)
		return (time_t)315493200L;          /* 1/1/1980 */
    
	ret = MIN(st->st_ctime, st->st_mtime);
	ret1 = MIN(ret, st->st_atime);

	if(ret1 != (time_t)0)
		return ret1;

	/*
	 * One of ctime, mtime or atime was zero (probably atime).
	 * Just return MIN(ctime, mtime).
	 */
	return ret;
}

/****************************************************************************
 Initialise an NTTIME to -1, which means "unknown" or "don't expire".
****************************************************************************/

void init_nt_time(NTTIME *nt)
{
	nt->high = 0x7FFFFFFF;
	nt->low = 0xFFFFFFFF;
}

/****************************************************************************
 Check if NTTIME is 0.
****************************************************************************/

BOOL nt_time_is_zero(NTTIME *nt)
{
	if(nt->high==0) 
		return True;
	return False;
}

/****************************************************************************
 Return a timeval difference in usec.
****************************************************************************/

SMB_BIG_INT usec_time_diff(struct timeval *larget, struct timeval *smallt)
{
	SMB_BIG_INT sec_diff = larget->tv_sec - smallt->tv_sec;
	return (sec_diff * 1000000) + (SMB_BIG_INT)(larget->tv_usec - smallt->tv_usec);
}
