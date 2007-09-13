//
// "$Id$"
//
// Slider header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Eq_H
#define Fl_Eq_H

#ifndef Fl_Valuator_H
#include <FL/Fl_Valuator.H>
#endif

#include <FL/Enumerations.H>

// values for type(), lowest bit indicate horizontal:
#define FL_VERT_SLIDER		0
#define FL_HOR_SLIDER		1
#define FL_VERT_FILL_SLIDER	2 // Vertical Equalizer
#define FL_VERT_EQ              2 // Vertical Equalizer
#define FL_HOR_FILL_SLIDER	3 // Horizontal Equalizer
#define FL_HOR_EQ               3 // Horizontal Equalizer
#define FL_VERT_NICE_SLIDER	4
#define FL_HOR_NICE_SLIDER	5

class Fl_Eq : public Fl_Valuator {

private:
  
  // Private member variables
  float slider_size_;
  float slider_size_min_;
  uchar slider_;
  Fl_Color color1; // First color of the Equalizer
  Fl_Color color2; // Second color of the Equalizer
  Fl_Color color3; // Third color of the Equalizer
  int percent1; // The percentage of the bar taken for first color of the Equalizer
  int percent2; // The percentage of the bar taken for the second color of the Equalizer
  int percent3; // The percentage of the bar takne for the third color of the Equalizer.

  // Private methods
  FL_EXPORT void _Fl_Eq();


public:

  // Public Interface
  FL_EXPORT void draw_bg(int, int, int, int);

  // these allow subclasses to put the slider in a smaller area:
  FL_EXPORT void draw(int, int, int, int);
  FL_EXPORT int handle(int, int, int, int, int);
  FL_EXPORT void draw();
  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Eq(int x,int y,int w,int h, const char *l = 0);
  FL_EXPORT Fl_Eq(uchar t,int x,int y,int w,int h, const char *l);

  // This is where the colored bars are drawn for a vertical equalizer.
  FL_EXPORT void slider_hor_lines(int x, int y, int h, int w, int W, Fl_Color c);

  // This is where the colored bars are drawn for a horizontal equalizer.
  FL_EXPORT void slider_ver_lines(int x, int y, int h, int w, int W, Fl_Color c);

  FL_EXPORT int scrollvalue(int windowtop,int windowsize,int first,int totalsize);
  FL_EXPORT void bounds(float a, float b);
  float slider_size() const {return slider_size_;}
  FL_EXPORT void slider_size(float v);
  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}
  void slider(Fl_Boxtype c) {slider_ = c;}

  // Equalizer color range.
  // The first parameter is the percentage of the total part taken
  // by the first color.
  // The second parameter is the percentage of the total part taken
  // by the second color, after the first color.
  // The third color is calculated from the remainder.
  // i.e. Range(30, 30); would use the first 30% for color 1, the second
  // 30% for color 2, and 40% for the color 3.
  void Range(int p1, int p2);

  // Set the first, second, and third colors for the equalizers (from
  // left to right (horizontal) or from bottom to top (vertical).
  // You must pass in a FL_COLOR (i.e. FL_RED or use the color cube.)
  void Colors(Fl_Color c1, Fl_Color  c2, Fl_Color c2);

};

#endif

//
// End of "$Id$".
//
