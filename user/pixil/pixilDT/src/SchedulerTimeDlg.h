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
#ifndef SCHEDULERTIMEDLG_H_

#define SCHEDULERTIMEDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Window.H>
class SchedulerTimeDlg:public Fl_Window
{
  public:SchedulerTimeDlg(time_t nStartTime,
		     // Constructor
		     time_t nEndTime, Fl_Widget * pParent);
    ~SchedulerTimeDlg();	// Destructor
    int DoModal();		// Run the modal dialog
    inline time_t GetEndTime() const	// Get the end time
    {
	return (m_nEndTime);
    }
    inline time_t GetStartTime() const	// Get the start time
    {
	return (m_nStartTime);
    }
  private:  Fl_Button * m_pCancelButton;
    // The Cancel button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Choice *m_pEndHour;	// The end hour
    Fl_Choice *m_pEndMinute;	// The end minute
    Fl_Choice *m_pStartHour;	// The start hour
    Fl_Choice *m_pStartMinute;	// The start minute
    Fl_Menu_Item *m_pEndHourMenu;	// The end hour menu
    Fl_Menu_Item *m_pEndMinuteMenu;	// The end minute menu
    Fl_Menu_Item *m_pStartHourMenu;	// The start hour menu
    Fl_Menu_Item *m_pStartMinuteMenu;	// The start minute menu
    Fl_Output *m_pEndTime;	// The end time widget
    Fl_Output *m_pStartTime;	// The start time widget
    int m_nDay;			// The day portion of the time
    int m_nMonth;		// The month portion of the time
    int m_nYear;		// The year portion of the time
    time_t m_nEndTime;		// The current end time
    time_t m_nStartTime;	// The current start time
    static Fl_Menu_Item *GetHourMenu();	// Set up an hour menu
    static Fl_Menu_Item *GetMinuteMenu();	// Set up a minute menu
    static void OnEndTime(Fl_Widget * pWidget,	// Process click on either the end hour or end menu choices
			  void *pUserData);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
    static void OnStartTime(Fl_Widget * pWidget,	// Process click on either the start hour or start menu choices
			    void *pUserData);
    void SetEndTime();		// Set the end time from the hour and minute choices
    void SetStartTime();	// Set the start time from the hour and minute choices
};


#endif /*  */
