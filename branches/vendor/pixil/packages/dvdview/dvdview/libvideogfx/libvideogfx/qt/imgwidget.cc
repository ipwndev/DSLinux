/*
 *  imwidget.cc
 */

#if 0
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#endif

#include <iostream.h>
#include <iomanip.h>

#include "main.hh"

#include "imgwidget.hh"

#include "x11/server.hh"
//#include "graphics/color/colorspace.hh"


ImageWidget_QT::ImageWidget_QT(QWidget* parent,const char* name)
  : QWidget(parent,name),
    d_initialized(false)
{
}


ImageWidget_QT::~ImageWidget_QT()
{
  Close();
}


void ImageWidget_QT::Close()
{
  ImageWindow_Autorefresh_X11::Close();
}


void ImageWidget_QT::Create(int w,int h,const char* title,const X11Server* server,Window id)
{
  resize(w,h);

  cout << "ImageWidget_QT::Create\n";
  ImageWindow_Autorefresh_X11::Create(w,h,title,server,winId());
}

