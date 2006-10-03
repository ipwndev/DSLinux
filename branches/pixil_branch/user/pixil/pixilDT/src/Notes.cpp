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
#include "config.h"
#include <cassert>
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Notes.h"
#include "NotesCategoryDB.h"
#include "Options.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
Notes::Notes(Fl_Widget * pParent)
:  Fl_Group(pParent->x(), pParent->y(), pParent->w(), pParent->h())
{
    // Initialize these
    m_bUndoCut = false;
    m_bUndoPaste = false;

    // Reset the default color
    color(FL_GRAY);

    // Set up the details display, must be first as the list makes use of it during construction.
    m_pNoteDetails = new NoteDetails(x() + w() - DETAILS_WIDTH - DLG_BORDER,
				     y() + DLG_BORDER,
				     DETAILS_WIDTH, h() - 2 * DLG_BORDER);
    m_pNoteDetails->Enable(false);

    // Set up the title
    m_pTitle = new Fl_Box(x() + DLG_BORDER,
			  y() + DLG_BORDER,
			  PAGE_LABEL_WIDTH, DLG_BUTTON_HEIGHT);
    m_pTitle->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pTitle->label(_("Notes"));
    m_pTitle->box(FL_DOWN_BOX);
    m_pTitle->labelfont(FL_HELVETICA_BOLD);

    m_pCategory =
	new Fl_Choice(x() + w() - DETAILS_WIDTH - 2 * DLG_BORDER -
		      CATEGORY_CHOICE_WIDTH, y() + DLG_BORDER,
		      CATEGORY_CHOICE_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCategoryMenu =
	NotesCategoryDB::GetNotesCategoryDB()->GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
    m_pCategory->callback(CategorySelected);

    // Set up the list
    m_pNoteList = new NoteList(x() + DLG_BORDER,
			       y() + 2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
			       w() - DETAILS_WIDTH - 3 * DLG_BORDER,
			       h() - 4 * DLG_BORDER - 2 * DLG_BUTTON_HEIGHT);
    m_pNoteList->color(FL_WHITE);

    // Set up the button
    m_pNewButton =
	new Fl_Button(x() + w() - (DLG_BUTTON_WIDTH + DLG_BORDER) -
		      DETAILS_WIDTH - DLG_BORDER,
		      y() + h() - (DLG_BORDER + DLG_BUTTON_HEIGHT),
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("&New"));
    m_pNewButton->callback(NewButton);
    end();

    // Set that the list area is resizable
    resizable(m_pNoteList);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
Notes::~Notes()
{
    FreeTranslatedMenu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Category selection has been changed, filter the Notes        //
// entries                                                      //
//--------------------------------------------------------------//
void
Notes::CategorySelected(Fl_Widget * pWidget, void *pUserData)
{
    Notes *pThis = reinterpret_cast < Notes * >(pWidget->parent());

    // Save any changes
    pThis->SaveDetailChanges();

    // Filter rows based on category
    // Get the category value -1 so that -1 is the All category
    pThis->Filter((reinterpret_cast < Fl_Choice * >(pWidget))->value() - 1);
}


//--------------------------------------------------------------//
// Copy a row to m_strCopyString.                               //
//--------------------------------------------------------------//
int
Notes::Copy(int nRow)
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nReturn;

    if (nRow >= 0) {
	// Save any changes
	SaveDetailChanges();

	// Copy this row
	pNoteDB->Export(nRow, m_vCopyString);
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
Notes::Cut(int nRow)
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nReturn;

    if (nRow >= 0) {
	// Save any changes
	SaveDetailChanges();

	// Cut this row
	pNoteDB->Export(nRow, m_vCopyString);
	pNoteDB->Delete(nRow);
	pNoteDB->Save();
	m_bUndoCut = true;
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
	nReturn = 1;
    } else {
	// No row to copy
	nReturn = 0;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Confirm the deletion of a Note.                              //
//--------------------------------------------------------------//
void
Notes::Delete(int nRow)
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nContinue;

    if (Options::GetConfirmDelete() == true) {
	nContinue = fl_ask(_("Delete Note: %s ?"),
			   pNoteDB->GetTitle(nRow).length() > 0
			   ? pNoteDB->GetTitle(nRow).c_str()
			   : _("(no title)"));
    } else {
	nContinue = 1;
    }

    if (nContinue == 1) {
	pNoteDB->Delete(nRow);
	pNoteDB->Save();
	PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
    }
}


//--------------------------------------------------------------//
// Insert a new entry entry via the dialog.                     //
//--------------------------------------------------------------//
void
Notes::EditNew()
{
    int nCategory;
    int nRecno;
    int nRow;
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    NotesCategoryDB *pNotesCategoryDB = NotesCategoryDB::GetNotesCategoryDB();

    // Save any changes
    SaveDetailChanges();

    // Create a new note
    nCategory = m_pCategory->value() - 1;
    if (nCategory >= 0) {
	nCategory = pNotesCategoryDB->GetCategoryID(nCategory);
    } else {
	nCategory = 0;
    }
    nRow = pNoteDB->Insert(nCategory);
    nRecno = pNoteDB->GetIndex(nRow);

    // OK button was pressed, refresh displays
    PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);

    // Select the row just added or updated
    m_pNoteList->SelectRow(pNoteDB->FindRow(NOTE_INDEX, nRecno));
}


//--------------------------------------------------------------//
// Change any line with a category just deleted back to Unfiled //
//--------------------------------------------------------------//
void
Notes::FixDeletedCategory(int nCategory)
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nMax = pNoteDB->NumRecs();
    int nRow;

    for (nRow = 0; nRow < nMax; ++nRow) {
	if (pNoteDB->IsDeleted(nRow) == false) {
	    if (pNoteDB->GetCategory(nRow) == nCategory) {
		// "0" should be the "Unfiled" category
		pNoteDB->SetCategory(nRow, 0);
	    }
	}
    }
    pNoteDB->Save();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
Notes::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case NOTES_CATEGORY_ADDED:
	// Fix and reset the category choice
	ResetCategoryChoice();
	break;

    case NOTES_CATEGORY_DELETED:
	// Fix all Notes entries with this category to be Unfiled
	FixDeletedCategory(nInfo);

	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the Note List
	m_pNoteList->Refresh();
	break;

    case NOTES_CATEGORY_RENAMED:
	// Fix and reset the category choice
	ResetCategoryChoice();

	// Refresh the Note List
	m_pNoteList->Refresh();
	break;

    case NOTES_CHANGED:	// Notes have been changed
	m_pNoteList->Refresh();
	break;

    case NOTES_DELETE:		// Delete the current item
	Delete(m_pNoteList->row());
	break;

    case NOTES_GOTO:		// Find dialog requested a note
	m_pNoteList->Message(nMessage, nInfo);
	break;

    case NOTES_NEW:		// Insertion of a new note has been requested
	EditNew();
	break;

    case NOTES_PRINT:		// Print the notes
	m_pNoteList->Print();
	break;

    case NOTES_REQUESTED:	// Selection changing back to notes, no processing needed
    case APPLICATION_CLOSING:	// No processing needed, already saved if need be
	break;

    case ADDRESS_BOOK_REQUESTED:	// Selection changing, save any outstanding updates
    case SCHEDULER_REQUESTED:	// Selection changing, save any outstanding updates
    case TODO_LIST_REQUESTED:	// Selection changing, save any outstanding updates
    case SELECTION_CHANGING:	// Save any outstanding updates
	nReturn = m_pNoteDetails->SaveChanges(true);
	break;

    case EDIT_COPY:		// Copy the currently selected line
	nReturn = Copy(m_pNoteList->row());
	break;

    case EDIT_CUT:		// Cut the currently selected line
	nReturn = Cut(m_pNoteList->row());
	break;

    case EDIT_PASTE:		// Paste a prior copy or cut
	nReturn = Paste();
	break;

    case EDIT_UNDO:		// Undo a prior cut/paste
	nReturn = Undo();
	break;

    case EDIT_COPY_AVAILABLE:	// Can a row be cut or copied
    case EDIT_CUT_AVAILABLE:
	nReturn = (m_pNoteList->rows() > 0 && m_pNoteList->row() >= 0);
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
Notes::Paste()
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nReturn;

    if (m_vCopyString.size() > 0) {
	// Save any changes
	SaveDetailChanges();

	m_nLastPaste = pNoteDB->Import(m_vCopyString);
	pNoteDB->Save();
	m_bUndoPaste = true;
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
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
Notes::ResetCategoryChoice()
{
    FreeTranslatedMenu(m_pCategoryMenu);
    m_pCategoryMenu =
	NotesCategoryDB::GetNotesCategoryDB()->GetCategoryMenu(true);
    m_pCategory->menu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Undo the most recent cut or paste                            //
//--------------------------------------------------------------//
int
Notes::Undo()
{
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    int nReturn;

    if (m_bUndoCut == true) {
	// Save any changes
	SaveDetailChanges();

	// Insert this row back
	pNoteDB->Import(m_vCopyString);
	pNoteDB->Save();
	m_bUndoCut = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
	nReturn = 1;
    } else if (m_bUndoPaste == true) {
	// Delete this row
	pNoteDB->Delete(m_nLastPaste);
	pNoteDB->Save();
	m_bUndoPaste = false;
	PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
	nReturn = 1;
    } else {
	nReturn = 0;
    }
    return (nReturn);
}
