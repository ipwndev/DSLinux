#ifndef __QPAINTDEVICE_H
#define __QPAINTDEVICE_H

#include "qobject.h"
#include "Fl_Widget.H"
#include "Fl_Window.H"
#include "Fl_Double_Window.H"
#include "fl_draw.H"

#define PDT_UNDEF       0x00
#define PDT_WIDGET      0x01
#define PDT_PIXMAP      0x02
#define PDT_PRINTER     0x03
#define PDT_PICTURE     0x04
#define PDT_SYSTEM      0x05
#define PDT_MASK        0x0f

enum RasterOp { CopyROP, OrROP, XorROP, EraseROP,
		NotCopyROP, NotOrROP, NotXorROP, 
		NotEraseROP, NotROP };

//#define USE_FL_WIDGET 1

#ifdef USE_FL_WIDGET
#define FLTK_PARENT_WIDGET Fl_Widget
#else
#define FLTK_PARENT_WIDGET Fl_Double_Window
#endif


class QPaintDevice : public FLTK_PARENT_WIDGET
{
 public:

  QPaintDevice() : FLTK_PARENT_WIDGET(0,0,0,0,"")
    {  end(); }
  QPaintDevice(const QPaintDevice & pd) : FLTK_PARENT_WIDGET(0,0,0,0,"")
    { end(); }
  ~QPaintDevice() { if(parent()) ((Fl_Group*)parent())->remove(this); }
  friend void bitBlt( QPaintDevice *, int, int,
		      const QPaintDevice *,
		      int, int, int, int, RasterOp, bool );

  int devType() const { return 0; }

  void make_current() const { ((QPaintDevice*)this)->FLTK_PARENT_WIDGET::make_current(); }
  void show() const { ((QPaintDevice*)this)->FLTK_PARENT_WIDGET::show(); }

  virtual FL_EXPORT void draw() {  }

  virtual int handle(int event) { return 0; }

  virtual void resize(const QSize & size)
    { resize(size.width(), size.height()); }
  virtual void resize(int w, int h) {   resize(x(),y(),w,h); }
  virtual void resize(int _x, int _y, int _w, int _h) 
    {
/*
      QSize old_size(width(), height());
      QSize new_size(_w,_h);
      QResizeEvent q(new_size,old_size);
*/
      FLTK_PARENT_WIDGET::resize(_x,_y,_w,_h);  
      //      resizeEvent(&q);
    }

  QPaintDevice & operator=(const QPaintDevice & pd) { return *this; }
};

inline void bitBlt(QPaintDevice * dst, int dx, int dy, 
		   const QPaintDevice * src, int sx, 
		   int sy, int sw, int sh,RasterOp rop = CopyROP,
		   bool ignoreMask=false) { }
#endif
