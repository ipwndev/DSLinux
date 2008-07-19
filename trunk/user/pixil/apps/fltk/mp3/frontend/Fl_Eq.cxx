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

//
// Equalizer Slider widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/Fl_Eq.H>
#include <FL/fl_draw.H>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <iostream.h>

void
Fl_Eq::_Fl_Eq()
{
    slider_size_ = 0;
    slider_ = 0;		// FL_UP_BOX;
}

Fl_Eq::Fl_Eq(int x, int y, int w, int h, const char *l)
    :
Fl_Valuator(x, y, w, h, l)
{
    percent1 = 33;
    percent2 = 33;
    percent3 = 34;
    color1 = FL_GREEN;
    color2 = FL_YELLOW;
    color3 = FL_RED;
    box(FL_DOWN_BOX);
    _Fl_Eq();
}

Fl_Eq::Fl_Eq(uchar t, int x, int y, int w, int h, const char *l)
    :
Fl_Valuator(x, y, w, h, l)
{
    percent1 = 33;
    percent2 = 33;
    percent3 = 34;
    color1 = FL_GREEN;
    color2 = FL_YELLOW;
    color3 = FL_RED;
    type(t);
    box(t == FL_HOR_NICE_SLIDER || t == FL_VERT_NICE_SLIDER ?
	FL_FLAT_BOX : FL_DOWN_BOX);
    _Fl_Eq();
}

void
Fl_Eq::slider_size(float v)
{
    if (v < 0)
	v = 0;
    if (v > 1)
	v = 1;
    if (slider_size_ != float (v)) {
	slider_size_ = float (v);
	damage(FL_DAMAGE_EXPOSE);
    }
}

void
Fl_Eq::bounds(float a, float b)
{
    if (minimum() != a || maximum() != b) {
	Fl_Valuator::bounds(a, b);
	damage(FL_DAMAGE_EXPOSE);
    }
}

void
Fl_Eq::Range(int p1, int p2)
{
    percent1 = p1;
    percent2 = p2;
    percent3 = 100 - (p1 + p2);
}

void
Fl_Eq::Colors(Fl_Color c1, Fl_Color c2, Fl_Color c3)
{
    color1 = c1;
    color2 = c2;
    color3 = c3;
}

void
Fl_Eq::slider_ver_lines(int x, int y, int w, int h, int W, Fl_Color c)
{

    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);

    int stepVal = 4;
    float stepBoxSize = 3;

    if (type() != FL_HORIZONTAL) {

	float curStep = 0.0;
	float p1 = percent1 / 100.0;
	float p2 = percent2 / 100.0;

	int convW = W / stepVal;

	for (int i = 1; i < (w - 1); i = i + stepVal, curStep++) {

	    c = color1;

	    if (curStep > (convW * p1)) {
		c = color2;
	    }

	    if (curStep > (convW * (p1 + p2))) {
		c = FL_RED;
	    }

	    draw_box(FL_FLAT_BOX, (x + i), (y + 1), (int) stepBoxSize,
		     (h - 2), c);

	}

    } else {
	if (w > 0 && h > 0) {
	    if (w < 17) {
		w = 17;
		slider_size(float (w) / float (W));
		slider_size_min_ = slider_size();
	    }
	    draw_box(FL_BORDER_BOX, x, y, w, h, c);
	    fl_color(FL_BLACK);
	    cx = x + w / 2;
	    fl_line(cx, cy - h / 2 + 4, cx, cy + h / 2 - 4);
	    fl_line(cx - 3, cy - h / 2 + 4, cx - 3, cy + h / 2 - 4);
	    fl_line(cx + 3, cy - h / 2 + 4, cx + 3, cy + h / 2 - 4);
	}
    }
}

