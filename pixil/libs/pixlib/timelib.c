/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


/*
**
** Imported "Include" Files
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pixlib/timelib.h>



/*
**
** Local Variable Declarations
**
*/
static char *full_month_list[] = {
    NULL,
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};
static char *short_month_list[] = {
    NULL, "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec"
};
static char *full_day_list[] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};
static char *short_day_list[] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
static short reg_days_in_month[] =
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static short leap_days_in_month[] =
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



/*
**
** The following function will determine if the specified year is a leap
** year. If it is, a non-zero value is returned. If it is not, zero is
** returned.
**
*/
static int
isLeapYear(unsigned short year)
{
    unsigned short result;

    result = year / 4;
    if ((result * 4) == year)
	return (1);
    else
	return (0);
}



/*
**
** This function will convert the time/date information found in the
** specified 'tm' structure into an pix_sys_date_t strcuture.
**
** The "tstruct" parameter is a pointer to the 'tm' structure. The
** "result" parameter is a pointer to an pix_sys_date_t structure that
** will contain the result.
**
** If successful, '0' is returned. If the time/date specified by the
** 'tm' structure is outside the bounds of an pix_sys_date_t structure, a
** non-zero value is returned.
**
*/
int
pix_sysConvertTMStruct(struct tm *tstruct, pix_sys_date_t * result)
{
    /* convert the structure */
    result->year = tstruct->tm_year + 1900;
    result->month = tstruct->tm_mon + 1;
    result->day = tstruct->tm_mday;
    result->hour = tstruct->tm_hour;
    result->minute = tstruct->tm_min;
    result->second = tstruct->tm_sec;

    /* exit with no errors */
    return (0);
}



/*
**
** This function will initialize an pix_sys_date_t structure with
** the current time and date.
**
** The "result" parameter points to the pix_sys_date_t structure
** that will contain the result.
**
*/
void
pix_sysGetCurrentTime(pix_sys_date_t * result)
{
    time_t tick;
    struct tm *tdata;

    tick = time(NULL);
    tdata = localtime(&tick);
    result->year = tdata->tm_year + 1900;
    result->month = tdata->tm_mon + 1;
    result->day = tdata->tm_mday;
    result->hour = tdata->tm_hour;
    result->minute = tdata->tm_min;
    result->second = tdata->tm_sec;
}



/*
**
** This function will compare the dates found in two pix_sys_date_t
** structures.
**
** The "stamp1" and "stamp2" parameters are pointers to initialized
** pix_sys_date_t structures.
**
** If the first date is earlier than the second, a negative
** value is returned. If the first date is later than the second,
** a positive value is returned. If the two dates are the same,
** '0' is returned.
**
*/
int
pix_sysCompareDates(pix_sys_date_t * stamp1, pix_sys_date_t * stamp2)
{
    /* compare years */
    if (stamp1->year < stamp2->year)
	return (-1);
    if (stamp1->year > stamp2->year)
	return (1);

    /* years are the same - compare months */
    if (stamp1->month < stamp2->month)
	return (-1);
    if (stamp1->month > stamp2->month)
	return (1);

    /* months are the same - compare days */
    if (stamp1->day < stamp2->day)
	return (-1);
    if (stamp1->day > stamp2->day)
	return (1);

    /* dates are the same */
    return (0);
}



/*
**
** The following function will compare the dates and times found
** in two pix_sys_date_t structures.
**
** The "stamp1" and "stamp2" parameters are pointers to initialized
** pix_sys_date_t structures.
**
** If the first timestamp is earlier than the second, a negative
** value is returned. If the first timestamp is later than the
** second, a positive value is returned. If the two timestamps
** are the same, '0' is returned.
**
*/
int
pix_sysCompareTimeStamps(pix_sys_date_t * stamp1, pix_sys_date_t * stamp2)
{
    /* compare years */
    if (stamp1->year < stamp2->year)
	return (-1);
    if (stamp1->year > stamp2->year)
	return (1);

    /* years are the same - compare months */
    if (stamp1->month < stamp2->month)
	return (-1);
    if (stamp1->month > stamp2->month)
	return (1);

    /* months are the same - compare days */
    if (stamp1->day < stamp2->day)
	return (-1);
    if (stamp1->day > stamp2->day)
	return (1);

    /* days are the same - compare hours */
    if (stamp1->hour < stamp2->hour)
	return (-1);
    if (stamp1->hour > stamp2->hour)
	return (1);

    /* hours are the same - compare minutes */
    if (stamp1->minute < stamp2->minute)
	return (-1);
    if (stamp1->minute > stamp2->minute)
	return (1);

    /* minutes are the same - compare seconds */
    if (stamp1->second < stamp2->second)
	return (-1);
    if (stamp1->second > stamp2->second)
	return (1);

    /* timestamps are equal */
    return (0);
}



