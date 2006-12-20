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
// Note Editor widget                                           //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/Fl.H>
#include "Dialog.h"
#include "Images.h"
#include "NoteEditor.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
NoteEditor::NoteEditor(int nX,
		       int nY,
		       int nWidth, int nHeight, bool bToolbar, Note * pNote)
    :
Fl_Group(nX, nY, nWidth, nHeight)
{
    // Save this note object
    m_pNote = pNote;

    if (bToolbar == true) {
	// Create the toolbar
	m_pToolbar =
	    new Toolbar(x(), y(), w(), TOOLBAR_HEIGHT, m_ToolbarItem);
    } else {
	// No toolbar
	m_pToolbar = NULL;
    }

    // Create the input widget
    m_pInput = new Fl_Multiline_Input(x(),
				      y() + (bToolbar ? TOOLBAR_HEIGHT +
					     DLG_BORDER : 0), w(),
				      h() - (bToolbar ? TOOLBAR_HEIGHT +
					     DLG_BORDER : 0));
    if (m_pNote != NULL) {
	m_pInput->value(m_pNote->GetText().c_str());
	if (pNote->GetMaxLength() > 0) {
	    m_pInput->maximum_size(pNote->GetMaxLength());
	}
    }
    // Finish this widget
    end();

    // Set to not automatically destroy the note
    m_bDestroyNote = false;
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
NoteEditor::~NoteEditor()
{
    if (m_bDestroyNote) {
	delete m_pNote;
    }
}


//--------------------------------------------------------------//
// Copy from the toolbar.                                       //
//--------------------------------------------------------------//
void
NoteEditor::EditCopy(Fl_Widget * pWidget, void *pUserData)
{
    NoteEditor *pThis = reinterpret_cast < NoteEditor * >(pWidget->parent());

    pThis->m_pInput->copy();
}


//--------------------------------------------------------------//
// Cut from the toolbar.                                        //
//--------------------------------------------------------------//
void
NoteEditor::EditCut(Fl_Widget * pWidget, void *pUserData)
{
    NoteEditor *pThis = reinterpret_cast < NoteEditor * >(pWidget->parent());

    pThis->m_pInput->cut();
}


//--------------------------------------------------------------//
// Paste from the toolbar.                                      //
//--------------------------------------------------------------//
void
NoteEditor::EditPaste(Fl_Widget * pWidget, void *pUserData)
{
    NoteEditor *pThis = reinterpret_cast < NoteEditor * >(pWidget->parent());

    Fl::paste(*(pThis->m_pInput));
}


//--------------------------------------------------------------//
// Undo from the toolbar.                                       //
//--------------------------------------------------------------//
void
NoteEditor::EditUndo(Fl_Widget * pWidget, void *pUserData)
{
    NoteEditor *pThis = reinterpret_cast < NoteEditor * >(pWidget->parent());

    pThis->m_pInput->undo();
}


//--------------------------------------------------------------//
// Enable/disable everything.                                   //
//--------------------------------------------------------------//
void
NoteEditor::Enable(bool bEnable)
{
    if (bEnable == true) {
	// Activate everything
	m_pInput->activate();
	m_pInput->color(FL_WHITE);
    } else {
	// Deactivate everything
	m_pInput->value("");
	m_pInput->deactivate();
	m_pInput->color(FL_GRAY);
    }

    // Enable/disable the toolbar
    if (m_pToolbar != NULL) {
	m_pToolbar->Enable(Images::EDITCUT_ICON, bEnable);
	m_pToolbar->Enable(Images::EDITCOPY_ICON, bEnable);
	m_pToolbar->Enable(Images::EDITPASTE_ICON, bEnable);
	m_pToolbar->Enable(Images::EDITUNDO_ICON, bEnable);
    }
}


//--------------------------------------------------------------//
// Get the note object.                                         //
//--------------------------------------------------------------//
Note *
NoteEditor::GetNote()
{
    if (m_pNote != NULL) {
	m_pNote->SetText(m_pInput->value());
    }
    return (m_pNote);
}


//--------------------------------------------------------------//
// Change the current Note object.  The Note object is owned by //
// the parent of this NoteEditor, that widget is responsible    //
// for deleting the older note unless it passes "true" for the  //
// bDestroy argument to this method.                            //
//--------------------------------------------------------------//
void
NoteEditor::SetNote(Note * pNote, bool bDestroy)
{
    if (bDestroy == true) {
	delete m_pNote;
    }
    m_pNote = pNote;
    if (m_pNote != NULL) {
	m_pInput->value(m_pNote->GetText().c_str());
    } else {
	m_pInput->value("");
    }
}


//--------------------------------------------------------------//
// The toolbar definition structure.                            //
//--------------------------------------------------------------//
const ToolbarItem
    NoteEditor::m_ToolbarItem[] = {
    {TOOLBAR_BUTTON, N_("Cut"), EditCut, Images::EDITCUT_ICON,
     Images::DISABLED_CUT_ICON},
    {TOOLBAR_BUTTON, N_("Copy"), EditCopy, Images::EDITCOPY_ICON,
     Images::DISABLED_COPY_ICON},
    {TOOLBAR_BUTTON, N_("Paste"), EditPaste, Images::EDITPASTE_ICON,
     Images::DISABLED_PASTE_ICON},
    {TOOLBAR_BUTTON, N_("Undo"), EditUndo, Images::EDITUNDO_ICON,
     Images::DISABLED_UNDO_ICON},
    {TOOLBAR_END, NULL, NULL, 0},
};
