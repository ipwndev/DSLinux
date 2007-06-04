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
// Scheduler Weekly tab page                                    //
//--------------------------------------------------------------//
#include "config.h"
#include "AddressBookCategoryDB.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "PixilDT.h"
#include "Printer.h"
#include "Scheduler.h"
#include "SchedulerDB.h"
#include "SchedulerDaily.h"
#include "TimeFunc.h"
#include "ToDoListCategoryDB.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerDaily::SchedulerDaily(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight, _("Daily"))
{
    Fl_Group *pGroup;
    int nDatePickerHeight = DatePicker::GetHeight(DatePicker::Daily);
    int nGroupHeight;
    int nGroupWidth;
    int nGroupX;
    int nGroupY;

    // Create this page
    pGroup = new Fl_Group(x() + DLG_BORDER,
			  y() + DLG_BORDER,
			  w() - 3 * DLG_BORDER - DatePicker::GetWidth(),
			  h() - 2 * DLG_BORDER);
    resizable(pGroup);
    nGroupX = pGroup->x();
    nGroupY = pGroup->y();
    nGroupHeight = pGroup->h();
    nGroupWidth = pGroup->w();
    m_pDate = new Fl_Box(nGroupX, nGroupY, nGroupWidth, DLG_INPUT_HEIGHT);
    m_pDate->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_pDate->box(FL_DOWN_BOX);
    m_pDate->color(FL_WHITE);
    m_pScheduleContainer = new ScheduleContainer(nGroupX, nGroupY + DLG_BORDER + DLG_INPUT_HEIGHT, nGroupWidth, nGroupHeight - 2 * DLG_BORDER - DLG_INPUT_HEIGHT - DLG_BUTTON_HEIGHT, 1);	// Contains one day
    m_pTodayButton =
	new Fl_Button(nGroupX + nGroupWidth - 2 * DLG_BORDER -
		      3 * DLG_BUTTON_WIDTH,
		      nGroupY + nGroupHeight - DLG_BUTTON_HEIGHT,
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&Today"));
    m_pTodayButton->callback(TodayCallback);
    m_pEditButton =
	new Fl_Button(nGroupX + nGroupWidth - 1 * DLG_BORDER -
		      2 * DLG_BUTTON_WIDTH,
		      nGroupY + nGroupHeight - DLG_BUTTON_HEIGHT,
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&Edit"));
    m_pEditButton->callback(OnEditButton);
    m_pNewButton = new Fl_Button(nGroupX + nGroupWidth - 1 * DLG_BUTTON_WIDTH,
				 nGroupY + nGroupHeight - DLG_BUTTON_HEIGHT,
				 DLG_BUTTON_WIDTH,
				 DLG_BUTTON_HEIGHT, _("&New"));
    m_pNewButton->callback(NewCallback);

    // End this first group
    pGroup->end();

    // Make it resizable
    pGroup->resizable(m_pScheduleContainer);
    resizable(pGroup);

    // Create the second group
    pGroup = new Fl_Group(x() + w() - DLG_BORDER - DatePicker::GetWidth(),
			  y() + DLG_BORDER,
			  DatePicker::GetWidth(), h() - 2 * DLG_BORDER);
    nGroupX = pGroup->x();
    nGroupY = pGroup->y();
    nGroupWidth = pGroup->w();
    m_pDatePicker = new DatePicker(nGroupX,
				   nGroupY, DatePicker::Daily, time(NULL));
    m_pAddressButton = new Fl_Button(nGroupX,
				     nGroupY + DLG_BORDER + nDatePickerHeight,
				     (3 * (nGroupWidth - DLG_BORDER)) / 10,
				     DLG_BUTTON_HEIGHT, _("Address"));
    m_pAddressButton->
	align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_pAddressButton->callback(OnAddressButton);
    m_pAddressButton->labelfont(FL_HELVETICA_BOLD);
    m_pAddressButton->labelsize((4 * labelsize()) / 5);
    m_pToDoButton = new Fl_Button(nGroupX + m_pAddressButton->w(),
				  nGroupY + DLG_BORDER + nDatePickerHeight,
				  (3 * (nGroupWidth - DLG_BORDER)) / 10,
				  DLG_BUTTON_HEIGHT, _("To Do"));
    m_pToDoButton->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_pToDoButton->callback(OnToDoButton);
    m_pToDoButton->labelfont(FL_HELVETICA_BOLD);
    m_pToDoButton->labelsize((4 * labelsize()) / 5);
    m_pAddressBookCategory =
	new Fl_Choice(nGroupX + 2 * m_pAddressButton->w() + DLG_BORDER,
		      nGroupY + DLG_BORDER + nDatePickerHeight,
		      DatePicker::GetWidth() - 2 * m_pAddressButton->w() -
		      DLG_BORDER, DLG_BUTTON_HEIGHT);
    m_pAddressBookCategoryMenu =
	AddressBookCategoryDB::GetAddressBookCategoryDB()->
	GetCategoryMenuWithFont(true, FL_HELVETICA_BOLD,
				(4 * labelsize()) / 5);
    m_pAddressBookCategory->menu(m_pAddressBookCategoryMenu);
    m_pAddressBookCategory->callback(AddressBookCategorySelected);
    m_pToDoListCategory =
	new Fl_Choice(nGroupX + 2 * m_pAddressButton->w() + DLG_BORDER,
		      nGroupY + DLG_BORDER + nDatePickerHeight,
		      DatePicker::GetWidth() - 2 * m_pAddressButton->w() -
		      DLG_BORDER, DLG_BUTTON_HEIGHT);
    m_pToDoListCategoryMenu =
	AddressBookCategoryDB::GetAddressBookCategoryDB()->
	GetCategoryMenuWithFont(true, FL_HELVETICA_BOLD,
				(4 * labelsize()) / 5);
    m_pToDoListCategory->menu(m_pToDoListCategoryMenu);
    m_pToDoListCategory->callback(ToDoListCategorySelected);
    m_pAddressBookList = new AddressBookList(nGroupX, nGroupY + 2 * DLG_BORDER + DLG_BUTTON_HEIGHT + nDatePickerHeight, nGroupWidth, pGroup->h() - 2 * DLG_BORDER - DLG_BUTTON_HEIGHT - nDatePickerHeight, true);	// Small list
    m_pAddressBookList->SetDoubleClick(OnAddressGoto);
    m_pToDoListList = new ToDoListList(nGroupX, nGroupY + 2 * DLG_BORDER + DLG_BUTTON_HEIGHT + nDatePickerHeight, nGroupWidth, pGroup->h() - 2 * DLG_BORDER - DLG_BUTTON_HEIGHT - nDatePickerHeight, true);	// Small list
    m_pToDoListList->SetDoubleClick(OnToDoGoto);

    // End this group
    pGroup->end();

    // Make the lists resizable
    pGroup->resizable(m_pAddressBookList);

    // End the tab page
    end();

    // Go display today's date
    DisplayDay(time(NULL));

    // Go display the address book
    DisplayAddressBook();

    // Process any further date picks
    m_pDatePicker->SetNotify(OnDatePick);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SchedulerDaily::~SchedulerDaily()
{
    FreeTranslatedMenu(m_pAddressBookCategoryMenu);
    FreeTranslatedMenu(m_pToDoListCategoryMenu);
}


//--------------------------------------------------------------//
// An address book category has been selected.  The choice      //
// widget is within a group within this page hence the          //
// parent()->parent() to get to this object.                    //
//--------------------------------------------------------------//
void
SchedulerDaily::AddressBookCategorySelected(Fl_Widget * pWidget,
					    void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Use value -1 so that all is category -1
    pThis->m_pAddressBookList->Filter(((Fl_Choice *) pWidget)->value() - 1);
}


//--------------------------------------------------------------//
// Display the address book                                     //
//--------------------------------------------------------------//
void
SchedulerDaily::DisplayAddressBook()
{
    // Fix up the buttons
    m_pAddressButton->value(1);
    m_pToDoButton->value(0);
    m_pAddressBookList->show();
    m_pToDoListList->hide();

    // Fix the category choice widgets
    m_pAddressBookCategory->show();
    m_pToDoListCategory->hide();
}


//--------------------------------------------------------------//
// Display a particular day                                     //
//--------------------------------------------------------------//
void
SchedulerDaily::DisplayDay(time_t nDate)
{
    int nDay;
    int nMonth;
    int nYear;
    time_t nToday;
    struct tm *pTm = localtime(&nDate);

    // Save the displayed date
    m_nDate =::NormalizeDate(nDate);
    nDay = pTm->tm_mday;
    nMonth = pTm->tm_mon;
    nYear = pTm->tm_year;

    // Set up the name of the day in the title of the page
    sprintf(m_szDate,
	    "%s, %s %d, %d",::GetDayOfWeek(m_nDate).
	    c_str(),::GetMonthName(m_nDate).c_str(), pTm->tm_mday,
	    pTm->tm_year + 1900);

    // Is this date today ?
    nDate = time(NULL);
    pTm = localtime(&nDate);
    if (nDay == pTm->tm_mday
	&& nMonth == pTm->tm_mon && nYear == pTm->tm_year) {
	m_pDate->labelcolor(FL_RED);
	m_pTodayButton->deactivate();
    } else {
	m_pDate->labelcolor(FL_BLACK);
	m_pTodayButton->activate();
    }

    // Enable or disable the Today button based on what day is being shown
    nToday = time(NULL);
    if (::NormalizeDate(nToday) == m_nDate) {
	m_pTodayButton->deactivate();
    } else {
	m_pTodayButton->activate();
    }

    // Refresh the container widget
    m_pScheduleContainer->Refresh(m_nDate);

    // Set the text of the widget
    m_pDate->label(m_szDate);
    m_pDate->redraw();

    // Select this date in the date picker
    m_pDatePicker->SetDate(m_nDate);
}


//--------------------------------------------------------------//
// Display the ToDo List                                        //
//--------------------------------------------------------------//
void
SchedulerDaily::DisplayToDoList()
{
    // Fix up the buttons
    m_pAddressButton->value(0);
    m_pToDoButton->value(1);
    m_pAddressBookList->hide();
    m_pToDoListList->show();

    // Fix the category choice widgets
    m_pAddressBookCategory->hide();
    m_pToDoListCategory->show();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
SchedulerDaily::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:	// Fix the address book display
	m_pAddressBookList->Message(nMessage, nInfo);
	break;

    case BEGIN_WEEK_CHANGED:	// Beginning day of week has changed
	m_pDatePicker->Message(nMessage, nInfo);
	break;

    case SCHEDULER_CHANGED:	// Scheduler data changed, go refresh the display
	m_pScheduleContainer->Refresh(m_nDate);
	break;

    case TODO_LIST_CHANGED:	// Fix the to do list display
	m_pToDoListList->Message(nMessage, nInfo);
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
// Process a click on the New button.  The button is within a   //
// group within this page hence the parent()->parent() to get   //
// to this object.                                              //
//--------------------------------------------------------------//
void
SchedulerDaily::NewCallback(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Go add a new appointment (parent is an Fl_Tabs whose parent is the Scheduler object
    ((Scheduler *) (pThis->parent()->parent()))->EditNew(pThis->m_nDate +
							 12 * 60 * 60);
}


//--------------------------------------------------------------//
// Process a click on the Address button.                       //
//--------------------------------------------------------------//
void
SchedulerDaily::OnAddressButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Go add a new appointment (parent is an Fl_Tabs whose parent is the Scheduler object
    pThis->DisplayAddressBook();
}


//--------------------------------------------------------------//
// Process a double click on an Address Book line.              //
//--------------------------------------------------------------//
void
SchedulerDaily::OnAddressGoto(int nRecord, int nCol)
{
    // Tell the entire application, go display this Address Book line
    nRecord = AddressBookDB::GetAddressBookDB()->GetRecordNumber(nRecord);
    PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_GOTO, nRecord);
}


//--------------------------------------------------------------//
// Process a date pick.                                         //
//--------------------------------------------------------------//
void
SchedulerDaily::OnDatePick(Fl_Widget * pWidget, time_t nDate)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    pThis->DisplayDay(nDate);
}


//--------------------------------------------------------------//
// Process a click on the Edit button.                          //
//--------------------------------------------------------------//
void
SchedulerDaily::OnEditButton(Fl_Widget * pWidget, void *pUserData)
{
    int nRow;
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Get the row number of the currently selected appointment
    nRow = pThis->m_pScheduleContainer->GetSelectedItem();

    // Go add a new appointment (parent is an Fl_Tabs whose parent is the Scheduler object
    if (nRow >= 0) {
	((Scheduler *) (pThis->parent()->parent()))->Edit(nRow,
							  pThis->m_nDate);
    }
}


//--------------------------------------------------------------//
// Process a click on the ToDo button.                          //
//--------------------------------------------------------------//
void
SchedulerDaily::OnToDoButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Go add a new appointment (parent is an Fl_Tabs whose parent is the Scheduler object
    pThis->DisplayToDoList();
}


//--------------------------------------------------------------//
// Process a double click on a ToDo item.                       //
//--------------------------------------------------------------//
void
SchedulerDaily::OnToDoGoto(int nRow, int nCol)
{
    int nRecord = ToDoListDB::GetToDoListDB()->GetRecordNumber(nRow);

    // Tell the entire application, go display this ToDo item
    PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_GOTO, nRecord);
}


//--------------------------------------------------------------//
// Print this data.                                             //
//--------------------------------------------------------------//
void
SchedulerDaily::Print()
{
    bool bFirst;
    char szData[64];
    int nBorder = INCH / 2;	// 1/2 inch border
    int nCopy;
    const int nFontSize = 10;
    multimap < int, int >mRecord;
    multimap < int, int >::iterator iter;
    Printer printer;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    string strData;
    time_t nTime;
    struct tm *pTm;

    // Open the printer
    if (printer.Open(_("SchedulerDaily")) == true) {
	// Set an hourglass cursor
	PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_WAIT);

	// Print once for eqch requested copy
	for (nCopy = 0; nCopy < printer.GetCopies(); ++nCopy) {
	    // Reset the page number
	    printer.ResetPageNumber();

	    // Get the current date
	    pTm = localtime(&m_nDate);
	    sprintf(szData,
		    "%s %d, %d",::GetMonthName(m_nDate).c_str(),
		    pTm->tm_mday, pTm->tm_year + 1900);

	    // Go to two column mode
	    printer.SetSerifFont(nFontSize);
	    printer.SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				     nBorder,
				     _("Scheduler Daily"), szData, 184);

	    // Print each visible entry
	    pSchedulerDB->GetAllAppointments(m_nDate, mRecord);

	    // Print each row
	    for (iter = mRecord.begin(), bFirst = true;
		 iter != mRecord.end(); ++iter) {
		// Has the date changed (it never will, but the logic works)
		if (bFirst == true) {
		    sprintf(szData,
			    "%s %s",::GetDayOfWeek(m_nDate).
			    c_str(),::FormatDate(m_nDate).c_str());

		    // Yes, output a box for the date
		    printer.ColumnSpaceLine();
		    printer.ColumnSpaceLine();
		    printer.ColumnSpaceLine();
		    printer.ColumnBox(szData, 15, 5 * INCH / 16, 184);
		    bFirst = false;
		} else {
		    printer.ColumnNewLine();
		}

		// Output the time
		nTime = pSchedulerDB->GetStartTime(iter->second);
		strData =::FormatTime(nTime);
		strData += " - ";
		nTime = pSchedulerDB->GetEndTime(iter->second);
		strData +=::FormatTime(nTime);
		printer.SetBoldSerifFont(nFontSize);
		printer.ColumnShowNoAdvance(strData.c_str(), 0);

		// Output the description
		printer.SetSerifFont(nFontSize);
		strData = pSchedulerDB->GetDescription(iter->second);
		printer.ColumnShow(strData.c_str(), (3 * INCH) / 2, 0);
	    }

	    // End the page
	    printer.EndTwoColumnMode();
	}

	// All done, close the printer
	printer.Close();

	// Reset the cursor
	PixilDT::GetApp()->GetMainWindow()->ResetCursor();
    }
}


//--------------------------------------------------------------//
// Process a click on the Today button.  The button is within a //
// group within this page hence the parent()->parent() to get   //
// to this object.                                              //
//--------------------------------------------------------------//
void
SchedulerDaily::TodayCallback(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Go display the current date
    pThis->DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// A ToDo List category has been selected.  The choice  widget //
// is within a group within this page hence the                //
// parent()->parent() to get to this object.                    //
//--------------------------------------------------------------//
void
SchedulerDaily::ToDoListCategorySelected(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerDaily *pThis =
	reinterpret_cast < SchedulerDaily * >(pWidget->parent()->parent());

    // Use value -1 so that all is category -1
    pThis->m_pToDoListList->Filter(((Fl_Choice *) pWidget)->value() - 1);
}
