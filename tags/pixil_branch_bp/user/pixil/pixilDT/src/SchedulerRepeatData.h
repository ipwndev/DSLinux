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
#ifndef SCHEDULERREPEATDATA_H_

#define SCHEDULERREPEATDATA_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

//--------------------------------------------------------------//
// Class to perform all manipulations on repetition settings    //
// for an event.  Vurrent field usage is:                       //
//                                                              //
// Repeat Flag 1 - type of repetition for the event, None,      //
//        Daily, Weekly, Monthly or Yearly.                     //
// Repeat Flag 2 - number of units between repetitions.         //
//--------------------------------------------------------------//
class SchedulerRepeatData
{
  public:SchedulerRepeatData(int nStartTime,
			// Constructor
			int nEndTime, int nRepeatFlag1 =
			0, int nRepeatFlag2 = 0, int nRepeatFlag3 =
			0, int nRepeatWeekMonth = 0);
      SchedulerRepeatData(const SchedulerRepeatData & Other);	// Copy constructor
    inline bool operator==(const SchedulerRepeatData & Other) const	// Comparison operator
    {
	return (m_nRepeatFlag1 == Other.m_nRepeatFlag1
		&& m_nRepeatFlag2 == Other.m_nRepeatFlag2
		&& m_nRepeatFlag3 == Other.m_nRepeatFlag3
		&& m_nRepeatWeekMonth == Other.m_nRepeatWeekMonth
		&& m_nEndTime == Other.m_nEndTime
		&& m_nStartTime == Other.m_nStartTime);
    }
    inline time_t GetEndDate() const	// Get the end date for this event
    {
	return (m_nRepeatFlag3);
    }
    inline bool GetRepeatByDow(int nDow) const	// Get whether this event repeats on this day of the week
    {
	return (((m_nRepeatWeekMonth >> nDow) & 0x01) == 0x01 ? true : false);
    }
    time_t GetRepeatDate(int nIndex) const;	// Get a date on which this event will repeat
    inline int GetRepeatEvery() const	// Get the number or days/weeks/months/years between repetitions
    {
	return (m_nRepeatFlag2 >=
		1 ? (m_nRepeatFlag2 <= 999 ? m_nRepeatFlag2 : 999) : 1);
    }
    inline int GetRepeatFlag1() const	// Get repeat flag 1
    {
	return (m_nRepeatFlag1);
    }
    inline int GetRepeatFlag2() const	// Get repeat flag 2
    {
	return (m_nRepeatFlag2);
    }
    inline int GetRepeatFlag3() const	// Get repeat flag 3
    {
	return (m_nRepeatFlag3);
    }
    int GetRepeatIndex() const;	// Get the repeat type as an index 0 through 4
    inline bool GetRepeatMonthByDay() const	// Get if the repeat monthly by week and day of week is true
    {
	return ((m_nRepeatWeekMonth & REPEAT_MONTH_DAY) != 0);
    }
    inline int GetRepeatType()	// Get the repeat type
    {
	return (m_nRepeatFlag1);
    }
    string GetRepeatTypeString() const;	// Get the repeat type as a string
    inline int GetRepeatWeekMonth() const	// Get weekly/monthly repeat flag
    {
	return (m_nRepeatWeekMonth);
    }
    void GetStartEnd(int &nStartSlot,	// Get the start and end time slots for drawing this appointment
		     int &nEndSlot, int nSlotSize) const;
    time_t GetStartTime() const	// Get the start time for this event
    {
	return (m_nStartTime);
    }
    void GetYearlyEvents(bool * pbEvent,	// Get indications as to which days of a year have scheduled events
			 time_t nYear) const;
    bool IsOnDay(time_t nTime) const;	// Does this event occur on a particular date
    bool IsWeekDaySelected(int nDow) const;	// Get whether a weekday has been selected or not
    void SetDailyRepetition(int nEvery,	// Set that the event will repeat on a daily basis
			    time_t nEndDate);
    void SetMonthlyRepetition(int nEvery,	// Set that the event will repeat on a monthly basis
			      int nRepeatType, time_t nEndDate);
    void SetNoRepetition();	// Set that the event will not repeat
    void SetStartDate(time_t nDate);	// Change the start date
    void SetWeeklyRepetition(int nEvery,	// Set that the event will repeat on a weekly basis
			     int nDow0, int nDow1, int nDow2, int nDow3,
			     int nDow4, int nDow5, int nDow6,
			     time_t nEndDate);
    void SetYearlyRepetition(int nEvery,	// Set that the event will repeat on a yearly basis
			     time_t nEndDate);
  private:int m_nRepeatFlag1;
    // First repeat flag
    int m_nRepeatFlag2;		// Second repeat flag
    int m_nRepeatFlag3;		// Third repeat flag
    int m_nRepeatWeekMonth;	// Weekly/Monthly repeat flags
    time_t m_nEndTime;		// End date/time of the event
    time_t m_nStartTime;	// Start date/time of the event
};


#endif /*  */
