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
// Date Picker Dialog.                                          //
//--------------------------------------------------------------//
#ifndef DATEPICKERDLG_H_

#define DATEPICKERDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <ctime>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include "DatePicker.h"
class DatePickerDlg:public Fl_Window
{
  public:DatePickerDlg(time_t nTime,
		  // Constructor
		  DatePicker::Type nType, Fl_Widget * pParent);
    int DoModal();		// Run the modal dialog
    inline time_t GetDate() const	// Get the chosen time
    {
	return (m_nTime);
    }
  private:static const char *m_pszTitle[3];
    // Dialog titles
    DatePicker *m_pDatePicker;	// Date picker
    Fl_Button *m_pCancelButton;	// The Cancel button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Button *m_pTodayButton;	// The Today button
    time_t m_nTime;		// The chosen time
    static void TodayButtonCallback(Fl_Widget * pWidget,	// Callback for the Today button
				    void *pUserData);
};


#endif /*  */
