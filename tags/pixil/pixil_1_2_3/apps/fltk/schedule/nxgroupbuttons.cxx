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


#include <stdio.h>
#include <FL/fl_draw.H>
#include "nxgroupbuttons.h"
#include <nxapp.h>

GroupButton::GroupButton(int X, int Y, int W, int H, int T)
    :
NxButton(X, Y, W, H)
{
    type = T;
    save_x = X;
    save_w = W;
    save_h = H;
}

void
GroupButton::draw()
{
    NxButton::draw();

#ifdef PDA
    if (value()) {
	fl_color(NxApp::Instance()->getGlobalColor(BUTTON_FACE));
    } else {
	fl_color(NxApp::Instance()->getGlobalColor(BUTTON_TEXT));
    }
    switch (type) {
    case type_daily:
	fl_rectf(x() + w() / 2 - 2, y() + h() / 2 - 1, 4, 4);
	break;
    case type_weekly:
	fl_rectf(x() + w() / 2 - 4, y() + h() / 2, 3, 3);
	fl_rectf(x() + w() / 2 + 1, y() + h() / 2, 3, 3);
	break;
    case type_monthly:
	W = 1;
	H = 4;
	Y = y() + 4;
	for (idx = 0, X = x() + 1; idx <= 3; idx++, X += 2) {
	    fl_rectf(X, Y, W, H);
	}
	Y = y() + 9;
	for (idx = 0, X = x() + 1; idx <= 3; idx++, X += 2) {
	    fl_rectf(X, Y, W, H);
	}
	break;
    case type_yearly:
	W = 1;
	H = 2;
	for (jdx = 1, Y = y() + 2; jdx <= 4; jdx++, Y += (H + 1)) {
	    for (idx = 0, X = x() + 1; idx <= 3; idx++, X += 2)
		fl_rectf(X, Y, W, H);
	}
	break;
    default:
	break;
    }
#else

    fl_color(FL_BLACK);
    switch (type) {
    case type_daily:
	fl_rectf(x() + w() / 2 - 2, y() + h() / 2 - 1, 4, 4);
	break;
    case type_weekly:
	fl_rectf(x() + w() / 2 - 4, y() + h() / 2 - 1, 3, 3);
	fl_rectf(x() + w() / 2 + 1, y() + h() / 2 - 1, 3, 3);
	break;
    case type_monthly:
	int Y;
	int W;
	int H;
	W = 1;
	H = 4;
	Y = y() + 4;
	for (int idx = 0, X = x() + 1; idx <= 3; idx++, X += 2) {
	    fl_rectf(X, Y, W, H);
	}
	Y = y() + 9;
	for (int idx = 0, X = x() + 1; idx <= 3; idx++, X += 2) {
	    fl_rectf(X, Y, W, H);
	}
	break;
    case type_yearly:
	break;
    default:
	break;
    }

#endif
}
