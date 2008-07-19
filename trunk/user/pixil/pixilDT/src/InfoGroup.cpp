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
// Center information for the main window.                      //
//--------------------------------------------------------------//
#include <cstdio>
#include "InfoGroup.h"
#include "Messages.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
InfoGroup::InfoGroup(int x, int y, int width, int height)
    :
Fl_Group(x, y, width, height)
{
    // Create each of the 4 groups to be displayed in this window
    m_pAddressBook = new AddressBook(this);
    m_pNotes = new Notes(this);
    m_pScheduler = new Scheduler(this);
    m_pToDoList = new ToDoList(this);
    end();

    // Set up a lowered box
    box(FL_DOWN_BOX);

    // Set that this is resizable
    resizable(this);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
InfoGroup::~InfoGroup()
{
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
InfoGroup::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:
	m_pAddressBook->Message(nMessage, 0);
	m_pScheduler->Message(nMessage, 0);
	break;

    case NOTES_CHANGED:
	m_pNotes->Message(nMessage, 0);
	break;

    case SCHEDULER_CHANGED:
	m_pScheduler->Message(nMessage, 0);
	break;

    case TODO_LIST_CHANGED:
	m_pScheduler->Message(nMessage, 0);
	m_pToDoList->Message(nMessage, 0);
	break;

    case ADDRESS_BOOK_REQUESTED:
    case NOTES_REQUESTED:
    case SCHEDULER_REQUESTED:
    case TODO_LIST_REQUESTED:
	m_pAddressBook->Message(nMessage, 0);
	m_pNotes->Message(nMessage, 0);
	m_pScheduler->Message(nMessage, 0);
	m_pToDoList->Message(nMessage, 0);
	ShowInfo(nMessage);
	break;

    case ADDRESS_BOOK_CATEGORY_ADDED:
    case ADDRESS_BOOK_CATEGORY_DELETED:
    case ADDRESS_BOOK_CATEGORY_RENAMED:
    case ADDRESS_BOOK_DELETE:
    case ADDRESS_BOOK_GOTO:
    case ADDRESS_BOOK_NEW:
    case ADDRESS_BOOK_PRINT:
    case SHOW_LIST_BY:
	m_pAddressBook->Message(nMessage, nInfo);
	break;

    case NOTES_CATEGORY_ADDED:
    case NOTES_CATEGORY_DELETED:
    case NOTES_CATEGORY_RENAMED:
    case NOTES_DELETE:
    case NOTES_GOTO:
    case NOTES_NEW:
    case NOTES_PRINT:
	m_pNotes->Message(nMessage, nInfo);
	break;

    case BEGIN_WEEK_CHANGED:	// The beginning day of week has been changed
    case SCHEDULER_CATEGORY_ADDED:
    case SCHEDULER_CATEGORY_DELETED:
    case SCHEDULER_CATEGORY_RENAMED:
    case SCHEDULER_DELETE:
    case SCHEDULER_GOTO:
    case SCHEDULER_NEW:
    case SCHEDULER_PRINT:
	m_pScheduler->Message(nMessage, nInfo);
	break;

    case SHOW_TODO_OPTIONS:
    case TODO_LIST_CATEGORY_ADDED:
    case TODO_LIST_CATEGORY_DELETED:
    case TODO_LIST_CATEGORY_RENAMED:
    case TODO_LIST_DELETE:
    case TODO_LIST_GOTO:
    case TODO_LIST_NEW:
    case TODO_LIST_PRINT:
	m_pToDoList->Message(nMessage, nInfo);
	break;

    case SELECTION_CHANGING:	// 0 return means to refuse the change
	nReturn = m_pAddressBook->Message(nMessage, nInfo);
	nReturn &= m_pNotes->Message(nMessage, nInfo);
	nReturn &= m_pScheduler->Message(nMessage, nInfo);
	nReturn &= m_pToDoList->Message(nMessage, nInfo);
	break;

    case APPLICATION_CLOSING:
	m_pAddressBook->Message(nMessage, 0);
	m_pNotes->Message(nMessage, 0);
	m_pScheduler->Message(nMessage, 0);
	m_pToDoList->Message(nMessage, 0);
	break;

    case EDIT_COPY:		// Pass these on to only the active page
    case EDIT_CUT:
    case EDIT_PASTE:
    case EDIT_UNDO:
    case EDIT_COPY_AVAILABLE:
    case EDIT_CUT_AVAILABLE:
    case EDIT_PASTE_AVAILABLE:
    case EDIT_UNDO_AVAILABLE:
	switch (m_nCurrentPage) {
	case ADDRESS_BOOK_REQUESTED:
	    nReturn = m_pAddressBook->Message(nMessage, nInfo);
	    break;

	case NOTES_REQUESTED:
	    nReturn = m_pNotes->Message(nMessage, nInfo);
	    break;

	case SCHEDULER_REQUESTED:
	    nReturn = m_pScheduler->Message(nMessage, nInfo);
	    break;

	case TODO_LIST_REQUESTED:
	    nReturn = m_pToDoList->Message(nMessage, nInfo);
	}
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }

    return (nReturn);
}

void
InfoGroup::Refresh()
{
    NxDbAccess::RefreshAll();
    m_pToDoList->Refresh();
    m_pAddressBook->Refresh();
    m_pScheduler->Refresh();
    m_pNotes->Refresh();
}

//--------------------------------------------------------------//
// Show a particular set of information                         //
//--------------------------------------------------------------//
void
InfoGroup::ShowInfo(PixilDTMessage nMessage)
{
    // Hide or show the correct group
    if (nMessage == ADDRESS_BOOK_REQUESTED) {
	m_pAddressBook->show();
    } else {
	m_pAddressBook->hide();
    }

    if (nMessage == NOTES_REQUESTED) {
	m_pNotes->show();
    } else {
	m_pNotes->hide();
    }

    if (nMessage == SCHEDULER_REQUESTED) {
	m_pScheduler->show();
    } else {
	m_pScheduler->hide();
    }

    if (nMessage == TODO_LIST_REQUESTED) {
	m_pToDoList->show();
    } else {
	m_pToDoList->hide();
    }

    m_nCurrentPage = nMessage;
}
