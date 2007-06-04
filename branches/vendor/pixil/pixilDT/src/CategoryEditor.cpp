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
#include "config.h"
#include <FL/fl_ask.H>
#include "AddressBookCategoryDB.h"
#include "CategoryEditor.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "InputBox.h"
#include "NotesCategoryDB.h"
#include "Options.h"
#include "PixilDT.h"
#include "SchedulerCategoryDB.h"
#include "ToDoListCategoryDB.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 400
#define DLG_WIDTH  340


//--------------------------------------------------------------//
// Dialog titles                                                //
//--------------------------------------------------------------//
const char *
    CategoryEditor::m_pszType[4] = {
    N_("Edit Address Book Categories"),
    N_("Edit Note Categories"),
    N_("Edit Scheduler Categories"),
    N_("Edit ToDo List Categories"),
};


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
CategoryEditor::CategoryEditor(Type nType, PixilMainWnd * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _(m_pszType[nType]))
{
    // Get the correct set of categories
    m_nType = nType;
    switch (nType) {
    case AddressBook:
	m_pDB = AddressBookCategoryDB::GetAddressBookCategoryDB();
	m_nHelpID = HELP_ADDRESS_BOOK_CATEGORY;
	break;

    case Notes:
	m_pDB = NotesCategoryDB::GetNotesCategoryDB();
	m_nHelpID = HELP_NOTES_CATEGORY;
	break;

    case Scheduler:
	m_pDB = SchedulerCategoryDB::GetSchedulerCategoryDB();
	m_nHelpID = HELP_SCHEDULER_CATEGORY;
	break;

    case ToDoList:
	m_pDB = ToDoListCategoryDB::GetToDoListCategoryDB();
	m_nHelpID = HELP_TODO_LIST_CATEGORY;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown type of information
#endif
	m_pDB = NULL;
    }

    // Create the dialog widgets
    m_pList = new CategoryEditorList(10, 10, 200, 380, m_pDB);
    m_pOKButton = new Fl_Button(230, 10, 80, 24, fl_ok);
    m_pNewButton = new Fl_Button(230, 45, 80, 24, _("&New"));
    m_pDeleteButton = new Fl_Button(230, 80, 80, 24, _("&Delete"));
    m_pRenameButton = new Fl_Button(230, 115, 80, 24, _("&Rename"));
    m_pHelpButton = new Fl_Button(230, 150, 80, 24, _("&Help"));

    // Finish the dialog
    end();

    // Set the button callbacks
    m_pNewButton->callback(OnNewButton);
    m_pDeleteButton->callback(OnDeleteButton);
    m_pRenameButton->callback(OnRenameButton);
    m_pHelpButton->callback(OnHelpButton);

    // Add the categories to the list widget
    m_pList->Refresh();

    // Run this as a modal dialog
    DoModal(this, m_pOKButton, NULL);
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
CategoryEditor::~CategoryEditor()
{
}


//--------------------------------------------------------------//
// Delete a category by row.                                    //
//--------------------------------------------------------------//
void
CategoryEditor::DeleteCategory(int nRow)
{
    int nCategory;

#ifdef DEBUG
    assert(nRow > 0 && nRow < m_pList->rows());
#endif

    // Only run if the current item is not the "Unfiled" category
    if (nRow > 0) {
	int nContinue;

	if (Options::GetConfirmDelete() == true) {
	    nContinue = fl_ask(_("Delete Category %s ?"),
			       m_pDB->GetCategory(nRow).c_str());
	} else {
	    nContinue = 1;
	}

	if (nContinue == 1) {
	    nCategory = m_pDB->GetCategoryID(nRow);
	    m_pDB->Delete(nRow);
	    m_pDB->Save();
	    m_pList->Refresh();
	    PixilDT::GetApp()->GetMainWindow()->Notify(CATEGORY_DELETED,
						       nCategory);
	}
    }
}


//--------------------------------------------------------------//
// Notification from a child widget.                            //
//--------------------------------------------------------------//
void
CategoryEditor::Notify(ButtonStatus nMessage)
{
    if (nMessage == Disable) {
	// Disable the delete and rename buttons
	m_pDeleteButton->deactivate();
	m_pRenameButton->deactivate();
    } else {
	// Enable the delete and rename buttons
	m_pDeleteButton->activate();
	m_pRenameButton->activate();
    }
}


//--------------------------------------------------------------//
// Insert a new category.                                       //
//--------------------------------------------------------------//
void
CategoryEditor::NewCategory()
{
    AddressBookCategoryDB *pDB =
	AddressBookCategoryDB::GetAddressBookCategoryDB();
    string strNewCategory;
    InputBox *pInputBox;

    // Get the new category name to be entered
    pInputBox = new InputBox(_("New Category"),
			     PixilDT::GetApp()->GetMainWindow(),
			     "",
			     pDB->GetColumnSize(CAT),
			     m_nHelpID,
			     Validate,
			     this,
			     NULL, _("Please Enter the New Category Name:"));
    strNewCategory = pInputBox->GetEntry();
    delete pInputBox;

    // Insert this category if one was entered
    if (strNewCategory.length() != 0) {
	int nRow = m_pDB->Insert(-1);	// -1 for next highest key value

	m_pDB->SetCategory(nRow, strNewCategory.c_str());
	m_pDB->Save();
	m_pList->Refresh();
	PixilDT::GetApp()->GetMainWindow()->Notify(CATEGORY_ADDED, 0);
    }
}


//--------------------------------------------------------------//
// Delete button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
CategoryEditor::OnDeleteButton(Fl_Widget * pWidget, void *pUserData)
{
    CategoryEditor *pThis =
	dynamic_cast < CategoryEditor * >(pWidget->parent());

    pThis->DeleteCategory(pThis->m_pList->row());
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
CategoryEditor::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_CATEGORY_EDITOR);
}


