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
// Class for the Address Book List.                             //
//--------------------------------------------------------------//
#include <FL/fl_draw.H>
#include "AddressBook.h"
#include "AddressBookCategoryDB.h"
#include "AddressBookList.h"
#include "AddressBookDB.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "Messages.h"
#include "NoteEditorDlg.h"
#include "PixilDT.h"
#include "Printer.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
// MS/Windows mis-named it
#define strcasecmp stricmp
#endif


//--------------------------------------------------------------//
// Icon column indications                                      //
//--------------------------------------------------------------//
const bool
    AddressBookList::m_bLargeIconColumns[ADDRESSBOOK_LARGE_COLUMNS] = {
	false,
	false,
	false,
	true,
true };

const bool
    AddressBookList::m_bSmallIconColumns[ADDRESSBOOK_SMALL_COLUMNS] = {
	false,
	true,
true };


//--------------------------------------------------------------//
// Category being filtered on                                   //
//--------------------------------------------------------------//
char
    AddressBookList::m_szCategory[16];


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
AddressBookList::AddressBookList(int nX,
				 int nY, int nWidth, int nHeight, bool bSmall)
    :
TableBase(nX, nY, nWidth, nHeight)
{
    m_bByLastName = true;
    m_bSmall = bSmall;
    m_nCategory = -1;		// Select all categories

    // Set no double click processing for a short display
    m_pfnDoubleClick = NULL;

    // Create the notes icon
    m_pNotesIcon = Images::GetNotesIcon();

    // Finish TableBase construction
    PostConstructor(AddressBookDB::GetAddressBookDB()->NumUndeletedRecs(),
		    bSmall ? ADDRESSBOOK_SMALL_COLUMNS :
		    ADDRESSBOOK_LARGE_COLUMNS,
		    bSmall ? m_bSmallIconColumns : m_bLargeIconColumns);

    // Display the details for the first line if it exists
    if (rows() > 0) {
	row(0);
	Refresh();
    }
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
AddressBookList::~AddressBookList()
{
    delete m_pNotesIcon;
}


//--------------------------------------------------------------//
// Calculate column widths after a resize.  The TableBase class //
// will already have set the m_nColWidth array to zeroes.       //
//--------------------------------------------------------------//
void
AddressBookList::CalculateColumnWidths(int nWidth)
{
    if (m_bSmall == true) {
	// Small display
	m_nColWidth[0] = nWidth - 12 - 12;	// All but two icons
	m_nColWidth[1] = 12;	// An icon
	m_nColWidth[2] = 12;	// An icon
    } else {
	// Large display
	m_nColWidth[0] = 55 * (nWidth - 12 - 12) / 100;	// 55% of all but two icons
	m_nColWidth[1] = 17 * (nWidth - 12 - 12) / 100;	// 17% of all but two icons
	m_nColWidth[2] = (nWidth - 12 - 12) - m_nColWidth[0] - m_nColWidth[1];	// The rest
	m_nColWidth[3] = 12;	// An icon
	m_nColWidth[4] = 12;	// An icon
    }
}


//--------------------------------------------------------------//
// Process a double click over a row.                           //
//--------------------------------------------------------------//
void
AddressBookList::DoubleClick(int nRow, int nCol)
{
    int nRow2 = AddressBookDB::GetAddressBookDB()->FindRealRow(nRow, m_vpRow);

    if (m_bSmall == false) {
	// Only process if not a small display
	((AddressBook *) parent())->Edit(nRow2);
    } else {
	// Perform custom processing for a small display
	if (m_pfnDoubleClick != NULL) {

	    (*m_pfnDoubleClick) (nRow2, nCol);
	}
    }
}


//--------------------------------------------------------------//
// Filter this list by category.                                //
//--------------------------------------------------------------//
void
AddressBookList::Filter(int nCategory)
{
    // Convert the category row to a category key
    if (nCategory != -1) {
	nCategory =
	    AddressBookCategoryDB::GetAddressBookCategoryDB()->
	    GetCategoryID(nCategory);
    }
    // Sort the rows in the database by category
    m_nCategory = nCategory;

    // Go refresh (and actually do the sort)
    Refresh();
}


//--------------------------------------------------------------//
// Get the icon for a column.  The icon is returned as an       //
// Fl_Pixmap pointer.                                           //
//--------------------------------------------------------------//
Fl_Pixmap *
AddressBookList::GetIconValue(int nRow, int nCol)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    Fl_Pixmap *pImage = NULL;

    if (m_bSmall == false) {
	// Large 5 column display
	switch (nCol) {
	case 3:		// Private icon - always blank for now
	    break;

	case 4:		// Notes icon - show if notes exist
	    nRow = pAddressBookDB->FindRealRow(nRow, m_vpRow);
	    if (pAddressBookDB->GetNoteFile(nRow).length() > 0) {
		// Get the notes icon                           pImage=m_pNotesIcon;

	    }
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Should not get here for some other column
#endif
	    ;
	}
    } else {
	// Smaller 3 column display
	switch (nCol) {
	case 1:		// Private icon - always blank for now
	    break;

	case 2:		// Notes icon - show if notes exist
	    nRow = pAddressBookDB->FindRealRow(nRow, m_vpRow);
	    if (pAddressBookDB->GetNoteFile(nRow).length() > 0) {
		// Get the notes icon
		pImage = m_pNotesIcon;
	    }
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Should not get here for some other column
#endif
	    ;
	}
    }

    return (pImage);
}


