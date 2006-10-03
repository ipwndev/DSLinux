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
#ifdef WIN32
#pragma warning(disable:4786)
#endif
#include "config.h"
#include <cassert>
#include <FL/fl_ask.H>
#include "AddressBook.h"
#include "AddressBookCategoryDB.h"
#include "AddressBookDB.h"
#include "AddressBookDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "ListByDlg.h"
#include "Options.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define SEARCH_PROMPT   80

#define LIST_TOP_BORDER (TITLE_HEIGHT+2*DLG_BORDER)


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
AddressBook::AddressBook(Fl_Widget * pParent)
:  Fl_Group(pParent->x(), pParent->y(), pParent->w(), pParent->h())
{
    // Initialize these
    m_bUndoCut = false;
    m_bUndoPaste = false;

    // Set up the details display, must be first as the list makes use of it during construction.
    m_pAddressBookDetails =
	new AddressBookDetails(x() + w() - DETAILS_WIDTH - DLG_BORDER,
			       y() + DLG_BORDER, DETAILS_WIDTH,
			       h() - 2 * DLG_BORDER);

    // Set up the title
    m_pTitle = new Fl_Box(x() + DLG_BORDER,
			  y() + DLG_BORDER,
			  PAGE_LABEL_WIDTH, DLG_BUTTON_HEIGHT);
    m_pTitle->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pTitle->label(_("Address"));
    m_pTitle->box(FL_DOWN_BOX);
    m_pTitle->labelfont(FL_HELVETICA_BOLD);

    m_pCategory =
	new Fl_Choice(x() + w() - DETAILS_WIDTH - 2 * DLG_BORDER -
		      CATEGORY_CHOICE_WIDTH, y() + DLG_BORDER,
		      CATEGORY_CHOICE_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCategoryMenu =
	AddressBookCategoryDB::GetAddressBookCategoryDB()->
	GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
    m_pCategory->callback(CategorySelected);

    // Set up the list
    m_pAddressBookList = new AddressBookList(x() + DLG_BORDER,
					     y() + 2 * DLG_BORDER +
					     DLG_BUTTON_HEIGHT,
					     w() - DETAILS_WIDTH -
					     3 * DLG_BORDER,
					     h() - 5 * DLG_BORDER -
					     3 * DLG_BUTTON_HEIGHT, false);
    m_pAddressBookList->color(FL_WHITE);

    // Set up the search string
    m_pSearch = new InputNotify(x() + DLG_BORDER + SEARCH_PROMPT,
				y() + h() - 2 * (DLG_BORDER +
						 DLG_BUTTON_HEIGHT),
				w() - DETAILS_WIDTH - 3 * DLG_BORDER -
				SEARCH_PROMPT, DLG_BUTTON_HEIGHT,
				_("Search:"), SearchEntered);

    // Set up the buttons
    m_pListByButton =
	new Fl_Button(x() + w() - 3 * (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&List By"));
    m_pListByButton->callback(ListByButton);
    m_pEditButton =
	new Fl_Button(x() + w() - 2 * (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&Edit"));
    m_pEditButton->callback(EditButton);
    m_pNewButton =
	new Fl_Button(x() + w() - (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&New"));
    m_pNewButton->callback(NewButton);
    end();

    // Set that the list area is resizable
    resizable(m_pAddressBookList);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
AddressBook::~AddressBook()
{
    FreeTranslatedMenu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Category selection has been changed, filter the Address Book //
// entries                                                      //
//--------------------------------------------------------------//
void
AddressBook::CategorySelected(Fl_Widget * pWidget, void *pUserData)
{
    AddressBook *pThis =
	reinterpret_cast < AddressBook * >(pWidget->parent());

    // Filter rows based on category
    // Get the category value -1 so that -1 is the All category
    pThis->Filter((reinterpret_cast < Fl_Choice * >(pWidget))->value() - 1);
}


//--------------------------------------------------------------//
// Copy a row to m_strCopyString.                               //
//--------------------------------------------------------------//
int
AddressBook::Copy(int nRow)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nReturn;

    if (nRow >= 0) {
	// Copy this row
	pAddressBookDB->Export(nRow, m_vCopyString);
	PixilDT::GetApp()->GetMainWindow()->Notify(FIX_MENU, 0);
	nReturn = 1;
    } else {
	// No row to copy
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Cut a row to m_strCopyString.                                //
//--------------------------------------------------------------//
int
AddressBook::Cut(int nRow)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nReturn;

    if (nRow >= 0) {
	// Copy this row
	pAddressBookDB->Export(nRow, m_vCopyString);
	pAddressBookDB->Delete(nRow);
	pAddressBookDB->Save();
	m_bUndoCut = true;
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
	nReturn = 1;
    } else {
	// No row to copy
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Confirm the deletion of a row in the address book.           //
//--------------------------------------------------------------//
void
AddressBook::Delete(int nRow)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nContinue;

    if (Options::GetConfirmDelete() == true) {
	nContinue = fl_ask(_("Delete Address for %s %s ?"),
			   pAddressBookDB->GetFirstName(nRow).c_str(),
			   pAddressBookDB->GetLastName(nRow).c_str());
    } else {
	nContinue = 1;
    }

    if (nContinue == 1) {
	pAddressBookDB->Delete(nRow);
	pAddressBookDB->Save();
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
    }
}


//--------------------------------------------------------------//
// Edit a row in the address book.                              //
//--------------------------------------------------------------//
void
AddressBook::Edit(int nRow)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    AddressBookDlg *pAddressBookDlg;

#ifdef DEBUG
    printf("nRow = %d rows = %d", nRow, m_pAddressBookList->rows());
    //  assert(nRow>=0&&nRow<m_pAddressBookList->rows());
#endif

    pAddressBookDlg =
	new AddressBookDlg(nRow, PixilDT::GetApp()->GetMainWindow());
    if (pAddressBookDlg->DoModal() == 1) {
	// OK button was pressed, refresh displays
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);

	// Select the row just added or updated
	m_pAddressBookList->SelectRow(pAddressBookDB->
				      FindRecno(pAddressBookDlg->GetRecno()));
    }
    delete pAddressBookDlg;
}


//--------------------------------------------------------------//
// Edit button callback (static).                               //
//--------------------------------------------------------------//
void
AddressBook::EditButton(Fl_Widget * pWidget, void *pUserData)
{
    AddressBook *pThis =
	reinterpret_cast < AddressBook * >(pWidget->parent());

    pThis->Edit(pThis->m_pAddressBookList->GetRealRow());
}


//--------------------------------------------------------------//
// Insert a new address book entry via the dialog.              //
//--------------------------------------------------------------//
void
AddressBook::EditNew()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    AddressBookDlg *pAddressBookDlg =
	new AddressBookDlg(-1, PixilDT::GetApp()->GetMainWindow());

    if (pAddressBookDlg->DoModal() == 1) {
	// OK button was pressed, refresh displays
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);

	// Select the row just added or updated
	m_pAddressBookList->SelectRow(pAddressBookDB->
				      FindRecno(pAddressBookDlg->GetRecno()));
    }
    delete pAddressBookDlg;
}


//--------------------------------------------------------------//
// Change any line with a category just deleted back to Unfiled //
//--------------------------------------------------------------//
void
AddressBook::FixDeletedCategory(int nCategory)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nMax = pAddressBookDB->NumRecs();
    int nRow;

    for (nRow = 0; nRow < nMax; ++nRow) {
	if (pAddressBookDB->IsDeleted(nRow) == false) {
	    if (pAddressBookDB->GetCategory(nRow) == nCategory) {
		// "0" should be the "Unfiled" category
		pAddressBookDB->SetCategory(nRow, 0);
	    }
	}
    }
    pAddressBookDB->Save();
}


//--------------------------------------------------------------//
// Process a click on the List By button.                       //
//--------------------------------------------------------------//
void
AddressBook::ListByButton(Fl_Widget * pWidget, void *pUserData)
{
    AddressBook *pThis =
	reinterpret_cast < AddressBook * >(pWidget->parent());

    pThis->ShowListByDialog();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
AddressBook::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case ADDRESS_BOOK_CATEGORY_ADDED:
	// Fix and reset the category choice
	ResetCategoryChoice();
	break;

    case ADDRESS_BOOK_CATEGORY_DELETED:
	// Fix all Address Book entries with this category to be Unfiled
	FixDeletedCategory(nInfo);

	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the Address Book List
	m_pAddressBookList->Refresh();
	break;

    case ADDRESS_BOOK_CATEGORY_RENAMED:
	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the Address Book List
	m_pAddressBookList->Refresh();
	break;

    case ADDRESS_BOOK_CHANGED:	// The selection has moved to a different row
	m_pAddressBookDetails->Message(nMessage, 0);
	m_pAddressBookList->Message(nMessage, 0);
	break;

    case ADDRESS_BOOK_DELETE:	// Delete the current item
	Delete(m_pAddressBookList->GetRealRow());
	break;

    case ADDRESS_BOOK_GOTO:	// Display a particular line
	m_pAddressBookList->Message(nMessage, nInfo);
	break;

    case ADDRESS_BOOK_NEW:	// Insertion of a new address book entry has been requested
	EditNew();
	break;

    case ADDRESS_BOOK_PRINT:	// Print the address book as currently sorted
	m_pAddressBookList->Print();
	break;

    case SHOW_LIST_BY:		// Show the List By dialog
	ShowListByDialog();
	break;

    case SELECTION_CHANGING:	// No processing needed here
	nReturn = 1;		// Indicate that no errors occurred
	break;

    case ADDRESS_BOOK_REQUESTED:	// Selection changing, no processing needed
    case NOTES_REQUESTED:	// Selection changing, no processing needed
    case SCHEDULER_REQUESTED:	// Selection changing, no processing needed
    case TODO_LIST_REQUESTED:	// Selection changing, no processing needed
    case APPLICATION_CLOSING:	// No processing needed here
	break;

    case EDIT_COPY:		// Copy the currently selected line
	nReturn = Copy(m_pAddressBookList->GetRealRow());
	break;

    case EDIT_CUT:		// Cut the currently selected line
	nReturn = Cut(m_pAddressBookList->GetRealRow());
	break;

    case EDIT_PASTE:		// Paste a prior copy or cut
	nReturn = Paste();
	break;

    case EDIT_UNDO:		// Undo a prior cut/paste
	nReturn = Undo();
	break;

    case EDIT_COPY_AVAILABLE:	// Can a row be cut or copied
    case EDIT_CUT_AVAILABLE:
	nReturn = (m_pAddressBookList->rows() > 0
		   && m_pAddressBookList->row() >= 0);
	break;

    case EDIT_PASTE_AVAILABLE:	// Has a row been cut or copied
	nReturn = (m_vCopyString.size() > 0);
	break;

    case EDIT_UNDO_AVAILABLE:	// Can the last cut/paste be undone
	nReturn = (m_bUndoCut | m_bUndoPaste ? 1 : 0);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Paste the most recently cut row.                             //
//--------------------------------------------------------------//
int
AddressBook::Paste()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nReturn;

    if (m_vCopyString.size() > 0) {
	m_nLastPaste = pAddressBookDB->Import(m_vCopyString);
	pAddressBookDB->Save();
	m_bUndoPaste = true;
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
	nReturn = 1;
    } else {
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Reset the category choice widget after a category deletion.  //
//--------------------------------------------------------------//
void
AddressBook::ResetCategoryChoice()
{
    FreeTranslatedMenu(m_pCategoryMenu);
    m_pCategoryMenu =
	AddressBookCategoryDB::GetAddressBookCategoryDB()->
	GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Pseudo callback from an InputNotify object that its text has //
// changed.                                                     //
//--------------------------------------------------------------//
void
AddressBook::SearchEntered(Fl_Widget * pWidget, void *pUserData)
{
    AddressBook *pThis = reinterpret_cast < AddressBook * >(pWidget);

    if (pThis->m_pAddressBookList->Search(pThis->m_pSearch->value()) == false) {
	string strValue = pThis->m_pSearch->value();

	if (strValue.length() > 1) {
	    strValue = strValue.substr(0, strValue.length() - 1);
	} else {
	    strValue = "";
	}
	pThis->m_pSearch->value(strValue.c_str());
    }
}


//--------------------------------------------------------------//
// Show the List By dialog                                      //
//--------------------------------------------------------------//
void
AddressBook::ShowListByDialog()
{
    ListByDlg *pDlg = new ListByDlg(m_pAddressBookList->GetListBy(),
				    PixilDT::GetApp()->GetMainWindow());

    if (pDlg->DoModal() == 1) {
	// OK button was pressed
	m_pAddressBookList->SetListBy(pDlg->GetSelection());
	m_pAddressBookList->Refresh();
    }
    delete pDlg;
}


//--------------------------------------------------------------//
// Undo the most recent cut or paste                            //
//--------------------------------------------------------------//
int
AddressBook::Undo()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nReturn;

    if (m_bUndoCut == true) {
	// Insert this row back
	pAddressBookDB->Import(m_vCopyString);
	pAddressBookDB->Save();
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
	nReturn = 1;
    } else if (m_bUndoPaste == true) {
	// Delete this row
	pAddressBookDB->Delete(m_nLastPaste);
	pAddressBookDB->Save();
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
	nReturn = 1;
    } else {
	nReturn = 0;
    }
    return (nReturn);
}
