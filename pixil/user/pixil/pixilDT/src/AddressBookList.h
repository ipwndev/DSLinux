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
#ifndef ADDRESSBOOKLIST_H_

#define ADDRESSBOOKLIST_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "AddressBookDB.h"
#include "Messages.h"
#include "TableBase.h"

#define ADDRESSBOOK_LARGE_COLUMNS 5
#define ADDRESSBOOK_SMALL_COLUMNS 3
typedef void (*ADDRESS_BOOK_CALLBACK) (int nRow, int nCol);
class AddressBookList:public TableBase
{
  public:AddressBookList(int nX,
		    // Constructor
		    int nY, int nWidth, int nHeight, bool bSmall);
     ~AddressBookList();	// Destructor
    void Filter(int nCategory);	// Filter the displayed rows by category (-1 = all categories)
    inline bool GetListBy() const	// Get the List By option
    {
	return (m_bByLastName);
    }
    inline int GetRealRow()	// Get the real row number of the currently selected row
    {
	return (AddressBookDB::GetAddressBookDB()->
		FindRealRow(row(), m_vpRow));
    }
    int Message(PixilDTMessage nMessage,	// Process a message from the parent widget
		int nInfo);
    void Print();		// Print the address book as currently sorted
    void Refresh(int nID = -1);	// Refresh this list
    bool Search(const char *pszValue);	// Search for a row beginning with...
    void SelectRow(int nRealRow);	// Select a row in the list and inform the parent of the selection
    inline void SetDoubleClick(ADDRESS_BOOK_CALLBACK pfnCallback)	// Process a double click on a non-icon column for a short display
    {
	m_pfnDoubleClick = pfnCallback;
    }
    inline void SetListBy(bool bByLastName)	// Set the List By option
    {
	m_bByLastName = bByLastName;
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
  private:ADDRESS_BOOK_CALLBACK m_pfnDoubleClick;
    // Process a double click from a short display
    static const bool m_bLargeIconColumns[ADDRESSBOOK_LARGE_COLUMNS];	// Large icon columns
    static const bool m_bSmallIconColumns[ADDRESSBOOK_SMALL_COLUMNS];	// Small icon columns
    bool m_bByLastName;		// Display by last name or by company name
    bool m_bSmall;		// Small list or not
    static char m_szCategory[16];	// Category being filtered on
    Fl_Pixmap *m_pNotesIcon;	// Notes icon used by all rows
    int m_nCategory;		// Category used for filtering (-1 for all)
    ROW_VECTOR m_vpRow;		// Private copy of pointers to address book rows in the current sort order
    static bool SortCategory1(NxDbRow * pRow1,	// Sort by last name, first name and filter by category
			      NxDbRow * pRow2);
    static bool SortCategory2(NxDbRow * pRow1,	// Sort by company, last name and filter by category
			      NxDbRow * pRow2);
    static bool SortCompare1(NxDbRow * pRow1,	// Sort by last name, first name
			     NxDbRow * pRow2);
    static bool SortCompare2(NxDbRow * pRow1,	// Sort by company, last name
			     NxDbRow * pRow2);
};


#endif /*  */
