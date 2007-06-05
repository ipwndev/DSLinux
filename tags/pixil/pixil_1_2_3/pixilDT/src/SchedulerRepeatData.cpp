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
// Class for processing Scheduler data base repeat settings for //
// a row.                                                       //
//--------------------------------------------------------------//
#include <ctime>
#include "SchedulerDB.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerRepeatData::SchedulerRepeatData(int nStartTime,
					 int nEndTime,
					 int nRepeatFlag1,
					 int nRepeatFlag2,
					 int nRepeatFlag3,
					 int nRepeatWeekMonth)
{
    // Save all of these values
    m_nStartTime = nStartTime;
    m_nEndTime = nEndTime;
    m_nRepeatFlag1 = nRepeatFlag1;
    m_nRepeatFlag2 = nRepeatFlag2;
    m_nRepeatFlag3 = nRepeatFlag3;
    m_nRepeatWeekMonth = nRepeatWeekMonth;
}


//--------------------------------------------------------------//
// Copy constructor                                             //
//--------------------------------------------------------------//
SchedulerRepeatData::SchedulerRepeatData(const SchedulerRepeatData & Other)
{
    m_nStartTime = Other.m_nStartTime;
    m_nEndTime = Other.m_nEndTime;
    m_nRepeatFlag1 = Other.m_nRepeatFlag1;
    m_nRepeatFlag2 = Other.m_nRepeatFlag2;
    m_nRepeatFlag3 = Other.m_nRepeatFlag3;
    m_nRepeatWeekMonth = Other.m_nRepeatWeekMonth;
}


