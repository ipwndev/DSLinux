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
// To Do List Show Dialog.                                      //
//--------------------------------------------------------------//
#ifndef TODOLISTSHOWDLG_H_

#define TODOLISTSHOWDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>
class ToDoListShowDlg:public Fl_Window
{
  public:ToDoListShowDlg(Fl_Widget * pParent);
    // Constructor
    ~ToDoListShowDlg();		// Destructor
    int DoModal();		// Run the modal dialog
  private:  Fl_Button * m_pCancelButton;
    // The Cancel button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Check_Button *m_pCategoryButton;	// Show categories check button
    Fl_Check_Button *m_pCompletedButton;	// Show Completed check button
    Fl_Check_Button *m_pDueButton;	// Show only due items check button
    Fl_Check_Button *m_pDueDateButton;	// Show due dates check button
    Fl_Check_Button *m_pPriorityButton;	// Show priorities check button
    Fl_Choice *m_pSortChoice;	// The sort choice widget
    Fl_Menu_Item *m_pSortMenu;	// The menu for the Sort choice widget
    void CreateOrderWidget(int nX,	// Create the sort order choice widget
			   int nY, int nWidth, int nHeight);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
};


#endif /*  */
