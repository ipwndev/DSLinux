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
#include <nxslider.h>

void
NxSlider::_NxSlider()
{
    slider_size_ = 0;
    slider_ = 0;		// FL_UP_BOX;
}

NxSlider::NxSlider(int x, int y, int w, int h, const char *l):
Fl_Valuator(x, y, w, h, l)
{

    // Provide the "look-and-feel"
    color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
    selection_color(NxApp::Instance()->getGlobalColor(SCROLL_FACE));
    box(FL_SHADOW_BOX);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    value(0);
    movable_ = true;
}				// end of NxSlider::NxSlider()


void
NxSlider::draw()
{
    if (damage() & FL_DAMAGE_ALL)
	draw_box();
    draw(x() + Fl::box_dx(box()),
	 y() + Fl::box_dy(box()),
	 w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));

}

void
NxSlider::slider_size(float v)
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
NxSlider::bounds(float a, float b)
{
    if (minimum() != a || maximum() != b) {
	Fl_Valuator::bounds(a, b);
	damage(FL_DAMAGE_EXPOSE);
    }
}

int
NxSlider::scrollvalue(int p, int w, int t, int l)
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

void
NxSlider::draw_bg(int x, int y, int w, int h)
{
    if (!(damage() & FL_DAMAGE_ALL)) {	// not a complete redraw
	if (type() == FL_VERT_NICE_SLIDER || type() == FL_HOR_NICE_SLIDER) {
	    fl_color(color());
	    fl_rectf(x, y, w, h);
	} else {
	    fl_color(color());
	    fl_rectf(x + 1, y, w - 2, h);
	}
    }
    Fl_Color black = active_r()? FL_BLACK : FL_INACTIVE_COLOR;
    if (type() == FL_VERT_NICE_SLIDER) {
	draw_box(FL_THIN_DOWN_BOX, x + w / 2 - 2, y, 4, h, black);
    } else if (type() == FL_HOR_NICE_SLIDER) {
	draw_box(FL_THIN_DOWN_BOX, x, y + h / 2 - 2, w, 4, black);
    }
}

void
NxSlider::draw(int x, int y, int w, int h)
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
	S = int (slider_size() * W + .5);
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

    draw_bg(x, y, w, h);

    /*
       if (damage()&FL_DAMAGE_ALL) { // complete redraw
       draw_bg(x, y, w, h);
       } else { // partial redraw, clip off new position of slider
       // for moving scrollbar down
       if (X > 0) {
       if (horizontal()) 
       fl_clip(x, ysl, X, hsl);
       else {
       fl_clip(xsl, y, wsl, X+3);
       }

       draw_bg(x, y, w, h);
       fl_pop_clip();
       }

       // for moving scrollbar up
       if (X+S < W) {
       if (horizontal()) 
       fl_clip(xsl+wsl, ysl, x+w-xsl-wsl, hsl);
       else {
       fl_clip(xsl, ysl+hsl+10, wsl, y+h-ysl-hsl);
       }

       draw_bg(x, y, w, h);
       fl_pop_clip();
       }
       }
     */

    Fl_Boxtype box1 = slider();
    if (!box1) {
	box1 = (Fl_Boxtype) (box() & -2);
	if (!box1)
	    box1 = FL_UP_BOX;
    }
    if (type() == FL_VERT_NICE_SLIDER) {
	wsl += 2;

	fl_color(selection_color());
	fl_begin_polygon();
	fl_transformed_vertex(xsl, ysl + 3);
	fl_transformed_vertex(xsl + 1, ysl + 2);
	fl_transformed_vertex(xsl + 2, ysl + 1);

	fl_transformed_vertex(xsl + wsl - 3, ysl);
	fl_transformed_vertex(xsl + wsl, ysl + 3);
	fl_transformed_vertex(xsl + wsl, ysl + hsl - 8);
	fl_transformed_vertex(xsl + wsl - 3, ysl + hsl - 4);
	fl_transformed_vertex(xsl + 3, ysl + hsl - 4);
	fl_transformed_vertex(xsl, ysl + hsl - 7);
	fl_end_polygon();

	// draw the lines
	int y_cen = (ysl + hsl) / 2;

	fl_color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
	fl_line(xsl + 4, y_cen - 2, xsl + wsl - 8, y_cen - 2);
	fl_line(xsl + 4, y_cen + 2, xsl + wsl - 8, y_cen + 2);
    } else if (type() == FL_HOR_NICE_SLIDER) {

	hsl += 2;
	fl_color(selection_color());
	fl_begin_polygon();
	fl_transformed_vertex(xsl, ysl + 3);
	fl_transformed_vertex(xsl + 1, ysl + 2);
	fl_transformed_vertex(xsl + 2, ysl + 1);

	fl_transformed_vertex(xsl + wsl - 3, ysl);
	fl_transformed_vertex(xsl + wsl, ysl + 3);
	fl_transformed_vertex(xsl + wsl, ysl + hsl - 8);
	fl_transformed_vertex(xsl + wsl - 3, ysl + hsl - 4);
	fl_transformed_vertex(xsl + 3, ysl + hsl - 4);
	fl_transformed_vertex(xsl, ysl + hsl - 7);
	fl_end_polygon();

	// draw the lines
	int x_cen = (xsl + (xsl + wsl)) / 2;

	fl_color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
	fl_line(x_cen - 2, ysl + 4, x_cen - 2, ysl + hsl - 8);
	fl_line(x_cen + 2, ysl + 4, x_cen + 2, ysl + hsl - 8);

    } else {			// draw the slider box
	Fl_Color col = selection_color();
	if (horizontal()) {
	    slider_ver_lines(xsl, ysl, wsl, hsl, W, col);
	} else {
	    slider_hor_lines(xsl, ysl, wsl, hsl, W, col);
	}
    }
    draw_label(xsl, ysl, wsl, hsl);
}

