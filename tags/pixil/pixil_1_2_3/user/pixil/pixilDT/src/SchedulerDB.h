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
// Scheduler database class.                                    //
//--------------------------------------------------------------//
#ifndef SCHEDULERDB_H_

#define SCHEDULERDB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <map>
#include <string>
#include <vector>
#include "NxDbAccess.h"
using namespace std;

//--------------------------------------------------------------//
// (08/15/2001) - There have been some discrepancies in the     //
// written documentation about the contents of the Scheduler    //
// data base fields. Here is what this class uses for each      //
// field:                                                       //
//                                                              //
// ID - database assigned ID number, unique to all rows in the  //
//      data base, but not necessarily equal to the physical    //
//      row number.                                             //
// Category - not used, always 0.                               //
// Start Time - the number of seconds (time_t value) for the    //
//      start time of the appointment.                          //
// End Time - the number of seconds (time_t value) for the end  //
//      time of the appointment.                                //
// All Day Flag - a value of 1 can indicate that the            //
//      appointment spans more than one day or lasts until the  //
//      end of the day.  This is not currently implemented, so  //
//      this field is always zero.                              //
// Repeat Flag 1 - indicates whether the appointment repeats or //
//      not, see the defines below.                             //
// Repeat Flag 2 - indicates the number of units between each   //
//      repetition of an event.  Example: a 2 here for a daily  //
//      event indicates that the event occurs every 2 days.     //
// Repeat Flag 3 - (as of 08/22/2001) the end date of the       //
//      repetition (as a time_t data type) or a zero if there   //
//      is no end date.                                         //
//      (Up until 08/22/2001 this had been the number of times  //
//      that an event will repeat.  The bits 0xf000 must be on  //
//      if there is an end time for the repetition.  If these   //
//      bits are on then the bits in the 0x0fff position of the //
//      field contain a repeat count.  The end repetition date  //
//      can be calculated useing the start time and the repeat  //
//      count.  The bits 0xffff0000 are not used.)              //
//                                                              //
// Weekly/Monthly Repeat Flag - contains more information about //
//      how a weekly or monthly repetition should occur.  For a //
//      weekly repetition, this contains bits for each day of   //
//      the week on which the event should occur.  For a        //
//      monthly repetition, there are two different bits which  //
//      indicate whether an event should occur on the same day  //
//      of month or on the same day-of-week and week of the     //
//      month.  For example, a month event can repeat on the    //
//      tenth of the month or on the second Tuesday of the      //
//      month.                                                  //
// Entry Type - indicates whether the event is an appointment   //
//      or a task.  Zero for an appointment or 1 for a task.    //
//      Currently not implemented, will always be zero.         //
// Description - the description of the event.                  //
//                                                              //
// 10/11/2001 - next update:                                    //
//                                                              //
// Exception Flag - This flag has two documented values.        //
//      SCHED_EXCEPTION was to be used to denote that the       //
//      record represents an override to a repetition of a      //
//      repeating event.  SCHED_DELETED_EXCEPTION indicates     //
//      that this is a repetition of a repeating event that     //
//      will not occur.  As per word-of-mouth, the              //
//      SCHED_EXCEPTION bit is not to be used on its own but    //
//      only in conjunction with the SCHED_DELETED_EXCEPTION    //
//      setting - a row must have both or neither of these on.  //
//      When a single instance of a repetition is to be         //
//      changed, the original will be deleted with a            //
//      SCHED_DELETED_EXCEPTION row (and the other bit as well) //
//      and then a new event will be added as a normal          //
//      appointment.                                            //
// Recno Pointer - for a row with the SCHED_EXCEPTION setting   //
//      in the exception flag, this will be the record number   //
//      (RECNO key value) of the original repeating row from    //
//      which the current row is an exception.                  //
// Alarm Interval - a value from 0 through 99 indicating the    //
//      number of time units prior to the event to sound an     //
//      alarm.  The Alarm Flags field will indicate whether     //
//      this event should have an alarm or not.                 //
// Alarm Flags - indicates what type of time units are used for //
//      the alarm interval.  See below for the actual values.   //
//                                                              //
// 10/12/2001 - More verbal specs:                              //
//   1) A deleted exception cannot be deleted by the user.      //
//      That means that once a user has changed a subsequent    //
//      repetition of an event causing a deleted exception and  //
//      a new normal event to be inserted into the data base,   //
//      the user cannot revert to the data base prior to the    //
//      change.                                                 //
//   2) Palm Desktop has what seems to be a bug - taking a      //
//      repeating event and changing the way in which it        //
//      repeats (weekly to monthly repetition) and requesting   //
//      that all events be changed will not change events prior //
//      to the date of the event selected for editing.          //
//      According to the Scheduler written for the PDA this     //
//      should change every event even in the past.             //
//   3) Changing only the description of an event should not    //
//      cause the dialog about changing all or only one event   //
//      to appear.  This should change the root record for the  //
//      repetition.                                             //
//--------------------------------------------------------------//

