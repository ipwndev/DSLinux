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
#include <FL/fl_draw.H>
#include <nxscrollbar.h>
#include <iostream>

NxScrollbar::NxScrollbar(int x, int y, int w, int h, const char *l):
NxSlider(x, y, w, h, l)
{
    box(FL_FLAT_BOX);
    slider(FL_UP_BOX);
    linesize_ = 16;
    pushed_ = 0;
    step(1);
}

#define INITIALREPEAT .5
#define REPEAT .05

void
NxScrollbar::increment_cb()
{
    int i;
    int W = horizontal()? w() : h();
    int S = int (slider_size() * W + .5);

    switch (pushed_) {
    case 1:
	i = -linesize_;
	break;
    default:
	i = linesize_;
	break;
    case 3:
	i = -int (S * (maximum() - minimum()) / W);
	break;
    case 4:
	i = int (S * (maximum() - minimum()) / W);
	break;
    }
    if (maximum() < minimum() && pushed_ < 3)
	i = -i;
    handle_drag(clamp(value() + i));
}

void
NxScrollbar::timeout_cb(void *v)
{
    NxScrollbar *s = (NxScrollbar *) v;
    s->increment_cb();
    //Fl::add_timeout(REPEAT, timeout_cb, s);
}

int
NxScrollbar::handle(int event)
{
    // area of scrollbar:
    int area;
    int X = x();
    int Y = y();
    int W = w();
    int H = h();
    int SX = X;
    int SY = Y;
    int SW = W;
    int SH = H;

    // adjust slider area to be inside the arrow buttons:
    if (horizontal()) {
	if (W >= 3 * H) {
	    X += H;
	    W -= 2 * H;
	}
    } else {
	if (H >= 3 * W) {
	    Y += W;
	    H -= 2 * W;
	}
    }

    // which widget part is highlighted?
    int mx = Fl::event_x();
    int my = Fl::event_y();
    if (!Fl::event_inside(SX, SY, SW, SH))
	area = 0;
    else if (horizontal()) {
	if (mx < X)
	    area = 1;
	else if (mx >= X + W)
	    area = 2;
	else {
	    int sliderx;
	    int S = int (slider_size() * W + .5);
	    float val = (value() - minimum()) / (maximum() - minimum());
	    if (val >= 1.0)
		sliderx = W - S;
	    else if (val <= 0.0)
		sliderx = 0;
	    else
		sliderx = int (val * (W - S) + .5);

	    if (mx < X + sliderx)
		area = 3;
	    else if (mx >= X + sliderx + S)
		area = 4;
	    else
		area = 5;
	}
    } else {
	if (mx < X || mx >= X + W)
	    area = 0;
	else if (my < Y)
	    area = 1;
	else if (my >= Y + H)
	    area = 2;
	else {
	    int slidery;
	    int S = int (slider_size() * H + .5);
	    float val = (value() - minimum()) / (maximum() - minimum());
	    if (val >= 1.0)
		slidery = H - S;
	    else if (val <= 0.0)
		slidery = 0;
	    else
		slidery = int (val * (H - S) + .5);

	    if (my < Y + slidery)
		area = 3;
	    else if (my >= Y + slidery + S)
		area = 4;
	    else
		area = 5;
	}
    }
    switch (event) {
    case FL_ENTER:
    case FL_LEAVE:
	return 1;
    case FL_RELEASE:
	damage(FL_DAMAGE_EXPOSE);
	if (pushed_) {
	    //Fl::remove_timeout(timeout_cb, this);
	    pushed_ = 0;
	}
	handle_release();
	return 1;
    case FL_PUSH:
	if (pushed_)
	    return 1;
	if (area != 5)
	    pushed_ = area;
	if (pushed_) {
	    handle_push();
	    //Fl::add_timeout(INITIALREPEAT, timeout_cb, this);
	    increment_cb();
	    damage(FL_DAMAGE_EXPOSE);
	    return 1;
	}
	return NxSlider::handle(event, X, Y, W, H);
    case FL_DRAG:
	if (pushed_)
	    return 1;
	return NxSlider::handle(event, X, Y, W, H);
    case FL_SHORTCUT:{
	    int v = value();
	    int ls = maximum() >= minimum()? linesize_ : -linesize_;
	    if (horizontal()) {
		switch (Fl::event_key()) {
		case FL_Left:
		    v -= ls;
		    break;
		case FL_Right:
		    v += ls;
		    break;
		default:
		    return 0;
		}
	    } else {		// vertical
		switch (Fl::event_key()) {
		case FL_Up:
		    v -= ls;
		    break;
		case FL_Down:
		    v += ls;
		    break;
		case FL_Page_Up:
		    if (slider_size() >= 1.0)
			return 0;
		    v -= int ((maximum() - minimum()) * slider_size() / (1.0 -
									 slider_size
									 ()));
		    v += ls;
		    break;
		case FL_Page_Down:
		    if (slider_size() >= 1.0)
			return 0;
		    v += int ((maximum() - minimum()) * slider_size() / (1.0 -
									 slider_size
									 ()));
		    v -= ls;
		    break;
		case FL_Home:
		    v = int (minimum());
		    break;
		case FL_End:
		    v = int (maximum());
		    break;
		default:
		    return 0;
		}
	    }
	    v = int (clamp(v));
	    if (v != value()) {
		NxSlider::value(v);
		value_damage();
		do_callback();
	    }
	    return 1;
	}
    }
    return 0;
}

