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
#ifndef SCHEDULERYEARLY_H_

#define SCHEDULERYEARLY_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <ctime>
#include <string>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pixmap.H>
#include "Messages.h"
using namespace std;
class SchedulerYearly:public Fl_Group
{
  public:SchedulerYearly(int nX,
		    // Constructor
		    int nY, int nWidth, int nHeight);
     ~SchedulerYearly();	// Destructor
    int Message(PixilDTMessage nMessage,	// Message from the parent widget
		int nInfo);
    void Print();		// Print this page
  private:  bool m_bEvent[366];
    // Event flags for each day of the year
    Fl_Box *m_pDate;		// Date banner at the top of the page
    Fl_Button *m_pGoToButton;	// Goto button
    Fl_Button *m_pLeftButton;	// Left arrow button
    Fl_Button *m_pRightButton;	// Right arrow button
    Fl_Button *m_pTodayButton;	// Today button
    Fl_Group *m_pMonth[12];	// A group for each month
    Fl_Pixmap *m_pLeftPixmap;	// Left arrow pixmap
    Fl_Pixmap *m_pRightPixmap;	// Right arrow pixmap
    string m_strDateLabel;	// Label for the banner at the top of the page
    string m_strMonthLabel[12];	// The month names
    time_t m_nDate;		// Date currently being displayed
    void CreateMonth(int nIndex);	// Create a month's boxes
    void DisplayDay(time_t nDate);	// Go display a particular day's year
    static void OnGotoButton(Fl_Widget * pWidget,	// Process a click on the goto buttton
			     void *pUserData);
    static void OnLeftButton(Fl_Widget * pWidget,	// Process a click on the left buttton
			     void *pUserData);
    static void OnRightButton(Fl_Widget * pWidget,	// Process a click on the right buttton
			      void *pUserData);
    static void OnTodayButton(Fl_Widget * pWidget,	// Process a click on the today buttton
			      void *pUserData);
    void UpdateMonth(int nIndex);	// Update the monthly group after a change in dates or events
    static bool ValidateYear(const char *pszString,	// Validate an entered year for "goto"
			     Fl_Widget * pWidget, void *pUserData);
  public:void Refresh()
    {
	DisplayDay(m_nDate);
    }
};


#endif /*  */