void
Fl_Eq::slider_hor_lines(int x, int y, int w, int h, int W, Fl_Color c)
{

    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);

    int stepVal = 4;
    float stepBoxSize = 3;

    if (type() != FL_VERTICAL) {

	// 1. Green
	c = fl_color_cube(105 * FL_NUM_RED / 256,
			  255 * FL_NUM_GREEN / 256, 0 * FL_NUM_BLUE / 256);
	int curStep = 0;

	float val = value();

	int event_y = 0;

	if (event_y == 0)
	    event_y = abs((int) (val - Fl_Widget::h())) + y;
	else
	    event_y = Fl::event_y();

	for (int i = Fl_Widget::h() + y - stepVal; i > event_y;
	     i = i - stepVal, curStep++) {

	    // 2. Green-Yellow
	    if (curStep == (w / 4))
		c = fl_color_cube(140 * FL_NUM_RED / 256,
				  255 * FL_NUM_GREEN / 256,
				  0 * FL_NUM_BLUE / 256);
	    else if (curStep == (w / 4 + 1))
		c = fl_color_cube(173 * FL_NUM_RED / 256,
				  255 * FL_NUM_GREEN / 256,
				  47 * FL_NUM_BLUE / 256);
	    // 4. Yellow
	    else if ((curStep > (w / 4 + 1)) && (curStep < (w - (w / 2))))
		c = FL_YELLOW;
	    // 5. Orange
	    else if (curStep == (w - (w / 2)))
		c = fl_color_cube(255 * FL_NUM_RED / 256,
				  195 * FL_NUM_GREEN / 256, 0);
	    else if (curStep == (w - (w / 4) + 1))
		c = fl_color_cube(255 * FL_NUM_RED / 256,
				  105 * FL_NUM_GREEN / 256, 0);
	    // 6. Red
	    else if (curStep > (w - (w / 4) + 1))
		c = FL_RED;

	    if (event_y == 0)
		break;

	    draw_box(FL_FLAT_BOX, (x + 1), i, (w - 2), (int) stepBoxSize, c);
	}

    } else {

	if (w > 0 && h > 0) {
	    if (h < 17) {
		h = 17;
		slider_size(float (h) / float (W));
		slider_size_min_ = slider_size();
	    }
	    draw_box(FL_BORDER_BOX, x, y, w, h, c);
	    fl_color(FL_BLACK);
	    cy = y + h / 2;
	    fl_line(cx + w / 2 - 4, cy, cx - w / 2 + 4, cy);
	    fl_line(cx + w / 2 - 4, cy - 3, cx - w / 2 + 4, cy - 3);
	    fl_line(cx + w / 2 - 4, cy + 3, cx - w / 2 + 4, cy + 3);
	}
    }

}

int
Fl_Eq::scrollvalue(int p, int w, int t, int l)
{
//      p = position, first line displayed
//      w = window, number of lines displayed
//      t = top, number of first line
//      l = length, total number of lines
    step(1, 1);
    if (p + w > t + l)
	l = p + w - t;
    slider_size(w >= l ? 1.0 : float (w) / float (l));
#ifdef PDA
    if (slider_size() < slider_size_min_) {
	slider_size(slider_size_min_);
    }
#endif
    bounds(t, l - w + t);
    return value(p);
}

// All slider interaction is done as though the slider ranges from
// zero to one, and the left (bottom) edge of the slider is at the
// given position.  Since when the slider is all the way to the
// right (top) the left (bottom) edge is not all the way over, a
// position on the widget itself covers a wider range than 0-1,
// actually it ranges from 0 to 1/(1-size).

void
Fl_Eq::draw_bg(int x, int y, int w, int h)
{

    if (!(damage() & FL_DAMAGE_ALL)) {	// not a complete redraw
	draw_box();
    }
    Fl_Color black = active_r()? FL_BLACK : FL_INACTIVE_COLOR;
    if (type() == FL_VERT_NICE_SLIDER) {
	draw_box(FL_THIN_DOWN_BOX, x + w / 2 - 2, y, 4, h, black);
    } else if (type() == FL_HOR_NICE_SLIDER) {
	draw_box(FL_THIN_DOWN_BOX, x, y + h / 2 - 2, w, 4, black);
    }
}

