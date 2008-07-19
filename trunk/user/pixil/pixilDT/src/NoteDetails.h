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
// Class for the Note Details.                                  //
//--------------------------------------------------------------//
#ifndef NOTEDETAILS_H_

#define NOTEDETAILS_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include "Messages.h"
#include "NoteEditor.h"
class NoteDetails:public Fl_Group
{
  public:NoteDetails(int nX,	// Constructor
		int nY, int nWidth, int nHeight);
     ~NoteDetails();		// Destructor
    void DisplayRow(int nRow,	// Display a Note
		    int nCategory);
    void Enable(bool bEnable);	// Enable or disable all input in the details
    int Message(PixilDTMessage nMessage,	// Process a message from the parent widget
		int nInfo);
    int SaveChanges(bool bAsk);	// Save any changes to disk
  private:  Fl_Box * m_pCategoryPrompt;
    // The prompt for the category choice
    Fl_Box *m_pTitlePrompt;	// The prompt for the description
    Fl_Button *m_pApplyButton;	// The Apply button
    Fl_Choice *m_pCategoryChoice;	// The category choice
    Fl_Input *m_pTitle;		// The note description
    Fl_Menu_Item *m_pCategoryMenu;	// Category choice menu
    int m_nIndex;		// The note index being displayed
    NoteEditor *m_pNoteEditor;	// The note text
    static void OnApply(Fl_Widget * pWidget,	// Process an apply button click
			void *pUserData);
};


#endif /*  */
