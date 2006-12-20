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
#ifndef DATEPICKER_H_

#define DATEPICKER_H_

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
typedef void (*DATEPICKER_CALLBACK) (Fl_Widget * pWidget, time_t nDate);
class DatePicker:public Fl_Group
{
  public:enum Type		// Type of date picker
    { Daily = 0, Weekly, Monthly,
    };
      DatePicker(int x,		// Constructor
		 int y, Type nType, time_t nTime);
     ~DatePicker();		// Destructor
    inline time_t GetDate() const	// Get the selected date
    {
	return (m_nDate);
    }
    static int GetHeight(Type nType);	// Get the height of the calendar widget
    static int GetWidth();	// Get the width of the calendar widget
    int Message(PixilDTMessage nMessage,	// Messages from a parent widget
		int nInfo);
    void SetDate(time_t nDate);	// Set the date for the widget
    inline void SetNotify(DATEPICKER_CALLBACK pfnCallback)	// Notification routine for when a date is selected
    {
	m_pfnCallback = pfnCallback;
    }
  private:char m_szDay[6][7][3];
    // Day numbers for the calendar
    char m_szYear[5];		// The current year number
    char m_szWeekDay[7][2];	// First characters of the days of the week (translated)
    DATEPICKER_CALLBACK m_pfnCallback;	// Notification routine for when a date is selected
    Fl_Box *m_pBoxYear;		// The year banner at the top of the calendar
    Fl_Button *m_pDateButton[6][7];	// Date buttons
    Fl_Button *m_pMonthButton[2][6];	// Month buttons
    Fl_Group *m_pGroupDays;	// Group containing the days of the month
    Fl_Group *m_pGroupMonths;	// Group containing the month names
    Fl_Pixmap *m_pLeftPixmap;	// The left arrow pixmap
    Fl_Pixmap *m_pRightPixmap;	// The right arrow pixmap
    int m_nDay;			// Current day of month
    int m_nDisplayMonth;	// Month being displayed
    int m_nDisplayYear;		// Year being displayed
    static const int m_nMDaysLeap[12];	// Number of days in each month
    static const int m_nMDaysNonLeap[12];	// Number of days in each month
    int m_nMonth;		// Month of the current year (0-based)
    int m_nYear;		// Year (2000 = 100)
    string m_strMonthAbbr[12];	// Abbreviated month names (translated)
    time_t m_nDate;		// The selected date
    Type m_nType;		// Type of date picker
    static void CallbackMonth(Fl_Widget * pWidget,	// Month button callback
			      void *pUserData);
    static void CallbackNextYear(Fl_Widget * pWidget,	// Next year button
				 void *pUserData);
    void static CallbackPriorYear(Fl_Widget * pWidget,	// Prior year button
				  void *pUserData);
    static void DateButtonCallback(Fl_Widget * pWidget,	// Date button callback
				   void *pUserData);
    void DecrementYear();	// Decrement the year
    void IncrementYear();	// Increment the year
    void FixDate(bool bFallback);	// Fix the date after a change
    void PreDraw(bool bProgramset);	// Processing prior to a redraw of the widget
    void SetMonth(int nMonth);	// Set the calendar to a particular month
};


#endif /*  */
