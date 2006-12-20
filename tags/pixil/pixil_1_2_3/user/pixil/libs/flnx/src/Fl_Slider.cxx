//
// "$Id$"
//
// Slider widget for the Fast Light Tool Kit (FLTK).
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

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>

void
Fl_Slider::_Fl_Slider()
{
    slider_size_ = 0;
    slider_ = 0;		// FL_UP_BOX;
}

Fl_Slider::Fl_Slider(int x, int y, int w, int h, const char *l)
    :
Fl_Valuator(x, y, w, h, l)
{
    box(FL_DOWN_BOX);
    _Fl_Slider();
}

Fl_Slider::Fl_Slider(uchar t, int x, int y, int w, int h, const char *l)
    :
Fl_Valuator(x, y, w, h, l)
{
    type(t);
    box(t == FL_HOR_NICE_SLIDER || t == FL_VERT_NICE_SLIDER ?
	FL_FLAT_BOX : FL_DOWN_BOX);
    _Fl_Slider();
}

void
Fl_Slider::slider_size(double v)
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
Fl_Slider::bounds(double a, double b)
{
    if (minimum() != a || maximum() != b) {
	Fl_Valuator::bounds(a, b);
	damage(FL_DAMAGE_EXPOSE);
    }
}

void
Fl_Slider::slider_ver_lines(int x, int y, int w, int h, int W, Fl_Color c)
{
    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);
    if (type() != FL_HORIZONTAL) {
	draw_box(FL_BORDER_BOX, x, y, w, h, c);
    } else {
	if (w > 0 && h > 0) {
#ifndef PDA
	    if (w < 17) {
		w = 17;
		slider_size(double (w) / double (W));
		slider_size_min_ = slider_size();
	    }
#else
	    slider_size_min_ = slider_size();
#endif
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
Fl_Slider::slider_hor_lines(int x, int y, int w, int h, int W, Fl_Color c)
{
    int cx = x + w / 2;
    int cy = y + h / 2;

    fl_color(FL_BLACK);
    if (type() != FL_VERTICAL) {
	draw_box(FL_BORDER_BOX, x, y, w, h, c);
    } else {
	if (w > 0 && h > 0) {
#ifndef PDA
	    if (h < 17) {
		h = 17;
		slider_size(double (h) / double (W));
		slider_size_min_ = slider_size();
	    }
#else
	    slider_size_min_ = slider_size();
#endif

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
Fl_Slider::scrollvalue(int p, int w, int t, int l)
{
//      p = position, first line displayed
//      w = window, number of lines displayed
//      t = top, number of first line
//      l = length, total number of lines
    step(1, 1);
    if (p + w > t + l)
	l = p + w - t;
    slider_size(w >= l ? 1.0 : double (w) / double (l));
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
Fl_Slider::draw_bg(int x, int y, int w, int h)
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
Fl_Slider::draw(int x, int y, int w, int h)
{
    double val;

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
	if (X > 0) {
	    if (horizontal())
		fl_clip(x, ysl, X, hsl);
	    else
		fl_clip(xsl, y, wsl, X);
	    draw_bg(x, y, w, h);
	    fl_pop_clip();
	}
	if (X + S < W) {
	    if (horizontal())
		fl_clip(xsl + wsl, ysl, x + w - xsl - wsl, hsl);
	    else
		fl_clip(xsl, ysl + hsl, wsl, y + h - ysl - hsl);
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
Fl_Slider::draw()
{
    if (damage() & FL_DAMAGE_ALL)
	draw_box();
    draw(x() + Fl::box_dx(box()),
	 y() + Fl::box_dy(box()),
	 w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
}

int
Fl_Slider::handle(int event, int x, int y, int w, int h)
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
		double val = (value() - minimum()) / (maximum() - minimum());

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
		double val = (value() - minimum()) / (maximum() - minimum());

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
	    double v;
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
Fl_Slider::handle(int event)
{
    return handle(event,
		  x() + Fl::box_dx(box()),
		  y() + Fl::box_dy(box()),
		  w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
}

//
// End of "$Id$".
//
