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
// Center of the window for the Notes display                   //
//--------------------------------------------------------------//
#ifndef NOTES_H_

#define NOTES_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include "Messages.h"
#include "NoteDetails.h"
#include "NoteEditor.h"
#include "NoteList.h"
class Notes:public Fl_Group
{
  public:Notes(Fl_Widget * pParent);
    // Default Constructor
    ~Notes();			// Destructor
    inline bool CanPaste() const	// Is there a row that can be pasted
    {
	return (m_vCopyString.size() > 0);
    }
    int Copy(int nRow);		// Copy a row to m_strCopyString
    int Cut(int nRow);		// Cut a row to m_strCopyString
    void Delete(int nRow);	// Delete a note
    inline void DisplayRow(int nRow)	// Display a particular row from the Notes List
    {
	m_pNoteDetails->DisplayRow(nRow, m_pCategory->value() - 1);
    }
    void EditNew();		// Insertion of a new Note row has been requested
    inline void Filter(int nCategory)	// Filter rows based on category
    {
	m_pNoteList->Filter(nCategory);
    }
    int Message(PixilDTMessage nMessage,	// Notification from the parent widget
		int nInfo);
    int Paste();		// Paste the most recently cut/copied row
    inline int SaveDetailChanges()	// Save any outstanding changes on the details window (return of 0 indicates the user said not to save)
    {
	return (m_pNoteDetails->SaveChanges(true));
    }
  private:bool m_bUndoCut;	// Can a cut be undone
    bool m_bUndoPaste;		// Can a paste be undone
    Fl_Box *m_pTitle;		// The page title
    Fl_Button *m_pNewButton;	// The new button
    Fl_Choice *m_pCategory;	// Category choice
    Fl_Menu_Item *m_pCategoryMenu;	// Category choice menu
    int m_nLastPaste;		// Last row pasted
    NoteDetails *m_pNoteDetails;	// The details for this note
    NoteList *m_pNoteList;	// The note list in the center of the window
    vector < string > m_vCopyString;	// Most recently copied or cut row
    static void CategorySelected(Fl_Widget * pWidget,	// Category filtering has been changed
				 void *pUserData);
    void FixDeletedCategory(int nCategory);	// Reset any entries with a deleted category back to Unfiled
    inline static void NewButton(Fl_Widget * pWidget,	// Callback for the New button
				 void *pUserData)
    {
	((Notes *) (pWidget->parent()))->EditNew();
    }
    void ResetCategoryChoice();	// Reset the category choice after a category deletion
    int Undo();			// Undo the most recent cut/paste
  public:void Refresh()
    {
	m_pNoteList->Refresh();
    }
};


#endif /*  */