/*
**
** This function will check to see if the subject timestamp falls
** within the bounds specified by the first and last timestamps.
**
** The "first" and "last" parameters are pointers to initialized
** pix_sys_date_t structures that define the bounds. The "subject"
** parameter is a pointer to an initialized pix_sys_date_t structure
** that specifies the time to be tested.
**
** If the subject timestamp is greater than or equal to the first
** timestamp and is less than or equal to the last timestamp, '0'
** is returned. Otherwise, the subject timestamp is outside of the
** bounds specified and a non-zero value is returned.
**
*/
int
pix_sysCompareBounds(pix_sys_date_t * first, pix_sys_date_t * last,
		     pix_sys_date_t * subject)
{
    if (pix_sysCompareTimeStamps(subject, first) == -1)
	return (-1);
    if (pix_sysCompareTimeStamps(subject, last) == 1)
	return (1);
    return (0);
}



/*
**
** This function will add one day to the date found in a
** pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysIncrementDate(pix_sys_date_t * stamp)
{
    unsigned short lastday;

    /* recover last date for this month */
    if (isLeapYear(stamp->year))
	lastday = leap_days_in_month[stamp->month];
    else
	lastday = reg_days_in_month[stamp->month];

    /* bump the date */
    if (stamp->day + 1 <= lastday) {
	stamp->day += 1;
	return;
    }

    /* date has overflowed - adjust */
    stamp->day = 1;
    stamp->month += 1;

    /* see if month is within bounds */
    if (stamp->month <= 12)
	return;

    /* month has overflowed - adjust */
    stamp->month = 1;
    stamp->year += 1;
}



/*
**
** This function will subtract one day from the date found in a
** pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysDecrementDate(pix_sys_date_t * stamp)
{
    /* bump the date */
    stamp->day -= 1;

    /* see if date is within bounds */
    if (stamp->day > 0)
	return;

    /* date has underflowed - handle special case at beginning of year */
    if (stamp->month == 1) {
	stamp->year -= 1;
	stamp->month = 12;
	stamp->day = 31;
	return;
    }

    /* date has underflowed - adjust */
    stamp->month -= 1;
    if (isLeapYear(stamp->year))
	stamp->day = leap_days_in_month[stamp->month];
    else
	stamp->day = reg_days_in_month[stamp->month];
}



/*
**
** This funtion will add one hour to the date found in an pix_sys_date_t
** structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysIncrementHour(pix_sys_date_t * stamp)
{
    /* bump the hour */
    stamp->hour += 1;

    /* see if hour is within bounds */
    if (stamp->hour < 24)
	return;

    /* hour has overflowed - adjust */
    stamp->hour = 0;
    pix_sysIncrementDate(stamp);
}



/*
**
** This funtion will subtract one hour from the date found in an
** pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysDecrementHour(pix_sys_date_t * stamp)
{
    /* bump the hour */
    stamp->hour -= 1;

    /* see if hour is within bounds */
    if (stamp->hour < 24)
	return;

    /* hour has overflowed - adjust */
    stamp->hour = 23;
    pix_sysDecrementDate(stamp);
}



/*
**
** This funtion will add one minute to the date found in an pix_sys_date_t
** structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysIncrementMinute(pix_sys_date_t * stamp)
{
    /* bump the minute */
    stamp->minute += 1;

    /* see if minute is within bounds */
    if (stamp->minute < 60)
	return;

    /* minute has overflowed - adjust */
    stamp->minute = 0;
    pix_sysIncrementHour(stamp);
}



/*
**
** This funtion will subtract one minute from the date found in an
** pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysDecrementMinute(pix_sys_date_t * stamp)
{
    /* bump the minute */
    stamp->minute -= 1;

    /* see if minute is within bounds */
    if (stamp->minute < 60)
	return;

    /* minute has overflowed - adjust */
    stamp->minute = 59;
    pix_sysDecrementHour(stamp);
}



/*
**
** This funtion will add one second to the date found in an pix_sys_date_t
** structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysIncrementSecond(pix_sys_date_t * stamp)
{
    /* bump the second */
    stamp->second += 1;

    /* see if second is within bounds */
    if (stamp->second < 60)
	return;

    /* second has overflowed - adjust */
    stamp->second = 0;
    pix_sysIncrementMinute(stamp);
}



/*
**
** This funtion will subtract one second from the date found in an
** pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
*/
void
pix_sysDecrementSecond(pix_sys_date_t * stamp)
{
    /* bump the second */
    stamp->second -= 1;

    /* see if second is within bounds */
    if (stamp->second < 60)
	return;

    /* second has overflowed - adjust */
    stamp->second = 59;
    pix_sysDecrementMinute(stamp);
}