// Field References
#define SCHED_ID                0
#define SCHED_CAT               1
#define SCHED_START_TIME        2
#define SCHED_END_TIME          3
#define SCHED_ALL_DAY_FLAG      4
#define SCHED_REPEAT_FLAG_1     5
#define SCHED_REPEAT_FLAG_2     6
#define SCHED_REPEAT_FLAG_3     7
#define SCHED_REPEAT_WEEK_MONTH 8
#define SCHED_ENTRY_TYPE        9
#define SCHED_DESC             10
#define SCHED_EXCEPTION_FLAG   11
#define SCHED_RECNO_POINTER    12
#define SCHED_ALARM_INTERVAL   13
#define SCHED_ALARM_FLAGS      14
#define SCHED_NUM_FIELDS       13

#define SCHED_DESC_LEN 100

// These flags are kept in Repeat_Flag_1
#define REPEAT_NONE           0x0000
#define REPEAT_DAILY          0x0001
#define REPEAT_WEEKLY         0x0002
#define REPEAT_MONTHLY        0x0004
#define REPEAT_YEARLY         0x0008

// These flags are kept in Week_Month_Repeat_Flag
#define REPEAT_WEEK_SUNDAY    0x0001
#define REPEAT_WEEK_MONDAY    0x0002
#define REPEAT_WEEK_TUESDAY   0x0004
#define REPEAT_WEEK_WEDNESDAY 0x0008
#define REPEAT_WEEK_THURSDAY  0x0010
#define REPEAT_WEEK_FRIDAY    0x0020
#define REPEAT_WEEK_SATURDAY  0x0040
#define REPEAT_WEEK_FLAGS     0x007f

#define REPEAT_MONTH_DAY      0x0080
#define REPEAT_MONTH_DATE     0x0100

// These flags are kept in Exception
#define SCHED_EXCEPTION       0x0001
#define SCHED_DELETED_EXCEPTION 0x0002

// These flags are kept in Alarm Flags
#define SCHED_ALARM_MINUTES   0x0000
#define SCHED_ALARM_HOURS     0x0001
#define SCHED_ALARM_DAYS      0x0002
#define SCHED_NO_ALARM        ((signed short)0xffff)	// Treated as a -1

// Include this after the above includes
#include "SchedulerRepeatData.h"