void
NxSlider::slider_ver_lines(int x, int y, int w, int h, int W, Fl_Color c)
{
    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);
    if (type() != FL_HORIZONTAL) {
	draw_box(FL_BORDER_BOX, x, y, w, h, c);
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
NxSlider::slider_hor_lines(int x, int y, int w, int h, int W, Fl_Color c)
{
    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);
    if (type() != FL_VERTICAL) {
	draw_box(FL_BORDER_BOX, x, y, w, h, c);
    } else {
	if (w > 0 && h > 0) {
	    if (h < 17) {
		h = 17;
		slider_size(float (h) / float (W));
		slider_size_min_ = slider_size();
	    }

	    fl_color(c);

	    fl_rectf(x + 3, y + 3, w - 6, h - 6);

	    // draw top rounded border
	    fl_line(x + 4, y + 2, x + w - 5, y + 2);

	    // draw bottom rounded border
	    fl_line(x + 4, y + h - 3, x + w - 5, y + h - 3);

	    fl_color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));

	    cy = y + h / 2;

	    fl_line(cx + w / 2 - 6, cy, cx - w / 2 + 5, cy);
	    fl_line(cx + w / 2 - 6, cy - 3, cx - w / 2 + 5, cy - 3);
	    fl_line(cx + w / 2 - 6, cy + 3, cx - w / 2 + 5, cy + 3);
	}
    }
}

int
NxSlider::handle(int event, int x, int y, int w, int h)
{
    switch (event) {
    case FL_PUSH:
	if (!Fl::event_inside(x, y, w, h))
	    return 0;
	handle_push();
    case FL_DRAG:{
	    int W = (horizontal()? w : h);
	    //int H = (horizontal() ? h : w);
	    int mx = (horizontal()? Fl::event_x() - x : Fl::event_y() - y);
	    int S = int (slider_size_ * W + .5);
	    int X;
	    static int offcenter;
	    if (type() == FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {
		float val = (value() - minimum()) / (maximum() - minimum());

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
		float val = (value() - minimum()) / (maximum() - minimum());

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
}

int
NxSlider::handle(int event)
{
    return handle(event,
		  x() + Fl::box_dx(box()),
		  y() + Fl::box_dy(box()),
		  w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
}
