//
// "$Id$"
//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
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

// "integer" circle drawing functions.  These draw the limited
// circle types provided by X and NT graphics.  The advantage of
// these is that small ones draw quite nicely (probably due to stored
// hand-drawn bitmaps of small circles!) and may be implemented by
// hardware and thus are fast.

// Probably should add fl_chord.

// 3/10/98: created

#include <FL/fl_draw.H>
#include <FL/x.H>
//#include <stdio.H> //tanghao
#ifdef WIN32
#include <FL/math.h>
#endif

void fl_arc(int x,int y,int w,int h,float a1,float a2) {
  if (w <= 0 || h <= 0) return;
#ifdef WIN32
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb); 
#else

#ifdef NANO_X 

  --w;  --h;

  GrArcAngle(fl_window, fl_gc, x + w/2, y + h/2, w/2, h/2, 
	     int(a1 * 64), int(a2 * 64), MWARC);

  ++w;  ++h;

#ifdef oldold //tanghao
  GrEllipse(fl_window,fl_gc,x+(w-1)/2,y+(h-1)/2,(w-1)/2,(h-1)/2);
  //llx+w/2,lly+h/2,w/2,h/2);
  //printf("not support ARC now,in Nano_X\n");
#endif

#else
  XDrawArc(fl_display, fl_window, fl_gc, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
#endif //tanghao
#endif
}


void fl_pie(int x,int y,int w,int h,float a1,float a2) {
  if (w <= 0 || h <= 0) return;
  if (a1 == a2) return;

#ifdef WIN32
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  SelectObject(fl_gc, fl_brush());
  Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb); 
#else
#ifdef NANO_X 
 
  --w;  --h;

  GrArcAngle(fl_window, fl_gc, x + w/2, y + h/2, w/2, h/2, 
	     int(a1 * 64), int(a2 * 64), MWPIE);
  
  ++w;  ++h;

#ifdef oldold //tanghao
  GrFillEllipse(fl_window,fl_gc,x+w/2,y+h/2,w/2,h/2);
  //  printf("not support fillARC now,in Nano_X\n");
#endif

#else
  XFillArc(fl_display, fl_window, fl_gc, x,y,w,h, int(a1*64),int((a2-a1)*64));
#endif //tanghao
#endif
}

//
// End of "$Id$".
//
