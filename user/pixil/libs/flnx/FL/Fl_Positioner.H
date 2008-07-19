//
// "$Id$"
//
// Positioner header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Positioner_H
#define Fl_Positioner_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

class Fl_Positioner : public Fl_Widget {

  float xmin, ymin;
  float xmax, ymax;
  float xvalue_, yvalue_;
  float xstep_, ystep_;

protected:

  // these allow subclasses to put the dial in a smaller area:
  FL_EXPORT void draw(int, int, int, int);
  FL_EXPORT int handle(int, int, int, int, int);
  FL_EXPORT void draw();

public:

  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Positioner(int x,int y,int w,int h, const char *l=0);
  float xvalue() const {return xvalue_;}
  float yvalue() const {return yvalue_;}
  FL_EXPORT int xvalue(float);
  FL_EXPORT int yvalue(float);
  FL_EXPORT int value(float,float);
  FL_EXPORT void xbounds(float, float);
  float xminimum() const {return xmin;}
  void xminimum(float a) {xbounds(a,xmax);}
  float xmaximum() const {return xmax;}
  void xmaximum(float a) {xbounds(xmin,a);}
  FL_EXPORT void ybounds(float, float);
  float yminimum() const {return ymin;}
  void yminimum(float a) {ybounds(a,ymax);}
  float ymaximum() const {return ymax;}
  void ymaximum(float a) {ybounds(ymin,a);}
  void xstep(float a) {xstep_ = a;}
  void ystep(float a) {ystep_ = a;}

};

#endif

//
// End of "$Id$".
//
