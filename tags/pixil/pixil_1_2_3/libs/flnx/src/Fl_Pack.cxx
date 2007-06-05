//
// "$Id$"
//
// Packing widget for the Fast Light Tool Kit (FLTK).
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

// Based on code by Curtis Edwards
// Group that compresses all it's children together and resizes to surround
// them on each redraw (only if box() is zero)
// Bugs: ?

#include <FL/Fl.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>

Fl_Pack::Fl_Pack(int x,int y,int w ,int h,const char *l)
: Fl_Group(x, y, w, h, l) {
  resizable(0);
  spacing_ = 0;
  // type(VERTICAL); // already set like this
}

void Fl_Pack::draw() {
  int tx = x()+Fl::box_dx(box());
  int ty = y()+Fl::box_dy(box());
  int tw = w()-Fl::box_dw(box());
  int th = h()-Fl::box_dh(box());
  int current_position = horizontal() ? tx : ty;
  int maximum_position = current_position;
  uchar d = damage();
  Fl_Widget*const* a = array();
  for (int i = children(); i--;) {
    Fl_Widget* o = *a++;
    if (o->visible()) {
      int X,Y,W,H;
      if (horizontal()) {
        X = current_position;
        W = o->w();
        Y = ty;
        H = th;
      } else {
        X = tx;
        W = tw;
        Y = current_position;
        H = o->h();
      }
      if (spacing_ && current_position>maximum_position &&
  	  (X != o->x() || Y != o->y() || d&FL_DAMAGE_ALL)) {
        fl_color(color());
        if (horizontal())
	  fl_rectf(maximum_position, ty, spacing_, th);
        else
	  fl_rectf(tx, maximum_position, tw, spacing_);
      }
      if (X != o->x() || Y != o->y() || W != o->w() || H != o->h()) {
        o->resize(X,Y,W,H);
        o->clear_damage(FL_DAMAGE_ALL);
      }
      if (d&FL_DAMAGE_ALL) draw_child(*o); else update_child(*o);
      // child's draw() can change it's size, so use new size:
      current_position += (horizontal() ? o->w() : o->h());
      if (current_position > maximum_position)
        maximum_position = current_position;
      current_position += spacing_;
    }
  }

  if (horizontal()) {
    if (maximum_position < tx+tw) {
      fl_color(color());
      fl_rectf(maximum_position, ty, tx+tw-maximum_position, th);
    }
    tw = maximum_position-tx;
  } else {
    if (maximum_position < ty+th) {
      fl_color(color());
      fl_rectf(tx, maximum_position, tw, ty+th-maximum_position);
    }
    th = maximum_position-ty;
  }

  tw += Fl::box_dw(box()); if (tw <= 0) tw = 1;
  th += Fl::box_dh(box()); if (th <= 0) th = 1;
  if (tw != w() || th != h()) {Fl_Widget::resize(x(),y(),tw,th); d = FL_DAMAGE_ALL;}
  if (d&FL_DAMAGE_ALL) draw_box();
}

//
// End of "$Id$".
//
