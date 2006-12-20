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
#include <cassert>
#include <climits>
#include "DatePickerDlg.h"
#include "Dialog.h"
#include "Images.h"
#include "Options.h"
#include "PixilDT.h"
#include "Printer.h"
#include "SchedulerDB.h"
#include "SchedulerWeekly.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


using namespace std;


#define BUTTON_BORDER 2


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerWeekly::SchedulerWeekly(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight, _("Weekly"))
{
    Fl_Group *pGroup;
    Fl_Group *pGroup2;
    int i;
    int nDayWidth;
    int nContainerWidth;
    int nGroupWidth;
    int nGroupX;
    int nGroupY;

    // Create the top line of the page, in a group for resizing
    pGroup = new Fl_Group(x() + DLG_BORDER,
			  y() + DLG_BORDER,
			  w() - 2 * DLG_BORDER, DLG_INPUT_HEIGHT);
    nGroupX = pGroup->x();
    nGroupY = pGroup->y();
    nGroupWidth = pGroup->w();
    m_pLeftButton = new Fl_Button(nGroupX,
				  nGroupY,
				  IMAGE_BUTTON_WIDTH, DLG_INPUT_HEIGHT);
    m_pLeftPixmap = Images::GetBigLeftIcon();
    m_pLeftPixmap->label(m_pLeftButton);
    m_pLeftButton->callback(OnLeftButton);
    m_pGoToButton =
	new Fl_Button(nGroupX + IMAGE_BUTTON_WIDTH + BUTTON_BORDER, nGroupY,
		      DLG_BUTTON_WIDTH, DLG_INPUT_HEIGHT, _("&Go To"));
    m_pGoToButton->callback(OnGotoButton);
    m_pRightButton =
	new Fl_Button(nGroupX + IMAGE_BUTTON_WIDTH + 2 * BUTTON_BORDER +
		      DLG_BUTTON_WIDTH, nGroupY, IMAGE_BUTTON_WIDTH,
		      DLG_INPUT_HEIGHT);
    m_pRightPixmap = Images::GetBigRightIcon();
    m_pRightPixmap->label(m_pRightButton);
    m_pRightButton->callback(OnRightButton);
    m_pDate =
	new Fl_Box(nGroupX + DLG_BORDER + 2 * IMAGE_BUTTON_WIDTH +
		   2 * BUTTON_BORDER + DLG_BUTTON_WIDTH, nGroupY,
		   nGroupWidth - 2 * DLG_BORDER - 2 * IMAGE_BUTTON_WIDTH -
		   2 * BUTTON_BORDER - 2 * DLG_BUTTON_WIDTH,
		   DLG_INPUT_HEIGHT);
    m_pDate->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_pDate->box(FL_DOWN_BOX);
    m_pDate->color(FL_WHITE);
    m_pTodayButton = new Fl_Button(nGroupX + nGroupWidth - DLG_BUTTON_WIDTH,
				   nGroupY,
				   DLG_BUTTON_WIDTH,
				   DLG_INPUT_HEIGHT, _("&Today"));
    m_pTodayButton->callback(OnTodayButton);
    pGroup->end();
    pGroup->resizable(m_pDate);

    // Create the weekly schedule
    m_pScheduleContainer = new ScheduleContainer(x() + DLG_BORDER, y() + 2 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT + BUTTON_BORDER, w() - 2 * DLG_BORDER, h() - 3 * DLG_BORDER - 2 * DLG_INPUT_HEIGHT - BUTTON_BORDER, 7);	// Contains 7 days

    // Set up to resize properly
    pGroup = new Fl_Group(x() + DLG_BORDER,
			  y() + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
			  w() - 2 * DLG_BORDER, DLG_INPUT_HEIGHT);
    pGroup2 =
	new Fl_Group(x() + DLG_BORDER + m_pScheduleContainer->GetHoursWidth(),
		     y() + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
		     w() - 2 * DLG_BORDER -
		     m_pScheduleContainer->GetHoursWidth(), DLG_INPUT_HEIGHT);
    pGroup->resizable(pGroup2);

    // Create the day-of-week title boxes
    nContainerWidth = m_pScheduleContainer->GetContainerWidth();
    nDayWidth = m_pScheduleContainer->GetContainerWidth() / 7;
    for (i = 0; i < 7; ++i) {
	m_pDow[i] =
	    new Fl_Box(x() + DLG_BORDER +
		       m_pScheduleContainer->GetHoursWidth() + i * nDayWidth,
		       y() + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
		       (i ==
			6 ? m_pScheduleContainer->GetContainerWidth() -
			6 * nDayWidth : nDayWidth), DLG_INPUT_HEIGHT);
	m_pDow[i]->labelfont(FL_HELVETICA_BOLD);
	m_pDow[i]->labelsize((4 * labelsize()) / 5);
	m_pDow[i]->box(FL_FLAT_BOX);
    }

    // Finish the resizing groups
    pGroup2->end();
    pGroup->end();

    // Finish with this widget
    end();

    // Set that the container is resizable
    resizable(m_pScheduleContainer);

    // Go display today's date
    DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SchedulerWeekly::~SchedulerWeekly()
{
    delete m_pLeftPixmap;
    delete m_pRightPixmap;
}


//--------------------------------------------------------------//
// Display a particular day.                                    //
//--------------------------------------------------------------//
void
SchedulerWeekly::DisplayDay(time_t nDate)
{
    char szString[16];
    int i;
    int nDays;
    int nWeekBegin = Options::GetWeekBegins();
    time_t nToday =::NormalizeDate(time(NULL));
    struct tm *pTm;

    // Get the Sunday or Monday prior to the requested date
    nDate =::NormalizeDate(nDate);
    nDays = GetDow(nDate) - nWeekBegin;
    if (nDays < 0) {
	nDays += 7;
    }
    nDate =::SubtractDays(nDate, nDays);
    m_nDate = nDate;

    // Refresh the container
    m_pScheduleContainer->Refresh(nDate);

    // Fix the day of week titles
    for (i = 0; i < 7; ++i) {
	m_strBoxLabel[i] =::GetDayOfWeekAbbr((i + nWeekBegin) % 7);
	pTm = localtime(&nDate);
	sprintf(szString, "%d", pTm->tm_mday);
	m_strBoxLabel[i] += ' ';
	m_strBoxLabel[i] += szString;
	m_pDow[i]->label(m_strBoxLabel[i].c_str());
	if (nDate == nToday) {
	    m_pDow[i]->labelcolor(FL_RED);
	} else {
	    m_pDow[i]->labelcolor(FL_BLACK);
	}
	nDate =::AddDays(nDate, 1);
    }

    // Fix the banner title's label
    nDate = m_nDate;
    pTm = localtime(&nDate);
    m_strDateLabel =::GetMonthName(m_nDate);
    sprintf(szString, " %d, %d", pTm->tm_mday, pTm->tm_year + 1900);
    m_strDateLabel += szString;
    m_strDateLabel += " - ";
    nDate =::AddDays(nDate, 6);
    m_strDateLabel +=::GetMonthName(nDate);
    pTm = localtime(&nDate);
    sprintf(szString, " %d, %d", pTm->tm_mday, pTm->tm_year + 1900);
    m_strDateLabel += szString;
    m_strDateLabel += "  ";
    m_strDateLabel += _("Week");
    m_strDateLabel += ' ';
    sprintf(szString, "%d", (pTm->tm_yday) / 7 + 1);
    m_strDateLabel += szString;
    m_pDate->label(m_strDateLabel.c_str());

    // Enable or disable the Today button based on what day is being shown
    nDays = GetDow(nToday) - nWeekBegin;
    if (nDays < 0) {
	nDays += 7;
    }
    nToday =::SubtractDays(nToday, nDays);
    if (nToday == m_nDate) {
	m_pTodayButton->deactivate();
    } else {
	m_pTodayButton->activate();
    }

    // Redraw the entire widget
    redraw();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
SchedulerWeekly::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value
    int nWDay;
    time_t nDate;

    switch (nMessage) {
    case BEGIN_WEEK_CHANGED:	// Begin of week has been changed
	nWDay = Options::GetWeekBegins();
	if (nWDay == 0) {
	    // Moving back to Sunday
	    nDate =::SubtractDays(m_nDate, 1);
	} else {
	    // Moving forward to Monday
	    nDate =::AddDays(m_nDate, 1);
	}
	DisplayDay(nDate);
	break;

    case SCHEDULER_CHANGED:	// Scheduler data changed, go refresh the display
	DisplayDay(m_nDate);
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
// Process a click on the goto button.                          //
//--------------------------------------------------------------//
void
SchedulerWeekly::OnGotoButton(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pDlg;
    SchedulerWeekly *pThis =
	reinterpret_cast < SchedulerWeekly * >(pWidget->parent()->parent());

    pDlg = new DatePickerDlg(pThis->m_nDate,
			     DatePicker::Weekly,
			     PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	// The OK button was pressed, refresh this display
	pThis->DisplayDay(pDlg->GetDate());
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Process a click on the left button.                          //
//--------------------------------------------------------------//
void
SchedulerWeekly::OnLeftButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerWeekly *pThis =
	reinterpret_cast < SchedulerWeekly * >(pWidget->parent()->parent());

    // Only go left if not at beginning of the possible time range
    if (pThis->m_nDate > 8 * 24 * 60 * 60) {
	pThis->DisplayDay(::SubtractDays(pThis->m_nDate, 7));
    }
}


//--------------------------------------------------------------//
// Process a click on the right button.                         //
//--------------------------------------------------------------//
void
SchedulerWeekly::OnRightButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerWeekly *pThis =
	reinterpret_cast < SchedulerWeekly * >(pWidget->parent()->parent());

    // Only go right if not at end of the possible time range
    if (pThis->m_nDate <= LONG_MAX - 2 * 7 * 24 * 60 * 60) {
	pThis->DisplayDay(::AddDays(pThis->m_nDate, 7));
    }
}


//--------------------------------------------------------------//
// Process a click on the Today button.  The button is within a //
// group within this page hence the parent()->parent() to get   //
// to this object.                                              //
//--------------------------------------------------------------//
void
SchedulerWeekly::OnTodayButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerWeekly *pThis =
	reinterpret_cast < SchedulerWeekly * >(pWidget->parent()->parent());

    // Go display the current date
    pThis->DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Print this data.                                             //
//--------------------------------------------------------------//
void
SchedulerWeekly::Print()
{
    bool bFirst;
    char szData[64];
    int nBorder = INCH / 2;	// 1/2 inch border
    int nCopy;
    int nDay;
    const int nFontSize = 10;
    multimap < int, int >mRecord;
    multimap < int, int >::iterator iter;
    Printer printer;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    string strData;
    time_t nTime;
    struct tm *pTm;

    // Open the printer
    if (printer.Open(_("SchedulerWeekly")) == true) {
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
	    strData = szData;
	    strData += " - ";
	    nTime =::AddDays(m_nDate, 6);
	    pTm = localtime(&nTime);
	    sprintf(szData,
		    "%s %d, %d",::GetMonthName(m_nDate).c_str(),
		    pTm->tm_mday, pTm->tm_year + 1900);
	    strData += szData;

	    // Go to two column mode
	    printer.SetSerifFont(nFontSize);
	    printer.SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				     nBorder,
				     _("Scheduler Weekly"),
				     strData.c_str(), 184);

	    for (nDay = 0; nDay < 7; ++nDay) {
		// Print each visible entry for this day
		nTime =::AddDays(m_nDate, nDay);
		pSchedulerDB->GetAllAppointments(nTime, mRecord);

		// Print each row
		for (iter = mRecord.begin(), bFirst = true;
		     iter != mRecord.end(); ++iter) {
		    // Has the date changed (it never will, but the logic works)
		    if (bFirst == true) {
			sprintf(szData,
				"%s %s",::GetDayOfWeek(nTime).
				c_str(),::FormatDate(nTime).c_str());

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