//--------------------------------------------------------------//
// New button was clicked (static callback).                    //
//--------------------------------------------------------------//
void
CategoryEditor::OnNewButton(Fl_Widget * pWidget, void *pUserData)
{
    CategoryEditor *pThis =
	dynamic_cast < CategoryEditor * >(pWidget->parent());

    pThis->NewCategory();
}


//--------------------------------------------------------------//
// Rename button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
CategoryEditor::OnRenameButton(Fl_Widget * pWidget, void *pUserData)
{
    CategoryEditor *pThis =
	dynamic_cast < CategoryEditor * >(pWidget->parent());

    pThis->RenameCategory(pThis->m_pList->row());
}


//--------------------------------------------------------------//
// Rename a category.                                           //
//--------------------------------------------------------------//
void
CategoryEditor::RenameCategory(int nRow)
{
    AddressBookCategoryDB *pDB =
	AddressBookCategoryDB::GetAddressBookCategoryDB();
    InputBox *pInputBox;
    int nCategory;
    string strNewCategory;

#ifdef DEBUG
    assert(nRow >= 0 && nRow < m_pList->rows());
#endif

    // Only run if the current item is not the "Unfiled" category
    if (nRow > 0) {
	// Get the new category name to be entered
	nCategory = m_pDB->GetCategoryID(nRow);
	pInputBox = new InputBox(_("Rename Category"),
				 PixilDT::GetApp()->GetMainWindow(),
				 m_pDB->GetCategory(nRow).c_str(),
				 pDB->GetColumnSize(CAT),
				 m_nHelpID,
				 Validate,
				 this,
				 reinterpret_cast < void *>(nCategory),
				 _("Please Enter the New Category Name:"));
	strNewCategory = pInputBox->GetEntry();
	delete pInputBox;

	// Change this category if one was entered
	if (strNewCategory.length() > 0) {
	    m_pDB->SetCategory(nRow, strNewCategory.c_str());
	    m_pDB->Save();
	    m_pList->Refresh();
	    PixilDT::GetApp()->GetMainWindow()->Notify(CATEGORY_RENAMED, 0);
	}
    }
}


//--------------------------------------------------------------//
// Validate a new category name (static callback).              //
//--------------------------------------------------------------//
bool
CategoryEditor::Validate(const char *pszString,
			 Fl_Widget * pThis, void *pUserData)
{
    // Test this name against all existing category names
    return (((CategoryEditor *) pThis)->m_pDB->
	    TestDuplicate(pszString, pUserData));
}
