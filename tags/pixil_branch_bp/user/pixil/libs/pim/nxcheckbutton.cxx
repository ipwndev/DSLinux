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
#include <nxapp.h>
#include <FL/Enumerations.H>
#include <nxcheckbutton.h>

void
nx_cb_draw(int x, int y, int w, int h, Fl_Color c)
{
    Fl_Color lcolor = NxApp::Instance()->getGlobalColor(APP_FG);
    fl_color(lcolor);
    fl_line(x + 1, y + 1, x + w - 2, y + h - 2);
    fl_line(x + w - 2, y + 1, x + 1, y + h - 2);
}

NxCheckButton::NxCheckButton(int x, int y, char *l):
Fl_Check_Button(x, y, CHECK_W, CHECK_H, l)
{

    move = true;

    // Provide the "look-and-feel"
    color(NxApp::Instance()->getGlobalColor(APP_BG));
    selection_color(NxApp::Instance()->getGlobalColor(APP_SEL));
    labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    NxApp::Instance()->def_font((Fl_Widget *) this);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    value(0);
}				// end of NxCheckButton::NxCheckButton()

NxCheckButton::NxCheckButton(int x, int y, int w, int h, char *l):
Fl_Check_Button(x, y, w, h, l)
{

    // Provide the "look-and-feel"
    color(NxApp::Instance()->getGlobalColor(APP_BG));
    selection_color(NxApp::Instance()->getGlobalColor(APP_SEL));
    labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    NxApp::Instance()->def_font((Fl_Widget *) this);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    value(0);
}				// end of NxCheckButton::NxCheckButton()

void
NxCheckButton::draw(void)
{
    int bh = y() + (h() / 2) / 2;
    int d = h() / 6;
    int W = w() < h()? w() : h();

    draw_box(FL_DOWN_BOX, x(), bh, h() / 2, h() / 2, color());


    if (value()) {
	nx_cb_draw(x(), bh, h() / 2, h() / 2, color());

    }
#ifdef PDA
    labelcolor(labelcolor());
#endif

    draw_label(x() + W - d, y(), w() - W + d, h());

}
