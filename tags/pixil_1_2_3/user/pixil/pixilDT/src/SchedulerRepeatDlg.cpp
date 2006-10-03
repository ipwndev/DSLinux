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
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Return_Button.H>
#include "DatePickerDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "Images.h"
#include "Options.h"
#include "PixilDT.h"
#include "SchedulerRepeatDlg.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT         200
#define DLG_WIDTH          350
#define PAGE_INDENT        100
#define RADIO_BUTTON_WIDTH 100


//--------------------------------------------------------------//
// Constructor, nRow is the row number in the Scheduler         //
// database as currently sorted or a -1 if this is to be a new  //
// row.                                                         //
//--------------------------------------------------------------//
SchedulerRepeatDlg::SchedulerRepeatDlg(const SchedulerRepeatData *
				       pSchedulerRepeatData,
				       Fl_Widget * pParent)
    :
Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Change Repeat"))
{
    int i;
    int nHeight;
    int nX;
    int nY;
    int nWidth;

    // Copy the Scheduler repetition settings
    m_pSchedulerRepeatData = new SchedulerRepeatData(*pSchedulerRepeatData);

    // Create the dialog widgets

    // Create the 5 radio buttons
    for (i = 0; i < 5; ++i) {
	m_strRadioLabel[i] =
	    SchedulerDB::GetRepeatTypeString(i == 0 ? 0 : (1 << (i - 1)));
	m_pRadioButton[i] =
	    new Fl_Check_Button(DLG_BORDER,
				DLG_BORDER + i * (DLG_BORDER +
						  DLG_INPUT_HEIGHT),
				DLG_RADIO_SIZE, DLG_INPUT_HEIGHT,
				m_strRadioLabel[i].c_str());
	m_pRadioButton[i]->type(FL_RADIO_BUTTON);
	m_pRadioButton[i]->down_box(FL_ROUND_DOWN_BOX);
	m_pRadioButton[i]->user_data((void *) i);
	m_pRadioButton[i]->
	    value((i == m_pSchedulerRepeatData->GetRepeatIndex())? 1 : 0);
	m_pRadioButton[i]->callback(OnRadioButton);
    }

    // Create each of the 5 groups corresponding to each of the 5 radio buttons
    nX = DLG_BORDER + RADIO_BUTTON_WIDTH;
    nY = DLG_BORDER;
    nWidth = w() - 2 * DLG_BORDER - RADIO_BUTTON_WIDTH;
    nHeight = h() - 4 * DLG_BORDER - DLG_BUTTON_HEIGHT;
    CreatePage1(nX, nY, nWidth, nHeight);
    CreatePage2(nX, nY, nWidth, nHeight);
    CreatePage3(nX, nY, nWidth, nHeight);
    CreatePage4(nX, nY, nWidth, nHeight);
    CreatePage5(nX, nY, nWidth, nHeight);

    // Show the correct page
    OnRadioButton(m_pRadioButton[m_pSchedulerRepeatData->GetRepeatIndex()],
		  (void *) m_pSchedulerRepeatData->GetRepeatIndex());

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

    // Get the initial values for the data

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
SchedulerRepeatDlg::~SchedulerRepeatDlg()
{
    FreeTranslatedMenu(m_pMonthMenu);
    delete m_pDayCalendarPixmap;
    delete m_pMonthCalendarPixmap;
    delete m_pWeekCalendarPixmap;
    delete m_pYearCalendarPixmap;
    delete m_pSchedulerRepeatData;
}


//--------------------------------------------------------------//
// Create an end date set of widgets.                           //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreateEndDate(int nIndex)
{
    Fl_Button *pButton;
    Fl_Input **ppInput;
    Fl_Pixmap **ppPixmap;

    // Set up for the page being created
    switch (nIndex) {
    case 1:			// Daily page
	ppInput = &m_pDayEnd;
	ppPixmap = &m_pDayCalendarPixmap;
	break;

    case 2:			// Weekly page
	ppInput = &m_pWeekEnd;
	ppPixmap = &m_pWeekCalendarPixmap;
	break;

    case 3:			// Monthly page
	ppInput = &m_pMonthEnd;
	ppPixmap = &m_pMonthCalendarPixmap;
	break;

    case 4:			// Yearly page
	ppInput = &m_pYearEnd;
	ppPixmap = &m_pYearCalendarPixmap;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Wrong page type
#endif
	;
    }

    *ppInput = new Fl_Input(m_pPage[nIndex]->x() + PAGE_INDENT,
			    m_pPage[nIndex]->y() + 2 * DLG_BORDER +
			    DLG_INPUT_HEIGHT,
			    m_pPage[nIndex]->w() - PAGE_INDENT -
			    IMAGE_BUTTON_WIDTH - 2 * DLG_BORDER,
			    DLG_INPUT_HEIGHT, _("End on:"));
    (*ppInput)->value(::FormatDate(m_pSchedulerRepeatData->GetEndDate()).
		      c_str());
    pButton =
	new Fl_Button(m_pPage[nIndex]->x() + m_pPage[nIndex]->w() -
		      DLG_BORDER - IMAGE_BUTTON_WIDTH,
		      m_pPage[nIndex]->y() + 2 * DLG_BORDER +
		      DLG_INPUT_HEIGHT, IMAGE_BUTTON_WIDTH,
		      IMAGE_BUTTON_HEIGHT);
    pButton->user_data((void *) nIndex);
    pButton->callback(OnDateButton);
    *ppPixmap = Images::GetCalendarIcon();
    (*ppPixmap)->label(pButton);
}


//--------------------------------------------------------------//
// Create the first "page".                                     //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreatePage1(int nX, int nY, int nWidth, int nHeight)
{
    Fl_Box *pBox;

    // Create the containing group
    m_pPage[0] = new Fl_Group(nX, nY, nWidth, nHeight);

    // Just some verbiage about selecting some repeat type
    pBox = new Fl_Box(nX + DLG_BORDER,
		      nY + DLG_BORDER,
		      nWidth - 2 * DLG_BORDER, nHeight - 2 * DLG_BORDER);
    pBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    pBox->
	label(_
	      ("Click one of the buttons to the left to select a repeat interval"));

    // Finish the group
    m_pPage[0]->end();
}


//--------------------------------------------------------------//
// Create the second "page".                                    //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreatePage2(int nX, int nY, int nWidth, int nHeight)
{
    // Create the containing group
    m_pPage[1] = new Fl_Group(nX, nY, nWidth, nHeight);

    // Create what looks like a spin control
    CreateSpinButton(1);

    // Create the end date widgets
    CreateEndDate(1);

    // Finish the group
    m_pPage[1]->end();
}


//--------------------------------------------------------------//
// Create the third "page".                                     //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreatePage3(int nX, int nY, int nWidth, int nHeight)
{
    Fl_Box *pBox;
    int i;
    int nButtonWidth;
    int nFontSize = (4 * labelsize()) / 5;
    int nTotalValue;
    int nValue;
    int nWeekBegin = Options::GetWeekBegins();

    // Create the containing group
    m_pPage[2] = new Fl_Group(nX, nY, nWidth, nHeight);

    // Create what looks like a spin control
    CreateSpinButton(2);

    // Create the end date widgets
    CreateEndDate(2);

    // Create the day of week label
    pBox = new Fl_Box(m_pPage[2]->x() + DLG_BORDER,
		      m_pPage[2]->y() + 3 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
		      PAGE_INDENT - 2 * DLG_BORDER,
		      DLG_INPUT_HEIGHT, _("Day(s) of Week:"));
    pBox->
	align(FL_ALIGN_RIGHT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);

    // Create the day of week widgets
    m_pWeekDays = new Fl_Group(nX + PAGE_INDENT,
			       nY + 3 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
			       nWidth - PAGE_INDENT - DLG_BORDER,
			       DLG_INPUT_HEIGHT);
    m_pWeekDays->box(FL_DOWN_BOX);
    m_pWeekDays->color(FL_WHITE);
    nButtonWidth = m_pWeekDays->w() / 7;
    nTotalValue = 0;
    for (i = 0; i < 7; ++i) {
	m_strWeekdayLabel[i] =
	    string(::GetDayOfWeek((i + nWeekBegin) % 7)).substr(0, 1);
	m_pWeekDayButton[i] =
	    new Fl_Button(m_pWeekDays->x() + i * nButtonWidth,
			  m_pWeekDays->y(), nButtonWidth, DLG_INPUT_HEIGHT,
			  m_strWeekdayLabel[i].c_str());
	m_pWeekDayButton[i]->box(FL_BORDER_BOX);
	m_pWeekDayButton[i]->labelsize(nFontSize);
	m_pWeekDayButton[i]->labelfont(FL_HELVETICA_BOLD);
	m_pWeekDayButton[i]->type(FL_TOGGLE_BUTTON);
	nValue =
	    (m_pSchedulerRepeatData->
	     IsWeekDaySelected((i + nWeekBegin) % 7) == true ? 1 : 0);
	m_pWeekDayButton[i]->value(nValue);
	nTotalValue += nValue;
	m_pWeekDayButton[i]->user_data((void *) ((i + nWeekBegin) % 7));
	OnWeekDay(m_pWeekDayButton[i], m_pWeekDayButton[i]->user_data());
	m_pWeekDayButton[i]->callback(OnWeekDay);
    }

    // Pick the current day of week if none were already selected
    if (nTotalValue == 0) {
	i =::GetDow(m_pSchedulerRepeatData->GetStartTime());
	i = ((i + 7 - nWeekBegin) % 7);
	m_pWeekDayButton[i]->value(1);
	OnWeekDay(m_pWeekDayButton[i], m_pWeekDayButton[i]->user_data());
    }
    // End the page
    m_pWeekDays->end();

    // Finish the window
    m_pPage[2]->end();
}


//--------------------------------------------------------------//
// Create the fourth "page".                                    //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreatePage4(int nX, int nY, int nWidth, int nHeight)
{
    static const Fl_Menu_Item menuChoice[] = {
	{N_("Date")},
	{N_("Day")},
	{NULL},
    };

    // Create the containing group
    m_pPage[3] = new Fl_Group(nX, nY, nWidth, nHeight);

    // Create what looks like a spin control
    CreateSpinButton(3);

    // Create the end date widgets
    CreateEndDate(3);

    // Create the "By" choice
    m_pMonthBy = new Fl_Choice(nX + PAGE_INDENT,
			       nY + 2 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
			       nWidth - PAGE_INDENT - DLG_BORDER,
			       DLG_BUTTON_HEIGHT, _("By:"));
    m_pMonthMenu = TranslateMenuItems(menuChoice);
    m_pMonthBy->menu(m_pMonthMenu);
    m_pMonthBy->value(m_pSchedulerRepeatData->GetRepeatMonthByDay()? 1 : 0);

    // Finish the group
    m_pPage[3]->end();
}


//--------------------------------------------------------------//
// Create the fifth "page".                                     //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreatePage5(int nX, int nY, int nWidth, int nHeight)
{
    // Create the containing group
    m_pPage[4] = new Fl_Group(nX, nY, nWidth, nHeight);

    // Create what looks like a spin control
    CreateSpinButton(4);

    // Create the end date widgets
    CreateEndDate(4);

    // Finish the group
    m_pPage[4]->end();
}


//--------------------------------------------------------------//
// Create a spin button                                         //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::CreateSpinButton(int nIndex)
{
    const char *pszSpinLabel;
    int nLabelHeight;
    int nLabelWidth;
    SpinInput **ppEvery;

    // Set up for the page being created
    switch (nIndex) {
    case 1:			// Daily page
	pszSpinLabel = _("Days");
	ppEvery = &m_pDayEvery;
	break;

    case 2:			// Weekly page
	pszSpinLabel = _("Weeks");
	ppEvery = &m_pWeekEvery;
	break;

    case 3:			// Monthly page
	pszSpinLabel = _("Months");
	ppEvery = &m_pMonthEvery;
	break;

    case 4:			// Yearly page
	pszSpinLabel = _("Years");
	ppEvery = &m_pYearEvery;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Wrong page type
#endif
	;
    }

    // Get the width of a "0000" constant in the current font
    fl_font(labelfont(), labelsize());
    fl_measure("0000", nLabelWidth, nLabelHeight);
    nLabelWidth += 2 * Fl::box_dw(FL_DOWN_BOX);

    // Create what looks like a spin control with some surrounding text
    *ppEvery = new SpinInput(m_pPage[nIndex]->x() + PAGE_INDENT,
			     m_pPage[nIndex]->y() + DLG_BORDER,
			     nLabelWidth + IMAGE_BUTTON_WIDTH,
			     DLG_INPUT_HEIGHT, _("Every:"), 3, 1, 999);
    new Fl_Box(m_pPage[nIndex]->x() + PAGE_INDENT + nLabelWidth +
	       IMAGE_BUTTON_WIDTH + DLG_BORDER,
	       m_pPage[nIndex]->y() + DLG_BORDER,
	       m_pPage[nIndex]->w() - PAGE_INDENT - nLabelWidth -
	       IMAGE_BUTTON_WIDTH - 2 * DLG_BORDER, DLG_INPUT_HEIGHT,
	       pszSpinLabel);

    // Set the initial value of the "Every" input widget
    if (nIndex == m_pSchedulerRepeatData->GetRepeatIndex()) {
	// Set the value from the repeat settings
	(*ppEvery)->value(m_pSchedulerRepeatData->GetRepeatEvery());
    } else {
	// Just default to 1 or every time unit
	(*ppEvery)->value(1);
    }
}


//--------------------------------------------------------------//
// Run the modal dialog                                         //
//--------------------------------------------------------------//
int
SchedulerRepeatDlg::DoModal()
{
    bool bGood = false;
    int nResult;
    int nReturn = 1;
    int nWeekBegin = Options::GetWeekBegins();
    time_t nDate;

    while (nReturn == 1 && bGood == false) {
	// Go run the dialog
	nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

	// Was the OK button pressed ?
	if (nReturn == 1) {
	    // Validate all fields
	    bGood = true;
	    if (m_pRadioButton[0]->value() == 1) {
		// No repetitions requested
		m_pSchedulerRepeatData->SetNoRepetition();
	    } else if (m_pRadioButton[1]->value() == 1) {
		// Daily repetitions requested

		// Validate the end date
		if ((nResult =::ValidateDate(m_pDayEnd->value(), nDate)) < 0) {
		    fl_alert(_
			     ("The Daily Repeat End Date is not a valid date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else if (nDate >= 24 * 60 * 60
			   && nDate < ::NormalizeDate(m_pSchedulerRepeatData->
						     GetStartTime())) {
		    fl_alert(_
			     ("The Daily Repeat End Date is prior to the event start date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else {
		    // Save the data
		    m_pSchedulerRepeatData->SetDailyRepetition(m_pDayEvery->
							       value(),
							       nDate);
		}
	    } else if (m_pRadioButton[2]->value() == 1) {
		// Weekly repetitions requested

		// Validate the end date
		if ((nResult =::ValidateDate(m_pWeekEnd->value(), nDate)) < 0) {
		    fl_alert(_
			     ("The Weekly Repeat End Date is not a valid date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else if (nDate >= 24 * 60 * 60
			   && nDate < ::NormalizeDate(m_pSchedulerRepeatData->
						     GetStartTime())) {
		    fl_alert(_
			     ("The Weekly Repeat End Date is prior to the event start date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else if (m_pWeekDayButton[0]->value() == 0
			   && m_pWeekDayButton[1]->value() == 0
			   && m_pWeekDayButton[2]->value() == 0
			   && m_pWeekDayButton[3]->value() == 0
			   && m_pWeekDayButton[4]->value() == 0
			   && m_pWeekDayButton[5]->value() == 0
			   && m_pWeekDayButton[6]->value() == 0) {
		    fl_alert(_("At least one day-of-week must be selected"));
		    bGood = false;
		} else {
		    // Save the data
		    m_pSchedulerRepeatData->SetWeeklyRepetition(m_pWeekEvery->
								value(),
								m_pWeekDayButton
								[(7 -
								  nWeekBegin)
								 %
								 7]->value(),
								m_pWeekDayButton
								[1 -
								 nWeekBegin]->
								value(),
								m_pWeekDayButton
								[2 -
								 nWeekBegin]->
								value(),
								m_pWeekDayButton
								[3 -
								 nWeekBegin]->
								value(),
								m_pWeekDayButton
								[4 -
								 nWeekBegin]->
								value(),
								m_pWeekDayButton
								[5 -
								 nWeekBegin]->
								value(),
								m_pWeekDayButton
								[6 -
								 nWeekBegin]->
								value(),
								nDate);
		}
	    } else if (m_pRadioButton[3]->value() == 1) {
		// Monthly repetitions requested

		// Validate the end date
		if ((nResult =::ValidateDate(m_pMonthEnd->value(), nDate)) <
		    0) {
		    fl_alert(_
			     ("The Monthly Repeat End Date is not a valid date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else if (nDate >= 24 * 60 * 60
			   && nDate < ::NormalizeDate(m_pSchedulerRepeatData->
						     GetStartTime())) {
		    fl_alert(_
			     ("The Monthly Repeat End Date is prior to the event start date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else {
		    // Save the data
		    m_pSchedulerRepeatData->
			SetMonthlyRepetition(m_pMonthEvery->value(),
					     m_pMonthBy->value(), nDate);
		}
	    } else		//if (m_pRadioButton[4]->value()==1)
	    {
		// Yearly repetitions requested

		// Validate the end date
		if ((nResult =::ValidateDate(m_pYearEnd->value(), nDate)) < 0) {
		    fl_alert(_
			     ("The Yearly Repeat End Date is not a valid date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else if (nDate >= 24 * 60 * 60
			   && nDate < ::NormalizeDate(m_pSchedulerRepeatData->
						     GetStartTime())) {
		    fl_alert(_
			     ("The Yearly Repeat End Date is prior to the event start date:\n\n%s"),::
			     GetDateError(nResult));
		    bGood = false;
		} else {
		    // Save the data
		    m_pSchedulerRepeatData->SetYearlyRepetition(m_pYearEvery->
								value(),
								nDate);
		}
	    }
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// One of the date/calendar buttons was clicked (static         //
// callback).                                                   //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::OnDateButton(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pDlg;
    Fl_Input *pEnd;
    int nButton = reinterpret_cast < int >(pUserData);
    int nResult;
    SchedulerRepeatDlg *pThis =
	reinterpret_cast <
	SchedulerRepeatDlg * >(pWidget->parent()->parent());
    time_t nDate;

    // Increase the "Every" setting if not at the maximum
    switch (nButton) {
    case 1:			// Daily button
	pEnd = pThis->m_pDayEnd;
	break;

    case 2:			// Weekly button
	pEnd = pThis->m_pWeekEnd;
	break;

    case 3:			// Monthly button
	pEnd = pThis->m_pMonthEnd;
	break;

    case 4:			// Yearly button
	pEnd = pThis->m_pYearEnd;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Incorrect user_data on button
#endif
	pEnd = pThis->m_pDayEnd;
    }

    // Get the current end date
    nResult =::ValidateDate(pEnd->value(), nDate);
    if (nResult != 0 || nDate == 0) {
	// Bad date, use today + the type of repetition
	struct tm *pTm;

	nDate =::NormalizeDate(time(NULL));
	switch (nButton) {
	case 1:		// Daily repetition
	    nDate = AddDays(nDate, 1);
	    pTm = localtime(&nDate);
	    break;

	case 2:		// Weekly repetition
	    nDate = AddDays(nDate, 7);
	    pTm = localtime(&nDate);
	    break;

	case 3:		// Monthly repetition
	    pTm = localtime(&nDate);
	    switch (pTm->tm_mon) {
	    case 0:		// January
		if (pTm->tm_mday > 28) {
		    // Is this a leap year
		    if ((pTm->tm_year % 4) == 0) {
			pTm->tm_mday = 29;
		    } else {
			pTm->tm_mday = 28;
		    }
		}
		++pTm->tm_mon;
		break;

	    case 1:		// February
	    case 3:		// April
	    case 5:		// June
	    case 6:		// July
	    case 8:		// September
	    case 10:		// November
		++pTm->tm_mon;
		break;

	    case 2:		// March
	    case 4:		// May
	    case 7:		// August
	    case 9:		// October
		if (pTm->tm_mday == 31) {
		    pTm->tm_mday = 30;
		}
		++pTm->tm_mon;
		break;

	    case 11:		// December
		++pTm->tm_year;
		pTm->tm_mon = 0;
	    }
	    break;

	case 4:		// Monthly repetition
	    pTm = localtime(&nDate);
	    if (pTm->tm_mon == 1 && pTm->tm_mday == 29) {
		pTm->tm_mday = 28;
	    }
	    ++pTm->tm_year;
	}

	// Get the trial end date
	nDate = mktime(pTm);
    }
    // Do the date picker dialog
    pDlg =
	new DatePickerDlg(nDate, DatePicker::Daily,
			  PixilDT::GetApp()->GetMainWindow());
    if (pDlg->DoModal() == 1) {
	// Save the date
	pEnd->value(::FormatDate(pDlg->GetDate()).c_str());
    }
    // Clean up
    delete pDlg;

    // This seems to be needed here
    pThis->redraw();
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_SCHEDULER_REPEAT_DLG);
}


//--------------------------------------------------------------//
// One of the radio buttons was clicked (static callback).      //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::OnRadioButton(Fl_Widget * pWidget, void *pUserData)
{
    int i;
    int nButton = reinterpret_cast < int >(pUserData);
    SchedulerRepeatDlg *pThis =
	reinterpret_cast < SchedulerRepeatDlg * >(pWidget->parent());

    // Hide the pages that don't go with this button
    for (i = 0; i < 5; ++i) {
	if (i != nButton) {
	    pThis->m_pPage[i]->hide();
	}
    }

    // Show the page that correspnds to this button
    pThis->m_pPage[nButton]->show();
}


//--------------------------------------------------------------//
// One of the day-of-week buttons on the weekly page was        //
// clicked (static callback).                                   //
//--------------------------------------------------------------//
void
SchedulerRepeatDlg::OnWeekDay(Fl_Widget * pWidget, void *pUserData)
{
    Fl_Color nColor;
    Fl_Color nLabelColor;

    // Simply toggle the color of the button
    if (((Fl_Button *) pWidget)->value() == 1) {
	nColor = FL_SELECTION_COLOR;
	nLabelColor = FL_WHITE;
    } else {
	nColor = FL_GRAY;
	nLabelColor = FL_BLACK;
    }
    ((Fl_Button *) pWidget)->color(nColor);
    ((Fl_Button *) pWidget)->selection_color(nColor);
    ((Fl_Button *) pWidget)->labelcolor(nLabelColor);
    pWidget->redraw();
}
