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


#include <nxapp.h>
#include <FL/Enumerations.H>
#include <nxbutton.h>
#include <stdio.h>

NxButton::NxButton(int x, int y, int w, int h, char *l):
Fl_Button(x, y, w, h, l)
{

    move = true;
    save_y = y;

    // Provide the specific "look-and-feel"
    active = 1;
    color(NxApp::Instance()->getGlobalColor(BUTTON_FACE));
    selection_color(NxApp::Instance()->getGlobalColor(BUTTON_PUSH));
    labelcolor(NxApp::Instance()->getGlobalColor(BUTTON_TEXT));
    NxApp::Instance()->def_font((Fl_Widget *) this);
    align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_WRAP);
    value(0);
}				// end of NxMenuButton::NxMenuButton()

void
NxButton::activate(void)
{
    if (!active) {
	active = 1;
	this->redraw();
    }				// end of if 
    return;
}				// end of NxButton::activate(void)

void
NxButton::deactivate(void)
{
    if (active) {
	active = 0;
	set_flag(INACTIVE);
	this->redraw();
	clear_flag(INACTIVE);
    }				// end of if
    return;
}				// end of NxButton::activate(void)

void
NxButton::drawButtonCode(int x, int y, int w, int h, Fl_Color c)
{
    // draw main box

    fl_color(c);

    fl_begin_polygon();
    fl_transformed_vertex(x, y + 3);
    fl_transformed_vertex(x + 1, y + 2);
    fl_transformed_vertex(x + 2, y + 1);

    fl_transformed_vertex(x + w - 3, y);
    fl_transformed_vertex(x + w, y + 3);
    fl_transformed_vertex(x + w, y + h - 4);
    fl_transformed_vertex(x + w - 3, y + h);
    fl_transformed_vertex(x + 3, y + h);
    fl_transformed_vertex(x, y + h - 3);
    fl_end_polygon();

}

void
NxButton::draw(void)
{
    if (!active)
	set_flag(INACTIVE);

    // Fl_Button::draw();
    if (type() == FL_HIDDEN_BUTTON)
	return;

    if ((box() == FL_BORDER_BOX) || (box() == FL_FLAT_BOX)) {
	Fl_Button::draw();
	return;
    }

    Fl_Color col = value()? selection_color() : color();

    if (value()) {

	Fl_Color old_label_clr = labelcolor();
	labelcolor(contrast(color(), labelcolor()));

	//draw_box(down(box()),selection_color());

	drawButtonCode(x(), y(), w(), h(),
		       NxApp::Instance()->getGlobalColor(APP_BG));
	drawButtonCode(x() + 2, y() + 2, w(), h(), selection_color());

	draw_label(x() + 2, y() + 2, w(), h());
	labelcolor(old_label_clr);

    } else {

	//draw_box(box(), col);

	drawButtonCode(x() + 2, y() + 2, w(), h(), selection_color());
	drawButtonCode(x(), y(), w(), h(), col);

	draw_label();
    }

    if (!active)
	clear_flag(INACTIVE);

    return;
}				// end of if 

int
NxButton::handle(int event)
{
    return ((active) ? Fl_Button::handle(event) : 0);
}				// end of NxButton::handle(int)
