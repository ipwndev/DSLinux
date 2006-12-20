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
// Find Dialog.                                                 //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include "AddressBookDB.h"
#include "Dialog.h"
#include "FindDlg.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "NoteDB.h"
#include "Options.h"
#include "PixilDT.h"
#include "SchedulerDB.h"
#include "ToDoListDB.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 500
#define DLG_INDENT 150
#define DLG_WIDTH  550


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
FindDlg::FindDlg(Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Search Database"))
{
    m_pInput = new Fl_Input(DLG_INDENT,
			    DLG_BORDER,
			    w() - DLG_INDENT - DLG_BUTTON_WIDTH -
			    2 * DLG_BORDER, DLG_INPUT_HEIGHT,
			    _("Search For:"));
    m_pInput->value(Options::GetSearchString().c_str());

    // Create the check buttons
    m_pRadioButton[0] = new Fl_Check_Button(DLG_INDENT,
					    2 * DLG_BORDER + DLG_INPUT_HEIGHT,
					    DLG_RADIO_SIZE,
					    DLG_INPUT_HEIGHT,
					    _("Match Whole Word"));
    m_pRadioButton[0]->down_box(FL_ROUND_DOWN_BOX);
    m_pRadioButton[1] = new Fl_Check_Button(DLG_INDENT,
					    3 * DLG_BORDER +
					    2 * DLG_INPUT_HEIGHT,
					    DLG_RADIO_SIZE, DLG_INPUT_HEIGHT,
					    _("Match Case"));
    m_pRadioButton[1]->down_box(FL_ROUND_DOWN_BOX);

    // Create the dialog buttons
    m_pSearchButton =
	new Fl_Return_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER, DLG_BORDER,
			     DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT,
			     _("&Search"));
    m_pSearchButton->callback(OnSearchButton);
    m_pGoToButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Go To"));
    m_pCancelButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				    3 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  4 * DLG_BORDER + 3 * DLG_BUTTON_HEIGHT,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);

    // Create the results table
    m_pResults = new FindList(DLG_BORDER,
			      4 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT,
			      w() - DLG_BUTTON_WIDTH - 3 * DLG_BORDER,
			      h() - 5 * DLG_BORDER - 3 * DLG_INPUT_HEIGHT);

    // Finish this dialog
    end();

    // The DoModal method will show this dialog
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
FindDlg::~FindDlg()
{
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
FindDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pGoToButton, m_pCancelButton);

    // Was the GoTo button clicked
    if (nReturn == 1) {
	// Yes, set to show this item
	m_nRecno = m_vFindRow[m_pResults->row()].m_nPhysicalRow;
	m_nType = m_vFindRow[m_pResults->row()].m_nType;
	m_nAction = ActionGoTo;
    } else {
	m_nAction = ActionCancel;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Get an Address Book key.                                     //
//--------------------------------------------------------------//
string
FindDlg::GetAddressKey(NxDbAccess * pDB, int nRow)
{
    string strFirstName = ((AddressBookDB *) pDB)->GetFirstName(nRow);
    string strLastName = ((AddressBookDB *) pDB)->GetLastName(nRow);
    string strReturn;

    if (strLastName.length() > 0 && strFirstName.length() > 0) {
	strReturn = strLastName;
	strReturn += ", ";
	strReturn += strFirstName;
    } else if (strLastName.length() > 0) {
	strReturn = strLastName;
    } else if (strFirstName.length() > 0) {
	strReturn = strFirstName;
    } else {
	strReturn = _("(unnamed)");
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Get a Notes key.                                             //
//--------------------------------------------------------------//
string
FindDlg::GetNoteKey(NxDbAccess * pDB, int nRow)
{
    return (((NoteDB *) pDB)->GetTitle(nRow));
}


//--------------------------------------------------------------//
// Get a Scheduler item key.                                    //
//--------------------------------------------------------------//
string
FindDlg::GetSchedulerKey(NxDbAccess * pDB, int nRow)
{
    return (((SchedulerDB *) pDB)->GetDescription(nRow));
}


//--------------------------------------------------------------//
// Get a ToDo List item key.                                    //
//--------------------------------------------------------------//
string
FindDlg::GetToDoKey(NxDbAccess * pDB, int nRow)
{
    return (((ToDoListDB *) pDB)->GetTitle(nRow));
}


//--------------------------------------------------------------//
// Notification from a child widget.                            //
//--------------------------------------------------------------//
void
FindDlg::Notify(PixilDTMessage nMessage, int nInfo)
{
    switch (nMessage) {
    case FIND_ITEM_REQUESTED:	// An item was double clicked in the list
	m_pGoToButton->do_callback();	// Simulate action on the GoTo button to end the form
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
FindDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_FIND_DLG);
}


//--------------------------------------------------------------//
// Search button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
FindDlg::OnSearchButton(Fl_Widget * pWidget, void *pUserData)
{
    FindDlg *pThis = reinterpret_cast < FindDlg * >(pWidget->parent());

    // There must be a search string
    if (strlen(pThis->m_pInput->value()) == 0) {
	fl_alert(_("The \"Search For\" string must be entered."));
    } else {
	// Have a search string, continue

	// Save the search string in the INI file
	Options::SetSearchString(pThis->m_pInput->value());

	// Clear the old search results
	pThis->m_vFindRow.clear();

	// Search each of the four major data bases
	pThis->Search(AddressBookDB::GetAddressBookDB(), ADDRESS_BOOK_GOTO,
		      GetAddressKey);
	pThis->Search(NoteDB::GetNoteDB(), NOTES_GOTO, GetNoteKey);
	pThis->Search(SchedulerDB::GetSchedulerDB(), SCHEDULER_GOTO,
		      GetSchedulerKey);
	pThis->Search(ToDoListDB::GetToDoListDB(), TODO_LIST_GOTO,
		      GetToDoKey);

	// Redraw the results
	pThis->m_pResults->rows(pThis->m_vFindRow.size());
	pThis->m_pResults->redraw();
    }
}


//--------------------------------------------------------------//
// Search a data base for a character string.                   //
//--------------------------------------------------------------//
void
FindDlg::Search(NxDbAccess * pDB,
		PixilDTMessage nType,
		string(*pfnGetKeyValue) (NxDbAccess *, int))
{
    FindRow findrow;
    int nMax = pDB->NumRecs();
    int nRow;

    // Search all character strings in this data base for the key
    for (nRow = 0; nRow < nMax; ++nRow) {
	if (pDB->Search(nRow,
			m_pInput->value(),
			(m_pRadioButton[0]->value() != 0),
			(m_pRadioButton[1]->value() != 0)) == true) {
	    findrow.m_nPhysicalRow = pDB->GetRecordNumber(nRow);
	    findrow.m_nType = nType;
	    findrow.m_strKeyValue = (*pfnGetKeyValue) (pDB, nRow);
	    m_vFindRow.push_back(findrow);
	}
    }
}
