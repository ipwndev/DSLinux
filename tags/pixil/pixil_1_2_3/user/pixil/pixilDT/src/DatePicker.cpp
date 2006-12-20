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
// Calendar widget.                                             //
//--------------------------------------------------------------//
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include "DatePicker.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "Options.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#define DATE_MARGIN           6
#define DATE_BUTTON_WIDTH    24
#define DATE_LINE_HEIGHT     17
#define CALENDAR_WIDTH       (5*42)	// + 2 times border width
#define CALENDAR_HEIGHT      (11*DATE_LINE_HEIGHT+2*DATE_MARGIN)	// +4 times border height
#define MONTHLY_HEIGHT       (4*DATE_LINE_HEIGHT+DATE_MARGIN)


// Days per month for a leap year
const int
    DatePicker::m_nMDaysLeap[12] = {
	31,
	29,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
31 };

// Days per month for a on-leap year
const int
    DatePicker::m_nMDaysNonLeap[12] = {
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
31 };


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
DatePicker::DatePicker(int nX, int nY, Type nType, time_t nTime)
    :
Fl_Group(nX,
	 nY,
	 CALENDAR_WIDTH + 2 * Fl::box_dx(FL_DOWN_BOX),
	 CALENDAR_HEIGHT + 4 * Fl::box_dy(FL_DOWN_BOX))
{
    Fl_Box *pBox;
    Fl_Button *pButton;
    int i;
    int j;
    int nBoxHeight = Fl::box_dy(FL_DOWN_BOX);
    int nBoxWidth = Fl::box_dx(FL_DOWN_BOX);
    int nButtonWidth;
    int nFontSize = (4 * labelsize()) / 5;
    int nWeekBegin;
    tm *pTm = localtime(&nTime);

    // Set no notification callback
    m_pfnCallback = NULL;

    // Set the type of this widget
    m_nType = nType;

    // Set the current date
    m_nDate = nTime;
    m_nDay = pTm->tm_mday;
    m_nDisplayMonth = m_nMonth = pTm->tm_mon;
    m_nDisplayYear = m_nYear = pTm->tm_year;

    // Create the first part for the year
    pButton = new Fl_Button(x(), y(), DATE_BUTTON_WIDTH, DATE_LINE_HEIGHT);
    m_pLeftPixmap = Images::GetLeftIcon();
    m_pLeftPixmap->label(pButton);
    pButton->callback(CallbackPriorYear);
    pButton =
	new Fl_Button(x() + CALENDAR_WIDTH + 2 * nBoxWidth -
		      DATE_BUTTON_WIDTH, y(), DATE_BUTTON_WIDTH,
		      DATE_LINE_HEIGHT);
    m_pRightPixmap = Images::GetRightIcon();
    m_pRightPixmap->label(pButton);
    pButton->callback(CallbackNextYear);
    m_pBoxYear = new Fl_Box(x() + DLG_BORDER + DATE_BUTTON_WIDTH,
			    y(),
			    CALENDAR_WIDTH + 2 * nBoxWidth - 2 * DLG_BORDER -
			    2 * DATE_BUTTON_WIDTH, DATE_LINE_HEIGHT);
    m_pBoxYear->align(FL_ALIGN_CENTER);
    m_pBoxYear->box(FL_DOWN_BOX);
    m_pBoxYear->color(FL_WHITE);
    m_pBoxYear->labelsize(nFontSize);
    m_pBoxYear->labelfont(FL_HELVETICA_BOLD);

    // Create two lines of buttons for months
    m_pGroupMonths = new Fl_Group(x(),
				  y() + DATE_LINE_HEIGHT + DATE_MARGIN,
				  w(), 2 * DATE_LINE_HEIGHT + 2 * nBoxHeight);
    m_pGroupMonths->box(FL_DOWN_BOX);
    m_pGroupMonths->color(FL_WHITE);
    nButtonWidth = (CALENDAR_WIDTH) / 6;
    for (i = 0; i < 2; ++i) {
	for (j = 0; j < 6; ++j) {
	    m_strMonthAbbr[i * 6 + j] =::GetMonthAbbr(i * 6 + j);
	    m_pMonthButton[i][j] =
		new Fl_Button(m_pGroupMonths->x() + j * nButtonWidth +
			      nBoxWidth,
			      m_pGroupMonths->y() + i * DATE_LINE_HEIGHT +
			      nBoxHeight, nButtonWidth, DATE_LINE_HEIGHT,
			      m_strMonthAbbr[i * 6 + j].c_str());
	    m_pMonthButton[i][j]->box(FL_BORDER_BOX);
	    m_pMonthButton[i][j]->callback(CallbackMonth);
	    m_pMonthButton[i][j]->user_data((void *) (i * 6 + j));
	    m_pMonthButton[i][j]->labelsize(nFontSize);
	    m_pMonthButton[i][j]->labelfont(FL_HELVETICA_BOLD);
	}
    }
    m_pGroupMonths->end();

    // Set up the day of month buttons only if this is not a monthly date picker
    if (m_nType != Monthly) {
	// Set up the day of month buttons
	m_pGroupDays = new Fl_Group(x(),
				    y() + 3 * DATE_LINE_HEIGHT +
				    2 * DATE_MARGIN + 2 * nBoxHeight, w(),
				    7 * DATE_LINE_HEIGHT + 2 * nBoxHeight);
	m_pGroupDays->box(FL_DOWN_BOX);
	m_pGroupDays->color(FL_WHITE);

	// Output the days of week
	nButtonWidth = CALENDAR_WIDTH / 7;
	nWeekBegin = Options::GetWeekBegins();
	for (i = 0; i < 7; ++i) {
	    // Get the first character of the day name
	    m_szWeekDay[i][0] = (::GetDayOfWeek((i + nWeekBegin) % 7))[0];
	    m_szWeekDay[i][1] = '\0';
	    pBox =
		new Fl_Box(m_pGroupDays->x() + nBoxWidth + i * nButtonWidth,
			   m_pGroupDays->y() + nBoxHeight, nButtonWidth,
			   DATE_LINE_HEIGHT, m_szWeekDay[i]);
	    pBox->box(FL_FLAT_BOX);
	    pBox->color(FL_GRAY);
	    pBox->labelsize(nFontSize);
	    pBox->labelfont(FL_HELVETICA_BOLD);
	}

	// Now create the day of month buttons
	for (i = 0; i < 6; ++i) {
	    for (j = 0; j < 7; ++j) {
		// Create the button
		m_pDateButton[i][j] =
		    new Fl_Button(m_pGroupDays->x() + j * nButtonWidth +
				  nBoxWidth,
				  m_pGroupDays->y() + (i +
						       1) * DATE_LINE_HEIGHT +
				  nBoxHeight, nButtonWidth, DATE_LINE_HEIGHT,
				  "");
		m_pDateButton[i][j]->box(FL_FLAT_BOX);
		m_pDateButton[i][j]->labelsize(nFontSize);
		m_pDateButton[i][j]->labelfont(FL_HELVETICA_BOLD);
		m_pDateButton[i][j]->color(FL_WHITE);
		m_pDateButton[i][j]->callback(DateButtonCallback);
	    }
	}
	m_pGroupDays->end();
    }
    // Finish this widget
    end();

    // Set up for the first draw event
    PreDraw(true);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
DatePicker::~DatePicker()
{
    delete m_pLeftPixmap;
    delete m_pRightPixmap;
}


//--------------------------------------------------------------//
// Go to a particular month                                     //
//--------------------------------------------------------------//
void
DatePicker::CallbackMonth(Fl_Widget * pWidget, void *pUserData)
{
    reinterpret_cast <
	DatePicker *
	>(pWidget->parent()->parent())->SetMonth(reinterpret_cast <
						 int >(pUserData));
}


//--------------------------------------------------------------//
// Go to the next year                                         //
//--------------------------------------------------------------//
void
DatePicker::CallbackNextYear(Fl_Widget * pWidget, void *pUserData)
{
    reinterpret_cast < DatePicker * >(pWidget->parent())->IncrementYear();
}


//--------------------------------------------------------------//
// Go to the prior year                                         //
//--------------------------------------------------------------//
void
DatePicker::CallbackPriorYear(Fl_Widget * pWidget, void *pUserData)
{
    reinterpret_cast < DatePicker * >(pWidget->parent())->DecrementYear();
}


//--------------------------------------------------------------//
// Date button callback.                                        //
//--------------------------------------------------------------//
void
DatePicker::DateButtonCallback(Fl_Widget * pWidget, void *pUserData)
{
    DatePicker *pThis =
	reinterpret_cast < DatePicker * >(pWidget->parent()->parent());
    int nUserData = (int) pUserData;

    pThis->m_nYear = nUserData / 10000;
    pThis->m_nMonth = ((nUserData / 100) % 100);
    pThis->m_nDay = (nUserData % 100);
    pThis->PreDraw(false);
}


//--------------------------------------------------------------//
// Decrement the year.                                          //
//--------------------------------------------------------------//
void
DatePicker::DecrementYear()
{
    // Don't decrement below 1970
    if (m_nYear > 70) {
	--m_nYear;
	m_nDisplayYear = 68;	// To cause a display month reset
	FixDate(false);
    }
}


//--------------------------------------------------------------//
// Fix a date after it has been changed.  The bFallback         //
// variable controls how a day of month is corrected when the   //
// day of month is too high.  If true then the last day of the  //
// prior month is chosen instead.                               //
//--------------------------------------------------------------//
void
DatePicker::FixDate(bool bFallback)
{
    const int *pnMDays;

    // Test if the date is too low
    if (m_nYear < 70 || (m_nYear == 70 && m_nMonth < 0)
	|| (m_nYear == 70 && m_nMonth == 0 && m_nDay < 1)) {
	// Date too low
	m_nYear = 70;
	m_nMonth = 0;
	m_nDay = 1;
    } else if (m_nYear > 136 || (m_nYear == 136 && m_nMonth > 11)
	       || (m_nYear == 136 && m_nMonth == 11 && m_nDay > 31)) {
	// Date too high
	m_nYear = 136;
	m_nMonth = 11;
	m_nDay = 31;
    } else {
	// Fix the date if the month or day is out-of-range
	while (m_nMonth < 0) {
	    m_nMonth += 12;
	    --m_nYear;
	}

	while (m_nMonth > 11) {
	    m_nMonth -= 12;
	    ++m_nYear;
	}

	// Correct for days out-of-range
	pnMDays = (m_nYear & 0x03) != 0 ? m_nMDaysNonLeap : m_nMDaysLeap;
	while (m_nDay < 1) {
	    --m_nMonth;
	    if (m_nMonth < 0) {
		m_nMonth += 12;
		--m_nYear;
		pnMDays =
		    (m_nYear & 0x03) != 0 ? m_nMDaysNonLeap : m_nMDaysLeap;
	    }
	    m_nDay += pnMDays[m_nMonth];
	}

	if (bFallback == true) {
	    // Fall back to the prior month
	    if (m_nDay > pnMDays[m_nMonth]) {
		m_nDay = pnMDays[m_nMonth];
	    }
	} else {
	    // Don't fall back
	    while (m_nDay > pnMDays[m_nMonth]) {
		m_nDay -= pnMDays[m_nMonth];
		++m_nMonth;
		if (m_nMonth > 11) {
		    m_nMonth -= 12;
		    ++m_nYear;
		    pnMDays =
			(m_nYear & 0x03) !=
			0 ? m_nMDaysNonLeap : m_nMDaysLeap;
		}
	    }
	}
    }

    PreDraw(false);
}


//--------------------------------------------------------------//
// Get the height of the calendar widget                        //
//--------------------------------------------------------------//
int
DatePicker::GetHeight(Type nType)
{
    return (nType == Monthly ? MONTHLY_HEIGHT + 2 * Fl::box_dy(FL_DOWN_BOX)
	    : CALENDAR_HEIGHT + 4 * Fl::box_dy(FL_DOWN_BOX));
}


//--------------------------------------------------------------//
// Get the width of the calendar widget                         //
//--------------------------------------------------------------//
int
DatePicker::GetWidth()
{
    return (CALENDAR_WIDTH + 2 * Fl::box_dx(FL_DOWN_BOX));
}


//--------------------------------------------------------------//
// Increment the year.                                          //
//--------------------------------------------------------------//
void
DatePicker::IncrementYear()
{
    // Don't increment above 2036
    if (m_nYear < 136) {
	++m_nYear;
	m_nDisplayYear = 68;	// To cause a display month reset
	FixDate(false);
    }
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
DatePicker::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case BEGIN_WEEK_CHANGED:	// Beginning day of week has changed
	{
	    int i;
	    int nWeekBegin;

	    // Set up the day of month buttons only if this is not a monthly date picker
	    if (m_nType != Monthly) {
		// Output the days of week
		nWeekBegin = Options::GetWeekBegins();
		for (i = 0; i < 7; ++i) {
		    // Get the first character of the day name
		    m_szWeekDay[i][0] =
			(::GetDayOfWeek((i + nWeekBegin) % 7))[0];
		    m_szWeekDay[i][1] = '\0';
		}

		// Go redraw the calendar
		PreDraw(true);
	    }
	}
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
// Set up for the next draw event.                              //
//--------------------------------------------------------------//
void
DatePicker::PreDraw(bool bProgramSet)
{
    bool bReset;
    Fl_Color nColor;
    Fl_Color nGray = GetFLTKColor(0xa0, 0xa0, 0xa0);
    Fl_Color nLabelColor;
    int i;
    int j;
    int nCumDisplayMonth;
    int nCumMonth;
    int nDay;
    int nDays;
    int nPriorDays;
    int nUserData;
    int nWDay;
    int nWeekBegin = Options::GetWeekBegins();
    const int *pnMDays;
    time_t timeCurrent = NormalizeDate(time(NULL));
    time_t timeDisplay;
    time_t timeValue;
    tm *pTm;

    // Fix the date for weekly or monthly options
    if (m_nType == Weekly) {
	time_t nDate;
	struct tm *pTm;

	nDate =::MakeDate(m_nYear, m_nMonth, m_nDay);
	nDays =::GetDow(nDate) - nWeekBegin;
	if (nDays < 0) {
	    nDays += 7;
	}
	nDate =::SubtractDays(nDate, nDays);
	pTm = localtime(&nDate);
	m_nDay = pTm->tm_mday;
	m_nMonth = pTm->tm_mon;
	m_nYear = pTm->tm_year;
    } else if (m_nType == Monthly) {
	m_nDay = 1;
    }
    // Get the selected date
    m_nDate = MakeDate(m_nYear, m_nMonth, m_nDay);

    // Get the day of week of the first day of the display month
    if (m_nDisplayYear >= 70) {
	timeDisplay = MakeDate(m_nDisplayYear, m_nMonth, 1);
    } else {
	// Fake a very low date but allow for a timeozne offset
	timeDisplay = 24 * 60 * 60 - 1;
    }
    pTm = localtime(&timeDisplay);

    // See if the display month still works for the selected date
    nCumDisplayMonth = 12 * m_nDisplayYear + m_nDisplayMonth;
    nCumMonth = 12 * m_nYear + m_nMonth;
    bReset = false;
    if (nCumDisplayMonth == nCumMonth + 1) {
	if ((m_nYear % 4) == 0) {
	    nDay = m_nMDaysLeap[m_nMonth] - pTm->tm_wday;
	} else {
	    nDay = m_nMDaysNonLeap[m_nMonth] - pTm->tm_wday;
	}
	if (nDay > m_nDay) {
	    bReset = true;
	}
    } else if (nCumDisplayMonth == nCumMonth - 1) {
	if ((m_nDisplayYear % 4) == 0) {
	    nDay = m_nMDaysLeap[m_nDisplayMonth] + pTm->tm_wday;
	} else {
	    nDay = m_nMDaysNonLeap[m_nDisplayMonth] + pTm->tm_wday;
	}
	if (m_nDay > (6 * 7) - nDay + 1) {
	    bReset = true;
	}
    } else if (nCumDisplayMonth != nCumMonth) {
	bReset = true;
    }
    if (bReset == true) {
	m_nDisplayMonth = m_nMonth;
	m_nDisplayYear = m_nYear;
    }
    // Set up the year banner
    sprintf(m_szYear, "%d", m_nDisplayYear + 1900);
    m_pBoxYear->label(m_szYear);

    // Set the colors of the month buttons
    for (i = 0; i < 12; ++i) {
	if (i == m_nDisplayMonth) {
	    nColor = FL_SELECTION_COLOR;
	    nLabelColor = FL_WHITE;
	} else {
	    nColor = FL_WHITE;
	    nLabelColor = FL_BLACK;
	}
	m_pMonthButton[i / 6][i % 6]->color(nColor);
	m_pMonthButton[i / 6][i % 6]->labelcolor(nLabelColor);
    }

    // Set up the date buttons only if this is not a monthly date picker
    if (m_nType != Monthly) {
	// Get the day of week of the current month
	timeValue = MakeDate(m_nDisplayYear, m_nDisplayMonth, 1);
	pTm = localtime(&timeValue);

	// Correct for the day of week of the start of this month
	nWDay = pTm->tm_wday - nWeekBegin;
	if (nWDay < 0) {
	    nWDay += 7;
	}
	timeValue = SubtractDays(timeValue, nWDay);

	// Get the number of days in the prior month
	pnMDays = (m_nYear & 0x03) != 0 ? m_nMDaysNonLeap : m_nMDaysLeap;
	if (m_nMonth == 0) {
	    // For January, this must have been December
	    nPriorDays = 31;
	} else {
	    nPriorDays = pnMDays[m_nMonth - 1];
	}

	// Set up each button
	for (i = 0; i < 6; ++i) {
	    for (j = 0; j < 7; ++j) {
		nDay = i * 7 + j - nWDay + 1;
		nColor = FL_WHITE;
		if (nDay <= 0) {
		    nDay += nPriorDays;
		    nLabelColor = nGray;
		    if (nDay == m_nDay
			&& (m_nMonth + 1 == m_nDisplayMonth
			    || (m_nMonth == 11 && m_nDisplayMonth == 0))) {
			nColor = FL_SELECTION_COLOR;
			nLabelColor = FL_WHITE;
		    }
		    nUserData =
			10000 * (m_nDisplayYear -
				 (m_nDisplayMonth == 0 ? 1 : 0))
			+ 100 * (m_nDisplayMonth ==
				 0 ? 12 : (m_nDisplayMonth - 1))
			+ nDay;
		} else if (nDay > pnMDays[m_nDisplayMonth]) {
		    nDay -= pnMDays[m_nDisplayMonth];
		    nLabelColor = nGray;
		    if (nDay == m_nDay
			&& (m_nMonth - 1 == m_nDisplayMonth
			    || (m_nMonth == 0 && m_nDisplayMonth == 11))) {
			nColor = FL_SELECTION_COLOR;
			nLabelColor = FL_WHITE;
		    }
		    nUserData =
			10000 * (m_nDisplayYear +
				 (m_nDisplayMonth == 11 ? 1 : 0))
			+ 100 * (m_nDisplayMonth ==
				 11 ? 1 : (m_nDisplayMonth + 1))
			+ nDay;
		} else {
		    nUserData =
			10000 * m_nDisplayYear + 100 * m_nDisplayMonth + nDay;
		    if (timeValue == timeCurrent) {
			nLabelColor = FL_RED;
		    } else {
			nLabelColor = FL_BLACK;
		    }
		    if (nDay == m_nDay && m_nMonth == m_nDisplayMonth) {
			nColor = FL_SELECTION_COLOR;
			nLabelColor = FL_WHITE;
		    }
		}

		// Set the button selection if this is a weekly date picker
		if (m_nType == Weekly) {
		    int nDiff;

		    nDiff =::DaysBetween(m_nYear, m_nMonth, m_nDay,
					 timeValue);
		    if (nDiff >= -6 && nDiff <= 0) {
			nColor = FL_SELECTION_COLOR;
			nLabelColor = FL_WHITE;
		    }
		}

		sprintf(m_szDay[i][j], "%d", nDay);
		m_pDateButton[i][j]->label(m_szDay[i][j]);
		m_pDateButton[i][j]->color(nColor);
		m_pDateButton[i][j]->labelcolor(nLabelColor);
		m_pDateButton[i][j]->user_data((void *) nUserData);
		timeValue =::AddDays(timeValue, 1);
	    }
	}
    }
    // If not programmatically set, notify (someone) about the change in date if needed
    if (bProgramSet == false && m_pfnCallback != NULL) {
	(*m_pfnCallback) (this, m_nDate);
    }
    // Redraw the entire widget
    redraw();
}


//--------------------------------------------------------------//
// Set to a particular month.                                   //
//--------------------------------------------------------------//
void
DatePicker::SetMonth(int nMonth)
{
    m_nMonth = nMonth;
    m_nDisplayYear = 68;	// To cause a display month reset
    FixDate(true);
}


//--------------------------------------------------------------//
// Set to a particular date.                                    //
//--------------------------------------------------------------//
void
DatePicker::SetDate(time_t nDate)
{
    struct tm *pTm;

    // Fix the date
    m_nDate = NormalizeDate(nDate);
    pTm = localtime(&m_nDate);
    if (m_nYear != pTm->tm_year
	|| m_nMonth != pTm->tm_mon || m_nDay != pTm->tm_mday) {
	m_nYear = m_nDisplayYear = pTm->tm_year;
	m_nMonth = m_nDisplayMonth = pTm->tm_mon;
	m_nDay = pTm->tm_mday;

	// Go redraw the control
	PreDraw(true);
    }
}
