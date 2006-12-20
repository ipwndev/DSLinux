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
// Find Dialog List of found items.                             //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/Fl_Menu_Item.H>
#include "FindDlg.h"
#include "FindList.h"
#include "FLTKUtil.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
FindList::FindList(int nX, int nY, int nWidth, int nHeight)
    :
TableBase(nX, nY, nWidth, nHeight)
{
    bool bIconColumns[2] = { false, false };

    PostConstructor(0, 2, bIconColumns);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
FindList::~FindList()
{
}


//--------------------------------------------------------------//
// Calculate column widths after a resize.  The TableBase class //
// will already have set the m_nColWidth array to zeroes.       //
//--------------------------------------------------------------//
void
FindList::CalculateColumnWidths(int nWidth)
{
    m_nColWidth[0] = 20 * nWidth / 100;	// 20% of it
    m_nColWidth[1] = nWidth - m_nColWidth[0];	// The rest
}


//--------------------------------------------------------------//
// Process a double click over a row, Go To that item.          //
//--------------------------------------------------------------//
void
FindList::DoubleClick(int nRow, int nCol)
{
    ((FindDlg *) parent())->Notify(FIND_ITEM_REQUESTED, nRow);
}


//--------------------------------------------------------------//
// Get the number of rows to be displayed                       //
//--------------------------------------------------------------//
int
FindList::GetRowCount()
{
    return (((FindDlg *) parent())->GetFindRows());
}


//--------------------------------------------------------------//
// Get the string value of a column.                            //
//--------------------------------------------------------------//
string
FindList::GetStringValue(int nRow, int nCol)
{
    string strReturn;

    switch (nCol) {
    case 0:			// First column
	switch (((FindDlg *) parent())->GetFindType(nRow)) {
	case ADDRESS_BOOK_GOTO:	// Address Book item
	    strReturn = _("Address");
	    break;

	case NOTES_GOTO:	// Notes item
	    strReturn = _("Note");
	    break;

	case SCHEDULER_GOTO:	// Scheduler/appointment item
	    strReturn = _("Appointment");
	    break;

	case TODO_LIST_GOTO:	// ToDo List item
	    strReturn = _("ToDo List");
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Unknown type
#endif
	    ;
	}
	break;

    case 1:			// Second columnKey of the item
	strReturn = ((FindDlg *) parent())->GetFindKey(nRow);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Should not get here for some other column
#endif
	;
    }

    return (strReturn);
}


//--------------------------------------------------------------//
// Process a right mouse click anywhere                         //
//--------------------------------------------------------------//
void
FindList::RightClick(int nRow, int nCol)
{
    static const Fl_Menu_Item menuPopup[] = {
	{N_("Go To this Item")},
	{NULL},
    };
    int nSelection;

    // Display the popup menu
    nSelection = DoPopupMenu(menuPopup, Fl::event_x(), Fl::event_y());

    if (nSelection >= 0) {
	// Only one selection
	((FindDlg *) parent())->Notify(FIND_ITEM_REQUESTED, nRow);
    }
}