#include <stdio.h>

void
NxScrollbar::draw()
{

    box(FL_BORDER_BOX);
    Fl_Color col = NxApp::Instance()->getGlobalColor(SCROLL_TRAY);
    Fl_Color sel_col = selection_color();

    int X = x() + Fl::box_dx(box());
    int Y = y() + Fl::box_dy(box());
    int W = w() - Fl::box_dw(box());
    int H = h() - Fl::box_dh(box());

    if (damage() & FL_DAMAGE_ALL) {
	Fl_Color clr = fl_color();

	fl_color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
	fl_rectf(X, Y, W, H);
	fl_color(NxApp::Instance()->getGlobalColor(SCROLL_FACE));
	fl_rect(X, Y, W, H);

	fl_color(clr);
    }


    if (horizontal()) {

	// the slider button
	if (W < 3 * H) {
	    NxSlider::draw(X - 1, Y, W + 2, H);
	    return;
	}
	NxSlider::draw(X + H - 1, Y, W - 2 * H + 2, H);
	// draw the boxes for the buttons
	if (damage()) {
	    //      draw_box((pushed_ == 1) ? FL_BLACK_BOX : FL_BORDER_BOX,
	    //               X, Y, H, H, col);
	    //      draw_box((pushed_ == 2) ? FL_BLACK_BOX : FL_BORDER_BOX,
	    //               X+W-H, Y, H, H, col);

	    Fl_Color clr = fl_color();

	    // Top Box
	    fl_color((pushed_ == 1) ? col : sel_col);
	    fl_rectf(X, Y, H, H);	// box
	    fl_rect(X + W - H, Y, H, H);	// border

	    // Bottom Box
	    fl_color((pushed_ == 2) ? col : sel_col);
	    fl_rectf(X + W - H, Y, H, H);	// box
	    fl_rect(X, Y, H, H);	// border

	    fl_color(clr);

	    if (active_r())
		fl_color(labelcolor());
	    else
		fl_color(inactive(labelcolor()));
	    int w1 = (H - 1) | 1;	// use odd sizes only
	    int Y1 = Y + w1 / 2;
	    int W1 = w1 / 3;
	    int X1 = X + w1 / 2 + W1 / 2;
	    //or arrows on buttons
	    if (pushed_ == 1)
		fl_color(FL_WHITE);
	    else
		fl_color(FL_BLACK);
	    fl_polygon(X1 - W1, Y1, X1, Y1 - W1, X1, Y1 + W1);
	    X1 = X + W - (X1 - X) - 1;
	    if (pushed_ == 2)
		fl_color(FL_WHITE);
	    else
		fl_color(FL_BLACK);
	    fl_polygon(X1 + W1, Y1, X1, Y1 + W1, X1, Y1 - W1);
	}
    } else {			// vertical
	if (H < 3 * W) {
	    NxSlider::draw(X, Y - 1, W, H + 2);
	    return;
	}
	NxSlider::draw(X, Y + W - 1, W, H - 2 * W + 2);
	if (damage()) {

	    /*
	       draw_box((pushed_ == 1) ? FL_BLACK_BOX : FL_BORDER_BOX,
	       X, Y, W, W, col);
	       draw_box((pushed_ == 2) ? FL_BLACK_BOX : FL_BORDER_BOX,
	       X, Y+H-W, W, W, col);
	     */

	    Fl_Color clr = fl_color();

	    // Top Box
	    fl_color((pushed_ == 1) ? sel_col : col);
	    fl_rectf(X, Y, W, W);	// box
	    fl_color(sel_col);
	    fl_rect(X, Y, W, W);	// border

	    // Bottom Box
	    fl_color((pushed_ == 2) ? sel_col : col);
	    fl_rectf(X, Y + H - W, W, W);	// box
	    fl_color(sel_col);
	    fl_rect(X, Y + H - W, W, W);	// border      

	    fl_color(clr);

	    /*
	       if (active_r())
	       fl_color(labelcolor());
	       else
	       fl_color(labelcolor() | 8);
	       ******* */

	    int w1 = (W - 1) | 1;	// use odd sizes only
	    int X1 = X + w1 / 2 + 1;

	    int W1 = w1 / 3 + 1;
	    int Y1 = Y + w1 / 2 + W1 / 2;

	    // draw the arrow on the buttons

	    if (pushed_ == 1)
		fl_color(col);
	    else
		fl_color(sel_col);


	    fl_polygon(X1, Y1 - W1, X1 + W1, Y1 + 1, X1 - W1, Y1);
	    Y1 = Y + H - (Y1 - Y) - 1;


	    if (pushed_ == 2)
		fl_color(col);
	    else
		fl_color(sel_col);

	    fl_polygon(X1, Y1 + W1, X1 - W1, Y1, X1 + W1, Y1);
	}
    }
}
