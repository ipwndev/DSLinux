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
// Class for the ToDo List List widget.                         //
//--------------------------------------------------------------//
#include "config.h"
#include <ctime>
#include "FLTKUtil.h"
#include "Images.h"
#include "NoteEditorDlg.h"
#include "Options.h"
#include "PixilDT.h"
#include "Printer.h"
#include "TimeFunc.h"
#include "ToDoListCategoryDB.h"
#include "ToDoListDB.h"
#include "ToDoListList.h"

#include "VCMemoryLeak.h"

#ifdef WIN32
#define strcasecmp stricmp
#endif


//--------------------------------------------------------------//
// Column counts for display types                              //
//--------------------------------------------------------------//
const int
    ToDoListList::m_nColumns[8] = {
	3,
	4,
	4,
	5,
	4,
	5,
	5,
6 };


//--------------------------------------------------------------//
// Database columns being display by type                       //
//--------------------------------------------------------------//
const int
    ToDoListList::m_nField[8][6] = {
    {-1, TODO_TITLE, -1, -1, -1, -1},
    {-1, TODO_TITLE, TODO_CAT, -1, -1, -1},
    {-1, TODO_PRIORITY, TODO_TITLE, -1, -1, -1},
    {-1, TODO_PRIORITY, TODO_TITLE, TODO_CAT, -1, -1},
    {-1, TODO_TITLE, TODO_TIME, -1, -1, -1},
    {-1, TODO_TITLE, TODO_TIME, TODO_CAT, -1, -1},
    {-1, TODO_PRIORITY, TODO_TITLE, TODO_TIME, -1, -1},
    {-1, TODO_PRIORITY, TODO_TITLE, TODO_TIME, TODO_CAT, -1},
};


//--------------------------------------------------------------//
// Table column widths by display type.  A negative value is an //
// absolute width, a positive value is a percentage (should all //
// add up to 100).                                              //
//--------------------------------------------------------------//
const int
    ToDoListList::m_nColumnWidth[8][6] = {
    {-16, 100, -12, 0, 0, 0},
    {-16, 88, 12, -12, 0, 0},
    {-16, -12, 100, -12, 0, 0},
    {-16, -12, 88, 12, -12, 0},
    {-16, 88, 12, -12, 0, 0},
    {-16, 76, 12, 12, -12, 0},
    {-16, -12, 88, 12, -12, 0},
    {-16, -12, 76, 12, 12, -12},
};


//--------------------------------------------------------------//
// Column to be selected                                        //
//--------------------------------------------------------------//
const int
    ToDoListList::m_nSelect[8] = {
	1,
	1,
	2,
	2,
	1,
	1,
	2,
2 };


//--------------------------------------------------------------//
// Icon column indicators                                       //
//--------------------------------------------------------------//
const bool
    ToDoListList::m_bIconColumns[8][6] = {
    //                                Due Date   Priorities   Categories
    {true, false, true},	// No         No           No
    {true, false, false, true},	// No         No           Yes
    {true, false, false, true},	// No         Yes          No
    {true, false, false, false, true},	// No         Yes          Yes
    {true, false, false, true},	// Yes        No           No
    {true, false, false, false, true},	// Yes        No           Yes
    {true, false, false, false, true},	// Yes        Yes          No
    {true, false, false, false, false, true},	// Yes        Yes          No
};


//--------------------------------------------------------------//
// Object currently doing the sorting                           //
//--------------------------------------------------------------//
ToDoListList *
    ToDoListList::m_pSort;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ToDoListList::ToDoListList(int nX,
			   int nY, int nWidth, int nHeight, bool bSmall)
    :
