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
// Category Editor List widget.                                 //
//--------------------------------------------------------------//
#include <FL/fl_draw.H>
#include <FL/forms.H>

#include "CategoryEditor.h"
#include "CategoryEditorList.h"
#include "FLTKUtil.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
CategoryEditorList::CategoryEditorList(int nX,	// Constructor
				       int nY,
				       int nWidth,
				       int nHeight, CategoryDB * pDB)
    :
Flv_List(nX, nY, nWidth, nHeight)
{
    Fl_Color cDeadSpaceColor = FL_WHITE;

    m_pDB = pDB;
    m_nCurrentRow = -1;
    selection_color(FL_SELECTION_COLOR);
    color(FL_WHITE);
    dead_space_color(cDeadSpaceColor);
    callback(Callback);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
CategoryEditorList::~CategoryEditorList()
{
}


//--------------------------------------------------------------//
// FLTK callback                                                //
//--------------------------------------------------------------//
void
CategoryEditorList::Callback(Fl_Widget * pWidget, void *pUserData)
{
    CategoryEditorList *pThis =
	dynamic_cast < CategoryEditorList * >(pWidget);

    // Check if the selected row number changed
    if (pThis->row() >= 0 && pThis->row() != pThis->m_nCurrentRow) {
	CategoryEditor::ButtonStatus nStatus;

	// The selection has changed
	pThis->m_nCurrentRow = pThis->row();

	// Is it the "unfiled" selection ?
	if (pThis->m_nCurrentRow == 0) {
	    // Disable the delete button
	    nStatus = CategoryEditor::Disable;
	} else {
	    // Enable the delete button
	    nStatus = CategoryEditor::Enable;
	}
	dynamic_cast < CategoryEditor * >(pThis->parent())->Notify(nStatus);
    }
}


//--------------------------------------------------------------//
// Draw a row - virtual callback, reason why this class exists  //
// as a distinct class...                                       //
//--------------------------------------------------------------//
void
CategoryEditorList::draw_row(int nOffset,
			     int &nX,
			     int &nY, int &nWidth, int &nHeight, int nRow)
{
    Flv_Style s;

    Flv_List::draw_row(nOffset, nX, nY, nWidth, nHeight, nRow);
    get_style(s, nRow);

    // Change the foreground color based on row selecton
    s.foreground((Fl_Color) (nRow == row()? FL_WHITE : FL_BLACK));
    fl_color(s.foreground());

    // Now draw the text
    fl_draw(m_pDB->GetStringValue(nRow, CAT).c_str(),
	    nX - nOffset, nY, nWidth, nHeight, s.align());
}


//--------------------------------------------------------------//
// Handle events.                                               //
//--------------------------------------------------------------//
int
CategoryEditorList::handle(int nEvent)
{
    static const Fl_Menu_Item menuPopup[] = {
	{N_("Delete ")},
	{N_("Rename "), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Category")},
	{NULL},
    };
    int nReturn = Flv_List::handle(nEvent);
    int nSelection;

    // Get right mouse button events
    switch (nEvent) {
    case FL_PUSH:
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
	    nReturn = 1;	// Get the FL_RELEASE event as well
	}
	break;

    case FL_RELEASE:
	int nRow = get_row(Fl::event_x(), Fl::event_y());
	string strDelete;
	string strRename;

	if (Fl::event_button() == FL_RIGHT_MOUSE) {
	    // Show a menu based on the position of the mouse
	    if (nRow >= 1) {
		// Not on a title and not on the first ("unfiled") category
		// Show all selections on the pop-up menu
		nSelection =
		    DoPopupMenu(menuPopup, Fl::event_x(), Fl::event_y());
	    } else {
		// Show a menu with only a "New Category" selection
		nSelection =
		    DoPopupMenu(&menuPopup[2], Fl::event_x(), Fl::event_y());
		if (nSelection >= 0) {
		    // Correct for the two missing selections in the menu
		    nSelection += 2;
		}
	    }

	    // Perform the requested action
	    if (nSelection >= 0) {
		int nRow = get_row(Fl::event_x(), Fl::event_y());
		switch (nSelection) {
		case 0:	// Delete this category
		    ((CategoryEditor *) parent())->DeleteCategory(nRow);
		    break;

		case 1:	// Rename this category
		    ((CategoryEditor *) parent())->RenameCategory(nRow);
		    break;

		case 2:	// Insert a new category
		    ((CategoryEditor *) parent())->NewCategory();
		}
	    }
	    // Eat this event anyway
	    nReturn = 1;
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Refresh this list from the data base                         //
//--------------------------------------------------------------//
void
CategoryEditorList::Refresh()
{
    // Sort the database by key
    m_pDB->Sort(CATID);

    // Cause the list to be redrawn
    rows(m_pDB->NumUndeletedRecs());
    redraw();
}
