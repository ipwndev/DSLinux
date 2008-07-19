/*********************************************************************
  qt/imgwidget.hh

  purpose:
    QT-widget that can display a DisplayImage_X11 which is VERY fast.

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   06/Aug/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_QT_IMGWIDGET_HH
#define LIBVIDEOGFX_QT_IMGWIDGET_HH

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

#include <qwidget.h>

#include "types.hh"

#include "x11/imgwin.hh"
#include "x11/dispimg.hh"
#include "graphics/lowlevel/img2raw.hh"


class ImageWidget_QT : public QWidget,
		       public ImageWindow_Autorefresh_X11
{
  Q_OBJECT
public:
  ImageWidget_QT(QWidget* parent=0,const char* name=0);
    
  ~ImageWidget_QT();

  void Create(int w,int h,const char* title,const X11Server* server=NULL,Window id=0);

  //Window   AskWindow() { assert(d_initialized); return d_win; }
  //Display* AskDisplay() { return d_display; }

private:
  void Close();

  bool        d_initialized;
  //Display*    d_display;

  //Window   d_win;
};

#endif
