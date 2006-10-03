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
#include <cstring>
#include <FL/Fl.H>
#include <FL/forms.H>
#include <FL/fl_draw.H>
#include "TableBase.h"

#include "FLTKUtil.h"
#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
TableBase::TableBase(int nX, int nY, int nWidth, int nHeight)
    :
Flv_Table(nX, nY, nWidth, nHeight)
{
    // Used in col_width in derived classes
    m_nLastWidth = -1;

    // Default initialization
    m_nColumns = 0;
    m_pbIconColumns = NULL;

    // Set up the scrollbars
    has_scrollbar(FLVS_AUTOMATIC + FLVS_VERTICAL);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
TableBase::~TableBase()
{
    // Delete the saved icon column indications
    delete[]m_pbIconColumns;
}


//--------------------------------------------------------------//
// Calculate column widths after a resize.                      //
//--------------------------------------------------------------//
int
TableBase::col_width(int nCol)
{
    int nScrollbarWidth = (scrollbar.visible()? scrollbar.w() : 0);
    int nWidth = w() - nScrollbarWidth - 1;	//-2*Fl::box_dw(FL_DOWN_BOX);

    // Either always calculate or be sure to check that width has not changed
    if (nWidth != m_nLastWidth) {
	// Width change, recalculate
	memset(m_nColWidth, 0, sizeof(m_nColWidth));

	// Call on the derived widget to calculate column widths into m_nColWidth
	CalculateColumnWidths(nWidth);

	// Set the last width for the next call
	m_nLastWidth = nWidth;
    }
    return (m_nColWidth[nCol]);
}


//--------------------------------------------------------------//
// Draw a cell.                                                 //
//--------------------------------------------------------------//
void
TableBase::draw_cell(int nOffset,
		     int &nX,
		     int &nY, int &nWidth, int &nHeight, int nRow, int nCol)
{
    Flv_Style s;

    Flv_Table::draw_cell(nOffset, nX, nY, nWidth, nHeight, nRow, nCol);
    get_style(s, nRow, nCol);
    if (m_pbIconColumns[nCol] == false) {
	// Non-icon column, get the text
	fl_draw(GetStringValue(nRow, nCol).c_str(),
		nX - nOffset, nY, nWidth, nHeight, s.align());
    } else {
	Fl_Pixmap *pImage;
	int nImageHeight;
	int nImageWidth;

	// Icon column, get the icon
	pImage = GetIconValue(nRow, nCol);
	if (pImage != NULL) {
	    fl_measure_pixmap(pImage->data, nImageWidth, nImageHeight);
	    pImage->draw(nX + ((nWidth - nImageWidth) >> 1),
			 nY + ((nHeight - nImageHeight) >> 1));
	}
    }
}


//--------------------------------------------------------------//
// Event handler                                                //
//--------------------------------------------------------------//
int
TableBase::handle(int nEvent)
{

    int nCol = get_col(Fl::event_x(), Fl::event_y());
    int nReturn = Flv_Table::handle(nEvent);
    int nRow = get_row(Fl::event_x(), Fl::event_y());


    switch (nEvent) {
    case FL_PUSH:		// Mouse button pushed
	if (Fl::event_button() == FL_LEFT_MOUSE) {
	    // Ask for release if not in a grey area
	    nReturn = (nRow != -4 ? 1 : 0);
	} else if (Fl::event_button() == FL_RIGHT_MOUSE) {
	    // Ask for release
	    nReturn = 1;
	}
	break;

    case FL_RELEASE:		// Mouse button released
	if (Fl::event_button() == FL_LEFT_MOUSE) {
	    if (nRow >= 0) {
		if (Fl::event_clicks() > 0) {
		    // Double click on a row, go process it
		    DoubleClick(nRow, nCol);
		    nReturn = 1;
		} else {
		    // Single click on a row, was it on an icon
		    if (m_pbIconColumns[nCol] == true) {
			// Go process a click on an icon
			IconClick(nRow, nCol);
			nReturn = 1;
		    } else {
			// Go process a click on a non-icon column
			LeftClick(nRow, nCol);
			nReturn = 1;
		    }
		}
	    }
	} else if (Fl::event_button() == FL_RIGHT_MOUSE) {
	    // If on a row or on a gray area then do a popup menu
	    if (nRow >= 0 || nRow == -4) {
		// Do a popup menu
		RightClick(nRow, nCol);
		nReturn = 1;
	    }
	}
	break;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Post construction code that will cause the widget to be      //
// re-drawn which will in turn invoke pure virtual functions.   //
// This code had to be removed from the TableBase constructor   //
// and must be called from the derived classes constructor.     //
//--------------------------------------------------------------//
void
TableBase::PostConstructor(int nRows,
			   int nColumns, const bool * pbIconColumns)
{

    // Save the column information
    m_nLastWidth = -1;		// Reset to force column width recalculation
    m_nColumns = nColumns;
    delete m_pbIconColumns;
    m_pbIconColumns = new bool[nColumns];
    memcpy(m_pbIconColumns, pbIconColumns, nColumns);

    // Set up the table
    color(FL_WHITE);
    selection_color(FL_SELECTION_COLOR);
    cols(nColumns);
    rows(nRows);

    // Set full row selection
    feature_add(FLVF_ROW_SELECT);
}
