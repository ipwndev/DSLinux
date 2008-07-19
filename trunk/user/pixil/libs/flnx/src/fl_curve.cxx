//
// "$Id$"
//
// Bezier curve functions for the Fast Light Tool Kit (FLTK).
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

// Utility for drawing Bezier curves, adding the points to
// the current fl_begin/fl_vertex/fl_end path.
// Incremental math implementation:
// I very much doubt this is optimal!  From Foley/vanDam page 511.
// If anybody has a better algorithim, please send it!

#include <FL/fl_draw.H>
#include <math.h>

void fl_curve(float X0, float Y0,
	      float X1, float Y1,
	      float X2, float Y2,
	      float X3, float Y3) {
  float x = fl_transform_x(X0,Y0);
  float y = fl_transform_y(X0,Y0);
  float x1 = fl_transform_x(X1,Y1);
  float y1 = fl_transform_y(X1,Y1);
  float x2 = fl_transform_x(X2,Y2);
  float y2 = fl_transform_y(X2,Y2);
  float x3 = fl_transform_x(X3,Y3);
  float y3 = fl_transform_y(X3,Y3);

  int n; { // find smaller size of bounding box
    float lx = x; if (x1<lx) lx=x1; if (x2<lx) lx=x2; if (x3<lx) lx=x3;
    float rx = x; if (x1>rx) rx=x1; if (x2>rx) rx=x2; if (x3>rx) rx=x3;
    float ly = y; if (y1<ly) ly=y1; if (y2<ly) ly=y2; if (y3<ly) ly=y3;
    float ry = y; if (y1>ry) ry=y1; if (y2>ry) ry=y2; if (y3>ry) ry=y3;
    // calculate number of pieces to cut curve into:
    n = int((rx-lx+ry-ly)/8); if (n < 3) n = 3;
  }
  float e = 1.0/n;

  // calculate the coefficients of 3rd order equation:
  float xa = (x3-3*x2+3*x1-x);
  float xb = 3*(x2-2*x1+x);
  float xc = 3*(x1-x);
  // calculate the forward differences:
  float dx1 = ((xa*e+xb)*e+xc)*e;
  float dx3 = 6*xa*e*e*e;
  float dx2 = dx3 + 2*xb*e*e;

  // calculate the coefficients of 3rd order equation:
  float ya = (y3-3*y2+3*y1-y);
  float yb = 3*(y2-2*y1+y);
  float yc = 3*(y1-y);
  // calculate the forward differences:
  float dy1 = ((ya*e+yb)*e+yc)*e;
  float dy3 = 6*ya*e*e*e;
  float dy2 = dy3 + 2*yb*e*e;

  // draw point 0:
  fl_transformed_vertex(x,y);

  // draw points 1 .. n-2:
  for (int m=2; m<n; m++) {
    x += dx1;
    dx1 += dx2;
    dx2 += dx3;
    y += dy1;
    dy1 += dy2;
    dy2 += dy3;
    fl_transformed_vertex(x,y);
  }

  // draw point n-1:
  fl_transformed_vertex(x+dx1, y+dy1);

  // draw point n:
  fl_transformed_vertex(x3,y3);
}

//
// End of "$Id$".
//
