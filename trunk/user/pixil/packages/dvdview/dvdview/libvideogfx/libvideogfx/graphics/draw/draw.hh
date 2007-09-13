/*********************************************************************
  libvideogfx/graphics/draw/draw.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany
   - Alexander Staller, alexander.staller@engineer.com

  modifications:
    14/Aug/2000 - Alexander Staller - New functions: 
      DrawRectangle(), DrawCircle(), DrawEllipse(), DrawArrow()
      new class ArrowPainter, new implementation of DrawLine().
    14/Aug/2000 - Dirk Farin - new function: DrawFilledRectangleHV()
    17/Nov/1999 - Dirk Farin - converted bitmap drawing functions to
                               function templates.
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_DRAW_DRAW_HH
#define LIBVIDEOGFX_GRAPHICS_DRAW_DRAW_HH

#include <math.h>

#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/graphics/basic/image.hh"


/* This function-call is very inefficient but provides clipping. */
template <class T> void DrawPoint     (Bitmap<T>&,int x,int y,T color);

// main function to draw a line very fast. Clipping is included, so don't think about it
template <class T> void DrawLine(Bitmap<T>& bm,int x0,int y0,int x1, int y1,T color);

/* Only draws every 4th dot. */
template <class T> void DrawDottedLine(Bitmap<T>&,int x1,int y1,int x2,int y2,T color);

/* Draw filled H/V-aligned rectangle. */
template <class T> void DrawFilledRectangleHV(Bitmap<T>&,int x0,int y0,int x1,int y1,T color);

/* Set all of the bitmap to the specified color. */
template <class T> void Clear         (Bitmap<T>&,T color);

// This function draws a rectangle. To do so it calls four times the DrawLineFast(...) function 
template <class T> void DrawRectangle(Bitmap<T>& bm,int x1,int y1,int w, int h,T color);

/* This function draws a line and places a head on one (arrows == false) or
   both (arrows==true) sides of the line. */
template <class T> void DrawArrow(Bitmap<T>& bm,int x0,int y0,int x1, int y1,float alpha,int len,
				  T color,bool arrows = false);

// main function to draw a circle
template <class T> void DrawCircle(Bitmap<T>& bm,int x0,int y0, int radius,T color,bool fill = false);

// this function draws an ellipse. Clipping is also included.
template <class T> void DrawEllipse(Bitmap<T>& bm,int xm,int ym, int a,int b,float angle,T color);

template <class Pel> class ArrowPainter
{
public:
  ArrowPainter();

  void SetAlpha(float a) { alpha=a*M_PI/180; }
  void SetHeadLength(int l) { len=l; }
  void DrawBothHeads(bool flag=true) { bothheads=flag; }
  void SetColor(Pel c) { color=c; }

  void DrawArrow(Bitmap<Pel>& bm,int x0,int y0,int x1, int y1)
    { ::DrawArrow(bm,x0,y0,x1,y1,alpha,len,color,bothheads); }

private:
  float alpha;
  int    len;
  bool   bothheads;
  Pel    color;
};

#if 0
/* Copy a number of lines at the inner border of the image. */
void CopyInnerBorder(const Image<Pixel>& src,Image<Pixel>& dst,int hwidth,int vwidth);
#endif

#if 1
/* Creates a border of the specified with around the image and copies the border
   pixel values into it. */
void EnhanceImageWithBorder(Image<Pixel>& img,int borderwidth,bool exactsize=false);
#endif


/* Old (obsolete) drawline code */
template <class T> void DrawLineSlow  (Bitmap<T>&,int x1,int y1,int x2,int y2,T color);

#endif
