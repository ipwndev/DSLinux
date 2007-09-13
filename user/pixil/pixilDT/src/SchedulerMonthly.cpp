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
// Scheduler Monthly tab page                                   //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <climits>
#include <FL/Fl.H>
#include <FL/forms.H>

#include "DatePickerDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "Options.h"
#include "PixilDT.h"
#include "Printer.h"
#include "SchedulerDB.h"
#include "SchedulerMonthly.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


using namespace std;


#define BUTTON_BORDER 2


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerMonthly::SchedulerMonthly(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight, _("Monthly"))
{
    Fl_Group *pGroup;
    int i;
    int j;
    int nDayHeight;
    int nDayWidth;
    int nGroupHeight;
    int nGroupWidth;
    int nGroupX;
    int nGroupY;
    int nWeekBegins = Options::GetWeekBegins();

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

    // Create the day-of-week title boxes
    nDayWidth = (w() - 2 * DLG_BORDER - Fl::box_dw(FL_DOWN_BOX)) / 7;
    for (i = 0; i < 7; ++i) {
	m_pDow[i] =
	    new Fl_Box(x() + DLG_BORDER + (Fl::box_dw(FL_DOWN_BOX) >> 1) +
		       i * nDayWidth, y() + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
		       (i ==
			6 ? w() - 2 * DLG_BORDER - Fl::box_dw(FL_DOWN_BOX) -
			6 * nDayWidth : nDayWidth), DLG_INPUT_HEIGHT);
	m_pDow[i]->labelfont(FL_HELVETICA_BOLD);
	m_pDow[i]->labelsize((4 * labelsize()) / 5);
	m_pDow[i]->box(FL_FLAT_BOX);
	m_strDow[i] =::GetDayOfWeek((i + nWeekBegins) % 7);
	m_pDow[i]->label(m_strDow[i].c_str());
    }

    // Create the day boxes
    pGroup = new Fl_Group(x() + DLG_BORDER,
			  y() + 2 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
			  w() - 2 * DLG_BORDER,
			  h() - 3 * DLG_BORDER - 2 * DLG_INPUT_HEIGHT);
    pGroup->box(FL_DOWN_BOX);
    resizable(pGroup);
    nGroupX = pGroup->x();
    nGroupY = pGroup->y();
    nGroupWidth = pGroup->w();
    nGroupHeight = pGroup->h();
    nDayHeight = (nGroupHeight - Fl::box_dh(FL_DOWN_BOX)) / 6;
    for (i = 0; i < 6; ++i) {
	for (j = 0; j < 7; ++j) {
	    m_pDay[i][j] =
		new Fl_Browser(nGroupX + (Fl::box_dw(FL_DOWN_BOX) >> 1) +
			       j * nDayWidth,
			       nGroupY + (Fl::box_dh(FL_DOWN_BOX) >> 1) +
			       i * nDayHeight,
			       (j ==
				6 ? nGroupWidth - Fl::box_dw(FL_DOWN_BOX) -
				6 * nDayWidth : nDayWidth),
			       (i ==
				5 ? nGroupHeight - Fl::box_dh(FL_DOWN_BOX) -
				5 * nDayHeight : nDayHeight));
	    m_pDay[i][j]->box(FL_BORDER_BOX);
	}
    }

    // Finish with this widget
    end();

    // Go display today's date
    DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SchedulerMonthly::~SchedulerMonthly()
{
    delete m_pLeftPixmap;
    delete m_pRightPixmap;
}


//--------------------------------------------------------------//
// Build the display for a date.                                //
//--------------------------------------------------------------//
void
SchedulerMonthly::BuildDate(time_t nDate, int nDayNo)
{
    char szString[16];
    Fl_Browser *pBrowser;
    int nMaxWidth;
    int nRow;
    multimap < int, int >::iterator iter;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    string strText;
    string strText2;
    time_t nToday =::NormalizeDate(time(NULL));
    struct tm *pTm;

    // Clear the old contents
    pBrowser = m_pDay[nDayNo / 7][nDayNo % 7];
    pBrowser->clear();

    // Set the maximum width for a title
    nMaxWidth = pBrowser->w() - 20;	// 20 is a guess at a scrollbar width

    // Get the day of month for this day
    pTm = localtime(&nDate);
    strText = "@r@C";
    sprintf(szString, "%d", (nDate == nToday ? FL_RED : FL_BLACK));
    strText += szString;
    strText += "@.";
    sprintf(szString, "%d", pTm->tm_mday);
    strText += szString;
    pBrowser->add(strText.c_str());

    // Now add each event to the browser in order
    for (iter = m_pmEvent[nDayNo].begin(); iter != m_pmEvent[nDayNo].end();
	 ++iter) {
	nRow = iter->second;
	strText = pSchedulerDB->GetStartTimeString(nRow);
	strText += ' ';
	strText += pSchedulerDB->GetDescription(nRow);
	strText = WrapText(strText.c_str(), nMaxWidth, pBrowser);
	strText2 = "@.";
	strText2 += strText;
	pBrowser->add(strText2.c_str());
    }
}


//--------------------------------------------------------------//
// Display a particular day.                                    //
//--------------------------------------------------------------//
void
SchedulerMonthly::DisplayDay(time_t nDate)
{
    char szString[16];
    int i;
    int j;
    int nDays;
    int nMonth;
    int nWeekBegins = Options::GetWeekBegins();
    time_t nStartDate;
    time_t nToday =::NormalizeDate(time(NULL));
    struct tm *pTm;

    // Get the Sunday or Monday prior to the requested date
    pTm = localtime(&nDate);
    pTm->tm_isdst = -1;
    pTm->tm_sec = 0;
    pTm->tm_min = 0;
    pTm->tm_hour = 0;
    pTm->tm_mday = 1;
    m_nDate = mktime(pTm);
    nMonth = pTm->tm_mon;
    pTm = localtime(&m_nDate);
    nDays = pTm->tm_wday - nWeekBegins;
    if (nDays < 0) {
	nDays += 7;
    }
    nStartDate =::SubtractDays(m_nDate, nDays);

    // Fix the banner title's label
    pTm = localtime(&m_nDate);
    m_strDateLabel =::GetMonthName(m_nDate);
    sprintf(szString, " %d", pTm->tm_year + 1900);
    m_strDateLabel += szString;
    m_pDate->label(m_strDateLabel.c_str());

    // Get the events for each browser
    SchedulerDB::GetSchedulerDB()->GetEvents(m_pmEvent, nStartDate, 6 * 7);

    // Refresh each browser
    nDate = nStartDate;
    for (i = 0; i < 6; ++i) {
	for (j = 0; j < 7; ++j) {
	    pTm = localtime(&nDate);
	    m_pDay[i][j]->color(pTm->tm_mon == nMonth ? FL_WHITE : FL_GRAY);
	    BuildDate(nDate, i * 7 + j);
	    nDate =::AddDays(nDate, 1);
	}
    }

    // Enable or disable the Today button based on what day is being shown
    pTm = localtime(&nToday);
    pTm->tm_isdst = -1;
    pTm->tm_sec = 0;
    pTm->tm_min = 0;
    pTm->tm_hour = 0;
    pTm->tm_mday = 1;
    nToday = mktime(pTm);
    if (nToday == m_nDate) {
	m_pTodayButton->deactivate();
    } else {
	m_pTodayButton->activate();
    }

    // Redraw the entire widget
    redraw();
}


//--------------------------------------------------------------//
// Handle a double click by going to that day.                  //
//--------------------------------------------------------------//
int
SchedulerMonthly::handle(int nEvent)
{
    bool bFound = false;
    int i;
    int j;
    int nReturn;
    time_t nDate;

    // Tell Fl_Group about the event
    nReturn = Fl_Group::handle(nEvent);

    // Process here if a left mouse double click
    if (nEvent == FL_PUSH
	&& Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() > 0) {
	// Select this day and go to it
	for (i = 0; i < 6; ++i) {
	    for (j = 0; j < 7; ++j) {
		if (Fl::event_inside(m_pDay[i][j])) {
		    bFound = true;
		    break;
		}
	    }

	    // Break out if found
	    if (bFound == true) {
		break;
	    }
	}

	// Was this click in a day
	if (i < 6 && j < 7) {
	    // Go to this day, calculate the date
	    nDate =::AddDays(m_nDate,
			     i * 7 + j -::GetDow(m_nDate) +
			     Options::GetWeekBegins());
	    ((Scheduler *) (parent()->parent()))->SelectDay(nDate);
	}
	// Indicate that this event was processed
	nReturn = 1;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
SchedulerMonthly::Message(PixilDTMessage nMessage, int nInfo)
{
    int i;
    int nReturn = 0;		// Default return value
    int nWeekBegins;

    switch (nMessage) {
    case BEGIN_WEEK_CHANGED:	// Beginning day of week has been changed
	nWeekBegins = Options::GetWeekBegins();
	for (i = 0; i < 7; ++i) {
	    m_strDow[i] =::GetDayOfWeek((i + nWeekBegins) % 7);
	    m_pDow[i]->label(m_strDow[i].c_str());
	}
	DisplayDay(m_nDate);
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
SchedulerMonthly::OnGotoButton(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pDlg;
    SchedulerMonthly *pThis =
	reinterpret_cast < SchedulerMonthly * >(pWidget->parent()->parent());

    pDlg = new DatePickerDlg(pThis->m_nDate,
			     DatePicker::Monthly,
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
SchedulerMonthly::OnLeftButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerMonthly *pThis =
	reinterpret_cast < SchedulerMonthly * >(pWidget->parent()->parent());
    struct tm *pTm;

    // Only go left if not at beginning of the possible time range
    if (pThis->m_nDate > 31 * 24 * 60 * 60) {
	pTm = localtime(&pThis->m_nDate);
	pTm->tm_isdst = -1;
	--pTm->tm_mon;
	if (pTm->tm_mon < 0) {
	    pTm->tm_mon = 11;
	    --pTm->tm_year;
	}
	pThis->DisplayDay(mktime(pTm));
    }
}


//--------------------------------------------------------------//
// Process a click on the right button.                         //
//--------------------------------------------------------------//
void
SchedulerMonthly::OnRightButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerMonthly *pThis =
	reinterpret_cast < SchedulerMonthly * >(pWidget->parent()->parent());
    struct tm *pTm;

    // Only go right if not at end of the possible time range
    if (pThis->m_nDate <= LONG_MAX - (30 + 31) * 24 * 60 * 60) {
	pTm = localtime(&pThis->m_nDate);
	pTm->tm_isdst = -1;
	++pTm->tm_mon;
	if (pTm->tm_mon > 11) {
	    pTm->tm_mon = 0;
	    ++pTm->tm_year;
	}
	pThis->DisplayDay(mktime(pTm));
    }
}


//--------------------------------------------------------------//
// Process a click on the Today button.  The button is within a //
// group within this page hence the parent()->parent() to get   //
// to this object.                                              //
//--------------------------------------------------------------//
void
SchedulerMonthly::OnTodayButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerMonthly *pThis =
	reinterpret_cast < SchedulerMonthly * >(pWidget->parent()->parent());

    // Go display the current date
    pThis->DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Print this data.                                             //
//--------------------------------------------------------------//
void
SchedulerMonthly::Print()
{
    bool bOverflow;
    float fBoxHeight;
    float fBoxWidth;
    int nBorder = INCH / 2;	// 1/2 inch border
    int nCol;
    int nCopy;
    int nHeight;
    int nLastX;
    int nLastY;
    int nMonth;
    int nRow;
    int nX;
    int nY;
    multimap < int, int >mapEvent;
    multimap < int, int >::iterator iter;
    Printer printer;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    string strData;
    string strKey;
    string strTrailer;
    time_t nDate;
    time_t nNow = time(NULL);
    time_t nTime;
    time_t nToday;
    struct tm *pTm;
    vector < string > vDescription;
    vector < string > vTime;
    vector < string > vShortTime;
    vector < time_t > vDate;
    vector < vector < string > >vvDescription;
    vector < vector < string > >vvTime;

    // Open the printer
    if (printer.Open(_("SchedulerMonthly")) == true) {
	// Set an hourglass cursor
	PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_WAIT);

	// Print once for eqch requested copy
	for (nCopy = 0; nCopy < printer.GetCopies(); ++nCopy) {
	    // Reset the page number
	    printer.ResetPageNumber();

	    // Start the first page
	    printer.StartPage();

	    // Set up the title for the page
	    printer.SetBoldSerifFont(18);
	    nHeight =
		printer.GetHeight() - (3 * printer.GetFontSize() / 2) -
		nBorder;
	    printer.DrawText(0, nHeight, printer.GetWidth(),
			     3 * printer.GetFontSize() / 2,
			     FormatMonth(m_nDate).c_str(), FL_ALIGN_TOP);
	    nHeight -= printer.GetFontSize() / 2;

	    // Set the default font size
	    printer.SetSerifFont(8);

	    // Calculate the sizeof the calendar boxes
	    fBoxHeight =
		(float (nHeight - (3 * printer.GetFontSize()) / 2 - nBorder))
		/6.0;
	    fBoxWidth = (float (printer.GetWidth() - 2 * nBorder)) /7.0;

	    // Draw the day names
	    for (nCol = 0; nCol < 7; ++nCol) {
		printer.DrawText(int (nCol * fBoxWidth + nBorder),
				 nHeight,
				 int (fBoxWidth),
				 2 * printer.GetFontSize(),
				 GetDayOfWeek((nCol +
					       Options::GetWeekBegins()) %
					      7).c_str(), FL_ALIGN_TOP);
	    }

	    // Calculate the starting day for the calendar
	    nDate =::SubtractDays(m_nDate,
				  (7 +::GetDow(m_nDate) -
				   Options::GetWeekBegins()) % 7);

	    // Get the current month
	    pTm = localtime(&m_nDate);
	    nMonth = pTm->tm_mon;

	    // Get today's date
	    nToday =::NormalizeDate(time(NULL));

	    // Draw the calendar boxes
	    printer.CalendarStart(vDate, vvTime, vvDescription);
	    nLastY = nHeight;
	    for (nRow = 0; nRow < 6; ++nRow) {
		// Calculate this Y and set the next "last Y" value
		nY = nLastY;
		nLastY = nHeight - int (float (nRow + 1) * fBoxHeight);

		// Get the first "last X" value
		nLastX = nBorder;

		// Output each day in this row
		for (nCol = 0; nCol < 7; ++nCol) {
		    // Calculate this X and set the next "last X" value
		    nX = nLastX;
		    nLastX = int (float ((nCol + 1)) * fBoxWidth) + nBorder;

		    // Get the events for this day
		    pSchedulerDB->GetAllAppointments(nDate, mapEvent);

		    // Set up a map of times and meeting titles
		    vShortTime.clear();
		    vTime.clear();
		    vDescription.clear();

		    // Get the info on each meeting
		    for (iter = mapEvent.begin();
			 iter != mapEvent.end(); ++iter) {
			nTime = pSchedulerDB->GetStartTime(iter->second);
			pTm = localtime(&nTime);
			strKey =::FormatShortTime(nTime);
			strData =::FormatTime(nTime);
			nTime = pSchedulerDB->GetEndTime(iter->second);
			strKey += '-';
			strData += " - ";
			strKey +=::FormatShortTime(nTime);
			strData +=::FormatTime(nTime);
			vShortTime.push_back(strKey);
			vTime.push_back(strData);
			vDescription.push_back(pSchedulerDB->
					       GetDescription(iter->second));
		    }

		    // Now build this calendar box
		    pTm = localtime(&nDate);
		    printer.CalendarBox(nDate,
					pTm->tm_mday,
					(nMonth == pTm->tm_mon) ? 255 : 224,
					(nToday == nDate),
					nX,
					nLastY,
					nLastX - nX,
					nY - nLastY,
					vShortTime,
					vTime, vDescription, bOverflow);

		    // Special overflow processing
		    if (bOverflow == true) {
			// Save the date that overflowed
			vDate.push_back(nDate);

			// Add to the vectors of vectors
			vvTime.push_back(vTime);
			vvDescription.push_back(vDescription);
		    }
		    // Go to the next date
		    nDate =::AddDays(nDate, 1);
		}
	    }

	    // Add a page trailer
	    strTrailer =::FormatDate(nNow);
	    strTrailer += ' ';
	    strTrailer +=::FormatTime(nNow);
	    printer.PageTrailer(nBorder, nBorder);

	    // Finish any overflow processing for the calendar
	    printer.CalendarEnd(_("Scheduler Monthly"),
				184, vDate, vvTime, vvDescription);

	    // End the page
	    printer.EndPage();
	}

	// All done, close the printer
	printer.Close();

	// Reset the cursor
	PixilDT::GetApp()->GetMainWindow()->ResetCursor();
    }
}


//--------------------------------------------------------------//
// Resize the text in each browser if needed when resizeing     //
//--------------------------------------------------------------//
void
SchedulerMonthly::resize(int nX, int nY, int nWidth, int nHeight)
{
    int nOldWidth = w();

    // Then invoke the base class
    Fl_Group::resize(nX, nY, nWidth, nHeight);

    // Change the browsers only if needed
    if (nWidth != nOldWidth) {
	int i;
	int j;
	time_t nDate;
	struct tm *pTm;

	pTm = localtime(&m_nDate);
	nDate =::SubtractDays(m_nDate, pTm->tm_wday);

	// Refresh each browser
	for (i = 0; i < 6; ++i) {
	    for (j = 0; j < 7; ++j) {
		pTm = localtime(&nDate);
		BuildDate(nDate, i * 7 + j);
		nDate =::AddDays(nDate, 1);
	    }
	}
    }
}
