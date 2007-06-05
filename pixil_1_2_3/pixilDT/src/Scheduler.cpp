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
// Center of the window for the Scheduler display               //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "Options.h"
#include "PixilDT.h"
#include "Scheduler.h"
#include "SchedulerChangeTypeDlg.h"
#include "SchedulerDB.h"
#include "SchedulerDlg.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
Scheduler::Scheduler(Fl_Widget * pParent)
:  Fl_Group(pParent->x(), pParent->y(), pParent->w(), pParent->h())
{
    Fl_Widget *pWidget;

    m_pTab = new Fl_Tabs(x() + DLG_BORDER,
			 y() + DLG_BORDER,
			 w() - 2 * DLG_BORDER, h() - 2 * DLG_BORDER);

    // Create the tab pages
    m_pDailyPage = new SchedulerDaily(m_pTab->x(),
				      m_pTab->y(),
				      m_pTab->w(),
				      m_pTab->h() - DLG_TAB_HEIGHT);
    m_pWeeklyPage = new SchedulerWeekly(m_pTab->x(),
					m_pTab->y(),
					m_pTab->w(),
					m_pTab->h() - DLG_TAB_HEIGHT);
    m_pMonthlyPage = new SchedulerMonthly(m_pTab->x(),
					  m_pTab->y(),
					  m_pTab->w(),
					  m_pTab->h() - DLG_TAB_HEIGHT);
    m_pYearlyPage = new SchedulerYearly(m_pTab->x(),
					m_pTab->y(),
					m_pTab->w(),
					m_pTab->h() - DLG_TAB_HEIGHT);

    // Finish this group
    end();

    // Display the most recently used page
    switch (Options::GetSchedulerPage()) {
    case 1:
	pWidget = m_pWeeklyPage;
	break;

    case 2:
	pWidget = m_pMonthlyPage;
	break;

    case 3:
	pWidget = m_pYearlyPage;
	break;

    default:			// Includes case 0 for daily
	pWidget = m_pDailyPage;
    }
    m_pTab->value(pWidget);

    // Set that this is resizable
    m_pTab->resizable(m_pDailyPage);
    resizable(m_pTab);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
Scheduler::~Scheduler()
{
}


//--------------------------------------------------------------//
// Delete an appointment                                        //
//--------------------------------------------------------------//
void
Scheduler::Delete(int nRow, time_t nDate)
{
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    int nContinue;

#ifdef DEBUG
    assert(nDate ==::NormalizeDate(nDate));	// Date must be normailized
#endif

    if (pSchedulerDB->GetRepeatingFlag(nRow) == false) {
	// Not a repeating or exception event
	if (Options::GetConfirmDelete() == true) {
	    nContinue =
		fl_ask(_("Delete Appointment for %s %s ?"),::
		       FormatDate(pSchedulerDB->GetStartTime(nRow)).
		       c_str(),::FormatTime(pSchedulerDB->GetStartTime(nRow)).
		       c_str());
	    nContinue =
		(nContinue ==
		 1 ? SCHEDULER_CHANGE_TYPE_ALL : SCHEDULER_CHANGE_TYPE_NONE);
	} else {
	    nContinue = SCHEDULER_CHANGE_TYPE_ALL;
	}
    } else {
	// A repeating or exception event
	SchedulerChangeTypeDlg *pDlg = new SchedulerChangeTypeDlg(this, true);

	pDlg->DoModal();
	nContinue = pDlg->GetChangeType();
	delete pDlg;
    }

    if (nContinue == SCHEDULER_CHANGE_TYPE_ALL) {
	// Delete this item and all exceptions related to it
	pSchedulerDB->Delete(nRow);
    } else if (nContinue == SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE) {
	// End this repeating event prior to the date in question
	pSchedulerDB->EndRepetitions(nRow, nDate);
    } else if (nContinue == SCHEDULER_CHANGE_TYPE_CURRENT_ONLY) {
	// Insert a deleted exception for this date
	pSchedulerDB->AddDeletedException(nRow, nDate);
    }
    // Save the changes if any occurred
    if (nContinue > 0) {
	pSchedulerDB->Save();
	PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);
    }
}