//--------------------------------------------------------------//
// Get a repeat date given an index.  The returned time will be //
// normalized to midnight.                                      //
//--------------------------------------------------------------//
time_t
SchedulerRepeatData::GetRepeatDate(int nIndex) const
{
    int i;
    int nCount;
    int nDays;
    int nDow;
    int nDow2;
    int nIndex2;
    int nMonth;
    int nOffset = 0;
    int nRepeat;
    int nWeek;
    int nWeekFlag;
    time_t nReturn;
    struct tm *pTm;

    switch (m_nRepeatFlag1) {
    case REPEAT_NONE:
	if (nIndex == 0) {
	    // Only happens on this day
	    nReturn =::NormalizeDate(m_nStartTime);
	} else {
	    // Does not repeat
	    nReturn = 0;
	}
	break;

    case REPEAT_DAILY:
	nReturn =::AddDays(::NormalizeDate(m_nStartTime),
			   nIndex * GetRepeatEvery());
	if (GetEndDate() >= 24 * 60 * 60
	    && nReturn >::NormalizeDate(GetEndDate())) {
	    // Too far into the future
	    nReturn = 0;
	}
	break;

    case REPEAT_WEEKLY:
	// Get the first day for this event
	nDow = GetDow(m_nStartTime);
	nReturn =::NormalizeDate(m_nStartTime);
	for (i = nDow; i < 7; ++i) {
	    if (((0x01 << i) & m_nRepeatWeekMonth) != 0) {
		break;
	    } else {
		nReturn =::AddDays(nReturn, 1);
	    }
	}
	if (i >= 7) {
	    // Try again into the next week
	    for (i = 0; i < nDow; ++i) {
		if (((0x01 << i) & m_nRepeatWeekMonth) != 0) {
		    break;
		} else {
		    nReturn =::AddDays(nReturn, 1);
		}
	    }
#ifdef DEBUG
	    assert(i < nDow);	// Must have found a day
#endif
	}
	// Reset the index if this is for a weekly event
	pTm = localtime(&nReturn);
	switch (pTm->tm_wday) {
	case 6:		// On a Saturday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_FRIDAY) != 0 ? 1 : 0);
	    // Fall through

	case 5:		// On a Friday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_THURSDAY) != 0 ? 1 : 0);
	    // Fall through

	case 4:		// On a Thursday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_WEDNESDAY) != 0 ? 1 : 0);
	    // Fall through

	case 3:		// On a Wednesday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_TUESDAY) != 0 ? 1 : 0);
	    // Fall through

	case 2:		// On a Tuesday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_MONDAY) != 0 ? 1 : 0);
	    // Fall through

	case 1:		// On a Monday
	    nOffset +=
		((m_nRepeatWeekMonth & REPEAT_WEEK_SUNDAY) != 0 ? 1 : 0);
	}

	// Get the number of times per week
	nDow = ((m_nRepeatWeekMonth & REPEAT_WEEK_SUNDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_MONDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_TUESDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_WEDNESDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_THURSDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_FRIDAY) != 0 ? 1 : 0)
	    + ((m_nRepeatWeekMonth & REPEAT_WEEK_SATURDAY) != 0 ? 1 : 0);

	// Determine how many full weeks into the repetition
	nIndex2 = nIndex + nOffset;
	nIndex2 /= nDow;

	// Get the date of the Saturday prior to the Sunday of the week
	// in which this repetition will occur
	nReturn =::AddDays(nReturn,
			   7 * nIndex2 * GetRepeatEvery() - GetDow(nReturn) -
			   1);
	pTm = localtime(&nReturn);
	nRepeat = ((nIndex + nOffset) % nDow);
	nCount = 0;
	nWeekFlag = (m_nRepeatWeekMonth & REPEAT_WEEK_FLAGS);

	// Now offset this for the number of repetitions within the week
	while (nCount <= nRepeat) {
#ifdef DEBUG
	    assert(nWeekFlag != 0);	// Too many bits are off
#endif
	    while ((nWeekFlag & 0x01) == 0) {
		nReturn =::AddDays(nReturn, 1);
		nWeekFlag >>= 1;
	    }

	    // Use this day
	    nReturn =::AddDays(nReturn, 1);
	    nWeekFlag >>= 1;
	    ++nCount;
	}

	// Say that there isn't one if too far into the future
	if (GetEndDate() >= 24 * 60 * 60
	    && nReturn >::NormalizeDate(GetEndDate())) {
	    // Too far into the future
	    nReturn = 0;
	}
	break;

    case REPEAT_MONTHLY:
	switch (m_nRepeatWeekMonth) {
	case REPEAT_MONTH_DATE:	// Repeats on the day day of the month
	    // Go to the requested month
	    pTm = localtime(&m_nStartTime);
	    pTm->tm_isdst = -1;
	    pTm->tm_sec = 0;
	    pTm->tm_min = 0;
	    pTm->tm_hour = 0;
	    pTm->tm_mon += nIndex * GetRepeatEvery();
	    pTm->tm_year += pTm->tm_mon / 12;
	    pTm->tm_mon %= 12;
	    if (pTm->tm_mday > 30 && (pTm->tm_mon == 3	// April
				      || pTm->tm_mon == 5	// June
				      || pTm->tm_mon == 8	// September
				      || pTm->tm_mon == 10))	// November
	    {
		pTm->tm_mday = 30;
	    } else if (pTm->tm_mday > 28 && pTm->tm_mon == 1)	// February
	    {
		if ((pTm->tm_year % 4) == 0) {
		    if (pTm->tm_mday > 29) {
			pTm->tm_mday = 29;
		    }
		} else {
		    pTm->tm_mday = 28;
		}
	    }
	    nReturn = mktime(pTm);
	    break;

	case REPEAT_MONTH_DAY:	// Repeats on the week and weekday of the month
	    // Get the week and day of week
	    nWeek = GetMonthWeek(m_nStartTime);
	    nDow = GetDow(m_nStartTime);

	    // Go to the first of the requested month
	    pTm = localtime(&m_nStartTime);
	    pTm->tm_isdst = -1;
	    pTm->tm_sec = 0;
	    pTm->tm_min = 0;
	    pTm->tm_hour = 0;
	    pTm->tm_mday = 1;
	    pTm->tm_mon += nIndex * GetRepeatEvery();
	    pTm->tm_year += pTm->tm_mon / 12;
	    pTm->tm_mon %= 12;

	    // Save the month for later
	    nMonth = pTm->tm_mon;

	    // Get the first of the new month
	    nReturn = mktime(pTm);

	    // Get the number of days into the month
	    nDow2 =::GetDow(nReturn);
	    nDays = 7 * nWeek + nDow - nDow2 + (nDow < nDow2 ? 7 : 0);

	    // Get the first try at the date
	    nReturn =::AddDays(nReturn, nDays);

	    // Fix if past the end of the selected month
	    pTm = localtime(&nReturn);
	    while (pTm->tm_mon != nMonth) {
		nReturn =::SubtractDays(nReturn, 1);
		pTm = localtime(&nReturn);
	    }
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Bad monthly repeat flag
#endif
	    nReturn = 0;	// Say that it does not repeat here
	}

	// Say that there isn't one if too far into the future
	if (GetEndDate() >= 24 * 60 * 60
	    && nReturn >::NormalizeDate(GetEndDate())) {
	    // Too far into the future
	    nReturn = 0;
	}
	break;

    case REPEAT_YEARLY:
	// Go to the requested year
	pTm = localtime(&m_nStartTime);
	pTm->tm_isdst = -1;
	pTm->tm_sec = 0;
	pTm->tm_min = 0;
	pTm->tm_hour = 0;
	pTm->tm_year += nIndex * GetRepeatEvery();
	if (pTm->tm_mday > 28 && pTm->tm_mon == 1	// February
	    && (pTm->tm_year % 4) != 0) {
	    pTm->tm_mday = 28;
	}

	nReturn = mktime(pTm);

	// Say that there isn't one if too far into the future
	if (GetEndDate() >= 24 * 60 * 60
	    && nReturn >::NormalizeDate(GetEndDate())) {
	    // Too far into the future
	    nReturn = 0;
	}
	break;

    default:
#ifdef DEBUG
	assert(false);		// Invalid repetition type
#endif
	nReturn = 0;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Get the repeat type as a numeric index 0 through 4.          //
//--------------------------------------------------------------//
int
SchedulerRepeatData::GetRepeatIndex() const
{
    static int nRepeatType[5] =
	{ REPEAT_NONE, REPEAT_DAILY, REPEAT_WEEKLY, REPEAT_MONTHLY,
REPEAT_YEARLY };
    int nIndex;

    for (nIndex = 0; nIndex < 5; ++nIndex) {
	if (nRepeatType[nIndex] == m_nRepeatFlag1) {
	    break;
	}
    }

    if (nIndex >= 5) {
#ifdef DEBUG
	assert(false);		// Bad repeat flag 1
#endif
	nIndex = 0;
    }

    return (nIndex);
}


//--------------------------------------------------------------//
// Get the start and end slots for drawing this appointment.    //
//--------------------------------------------------------------//
void
SchedulerRepeatData::GetStartEnd(int &nStartSlot,
				 int &nEndSlot, int nSlotSize) const
{
    struct tm *pTm = localtime(&m_nStartTime);

    nStartSlot =
	(pTm->tm_hour * 60 * 60 + pTm->tm_min * 60 + pTm->tm_sec) / nSlotSize;
    pTm = localtime(&m_nEndTime);
    nEndSlot =
	(pTm->tm_hour * 60 * 60 + pTm->tm_min * 60 + pTm->tm_sec -
	 1) / nSlotSize;
    if (nEndSlot < nStartSlot) {
	nEndSlot = nStartSlot;
    }
}


//--------------------------------------------------------------//
// Get the repeat type, not inline because of recursive header  //
// includes.                                                    //
//--------------------------------------------------------------//
string
SchedulerRepeatData::GetRepeatTypeString() const
{
    return (SchedulerDB::GetRepeatTypeString(m_nRepeatFlag1));
}


//--------------------------------------------------------------//
// Mark the days of the year that have scheduled events.        //
// Assumes that pbEvent is an array of at least 366 bools.      //
//--------------------------------------------------------------//
void
SchedulerRepeatData::GetYearlyEvents(bool * pbEvent, time_t nTimeYear) const
{
    bool bDone = false;
    int i;
    int nYear;
    time_t nTime;
    struct tm *pTm;

    // Get the year
    pTm = localtime(&nTimeYear);
    nYear = pTm->tm_year;

    // Is the start date after the end of this year ?
    pTm = localtime(&m_nStartTime);
    if (pTm->tm_year > nYear) {
	bDone = true;
    }
    // Does this event end before the start of this year
    if (bDone == false) {
	nTime = GetEndDate();
	if (nTime >= 24 * 60 * 60) {
	    pTm = localtime(&nTime);
	    if (pTm->tm_year < nYear) {
		bDone = true;
	    }
	}
    }
    // Now test each repetition of the event against this year
    if (bDone == false) {
	i = 0;
	do {
	    // Get the next repeat date
	    nTime = GetRepeatDate(i);

	    // Is the event still repeating
	    if (nTime >= 24 * 60 * 60) {
		// Test the year of the event
		pTm = localtime(&nTime);
		if (pTm->tm_year == nYear) {
		    // Mark this day of year as having an event
		    pbEvent[pTm->tm_yday] = true;
		} else if (pTm->tm_year > nYear) {
		    // Past the year in question, quit
		    break;
		}
	    }
	    // Increment the repeat index
	    ++i;
	} while (nTime >= 24 * 60 * 60);
    }
}


//--------------------------------------------------------------//
// Test if event will occur on a particular day.                //
//--------------------------------------------------------------//
bool
SchedulerRepeatData::IsOnDay(time_t nTime) const
{
    bool bReturn = false;
    int nBetween;

#ifdef DEBUG
    assert(nTime ==::NormalizeDate(nTime));	// Must be a normalized date
#endif

    if (nTime >=::NormalizeDate(m_nStartTime)
	&& (GetEndDate() < 24 * 60 * 60
	    || nTime <=::NormalizeDate(GetEndDate()))) {
	switch (m_nRepeatFlag1) {
	case REPEAT_NONE:	// Does not repeat
	    if (nTime ==::NormalizeDate(m_nStartTime)) {
		bReturn = true;
	    }
	    break;

	case REPEAT_DAILY:	// Repeats on a daily basis
	    nBetween = DaysBetween(nTime,::NormalizeDate(m_nStartTime));
	    if ((nBetween % GetRepeatEvery()) == 0) {
		// This is on a correct day
		bReturn = true;
	    }
	    break;

	case REPEAT_WEEKLY:
	    if (IsWeekDaySelected(GetDow(nTime)) == true) {
		// On the correct weekday, get the number of weeks between the two dates
		nBetween = WeeksBetween(nTime,::NormalizeDate(m_nStartTime));

		// Is it on a good week
		if ((nBetween % GetRepeatEvery()) == 0) {
		    // This is on a correct week
		    bReturn = true;
		}
	    }
	    break;

	case REPEAT_MONTHLY:
	    // Determine how to repeat this event
	    switch (GetRepeatWeekMonth()) {
	    case REPEAT_MONTH_DAY:	// Repeats on the week and weekday of the month
		// Test if the target date is on the same day-of-week
		if (GetDow(m_nStartTime) == GetDow(nTime)) {
		    // Test if the target date is in the same week of the month
		    if (GetMonthWeek(m_nStartTime) == GetMonthWeek(nTime)) {
			// Now test for the "repeats" every setting
			nBetween = MonthsBetween(nTime, m_nStartTime);
			if ((nBetween % GetRepeatEvery()) == 0) {
			    // This is on a correct month
			    bReturn = true;
			}
		    }
		}
		break;

	    case REPEAT_MONTH_DATE:	// Repeats on the same day of the month
		if (IsDayOfMonth(nTime, m_nStartTime) == true) {
		    // Now test for the "repeats" every setting
		    nBetween = MonthsBetween(nTime, m_nStartTime);
		    if ((nBetween % GetRepeatEvery()) == 0) {
			// This is on a correct month
			bReturn = true;
		    }
		}
		break;

	    default:
#ifdef DEBUG
		assert(false);	// Incorrect type for month repeat
#endif
		;
	    }
	    break;

	case REPEAT_YEARLY:
	    // Is this on the correct month and day-of-month
	    if (IsDayOfYear(nTime, m_nStartTime) == true) {
		// Now test for the "repeats every setting
		nBetween = YearsBetween(nTime, m_nStartTime);
		if ((nBetween % GetRepeatEvery()) == 0) {
		    // This is on a correct year
		    bReturn = true;
		}
	    }
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Incorrect repeat flag 1
#endif
	    ;
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Test if a particular weekday has been selected or not.       //
// This test makes useof the fact that the Sunday through       //
// Saturday bits can be represented by a simple bit shift.      //
//--------------------------------------------------------------//
bool
SchedulerRepeatData::IsWeekDaySelected(int nDow) const
{
#ifdef DEBUG
    assert(nDow >= 0 && nDow < 7);
#endif

    return (((0x01 << (nDow)) & m_nRepeatWeekMonth) != 0);
}


//--------------------------------------------------------------//
// Set that the event repeats on a daily basis.  The nEvery     //
// argument is the number of days between repetitions and the   //
// nEndDate argument is the end day, the event will not repeat  //
// after this date.  If nEndDate is less than the event start   //
// date then this function will assume that there is no end     //
// date (no repetitions).                                       //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetDailyRepetition(int nEvery, time_t nEndDate)
{
    // Set the simple settings
    m_nRepeatFlag1 = REPEAT_DAILY;
    m_nRepeatFlag2 = (nEvery < 0 || nEvery > 999 ? 999 : nEvery);
    m_nRepeatWeekMonth = 0;

    // Set the end date
    m_nRepeatFlag3 =
	(nEndDate >= 24 * 60 * 60 ?::NormalizeDate(nEndDate) : 0);
}


//--------------------------------------------------------------//
// Set that the event repeats on a monthly basis.  The nEvery   //
// argument is the number of months between repetitions and the //
// nEndDate argument is the end day, the event will not repeat  //
// after this date.  If nEndDate is less than the event start   //
// date then this function will assume that there is no end     //
// date (no repetitions).  The nRepeatType argument indicates   //
// how the event repeats, a zero indicates that the date        //
// repeats on a particular day of the month while any other     //
// value indicates that the event repeats on the same day of    //
// week in the same week of the month as the original event.    //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetMonthlyRepetition(int nEvery,
					  int nRepeatType, time_t nEndDate)
{
    // Set the simple settings
    m_nRepeatFlag1 = REPEAT_MONTHLY;
    m_nRepeatFlag2 = (nEvery < 0 || nEvery > 999 ? 999 : nEvery);

    // Set the monthly repeat flag
    m_nRepeatWeekMonth =
	(nRepeatType == 0 ? REPEAT_MONTH_DATE : REPEAT_MONTH_DAY);

    // Set the end date
    m_nRepeatFlag3 =
	(nEndDate >= 24 * 60 * 60 ?::NormalizeDate(nEndDate) : 0);
}


//--------------------------------------------------------------//
// Set that the event does not repeat.                          //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetNoRepetition()
{
    m_nRepeatFlag1 = REPEAT_NONE;
    m_nRepeatFlag2 = 0;
    m_nRepeatFlag3 = 0;
    m_nRepeatWeekMonth = 0;
}


//--------------------------------------------------------------//
// Change the date for this repeating event.                    //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetStartDate(time_t nDate)
{
    // Set the starting time
    m_nStartTime = nDate + (m_nStartTime -::NormalizeDate(m_nStartTime));

    // Set the ending time
    m_nEndTime = nDate + (m_nEndTime -::NormalizeDate(m_nEndTime));
}


//--------------------------------------------------------------//
// Set that the event repeats on a weekly basis.  The nEvery    //
// argument is the number of weeks between repetitions and the  //
// nEndDate argument is the end day, the event will not repeat  //
// after this date.  If nEndDate is less than the event start   //
// date then this function will assume that there is no end     //
// date (no repetitions).  The number of repetitions represents //
// the number of full weeks, when an event occurs on more than  //
// one weekday and also repeats, the repetition includes all    //
// weekdays.                                                    //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetWeeklyRepetition(int nEvery,
					 int nDow0,
					 int nDow1,
					 int nDow2,
					 int nDow3,
					 int nDow4,
					 int nDow5,
					 int nDow6, time_t nEndDate)
{
    // Set the simple settings
    m_nRepeatFlag1 = REPEAT_WEEKLY;
    m_nRepeatFlag2 = (nEvery < 0 || nEvery > 999 ? 999 : nEvery);

#ifdef DEBUG
    assert(nDow0 != 0 || nDow1 != 0 || nDow2 != 0 || nDow3 != 0 || nDow4 != 0
	   || nDow5 != 0 || nDow6 != 0);
#endif

    // Set the days of the week on which to repeat the event
    m_nRepeatWeekMonth = (nDow0 != 0 ? REPEAT_WEEK_SUNDAY : 0)
	+ (nDow1 != 0 ? REPEAT_WEEK_MONDAY : 0)
	+ (nDow2 != 0 ? REPEAT_WEEK_TUESDAY : 0)
	+ (nDow3 != 0 ? REPEAT_WEEK_WEDNESDAY : 0)
	+ (nDow4 != 0 ? REPEAT_WEEK_THURSDAY : 0)
	+ (nDow5 != 0 ? REPEAT_WEEK_FRIDAY : 0)
	+ (nDow6 != 0 ? REPEAT_WEEK_SATURDAY : 0);

    // Set the end date
    m_nRepeatFlag3 =
	(nEndDate >= 24 * 60 * 60 ?::NormalizeDate(nEndDate) : 0);
}


//--------------------------------------------------------------//
// Set that the event repeats on a yearly basis.  The nEvery    //
// argument is the number of years between repetitions and the  //
// nEndDate argument is the end day, the event will not repeat  //
// after this date.  If nEndDate is less than the event start   //
// date then this function will assume that there is no end     //
// date (no repetitions).                                       //
//--------------------------------------------------------------//
void
SchedulerRepeatData::SetYearlyRepetition(int nEvery, time_t nEndDate)
{
    // Set the simple settings
    m_nRepeatFlag1 = REPEAT_YEARLY;
    m_nRepeatFlag2 = (nEvery < 0 || nEvery > 999 ? 999 : nEvery);
    m_nRepeatWeekMonth = 0;

    // Set the end date
    m_nRepeatFlag3 =
	(nEndDate >= 24 * 60 * 60 ?::NormalizeDate(nEndDate) : 0);
}
