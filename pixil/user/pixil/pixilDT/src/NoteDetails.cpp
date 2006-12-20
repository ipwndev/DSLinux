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
#include "config.h"
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "NoteDB.h"
#include "NoteDetails.h"
#include "NotesCategoryDB.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NoteDetails::NoteDetails(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight)
{
    m_pTitlePrompt = new Fl_Box(x() + DLG_BORDER,
				y(),
				w() - 2 * DLG_BORDER,
				DLG_INPUT_HEIGHT, _("Description:"));
    m_pTitlePrompt->
	align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    m_pTitle =
	new Fl_Input(x() + DLG_BORDER, y() + DLG_INPUT_HEIGHT + DLG_BORDER,
		     w() - 2 * DLG_BORDER, DLG_INPUT_HEIGHT);
    m_pTitle->maximum_size(NoteDB::GetNoteDB()->GetColumnSize(NOTE_DESC));
    m_pNoteEditor = new NoteEditor(x() + DLG_BORDER,
				   y() + 2 * DLG_INPUT_HEIGHT +
				   2 * DLG_BORDER, w() - 2 * DLG_BORDER,
				   h() - DLG_INPUT_HEIGHT -
				   3 * DLG_BUTTON_HEIGHT - 4 * DLG_BORDER,
				   false);
    m_pNoteEditor->SetDestroyNote(true);
    m_pCategoryPrompt = new Fl_Box(x() + DLG_BORDER,
				   y() + h() - 2 * DLG_BUTTON_HEIGHT -
				   DLG_BORDER,
				   w() - DLG_BUTTON_WIDTH - 2 * DLG_BORDER,
				   DLG_BUTTON_HEIGHT, _("Category:"));
    m_pCategoryChoice =
	new Fl_Choice(x() + w() - DLG_BUTTON_WIDTH - DLG_BORDER,
		      y() + h() - 2 * DLG_BUTTON_HEIGHT - DLG_BORDER,
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);
    m_pCategoryMenu =
	NotesCategoryDB::GetNotesCategoryDB()->GetCategoryMenu(false);
    m_pCategoryChoice->menu(m_pCategoryMenu);
    m_pApplyButton = new Fl_Button(x() + w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				   y() + h() - DLG_BUTTON_HEIGHT,
				   DLG_BUTTON_WIDTH,
				   DLG_BUTTON_HEIGHT, _("&Apply"));
    m_pApplyButton->callback(OnApply);
    m_pApplyButton->deactivate();
    end();

    // Set that only the note text is resizable
    resizable(m_pNoteEditor);

    // Set that no note is being displayed
    m_nIndex = -1;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NoteDetails::~NoteDetails()
{
    FreeTranslatedMenu(m_pCategoryMenu);
}


//--------------------------------------------------------------//
// Display a note                                               //
//--------------------------------------------------------------//
void
NoteDetails::DisplayRow(int nRow, int nCategory)
{
    int nCategoryNo;
    int nRows;
    NoteDB *pNoteDB = NoteDB::GetNoteDB();

    // Get the number of records that should be in the list
    if (nCategory == -1) {
	nRows = pNoteDB->NumUndeletedRecs();
    } else {
	// Translate the category
	nCategoryNo =
	    NotesCategoryDB::GetNotesCategoryDB()->GetCategoryID(nCategory);
	nRows = pNoteDB->NumRecsByKey(NOTE_CAT, nCategoryNo);
    }

    // Enable everything if there are some rows
    if (nRows > 0) {
	// Enable all input objects
	Enable(true);

	// Change the display if this is a different row in the data base
	if (pNoteDB->GetIndex(nRow) != m_nIndex) {
	    // Save any outstanding changes
	    SaveChanges(true);

	    // Display this row (and destroy the older note object)
	    m_pTitle->value(pNoteDB->GetTitle(nRow).c_str());
	    nCategoryNo =
		NotesCategoryDB::GetNotesCategoryDB()->FindRow(CATID,
							       pNoteDB->
							       GetCategory
							       (nRow));
	    if (nCategoryNo < 0) {
		// Fix if bad category
		nCategoryNo = 0;
	    }
	    m_pCategoryChoice->value(nCategoryNo);
	    m_pNoteEditor->SetNote(pNoteDB->GetNote(nRow), true);

	    // Note the key-id being displayed
	    m_nIndex = pNoteDB->GetIndex(nRow);
	}
    } else {
	// Disable everything if there are no rows for the category choice
	Enable(false);
	m_nIndex = -1;
    }
}


//--------------------------------------------------------------//
// Enable or disable all input fields.                          //
//--------------------------------------------------------------//
void
NoteDetails::Enable(bool bEnable)
{
    if (bEnable == true) {
	// Activate everything
	m_pApplyButton->activate();
	m_pCategoryChoice->activate();
	m_pTitle->activate();
	m_pTitle->color(FL_WHITE);
    } else {
	// Deactivate everything
	m_pApplyButton->deactivate();
	m_pCategoryChoice->value(0);
	m_pCategoryChoice->deactivate();
	m_pTitle->value("");
	m_pTitle->deactivate();
	m_pTitle->color(FL_GRAY);
    }
    m_pNoteEditor->Enable(bEnable);
}


//--------------------------------------------------------------//
// Process a message from the parent widget                     //
//--------------------------------------------------------------//
int
NoteDetails::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case -1:			// Not implemented yet
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
// Apply changed to a note                                      //
//--------------------------------------------------------------//
void
NoteDetails::OnApply(Fl_Widget * pWidget, void *pUserData)
{
    // Save any changes without asking whether to do so or not
    ((NoteDetails *) (pWidget->parent()))->SaveChanges(false);
}


//--------------------------------------------------------------//
// Save any changes to the current note.  A return of 0         //
// indicates that changes were not saved at the user's request. //
//--------------------------------------------------------------//
int
NoteDetails::SaveChanges(bool bAsk)
{
    int nCategory;
    int nResult;
    int nReturn = 1;
    int nRow;
    Note *pNote;
    NoteDB *pNoteDB = NoteDB::GetNoteDB();

    // Find the current row
    nRow = pNoteDB->FindRow(NOTE_INDEX, m_nIndex);
    if (nRow >= 0) {
	// Has anything been changed
	nCategory =
	    NotesCategoryDB::GetNotesCategoryDB()->
	    GetCategoryID(m_pCategoryChoice->value());
	pNote = m_pNoteEditor->GetNote();
	if (pNoteDB->GetCategory(nRow) != nCategory
	    || strcmp(pNoteDB->GetTitle(nRow).c_str(), m_pTitle->value()) != 0
	    || pNote->IsChanged()) {
	    // Does the user want to save these changes
	    if (bAsk == true) {
		nResult =
		    fl_choice(_
			      ("Do you wish to save the changes to the Note:\n\n%s"),
			      fl_no, fl_yes, fl_cancel, m_pTitle->value());
		if (nResult == 1) {
		    // Yes button, indicate to save the data
		    nReturn = 1;
		} else if (nResult == 0) {
		    // No button, indicate to not save the data
		    nResult = 0;
		} else		//if (nResult==2)
		{
		    // Cancel button, indicate to not save and to cancel the calling function
		    nResult = 0;
		    nReturn = 0;
		}
	    } else {
		// Save without asking
		nResult = 1;
	    }

	    // Save the changes
	    if (nResult == 1) {
		pNoteDB->SetCategory(nRow, nCategory);
		pNoteDB->SetTitle(nRow, m_pTitle->value());
		pNote->Save();
		pNoteDB->SetFile(nRow, pNote->GetFileName().c_str());
		pNoteDB->Save();

		// OK button was pressed, refresh displays
		PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
	    }
	}
    }

    return (nReturn);
}
