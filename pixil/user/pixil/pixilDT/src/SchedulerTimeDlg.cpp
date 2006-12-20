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
// Dialog for the times for a Scheduler appointment.            //
//--------------------------------------------------------------//
#include "config.h"
#include <ctime>
#include <FL/fl_ask.H>
#include <FL/Fl_Return_Button.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "PixilDT.h"
#include "SchedulerTimeDlg.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT       (6*DLG_BORDER+3*DLG_BUTTON_HEIGHT)
#define DLG_INDENT       (DLG_BUTTON_WIDTH+2*DLG_BORDER)
#define DLG_WIDTH        (100+DLG_INDENT+3*DLG_BORDER+2*DLG_BUTTON_WIDTH)


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
SchedulerTimeDlg::SchedulerTimeDlg(time_t nStartTime,
				   time_t nEndTime, Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Edit Appointment Times"))
{
    int nMinute;
    struct tm *pTm;

    // Create the dialog widgets
    m_pStartTime = new Fl_Output(DLG_INDENT,
				 DLG_BORDER,
				 DLG_WIDTH - 3 * DLG_BORDER -
				 2 * DLG_BUTTON_WIDTH - DLG_INDENT,
				 DLG_INPUT_HEIGHT, _("Start Time:"));
    m_pStartTime->color(FL_GRAY);
    m_pStartHour = new Fl_Choice(DLG_INDENT + m_pStartTime->w() + DLG_BORDER,
				 DLG_BORDER,
				 DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);
    m_pStartHourMenu = GetHourMenu();
    m_pStartHour->menu(m_pStartHourMenu);
    m_pStartHour->callback(OnStartTime);
    m_pStartMinute =
	new Fl_Choice(DLG_INDENT + m_pStartTime->w() + 2 * DLG_BORDER +
		      DLG_BUTTON_WIDTH, DLG_BORDER, DLG_BUTTON_WIDTH,
		      DLG_BUTTON_HEIGHT);
    m_pStartMinuteMenu = GetMinuteMenu();
    m_pStartMinute->menu(m_pStartMinuteMenu);
    m_pStartMinute->callback(OnStartTime);
    m_pEndTime = new Fl_Output(DLG_INDENT,
			       2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
			       DLG_WIDTH - 3 * DLG_BORDER -
			       2 * DLG_BUTTON_WIDTH - DLG_INDENT,
			       DLG_INPUT_HEIGHT, _("End Time:"));
    m_pEndTime->color(FL_GRAY);
    m_pEndHour = new Fl_Choice(DLG_INDENT + m_pStartTime->w() + DLG_BORDER,
			       2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
			       DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);
    m_pEndHourMenu = GetHourMenu();
    m_pEndHour->menu(m_pEndHourMenu);
    m_pEndHour->callback(OnEndTime);
    m_pEndMinute =
	new Fl_Choice(DLG_INDENT + m_pStartTime->w() + 2 * DLG_BORDER +
		      DLG_BUTTON_WIDTH, 2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);
    m_pEndMinuteMenu = GetMinuteMenu();
    m_pEndMinute->menu(m_pEndMinuteMenu);
    m_pEndMinute->callback(OnEndTime);

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
    if (nStartTime < 24 * 60 * 60) {
	// No time set yet, use the current hour
	nStartTime = time(NULL);
	pTm = localtime(&nStartTime);
	if (pTm->tm_hour >= 23) {
	    --pTm->tm_hour;
	}
	m_pStartHour->value(pTm->tm_hour);
	m_pEndHour->value(pTm->tm_hour + 1);
	m_pStartMinute->value(0);
	m_pEndMinute->value(0);
    } else {
	// Set the end time (round to the nearest 5 minutes)
	pTm = localtime(&nEndTime);
	nMinute = (pTm->tm_min + 60 * pTm->tm_hour + 2) / 5;
	m_pEndHour->value(nMinute / 12);
	m_pEndMinute->value(nMinute % 12);

	// Set the start time (round to the nearest 5 minutes)
	pTm = localtime(&nStartTime);
	nMinute = (pTm->tm_min + 60 * pTm->tm_hour + 2) / 5;
	m_pStartHour->value(nMinute / 12);
	m_pStartMinute->value(nMinute % 12);
    }
    m_nYear = pTm->tm_year;
    m_nMonth = pTm->tm_mon;
    m_nDay = pTm->tm_mday;
    SetStartTime();
    SetEndTime();

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
SchedulerTimeDlg::~SchedulerTimeDlg()
{
    FreeTranslatedMenu(m_pStartHourMenu);
    FreeTranslatedMenu(m_pStartMinuteMenu);
    FreeTranslatedMenu(m_pEndHourMenu);
    FreeTranslatedMenu(m_pEndMinuteMenu);
}


//--------------------------------------------------------------//
// Run the modal dialog                                         //
//--------------------------------------------------------------//
int
SchedulerTimeDlg::DoModal()
{
    bool bGood = false;
    int nReturn = 1;

    while (nReturn == 1 && bGood == false) {
	// Go run the dialog
	nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

	// Was the OK button pressed ?
	if (nReturn == 1) {
	    // Validate all fields
	    if (m_nEndTime <= m_nStartTime) {
		fl_alert(_("The End Time must be after the Start Time."));
	    } else {
		// Break out of the loop
		bGood = true;
	    }
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Get the menu for an hour choice.                             //
//--------------------------------------------------------------//
Fl_Menu_Item *
SchedulerTimeDlg::GetHourMenu()
{
    char szLabel[24];
    Fl_Menu_Item *pMenuItem = new Fl_Menu_Item[25];
    int i;
    int nHour;
    string strAM;
    string strPM;

    PixilDT::GetAMPM(strAM, strPM);
    memset(pMenuItem, 0, 25 * sizeof(Fl_Menu_Item));
    for (i = 0; i < 24; ++i) {
	// Set up the menu text with strdup so that FreeTranslatedMenu will work properly
	if (strAM.length() != 0) {
	    // Twelve hour clock
	    nHour = (i % 12);
	    if (nHour == 0) {
		nHour = 12;
	    }
	    sprintf(szLabel, "%2d %s", nHour,
		    i >= 12 ? strPM.c_str() : strAM.c_str());
	} else {
	    // 24 hour clock
	    sprintf(szLabel, "%d", i);
	}
	pMenuItem[i].text = strdup(szLabel);
    }
    return (pMenuItem);
}


//--------------------------------------------------------------//
// Get the menu for a minute choice.                            //
//--------------------------------------------------------------//
Fl_Menu_Item *
SchedulerTimeDlg::GetMinuteMenu()
{
    char szLabel[16];
    Fl_Menu_Item *pMenuItem = new Fl_Menu_Item[13];
    int i;

    memset(pMenuItem, 0, 13 * sizeof(Fl_Menu_Item));
    for (i = 0; i < 12; ++i) {
	// Set up the menu text with strdup so that FreeTranslatedMenu will work properly
	sprintf(szLabel, "%02d", i * 5);
	pMenuItem[i].text = strdup(szLabel);
    }
    return (pMenuItem);
}


//--------------------------------------------------------------//
// Process a change to either of the end hour or end minute     //
// choices.                                                     //
//--------------------------------------------------------------//
void
SchedulerTimeDlg::OnEndTime(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerTimeDlg *pThis =
	reinterpret_cast < SchedulerTimeDlg * >(pWidget->parent());

    pThis->SetEndTime();
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
SchedulerTimeDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_SCHEDULER_TIME_DLG);
}


//--------------------------------------------------------------//
// Process a change to either or the start hour or start minute //
// choices.                                                     //
//--------------------------------------------------------------//
void
SchedulerTimeDlg::OnStartTime(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerTimeDlg *pThis =
	reinterpret_cast < SchedulerTimeDlg * >(pWidget->parent());

    pThis->SetStartTime();
}


//--------------------------------------------------------------//
// Set the end time from the choices.                           //
//--------------------------------------------------------------//
void
SchedulerTimeDlg::SetEndTime()
{
    struct tm tmStruct;

    // Create the time_t value for this time
    memset(&tmStruct, 0, sizeof(tmStruct));
    tmStruct.tm_isdst = -1;
    tmStruct.tm_min = m_pEndMinute->value() * 5;
    tmStruct.tm_hour = m_pEndHour->value();
    tmStruct.tm_mday = m_nDay;
    tmStruct.tm_mon = m_nMonth;
    tmStruct.tm_year = m_nYear;

    // Save the current end time
    m_nEndTime = mktime(&tmStruct);

    // Update the widget value
    m_pEndTime->value(::FormatTime(m_nEndTime).c_str());
}


//--------------------------------------------------------------//
// Set the start time from the choices.                         //
//--------------------------------------------------------------//
void
SchedulerTimeDlg::SetStartTime()
{
    struct tm tmStruct;

    // Create the time_t value for this time
    memset(&tmStruct, 0, sizeof(tmStruct));
    tmStruct.tm_isdst = -1;
    tmStruct.tm_min = m_pStartMinute->value() * 5;
    tmStruct.tm_hour = m_pStartHour->value();
    tmStruct.tm_mday = m_nDay;
    tmStruct.tm_mon = m_nMonth;
    tmStruct.tm_year = m_nYear;

    // Save the current end time
    m_nStartTime = mktime(&tmStruct);

    // Update the widget value
    m_pStartTime->value(::FormatTime(m_nStartTime).c_str());
}
