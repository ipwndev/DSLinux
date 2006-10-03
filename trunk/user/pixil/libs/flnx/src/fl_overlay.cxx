//
// "$Id$"
//
// Overlay support for the Fast Light Tool Kit (FLTK).
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

// Extremely limited "overlay" support.  You can use this to drag out
// a rectangle in response to mouse events.  It is your responsibility
// to erase the overlay before drawing anything that might intersect
// it.

#include <FL/x.H>
#include <FL/fl_draw.H>

static int px,py,pw,ph;

static void draw_current_rect() {
#ifdef WIN32
  int old = SetROP2(fl_gc, R2_NOT);
  fl_rect(px, py, pw, ph);
  SetROP2(fl_gc, old);
#else
#ifdef NANO_X //tanghao
        GR_GC_ID temp_gc = GrCopyGC(fl_gc);
        GrSetGCMode(temp_gc,GR_MODE_XOR);
	GrSetGCForeground(temp_gc,0xffffff);
	GrRect(fl_window,temp_gc,px,py,pw,ph);
	GrDestroyGC(temp_gc);
#else
  XSetFunction(fl_display, fl_gc, GXxor);
  XSetForeground(fl_display, fl_gc, 0xffffffff);
  XDrawRectangle(fl_display, fl_window, fl_gc, px, py, pw, ph);
  XSetFunction(fl_display, fl_gc, GXcopy);
#endif 
#endif
}

void fl_overlay_clear() {
  if (pw > 0) {draw_current_rect(); pw = 0;}
}

void fl_overlay_rect(int x, int y, int w, int h) {
  if (w < 0) {x += w; w = -w;} else if (!w) w = 1;
  if (h < 0) {y += h; h = -h;} else if (!h) h = 1;
  if (pw > 0) {
    if (x==px && y==py && w==pw && h==ph) return;
    draw_current_rect();
  }
  px = x; py = y; pw = w; ph = h;
  draw_current_rect();
}

//
// End of "$Id$".
//
