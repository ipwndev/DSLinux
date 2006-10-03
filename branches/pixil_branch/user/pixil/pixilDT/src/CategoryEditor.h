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
// Category Editor dialog.                                      //
//--------------------------------------------------------------//
#ifndef CATEGORYEDITOR_H_

#define CATEGORYEDITOR_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include "CategoryDB.h"
#include "CategoryEditorList.h"
#include "HelpID.h"
#include "PixilMainWnd.h"
class CategoryEditor:public Fl_Window
{
  public:enum Type
    { AddressBook, Notes, Scheduler, ToDoList,
    };
    enum ButtonStatus
    { Disable = 0, Enable = 1,
    };
      CategoryEditor(Type nType,	// Constructor
		     PixilMainWnd * pParent);
     ~CategoryEditor();		// Destructor
    void DeleteCategory(int nRow);	// Delete this row
    void NewCategory();		// Insert a new category
    void Notify(ButtonStatus nMessage);	// Notification from a child widget
    void RenameCategory(int nRow);	// Rename this row
  private:  CategoryDB * m_pDB;
    // Pointer to the database to be used for this dialog
    static const char *m_pszType[4];	// Titles for the dialog based on type of information
    CategoryEditorList *m_pList;	// The list widget in the center of the screen
    Fl_Button *m_pDeleteButton;	// The Delete button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pNewButton;	// The New button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Button *m_pRenameButton;	// The Rename button
    enum HelpID m_nHelpID;	// Help ID
    enum Type m_nType;		// Type of categories being edited
    static void OnHelpButton(Fl_Widget * pWidget,	// Help button callback
			     void *pUserData);
    static void OnNewButton(Fl_Widget * pWidget,	// New button callback
			    void *pUserData);
    static void OnDeleteButton(Fl_Widget * pWidget,	// Delete button callback
			       void *pUserData);
    static void OnRenameButton(Fl_Widget * pWidget,	// Rename button callback
			       void *pUserData);
    static bool Validate(const char *pszString,	// Validate a new category
			 Fl_Widget * pThis, void *pData);
};


#endif /*  */