//--------------------------------------------------------------//
// Get the string value of a column.                            //
//--------------------------------------------------------------//
string
AddressBookList::GetStringValue(int nRow, int nCol)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    static const int nShowColumn[7] =
	{ AB_DEP1, AB_DEP2, AB_DEP3, AB_DEP4, AB_DEP5, AB_DEP6, AB_DEP7 };
    int nShow;
    string strReturn;

    // Translate the row number
    nRow = pAddressBookDB->FindRealRow(nRow, m_vpRow);

    // Get the data for the column
    switch (nCol) {
    case 0:			// First column
	if (m_bByLastName == true) {
	    // Set up the last name, first name string
	    strReturn = pAddressBookDB->GetLastName(nRow)
		+ ", " + pAddressBookDB->GetFirstName(nRow);
	} else {
	    // Set up by company name/last name
	    strReturn = pAddressBookDB->GetCompany(nRow)
		+ ", " + pAddressBookDB->GetLastName(nRow);
	}
	break;

    case 1:			// Second column, depends on the show flag
	// Fix for a bad show column
	nShow = pAddressBookDB->GetShowFlag(nRow);
	if (nShow < 0 || nShow > 6) {
	    nShow = 0;
	}
	// Get the column to be shown
	strReturn =
	    pAddressBookDB->GetInfoName(pAddressBookDB->
					GetInfoID(nRow, nShow));
	strReturn += ":";
	break;

    case 2:			// third column, depends on the show flag
	// Fix for a bad show column
	nShow = pAddressBookDB->GetShowFlag(nRow);
	if (nShow < 0 || nShow > 6) {
	    nShow = 0;
	}
	// Get the column to be shown
	strReturn += pAddressBookDB->GetStringValue(nRow, nShowColumn[nShow]);
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
// Process a click over an icon column                          //
//--------------------------------------------------------------//
void
AddressBookList::IconClick(int nRow, int nCol)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();

    // Translate the row number
    nRow = pAddressBookDB->FindRealRow(nRow, m_vpRow);

    // Display the details for this line to the right
    SelectRow(nRow);

    // Process if a click over the Notes icon column and the line has notes
    if (((m_bSmall == true && nCol == 2) || (m_bSmall == false && nCol == 4))
	&& pAddressBookDB->GetNoteFile(nRow).length() > 0) {
	Note *pNote = pAddressBookDB->GetNote(nRow);
	NoteEditorDlg *pDlg =
	    new NoteEditorDlg(pNote, PixilDT::GetApp()->GetMainWindow());

	if (pDlg->DoModal() == 1) {
	    // OK button ended the dialog, save the note changes
	    pAddressBookDB->SetNote(nRow, pDlg->GetNote());

	    // Notify everyone of the changes
	    PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED,
						       0);
	}
	// Clean up
	delete pNote;
	delete pDlg;
    }
}


