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
// Dialog for setting Scheduler repeat options.                 //
//--------------------------------------------------------------//
#ifndef SCHEDULERREPEATDLG_H_

#define SCHEDULERREPEATDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>
#include "SchedulerDB.h"
#include "SpinInput.h"
class SchedulerRepeatDlg:public Fl_Window
{
  public:SchedulerRepeatDlg(const SchedulerRepeatData *
		       pSchedulerRepeatData,
		       // Constructor
		       Fl_Widget * pParent);
     ~SchedulerRepeatDlg();	// Destructor
    int DoModal();		// Run the modal dialog
    const SchedulerRepeatData & GetSchedulerRepeatData() const	// Get the repetition settings
    {
	return (*m_pSchedulerRepeatData);
    }
  private:  Fl_Button * m_pCancelButton;
    // The Cancel button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Button *m_pWeekDayButton[7];	// The days of the week for the weekly page
    Fl_Check_Button *m_pRadioButton[5];	// The five radio buttons for the various repeat choices
    Fl_Choice *m_pMonthBy;	// Choice for how month repetitions are handled
    Fl_Group *m_pPage[5];	// Each of the pages
    Fl_Group *m_pWeekDays;	// Group containing week day buttons for the weekly page
    Fl_Input *m_pDayEnd;	// The end for daily repeats
    Fl_Input *m_pMonthEnd;	// The end for monthly repeats
    Fl_Input *m_pWeekEnd;	// The end for weekly repeats
    Fl_Input *m_pYearEnd;	// The end for yearly repeats
    Fl_Menu_Item *m_pMonthMenu;	// The menu for the choice on the monthly page
    Fl_Pixmap *m_pDayCalendarPixmap;	// The pixmap for the daily end date calendar button
    Fl_Pixmap *m_pMonthCalendarPixmap;	// The pixmap for the monthly end date calendar button
    Fl_Pixmap *m_pWeekCalendarPixmap;	// The pixmap for the weekly end date calendar button
    Fl_Pixmap *m_pYearCalendarPixmap;	// The pixmap for the yearly end date calendar button
    SchedulerRepeatData *m_pSchedulerRepeatData;	// The current repeat settings
    SpinInput *m_pDayEvery;	// The number of days between repeats
    SpinInput *m_pMonthEvery;	// The number of months between repeats
    SpinInput *m_pWeekEvery;	// The number of weeks between repeats
    SpinInput *m_pYearEvery;	// The number of years between repeats
    string m_strRadioLabel[5];	// Labels for the radio buttons
    string m_strWeekdayLabel[7];	// Labels for the days-of-week for theweekly page
    void CreateEndDate(int nIndex);	// Create end date widgets
    void CreatePage1(int nX,	// Create the "page" for the first (None) radio button
		     int nY, int nWidth, int nHeight);
    void CreatePage2(int nX,	// Create the "page" for the seocnd (Daily) radio button
		     int nY, int nWidth, int nHeight);
    void CreatePage3(int nX,	// Create the "page" for the third (Weekly) radio button
		     int nY, int nWidth, int nHeight);
    void CreatePage4(int nX,	// Create the "page" for the fourth (Monthly) radio button
		     int nY, int nWidth, int nHeight);
    void CreatePage5(int nX,	// Create the "page" for the fifth (Yearly) radio button
		     int nY, int nWidth, int nHeight);
    void CreateSpinButton(int nIndex);	// Create a spin button set of widgets
    static void OnDateButton(Fl_Widget * pWidget,	// Process click on one of the Date buttons
			     void *pUserData);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
    static void OnRadioButton(Fl_Widget * pWidget,	// Process click on one of the radio buttons
			      void *pUserData);
    static void OnWeekDay(Fl_Widget * pWidget,	// Process click on one of the day-of-week buttons
			  void *pUserData);
};


#endif /*  */
