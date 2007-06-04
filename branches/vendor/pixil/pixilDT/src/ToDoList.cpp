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
// Center of the window for the ToDo List display               //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "Options.h"
#include "PixilDT.h"
#include "ToDoList.h"
#include "ToDoListCategoryDB.h"
#include "ToDoListShowDlg.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
ToDoList::ToDoList(Fl_Widget * pParent)
:  Fl_Group(pParent->x(), pParent->y(), pParent->w(), pParent->h())
{
    // Initialize these
    m_bUndoCut = false;
    m_bUndoPaste = false;

    // Reset the default color
    color(FL_GRAY);

    // Set up the details display, must be first as the list makes use of it during construction.
    m_pToDoListDetails =
	new ToDoListDetails(x() + w() - DETAILS_WIDTH - DLG_BORDER,
			    y() + DLG_BORDER, DETAILS_WIDTH,
			    h() - 2 * DLG_BORDER);
    m_pToDoListDetails->Enable(false);

    // Set up the title
    m_pTitle = new Fl_Box(x() + DLG_BORDER,
			  y() + DLG_BORDER,
			  PAGE_LABEL_WIDTH, DLG_BUTTON_HEIGHT);
    m_pTitle->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pTitle->label(_("To Do List"));
    m_pTitle->box(FL_DOWN_BOX);
    m_pTitle->labelfont(FL_HELVETICA_BOLD);

    m_pCategory =
	new Fl_Choice(x() + w() - DETAILS_WIDTH - 2 * DLG_BORDER -
		      CATEGORY_CHOICE_WIDTH, y() + DLG_BORDER,
		      CATEGORY_CHOICE_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCategoryMenu =
	ToDoListCategoryDB::GetToDoListCategoryDB()->GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
    m_pCategory->callback(CategorySelected);

    // Set up the list
    m_pToDoListList = new ToDoListList(x() + DLG_BORDER, y() + 2 * DLG_BORDER + DLG_BUTTON_HEIGHT, w() - DETAILS_WIDTH - 3 * DLG_BORDER, h() - 4 * DLG_BORDER - 2 * DLG_BUTTON_HEIGHT, false);	// Not a small list
    m_pToDoListList->color(FL_WHITE);

    // Set up the buttons
    m_pShowButton =
	new Fl_Button(x() + w() - 2 * (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("Sh&ow..."));
    m_pShowButton->callback(ShowButton);
    m_pNewButton =
	new Fl_Button(x() + w() - (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&New"));
    m_pNewButton->callback(NewButton);
    end();

    // Set that the list area is resizable
    resizable(m_pToDoListList);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ToDoList::~ToDoList()
{
    FreeTranslatedMenu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Category selection has been changed, filter the ToDo List    //
// entries                                                      //
//--------------------------------------------------------------//
void
ToDoList::CategorySelected(Fl_Widget * pWidget, void *pUserData)
{
    ToDoList *pThis = reinterpret_cast < ToDoList * >(pWidget->parent());

    // Filter rows based on category
    // Get the category value -1 so that -1 is the All category
    pThis->Filter((reinterpret_cast < Fl_Choice * >(pWidget))->value() - 1);
}


//--------------------------------------------------------------//
// Copy a row to m_strCopyString.                               //
//--------------------------------------------------------------//
int
ToDoList::Copy(int nRow)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nReturn;

    if (nRow >= 0) {
	// Save any outstanding changes
	SaveDetailChanges();

	// Copy this row
	pToDoListDB->Export(nRow, m_vCopyString);
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
ToDoList::Cut(int nRow)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nReturn;

    if (nRow >= 0) {
	// Save any outstanding changes
	SaveDetailChanges();

	// Copy this row
	pToDoListDB->Export(nRow, m_vCopyString);
	pToDoListDB->Delete(nRow);
	pToDoListDB->Save();
	m_bUndoCut = true;
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	nReturn = 1;
    } else {
	// No row to copy
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Confirm the deletion of a ToDo item.                         //
//--------------------------------------------------------------//
void
ToDoList::Delete(int nRow)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nContinue;

    if (Options::GetConfirmDelete() == true) {
	nContinue = fl_ask(_("Delete To Do Item: %s ?"),
			   pToDoListDB->GetTitle(nRow).c_str());
    } else {
	nContinue = 1;
    }

    if (nContinue == 1) {
	pToDoListDB->Delete(nRow);
	pToDoListDB->Save();
	PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
    }
}


//--------------------------------------------------------------//
// Insert a new entry via the dialog.                           //
//--------------------------------------------------------------//
void
ToDoList::EditNew()
{
    int nCategory;
    int nRecno;
    int nRow;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    ToDoListCategoryDB *pToDoListCategoryDB =
	ToDoListCategoryDB::GetToDoListCategoryDB();

    // Create a new todo item
    nCategory = m_pCategory->value() - 1;
    if (nCategory >= 0) {
	nCategory = pToDoListCategoryDB->GetCategoryID(nCategory);
    } else {
	nCategory = 0;
    }
    nRow = pToDoListDB->Insert(m_pCategory->value() - 1);
    nRecno = pToDoListDB->GetID(nRow);

    // OK button was pressed, refresh displays
    PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);

    // Select the row just added or updated
    m_pToDoListList->SelectRow(pToDoListDB->FindRow(TODO_ID, nRecno));
}


//--------------------------------------------------------------//
// Change any line with a category just deleted back to Unfiled //
//--------------------------------------------------------------//
void
ToDoList::FixDeletedCategory(int nCategory)
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nMax = pToDoListDB->NumRecs();
    int nRow;

    for (nRow = 0; nRow < nMax; ++nRow) {
	if (pToDoListDB->IsDeleted(nRow) == false) {
	    if (pToDoListDB->GetCategory(nRow) == nCategory) {
		// "0" should be the "Unfiled" category
		pToDoListDB->SetCategory(nRow, 0);
	    }
	}
    }
    pToDoListDB->Save();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
ToDoList::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case SHOW_TODO_OPTIONS:
	// Show the ToDo list options dialog
	ViewShow();
	break;

    case TODO_LIST_CATEGORY_ADDED:
	// Fix and reset the category choice
	ResetCategoryChoice();
	break;

    case TODO_LIST_CATEGORY_DELETED:
	// Fix all ToDo entries with this category to be Unfiled
	FixDeletedCategory(nInfo);

	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the ToDo List
	m_pToDoListList->Refresh();
	break;

    case TODO_LIST_CATEGORY_RENAMED:
	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the ToDo List
	m_pToDoListList->Refresh();
	break;

    case TODO_LIST_CHANGED:	// The todo list has been changed
	m_pToDoListList->Refresh();
	break;

    case TODO_LIST_DELETE:	// Delete the current item
	Delete(m_pToDoListList->GetRealRow());
	break;

    case TODO_LIST_GOTO:	// Find dialog requested a ToDo List item, have the list do it
	m_pToDoListList->Message(nMessage, nInfo);
	break;

    case TODO_LIST_NEW:	// Insertion of a new to do list entry has been requested
	EditNew();
	break;

    case TODO_LIST_PRINT:	// Print the To Do List
	m_pToDoListList->Print();
	break;

    case TODO_LIST_REQUESTED:	// Selection changing, no processing needed
    case APPLICATION_CLOSING:	// No processing needed, alredy saved if going to
	break;

    case ADDRESS_BOOK_REQUESTED:	// Selection changing, save if needed
    case NOTES_REQUESTED:	// Selection changing, save if needed
    case SCHEDULER_REQUESTED:	// Selection changing, save if needed
    case SELECTION_CHANGING:	// Save if needed
	nReturn = m_pToDoListDetails->SaveChanges(true);
	break;

    case EDIT_COPY:		// Copy the currently selected line
	nReturn = Copy(m_pToDoListList->GetRealRow());
	break;

    case EDIT_CUT:		// Cut the currently selected line
	nReturn = Cut(m_pToDoListList->GetRealRow());
	break;

    case EDIT_PASTE:		// Paste a prior copy or cut
	nReturn = Paste();
	break;

    case EDIT_UNDO:		// Undo a prior cut/paste
	nReturn = Undo();
	break;

    case EDIT_COPY_AVAILABLE:	// Can a row be cut or copied
    case EDIT_CUT_AVAILABLE:
	nReturn = (m_pToDoListList->rows() > 0
		   && m_pToDoListList->row() >= 0);
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
ToDoList::Paste()
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nReturn;

    if (m_vCopyString.size() > 0) {
	// Save any outstanding changes
	SaveDetailChanges();

	m_nLastPaste = pToDoListDB->Import(m_vCopyString);
	pToDoListDB->Save();
	m_bUndoPaste = true;
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
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
ToDoList::ResetCategoryChoice()
{
    FreeTranslatedMenu(m_pCategoryMenu);
    m_pCategoryMenu =
	ToDoListCategoryDB::GetToDoListCategoryDB()->GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Undo the most recent cut or paste                            //
//--------------------------------------------------------------//
int
ToDoList::Undo()
{
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();
    int nReturn;

    if (m_bUndoCut == true) {
	// Save any pending changes
	SaveDetailChanges();

	// Insert this row back
	pToDoListDB->Import(m_vCopyString);
	pToDoListDB->Save();
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	nReturn = 1;
    } else if (m_bUndoPaste == true) {
	// Delete this row
	pToDoListDB->Delete(m_nLastPaste);
	pToDoListDB->Save();
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	nReturn = 1;
    } else {
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Change the view options via the dialog.                      //
//--------------------------------------------------------------//
void
ToDoList::ViewShow()
{
    ToDoListShowDlg *pDlg =
	new ToDoListShowDlg(PixilDT::GetApp()->GetMainWindow());

    if (pDlg->DoModal() == 1) {
	m_pToDoListList->Refresh();
    }
    delete pDlg;
}
