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
// Base class for table based widgets, in turn based on         //
// FLVW_Table.                                                  //
//--------------------------------------------------------------//
#ifndef TABLEBASE_H_

#define TABLEBASE_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
#include <FL/Enumerations.H>
#include <FL/Fl_Pixmap.H>
#include <Flek/Flv_Table.H>
using namespace std;
class TableBase:public Flv_Table
{
  public:TableBase(int nX,	// Constructor
	      int nY, int nWidth, int nHeight);
     ~TableBase();		// Destructor
    int col_width(int nCol);	// Recalculate column widths
    virtual void Filter(int nCategory) = 0;	// Filter the displayed items for a category
    virtual void Refresh(int nID = -1) = 0;	// Refresh this list display
  protected:int m_nColWidth[16];
    // Used in CalculateColumnWidths methods in derived classes, must be larger than the max number of columns
    int m_nLastWidth;		// Used in col_width
    virtual void CalculateColumnWidths(int nWidth) = 0;	// Calculate column widths
    virtual void DoubleClick(int nRow,	// Process a double click over a row
			     int nCol) = 0;
    void draw_cell(int nOffset,	// Draw a cell
		   int &nX, int &nY, int &nWidth, int &nHeight,
		   int nRow, int nCol);
    virtual Fl_Pixmap *GetIconValue(int nRow,	// Get the icon for an icon column
				    int nCol) = 0;
    virtual string GetStringValue(int nRow,	// Get the string value for a column
				  int nCol) = 0;
    int handle(int nEvent);	// Event handler
    virtual void IconClick(int nRow,	// Process a left mouse click over an icon column
			   int nCol) = 0;
    virtual void LeftClick(int nRow,	// Process a left mouse click over a non-icon column
			   int nCol) = 0;
    void PostConstructor(int nRows,	// Post construction code that will eventually invoke pure virtual functions
			 int nColumns, const bool * pbIconColumns);
    virtual void RightClick(int nRow,	// Process a right mouse click anywhere
			    int nCol) = 0;
  private:  bool * m_pbIconColumns;
    // Which columns are icon columns
    int m_nColumns;		// Number of columns
};


#endif /*  */