//--------------------------------------------------------------//
// Process a left mouse click over a non-icon column.           //
//--------------------------------------------------------------//
void
AddressBookList::LeftClick(int nRow, int nCol)
{
    // Display the details for this line to the right
    SelectRow(AddressBookDB::GetAddressBookDB()->FindRealRow(nRow, m_vpRow));
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
AddressBookList::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:
	Refresh();
	break;

    case ADDRESS_BOOK_GOTO:
	{
	    int nCount;
	    int nLocalRow;
	    int nRealRow;
	    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();

	    // Is this item visible ?
	    nLocalRow = pAddressBookDB->FindPhysicalRecord(nInfo, m_vpRow);
	    nCount = pAddressBookDB->NumRecsByCategory(m_nCategory);
	    if (nLocalRow >= nCount) {
		// Show all categories so this row can be seen
		m_nCategory = -1;

		// Refresh for this row
		nRealRow = pAddressBookDB->FindPhysicalRecord(nInfo);
		Refresh(pAddressBookDB->GetRecno(nRealRow));
	    } else {
		// The row is visible, just select it
		nRealRow = pAddressBookDB->FindPhysicalRecord(nInfo);
		SelectRow(nRealRow);
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
// Print the Address Book as currently sorted.                  //
//--------------------------------------------------------------//
void
AddressBookList::Print()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    bool bLastWasBox = false;
    char szLastChar[2] = "0";
    int nBorder = INCH / 2;	// 1/2 inch border
    int nChar;
    int nCopy;
    const int nFontSize = 10;
    int nInfo;
    int nRow;
    int nRow2;
    int nRows;
    Note *pNote;
    Printer printer;
    string strData;
    string strInfo;

    // Open the printer
    if (printer.Open(_("AddressBook")) == true) {
	// Set an hourglass cursor
	PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_WAIT);

	// Print once for eqch requested copy
	for (nCopy = 0; nCopy < printer.GetCopies(); ++nCopy) {
	    // Reset the page number
	    printer.ResetPageNumber();

	    // Go to two column mode
	    printer.SetSerifFont(nFontSize);
	    printer.SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				     nBorder, _("Address Book"), NULL, 184);

	    // Print each visible entry
	    if (m_nCategory == -1) {
		// Show all rows
		nRows = pAddressBookDB->NumUndeletedRecs();
	    } else {
		// Get the selected category
		sprintf(m_szCategory, "%d", m_nCategory);

		// Filter by category
		nRows = pAddressBookDB->NumRecsByKey(AB_CAT, m_szCategory);
	    }

	    // Print each row
	    for (nRow = 0; nRow < nRows; ++nRow) {
		// Translate the row number
		nRow2 = pAddressBookDB->FindRealRow(nRow, m_vpRow);

		// Has the first character changed
		nChar = toupper(m_bByLastName == true
				? (pAddressBookDB->GetLastName(nRow2))[0]
				: (pAddressBookDB->GetCompany(nRow2))[0]);
		if (nRow == 0 || nChar != szLastChar[0]) {
		    // Yes, output a box for the character
		    szLastChar[0] = nChar;
		    printer.ColumnSpaceLine();
		    printer.ColumnSpaceLine();
		    printer.ColumnSpaceLine();
		    printer.ColumnBox(szLastChar, 15, 5 * INCH / 16, 184);
		    bLastWasBox = true;
		}
		// Output this person's data
		if (nRow != 0 && bLastWasBox == false) {
		    printer.ColumnNewLine();
		}
		bLastWasBox = false;

		// Output person or company name
		if (m_bByLastName == true) {
		    // Get the last name
		    strData = pAddressBookDB->GetFirstName(nRow2);
		    strData += ' ';
		    strData += pAddressBookDB->GetLastName(nRow2);
		} else {
		    // Get the company
		    strData = pAddressBookDB->GetCompany(nRow2);
		}

		// Output the identifier
		printer.SetBoldSerifFont(nFontSize);
		printer.ColumnShowNoAdvance(strData.c_str(), 0);
		printer.SetSerifFont(nFontSize);

		// Output the category
		printer.ColumnShowNoAdvanceRight(pAddressBookDB->
						 GetCategoryName(nRow).
						 c_str(), 0);
		printer.ColumnNewLine();

		// Output the title
		printer.ColumnShow(pAddressBookDB->GetTitle(nRow).c_str(),
				   INCH / 4, 0);

		// Output the company or person name
		if (m_bByLastName == true) {
		    // Get the company
		    strData = pAddressBookDB->GetCompany(nRow2);
		} else {
		    // Get the last name
		    strData = pAddressBookDB->GetFirstName(nRow2);
		    strData += ' ';
		    strData += pAddressBookDB->GetLastName(nRow2);
		}
		printer.ColumnShow(strData.c_str(), INCH / 4, 0);

		// Output each of the info fields
		for (nInfo = 0; nInfo < 7; ++nInfo) {
		    strInfo = pAddressBookDB->GetInfo(nRow2, nInfo);
		    if (strInfo.length() > 0) {
			strData = pAddressBookDB->GetInfoName(nRow2, nInfo);
			strData += ':';
			printer.SetBoldSerifFont(nFontSize);
			printer.ColumnShowNoAdvance(strData.c_str(),
						    INCH / 4);
			printer.SetSerifFont(nFontSize);
			printer.ColumnShow(strInfo.c_str(), (5 * INCH) / 4,
					   0);
		    }
		}

		// Output the address
		printer.ColumnShow(pAddressBookDB->GetAddress(nRow2).c_str(),
				   INCH / 4, 0);
		strData = pAddressBookDB->GetCity(nRow2);
		strData += ", ";
		strData += pAddressBookDB->GetRegion(nRow2);
		strData += ' ';
		strData += pAddressBookDB->GetPostalCode(nRow2);
		printer.ColumnShow(strData.c_str(), INCH / 4, 0);
		strData = pAddressBookDB->GetCountry(nRow2);
		if (strData.length() > 0) {
		    printer.ColumnShow(strData.c_str(), INCH / 4, 0);
		}
		// Output the custom fields
		for (nInfo = 0; nInfo < 4; ++nInfo) {
		    strInfo = pAddressBookDB->GetCustomField(nRow2, nInfo);
		    if (strInfo.length() > 0) {
			strData = pAddressBookDB->GetCustomFieldName(nInfo);
			strData += ':';
			printer.ColumnShowNoAdvance(strData.c_str(),
						    INCH / 4);
			printer.ColumnShow(strInfo.c_str(), (5 * INCH) / 4,
					   0);
		    }
		}

		// Output the note
		pNote = pAddressBookDB->GetNote(nRow2);
		if (pNote->GetText().length() > 0) {
		    printer.ColumnShowNoAdvance(_("Notes:"), INCH / 4);
		    printer.ColumnShow(pNote->GetText().c_str(),
				       (5 * INCH) / 4, 0);
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
// Refresh this list.                                           //
//--------------------------------------------------------------//

void
AddressBookList::Refresh(int nID)
{

    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    m_vpRow = pAddressBookDB->GetRows();

    int nLocalRow = row();
    int nRealRow;
    int nRows = pAddressBookDB->NumUndeletedRecs();

    // Get the currently selected row
    if (nID < 0) {
	if (nRows > 0 && nLocalRow >= 0 && m_vpRow.size() > 0) {
	    // Get the row's ID number
	    nID =
		pAddressBookDB->GetRecno(pAddressBookDB->
					 FindRealRow(nLocalRow, m_vpRow));
	} else {
	    // No row currently selected
	    nID = -1;
	}
    }
    // Get a copy of the rows
    m_vpRow = pAddressBookDB->GetRows();

    // Re-sort the data base
    if (m_nCategory == -1) {
	// Sort for all categories
	pAddressBookDB->Sort(m_bByLastName ==
			     true ? SortCompare1 : SortCompare2, m_vpRow);
	nRows = pAddressBookDB->NumUndeletedRecs();
    } else {
	// Get the selected category
	sprintf(m_szCategory, "%d", m_nCategory);

	// Filter by category
	pAddressBookDB->Sort(m_bByLastName ==
			     true ? SortCategory1 : SortCategory2, m_vpRow);
	nRows = pAddressBookDB->NumRecsByKey(AB_CAT, m_szCategory);
    }
    row(-1);			// Workaround for inadvertant top_row update
    rows(nRows);

    // Find the row to be selected
    if (nID >= 0) {
	// Get the row number for the selected ID
	nRealRow = pAddressBookDB->FindRecno(nID);
	if (nRealRow >= 0) {
	    nLocalRow = pAddressBookDB->GetLocalRow(nRealRow, m_vpRow);
	} else {
	    nLocalRow = -1;
	}
    } else {
	nLocalRow = -1;
    }
    if (nLocalRow >= 0) {
	// See if visible based on selected category
	if (m_nCategory != -1) {
	    if (nLocalRow >=
		pAddressBookDB->NumRecsByKey(AB_CAT, m_szCategory)) {
		// Not visible, reselect row 0
		nLocalRow = 0;
	    }
	}
    } else {
	nLocalRow = 0;
    }
    row(nLocalRow);

    // Cause the list to be redrawn
    redraw();
    SelectRow(nRows >
	      0 ? pAddressBookDB->FindRealRow(nLocalRow, m_vpRow) : -1);
}


//--------------------------------------------------------------//
// Process a right mouse click anywhere                         //
//--------------------------------------------------------------//
void
AddressBookList::RightClick(int nRow, int nCol)
{
    static const Fl_Menu_Item menuPopup1[] = {
	{N_("Delete Address")},
	{N_("Copy Address")},
	{N_("Cut Address")},
	{N_("Edit Address"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Address")},
	{N_("Paste Address")},
	{NULL},
    };
    static const Fl_Menu_Item menuPopup2[] = {
	{N_("Delete Address")},
	{N_("Copy Address")},
	{N_("Cut Address")},
	{N_("Edit Address"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Address")},
	{NULL},
    };
    const Fl_Menu_Item *pmenuPopup;
    int nRealRow;
    int nSelection;

    // Translate the row number
    if (nRow >= 0) {
	nRealRow =
	    AddressBookDB::GetAddressBookDB()->FindRealRow(nRow, m_vpRow);
    } else {
	nRealRow = -1;
    }

    // Display the details for this line to the right
    SelectRow(nRealRow);

    if (m_bSmall == false) {
	// Only process if not a small display
	// Display the popup menu
	pmenuPopup = ((AddressBook *) parent())->CanPaste()
	    ? menuPopup1 : menuPopup2;
	nSelection = DoPopupMenu((nRow >= 0 ? pmenuPopup : &pmenuPopup[4]),
				 Fl::event_x(), Fl::event_y());

	if (nSelection >= 0) {
	    // Fix the selection for the shorter menu
	    nSelection += (nRow < 0 ? 4 : 0);

	    // Process the selection
	    switch (nSelection) {
	    case 0:		// Delete this row
		((AddressBook *) parent())->Delete(nRealRow);
		break;

	    case 1:		// Copy this row
		((AddressBook *) parent())->Copy(nRealRow);
		break;

	    case 2:		// Cut this row
		((AddressBook *) parent())->Cut(nRealRow);
		break;

	    case 3:		// Edit this row
		((AddressBook *) parent())->Edit(nRealRow);
		break;

	    case 4:		// Insert a new row
		((AddressBook *) parent())->EditNew();
		break;

	    case 5:		// Paste a row
		((AddressBook *) parent())->Paste();
	    }
	}
    }
}


//--------------------------------------------------------------//
// Search for a row beginning with these characters.            //
//--------------------------------------------------------------//
bool
AddressBookList::Search(const char *pszValue)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    bool bReturn;
    int nCol = (m_bByLastName == true ? AB_LASTNAME : AB_COMPANY);
    int nRow = pAddressBookDB->FindFirst(nCol, pszValue);

#ifdef WIN32
    // Beep if an error under Windows
    if (nRow == -1) {
	MessageBeep(MB_ICONQUESTION);
    }
#endif

    // Don't change the row if an error occurred
    if (nRow == -1) {
	bReturn = false;
    } else {
	// No error, change the row
	bReturn = true;

	// Select this row
	nRow = pAddressBookDB->GetLocalRow(nRow, m_vpRow);
	SelectRow(nRow);
    }

    // Set to the selected row
    return (bReturn);
}


//--------------------------------------------------------------//
// Select a row and inform the parent of the selection.         //
//--------------------------------------------------------------//
void
AddressBookList::SelectRow(int nRealRow)
{
    int nLocalRow;

    // Translate the row number
    if (nRealRow >= 0) {
	nLocalRow =
	    AddressBookDB::GetAddressBookDB()->GetLocalRow(nRealRow, m_vpRow);
    } else {
	nLocalRow = -1;
    }

    // Select this row
    row(nLocalRow);

    // Only notify the parent if not a small display
    if (m_bSmall == false) {
	// Tell the parent
	((AddressBook *) parent())->DisplayRow(nRealRow);
    }
}


//--------------------------------------------------------------//
// Compare by last name, first name and filter by category for  //
// sorting.                                                     //
//--------------------------------------------------------------//
bool
AddressBookList::SortCategory1(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    // Is the first row the correct category
    if (pRow1->GetStringValue(AB_CAT) == m_szCategory) {
	// Is the second row the correct category
	if (pRow2->GetStringValue(AB_CAT) == m_szCategory) {
	    // Both good, sort like normal
	    bReturn = SortCompare1(pRow1, pRow2);
	} else {
	    // First is good, second is not, say first is lower
	    bReturn = true;
	}
    } else {
	// Is the second row the correct category
	if (pRow2->GetStringValue(AB_CAT) == m_szCategory) {
	    // First is not good, second is good, say first is not lower
	    bReturn = false;
	} else {
	    // Both not good, sort like normal
	    bReturn = SortCompare1(pRow1, pRow2);
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare by company name, last name and filter by category    //
// for sorting.                                                 //
//--------------------------------------------------------------//
bool
AddressBookList::SortCategory2(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    // Is the first row the correct category
    if (pRow1->GetStringValue(AB_CAT) == m_szCategory) {
	// Is the second row the correct category
	if (pRow2->GetStringValue(AB_CAT) == m_szCategory) {
	    // Both good, sort like normal
	    bReturn = SortCompare2(pRow1, pRow2);
	} else {
	    // First is good, second is not, say first is lower
	    bReturn = true;
	}
    } else {
	// Is the second row the correct category
	if (pRow2->GetStringValue(AB_CAT) == m_szCategory) {
	    // First is not good, second is good, say first is not lower
	    bReturn = false;
	} else {
	    // Both not good, sort like normal
	    bReturn = SortCompare1(pRow1, pRow2);
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare by last name, first name for sorting.                //
//--------------------------------------------------------------//
bool
AddressBookList::SortCompare1(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;
    string strString1 = pRow1->GetStringValue(AB_LASTNAME);
    string strString2 = pRow2->GetStringValue(AB_LASTNAME);

    if (strcasecmp(strString1.c_str(), strString2.c_str()) < 0) {
	bReturn = true;
    } else if (strcasecmp(strString1.c_str(), strString2.c_str()) > 0) {
	bReturn = false;
    } else			// equal
    {
	bReturn = (strcasecmp(pRow1->GetStringValue(AB_FIRSTNAME).c_str(),
			      pRow2->GetStringValue(AB_FIRSTNAME).c_str()) <
		   0);
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare by company name, last name for sorting.              //
//--------------------------------------------------------------//
bool
AddressBookList::SortCompare2(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;
    string strString1 = pRow1->GetStringValue(AB_COMPANY);
    string strString2 = pRow2->GetStringValue(AB_COMPANY);

    if (strcasecmp(strString1.c_str(), strString2.c_str()) < 0) {
	bReturn = true;
    } else if (strcasecmp(strString1.c_str(), strString2.c_str()) > 0) {
	bReturn = false;
    } else			// equal
    {
	bReturn = (strcasecmp(pRow1->GetStringValue(AB_LASTNAME).c_str(),
			      pRow2->GetStringValue(AB_LASTNAME).c_str()) <
		   0);
    }
    return (bReturn);
}