void
Fl_Eq::draw(int x, int y, int w, int h)
{

    float val;

    if (minimum() == maximum())
	val = 0.5;
    else {
	val = (value() - minimum()) / (maximum() - minimum());
	if (val > 1.0)
	    val = 1.0;
	else if (val < 0.0)
	    val = 0.0;
    }
    int W = (horizontal()? w : h);
    int X, S;
    if (type() == FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {
	S = int (val * W + .5);
	if (minimum() > maximum()) {
	    S = W - S;
	    X = W - S;
	} else
	    X = 0;
    } else {
	S = int (slider_size_ * W + .5);
	int T = (horizontal()? h : w) / 2 + 1;
	if (type() == FL_VERT_NICE_SLIDER || type() == FL_HOR_NICE_SLIDER)
	    T += 4;
	if (S < T)
	    S = T;
	X = int (val * (W - S) + .5);
    }
    int xsl, ysl, wsl, hsl;
    if (horizontal()) {
	xsl = x + X;
	wsl = S;
	ysl = y;
	hsl = h;
    } else {
	ysl = y + X;
	hsl = S;
	xsl = x;
	wsl = w;
    }

    if (damage() & FL_DAMAGE_ALL) {	// complete redraw
	draw_bg(x, y, w, h);
    } else {			// partial redraw, clip off new position of slider

	//fl_clip(x, y, w, Fl::event_y());
	//  draw_bg(x, y, w, h);
	//  fl_pop_clip();

	if (X > 0) {
	    if (horizontal())
		fl_clip(x, ysl, X, hsl);
	    else
		fl_clip(xsl, y, wsl, hsl);
	    draw_bg(x, y, w, h);
	    fl_pop_clip();
	}
	if (X + S < W) {

	    if (horizontal()) {

		fl_clip(xsl + wsl, ysl, x + w - xsl - wsl, hsl);
	    } else {
		if (type() == 2)
		    fl_clip(xsl, y, wsl, Fl::event_y());
		else
		    fl_clip(xsl, ysl + hsl, wsl, y + h - ysl - hsl);
	    }
	    draw_bg(x, y, w, h);
	    fl_pop_clip();
	}

    }
    Fl_Boxtype box1 = slider();
    if (!box1) {
	box1 = (Fl_Boxtype) (box() & -2);
	if (!box1)
	    box1 = FL_UP_BOX;
    }
    if (type() == FL_VERT_NICE_SLIDER) {
	draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
	int d = (hsl - 4) / 2;
	draw_box(FL_THIN_DOWN_BOX, xsl + 2, ysl + d, wsl - 4, hsl - 2 * d,
		 selection_color());
    } else if (type() == FL_HOR_NICE_SLIDER) {
	draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
	int d = (wsl - 4) / 2;
	draw_box(FL_THIN_DOWN_BOX, xsl + d, ysl + 2, wsl - 2 * d, hsl - 4,
		 selection_color());
    } else {			// draw the slider box
#ifdef PDA
	Fl_Color col = selection_color();
	if (horizontal()) {
	    slider_ver_lines(xsl, ysl, wsl, hsl, W, col);
	} else {
	    slider_hor_lines(xsl, ysl, wsl, hsl, W, col);
	}
#else
	if (wsl > 0 && hsl > 0)
	    draw_box(box1, xsl, ysl, wsl, hsl, selection_color());
#endif
    }
    draw_label(xsl, ysl, wsl, hsl);
}

void
Fl_Eq::draw()
{
    if (damage() & FL_DAMAGE_ALL)
	draw_box();
    draw(x() + Fl::box_dx(box()),
	 y() + Fl::box_dy(box()),
	 w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
}

int
Fl_Eq::handle(int event, int x, int y, int w, int h)
{


    if (event == FL_PUSH)
	switch (event) {
	case FL_PUSH:
	    if (!Fl::event_inside(x, y, w, h))
		return 0;
	    handle_push();
	case FL_DRAG:{
		int W = (horizontal()? w : h);

		//int H = (horizontal() ? h : w);

		int mx =
		    (horizontal()? Fl::event_x() - x : Fl::event_y() - y);

		int S = int (slider_size_ * W + .5);

		int X;

		static int offcenter;

		if (type() == FL_HOR_FILL_SLIDER
		    || type() == FL_VERT_FILL_SLIDER) {

		    float val =
			(value() - minimum()) / (maximum() - minimum());

		    if (val >= 1.0)
			X = W;
		    else if (val <= 0.0)
			X = 0;
		    else
			X = int (val * W + .5);

		    if (event == FL_PUSH) {
			offcenter = mx - X;
			if (offcenter < -S / 2)
			    offcenter = 0;
			else if (offcenter > S / 2)
			    offcenter = 0;
			else
			    return 1;
		    }

		    S = 0;
		} else {
		    float val =
			(value() - minimum()) / (maximum() - minimum());

		    if (val >= 1.0)
			X = W - S;
		    else if (val <= 0.0)
			X = 0;
		    else
			X = int (val * (W - S) + .5);

		    if (event == FL_PUSH) {
			offcenter = mx - X;
			if (offcenter < 0)
			    offcenter = 0;
			else if (offcenter > S)
			    offcenter = S;
			else
			    return 1;
		    }
		}
		X = mx - offcenter;
		float v;
	      TRY_AGAIN:
		if (X < 0) {
		    X = 0;
		    offcenter = mx;
		    if (offcenter < 0)
			offcenter = 0;
		} else if (X > (W - S)) {
		    X = W - S;
		    offcenter = mx - X;
		    if (offcenter > S)
			offcenter = S;
		}
		v = round(X * (maximum() - minimum()) / (W - S) + minimum());
		// make sure a click outside the sliderbar moves it:
		if (event == FL_PUSH && v == value()) {
		    offcenter = S / 2;
		    event = FL_DRAG;
		    goto TRY_AGAIN;
		}
		handle_drag(clamp(v));
	    }
	    return 1;
	case FL_RELEASE:
	    handle_release();
	    return 1;
	default:
	    return 0;
	}

    return 0;
}

int
Fl_Eq::handle(int event)
{

    return handle(event,
		  x() + Fl::box_dx(box()),
		  y() + Fl::box_dy(box()),
		  w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
}
