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
// Dialog for adding/editing Scheduler appointments.            //
//--------------------------------------------------------------//
#ifndef SCHEDULERDLG_H_

#define SCHEDULERDLG_H_

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
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Window.H>
#include "SpinInput.h"
class SchedulerDlg:public Fl_Window
{
  public:SchedulerDlg(int nRow,
		 // Constructor
		 time_t nDate, Fl_Widget * pParent);
     ~SchedulerDlg();		// Destructor
    int DoModal();		// Run the modal dialog
  private:  Fl_Button * m_pCancelButton;
    // The Cancel button
    Fl_Button *m_pDateButton;	// The Date button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Button *m_pRepeatButton;	// The Repeat button
    Fl_Button *m_pTimeButton;	// The Time button
    Fl_Check_Button *m_pAlarmButton;	// The alarm button
    Fl_Choice *m_pTimeUnit;	// Minutes/Hours/Days prior for the alarm
    Fl_Input *m_pDate;		// The date
    Fl_Input *m_pEndTime;	// The end time
    Fl_Input *m_pStartTime;	// The start time
    Fl_Menu_Item *m_pTimeUnitMenu;	// The time interval units menu
    Fl_Multiline_Input *m_pDescription;	// The Description
    Fl_Output *m_pDayOfWeek;	// The day of week
    Fl_Pixmap *m_pDatePixmap;	// The Date button icon
    Fl_Pixmap *m_pTimePixmap;	// The Time button icon
    int m_nEndTime;		// Ending time
    int m_nRow;			// The row being updated or -1 if a new row
    int m_nStartTime;		// Starting time
    SchedulerRepeatData *m_pSchedulerRepeatData;	// Repetition settings for an event
    SpinInput *m_pInterval;	// The number of minutes prior for the alarm
    string m_strRepeatLabel;	// The label on the repeat button
    time_t m_nDate;		// The date from the dialog
    static void OnAlarm(Fl_Widget * pWidget,	// Process click on the Alarm button
			void *pUserData);
    static void OnDateButton(Fl_Widget * pWidget,	// Process click on the Date/Calendar button
			     void *pUserData);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
    static void OnRepeatButton(Fl_Widget * pWidget,	// Process click on the Repeat button
			       void *pUserData);
    static void OnTimeButton(Fl_Widget * pWidget,	// Process click on the Time button
			     void *pUserData);
    void SetFields(int nRow);	// Move data into the SchedulerDB row
    void SetRepeatButton();	// Set the label of the repeat button
};


#endif /*  */