/*
**
** This function will calculate what day of the week corresponds
** to the specified pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
** A pointer to the text string that specifies the day of the week
** is returned.
**
*/
char *
pix_sysGetFullDayOfWeek(pix_sys_date_t * stamp)
{
    short d, i, m, y;

    d = stamp->day;
    m = stamp->month;
    y = stamp->year;
    if ((m == 1) || (m == 2)) {
	m += 12;
	--y;
    }
    i = (d + (2 * m) + ((3 * (m + 1)) / 5) + y + (y / 4) - (y / 100) +
	 (y / 400)) % 7;
    return (full_day_list[i]);
}



/*
**
** This function will calculate what day of the week corresponds
** to the specified pix_sys_date_t structure.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
** A pointer to the text string that specifies the day of the week
** is returned.
**
*/
char *
pix_sysGetShortDayOfWeek(pix_sys_date_t * stamp)
{
    short d, i, m, y;

    d = stamp->day;
    m = stamp->month;
    y = stamp->year;
    if ((m == 1) || (m == 2)) {
	m += 12;
	--y;
    }
    i = (d + (2 * m) + ((3 * (m + 1)) / 5) + y + (y / 4) - (y / 100) +
	 (y / 400)) % 7;
    return (short_day_list[i]);
}



/*
**
** This function will recover a pointer to a text string that describes
** the specified month with a three letter abbreviation.
**
** The "month" parameter specifies the month's numeric value.
**
** If successful, a pointer to the text string for the specified month
** is returned. If an error occures during function execution, a pointer
** to an empty string is returned.
**
*/
char *
pix_sysGetShortMonth(unsigned char month)
{
    if ((month < 1) || (month > 12))
	return (NULL);
    else
	return (short_month_list[month]);
}



/*
**
** This function will recover a pointer to a text string that describes
** the specified month.
**
** The "month" parameter specifies the month's numeric value.
**
** If successful, a pointer to the text string for the specified month
** is returned. If an error occures during function execution, a pointer
** to an empty string is returned.
**
*/
char *
pix_sysGetFullMonth(unsigned char month)
{
    if ((month < 1) || (month > 12))
	return (NULL);
    else
	return (full_month_list[month]);
}



/*
**
** This function will recover the number of days in the month specified
** in the associated pix_sys_date_t. Leap year calculations are included.
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure.
**
** If successful, the number of days in the specified month are returned.
** If an error occures during function execution, '0' is returned.
**
*/
int
pix_sysGetDaysInMonth(pix_sys_date_t * stamp)
{
    if ((stamp->month == 0) || (stamp->month > 12))
	return (0);
    if (isLeapYear(stamp->year))
	return (leap_days_in_month[stamp->month]);
    else
	return (reg_days_in_month[stamp->month]);
}



