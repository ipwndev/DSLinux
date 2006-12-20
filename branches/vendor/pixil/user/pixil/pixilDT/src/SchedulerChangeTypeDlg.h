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
// A dialog to ask about how to apply changes to a scheduled    //
// event.                                                       //
//--------------------------------------------------------------//
#ifndef SCHEDULERCHANGETYPEDLG_H_

#define SCHEDULERCHANGETYPEDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>

#define SCHEDULER_CHANGE_TYPE_ALL            1
#define SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE 2
#define SCHEDULER_CHANGE_TYPE_CURRENT_ONLY   3
#define SCHEDULER_CHANGE_TYPE_NONE          -1
class SchedulerChangeTypeDlg:public Fl_Window
{
  public:SchedulerChangeTypeDlg(Fl_Widget * pParent,
			   // Constructor
			   bool bDelete);
    ~SchedulerChangeTypeDlg();	// Destructor
    int DoModal();		// Run the modal dialog
    inline int GetChangeType() const	// Get the requested change type
    {
	return (m_nChangeType);
    }
  private:  Fl_Button * m_pAllButton;
    // The "All events" button
    Fl_Button *m_pCancelButton;	// The Cancel button
    Fl_Button *m_pCurrentButton;	// The "Current event only" button
    Fl_Button *m_pFutureButton;	// The "Current and future events" button
    int m_nChangeType;		// The requested change type
    static void OnAllButton(Fl_Widget * pWidget,	// Callback for the All button
			    void *pUserData);
    static void OnCurrentButton(Fl_Widget * pWidget,	// Callback for the Current button
				void *pUserData);
    static void OnFutureButton(Fl_Widget * pWidget,	// Callback for the Current + Future button
			       void *pUserData);
};


#endif /*  */
