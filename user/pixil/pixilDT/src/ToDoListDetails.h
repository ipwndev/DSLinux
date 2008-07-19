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
// Class for the ToDo List Details.                             //
//--------------------------------------------------------------//
#ifndef TODOLISTDETAILS_H_

#define TODOLISTDETAILS_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pixmap.H>
#include "Messages.h"
class ToDoListDetails:public Fl_Group
{
  public:ToDoListDetails(int nX,
		    // Constructor
		    int nY, int nWidth, int nHeight);
     ~ToDoListDetails();	// Destructor
    void ChangeComplete(int nRow,	// The completed flag has been changed in the list, change in the details
			int nComplete);
    void ChangeTime(int nRow,	// The due date/date completed has been changed in the list, change in the details
		    time_t nTime);
    void DisplayRow(int nRow,	// Display a ToDo item
		    int nRows);
    void Enable(bool bEnable);	// Enable or disable all input in the details
    int Message(PixilDTMessage nMessage,	// Process a message from the parent widget
		int nInfo);
    int SaveChanges(bool bAsk);	// Save any changes to disk
  private:char m_szLabel[5][2];
    // Labels for the priority radio buttons
    Fl_Button *m_pApplyButton;	// The Apply button
    Fl_Button *m_pDateButton;	// The date picker button
    Fl_Button *m_pNoteButton;	// The Note button
    Fl_Button *m_pPriority[5];	// Priority radio buttons
    Fl_Check_Button *m_pCompleteButton;	// The complete check box
    Fl_Choice *m_pCategoryChoice;	// The category choice
    Fl_Input *m_pDate;		// The date due/completed
    Fl_Multiline_Input *m_pTitle;	// The Title
    Fl_Menu_Item *m_pCategoryMenu;	// Category choice menu
    Fl_Output *m_pDayOfWeek;	// The day of week for the due date
    Fl_Output *m_pDesc;		// The note description
    Fl_Pixmap *m_pCalendarPixmap;	// The date picker button image
    Fl_Pixmap *m_pNotePixmap;	// The Notes/Description button image
    int m_nID;			// The ToDo item's ID being displayed
    time_t m_nDate;		// The current due/complete date
    static void DateButtonCallback(Fl_Widget * pWidget,	// Date button callback
				   void *pUserData);
    static void NoteButtonCallback(Fl_Widget * pWidget,	// Note/Description button callback
				   void *pUserData);
    static void OnApply(Fl_Widget * pWidget,	// Process an apply button click
			void *pUserData);
    bool ProcessDate();		// Process a newly entered date
};


#endif /*  */