#define SCHED_NOTES_PREFIX "scd_"
class SchedulerDB:public NxDbAccess
{
  public:SchedulerDB();	// Constructor
    ~SchedulerDB();		// Destructor
    void AddDeletedException(int nRow,	// Add a deleted exception for a given date
			     time_t nDate);
    int CopyRow(int nRow);	// Insert a new row exactly like the given row
    void Delete(int nRow);	// Delete a row and all exceptions for it
    void EndRepetitions(int nRow,	// End repetitions on a given date
			time_t nDate);
    inline int GetAlarmFlags(int nRow)	// Get the alarm flags
    {
	return (GetIntValue(nRow, SCHED_ALARM_FLAGS));
    }
    inline int GetAlarmInterval(int nRow)	// Get the minutes/hours/days prior for the alarm
    {
	return (GetIntValue(nRow, SCHED_ALARM_INTERVAL));
    }
    int GetAllAppointments(time_t nDate,	// Get an array of physical record numbers that are events for the day
			   multimap < int, int >&mRecord);
    inline int GetAllDayFlag(int nRow) const	// Get the all day flag
    {
	return (GetIntValue(nRow, SCHED_ALL_DAY_FLAG));
    }
    inline int GetCategory(int nRow) const	// Get the Category
    {
	return (GetIntValue(nRow, SCHED_CAT));
    }
    inline string GetDescription(int nRow) const	// Get the description
    {
	return (GetStringValue(nRow, SCHED_DESC));
    }
    inline int GetEndTime(int nRow) const	// Get the end time
    {
	return (GetIntValue(nRow, SCHED_END_TIME));
    }
    inline int GetEntryType(int nRow) const	// Get the entry type
    {
	return (GetIntValue(nRow, SCHED_ENTRY_TYPE));
    }
    void GetEvents(multimap < int, int >*pmEvent,	// Get all events for a range of days
		   time_t nDate, int nDays);
    inline int GetExceptionFlag(int nRow)	// Get the exception flag
    {
	return (GetIntValue(nRow, SCHED_EXCEPTION_FLAG));
    }
    inline int GetExceptionRecno(int nRow)	// Get the exception record number
    {
	return (GetIntValue(nRow, SCHED_RECNO_POINTER));
    }
    inline int GetID(int nRow) const	// Get the ID
    {
	return (GetIntValue(nRow, SCHED_ID));
    }
    inline bool GetRepeatingFlag(int nRow) const	// Get whether this event is a repeating event (or exception to a repeating event)
    {
	return ((GetIntValue(nRow, SCHED_REPEAT_FLAG_1) &
		 (REPEAT_DAILY | REPEAT_WEEKLY | REPEAT_MONTHLY |
		  REPEAT_YEARLY)) || (GetIntValue(nRow,
						  SCHED_EXCEPTION_FLAG) &
				      (SCHED_EXCEPTION |
				       SCHED_DELETED_EXCEPTION)));
    }
    inline int GetRepeatType(int nRow)	// Get the repeat type for a row
    {
	return (GetRepeatFlag1(nRow));
    }
    static const char *GetRepeatTypeString(int nRepeatType);	// Get the repeat type as a string
    static SchedulerDB *GetSchedulerDB();	// Get the singleton pointer
    SchedulerRepeatData *GetSchedulerRepeatData(int nRow) const;	// Get the repetition settings
    inline int GetStartTime(int nRow) const	// Get the start time
    {
	return (GetIntValue(nRow, SCHED_START_TIME));
    }
    string GetStartTimeString(int nRow) const;	// Get the start time as a string
    void GetYearlyEvents(bool * pEvent,	// Get flags for whether any day of the year has an event
			 int nYear);
    int Import(const vector < string > &strData);	// Import a delimited string
    int Insert();		// Insert a row and set its key value
    inline bool IsDeletedException(int nRow)	// Is this row a deleted exception
    {
	return ((GetExceptionFlag(nRow) & SCHED_DELETED_EXCEPTION) != 0);
    }
    inline bool IsException(int nRow)	// Is this row an exception
    {
	return ((GetExceptionFlag(nRow) & SCHED_EXCEPTION) != 0);
    }
    bool MoveStartTime(int nRow,	// Change the start and end times of an event
		       int nSeconds);
    void RemoveExceptions(int nRow);	// Remove all deleted exceptions related to a row
    bool RepeatDataChanged(int nRow,	// Test if the repeat data is different
			   SchedulerRepeatData * pRepeatData);
    inline void SetAlarmFlags(int nRow, int nFlags)	// Set the alarm flags
    {
	SetColumn(nRow, SCHED_ALARM_FLAGS, nFlags);
    }
    inline void SetAlarmInterval(int nRow, int nInterval)	// Set the minutes/hours/days prior for the alarm
    {
	SetColumn(nRow, SCHED_ALARM_INTERVAL, nInterval);
    }
    inline void SetAllDayFlag(int nRow, int nAllDayFlag)	// Set the all day flag
    {
	SetColumn(nRow, SCHED_ALL_DAY_FLAG, nAllDayFlag);
    }
    inline void SetCategory(int nRow, int nCategory)	// Set the Category
    {
	SetColumn(nRow, SCHED_CAT, nCategory);
    }
    inline void SetDescription(int nRow, const char *pszDesc)	// Set the description
    {
	SetColumn(nRow, SCHED_DESC, pszDesc);
    }
    inline void SetEndTime(int nRow, int nEndTime)	// Set the end time
    {
	SetColumn(nRow, SCHED_END_TIME, nEndTime);
    }
    inline void SetEntryType(int nRow, int nEntryType)	// Set the entry type
    {
	SetColumn(nRow, SCHED_ENTRY_TYPE, nEntryType);
    }
    inline void SetID(int nRow, int nID)	// Set the ID
    {
	SetColumn(nRow, SCHED_ID, nID);
    }
    void SetNoRepetition(int nRow);	// Set that this event does not repeat
    bool SetRoundedDuration(int nRow,	// Set the duration, but round the end to the nearest half hour
			    int nSeconds);
    void SetSchedulerRepeatData(int nRow,	// Set the repetition settings
				const SchedulerRepeatData *
				pSchedulerRepeatData);
    void SetStartDate(int nRow,	// Set the start date and move the end date to the same date
		      time_t nDate, int nTime);
    inline void SetStartTime(int nRow, int nStartTime)	// Set the start time
    {
	SetColumn(nRow, SCHED_START_TIME, nStartTime);
    }
  private:static SchedulerDB *m_pThis;
    // One and only object
    inline int GetRepeatFlag1(int nRow) const	// Get the first repeat flag
    {
	return (GetIntValue(nRow, SCHED_REPEAT_FLAG_1));
    }
    inline int GetRepeatFlag2(int nRow) const	// Get the second repeat flag
    {
	return (GetIntValue(nRow, SCHED_REPEAT_FLAG_2));
    }
    inline int GetRepeatFlag3(int nRow) const	// Get the third repeat flag
    {
	return (GetIntValue(nRow, SCHED_REPEAT_FLAG_3));
    }
    inline int GetRepeatWeekMonth(int nRow) const	// Get the repeat weekly/monthly flags
    {
	return (GetIntValue(nRow, SCHED_REPEAT_WEEK_MONTH));
    }
    bool HasDeletedException(int nRow,	// Test whether this row has a deleted exception for a given date
			     time_t nDate);
    inline void SetExceptionFlag(int nRow, int nException)	// Set the exception type
    {
	SetColumn(nRow, SCHED_EXCEPTION_FLAG, nException);
    }
    inline void SetExceptionRecno(int nRow, int nRecno)	// Set the exception parent record number
    {
	SetColumn(nRow, SCHED_RECNO_POINTER, nRecno);
    }
    inline void SetRepeatFlag1(int nRow, int nRepeatFlag1)	// Set the first repeat flag
    {
	SetColumn(nRow, SCHED_REPEAT_FLAG_1, nRepeatFlag1);
    }
    inline void SetRepeatFlag2(int nRow, int nRepeatFlag2)	// Set the second repeat flag
    {
	SetColumn(nRow, SCHED_REPEAT_FLAG_2, nRepeatFlag2);
    }
    inline void SetRepeatFlag3(int nRow, int nRepeatFlag3)	// Set the third repeat flag
    {
	SetColumn(nRow, SCHED_REPEAT_FLAG_3, nRepeatFlag3);
    }
    inline void SetRepeatWeekMonth(int nRow, int nRepeatWeekMonth)	// Set the repeat weekly/monthly flags
    {
	SetColumn(nRow, SCHED_REPEAT_WEEK_MONTH, nRepeatWeekMonth);
    }
};


#endif /*  */
