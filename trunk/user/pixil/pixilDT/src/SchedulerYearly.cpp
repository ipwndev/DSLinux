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
// Scheduler Yearly tab page                                    //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <climits>
#include <cstdio>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "HelpID.h"
#include "Images.h"
#include "InputBox.h"
#include "Options.h"
#include "PixilDT.h"
#include "SchedulerDB.h"
#include "SchedulerYearly.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


using namespace std;


#define BUTTON_BORDER 2
#define DATE_BORDER   4


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SchedulerYearly::SchedulerYearly(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight, _("Yearly"))
{
    Fl_Group *pGroup;
    int i;
    int j;
    int nBoxHeight;
    int nBoxWidth;
    int nGroupHeight;
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

    // Create the yearly view
    pGroup = new Fl_Group(x() + DLG_BORDER,
			  y() + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
			  w() - 2 * DLG_BORDER,
			  h() - 3 * DLG_BORDER - DLG_INPUT_HEIGHT);
    pGroup->box(FL_DOWN_BOX);
    resizable(pGroup);
    nGroupX = pGroup->x();
    nGroupY = pGroup->y();
    nGroupWidth = pGroup->w();
    nGroupHeight = pGroup->h();
    nBoxWidth = (nGroupWidth - Fl::box_dw(FL_DOWN_BOX)) / 3;
    nBoxHeight = (nGroupHeight - Fl::box_dh(FL_DOWN_BOX)) / 4;
    for (i = 0; i < 4; ++i) {
	for (j = 0; j < 3; ++j) {
	    m_pMonth[i * 3 + j] =
		new Fl_Group(nGroupX + (Fl::box_dw(FL_DOWN_BOX) >> 1) +
			     j * nBoxWidth,
			     nGroupY + (Fl::box_dh(FL_DOWN_BOX) >> 1) +
			     i * nBoxHeight,
			     (j ==
			      2 ? nGroupWidth - Fl::box_dw(FL_DOWN_BOX) -
			      2 * nBoxWidth : nBoxWidth),
			     (i ==
			      3 ? nGroupHeight - Fl::box_dh(FL_DOWN_BOX) -
			      3 * nBoxHeight : nBoxHeight));
	    m_pMonth[i * 3 + j]->box(FL_BORDER_BOX);
	    m_pMonth[i * 3 + j]->color(FL_WHITE);
	    CreateMonth(i * 3 + j);
	    m_pMonth[i * 3 + j]->end();
	}
    }

    // Finish with this group
    pGroup->end();
    pGroup->resizable(pGroup);

    // Finish with this widget
    end();

    // Go display today's date
    DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SchedulerYearly::~SchedulerYearly()
{
    delete m_pLeftPixmap;
    delete m_pRightPixmap;
}


//--------------------------------------------------------------//
// Create the Fl_Boxes for a month.                             //
// Each monthly box is divided into 7 vertical pieces and 7     //
// horizontal pieces.  The top vertical piece is twice the      //
// height of any others and is not divided horizontally.  This  //
// is where the month name goes.  The others are where the day  //
// numbers go.  See the UpdateMonth method for how the days are //
// actually displayed.
//--------------------------------------------------------------//
void
SchedulerYearly::CreateMonth(int nIndex)
{
    Fl_Box *pBox;
    int i;
    int j;
    int nH = m_pMonth[nIndex]->h();
    int nHeight = nH / 8;
    int nW = m_pMonth[nIndex]->w();
    int nWidth = (nW - 2 * Fl::box_dw(FL_DOWN_BOX)) / 7;
    int nX = m_pMonth[nIndex]->x();
    int nY = m_pMonth[nIndex]->y();

    // Create the box for the month name
    pBox = new Fl_Box(nX + (Fl::box_dw(FL_DOWN_BOX) >> 1),
		      nY, nW - Fl::box_dw(FL_DOWN_BOX), nH - 6 * nHeight);
    m_strMonthLabel[nIndex] =::GetMonthName(nIndex);
    pBox->
	align(FL_ALIGN_TOP | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_CLIP);
    pBox->box(FL_FLAT_BOX);
    pBox->color(FL_WHITE);
    pBox->label(m_strMonthLabel[nIndex].c_str());

    // Now create each box for the dates
    for (i = 0; i < 6; ++i) {
	for (j = 0; j < 7; ++j) {
	    pBox =
		new Fl_Box(nX + (Fl::box_dw(FL_DOWN_BOX) >> 1) + j * nWidth +
			   DATE_BORDER, nY + nH - (7 - i) * nHeight,
			   (j ==
			    6 ? nW - 6 * nWidth -
			    2 * Fl::box_dw(FL_DOWN_BOX) : nWidth) -
			   DATE_BORDER, nHeight);
	    pBox->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	    pBox->box(FL_FLAT_BOX);
	    pBox->labelfont(FL_HELVETICA_BOLD);
	    pBox->labelsize((4 * labelsize()) / 5);
	}
    }
}


//--------------------------------------------------------------//
// Display the year for a particular day.                       //
//--------------------------------------------------------------//
void
SchedulerYearly::DisplayDay(time_t nDate)
{
    char szString[16];
    int i;
    time_t nToday = time(NULL);
    struct tm *pTm;

    // Get the start of the year
    pTm = localtime(&nDate);
    pTm->tm_isdst = -1;
    pTm->tm_sec = 0;
    pTm->tm_min = 0;
    pTm->tm_hour = 0;
    pTm->tm_mday = 1;
    pTm->tm_mon = 0;
    m_nDate = mktime(pTm);

    // Fix the banner title's label
    sprintf(szString, "%d", pTm->tm_year + 1900);
    m_strDateLabel = szString;
    m_pDate->label(m_strDateLabel.c_str());

    // Get a boolean flag for events for each day of the year
    SchedulerDB::GetSchedulerDB()->GetYearlyEvents(m_bEvent, m_nDate);

    // Refresh each month's group
    for (i = 0; i < 12; ++i) {
	UpdateMonth(i);
    }

    // Enable or disable the Today button based on what day is being shown
    pTm = localtime(&nToday);
    pTm->tm_isdst = -1;
    pTm->tm_sec = 0;
    pTm->tm_min = 0;
    pTm->tm_hour = 0;
    pTm->tm_mday = 1;
    pTm->tm_mon = 0;
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
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
SchedulerYearly::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case BEGIN_WEEK_CHANGED:	// Beginning day of week has changed
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
SchedulerYearly::OnGotoButton(Fl_Widget * pWidget, void *pUserData)
{
    char szYear[5];
    InputBox *pDlg;
    int nYear;
    SchedulerYearly *pThis =
	reinterpret_cast < SchedulerYearly * >(pWidget->parent()->parent());
    struct tm *pTm = localtime(&(pThis->m_nDate));

    sprintf(szYear, "%04d", pTm->tm_year + 1900);
    pDlg = new InputBox(_("Select Year"), PixilDT::GetApp()->GetMainWindow(), szYear, 4,	// Maximum size
			HELP_NO_TOPIC,	// Help ID - no topic
			ValidateYear,	// Validation function
			NULL,	// Widget for validation
			NULL,	// Extra validation info
			_("Please enter the year to be viewed:"));

    // Was a new year entered
    if (pDlg->GetEntry().length() > 0) {
	nYear = atoi(pDlg->GetEntry().c_str()) - 1900;
	pThis->DisplayDay(::MakeDate(nYear, 1, 1));
    }
    // Clean up
    delete pDlg;
}


//--------------------------------------------------------------//
// Process a click on the left button.                          //
//--------------------------------------------------------------//
void
SchedulerYearly::OnLeftButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerYearly *pThis =
	reinterpret_cast < SchedulerYearly * >(pWidget->parent()->parent());
    struct tm *pTm;

    // Only go left if not at beginning of the possible time range
    if (pThis->m_nDate > 366 * 24 * 60 * 60) {
	pTm = localtime(&pThis->m_nDate);
	pTm->tm_isdst = -1;
	--pTm->tm_year;
	pThis->DisplayDay(mktime(pTm));
    }
}


//--------------------------------------------------------------//
// Process a click on the right button.                         //
//--------------------------------------------------------------//
void
SchedulerYearly::OnRightButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerYearly *pThis =
	reinterpret_cast < SchedulerYearly * >(pWidget->parent()->parent());
    struct tm *pTm;

    // Only go right if not at end of the possible time range
    if (pThis->m_nDate <= LONG_MAX - (365 + 366) * 24 * 60 * 60) {
	pTm = localtime(&pThis->m_nDate);
	pTm->tm_isdst = -1;
	++pTm->tm_year;
	pThis->DisplayDay(mktime(pTm));
    }
}


