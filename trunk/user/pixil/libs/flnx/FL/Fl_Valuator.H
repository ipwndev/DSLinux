//
// "$Id$"
//
// Valuator header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Valuator_H
#define Fl_Valuator_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// shared type() values for classes that work in both directions:
#define FL_VERTICAL		0
#define FL_HORIZONTAL		1

class Fl_Valuator : public Fl_Widget {

  float value_;
  float previous_value_;
  float min, max; // truncates to this range *after* rounding
  float A; int B; // rounds to multiples of A/B, or no rounding if A is zero

protected:

  int horizontal() const {return type()&1;}
  FL_EXPORT Fl_Valuator(int X, int Y, int W, int H, const char* L);

  float previous_value() const {return previous_value_;}
  void handle_push() {previous_value_ = value_;}
  FL_EXPORT float softclamp(float);
  FL_EXPORT void handle_drag(float newvalue);
  FL_EXPORT void handle_release(); // use drag() value
  virtual FL_EXPORT void value_damage(); // cause damage() due to value() changing

public:

  void bounds(float a, float b) {min=a; max=b;}
  float minimum() const {return min;}
  void minimum(float a) {min = a;}
  float maximum() const {return max;}
  void maximum(float a) {max = a;}
  void range(float a, float b) {min = a; max = b;}
  void step(int a) {A = a; B = 1;}
  void step(float a, int b) {A = a; B = b;}
  FL_EXPORT void step(float s);
  float step() const {return A/B;}
  FL_EXPORT void precision(int);

  float value() const {return value_;}
  FL_EXPORT int value(float);

  virtual int format(char*);
  FL_EXPORT float round(float); // round to nearest multiple of step
  FL_EXPORT float clamp(float); // keep in range
  FL_EXPORT float increment(float, int); // add n*step to value
};

#endif

//
// End of "$Id$".
//
