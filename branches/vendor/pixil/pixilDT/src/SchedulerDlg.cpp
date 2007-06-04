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
// Dialog for adding/updating Scheduler appointments.           //
//--------------------------------------------------------------//
#include "config.h"
#include <cstdlib>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include "DatePickerDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "Images.h"
#include "Messages.h"
#include "PixilDT.h"
#include "SchedulerChangeTypeDlg.h"
#include "SchedulerDB.h"
#include "SchedulerDlg.h"
#include "SchedulerRepeatDlg.h"
#include "SchedulerTimeDlg.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
#define max __max
#endif


#define DLG_HEIGHT       (9*DLG_BORDER+6*DLG_INPUT_HEIGHT+2*DLG_BUTTON_HEIGHT)
#define DLG_INDENT       (DLG_BUTTON_WIDTH+2*DLG_BORDER)
#define DLG_WIDTH        (DLG_INDENT+2*DLG_BORDER+2*DLG_BUTTON_WIDTH+20)


//--------------------------------------------------------------//
// Constructor, nRow is the row number in the Scheduler         //
// database as currently sorted or a -1 if this is to be a new  //
// row.                                                         //
//--------------------------------------------------------------//
SchedulerDlg::SchedulerDlg(int nRow, time_t nDate, Fl_Widget * pParent)
    :
Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Edit Event"))
{
    static const char *ppszTimeUnits[3] = {
	N_("Minutes Before"),
	N_("Hours Before"),
	N_("Days before"),
    };
    Fl_Box *pBox;
    int nToHeight;
    int nToWidth;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    string strAm;
    string strPm;

    // Get the strings used for AM and PM
    PixilDT::GetAMPM(strAm, strPm);

    // Get the record number for this Scheduler key
    if (nRow < 0) {
	// New row
	m_nRow = -1;
    } else {
	m_nRow = nRow;
    }

    // Get the width of the word "to"
    fl_font(labelfont(), labelsize());
    fl_measure(_("t&o"), nToWidth, nToHeight);

    // Create the dialog widgets
    m_pDescription = new Fl_Multiline_Input(DLG_INDENT,
					    DLG_BORDER,
					    DLG_WIDTH - DLG_BORDER -
					    DLG_INDENT, 3 * DLG_INPUT_HEIGHT,
					    _("Description:"));
    m_pDescription->maximum_size(pSchedulerDB->GetColumnSize(SCHED_DESC));
    m_pStartTime = new Fl_Input(DLG_INDENT,
				2 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT,
				(DLG_WIDTH - 3 * DLG_BORDER - nToWidth -
				 DLG_INDENT - IMAGE_BUTTON_WIDTH) / 2,
				DLG_INPUT_HEIGHT, _("Time:"));
    m_pStartTime->maximum_size(6 + max(strAm.length(), strPm.length()));
    m_pEndTime =
	new Fl_Input(DLG_INDENT + nToWidth + DLG_BORDER + m_pStartTime->w(),
		     2 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT, m_pStartTime->w(),
		     DLG_INPUT_HEIGHT, _("to"));
    m_pEndTime->maximum_size(6 + max(strAm.length(), strPm.length()));
    m_pTimeButton = new Fl_Button(w() - DLG_BORDER - IMAGE_BUTTON_WIDTH,
				  2 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT,
				  IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT);
    m_pTimePixmap = Images::GetTimeIcon();
    m_pTimePixmap->label(m_pTimeButton);
    m_pTimeButton->callback(OnTimeButton);
    m_pDate = new Fl_Input(DLG_INDENT,
			   3 * DLG_BORDER + 4 * DLG_INPUT_HEIGHT,
			   (DLG_WIDTH - 3 * DLG_BORDER - DLG_INDENT -
			    IMAGE_BUTTON_WIDTH) / 2, DLG_INPUT_HEIGHT,
			   _("Date:"));
    m_pDate->maximum_size(10);
    m_pDayOfWeek = new Fl_Output(DLG_INDENT + DLG_BORDER + m_pDate->w(),
				 3 * DLG_BORDER + 4 * DLG_INPUT_HEIGHT,
				 m_pDate->w(), DLG_INPUT_HEIGHT);
    m_pDayOfWeek->color(FL_GRAY);
    m_pDayOfWeek->textfont(FL_HELVETICA_BOLD);
    m_pDayOfWeek->textsize((4 * labelsize()) / 5);
    m_pDateButton = new Fl_Button(w() - DLG_BORDER - IMAGE_BUTTON_WIDTH,
				  3 * DLG_BORDER + 4 * DLG_INPUT_HEIGHT,
				  IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT);
    m_pDatePixmap = Images::GetCalendarIcon();
    m_pDatePixmap->label(m_pDateButton);
    m_pDateButton->callback(OnDateButton);
    pBox = new Fl_Box(DLG_BORDER,
		      4 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT,
		      DLG_INDENT - DLG_BORDER,
		      DLG_INPUT_HEIGHT, _("Repeat:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pRepeatButton =
	new Fl_Button(DLG_INDENT, 4 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT,
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("None..."));
    m_pRepeatButton->callback(OnRepeatButton);
    pBox = new Fl_Box(DLG_BORDER,
		      5 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT +
		      DLG_BUTTON_HEIGHT, DLG_INDENT - DLG_BORDER,
		      DLG_INPUT_HEIGHT);
    pBox->label(_("Alarm:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pAlarmButton =
	new Fl_Check_Button(DLG_INDENT,
			    5 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT +
			    DLG_BUTTON_HEIGHT, DLG_RADIO_SIZE,
			    DLG_INPUT_HEIGHT);
    m_pAlarmButton->callback(OnAlarm);
    m_pAlarmButton->type(FL_TOGGLE_BUTTON);
    m_pAlarmButton->down_box(FL_ROUND_DOWN_BOX);
    m_pInterval = new SpinInput(DLG_INDENT + DLG_RADIO_SIZE + DLG_BORDER, 5 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT + DLG_BUTTON_HEIGHT, 3 * labelsize() + IMAGE_BUTTON_WIDTH, DLG_INPUT_HEIGHT, NULL, 2,	// Maximum size
				0,	// Minimum value
				99);	// Maximum value
    m_pInterval->value(5);	// Default to 5 minutes
    m_pTimeUnit =
	new Fl_Choice(DLG_INDENT + DLG_RADIO_SIZE + 3 * labelsize() +
		      IMAGE_BUTTON_WIDTH + 2 * DLG_BORDER,
		      5 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT +
		      DLG_BUTTON_HEIGHT,
		      DLG_WIDTH - (DLG_INDENT + DLG_RADIO_SIZE +
				   3 * labelsize() + IMAGE_BUTTON_WIDTH +
				   3 * DLG_BORDER), DLG_INPUT_HEIGHT);
    m_pTimeUnitMenu = CreateChoice(3, ppszTimeUnits, true);
    m_pTimeUnit->value(0);	// Default to 5 minutes
    m_pTimeUnit->menu(m_pTimeUnitMenu);

    // Create the buttons
    m_pOKButton =
	new Fl_Return_Button(w() - 3 * DLG_BUTTON_WIDTH - 3 * DLG_BORDER,
			     h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
			     DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton =
	new Fl_Button(w() - 2 * DLG_BUTTON_WIDTH - 2 * DLG_BORDER,
		      h() - DLG_BORDER - DLG_BUTTON_HEIGHT, DLG_BUTTON_WIDTH,
		      DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);

    end();

    // Save the selected date
    m_nDate = nDate;

    // Get the initial values for the data
    if (m_nRow == -1) {
	time_t nEndTime;
	time_t nStartTime;
	struct tm *pTm;

	// New row, set the date to the requested date
	m_pDate->value(::FormatDate(nDate).c_str());
	m_pDayOfWeek->value(::FormatDayOfWeek(nDate).c_str());

	// Initialize the repetition settings
	pTm = localtime(&nDate);
	pTm->tm_sec = 0;
	pTm->tm_min = 0;
	if (pTm->tm_hour < 23) {
	    ++pTm->tm_hour;
	}
	nEndTime = mktime(pTm);
	--pTm->tm_hour;
	nStartTime = mktime(pTm);
	m_pSchedulerRepeatData = new SchedulerRepeatData(nStartTime,
							 nEndTime);

	// Set the times in the dialog as well
	m_pStartTime->value(::FormatTime(nStartTime).c_str());
	m_pEndTime->value(::FormatTime(nEndTime).c_str());

	// Hide the alarm settings
	m_pInterval->hide();
	m_pTimeUnit->hide();
    } else {
	// Initialize these for possible use later
	m_pSchedulerRepeatData = pSchedulerDB->GetSchedulerRepeatData(nRow);

	// Existing row, set everything
	m_pDescription->value(pSchedulerDB->GetDescription(m_nRow).c_str());
	m_pStartTime->value(::FormatTime(pSchedulerDB->GetStartTime(m_nRow)).
			    c_str());
	m_pEndTime->value(::FormatTime(pSchedulerDB->GetEndTime(m_nRow)).
			  c_str());
	m_pDate->value(::FormatDate(m_nDate).c_str());
	m_pDayOfWeek->value(::FormatDayOfWeek(m_nDate).c_str());
	SetRepeatButton();

	// Process the alarm settings
	if (pSchedulerDB->GetAlarmFlags(m_nRow) != SCHED_NO_ALARM) {
	    // Use this row's alarm settings
	    m_pAlarmButton->value(1);
	    m_pInterval->value(pSchedulerDB->GetAlarmInterval(m_nRow));
	    m_pTimeUnit->value(pSchedulerDB->GetAlarmFlags(m_nRow));
	} else {
	    // Hide the alarm settings
	    m_pInterval->hide();
	    m_pTimeUnit->hide();
	}
    }

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
SchedulerDlg::~SchedulerDlg()
{
    ::FreeTranslatedMenu(m_pTimeUnitMenu);
    delete m_pDatePixmap;
    delete m_pTimePixmap;
    delete m_pSchedulerRepeatData;
}


//--------------------------------------------------------------//
// Run the modal dialog                                         //
//--------------------------------------------------------------//
int
SchedulerDlg::DoModal()
{
    bool bGood = false;
    int nChangeType;
    int nResult;
    int nReturn = 1;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();

    while (nReturn == 1 && bGood == false) {
	// Go run the dialog
	nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

	// Was the OK button pressed ?
	if (nReturn == 1) {
	    // Validate all fields
	    bGood = true;
	    if (strlen(m_pDescription->value()) == 0) {
		fl_alert(_
			 ("The Description is missing.  Please enter a Description."));
		bGood = false;
	    } else
		if ((nResult =::
		     ValidateTime(m_pStartTime->value(),
				  m_nStartTime)) != 0) {
		fl_alert(_("The Start Time is not a valid time:\n\n%s"),::
			 GetTimeError(nResult));
		bGood = false;
	    } else
		if ((nResult =::
		     ValidateTime(m_pEndTime->value(), m_nEndTime)) != 0) {
		fl_alert(_("The End Time is not a valid time:\n\n%s"),::
			 GetTimeError(nResult));
		bGood = false;
	    } else if (m_nStartTime >= m_nEndTime) {
		fl_alert(_("The End Time must be after the Start Time"));
		bGood = false;
	    } else if ((nResult =::ValidateDate(m_pDate->value(), m_nDate)) !=
		       0) {
		fl_alert(_("The Date is not a valid date:\n\n%s"),::
			 GetDateError(nResult));
		bGood = false;
	    }
	    // Update if all pased
	    if (bGood == true) {
		// Set to change the original record
		nChangeType = SCHEDULER_CHANGE_TYPE_ALL;

		// Create a new Scheduler record if needed
		if (m_nRow < 0) {
		    m_nRow = pSchedulerDB->Insert();
		} else {
		    // Updating an existing item, determine whether to ask
		    // about updating events in a repeating sequence or not.
		    if (pSchedulerDB->GetRepeatingFlag(m_nRow) == true) {
			// Something besides the description must have changed
			if (pSchedulerDB->
			    RepeatDataChanged(m_nRow,
					      m_pSchedulerRepeatData) == true
			    || pSchedulerDB->GetStartTime(m_nRow) !=
			    m_nStartTime + m_nDate
			    || pSchedulerDB->GetEndTime(m_nRow) !=
			    m_nEndTime + m_nDate
			    || (m_pAlarmButton->value() == 0
				&& pSchedulerDB->GetAlarmFlags(m_nRow) ==
				SCHED_NO_ALARM)
			    || (m_pAlarmButton->value() == 1
				&& (pSchedulerDB->GetAlarmFlags(m_nRow) !=
				    m_pTimeUnit->value()
				    || pSchedulerDB->
				    GetAlarmInterval(m_nRow) !=
				    m_pInterval->value()))) {
			    SchedulerChangeTypeDlg *pDlg =
				new SchedulerChangeTypeDlg(this, false);

			    // Ask about whether to change the current item only,
			    // the current item and future items or all items
			    if (pDlg->DoModal() == 1) {
				nChangeType = pDlg->GetChangeType();
			    } else {
				// Set no changes wanted - the user cancelled the dialog
				nChangeType = SCHEDULER_CHANGE_TYPE_NONE;
			    }
			    delete pDlg;
			}
		    }
		}

		switch (nChangeType) {
		case SCHEDULER_CHANGE_TYPE_ALL:	// Change all repetitions
		    if (pSchedulerDB->GetRepeatType(m_nRow) !=
			m_pSchedulerRepeatData->GetRepeatType()) {
			// Remove any prior exceptions if changing the repetition type
			pSchedulerDB->RemoveExceptions(m_nRow);
		    }
		    SetFields(m_nRow);
		    break;

		case SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE:	// Change current and future repetitions
		    // Make these repetitions end one day prior to the selected event
		    pSchedulerDB->
			EndRepetitions(m_nRow,::SubtractDays(m_nDate, 1));

		    // Add a new row to carry on from here
		    m_nRow = pSchedulerDB->Insert();
		    m_pSchedulerRepeatData->SetStartDate(m_nDate);
		    SetFields(m_nRow);
		    break;

		case SCHEDULER_CHANGE_TYPE_CURRENT_ONLY:	// Change only the current repetition
		    // Add a deleted exception for this date
		    pSchedulerDB->AddDeletedException(m_nRow, m_nDate);

		    // Add a new row for this exception date
		    m_nRow = pSchedulerDB->Insert();
		    m_pSchedulerRepeatData->SetNoRepetition();
		    m_pSchedulerRepeatData->SetStartDate(m_nDate);
		    SetFields(m_nRow);
		}

		// Save these changes
		pSchedulerDB->Save();

		// Notify everyone of these changes
		PixilDT::GetApp()->GetMainWindow()->
		    Notify(ADDRESS_BOOK_CHANGED, 0);
	    }
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Process a click on the alarm radio button.                   //
//--------------------------------------------------------------//
void
SchedulerDlg::OnAlarm(Fl_Widget * pWidget, void *pUserData)
{
    Fl_Check_Button *pButton =
	reinterpret_cast < Fl_Check_Button * >(pWidget);
    SchedulerDlg *pThis =
	reinterpret_cast < SchedulerDlg * >(pWidget->parent());

    if (pButton->value() == 1) {
	// Show the other alarm fields
	pThis->m_pInterval->show();
	pThis->m_pTimeUnit->show();
    } else {
	// Hide the other alarm fields
	pThis->m_pInterval->hide();
	pThis->m_pTimeUnit->hide();
    }
}


//--------------------------------------------------------------//
// Date/Calendar button was clicked (static callback).          //
//--------------------------------------------------------------//
void
SchedulerDlg::OnDateButton(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pDlg;
    SchedulerDlg *pThis =
	reinterpret_cast < SchedulerDlg * >(pWidget->parent());
    time_t nDate;

    // Get the current date from the dialog
    if (::ValidateDate(pThis->m_pDate->value(), nDate) != 0) {
	// If bad, use the one from the data base
	if (pThis->m_nRow >= 0) {
	    // Get from an existing row
	    nDate =
		SchedulerDB::GetSchedulerDB()->GetStartTime(pThis->m_nRow);
	} else {
	    // This is a new row, use today
	    nDate = time(NULL);
	}
    }
    // Now run the dialog
    pDlg =
	new DatePickerDlg(nDate, DatePicker::Daily,
			  PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	// Get the new start and end times
	pThis->m_pDate->value(::FormatDate(pDlg->GetDate()).c_str());
	pThis->m_pDayOfWeek->value(::FormatDayOfWeek(pDlg->GetDate()).
				   c_str());
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
SchedulerDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_SCHEDULER_DLG);
}


//--------------------------------------------------------------//
// Repeat button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
SchedulerDlg::OnRepeatButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDlg *pThis =
	reinterpret_cast < SchedulerDlg * >(pWidget->parent());
    SchedulerRepeatDlg *pDlg;

    // Now run the dialog
    pDlg = new SchedulerRepeatDlg(pThis->m_pSchedulerRepeatData,
				  PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	delete pThis->m_pSchedulerRepeatData;
	pThis->m_pSchedulerRepeatData =
	    new SchedulerRepeatData(pDlg->GetSchedulerRepeatData());
	pThis->SetRepeatButton();

	// Display the date in case it changed
	pThis->m_pDate->
	    value(::FormatDate(pThis->m_pSchedulerRepeatData->GetStartTime()).
		  c_str());
	pThis->m_pDayOfWeek->
	    value(::
		  FormatDayOfWeek(pThis->m_pSchedulerRepeatData->
				  GetStartTime()).c_str());
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Time button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
SchedulerDlg::OnTimeButton(Fl_Widget * pWidget, void *pUserData)
{
    int nEndTime;
    int nStartTime;
    SchedulerDlg *pThis =
	reinterpret_cast < SchedulerDlg * >(pWidget->parent());
    SchedulerTimeDlg *pDlg;
    time_t nEndDate;
    time_t nStartDate;
    struct tm *pTm;

    // Get the current start time from the dialog
    if (::ValidateTime(pThis->m_pStartTime->value(), nStartTime) != 0) {
	// If the currently entered time is bad, use midnight
	nStartTime = 0;
    }
    // Get the current end time from the dialog
    if (::ValidateTime(pThis->m_pEndTime->value(), nEndTime) != 0) {
	// If the currently entered time is bad, use 5 minutes before midnight
	nEndTime = 24 * 60 * 60 - 5 * 60;
    }
    // Get the current date from the dialog
    if (::ValidateDate(pThis->m_pDate->value(), nStartDate) != 0) {
	// If bad, use the one from the data base
	if (pThis->m_nRow >= 0) {
	    // Get from an existing row
	    nStartDate =
		SchedulerDB::GetSchedulerDB()->GetStartTime(pThis->m_nRow);
	} else {
	    // This is a new row, use today
	    nStartDate = time(NULL);
	}
    }
    // Get current start date
    pTm = localtime(&nStartDate);
    pTm->tm_sec = 0;
    pTm->tm_min = (nStartTime % (60 * 60)) / 60;
    pTm->tm_hour = nStartTime / (60 * 60);
    nStartDate = mktime(pTm);

    // Get the current end date
    pTm->tm_min = (nEndTime % (60 * 60)) / 60;
    pTm->tm_hour = nEndTime / (60 * 60);
    nEndDate = mktime(pTm);

    // Now run the dialog
    pDlg =
	new SchedulerTimeDlg(nStartDate, nEndDate,
			     PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	// Get the new start and end times
	pThis->m_pStartTime->value(::FormatTime(pDlg->GetStartTime()).
				   c_str());
	pThis->m_pEndTime->value(::FormatTime(pDlg->GetEndTime()).c_str());
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Move all dialog data to a data base row.                     //
//--------------------------------------------------------------//
void
SchedulerDlg::SetFields(int nRow)
{
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();

    pSchedulerDB->SetSchedulerRepeatData(nRow, m_pSchedulerRepeatData);
    pSchedulerDB->SetStartTime(nRow, m_nStartTime + m_nDate);
    pSchedulerDB->SetEndTime(nRow, m_nEndTime + m_nDate);
    pSchedulerDB->SetAllDayFlag(nRow, 0);
    pSchedulerDB->SetEntryType(nRow, 0);
    pSchedulerDB->SetDescription(nRow, m_pDescription->value());

    // Get the alarm settings
    if (m_pAlarmButton->value() == 1) {
	// Turn the alarm on
	pSchedulerDB->SetAlarmFlags(nRow, m_pTimeUnit->value());
	pSchedulerDB->SetAlarmInterval(nRow, m_pInterval->value());
    } else {
	// Turn the alarm off
	pSchedulerDB->SetAlarmFlags(nRow, SCHED_NO_ALARM);
	pSchedulerDB->SetAlarmInterval(nRow, 0);
    }
}


//--------------------------------------------------------------//
// Set the label on the repeat button.                          //
//--------------------------------------------------------------//
void
SchedulerDlg::SetRepeatButton()
{
    m_strRepeatLabel = m_pSchedulerRepeatData->GetRepeatTypeString();
    m_strRepeatLabel += _("...");
    m_pRepeatButton->label(m_strRepeatLabel.c_str());
}
