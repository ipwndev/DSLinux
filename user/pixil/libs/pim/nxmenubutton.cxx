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
#include <nxmenubutton.h>

#define ARROW_BUTTON_W 13
#define ARROW_BUTTON_H 10

NxMenuButton::NxMenuButton(int x, int y, int w, int h, char *l,
			   int scroll_size):
NxMenu_(x, y, w, h, l)
{

    move = false;
    scrollsize = scroll_size;

    // Provide the "look-and-feel"
    color(NxApp::Instance()->getGlobalColor(BUTTON_FACE));
    selection_color(NxApp::Instance()->getGlobalColor(HILIGHT));
    labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    textcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    box(FL_SHADOW_BOX);
    down_box(FL_NO_BOX);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    NxApp::Instance()->def_font(this);

    textfont(DEFAULT_TEXT_FONT);
    textsize(DEFAULT_TEXT_SIZE);

    value(0);
}				// end of NxMenuButton::NxMenuButton()

const NxMenuItem *
NxMenuButton::popup()
{
    const NxMenuItem *m = NULL;

    Fl_Color c = color();
    const NxMenuItem *p = menu();
    int val_ = -1;
    bool text_flag = false;

    if (!text() || 0 > value()) {
	for (int idx = 0; idx < size() - 1; idx++) {
	    if (0 == strcmp(label(), p[idx].label())) {
		text_flag = true;
		value(idx);
		break;
	    }
	}
    } else
	text_flag = true;

    if (text_flag) {
	for (int idx = 0; idx < size() - 1; idx++) {
	    if (p[idx].label()) {
		if (0 == strcmp(p[idx].label(), text())) {
		    val_ = idx;
		    break;
		}
	    }
	}
    }

    if (!box() || type()) {
	color(NxApp::Instance()->getGlobalColor(APP_BG));
	if (-1 == val_)
	    m = menu()->popup(Fl::event_x(), Fl::event_y(), scrollsize,
			      label(), mvalue(), this);
	else
	    m = menu()->popup(Fl::event_x(), Fl::event_y(), scrollsize,
			      label(), &p[val_], this);
    } else {
	color(NxApp::Instance()->getGlobalColor(APP_BG));
	if (-1 == val_)
	    m = menu()->pulldown(x(), y(), w(), h(), scrollsize, 0, this);
	else
	    m = menu()->pulldown(x(), y() + (val_ * h()) + (val_ * 2) + h(),
				 w(), h(), scrollsize, &p[val_], this);
    }

    color(c);

    picked(m);
    return m;
}

// Murphy
const NxMenuItem *
NxMenuButton::popup(int xx, int yy)
{
    const NxMenuItem *m;

    Fl_Color c = color();

    if (!box() || type()) {
	color(NxApp::Instance()->getGlobalColor(APP_BG));
	m = menu()->popup(xx, yy, scrollsize, label(), mvalue(), this);
    } else {
	color(NxApp::Instance()->getGlobalColor(APP_BG));
	m = menu()->pulldown(x(), y(), w(), h(), scrollsize, 0, this);
    }

    color(c);

    picked(m);
    return m;
}

int
NxMenuButton::handle(int e)
{
    if (!menu() || !menu()->text)
	return 0;
    switch (e) {
    case FL_ENTER:
    case FL_LEAVE:
	return (box() && !type())? 1 : 0;
    case FL_PUSH:
	if (!box()) {
	    if (Fl::event_button() != 3)
		return 0;
	} else if (type()) {
	    if (!(type() & (1 << (Fl::event_button() - 1))))
		return 0;
	}
	popup();
	return 1;
    case FL_SHORTCUT:
	if (Fl_Widget::test_shortcut()) {
	    popup();
	    return 1;
	}
	return test_shortcut() != 0;
    default:
	return 0;
    }
}

void
NxMenuButton::draw()
{

    if (!box() || type())
	return;

    int _x = x() + w();
    int _y = y() + h();

    // Added because the labels were getting written on top of each other ---
    // if this is not the correct fix, feel free to implement the correct solution
    fl_color(NxApp::Instance()->getGlobalColor(APP_BG));
    fl_rectf(x(), y(), w(), h());

    Fl_Color c = fl_color();

    fl_color(color());
    fl_line(x(), _y, _x, _y);

    fl_rectf(_x - ARROW_BUTTON_W + 1,
	     _y - ARROW_BUTTON_H + 1, ARROW_BUTTON_W, ARROW_BUTTON_H - 1);

    fl_line(_x - ARROW_BUTTON_W + 2,
	    _y - ARROW_BUTTON_H, _x - 1, _y - ARROW_BUTTON_H);

    fl_color(NxApp::Instance()->getGlobalColor(APP_BG));

    fl_begin_polygon();

    _x = x() + w() - ARROW_BUTTON_W;
    _y = y() + h() - ARROW_BUTTON_H;

    fl_transformed_vertex(_x + 3, _y + 2);

    fl_transformed_vertex(_x + ARROW_BUTTON_W - 2, _y + 2);

    fl_transformed_vertex(_x + (ARROW_BUTTON_W / 2) + 1,
			  _y + ARROW_BUTTON_H - 3);

    fl_transformed_vertex(_x + 3, _y + 2);

    fl_end_polygon();

    fl_line(_x + 3, _y + 2, _x + ARROW_BUTTON_W - 2, _y + 2);

    fl_line(_x + ARROW_BUTTON_W - 2,
	    _y + 2, _x + (ARROW_BUTTON_W / 2) + 1, _y + ARROW_BUTTON_H - 3);

    fl_line(_x + 3,
	    _y + 2, _x + (ARROW_BUTTON_W / 2) + 1, _y + ARROW_BUTTON_H - 3);


    draw_label();

    fl_color(c);

}
