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
// Class for the Notes List.                                    //
//--------------------------------------------------------------//
#include "config.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "NoteList.h"
#include "Notes.h"
#include "NotesCategoryDB.h"
#include "PixilDT.h"
#include "Printer.h"

#include "VCMemoryLeak.h"

#ifdef WIN32
#define strcasecmp stricmp
#endif


//--------------------------------------------------------------//
// Icon column indicators                                       //
//--------------------------------------------------------------//
const bool
    NoteList::m_bIconColumns[2] = {
	true,
false };


//--------------------------------------------------------------//
// Category currently being filtered for                        //
//--------------------------------------------------------------//
int
    NoteList::m_nSortCategory;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NoteList::NoteList(int nX, int nY, int nWidth, int nHeight)
    :
TableBase(nX, nY, nWidth, nHeight)
{
    m_nCategory = -1;		// Select all categories

    // Create the notes icon
    m_pNotesIcon = Images::GetNotesIcon();

    // Finish TableBase construction
    PostConstructor(NoteDB::GetNoteDB()->NumUndeletedRecs(), 2,	// 2 columns
		    m_bIconColumns);

    // Display the details for the first line if it exists
    if (rows() > 0) {
	row(0);
	Refresh();
	SelectRow(0);
    }
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NoteList::~NoteList()
{
    delete m_pNotesIcon;
}


//--------------------------------------------------------------//
// Recalculate column widths                                    //
//--------------------------------------------------------------//
void
NoteList::CalculateColumnWidths(int nWidth)
{
    m_nColWidth[0] = 12;	// An icon
    m_nColWidth[1] = nWidth - 12;	// All but one icons
}


//--------------------------------------------------------------//
// Process a double click over a row (do nothing)               //
//--------------------------------------------------------------//
void
NoteList::DoubleClick(int nRow, int nCol)
{
}


//--------------------------------------------------------------//
// Filter the displayed rows by category (-1 = all categories)  //
//--------------------------------------------------------------//
void
NoteList::Filter(int nCategory)
{
    // Convert the category row to a category key
    if (nCategory != -1) {
	nCategory =
	    NotesCategoryDB::GetNotesCategoryDB()->GetCategoryID(nCategory);
    }
    // Sort the rows in the database by category
    m_nCategory = nCategory;

    // Go refresh (and actually do the sort)
    Refresh();
}


//--------------------------------------------------------------//
// Get the icon for an icon column                              //
//--------------------------------------------------------------//
Fl_Pixmap *
NoteList::GetIconValue(int nRow, int nCol)
{
    Fl_Pixmap *pPixmap;

    switch (nCol) {
    case 0:			// Always gets a Notes icon
	pPixmap = m_pNotesIcon;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Should not get here for some other column
#endif
	;
    }
    return (pPixmap);
}


//--------------------------------------------------------------//
// Get the value of a column.                                   //
//--------------------------------------------------------------//
string
NoteList::GetStringValue(int nRow, int nCol)
{
    string strData;

    switch (nCol) {
    case 1:			// First part of the notes text
	strData =::WrapText(NoteDB::GetNoteDB()->GetTitle(nRow).c_str(),
			    m_nColWidth[1] - DLG_BORDER, this);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Should not get here for some other column
#endif
	;
    }
    return (strData);
}

//--------------------------------------------------------------//
// Process a left mouse click over an icon, same as over        //
// anywhere else.                                               //
//--------------------------------------------------------------//
void
NoteList::IconClick(int nRow, int nCol)
{
    LeftClick(nRow, nCol);
}


//--------------------------------------------------------------//
// Process a left mouse click over a non-icon column.           //
//--------------------------------------------------------------//
void
NoteList::LeftClick(int nRow, int nCol)
{
    SelectRow(nRow);
}


//--------------------------------------------------------------//
// Process a message from the parent widget                     //
//--------------------------------------------------------------//
int
NoteList::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case NOTES_CHANGED:
	Refresh();
	break;

    case NOTES_GOTO:		// Find dialog requested a note
	{
	    int nCount;
	    int nRow;
	    NoteDB *pNoteDB = NoteDB::GetNoteDB();

	    // Is this item visible ?
	    nRow = pNoteDB->FindPhysicalRecord(nInfo);
	    nCount = pNoteDB->NumRecsByKey(NOTE_CAT, m_nCategory);
	    if (nRow >= nCount) {
		// Show all categories so this row can be seen
		m_nCategory = -1;

		// Refresh for this row
		Refresh(pNoteDB->GetIndex(nRow));
	    } else {
		// The row is visible, just select it
		SelectRow(nRow);
	    }
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


//--------------------------------------------------------------//
// Print the notes.                                             //
//--------------------------------------------------------------//
void
NoteList::Print()
{
    int nBorder = INCH / 2;	// 1/2 inch border
    int nCopy;
    const int nFontSize = 10;
    int nRow;
    int nRows;
    Note *pNote;
    NoteDB *pNoteDB = NoteDB::GetNoteDB();
    Printer printer;

    // Save any changes
    ((Notes *) parent())->SaveDetailChanges();

    // Open the printer
    if (printer.Open(_("Notes")) == true) {
	// Set an hourglass cursor
	PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_WAIT);

	// Print once for eqch requested copy
	for (nCopy = 0; nCopy < printer.GetCopies(); ++nCopy) {
	    // Reset the page number
	    printer.ResetPageNumber();

	    // Go to two column mode
	    printer.SetSerifFont(nFontSize);
	    printer.SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				     nBorder, _("Notes"), NULL, 184);

	    // Print each visible entry
	    if (m_nCategory == -1) {
		// Show all rows
		nRows = pNoteDB->NumUndeletedRecs();
	    } else {
		// Filter by category
		nRows = pNoteDB->NumRecsByKey(NOTE_INDEX, m_nCategory);
	    }

	    // Print each row
	    for (nRow = 0; nRow < nRows; ++nRow) {
		// Output a blank line if needed
		if (nRow != 0) {
		    printer.ColumnNewLine();
		}
		// Output the note title
		printer.SetBoldSerifFont(nFontSize);
		printer.ColumnShow(pNoteDB->GetTitle(nRow).c_str(), 0, 0);
		printer.SetSerifFont(nFontSize);

		// Output the category
		printer.ColumnShowNoAdvance(_("Category:"), INCH / 4);
		printer.ColumnShow(pNoteDB->GetCategoryName(nRow).c_str(),
				   (9 * INCH) / 8, 0);

		// Output the note
		pNote = pNoteDB->GetNote(nRow);
		if (pNote->GetText().length() > 0) {
		    printer.ColumnShow(pNote->GetText().c_str(), INCH / 4, 0);
		}
		delete pNote;
	    }

	    // End the page
	    printer.EndTwoColumnMode();
	}

	// All done, close the printer
	printer.Close();

	// Reset the cursor
	PixilDT::GetApp()->GetMainWindow()->ResetCursor();
    }
}


