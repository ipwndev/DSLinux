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
// Schedule Hours displayed to the left of a day's appointments //
//--------------------------------------------------------------//
#include "config.h"
#include <cstdio>
#include <FL/fl_draw.H>
#include "PixilDT.h"
#include "ScheduleHours.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ScheduleHours::ScheduleHours(int nX, int nY, int nWidth, int nHourHeight)
    :
Fl_Box(nX, nY, nWidth, 24 * nHourHeight)
{
    m_nHourHeight = nHourHeight;
    labelfont(FL_HELVETICA);
    labelsize((4 * labelsize()) / 5);
    box(FL_BORDER_BOX);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ScheduleHours::~ScheduleHours()
{
}


//--------------------------------------------------------------//
// Override the virtual draw function                           //
//--------------------------------------------------------------//
void
ScheduleHours::draw()
{
    char szLabel[24];
    int i;
    int nDashWidth;
    int nHeight;
    int nHour;
    int nWidth;
    int nX;
    int nY;
    string strAM;
    string strPM;

    // Get the wording for AM/PM
    PixilDT::GetAMPM(strAM, strPM);

    // Do the base widget stuff
    draw_box();
    draw_label();

    // Now do the custom stuff
    fl_font(labelfont(), labelsize());
    fl_color(FL_BLACK);

    // Get the width of the small line
    fl_measure("a", nDashWidth, nHeight);

    // Draw each hour's text
    for (i = 0; i < 24; ++i) {
	// Draw each hour as needed
	if (strAM.length() != 0) {
	    // Twelve hour clock
	    nHour = i % 12;
	    if (nHour == 0) {
		nHour = 12;
	    }
	    sprintf(szLabel,
		    "%d%c00%s",
		    nHour,
		    PixilDT::GetTimeSeparator(),
		    i == 0 ? strAM.c_str() : i == 12 ? strPM.c_str() : "");
	} else {
	    // 24 hour clock
	    sprintf(szLabel, "%d%c00", i, PixilDT::GetTimeSeparator());
	}

	// Get the size of the label
	nWidth = w();
	nHeight = h() / 24;
	fl_measure(szLabel, nWidth, nHeight);

	// Determine where to draw the label
	nX = x() + (w() - nWidth) / 2;
	nY = y() + i * m_nHourHeight + (m_nHourHeight + nHeight) / 2 -
	    fl_descent();
	fl_draw(szLabel, nX, nY);

	// Draw the small line
	nHeight = y() + i * m_nHourHeight + (m_nHourHeight >> 1);
	fl_line(x() + w() - nDashWidth, nHeight, x() + w() - 1, nHeight);

	// Draw the separator line
	if (i != 24 - 1) {
	    nHeight = y() + (i + 1) * m_nHourHeight;
	    fl_line(x(), nHeight, x() + w() - 1, nHeight);
	}
    }
}
