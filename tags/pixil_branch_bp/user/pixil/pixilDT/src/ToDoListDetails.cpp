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
// Class for the ToDo List Details.                             //
//--------------------------------------------------------------//
#include "config.h"
#include <ctime>
#include <FL/fl_ask.H>
#include "DatePickerDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "NoteEditorDlg.h"
#include "PixilDT.h"
#include "TimeFunc.h"
#include "ToDoListDB.h"
#include "ToDoListDetails.h"
#include "ToDoListCategoryDB.h"

#include "VCMemoryLeak.h"


#define DATE_INPUT_WIDTH   84
#define PRIORITY_WIDTH     16
#define PROMPT_BORDER       3
#define PROMPT_WIDTH       80


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ToDoListDetails::ToDoListDetails(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight)
{
    Fl_Box *pBox;
    Fl_Group *pGroup;
    int i;
    int nButtonWidth;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Create widgets for this object
    pBox = new Fl_Box(x(), y(), PROMPT_WIDTH, DLG_INPUT_HEIGHT, _("Title:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pTitle =
	new Fl_Multiline_Input(x() + PROMPT_WIDTH + PROMPT_BORDER, y(),
			       w() - PROMPT_BORDER - PROMPT_WIDTH,
			       2 * DLG_INPUT_HEIGHT);
    m_pTitle->maximum_size(pToDoListDB->GetColumnSize(TODO_TITLE));
    pBox = new Fl_Box(x(),
		      y() + 2 * DLG_INPUT_HEIGHT + DLG_BORDER,
		      PROMPT_WIDTH,
		      DLG_BUTTON_HEIGHT + 2 * DLG_BORDER, _("Priority:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    pGroup =
	new Fl_Group(x() + PROMPT_WIDTH + PROMPT_BORDER,
		     y() + 2 * DLG_INPUT_HEIGHT + DLG_BORDER,
		     w() - PROMPT_WIDTH - PROMPT_BORDER,
		     DLG_BUTTON_HEIGHT + 2 * DLG_BORDER);
    pGroup->box(FL_EMBOSSED_FRAME);
    nButtonWidth = (pGroup->w() - 4 * DLG_BORDER) / (5 - 1);
    for (i = 0; i < 5; ++i) {
	m_szLabel[i][0] = i + '1';
	m_szLabel[i][1] = '\0';
	m_pPriority[i] =
	    new Fl_Button(x() + PROMPT_WIDTH + PROMPT_BORDER + DLG_BORDER +
			  i * nButtonWidth,
			  y() + 2 * DLG_INPUT_HEIGHT + 2 * DLG_BORDER,
			  PRIORITY_WIDTH, DLG_BUTTON_HEIGHT, m_szLabel[i]);
	m_pPriority[i]->type(FL_RADIO_BUTTON);
    }
    pGroup->end();
    pBox = new Fl_Box(x(),
		      y() + 2 * DLG_INPUT_HEIGHT + DLG_BUTTON_HEIGHT +
		      4 * DLG_BORDER, PROMPT_WIDTH, DLG_INPUT_HEIGHT,
		      _("Date:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pDate =
	new Fl_Input(x() + PROMPT_WIDTH + PROMPT_BORDER,
		     y() + 2 * DLG_INPUT_HEIGHT + DLG_BUTTON_HEIGHT +
		     4 * DLG_BORDER, DATE_INPUT_WIDTH, DLG_INPUT_HEIGHT);
    m_pDate->maximum_size(10);
    m_pDateButton =
	new Fl_Button(x() + PROMPT_WIDTH + DATE_INPUT_WIDTH +
		      2 * PROMPT_BORDER,
		      y() + 2 * DLG_INPUT_HEIGHT + DLG_BUTTON_HEIGHT +
		      4 * DLG_BORDER, IMAGE_BUTTON_WIDTH,
		      IMAGE_BUTTON_HEIGHT);
    m_pCalendarPixmap = Images::GetCalendarIcon();
    m_pCalendarPixmap->label(m_pDateButton);
    m_pDateButton->callback(DateButtonCallback);
    m_pDayOfWeek =
	new Fl_Output(x() + PROMPT_WIDTH + DATE_INPUT_WIDTH +
		      IMAGE_BUTTON_WIDTH + 3 * PROMPT_BORDER,
		      y() + 2 * DLG_INPUT_HEIGHT + DLG_BUTTON_HEIGHT +
		      4 * DLG_BORDER,
		      w() - PROMPT_WIDTH - DATE_INPUT_WIDTH -
		      IMAGE_BUTTON_WIDTH - 3 * PROMPT_BORDER,
		      DLG_INPUT_HEIGHT, "");
    m_pDayOfWeek->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pDayOfWeek->color(FL_GRAY);
    m_pDayOfWeek->textsize((4 * labelsize()) / 5);
    m_pDayOfWeek->textfont(FL_HELVETICA_BOLD);
    pBox = new Fl_Box(x(),
		      y() + 3 * DLG_INPUT_HEIGHT + 1 * DLG_BUTTON_HEIGHT +
		      5 * DLG_BORDER, PROMPT_WIDTH, DLG_BUTTON_HEIGHT,
		      _("Category:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pCategoryChoice =
	new Fl_Choice(x() + PROMPT_WIDTH + PROMPT_BORDER,
		      y() + 3 * DLG_INPUT_HEIGHT + 1 * DLG_BUTTON_HEIGHT +
		      5 * DLG_BORDER, w() - PROMPT_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCategoryMenu =
	ToDoListCategoryDB::GetToDoListCategoryDB()->GetCategoryMenu(false);
    m_pCategoryChoice->menu(m_pCategoryMenu);
    pBox = new Fl_Box(x(),
		      y() + 3 * DLG_INPUT_HEIGHT + 2 * DLG_BUTTON_HEIGHT +
		      6 * DLG_BORDER, PROMPT_WIDTH, DLG_BUTTON_HEIGHT,
		      _("Description:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pDesc =
	new Fl_Output(x() + PROMPT_WIDTH + PROMPT_BORDER,
		      y() + 3 * DLG_INPUT_HEIGHT + 2 * DLG_BUTTON_HEIGHT +
		      6 * DLG_BORDER,
		      w() - PROMPT_WIDTH - IMAGE_BUTTON_WIDTH -
		      2 * PROMPT_BORDER, DLG_INPUT_HEIGHT);
    m_pDesc->color(FL_GRAY);
    m_pNoteButton = new Fl_Button(x() + w() - IMAGE_BUTTON_WIDTH,
				  y() + 3 * DLG_INPUT_HEIGHT +
				  2 * DLG_BUTTON_HEIGHT + 6 * DLG_BORDER,
				  IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT);
    m_pNotePixmap = Images::GetNotesIcon();
    m_pNotePixmap->label(m_pNoteButton);
    m_pNoteButton->callback(NoteButtonCallback);
    pBox = new Fl_Box(x(),
		      y() + 4 * DLG_INPUT_HEIGHT + 2 * DLG_BUTTON_HEIGHT +
		      7 * DLG_BORDER, PROMPT_WIDTH, DLG_BUTTON_HEIGHT,
		      _("Complete:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pCompleteButton =
	new Fl_Check_Button(x() + PROMPT_WIDTH + PROMPT_BORDER,
			    y() + 4 * DLG_INPUT_HEIGHT +
			    2 * DLG_BUTTON_HEIGHT + 7 * DLG_BORDER,
			    IMAGE_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCompleteButton->down_box(FL_ROUND_DOWN_BOX);
    m_pApplyButton =
	new Fl_Button(x() + w() - DLG_BUTTON_WIDTH - PROMPT_BORDER,
		      y() + 4 * DLG_INPUT_HEIGHT + 3 * DLG_BUTTON_HEIGHT +
		      8 * DLG_BORDER, DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT,
		      _("&Apply"));
    m_pApplyButton->callback(OnApply);
    m_pApplyButton->deactivate();
    end();

    // Set that nothing is resizable
    resizable(new Fl_Box(0, y() + h() - 1, 1, 1));

    // Set that no item is being displayed
    m_nID = -1;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ToDoListDetails::~ToDoListDetails()
{
    FreeTranslatedMenu(m_pCategoryMenu);
    delete m_pCalendarPixmap;
    delete m_pNotePixmap;
}


//--------------------------------------------------------------//
// The completed flag has been changed in the list, change in   //
// the details if needed.                                       //
//--------------------------------------------------------------//
void
ToDoListDetails::ChangeComplete(int nRow, int nComplete)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    if (pToDoListDB->GetID(nRow) == m_nID) {
	m_pCompleteButton->value(nComplete);
    }
}


//--------------------------------------------------------------//
// The due date/date completed has been changed in the list,    //
// change in the details if needed.                             //
//--------------------------------------------------------------//
void
ToDoListDetails::ChangeTime(int nRow, time_t nTime)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    if (pToDoListDB->GetID(nRow) == m_nID) {
	m_pDate->value(::FormatDate(nTime).c_str());
	m_nDate = nTime;
	m_pDayOfWeek->value(::FormatDayOfWeek(m_nDate).c_str());
    }
}


//--------------------------------------------------------------//
// Process a click on the date button                           //
//--------------------------------------------------------------//
void
ToDoListDetails::DateButtonCallback(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pDlg;
    time_t nStartDate = time(NULL);
    ToDoListDetails *pThis =
	reinterpret_cast < ToDoListDetails * >(pWidget->parent());

    // Invoke the date picker dialog
    if (pThis->m_nDate < 24 * 60 * 60) {
	nStartDate = NormalizeDate(time(NULL));
    } else {
	nStartDate = pThis->m_nDate;
    }

    pDlg =
	new DatePickerDlg(nStartDate, DatePicker::Daily,
			  PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	pThis->m_nDate = pDlg->GetDate();
	pThis->m_pDate->value(::FormatDate(pThis->m_nDate).c_str());
	pThis->m_pDayOfWeek->value(::FormatDayOfWeek(pThis->m_nDate).c_str());
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Display a ToDo Item                                          //
//--------------------------------------------------------------//
void
ToDoListDetails::DisplayRow(int nRow, int nRows)
{
    int nCategoryNo;
    int nPrio;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Enable everything if there are some rows
    if (nRow >= 0 && nRows > 0) {
	// Enable all input objects
	Enable(true);

	// Change the display if this is a different row in the data base
	if (pToDoListDB->GetID(nRow) != m_nID) {
	    // Save any outstanding changes
	    SaveChanges(true);

	    // Display this row
	    m_pTitle->value(pToDoListDB->GetTitle(nRow).c_str());

	    // Get the priority
	    nPrio = pToDoListDB->GetPriority(nRow);
	    if (nPrio < 0 || nPrio >= 5) {
		nPrio = 0;
	    }
	    m_pPriority[nPrio]->setonly();

	    // Set up the date
	    m_nDate = pToDoListDB->GetTime(nRow);
	    m_pDate->value(pToDoListDB->GetTimeString(nRow).c_str());
	    m_pDayOfWeek->value(::FormatDayOfWeek(m_nDate).c_str());

	    // Set the category
	    nCategoryNo =
		ToDoListCategoryDB::GetToDoListCategoryDB()->FindRow(CATID,
								     pToDoListDB->
								     GetCategory
								     (nRow));
	    if (nCategoryNo < 0) {
		// Fix for a bad category
		nCategoryNo = 0;
	    }
	    m_pCategoryChoice->value(nCategoryNo);

	    // Set the description (like a note)
	    m_pDesc->
		value(::
		      WrapText(pToDoListDB->GetDescription(nRow).c_str(),
			       m_pDesc->w() - 2 * Fl::box_dx(FL_DOWN_BOX),
			       m_pDesc).c_str());

	    // Set the completed flag
	    m_pCompleteButton->value(pToDoListDB->GetComplete(nRow) !=
				     0 ? 1 : 0);

	    // Note the key-id being displayed
	    m_nID = pToDoListDB->GetID(nRow);
	}
    } else {
	// Disable everything if there are no rows for the category choice
	Enable(false);
	m_nID = -1;
    }
}


//--------------------------------------------------------------//
// Enable or disable all input fields.                          //
//--------------------------------------------------------------//
void
ToDoListDetails::Enable(bool bEnable)
{
    int i;

    if (bEnable == true) {
	// Activate everything
	m_pApplyButton->activate();
	m_pNoteButton->activate();
	for (i = 0; i < 5; ++i) {
	    m_pPriority[i]->activate();
	}
	m_pCompleteButton->activate();
	m_pCategoryChoice->activate();
	m_pDate->activate();
	m_pDate->color(FL_WHITE);
	delete m_pCalendarPixmap;
	m_pCalendarPixmap = Images::GetCalendarIcon();
	m_pCalendarPixmap->label(m_pDateButton);
	m_pDateButton->activate();
	m_pTitle->activate();
	m_pTitle->color(FL_WHITE);
	delete m_pNotePixmap;
	m_pNotePixmap = Images::GetNotesIcon();
	m_pNotePixmap->label(m_pNoteButton);
    } else {
	// Deactivate everything
	m_pApplyButton->deactivate();
	m_pPriority[0]->setonly();
	for (i = 0; i < 5; ++i) {
	    m_pPriority[i]->deactivate();
	}
	m_pCompleteButton->value(0);
	m_pCompleteButton->deactivate();
	m_pCategoryChoice->value(0);
	m_pCategoryChoice->deactivate();
	m_pDate->value("");
	m_pDate->deactivate();
	m_pDate->color(FL_GRAY);
	m_nDate = 0;
	m_pDayOfWeek->value(::FormatDayOfWeek(m_nDate).c_str());
	delete m_pCalendarPixmap;
	m_pCalendarPixmap =
	    Images::GetDisabledImage(Images::DISABLED_CALENDAR_ICON);
	m_pCalendarPixmap->label(m_pDateButton);
	m_pDateButton->deactivate();
	m_pTitle->value("");
	m_pTitle->deactivate();
	m_pTitle->color(FL_GRAY);
	delete m_pNotePixmap;
	m_pNotePixmap = Images::GetDisabledImage(Images::DISABLED_NOTES);
	m_pNotePixmap->label(m_pNoteButton);
	m_pNoteButton->deactivate();
    }
}


//--------------------------------------------------------------//
// Process a message from the parent widget                     //
//--------------------------------------------------------------//
int
ToDoListDetails::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case -1:			// Not implemented yet
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
// Process a click on the Notes/Description button              //
//--------------------------------------------------------------//
void
ToDoListDetails::NoteButtonCallback(Fl_Widget * pWidget, void *pUserData)
{
    int nRow;
    Note *pNote;
    NoteEditorDlg *pDlg;
    ToDoListDetails *pThis =
	reinterpret_cast < ToDoListDetails * >(pWidget->parent());
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Invoke the Note Editor dialog
    pNote = new Note(pToDoListDB->GetColumnSize(TODO_DESC));
    nRow = pToDoListDB->FindRow(TODO_ID, pThis->m_nID);
    pNote->SetText(pToDoListDB->GetDescription(nRow).c_str());
    pDlg = new NoteEditorDlg(pNote, PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	// Save the changed note
	pNote = pDlg->GetNote();
	if (pNote->IsChanged() == true) {
	    pToDoListDB->SetDescription(nRow, pNote->GetText().c_str());
	    pToDoListDB->Save();

	    // Set the description (like a note)
	    pThis->m_pDesc->
		value(::
		      WrapText(pToDoListDB->GetDescription(nRow).c_str(),
			       pThis->m_pDesc->w() -
			       2 * Fl::box_dx(FL_DOWN_BOX),
			       pThis->m_pDesc).c_str());

	    // Tell everyone of the change, (DisplayRow in this class will not refresh due to same row being displayed)
	    PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	}
    }
    // Clean up
    delete pNote;
    delete pDlg;
}


//--------------------------------------------------------------//
// Apply changed to a note                                      //
//--------------------------------------------------------------//
void
ToDoListDetails::OnApply(Fl_Widget * pWidget, void *pUserData)
{
    ToDoListDetails *pThis = (ToDoListDetails *) (pWidget->parent());

    // Save any changes without asking whether to do so or not
    pThis->SaveChanges(false);
}


//--------------------------------------------------------------//
// Process a newly entered date.  The entered date must be a    //
// valid date in YY/MM/DD or YYYY/MM/DD format.  If only one    //
// delimiter is found then the date is assumed to be in MM/DD   //
// format for the current year.                                 //
//--------------------------------------------------------------//
bool
ToDoListDetails::ProcessDate()
{
    bool bReturn;
    int nResult;

    // Test the date
    nResult =::ValidateDate(m_pDate->value(), m_nDate);

    // Determine the results of the validation
    if (nResult < 0) {
	fl_alert(_("The due/completed date is not a valid date:\n\n%s"),::
		 GetDateError(nResult));
	bReturn = false;
    } else {
	m_pDate->value(::FormatDate(m_nDate).c_str());
	bReturn = true;
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Save any changes to the current ToDo Item.  A return of 0    //
// indicates that the user bypassed saving outstanding changes. //
//--------------------------------------------------------------//
int
ToDoListDetails::SaveChanges(bool bAsk)
{
    int i;
    int nCategory;
    int nPrio;
    int nResult;
    int nReturn = 1;
    int nRow;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Find the current row
    nRow = pToDoListDB->FindRow(TODO_ID, m_nID);
    if (nRow >= 0) {
	// Get the category id
	nCategory =
	    ToDoListCategoryDB::GetToDoListCategoryDB()->
	    GetCategoryID(m_pCategoryChoice->value());

	// Get the selected priority
	for (i = 0, nPrio = 0; i < 5; ++i) {
	    if (m_pPriority[i]->value() != 0) {
		nPrio = i;
		break;
	    }
	}

	// Has anything been changed (changes to description/TODO_DESC have already been written to disk using note processing)
	if (pToDoListDB->GetCategory(nRow) != nCategory
	    || pToDoListDB->GetComplete(nRow) != m_pCompleteButton->value()
	    || pToDoListDB->GetPriority(nRow) != nPrio
	    || strcmp(pToDoListDB->GetTitle(nRow).c_str(),
		      m_pTitle->value()) != 0
	    || strcmp(pToDoListDB->GetTimeString(nRow).c_str(),
		      m_pDate->value()) != 0) {
	    // Does the user want to save these changes
	    if (bAsk == true) {
		nResult =
		    fl_choice(_
			      ("Do you wish to save the changes to the To Do item:\n\n%s"),
			      fl_no, fl_yes, fl_cancel, m_pTitle->value());
		if (nResult == 1) {
		    // Yes button, indicate to save
		    nResult = 1;
		} else if (nResult == 0) {
		    // No button, indicate to not save
		    nResult = 0;
		} else		//if (nResult==2)
		{
		    // Cancel button, indicate no save and cancel the calling operation
		    nResult = 0;
		    nReturn = 0;
		}
	    } else {
		// Save without asking
		nResult = 1;
	    }

	    // Save the changes
	    if (nResult == 1) {
		// Validate the date string
		if (ProcessDate() == true) {
		    pToDoListDB->SetCategory(nRow, nCategory);
		    pToDoListDB->SetComplete(nRow,
					     m_pCompleteButton->value());
		    pToDoListDB->SetPriority(nRow, nPrio);
		    pToDoListDB->SetTitle(nRow, m_pTitle->value());
		    pToDoListDB->SetTime(nRow, m_nDate);
		    pToDoListDB->Save();

		    // OK button was pressed, refresh displays
		    PixilDT::GetApp()->GetMainWindow()->
			Notify(TODO_LIST_CHANGED, 0);
		} else {
		    // Date is not valid, cancel the calling process
		    nReturn = 0;
		}
	    }
	}
    }

    return (nReturn);
}