//--------------------------------------------------------------//
// Process a click on the Today button.  The button is within a //
// group within this page hence the parent()->parent() to get   //
// to this object.                                              //
//--------------------------------------------------------------//
void
SchedulerYearly::OnTodayButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerYearly *pThis =
	reinterpret_cast < SchedulerYearly * >(pWidget->parent()->parent());

    // Go display the current date
    pThis->DisplayDay(time(NULL));
}


//--------------------------------------------------------------//
// Print this page - not implemented.                           //
//--------------------------------------------------------------//
void
SchedulerYearly::Print()
{
    fl_alert(_
	     ("The yearly version of the Scheduler data cannot be printed."));
}


//--------------------------------------------------------------//
// Update a month for the currently displayed date.             //
//--------------------------------------------------------------//
void
SchedulerYearly::UpdateMonth(int nIndex)
{
    static const char *pszDayNumber[32] = {
	"",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
	"11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
	"21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
	"31",
    };
    const char *pszLabel;
    Fl_Box *pBox;
    int i;
    int nColor;
    int nLabelColor;
    static const int nMDaysLeap[12] =
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static const int nMDaysNonLeap[12] =
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int nMonth;
    int nStartDay;
    int nToday;
    int nWDay;
    int nYear;
    const int *pnDaysPerMonth;
    time_t nDate;
    struct tm *pTm;

    // The month name does not change

    // Get the day of month for today
    nDate = time(NULL);
    pTm = localtime(&nDate);
    nToday = pTm->tm_mday;
    nMonth = pTm->tm_mon;
    nYear = pTm->tm_year;

    // Get the day of week of the start of this month
    pTm = localtime(&m_nDate);
    pTm->tm_isdst = -1;
    pTm->tm_mon = nIndex;
    nDate = mktime(pTm);
    pTm = localtime(&nDate);
    nWDay = pTm->tm_wday - Options::GetWeekBegins();
    if (nWDay < 0) {
	nWDay += 7;
    }
    nStartDay = -nWDay + 1;
    pnDaysPerMonth = ((pTm->tm_year % 4) == 0 ? nMDaysLeap : nMDaysNonLeap);

    // Reset the day of month for today if not this month
    if (nMonth != pTm->tm_mon || nYear != pTm->tm_year) {
	nToday = 100;		// Just put it out-of-range
    }
    // Set the labels and colors for each button
    for (i = 0; i < 42; ++i, ++nStartDay) {
	if (nStartDay <= 0 || nStartDay > pnDaysPerMonth[pTm->tm_mon]) {
	    pszLabel = pszDayNumber[0];
	    nLabelColor = FL_BLACK;
	    nColor = FL_WHITE;
	} else {
	    pszLabel = pszDayNumber[nStartDay];
	    nLabelColor = (nStartDay == nToday ? FL_RED : FL_BLACK);
	    nColor =
		(m_bEvent[pTm->tm_yday - 1 + nStartDay] ==
		 true ? FL_YELLOW : FL_WHITE);
	}

	// Depends on the order of creation of the widgets
	pBox = dynamic_cast < Fl_Box * >(m_pMonth[nIndex]->child(i + 1));
	pBox->label(pszLabel);
	pBox->labelcolor(nLabelColor);
	pBox->color(nColor);
    }
}


//--------------------------------------------------------------//
// Validate an entered year for the "goto" button.              //
//--------------------------------------------------------------//
bool
SchedulerYearly::ValidateYear(const char *pszString,
			      Fl_Widget * pWidget, void *pUserData)
{
    bool bReturn;
    int nYear;
    string strData = pszString;

    if ((bReturn = TestNumeric(pszString)) == true) {
	nYear = atoi(pszString);
	if (nYear < 1970 || nYear > 2036) {
	    bReturn = false;
	}
    }
    return (bReturn);
}
