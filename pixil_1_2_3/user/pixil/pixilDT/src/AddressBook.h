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
// Center of the window for the Address Book display            //
//--------------------------------------------------------------//
#ifndef ADDRESSBOOK_H_

#define ADDRESSBOOK_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <vector>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include "AddressBookDetails.h"
#include "AddressBookList.h"
#include "InputNotify.h"
#include "Messages.h"
using namespace std;
class AddressBook:public Fl_Group
{
  public:AddressBook(Fl_Widget * pParent);
    // Default Constructor
    ~AddressBook();		// Destructor
    inline bool CanPaste() const	// Is there a row that can be pasted
    {
	return (m_vCopyString.size() > 0);
    }
    int Copy(int nRow);		// Copy a row to m_strCopyString
    int Cut(int nRow);		// Cut a row to m_strCopyString
    void Delete(int nRow);	// Delete an address book line
    inline void DisplayRow(int nRow)	// Display a particular row from the Address Book List
    {
	m_pAddressBookDetails->DisplayRow(nRow);
    }
    void Edit(int nRow);	// Edit an address book line
    void EditNew();		// Insertion of a new address book row has been requested
    int Message(PixilDTMessage nMessage,	// Notification from the parent widget
		int nInfo);
    int Paste();		// Paste the most recently cut/copied row
  private:AddressBookDetails * m_pAddressBookDetails;
    // The details portion of the display
    AddressBookList *m_pAddressBookList;	// The list portion of the display
    bool m_bUndoCut;		// Can a cut be undone
    bool m_bUndoPaste;		// Can a paste be undone
    Fl_Box *m_pTitle;		// Label box
    Fl_Button *m_pEditButton;	// Edit button
    Fl_Button *m_pListByButton;	// List By button
    Fl_Button *m_pNewButton;	// New button
    Fl_Choice *m_pCategory;	// Category choice
    Fl_Menu_Item *m_pCategoryMenu;	// Category choice menu
    InputNotify *m_pSearch;	// Search string
    int m_nLastPaste;		// Last row pasted
    vector < string > m_vCopyString;	// Most recently cut/copied row
    static void CategorySelected(Fl_Widget * pWidget,	// Category filtering has been changed
				 void *pUserData);
    static void EditButton(Fl_Widget * pWidget,	// Callback for the Edit button
			   void *pUserData);
    inline void Filter(int nCategory)	// Filter rows based on category
    {
	m_bUndoPaste = false;	// Paste is done by row number and rows may be re-ordered
	m_pAddressBookList->Filter(nCategory);
    }
    void FixDeletedCategory(int nCategory);	// Change any line for a deleted category back to Unfiled
    static void ListByButton(Fl_Widget * pWidget,	// Callback for the List By button
			     void *pUserData);
    inline static void NewButton(Fl_Widget * pWidget,	// Callback for the New button
				 void *pUserData)
    {
	((AddressBook *) (pWidget->parent()))->EditNew();
    }
    void ResetCategoryChoice();	// Reset the category choice after a category deletion
    static void SearchEntered(Fl_Widget * pWidget,	// Search string has been entered
			      void *pUserData);
    void ShowListByDialog();	// Show the List By dialog
    int Undo();			// Undo the most recent cut/paste
  public:void Refresh()
    {
	m_pAddressBookList->Refresh();
    }
};


#endif /*  */