//--------------------------------------------------------------//
// Refresh this list                                            //
//--------------------------------------------------------------//
void
NoteList::Refresh(int nID)
{
    int nRow = row();
    int nRows;
    NoteDB *pNoteDB = NoteDB::GetNoteDB();


    // Get the currently selected row
    if (nID < 0) {
	if (nRow >= 0 && pNoteDB->NumUndeletedRecs() > 0) {
	    nID = pNoteDB->GetIndex(nRow);
	} else {
	    // No row currently selected
	    nID = -1;
	}
    }
    // Re-sort the data base
    if (m_nCategory == -1) {
	// Sort for all categories
	pNoteDB->Sort(SortCompare);
	nRows = pNoteDB->NumUndeletedRecs();
    } else {
	// Filter by category
	m_nSortCategory = m_nCategory;
	pNoteDB->Sort(SortCategory);
	nRows = pNoteDB->NumRecsByKey(NOTE_CAT, m_nCategory);
    }
    row(-1);			// Workaround for top_row update
    rows(nRows);

#if 0				// These buttons are non-functional so far (08/06/01)
    // Enable/Disable the Cut/Copy toolbar items
    PixilDT::GetApp()->GetMainWindow()->Notify(row >=
					       0 ? ENABLE_TOOLBAR_BUTTON :
					       DISABLE_TOOLBAR_BUTTON,
					       EDITCUT_ICON);
    PixilDT::GetApp()->GetMainWindow()->Notify(row >=
					       0 ? ENABLE_TOOLBAR_BUTTON :
					       DISABLE_TOOLBAR_BUTTON,
					       EDITCOPY_ICON);
#endif

    // Reselect the same row if possible
    nRow = pNoteDB->FindRow(NOTE_INDEX, nID);
    if (nRow >= 0) {
	// See if visible based on selected category
	if (m_nCategory != -1) {
	    if (nRow >= pNoteDB->NumRecsByKey(NOTE_CAT, m_nCategory)) {
		// Not visible, reselect row 0
		nRow = 0;
	    }
	}
    } else {
	nRow = 0;
    }
    row(nRow);

    // Cause the list to be redrawn
    redraw();
    SelectRow(nRows > 0 ? row() : -1);
}


