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
// Scheduler database definition fields.                        //
//--------------------------------------------------------------//
#include "config.h"
#include <ctime>
#include "SchedulerDB.h"
#include "SchedulerDBDef.h"
#include "TimeFunc.h"

#include "FLTKUtil.h"
#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
SchedulerDB *
    SchedulerDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerDB::SchedulerDB()
:  NxDbAccess("sched", &sFile, sFields)
{
    bool bGood;
    int nRecno;
    int nRow;
    int nRow2;
    int nMax = NumRecs();

    // Examine the data base for bad exception record pointers
    for (nRow = 0; nRow < nMax; ++nRow) {
	// Process if this is an exception row
	if (!IsDeleted(nRow)) {
	    if (IsException(nRow)) {
		// Reset this flag
		bGood = true;

		// Find the parent repeating event
		nRecno = GetExceptionRecno(nRow);
		nRow2 = FindRow(SCHED_ID, nRecno);
		if (nRow2 < 0) {
		    // Exception for non-existent repeating event
		    bGood = false;
		} else if (GetRepeatFlag1(nRow2) == REPEAT_NONE) {
		    // Exception to a non-repeating event
		    bGood = false;
		}
		// Is this a bad row
		if (bGood == false) {
#ifdef DEBUG
		    assert(false);	// Bad exception row
#endif

		    // Delete this row
		    Delete(nRow);
		}
	    }
	}
    }
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SchedulerDB::~SchedulerDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Add a deleted exception row for a given root row and a       //
// selected date.                                               //
//--------------------------------------------------------------//
void
SchedulerDB::AddDeletedException(int nRow, time_t nDate)
{
    int nRowNew = Insert();
    time_t nTime;

#ifdef DEBUG
    assert(::NormalizeDate(nDate) == nDate);	// Date must be normalized
#endif

    // Set every field of the deleted exception
    nTime = GetStartTime(nRow);
    SetStartTime(nRowNew, nDate + (nTime -::NormalizeDate(nTime)));
    nTime = GetEndTime(nRow);
    SetEndTime(nRowNew, nDate + (nTime -::NormalizeDate(nTime)));
    SetAllDayFlag(nRowNew, 0);
    SetRepeatFlag1(nRowNew, 0);
    SetRepeatFlag2(nRowNew, 0);
    SetRepeatFlag3(nRowNew, 0);
    SetRepeatWeekMonth(nRowNew, 0);
    SetEntryType(nRowNew, 0);
    SetDescription(nRowNew, "");
    SetExceptionFlag(nRowNew, SCHED_EXCEPTION + SCHED_DELETED_EXCEPTION);
    SetExceptionRecno(nRowNew, GetID(nRow));
    SetAlarmInterval(nRowNew, 0);
    SetAlarmFlags(nRowNew, SCHED_NO_ALARM);
}


//--------------------------------------------------------------//
// Make a copy of a row.                                        //
//--------------------------------------------------------------//
int
SchedulerDB::CopyRow(int nRow)
{
    int nNewRow = Insert();

    SetCategory(nNewRow, GetCategory(nRow));
    SetStartTime(nNewRow, GetStartTime(nRow));
    SetEndTime(nNewRow, GetEndTime(nRow));
    SetAllDayFlag(nNewRow, GetAllDayFlag(nRow));
    SetRepeatFlag1(nNewRow, GetRepeatFlag1(nRow));
    SetRepeatFlag2(nNewRow, GetRepeatFlag2(nRow));
    SetRepeatFlag3(nNewRow, GetRepeatFlag3(nRow));
    SetRepeatWeekMonth(nNewRow, GetRepeatWeekMonth(nRow));
    SetEntryType(nNewRow, GetEntryType(nRow));
    SetDescription(nNewRow, GetDescription(nRow).c_str());
    SetExceptionFlag(nNewRow, GetExceptionFlag(nRow));
    SetExceptionRecno(nNewRow, GetExceptionRecno(nRow));
    SetAlarmInterval(nNewRow, GetAlarmInterval(nRow));
    SetAlarmFlags(nNewRow, GetAlarmFlags(nRow));

    return (nNewRow);
}


//--------------------------------------------------------------//
// Delete a row and all exceptions for the row.                 //
//--------------------------------------------------------------//
void
SchedulerDB::Delete(int nRow)
{
    // Delete the row
    NxDbAccess::Delete(nRow);

    // Delete all exceptions for this row
    RemoveExceptions(nRow);
}


//--------------------------------------------------------------//
// Set the ending date for a repeating event.                   //
//--------------------------------------------------------------//
void
SchedulerDB::EndRepetitions(int nRow, time_t nDate)
{
#ifdef DEBUG
    assert(nDate == 0 || nDate ==::NormalizeDate(nDate));	// The date must be normalized
#endif

    if (nDate == 0) {
	// Set no end date
	SetRepeatFlag3(nRow, 0);
    } else if (nDate < GetStartTime(nRow)) {
	// If ending prior to the start time then delete this event
	Delete(nRow);
    } else {
	// Set the ending date
	SetRepeatFlag3(nRow,::AddDays(nDate, 1) - 1);
    }
}


//--------------------------------------------------------------//
// Get all events for a given day.  The multimap will be        //
// returned as an array of row numbers that represent events    //
// for this date.                                               //
//--------------------------------------------------------------//
int
SchedulerDB::GetAllAppointments(time_t nDate, multimap < int, int >&mRecord)
{
    bool bRemove;
    int nKey;
    int nMax = NumRecs();
    int nRow;
    SchedulerRepeatData *pSchedulerRepeatData;
    time_t nEndTime;
    time_t nStartTime;

#ifdef DEBUG
    assert(nDate == NormalizeDate(nDate));	// Must be a "normalized" date (midnight)
#endif

    // Clear the vector
    mRecord.clear();

    // Process each row in the data base
    for (nRow = 0; nRow < nMax; ++nRow) {
	if (!IsDeleted(nRow) && !IsDeletedException(nRow)) {
	    pSchedulerRepeatData = GetSchedulerRepeatData(nRow);
	    if (pSchedulerRepeatData->IsOnDay(nDate) == true) {
		// Test for a deleted exception to a repeating event
		bRemove = false;
		if (GetRepeatFlag1(nRow) != REPEAT_NONE) {
		    if (HasDeletedException(nRow, nDate)) {
			bRemove = true;
		    }
		}
		// Add only if not deleted
		if (bRemove == false) {
		    // Get the appointment times
		    nStartTime = GetStartTime(nRow);
		    nEndTime = GetEndTime(nRow);

		    // Set up a key and add it to the map of events
		    nKey =
			1440 * ((nStartTime -::NormalizeDate(nStartTime)) /
				60)
			+ ((nEndTime -::NormalizeDate(nEndTime)) / 60);
		    mRecord.insert(make_pair(nKey, nRow));
		}
	    }
	    delete pSchedulerRepeatData;
	}
    }

    return (mRecord.size());
}


//--------------------------------------------------------------//
// Get all events for a range of days.                          //
//--------------------------------------------------------------//
void
SchedulerDB::GetEvents(multimap < int, int >*pmEvent, time_t nDate, int nDays)
{
    bool bRemove;
    int i;
    int nKey;
    int nMax = NumRecs();
    int nRow;
    SchedulerRepeatData *pSchedulerRepeatData;
    time_t nEndTime;
    time_t nStartTime;
    time_t nTime;

    // Clear out the older events
    for (i = 0; i < nDays; ++i) {
	pmEvent[i].clear();
    }

    // Look at each event
    for (nRow = 0; nRow < nMax; ++nRow) {
	if (!IsDeleted(nRow)
	    && !IsDeletedException(nRow)) {
	    // Get the repetition parameters
	    pSchedulerRepeatData = GetSchedulerRepeatData(nRow);

	    // Test each day in turn
	    for (i = 0; i < nDays; ++i) {
		nTime =::AddDays(nDate, i);
		if (pSchedulerRepeatData->IsOnDay(nTime) == true) {
		    // Test if this event has been removed via a repeating exception
		    bRemove = false;
		    if (GetRepeatFlag1(nRow) != REPEAT_NONE) {
			if (HasDeletedException(nRow, nTime)) {
			    bRemove = true;
			}
		    }
		    // Add only if not deleted
		    if (bRemove == false) {
			// Get the start and end times
			nStartTime = GetStartTime(nRow);
			nEndTime = GetEndTime(nRow);

			// Add this event to the map of events
			nKey =
			    1440 *
			    ((nStartTime -::NormalizeDate(nStartTime)) / 60)
			    + ((nEndTime -::NormalizeDate(nEndTime)) / 60);
			pmEvent[i].insert(make_pair(nKey, nRow));
		    }
		}
	    }

	    // Clean up after this event
	    delete pSchedulerRepeatData;
	}
    }
}


//--------------------------------------------------------------//
// Get the repeat type (repeat flag 1) as a string.             //
//--------------------------------------------------------------//
const char *
SchedulerDB::GetRepeatTypeString(int nRepeatType)
{
    static const char *pszRepeatType[5] = {
	N_("None"),
	N_("Daily"),
	N_("Weekly"),
	N_("Monthly"),
	N_("Yearly"),
    };
    int nIndex;

    switch (nRepeatType) {
    case REPEAT_NONE:
	nIndex = 0;
	break;

    case REPEAT_DAILY:
	nIndex = 1;
	break;

    case REPEAT_WEEKLY:
	nIndex = 2;
	break;

    case REPEAT_MONTHLY:
	nIndex = 3;
	break;

    case REPEAT_YEARLY:
	nIndex = 4;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown repeat type
#endif
	nIndex = 0;		// Call it not repeating
    }
    return (_(pszRepeatType[nIndex]));
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
SchedulerDB *
SchedulerDB::GetSchedulerDB()
{
    if (m_pThis == NULL) {
	m_pThis = new SchedulerDB;
    }
    return (m_pThis);
}


//--------------------------------------------------------------//
// Get the repeat settings.                                     //
//--------------------------------------------------------------//
SchedulerRepeatData *
SchedulerDB::GetSchedulerRepeatData(int nRow) const
{
    return (new SchedulerRepeatData(GetStartTime(nRow),
				    GetEndTime(nRow),
				    GetRepeatFlag1(nRow),
				    GetRepeatFlag2(nRow),
				    GetRepeatFlag3(nRow),
				    GetRepeatWeekMonth(nRow)));
}


//--------------------------------------------------------------//
// Get the start time as a string.                              //
//--------------------------------------------------------------//
string
SchedulerDB::GetStartTimeString(int nRow) const
{
    string strTime;
    time_t nTime = GetStartTime(nRow);

    strTime =::FormatTime(nTime);
    return (strTime);
}


//--------------------------------------------------------------//
// Get indications of which days have scheduled events.         //
// Assumes that pbEvent point to an array of at least 366       //
// bools.                                                       //
//--------------------------------------------------------------//
void
SchedulerDB::GetYearlyEvents(bool * pbEvent, int nYear)
{
    int nMax = NumRecs();
    int nRow;
    SchedulerRepeatData *pSchedulerRepeatData;

    // Clear the old event indications
    memset(pbEvent, 0, 366 * sizeof(bool));

    // Examine each event
    for (nRow = 0; nRow < nMax; ++nRow) {
	if (!IsDeleted(nRow)) {
	    pSchedulerRepeatData = GetSchedulerRepeatData(nRow);
	    pSchedulerRepeatData->GetYearlyEvents(pbEvent, nYear);
	    delete pSchedulerRepeatData;
	}
    }
}


//--------------------------------------------------------------//
// Test whether this row has a deleted exception for a given    //
// date or not.                                                 //
//--------------------------------------------------------------//
bool
SchedulerDB::HasDeletedException(int nRow, time_t nDate)
{
    bool bReturn = false;
    int i;
    int nID = GetID(nRow);
    int nMax = NumRecs();

#ifdef DEBUG
    assert(GetRepeatFlag1(nRow) != REPEAT_NONE);	// Must be a repeating event
#endif

    // Find an exception for this row
    for (i = 0; i < nMax && bReturn == false; ++i) {
	// Only examine non-deleted rows
	if (!IsDeleted(i)) {
	    // Only look further at deleted exception rows
	    if (IsDeletedException(i)) {
		// Find exceptions to this row
		if (GetExceptionRecno(i) == nID) {
		    // Is it for this day
		    if (::NormalizeDate(GetStartTime(i)) == nDate) {
			bReturn = true;
		    }
		}
	    }
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Import a row from a set of strings.                          //
//--------------------------------------------------------------//
int
SchedulerDB::Import(const vector < string > &vExportString)
{
    int nRow;

    // Import the data
    nRow = NxDbAccess::Import(vExportString);

    // Now fix the record number
    SetColumn(nRow, SCHED_ID, 0);
    SetHighStringKey(nRow, SCHED_ID);

    return (nRow);
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
SchedulerDB::Insert()
{
    int nRow = NxDbAccess::Insert();

    // Turn off any alarms
    SetAlarmFlags(nRow, SCHED_NO_ALARM);

    // Now set a unique key value
    SetHighKey(nRow, SCHED_ID);

    return (nRow);
}


//--------------------------------------------------------------//
// Move an event to a new time (with the same duration).        //
//--------------------------------------------------------------//
bool
SchedulerDB::MoveStartTime(int nRow, int nSeconds)
{
    bool bReturn;
    int nChange;
    time_t nStartDate =::NormalizeDate(GetStartTime(nRow));
    time_t nTime;

#ifdef DEBUG
    assert(nSeconds >= 0 && nSeconds < 24 * 60 * 60);	// Movement amount must be in range
#endif

    // Calculate the change in the start time
    nChange = nSeconds - GetStartTime(nRow) + nStartDate;

    // Change the start time
    nTime = nStartDate + nSeconds;

    // Correct for a daylight savings time shift
    while (::NormalizeDate(nTime) != nStartDate) {
	// Reduce it one hour
	nTime -= 60 * 60;
    }

    // Set the new start time
    bReturn = (nTime != GetStartTime(nRow));
    SetStartTime(nRow, nTime);

    // Change the end time
    nTime =
	((GetEndTime(nRow) - nStartDate + nChange) % (24 * 60 * 60)) +
	nStartDate;
    if (nTime < GetStartTime(nRow)) {
	nTime = 24 * 60 * 60 - 1 + nStartDate;
    }
    // Correct for a daylight savings time shift
    while (::NormalizeDate(nTime) != nStartDate) {
	// Reduce it one hour
	nTime -= 60 * 60;
    }

    // Set the new end time
    bReturn |= (nTime != GetEndTime(nRow));
    SetEndTime(nRow, nTime);

    return (bReturn);
}


//--------------------------------------------------------------//
// Remove all exceptons for a row.                              //
//--------------------------------------------------------------//
void
SchedulerDB::RemoveExceptions(int nRow)
{
    int nID = GetID(nRow);
    int nMax = NumRecs();

    // Delete all exceptions for this row
    for (nRow = 0; nRow < nMax; ++nRow) {
	if (!IsDeleted(nRow)
	    && IsException(nRow)
	    && GetExceptionRecno(nRow) == nID) {
	    NxDbAccess::Delete(nRow);
	}
    }
}


//--------------------------------------------------------------//
// Test if this repeat data is different from a row.            //
//--------------------------------------------------------------//
bool
SchedulerDB::RepeatDataChanged(int nRow, SchedulerRepeatData * pRepeatData)
{
    bool bReturn;
    SchedulerRepeatData *pRepeatData2 = GetSchedulerRepeatData(nRow);

    bReturn = (*pRepeatData2 == *pRepeatData);
    delete pRepeatData2;
    return (bReturn);
}


//--------------------------------------------------------------//
// Set that this event does not repeat.                         //
//--------------------------------------------------------------//
void
SchedulerDB::SetNoRepetition(int nRow)
{
    SetRepeatFlag1(nRow, 0);
    SetRepeatFlag2(nRow, 0);
    SetRepeatFlag3(nRow, 0);
    SetRepeatWeekMonth(nRow, 0);
}


//--------------------------------------------------------------//
// Set the duration to some number of seconds, but then change  //
// the end-time to the nearest half-hour.                       //
//--------------------------------------------------------------//
bool
SchedulerDB::SetRoundedDuration(int nRow, int nSeconds)
{
    bool bReturn;
    time_t nEndTime;
    struct tm *pTm;

    nEndTime = GetStartTime(nRow) + nSeconds;
    pTm = localtime(&nEndTime);
    nSeconds = pTm->tm_hour * 60 * 60 + pTm->tm_min * 60 + pTm->tm_sec;
    nSeconds = 30 * 60 * ((nSeconds + 15 * 60 - 1) / (30 * 60));
    if (nSeconds >= 24 * 60 * 60) {
	nSeconds = 24 * 60 * 60 - 1;
    }
    pTm->tm_sec = (nSeconds % (60 * 60));
    pTm->tm_min = ((nSeconds / 60) % 60);
    pTm->tm_hour = nSeconds / (60 * 60);
    nEndTime = mktime(pTm);
    if (nEndTime != GetEndTime(nRow)) {
	SetEndTime(nRow, nEndTime);
	bReturn = true;
    } else {
	bReturn = false;
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Set the repeat settings, the start and end times are not     //
// changed here.                                                //
//--------------------------------------------------------------//
void
SchedulerDB::SetSchedulerRepeatData(int nRow,
				    const SchedulerRepeatData *
				    pSchedulerRepeatData)
{
    SetRepeatFlag1(nRow, pSchedulerRepeatData->GetRepeatFlag1());
    SetRepeatFlag2(nRow, pSchedulerRepeatData->GetRepeatFlag2());
    SetRepeatFlag3(nRow, pSchedulerRepeatData->GetRepeatFlag3());
    SetRepeatWeekMonth(nRow, pSchedulerRepeatData->GetRepeatWeekMonth());
}


//--------------------------------------------------------------//
// Used when dragging an event to a new time.  This will set    //
// the start time as per the arguments given and then set the   //
// end time for the same duration.                              //
//--------------------------------------------------------------//
void
SchedulerDB::SetStartDate(int nRow, time_t nDate, int nTime)
{
    int nDuration = GetEndTime(nRow) - GetStartTime(nRow);
    time_t nNewTime;

#ifdef DEBUG
    assert(nDate ==::NormalizeDate(nDate));	// The date must be normalized
#endif

    // Set the start time
    nNewTime = nDate + nTime;
    if (::NormalizeDate(nNewTime) != nDate) {
	// Fix if spans midnight
	nNewTime =::AddDays(nDate, 1) - 5 * 60;
    }
    SetStartTime(nRow, nNewTime);

    // Now set the end time
    nNewTime = nDate + nTime + nDuration;
    if (::NormalizeDate(nNewTime) != nDate) {
	// Fix if spans midnight
	nNewTime =::AddDays(nDate, 1) - 1;
    }
    SetEndTime(nRow, nNewTime);
}
