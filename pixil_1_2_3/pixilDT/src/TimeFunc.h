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

//--------------------------------------------------------------//
// Date/Time utilities.  These functions process dates and      //
// times.  Many of the calculation routines will determine the  //
// number of days, weeks, months or years between two dates     //
// taking daylight savings time into consideration.  Otherwise  //
// the possible one hour difference in the duration of a day    //
// might produce incorrect results.                             //
//--------------------------------------------------------------//
#ifndef TIMEFUNC_H_

#define TIMEFUNC_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
using namespace std;
time_t AddDays(time_t nDate,	// Add some number of days and return the new date at midnight
	       int nDays);
int DaysBetween(time_t nDate1,	// Get the number of days between two dates
		time_t nDate2);
string FormatDate(time_t nDate);	// Format the date portion of a date/time
string FormatDayOfWeek(time_t nDate);	// Get the day-of-week for a date/time
string FormatMonth(time_t nDate);	// Format the month as "longname, year"
string FormatShortTime(time_t nDate);	// Format the time portion of a date/time as HH[:MM](AM/PM)
string FormatTime(time_t nDate);	// Format the time portion of a date/time as HH:MM (AM/PM)
const char *GetDateError(int nError);	// Get the text for a date error from ValidateDate
string GetDayOfWeek(int nDay);	// Get the day of week for a date
string GetDayOfWeek(time_t nDate);	// Get the day of week for a date
string GetDayOfWeekAbbr(int nDay);	// Get the day of week abbreviation for a date
string GetDayOfWeekAbbr(time_t nDate);	// Get the day of week abbreviation for a date
int GetDow(time_t nDate);	// Get the day of week for a date
string GetMonthAbbr(int nMonth);	// Get the abbreviated name of a month
string GetMonthAbbr(time_t nDate);	// Get the abbreviated name of a month
string GetMonthName(int nMonth);	// Get the name of a month
string GetMonthName(time_t nDate);	// Get the name of a month
int GetMonthWeek(time_t nDate);	// Get the week of the month
const char *GetTimeError(int nError);	// Get the text for a time error from ValidateTime
bool IsDayOfMonth(time_t nDate,	// Is this day on a particular day of month (the one used by the second date)
		  time_t nDom);
bool IsDayOfYear(time_t nDate,	// Is this day on the same month and day (as the second date)
		 time_t nDoy);
time_t MakeDate(int nYear,	// Make a date for midnight of the given day
		int nMonth, int nDay);
int MonthsBetween(time_t nDate1,	// Get the number of months between two dates
		  time_t nDate2);
time_t NormalizeDate(time_t nDate);	// Normalize a date to midnight
bool TestNumeric(const string & strData);	// Test a string for all numeric characters
int ValidateDate(const char *pszDate,	// Validate a date entry string
		 time_t & nDate);
int ValidateTime(const char *pszTime,	// Validate a time entry string
		 int &nTime);
int WeeksBetween(time_t nDate1,	// Determine the number of weeks between two dates
		 time_t nDate2);
int YearsBetween(time_t nDate1,	// Get the number of years between the January 1st's prior to the two dates
		 time_t nDate2);

//--------------------------------------------------------------//
// Inline functions.                                            //
//--------------------------------------------------------------//
inline int
DaysBetween(time_t nDate1,	// Get the number of days between two dates accounting for daylight savings time
	    time_t nDate2)
{

#ifdef DEBUG
    assert(nDate1 == NormalizeDate(nDate1));	// Must be "normalized" dates (midnight)
    assert(nDate2 == NormalizeDate(nDate2));	// Must be "normalized" dates (midnight)
#endif /*  */
    return ((nDate1 - nDate2 +
	     (12 * 60 * 60 - 1) * (nDate1 >=
				   nDate2 ? 1 : -1)) / (24 * 60 * 60));
}
inline int
GetDow(int nYear,		// Get the day of week for a date
       int nMonth, int nDay)
{
    return (GetDow(MakeDate(nYear, nMonth, nDay)));
}
inline int
GetMonthWeek(int nDay)		// Get the week of the month, (first Tuesday will always be week 0)
{
    return (nDay / 7);
}

inline time_t
SubtractDays(time_t nDate,	// Subtract some number of days from a date and return a new date at midnight
	     int nDays)
{
    return (AddDays(nDate, -nDays));
}
inline int
WeeksBetween(int nYear1,	// Get the number of weeks between the Sunday's prior to the two dates
	     int nMonth1, int nDay1, time_t nDate2)
{
    return (WeeksBetween(MakeDate(nYear1, nMonth1, nDay1), nDate2));
}


//--------------------------------------------------------------//
// Second level of inline functions.                            //
//--------------------------------------------------------------//
inline int
DaysBetween(int nYear1,		// Get the number of days between two dates
	    int nMonth1, int nDay1, time_t nDate2)
{
    return (DaysBetween(MakeDate(nYear1, nMonth1, nDay1), nDate2));
}


#endif /*  */