//--------------------------------------------------------------//
// Process a right mouse click anywhere.                        //
//--------------------------------------------------------------//
void
NoteList::RightClick(int nRow, int nCol)
{
    static const Fl_Menu_Item menuPopup1[] = {
	{N_("Delete Note"),},
	{N_("Copy Note"),},
	{N_("Cut Note"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Note"),},
	{N_("Paste Note"),},
	{NULL},
    };
    static const Fl_Menu_Item menuPopup2[] = {
	{N_("Delete Note"),},
	{N_("Copy Note"),},
	{N_("Cut Note"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Note"),},
	{NULL},
    };
    const Fl_Menu_Item *pmenuPopup;
    int nSelection;

    // Only ask whether to save the pending detail changes if not clicking on a row
    if (nRow < 0) {
	((Notes *) parent())->SaveDetailChanges();
    }
    // Display the details for this line to the right
    SelectRow(nRow);

    // Display the popup menu
    pmenuPopup = ((Notes *) parent())->CanPaste()
	? menuPopup1 : menuPopup2;
    nSelection = DoPopupMenu((nRow >= 0 ? pmenuPopup : &pmenuPopup[3]),
			     Fl::event_x(), Fl::event_y());

    if (nSelection >= 0) {
	// Correct for no delete row option
	nSelection += (nRow < 0 ? 3 : 0);

	switch (nSelection) {
	case 0:		// Delete this row
	    ((Notes *) parent())->Delete(nRow);
	    break;

	case 1:		// Copy this row
	    ((Notes *) parent())->Copy(nRow);
	    break;

	case 2:		// Cut this row
	    ((Notes *) parent())->Cut(nRow);
	    break;

	case 3:		// Insert a new row
	    ((Notes *) parent())->EditNew();
	    break;

	case 4:		// Paste a row
	    ((Notes *) parent())->Paste();
	}
    }
}


//--------------------------------------------------------------//
// Select a row in the list and inform the parent of the        //
// selection                                                    //
//--------------------------------------------------------------//
void
NoteList::SelectRow(int nRow)
{
    if (nRow >= 0) {
	row(nRow);
	((Notes *) parent())->DisplayRow(nRow);
    }
}


//--------------------------------------------------------------//
// Compare by note title and filter by category for sorting.    //
//--------------------------------------------------------------//
bool
NoteList::SortCategory(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    // Is the first row the correct category
    if (pRow1->GetIntValue(NOTE_CAT) == m_nSortCategory) {
	// Is the second row the correct category
	if (pRow2->GetIntValue(NOTE_CAT) == m_nSortCategory) {
	    // Both good, sort like normal
	    bReturn = SortCompare(pRow1, pRow2);
	} else {
	    // First is good, second is not, say first is lower
	    bReturn = true;
	}
    } else {
	// Is the second row the correct category
	if (pRow2->GetIntValue(NOTE_CAT) == m_nSortCategory) {
	    // First is not good, second is good, say first is not lower
	    bReturn = false;
	} else {
	    // Both not good, sort like normal
	    bReturn = SortCompare(pRow1, pRow2);
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare by note title for sorting.                           //
//--------------------------------------------------------------//
bool
NoteList::SortCompare(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;
    string strString1 = pRow1->GetStringValue(NOTE_DESC);
    string strString2 = pRow2->GetStringValue(NOTE_DESC);

    if (strcasecmp(strString1.c_str(), strString2.c_str()) < 0) {
	bReturn = true;
    } else {
	bReturn = false;
    }
    return (bReturn);
}