TableBase(nX, nY, nWidth, nHeight)
{
    // Select all categories
    m_nCategory = -1;

    // Record whether this is a small list or not
    m_bSmall = bSmall;

    // Create the icons
    m_pBoxIcon = Images::GetBoxIcon();
    m_pCheckboxIcon = Images::GetCheckboxIcon();
    m_pNotesIcon = Images::GetNotesIcon();

    // Get the display options
    GetOptions();

    // Get a copy of the rows
    m_vpRow = ToDoListDB::GetToDoListDB()->GetRows();

    // Finish TableBase construction, showing all columns
    PostConstructor(ToDoListDB::GetToDoListDB()->NumUndeletedRecs(),
		    m_nColumns[m_nDisplayType],
		    m_bIconColumns[m_nDisplayType]);
    feature_remove(FLVF_ROW_SELECT);

    // Display the details for the first line if it exists
    if (rows() > 0) {
	row(0);
	Refresh();
	SelectRow(ToDoListDB::GetToDoListDB()->FindRealRow(0, m_vpRow));
    }
    // Set that there is no double click callback
    m_pfnDoubleClick = NULL;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ToDoListList::~ToDoListList()
{
    delete m_pBoxIcon;
    delete m_pCheckboxIcon;
    delete m_pNotesIcon;
}


//--------------------------------------------------------------//
// Recalculate column widths                                    //
//--------------------------------------------------------------//
void
ToDoListList::CalculateColumnWidths(int nWidth)
{
    int i;
    int nLastPctColumn = -1;
    int nTotalAbs = 0;
    int nTotalPct = 0;
    int nTotalWidth = 0;

    // Get the total of the absolute and percentage widths
    for (i = 0; i < m_nColumns[m_nDisplayType]; ++i) {
	if (m_nColumnWidth[m_nDisplayType][i] >= 0) {
	    nTotalPct += m_nColumnWidth[m_nDisplayType][i];
	} else {
	    nTotalAbs -= m_nColumnWidth[m_nDisplayType][i];
	}
    }

    // Now calculate each column width
    for (i = 0; i < m_nColumns[m_nDisplayType]; ++i) {
	if (m_nColumnWidth[m_nDisplayType][i] >= 0) {
	    m_nColWidth[i] =
		(nWidth -
		 nTotalAbs) * m_nColumnWidth[m_nDisplayType][i] / nTotalPct;
	    nLastPctColumn = i;
	} else {
	    m_nColWidth[i] = -m_nColumnWidth[m_nDisplayType][i];
	}
	nTotalWidth += m_nColWidth[i];
    }

    // Fix the percentage column for any rounding errors
    if (nLastPctColumn != -1) {
	m_nColWidth[nLastPctColumn] += nWidth - nTotalWidth;
    }
}


//--------------------------------------------------------------//
// Count the rows that will be visible                          //
//--------------------------------------------------------------//
int
ToDoListList::CountDBRows()
{
    int i;
    int nCount = 0;
    int nMax = m_vpRow.size();
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    for (i = 0; i < nMax; ++i) {
	nCount += ((!pToDoListDB->IsDeleted(i, m_vpRow))
		   && IsVisible(i) == true);
    }
    return (nCount);
}


//--------------------------------------------------------------//
// Process a double click over a row (do nothing)               //
//--------------------------------------------------------------//
void
ToDoListList::DoubleClick(int nRow, int nCol)
{
    if (m_pfnDoubleClick != NULL) {
	int nRow2 = ToDoListDB::GetToDoListDB()->FindRealRow(nRow, m_vpRow);

	(*m_pfnDoubleClick) (nRow2, nCol);
    }
}


//--------------------------------------------------------------//
// Filter the displayed rows by category (-1 = all categories)  //
//--------------------------------------------------------------//
void
ToDoListList::Filter(int nCategory)
{
    // Convert the category row to a category key
    if (nCategory != -1) {
	nCategory =
	    ToDoListCategoryDB::GetToDoListCategoryDB()->
	    GetCategoryID(nCategory);
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
ToDoListList::GetIconValue(int nRow, int nCol)
{
    Fl_Pixmap *pPixmap;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Translate the row number
    nRow = pToDoListDB->FindRealRow(nRow, m_vpRow);

    if (nCol == 0) {
	// Show the clear or checked box for uncompleted/completed
	if (pToDoListDB->GetComplete(nRow) == 1) {
	    pPixmap = m_pCheckboxIcon;
	} else {
	    pPixmap = m_pBoxIcon;
	}
    } else {
#ifdef DEBUG
	assert(nCol == m_nColumns[m_nDisplayType] - 1);	// This must be the last column
#endif
	if (pToDoListDB->GetDescription(nRow).length() > 0) {
	    pPixmap = m_pNotesIcon;
	} else {
	    pPixmap = NULL;
	}
    }

    return (pPixmap);
}


//--------------------------------------------------------------//
// Get the display options.                                     //
//--------------------------------------------------------------//
void
ToDoListList::GetOptions()
{
    m_bShowCategory = Options::GetToDoShowCategory();
    m_bShowCompleted = Options::GetToDoShowCompleted();
    m_bShowDueDate = Options::GetToDoShowDueDate();
    m_bShowOnlyDue = Options::GetToDoShowOnlyDue();
    m_bShowPriority = Options::GetToDoShowPriority();
    m_nDisplayType = 4 * (m_bShowDueDate == true ? 1 : 0)
	+ 2 * (m_bShowPriority == true ? 1 : 0)
	+ (m_bShowCategory == true ? 1 : 0);
    m_nSortType = Options::GetToDoSort();
}


//--------------------------------------------------------------//
// Get the value of a column.                                   //
//--------------------------------------------------------------//
string
ToDoListList::GetStringValue(int nRow, int nCol)
{
    char szValue[24];
    long nValue;
    string strData;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Translate the row number
    nRow = pToDoListDB->FindRealRow(nRow, m_vpRow);

    switch (m_nField[m_nDisplayType][nCol]) {
    case TODO_CAT:		// Get the ToDo item category name
	strData = pToDoListDB->GetCategoryName(nRow);
	break;

    case TODO_PRIORITY:	// Get the ToDo item priority
	nValue = pToDoListDB->GetPriority(nRow);
	sprintf(szValue, "%d", (int) nValue + 1);
	strData = szValue;
	break;

    case TODO_TIME:		// Get the ToDo item date
	nValue = pToDoListDB->GetTime(nRow);
	if (nValue > 24 * 60 * 60) {
	    strftime(szValue, sizeof(szValue), "%m/%d", localtime(&nValue));
	    strData = szValue;
	}
	break;

    case TODO_TITLE:		// Get the ToDo item title
	strData =::WrapText(pToDoListDB->GetTitle(nRow).c_str(),
			    m_nColWidth[nCol], this);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Bad column data ID
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
ToDoListList::IconClick(int nRow, int nCol)
{
    int nCompleted;
    int nRealRow;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    if (nRow >= 0) {
	nRealRow = pToDoListDB->FindRealRow(nRow, m_vpRow);
	SelectRow(nRealRow);

	// Translate the row number
	nRow = pToDoListDB->FindRealRow(nRow, m_vpRow);

	if (nCol == 0) {
	    // Toggle the completed flag
	    nCompleted = pToDoListDB->GetComplete(nRow);
	    pToDoListDB->SetComplete(nRow, nCompleted == 0 ? 1 : 0);

	    // Tell the details display of the change
	    if (m_bSmall == false) {
		((ToDoList *) parent())->ChangeComplete(nRow,
							nCompleted ==
							0 ? 1 : 0);
	    }
	    // Set the date completed if now complete
	    tzset();		// Just in case
	    if (nCompleted == 0) {
		time_t nTime = NormalizeDate(time(NULL));

		pToDoListDB->SetTime(nRow, nTime);

		// Tell the details display of the change
		if (m_bSmall == false) {
		    ((ToDoList *) parent())->ChangeTime(nRow, nTime);
		}
	    }
	    // Save the data base
	    pToDoListDB->Save();

	    // Tell everyone of the change
	    PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	} else if (nCol == m_nColumns[m_nDisplayType] - 1) {
	    string strDesc = pToDoListDB->GetDescription(nRow);

	    // Is there a note (description) to edit
	    if (strDesc.length() > 0) {
		Note *pNote = new Note(pToDoListDB->GetColumnSize(TODO_DESC));

		// Go edit this note
		pNote->SetText(strDesc.c_str());
		NoteEditorDlg *pDlg =
		    new NoteEditorDlg(pNote,
				      PixilDT::GetApp()->GetMainWindow());
		if (pDlg->DoModal() == 1) {
		    pNote = pDlg->GetNote();
		    pToDoListDB->SetDescription(nRow,
						pNote->GetText().c_str());
		    pToDoListDB->Save();

		    // Tell everyone of the change
		    PixilDT::GetApp()->GetMainWindow()->
			Notify(ADDRESS_BOOK_CHANGED, 0);
		}
		// Clean up
		delete pDlg;
		delete pNote;
	    }
	}
    }
}


//--------------------------------------------------------------//
// Test if a row is visible or not.                             //
//--------------------------------------------------------------//
bool
ToDoListList::IsVisible(int nRow)
{
    bool bReturn = true;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Translate the row number
    nRow = pToDoListDB->FindRealRow(nRow, m_vpRow);

    // Test if this is the correct category
    if (m_nCategory != -1) {
	if (pToDoListDB->GetCategory(nRow) != m_nCategory) {
	    bReturn = false;
	}
    }
    // Test if this is completed and not showing completed items
    if (bReturn == true) {
	if (m_bShowCompleted == false && pToDoListDB->GetComplete(nRow) == 1) {
	    bReturn = false;
	}
    }
    // Is this item not due and only showing due items
    if (bReturn == true) {
	if (m_bShowOnlyDue == true && pToDoListDB->GetTime(nRow) > time(NULL)) {
	    bReturn = false;
	}
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Test if a row is visible or not.                             //
//--------------------------------------------------------------//
bool
ToDoListList::IsVisible(NxDbRow * pRow)
{
    bool bReturn = true;

    // Test if this is the correct category
    if (m_nCategory != -1) {
	if (pRow->GetIntValue(TODO_CAT) != m_nCategory) {
	    bReturn = false;
	}
    }
    // Test if this is completed and not showing completed items
    if (bReturn == true) {
	if (m_bShowCompleted == false
	    && pRow->GetIntValue(TODO_COMPLETE) == 1) {
	    bReturn = false;
	}
    }
    // Is this item not due and only showing due items
    if (bReturn == true) {
	if (m_bShowOnlyDue == true
	    && pRow->GetIntValue(TODO_TIME) > time(NULL)) {
	    bReturn = false;
	}
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Process a left mouse click over a non-icon column.           //
//--------------------------------------------------------------//
void
ToDoListList::LeftClick(int nRow, int nCol)
{
    SelectRow(ToDoListDB::GetToDoListDB()->FindRealRow(nRow, m_vpRow));
}


//--------------------------------------------------------------//
// Process a message from the parent widget                     //
//--------------------------------------------------------------//
int
ToDoListList::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    switch (nMessage) {
    case TODO_LIST_CHANGED:
	Refresh();
	break;

    case TODO_LIST_GOTO:	// Find dialog request going to a particular item
	{
	    int nLocalRow;
	    int nRow;
	    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

	    // Is this item visible ?
	    nRow = pToDoListDB->FindPhysicalRecord(nInfo);
	    nLocalRow = pToDoListDB->GetLocalRow(nRow, m_vpRow);
	    if (IsVisible(nLocalRow) == false) {
		// Show everything so this row can be seen

		// Turn on all categories if needed
		if (m_nCategory != -1
		    && m_nCategory != pToDoListDB->GetCategory(nLocalRow)) {
		    // Show all categories
		    m_nCategory = -1;
		}
		// Turn on show completed if needed
		if (m_bShowCompleted == false
		    && pToDoListDB->GetComplete(nLocalRow) == 1) {
		    m_bShowCompleted = true;
		}
		// Turn off show only due items if needed
		if (m_bShowOnlyDue == true
		    && pToDoListDB->GetTime(nLocalRow) > time(NULL)) {
		    m_bShowOnlyDue = false;
		}
		// Refresh for this row
		Refresh(pToDoListDB->GetID(nLocalRow));
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
// Print the To Do List as currently sorted.                    //
//--------------------------------------------------------------//
void
ToDoListList::Print()
{
    bool bLastWasBox = false;
    char szData[48];
    int nBorder = INCH / 2;	// 1/2 inch border
    int nCategory;
    int nCopy;
    const int nFontSize = 10;
    int nLastCategory;
    int nLastPriority;
    int nPriority;
    int nRow;
    int nRow2;
    int nRows;
    int nSortType;
    Printer printer;
    string strData;
    string strRight;
    time_t nDueDate;
    time_t nLastDueDate;
    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();

    // Save any changes
    ((ToDoList *) parent())->SaveDetailChanges();

    // Open the printer
    if (printer.Open(_("ToDoList")) == true) {
	// Set an hourglass cursor
	PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_WAIT);

	// Print once for eqch requested copy
	for (nCopy = 0; nCopy < printer.GetCopies(); ++nCopy) {
	    // Reset the page number
	    printer.ResetPageNumber();

	    // Get the sort type for the items
	    nSortType = Options::GetToDoSort();

	    // Get the right-hand page title
	    if (Options::GetToDoShowCompleted() == true) {
		if (Options::GetToDoShowOnlyDue() == true) {
		    strRight = _("All Due Items");
		} else {
		    strRight = _("All Items");
		}
	    } else {
		if (Options::GetToDoShowOnlyDue() == true) {
		    strRight = _("All Incomplete Due Items");
		} else {
		    strRight = _("All Imcomplete Items");
		}
	    }

	    // Go to two column mode
	    printer.SetSerifFont(nFontSize);
	    printer.SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				     nBorder,
				     _("To Do List"), strRight.c_str(), 184);

	    // Print each visible entry
	    nRows = CountDBRows();

	    // Print each row
	    for (nRow = 0; nRow < nRows; ++nRow) {
		// Translate the row number
		nRow2 = pToDoListDB->FindRealRow(nRow, m_vpRow);

		switch (nSortType) {
		case 0:	// Sorted by priority/due date
		    // Has the priority number changed
		    nPriority = pToDoListDB->GetPriority(nRow2);
		    if (nRow == 0 || nPriority != nLastPriority) {
			// Yes, output a box for the priority
			nLastPriority = nPriority;
			sprintf(szData, _("Priority %d"), nLastPriority + 1);
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnBox(szData, 15, 5 * INCH / 16, 184);
			bLastWasBox = true;
		    }
		    break;

		case 1:	// Sorted by due date/priority
		    // Has the due date changed
		    nDueDate =::NormalizeDate(pToDoListDB->GetTime(nRow2));
		    if (nRow == 0 || nDueDate != nLastDueDate) {
			// Yes, output a box for the due date
			nLastDueDate = nDueDate;
			strData =::FormatDate(nLastDueDate);
			if (strData.length() == 0) {
			    strcpy(szData, _("No Due Date"));
			} else {
			    sprintf(szData, _("Due Date %s"),
				    strData.c_str());
			}
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnBox(szData, 15, 5 * INCH / 16, 184);
			bLastWasBox = true;
		    }
		    break;

		default:	// Cases 2 and 3 sorted by Category
		    // Has the category changed
		    nCategory = pToDoListDB->GetCategory(nRow2);
		    if (nRow == 0 || nCategory != nLastCategory) {
			// Yes, output a box for the category
			nLastCategory = nCategory;
			strData = pToDoListDB->GetCategoryName(nRow2);
			sprintf(szData, _("Category %s"), strData.c_str());
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnSpaceLine();
			printer.ColumnBox(szData, 15, 5 * INCH / 16, 184);
			bLastWasBox = true;
		    }
		}

		// Output this To Do Item
		if (nRow != 0 && bLastWasBox == false) {
		    printer.ColumnNewLine();
		}
		bLastWasBox = false;

		// Output the completed icon
		printer.ColumnIconComplete(0,
					   pToDoListDB->GetComplete(nRow2) ==
					   1);

		// Output the title
		strData = pToDoListDB->GetTitle(nRow2);
		printer.SetBoldSerifFont(nFontSize);
		printer.ColumnShow(strData.c_str(), INCH / 4, 0);
		printer.SetSerifFont(nFontSize);

		// Output the due date if needed
		if (nSortType != 1) {
		    printer.ColumnShowNoAdvance(_("Due Date:"), INCH / 4);
		    strData = pToDoListDB->GetTimeString(nRow2);
		    if (strData.length() == 0) {
			strData = _("None");
		    }
		    printer.ColumnShow(strData.c_str(), (9 * INCH) / 8, 0);
		}
		// Output the priority if needed
		if (nSortType != 0) {
		    printer.ColumnShowNoAdvance(_("Priority:"), INCH / 4);
		    sprintf(szData, "%d",
			    pToDoListDB->GetPriority(nRow2) + 1);
		    printer.ColumnShow(szData, (9 * INCH) / 8, 0);
		}
		// Output the category if needed
		if (nSortType != 2 && nSortType != 3) {
		    printer.ColumnShowNoAdvance(_("Category:"), INCH / 4);
		    strData = pToDoListDB->GetCategoryName(nRow2);
		    printer.ColumnShow(strData.c_str(), (9 * INCH) / 8, 0);
		}
		// Output the description if needed
		strData = pToDoListDB->GetDescription(nRow2);
		if (strData.length() > 0) {
		    printer.ColumnShowNoAdvance(_("Note:"), INCH / 4);
		    strData = pToDoListDB->GetDescription(nRow2);
		    printer.ColumnShow(strData.c_str(), (9 * INCH) / 8, 0);
		}
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
ToDoListList::Refresh(int nID)
{
    m_vpRow = ToDoListDB::GetToDoListDB()->GetRows();
    int nRow = row();
    int nRows = CountDBRows();

    ToDoListDB *pToDoListDB = ToDoListDB::GetToDoListDB();



    // Get the currently selected row
    if (nID < 0) {
	if (nRows > 0 && nRow >= 0) {
	    nID = pToDoListDB->GetID(pToDoListDB->FindRealRow(nRow, m_vpRow));
	} else {
	    // No row currently selected
	    nID = -1;
	}
    }
    // Refresh the local copy of the rows
    m_vpRow = pToDoListDB->GetRows();

    // Get the display options
    GetOptions();
    nRows = CountDBRows();
    PostConstructor(nRows,
		    m_nColumns[m_nDisplayType],
		    m_bIconColumns[m_nDisplayType]);
    feature_remove(FLVF_ROW_SELECT);

    // Re-sort the data base
    m_pSort = this;
    pToDoListDB->Sort(SortCompare, m_vpRow);
    row(-1);			// Workaround for top_row update
    rows(nRows);

    // Reselect the same row if possible
    if (nRows > 0) {
	if (nID >= 0) {
	    nRow = pToDoListDB->FindRow(TODO_ID, nID);
	    if (nRow >= 0) {
		nRow = pToDoListDB->GetLocalRow(nRow, m_vpRow);
		if (nRow >= 0) {
		    // Get the local row number
		    nRow = pToDoListDB->GetLocalRow(nRow, m_vpRow);

		    // See if visible
		    if (!IsVisible(nRow)) {
			// Not visible, reselect row 0
			nRow = 0;
		    }
		} else {
		    // Could not find the row, just select the first row
		    nRow = 0;
		}
	    } else {
		// Could not find the row, just select the first row
		nRow = 0;
	    }
	} else {
	    // No prior selected row, just select the first row
	    nRow = 0;
	}
    }
    row(nRow);

    // Cause the list to be redrawn
    redraw();
    SelectRow(nRows > 0 ? pToDoListDB->FindRealRow(row(), m_vpRow) : -1);
}


//--------------------------------------------------------------//
// Process a right mouse click anywhere.                        //
//--------------------------------------------------------------//
void
ToDoListList::RightClick(int nRow, int nCol)
{
    static const Fl_Menu_Item menuPopup1[] = {
	{N_("Delete To Do Item"),},
	{N_("Copy To Do Item"),},
	{N_("Cut To Do Item"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New To Do Item")},
	{N_("Paste To Do Item")},
	{NULL},
    };
    static const Fl_Menu_Item menuPopup2[] = {
	{N_("Delete To Do Item"),},
	{N_("Copy To Do Item"),},
	{N_("Cut To Do Item"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New To Do Item")},
	{NULL},
    };
    const Fl_Menu_Item *pmenuPopup;
    int nRealRow = -1;
    int nSelection;

    // Translate the row
    if (nRow >= 0) {
	nRealRow = ToDoListDB::GetToDoListDB()->FindRealRow(nRow, m_vpRow);
    }
    // Display the details for this line to the right
    SelectRow(nRealRow);

    if (m_bSmall == false) {
	// Display the popup menu
	pmenuPopup = ((ToDoList *) parent())->CanPaste()
	    ? menuPopup1 : menuPopup2;
	nSelection = DoPopupMenu((nRow >= 0 ? pmenuPopup : &pmenuPopup[3]),
				 Fl::event_x(), Fl::event_y());

	if (nSelection >= 0) {
	    // Correct for no delete row option
	    nSelection += (nRow < 0 ? 3 : 0);

	    switch (nSelection) {
	    case 0:		// Delete this row
		((ToDoList *) parent())->Delete(nRealRow);
		break;

	    case 1:		// Copy this row
		((ToDoList *) parent())->Copy(nRealRow);
		break;

	    case 2:		// Cut this row
		((ToDoList *) parent())->Cut(nRealRow);
		break;

	    case 3:		// Insert a new row
		((ToDoList *) parent())->EditNew();
		break;

	    case 4:		// Paste a row
		((ToDoList *) parent())->Paste();
	    }
	}
    }
}


//--------------------------------------------------------------//
// Select a row in the list and inform the parent of the        //
// selection                                                    //
//--------------------------------------------------------------//
void
ToDoListList::SelectRow(int nRealRow)
{
    int nLocalRow;

    if (nRealRow >= 0) {
	// Translate the row number
	nLocalRow =
	    ToDoListDB::GetToDoListDB()->GetLocalRow(nRealRow, m_vpRow);

	// Now process it
	row(nLocalRow);
	col(m_nSelect[m_nDisplayType]);
	if (m_bSmall == false) {
	    // Tell the parent
	    ((ToDoList *) parent())->DisplayRow(nRealRow, CountDBRows());
	}
    } else {
	if (m_bSmall == false) {
	    // Tell the parent
	    ((ToDoList *) parent())->DisplayRow(nRealRow, CountDBRows());
	}
    }
}


//--------------------------------------------------------------//
// Compare for sorting.                                         //
//--------------------------------------------------------------//
bool
ToDoListList::SortCompare(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;
    int nNotVisible = 0;
    int nValue1;
    int nValue2;

    // Test the first row for visibility
    if (m_pSort->IsVisible(pRow1) == false) {
	// First row is not visible, sort it higher
	bReturn = false;
	++nNotVisible;
    }
    // Test the second row for visibility
    if (m_pSort->IsVisible(pRow2) == false) {
	// Second row is not visible, sort it higher
	bReturn = true;
	++nNotVisible;
    }
    // Test more if the visibility matches for both
    if (nNotVisible != 1) {
	// Compare fields to see which is higher
	switch (m_pSort->m_nSortType) {
	case TODO_LIST_SORT_PRIO_DUE:	// By priority/due date
	    nValue1 = pRow1->GetIntValue(TODO_PRIORITY);
	    nValue2 = pRow2->GetIntValue(TODO_PRIORITY);
	    if (nValue1 > nValue2) {
		bReturn = false;
	    } else if (nValue1 < nValue2) {
		bReturn = true;
	    } else {
		bReturn =
		    (pRow1->GetIntValue(TODO_TIME) <
		     pRow2->GetIntValue(TODO_TIME));
	    }
	    break;

	case TODO_LIST_SORT_DUE_PRIO:	// By due date/priority
	    nValue1 = pRow1->GetIntValue(TODO_TIME);
	    nValue2 = pRow2->GetIntValue(TODO_TIME);
	    if (nValue1 > nValue2) {
		bReturn = false;
	    } else if (nValue1 < nValue2) {
		bReturn = true;
	    } else {
		bReturn =
		    (pRow1->GetIntValue(TODO_PRIORITY) <
		     pRow2->GetIntValue(TODO_PRIORITY));
	    }
	    break;

	case TODO_LIST_SORT_CAT_PRIO:	// By category/priority
	    nValue1 = pRow1->GetIntValue(TODO_CAT);
	    nValue2 = pRow2->GetIntValue(TODO_CAT);
	    if (nValue1 > nValue2) {
		bReturn = false;
	    } else if (nValue1 < nValue2) {
		bReturn = true;
	    } else {
		bReturn =
		    (pRow1->GetIntValue(TODO_PRIORITY) <
		     pRow2->GetIntValue(TODO_PRIORITY));
	    }
	    break;

	case TODO_LIST_SORT_CAT_DUE:	// By category/due date
	    nValue1 = pRow1->GetIntValue(TODO_CAT);
	    nValue2 = pRow2->GetIntValue(TODO_CAT);
	    if (nValue1 > nValue2) {
		bReturn = false;
	    } else if (nValue1 < nValue2) {
		bReturn = true;
	    } else {
		bReturn =
		    (pRow1->GetIntValue(TODO_TIME) <
		     pRow2->GetIntValue(TODO_TIME));
	    }
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Unknown sort algorithm
#endif
	    bReturn = false;
	}
    }

    return (bReturn);
}
