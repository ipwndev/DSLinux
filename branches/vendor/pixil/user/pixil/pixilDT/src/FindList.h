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
#ifndef FINDLIST_H_

#define FINDLIST_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include "TableBase.h"
class FindList:public TableBase
{
  public:FindList(int nX,	// Constructor
	     int nY, int nWidth, int nHeight);
     ~FindList();		// Destructor
    void Filter(int nCategory)	// Filter the displayed items for a category, not used here
    {
    }
    void Refresh(int nID = -1)	// Refresh this list display, not used here
    {
    }
  protected:void CalculateColumnWidths(int nWidth);
    // Calculate column widths
    void DoubleClick(int nRow,	// Process a double click over a row
		     int nCol);
    Fl_Pixmap *GetIconValue(int nRow,	// Get the icon for an icon column, not used here
			    int nCol)
    {
	return (NULL);
    }
    int GetRowCount();		// Get the number of rows to be displayed
    string GetStringValue(int nRow,	// Get the string value for a column
			  int nCol);
    void IconClick(int nRow,	// Process a left mouse click over an icon column, not used here
		   int nCol)
    {
    }
    void LeftClick(int nRow,	// Process a left mouse click over a non-icon column, nothing special here
		   int nCol)
    {
    }
    void RightClick(int nRow,	// Process a right mouse click anywhere
		    int nCol);
};


#endif /*  */