/*
**
** This function will create an ASCIIZ string that represents the date
** and time found in the specified pix_sys_date_t. The resulting string will
** be in one of the following formats:
**
** format=0:   June 27, 2001
** format=1:   June 27, 2001, 10:58 AM
** format=2:   June 27, 2001, 10:58:30 AM
** format=3:   Wednesday, June 27, 2001
** format=4:   Wednesday, June 27, 2001, 10:58 AM
** format=5:   Wednesday, June 27, 2001, 10:58:30 AM
** format=6:   06/27/01
** format=7:   06/27/01 10:58a
** format=8:   06/27/01 10:58:30a
** format=9:   Wed 06/27/01
** format=10:  Wed 06/27/01 10:58a
** format=11:  Wed 06/27/01 10:58:30a
** format=12:  27 Jun 2001
** format=13:  27 Jun 2001 10:58 AM
** format=14:  27 Jun 2001 10:58:00 AM
** format=15:  Wed 27 Jun 2001
** format=16:  Wed 27 Jun 2001 10:58 AM
** format=17:  Wed 27 Jun 2001 10:58:30 AM
** format=18:  10:58 AM
** format=19:  10:58:30 AM
**
** The "stamp" parameter is a pointer to an initialized pix_sys_date_t
** structure. The "format" parameter specifies the string's format.
** The "result" parameter is a pointer to the buffer where the 
** resulting ASCIIZ string will be stored.
**
*/
void
pix_sysGetTimeDateString(pix_sys_date_t * stamp, short format, char *result)
{
    char AP, ap;
    unsigned short hour, year;

    /* do AM/PM conversion */
    hour = stamp->hour;
    if (hour == 0) {
	hour = 12;
	AP = 'A';
	ap = 'a';
    } else if ((hour > 0) && (hour < 12)) {
	AP = 'A';
	ap = 'a';
    } else if (hour == 12) {
	AP = 'P';
	ap = 'p';
    } else {
	hour -= 12;
	AP = 'P';
	ap = 'p';
    }

    /* abbreviate year */
    year = stamp->year % 100;

    /* generate date string */
    switch (format) {
	/* June 27, 2001 */
    case 0:
	sprintf(result,
		"%s %d, %d",
		full_month_list[stamp->month], stamp->day, stamp->year);
	break;

	/* June 27, 2001, 10:58 AM */
    case 1:
	sprintf(result,
		"%s %d, %d, %d:%02d %cM",
		full_month_list[stamp->month],
		stamp->day, stamp->year, hour, stamp->minute, AP);
	break;

	/* June 27, 2001, 10:58:30 AM */
    case 2:
	sprintf(result,
		"%s %d, %d, %d:%02d:%02d %cM",
		full_month_list[stamp->month],
		stamp->day,
		stamp->year, hour, stamp->minute, stamp->second, AP);
	break;

	/* Wednesday, June 27, 2001 */
    case 3:
	sprintf(result,
		"%s, %s %d, %d",
		pix_sysGetFullDayOfWeek(stamp),
		full_month_list[stamp->month], stamp->day, stamp->year);
	break;

	/* Wednesday, June 27, 2001, 10:58 AM */
    case 4:
	sprintf(result,
		"%s, %s %d, %d, %d:%02d %cM",
		pix_sysGetFullDayOfWeek(stamp),
		full_month_list[stamp->month],
		stamp->day, stamp->year, hour, stamp->minute, AP);
	break;

	/* Wednesday, June 27, 2001, 10:58:30 AM */
    case 5:
	sprintf(result,
		"%s, %s %d, %d, %d:%02d:%02d %cM",
		pix_sysGetFullDayOfWeek(stamp),
		full_month_list[stamp->month],
		stamp->day,
		stamp->year, hour, stamp->minute, stamp->second, AP);
	break;

	/* 06/27/01 */
    case 6:
	sprintf(result, "%02d/%02d/%02d", stamp->month, stamp->day, year);
	break;

	/* 06/27/01 10:58a */
    case 7:
	sprintf(result,
		"%02d/%02d/%02d %d:%02d%c",
		stamp->month, stamp->day, year, hour, stamp->minute, ap);
	break;

	/* 06/27/01 10:58:30a */
    case 8:
	sprintf(result,
		"%02d/%02d/%02d %d:%02d:%02d%c",
		stamp->month,
		stamp->day, year, hour, stamp->minute, stamp->second, ap);
	break;

	/*  Wed 06/27/01 */
    case 9:
	sprintf(result,
		"%s %02d/%02d/%02d",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->month, stamp->day, year);
	break;

	/* Wed 06/27/01 10:58a */
    case 10:
	sprintf(result,
		"%s %02d/%02d/%02d %d:%02d%c",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->month, stamp->day, year, hour, stamp->minute, ap);
	break;

	/* Wed 06/27/01 10:58:30a */
    case 11:
	sprintf(result,
		"%s %02d/%02d/%02d %d:%02d:%02d%c",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->month,
		stamp->day, year, hour, stamp->minute, stamp->second, ap);
	break;

	/* 27 Jun 2001 */
    case 12:
	sprintf(result,
		"%d %s %d",
		stamp->day, short_month_list[stamp->month], stamp->year);
	break;

	/* 27 Jun 2001 10:58 AM */
    case 13:
	sprintf(result,
		"%d %s %d %d:%02d %cM",
		stamp->day,
		short_month_list[stamp->month],
		stamp->year, hour, stamp->minute, AP);
	break;

	/* 27 Jun 2001 10:58:00 AM */
    case 14:
	sprintf(result,
		"%d %s %d %d:%02d:%02d %cM",
		stamp->day,
		short_month_list[stamp->month],
		stamp->year, hour, stamp->minute, stamp->second, ap);
	break;

	/* Wed 27 Jun 2001 */
    case 15:
	sprintf(result,
		"%s %d %s %d",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->day, short_month_list[stamp->month], stamp->year);
	break;

	/* Wed 27 Jun 2001 10:58 AM */
    case 16:
	sprintf(result,
		"%s %d %s %d %d:%02d %cM",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->day,
		short_month_list[stamp->month],
		stamp->year, hour, stamp->minute, AP);
	break;

	/* Wed 27 Jun 2001 10:58:30 AM */
    case 17:
	sprintf(result,
		"%s %d %s %d %d:%02d:%02d %cM",
		pix_sysGetShortDayOfWeek(stamp),
		stamp->day,
		short_month_list[stamp->month],
		stamp->year, hour, stamp->minute, stamp->second, AP);
	break;

	/* 10:58 AM */
    case 18:
	sprintf(result, "%d:%02d %cM", hour, stamp->minute, AP);
	break;

	/* 10:58:30 AM */
    case 19:
	sprintf(result,
		"%d:%02d:%02d %cM", hour, stamp->minute, stamp->second, AP);
	break;
    }
}
