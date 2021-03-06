//
// "$Id$"
//
// Portable drawing function header file for the Fast Light Tool Kit (FLTK).
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

#ifndef fl_draw_H
#define fl_draw_H

#include "Enumerations.H"  // for the color names

// Colors:
FL_EXPORT void	fl_color(Fl_Color); // select indexed color
inline void fl_color(int c) {fl_color((Fl_Color)c);} // for back compatability
FL_EXPORT void	fl_color(uchar, uchar, uchar); // select actual color
extern FL_EXPORT Fl_Color fl_color_;
inline Fl_Color fl_color() {return fl_color_;}

// clip:
FL_EXPORT void fl_clip(int x, int y, int w, int h);
#define fl_push_clip fl_clip
FL_EXPORT void fl_push_no_clip();
FL_EXPORT void fl_pop_clip();
FL_EXPORT int fl_not_clipped(int x, int y, int w, int h);
FL_EXPORT int fl_clip_box(int, int, int, int, int& x, int& y, int& w, int& h);

// points:
FL_EXPORT void fl_point(int x, int y);

// rectangles tweaked to exactly fill the pixel rectangle:
FL_EXPORT void fl_rect(int x, int y, int w, int h);
FL_EXPORT void fl_rectf(int x, int y, int w, int h);

// line segments:
FL_EXPORT void fl_line(int,int, int,int);
FL_EXPORT void fl_line(int,int, int,int, int,int);

// closed line segments:
FL_EXPORT void fl_loop(int,int, int,int, int,int);
FL_EXPORT void fl_loop(int,int, int,int, int,int, int,int);

// filled polygons
FL_EXPORT void fl_polygon(int,int, int,int, int,int);
FL_EXPORT void fl_polygon(int,int, int,int, int,int, int,int);

// draw rectilinear lines, horizontal segment first:
FL_EXPORT void fl_xyline(int x, int y, int x1);
FL_EXPORT void fl_xyline(int x, int y, int x1, int y2);
FL_EXPORT void fl_xyline(int x, int y, int x1, int y2, int x3);

// draw rectilinear lines, vertical segment first:
FL_EXPORT void fl_yxline(int x, int y, int y1);
FL_EXPORT void fl_yxline(int x, int y, int y1, int x2);
FL_EXPORT void fl_yxline(int x, int y, int y1, int x2, int y3);

// circular lines and pie slices (code in fl_arci.C):
FL_EXPORT void fl_arc(int x, int y, int w, int h, float a1, float a2);
FL_EXPORT void fl_pie(int x, int y, int w, int h, float a1, float a2);
FL_EXPORT void fl_chord(int x, int y, int w, int h, float a1, float a2); // nyi

// scalable drawing code (code in fl_vertex.C and fl_arc.C):
FL_EXPORT void fl_push_matrix();
FL_EXPORT void fl_pop_matrix();
FL_EXPORT void fl_scale(float x, float y);
FL_EXPORT void fl_scale(float x);
FL_EXPORT void fl_translate(float x, float y);
FL_EXPORT void fl_rotate(float d);
FL_EXPORT void fl_mult_matrix(float a, float b, float c, float d, float x,float y);
FL_EXPORT void fl_begin_points();
FL_EXPORT void fl_begin_line();
FL_EXPORT void fl_begin_loop();
FL_EXPORT void fl_begin_polygon();
FL_EXPORT void fl_vertex(float x, float y);
FL_EXPORT void fl_curve(float, float, float, float, float, float, float, float);
FL_EXPORT void fl_arc(float x, float y, float r, float start, float a);
FL_EXPORT void fl_circle(float x, float y, float r);
FL_EXPORT void fl_end_points();
FL_EXPORT void fl_end_line();
FL_EXPORT void fl_end_loop();
FL_EXPORT void fl_end_polygon();
FL_EXPORT void fl_begin_complex_polygon();
FL_EXPORT void fl_gap();
FL_EXPORT void fl_end_complex_polygon();
// get and use transformed positions:
FL_EXPORT float fl_transform_x(float x, float y);
FL_EXPORT float fl_transform_y(float x, float y);
FL_EXPORT float fl_transform_dx(float x, float y);
FL_EXPORT float fl_transform_dy(float x, float y);
FL_EXPORT void fl_transformed_vertex(float x, float y);

// current font:
FL_EXPORT void fl_font(int face, int size);
extern FL_EXPORT int fl_font_;
inline int fl_font() {return fl_font_;}
extern FL_EXPORT int fl_size_;
inline int fl_size() {return fl_size_;}

// information you can get about the current font:
FL_EXPORT int   fl_height();	// using "size" should work ok
FL_EXPORT int   fl_descent();
FL_EXPORT float fl_width(const char*);
FL_EXPORT float fl_width(const char*, int n);
FL_EXPORT float fl_width(uchar);

// draw using current font:
FL_EXPORT void fl_draw(const char*, int x, int y);
FL_EXPORT void fl_draw(const char*, int n, int x, int y);
FL_EXPORT void fl_measure(const char*, int& x, int& y);
FL_EXPORT void fl_draw(const char*, int,int,int,int, Fl_Align);
FL_EXPORT void fl_draw(const char*, int,int,int,int, Fl_Align,
	               void (*callthis)(const char *, int n, int x, int y));

// boxtypes:
FL_EXPORT void fl_frame(const char* s, int x, int y, int w, int h);
FL_EXPORT void fl_frame2(const char* s, int x, int y, int w, int h);
FL_EXPORT void fl_draw_box(Fl_Boxtype, int x, int y, int w, int h, Fl_Color);

// images:
FL_EXPORT void fl_draw_image(const uchar*, int,int,int,int, int delta=3, int ldelta=0);
FL_EXPORT void fl_draw_image_mono(const uchar*, int,int,int,int, int delta=1, int ld=0);
typedef void (*Fl_Draw_Image_Cb)(void*,int,int,int,uchar*);
FL_EXPORT void fl_draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=3);
FL_EXPORT void fl_draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=1);
FL_EXPORT void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b);

// pixmaps:
FL_EXPORT int fl_draw_pixmap(/*const*/ char* const* data, int x,int y,Fl_Color=FL_GRAY);
FL_EXPORT int fl_measure_pixmap(/*const*/ char* const* data, int &w, int &h);

// other:
extern FL_EXPORT void fl_scroll(int X, int Y, int W, int H, int dx, int dy,
	                        void (*draw_area)(void*, int,int,int,int), void* data);
FL_EXPORT const char* fl_shortcut_label(int);
FL_EXPORT void fl_overlay_rect(int,int,int,int);
FL_EXPORT void fl_overlay_clear();
FL_EXPORT void fl_cursor(Fl_Cursor, Fl_Color=FL_BLACK, Fl_Color=FL_WHITE);

// XForms symbols:
FL_EXPORT int fl_draw_symbol(const char* label,int x,int y,int w,int h, Fl_Color);
FL_EXPORT int fl_add_symbol(const char* name, void (*drawit)(Fl_Color), int scalable);

#endif

//
// End of "$Id$".
//
