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
// Note Editor Dialog.                                          //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/Fl_Return_Button.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "NoteEditorDlg.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 350
#define DLG_WIDTH  (3*DLG_BUTTON_WIDTH+4*DLG_BORDER)

//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NoteEditorDlg::NoteEditorDlg(Note * pNote, Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Edit Note"))
{
    m_pNoteEditor = new NoteEditor(DLG_BORDER,
				   DLG_BORDER,
				   w() - 2 * DLG_BORDER,
				   h() - DLG_BUTTON_HEIGHT - 3 * DLG_BORDER,
				   true, pNote);

    // Create the buttons
    m_pOKButton =
	new Fl_Return_Button(w() - 3 * (DLG_BUTTON_WIDTH + DLG_BORDER),
			     h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
			     DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton =
	new Fl_Button(w() - 2 * (DLG_BUTTON_WIDTH + DLG_BORDER),
		      h() - DLG_BUTTON_HEIGHT - DLG_BORDER, DLG_BUTTON_WIDTH,
		      DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  h() - (DLG_BUTTON_HEIGHT + DLG_BORDER),
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);

    // Finish the dialog
    end();

    m_pNote = NULL;

    // The DoModal method will show this dialog
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NoteEditorDlg::~NoteEditorDlg()
{
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
NoteEditorDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// OK button was pressed, get the updated note
	m_pNote = m_pNoteEditor->GetNote();

	// Notify everyone of the changes
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
NoteEditorDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_NOTE_EDITOR_DLG);
}
