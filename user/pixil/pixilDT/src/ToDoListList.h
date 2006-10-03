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
#ifndef TODOLISTLIST_H_

#define TODOLISTLIST_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include "Messages.h"
#include "ToDoListDB.h"
#include "TableBase.h"
typedef void (*TODO_CALLBACK) (int nRow, int nCol);
class ToDoListList:public TableBase
{
  public:enum SortOption
    { TODO_LIST_SORT_PRIO_DUE =
	    0, TODO_LIST_SORT_DUE_PRIO, TODO_LIST_SORT_CAT_PRIO,
	TODO_LIST_SORT_CAT_DUE,
    };
      ToDoListList(int nX,	// Constructor
		   int nY, int nWidth, int nHeight, bool bSmall);
     ~ToDoListList();		// Destructor
    int CountDBRows();		// Count the number of visible rows based on sorting and filtering
    void Filter(int nCategory);	// Filter the displayed rows by category (-1 = all categories)
    inline int GetRealRow()	// Get the real row number of the currently selected row
    {
	return (ToDoListDB::GetToDoListDB()->FindRealRow(row(), m_vpRow));
    }
    int Message(PixilDTMessage nMessage,	// Process a message from the parent widget
		int nInfo);
    void Print();		// Print the To Do List as currently sorted
    void Refresh(int nID = -1);	// Refresh this list
    void SelectRow(int nRealRow);	// Select a row in the list and inform the parent of the selection
    inline void SetDoubleClick(TODO_CALLBACK pfnCallback)	// Set the double click callback
    {
	m_pfnDoubleClick = pfnCallback;
    }
  protected:void CalculateColumnWidths(int nWidth);
    // Recalculate column widths
    void DoubleClick(int nRow,	// Process a double click over a row
		     int nCol);
    Fl_Pixmap *GetIconValue(int nRow,	// Get the icon for an icon column
			    int nCol);
    string GetStringValue(int nRow,	// Get the value of a column
			  int nCol);
    void IconClick(int nRow,	// Process a left mouse click over an icon column
		   int nCol);
    void LeftClick(int nRow,	// Process a left mouse click over a non-icon column
		   int nCol);
    void RightClick(int nRow,	// Process a right mouse click anywhere
		    int nCol);
  private:static const bool m_bIconColumns[8][6];
    // Icon columns based on display options
    bool m_bSmall;		// Small list, no parent notifications
    bool m_bShowCategory;	// Show category
    bool m_bShowCompleted;	// Show completed items or not
    bool m_bShowDueDate;	// Show due dates
    bool m_bShowOnlyDue;	// Show only due items
    bool m_bShowPriority;	// Show the priority
    Fl_Pixmap *m_pBoxIcon;	// Unchecked box icon
    Fl_Pixmap *m_pCheckboxIcon;	// Checked box icon
    Fl_Pixmap *m_pNotesIcon;	// Notes icon used by all rows
    int m_nCategory;		// Category used for filtering (-1 for all)
    static const int m_nColumns[8];	// Column counts by display type
    static const int m_nColumnWidth[8][6];	// Column widths by display type
    int m_nDisplayType;		// Current display type, calculated by GetOptions
    static const int m_nField[8][6];	// Data base columns displayed in table columns by display type
    static const int m_nSelect[8];	// Column to be selected by display type
    int m_nSortType;		// The type of sorting in effect
    ROW_VECTOR m_vpRow;		// Local copy of database rows
    TODO_CALLBACK m_pfnDoubleClick;	// Function used to process double clicks
    static ToDoListList *m_pSort;	// Object currently sorting
    void GetOptions();		// Get the display options
    bool IsVisible(int nRow);	// Test if a row is visible or not
    bool IsVisible(NxDbRow * pRow);	// Test if a row is visible or not
    static bool SortCompare(NxDbRow * pRow1,	// Sort by priority/due date
			    NxDbRow * pRow2);
};


#endif /*  */