//--------------------------------------------------------------//
// Edit an appointment                                          //
//--------------------------------------------------------------//
void
Scheduler::Edit(int nRow, time_t nDate)
{
    SchedulerDlg *pSchedulerDlg;

#ifdef DEBUG
    assert(nDate ==::NormalizeDate(nDate));	// The date must be normalized
#endif

    pSchedulerDlg = new SchedulerDlg(nRow,
				     nDate,
				     PixilDT::GetApp()->GetMainWindow());
    if (pSchedulerDlg->DoModal() == 1) {
	// OK button was pressed, refresh displays
	PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);
    }
    delete pSchedulerDlg;
}


//--------------------------------------------------------------//
// Add a new appointment                                        //
//--------------------------------------------------------------//
void
Scheduler::EditNew(time_t nDate)
{
    SchedulerDlg *pSchedulerDlg = new SchedulerDlg(-1,
						   nDate,
						   PixilDT::GetApp()->
						   GetMainWindow());

    if (pSchedulerDlg->DoModal() == 1) {
	// OK button was pressed, refresh displays
	PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);
    }
    delete pSchedulerDlg;
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
Scheduler::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:	// Fix the address book display
    case TODO_LIST_CHANGED:	// Fix the to do list display
	m_pDailyPage->Message(nMessage, nInfo);
	break;

    case BEGIN_WEEK_CHANGED:	// Beginning day of week has changed
    case SCHEDULER_CHANGED:	// Scheduler data changed, go refresh the display
	m_pDailyPage->Message(nMessage, nInfo);
	m_pWeeklyPage->Message(nMessage, nInfo);
	m_pMonthlyPage->Message(nMessage, nInfo);
	m_pYearlyPage->Message(nMessage, nInfo);
	break;

    case SELECTION_CHANGING:	// No processing needed here
	nReturn = 1;		// Set no errors occurred
	break;

    case SCHEDULER_GOTO:	// Find dialog has requested the display of an appointment
	{
	    int nRow;
	    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
	    time_t nStartTime;

	    m_pTab->value(m_pDailyPage);
	    nRow = pSchedulerDB->FindPhysicalRecord(nInfo);
	    nStartTime = pSchedulerDB->GetStartTime(nRow);
	    m_pDailyPage->DisplayDay(nStartTime);
	    Edit(nRow,::NormalizeDate(nStartTime));
	}
	break;

    case SCHEDULER_PRINT:
	if (m_pTab->value() == m_pDailyPage) {
	    m_pDailyPage->Print();
	} else if (m_pTab->value() == m_pWeeklyPage) {
	    m_pWeeklyPage->Print();
	} else if (m_pTab->value() == m_pMonthlyPage) {
	    m_pMonthlyPage->Print();
	} else if (m_pTab->value() == m_pYearlyPage) {
	    m_pYearlyPage->Print();
	}
	break;

    case ADDRESS_BOOK_REQUESTED:	// Selection changing, no processing needed
    case NOTES_REQUESTED:	// Selection changing, no processing needed
    case SCHEDULER_REQUESTED:	// Selection changing, no processing needed
    case TODO_LIST_REQUESTED:	// Selection changing, no processing needed
	break;

    case APPLICATION_CLOSING:	// Save the most recently selected page to the INI file
	{
	    Fl_Widget *pWidget = m_pTab->value();
	    int nValue;

	    // Set the most recently used page
	    if (pWidget == m_pWeeklyPage) {
		nValue = 1;
	    } else if (pWidget == m_pMonthlyPage) {
		nValue = 2;
	    } else if (pWidget == m_pYearlyPage) {
		nValue = 3;
	    } else {
		// Must be the daily page
		nValue = 0;
	    }
	    Options::SetSchedulerPage(nValue);
	}
	break;

    case EDIT_COPY_AVAILABLE:	// No cut/copy/paste or undo allowed
    case EDIT_CUT_AVAILABLE:
    case EDIT_PASTE_AVAILABLE:
    case EDIT_UNDO_AVAILABLE:
	nReturn = 0;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Select and display a given day.                              //
//--------------------------------------------------------------//
void
Scheduler::SelectDay(time_t nDate)
{
    m_pDailyPage->DisplayDay(nDate);
    m_pTab->value(m_pDailyPage);
}
