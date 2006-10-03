/*
 *  imgwin.cc
 */

#include "config.h"

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>

#include <iostream.h>
#include <iomanip.h>

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

#include "libvideogfx/init.hh"

#include "server.hh"
#include "imgwin.hh"

#include "libvideogfx/graphics/color/colorspace.hh"


struct X11SpecificData
{
  Display*  d_display;
  Window    d_win;
};


ImageWindow_X11::ImageWindow_X11()
  : d_initialized(false),
    d_xpos(-1),d_ypos(-1)
{
  d_x11data = new X11SpecificData;
}


ImageWindow_X11::~ImageWindow_X11()
{
  //printf("ImageWindow_X11::~ImageWindow_X11()\n");
  Close();

  if (d_x11data) { delete d_x11data; d_x11data=NULL; }
}

void ImageWindow_X11::Close()
{
  if (!d_initialized)
    return;

  XUnmapWindow(d_x11data->d_display , d_x11data->d_win);
  XFlush(d_x11data->d_display);
  d_initialized=false;
}


Window   ImageWindow_X11::AskWindow()  { assert(d_initialized); return d_x11data->d_win; }
Display* ImageWindow_X11::AskDisplay() { return d_x11data->d_display; }


void ImageWindow_X11::Create(int w,int h,const char* title,const X11Server* server,Window parent)
{
  assert(!d_initialized);

  // Get X11 server.

  if (server)
    d_x11data->d_display = server->AskDisplay();
  else
    d_x11data->d_display = default_x11server.AskDisplay();

  int screen = DefaultScreen(d_x11data->d_display);
  Window rootwin = RootWindow(d_x11data->d_display,screen);


  // Choose VisualInfo

  XVisualInfo vinfo;
  //bool use_cmap8=false;

  if (XMatchVisualInfo(d_x11data->d_display, screen, 16, TrueColor, &vinfo))
    {
    }
  else
  if (XMatchVisualInfo(d_x11data->d_display, screen, 15, TrueColor, &vinfo))
    {
    }
  else
  if (XMatchVisualInfo(d_x11data->d_display, screen, 24, TrueColor, &vinfo))
    {
    }
  else
  if (XMatchVisualInfo(d_x11data->d_display, screen, 32, TrueColor, &vinfo))
    {
    }
  else
  if (XMatchVisualInfo(d_x11data->d_display, screen,  8, PseudoColor, &vinfo))
    {
      // use_cmap8=true;
    }
#if 0
  else
  if (XMatchVisualInfo(d_x11data->d_display, screen,  8, GrayScale, &vinfo))
    {
    }
#endif
  else
    {
      // TODO
      cerr << "no matching visual found\n";
      exit(10);
      // throw Excpt_Base(ErrSev_Error,"I'm sorry, no matching visual info found.");
    }

  //cout << "VISUAL-ID used for window: 0x" << hex << vinfo.visualid << dec << endl;

  // Create window

  Colormap theCmap = XCreateColormap(d_x11data->d_display, rootwin, vinfo.visual, AllocNone);

  XSetWindowAttributes attr;
  attr.colormap = theCmap;
  attr.background_pixel = 0;
  attr.border_pixel     = 1;

  Window parent_window;

#if 1
  if (parent)
    parent_window = parent;
  else
#endif
    parent_window = RootWindow(d_x11data->d_display,screen);

  //printf("WINID: %d %p\n",parent,parent);

  d_x11data->d_win = XCreateWindow(d_x11data->d_display, parent_window,
				   d_xpos,d_ypos,w,h, 2, vinfo.depth, InputOutput, vinfo.visual,
				   CWBackPixel|CWBorderPixel|CWColormap,&attr);
  
#if 1
  XSizeHints sizeh;
  sizeh.flags  = PSize; //|PMinSize|PMaxSize;
  if (d_xpos>=0 && d_ypos>=0) sizeh.flags |= PPosition;
  sizeh.width  = w;
  sizeh.height = h;
  sizeh.min_width  = w;
  sizeh.min_height = h;
  sizeh.max_width  = w;
  sizeh.max_height = h;
#endif

  XSetStandardProperties(d_x11data->d_display,d_x11data->d_win,
			 title,
			 title,
			 None,
                         glob_argv,glob_argc,&sizeh);
  
  XSelectInput(d_x11data->d_display, d_x11data->d_win, ExposureMask|KeyPressMask);
  XMapWindow(d_x11data->d_display,d_x11data->d_win);
  XFlush(d_x11data->d_display);

  while (1)
    {
      XEvent xev;
      XNextEvent(d_x11data->d_display,&xev);

      if (xev.type == Expose)
        break;
    }

  // Set Colormap

  // TODO

  d_initialized = true; 
}


ImageWindow_Autorefresh_X11::ImageWindow_Autorefresh_X11()
  : d_lastimg_was_RGB(false),
    d_lastimg_was_YUV(false)
{
}

ImageWindow_Autorefresh_X11::~ImageWindow_Autorefresh_X11()
{
  //printf("ImageWindow_Autorefresh_X11::~ImageWindow_Autorefresh_X11()\n");
}

void ImageWindow_Autorefresh_X11::Create(int w,int h,const char*title,const X11Server* server,Window win)
{
  ImageWindow_X11::Create(w,h,title,server,win);
  DisplayImage_X11::Create(w,h,AskWindow(),server);
}

void ImageWindow_Autorefresh_X11::Close()
{
  ImageWindow_X11::Close();
}

void ImageWindow_Autorefresh_X11::Display_const(const Image_RGB<Pixel>& img)
{
  if (!d_lastimg_was_RGB)
    {
      XImage& ximg = AskXImage();

      RawImageSpec_RGB spec;
      spec.bytes_per_line = ximg.bytes_per_line;
      spec.bits_per_pixel = ximg.bits_per_pixel;
      spec.little_endian  = (ximg.byte_order==LSBFirst);
      spec.SetRGBMasks(ximg.red_mask,ximg.green_mask,ximg.blue_mask);

      SetOutputSpec(spec);

      d_lastimg_was_RGB = true;
      d_lastimg_was_YUV = false;
    }
  
  TransformRGB(img,(uint8*)(AskXImage().data));
  PutImage();
}

void ImageWindow_Autorefresh_X11::Display_const(const Image_YUV<Pixel>& img)
{
  if (!d_lastimg_was_YUV)
    {
      XImage& ximg = AskXImage();

      RawImageSpec_RGB spec;
      spec.bytes_per_line = ximg.bytes_per_line;
      spec.bits_per_pixel = ximg.bits_per_pixel;
      spec.little_endian  = (ximg.byte_order==LSBFirst);
      spec.SetRGBMasks(ximg.red_mask,ximg.green_mask,ximg.blue_mask);

      SetOutputSpec(spec);

      d_lastimg_was_RGB = false;
      d_lastimg_was_YUV = true;
    }
  
  TransformYUV(img,(uint8*)(AskXImage().data));
  PutImage();
}

void ImageWindow_Autorefresh_X11::Display(Image_YUV<Pixel>& img)
{
  if (!d_lastimg_was_YUV)
    {
      XImage& ximg = AskXImage();

      RawImageSpec_RGB spec;
      spec.bytes_per_line = ximg.bytes_per_line;
      spec.bits_per_pixel = ximg.bits_per_pixel;
      spec.little_endian  = (ximg.byte_order==LSBFirst);
      spec.SetRGBMasks(ximg.red_mask,ximg.green_mask,ximg.blue_mask);

      SetOutputSpec(spec);

      d_lastimg_was_RGB = false;
      d_lastimg_was_YUV = true;
    }
  
  TransformYUV(img,(uint8*)(AskXImage().data));
  PutImage();
}

void ImageWindow_Autorefresh_X11::CheckForRedraw()
{
  XEvent event;
  while (XCheckWindowEvent(AskDisplay(),AskWindow(),ExposureMask,&event))
    {
      Redraw(event.xexpose);
    }
}


void ImageWindow_Autorefresh_X11::RedrawForever()
{
  XEvent event;
  for (;;)
    {
      XWindowEvent(AskDisplay(),AskWindow(),ExposureMask,&event);
      Redraw(event.xexpose);
    }
}


char ImageWindow_Autorefresh_X11::CheckForKeypress()
{
  XEvent event;
  if (XCheckWindowEvent(AskDisplay(),AskWindow(),KeyPressMask,&event))
    {
      char buf;

      if (XLookupString(&event.xkey,&buf,1,NULL,NULL) > 0)
	return buf;
      else
	return 0;
    }
  else
    return 0;
}


char ImageWindow_Autorefresh_X11::WaitForKeypress()
{
  XEvent event;

  for (;;)
    {
      XWindowEvent(AskDisplay(),AskWindow(),KeyPressMask|ExposureMask,&event);

      if (event.type == Expose)
	{
	  Redraw(event.xexpose);
	}
      else
	{
	  char buf;

	  if (XLookupString(&event.xkey,&buf,1,NULL,NULL) > 0)
	    return buf;
	  else
	    return 0;
	}
    }
}


void ImageWindow_Autorefresh_X11::Redraw(XExposeEvent& ev)
{
  PutImage(ev.x     , ev.y,
	   ev.width , ev.height,
	   ev.x     , ev.y);
}


/* All windows have to be on the same X-server.
 */
int MultiWindowRefresh(ImageWindow_Autorefresh_X11*const* windows,int nWindows)
{
  XEvent ev;
  bool refresh_occured=false;

  while (!refresh_occured)
    {
      XMaskEvent(windows[0]->AskDisplay(),ExposureMask|KeyPressMask,&ev);
      for (int i=0;i<nWindows;i++)
	{
	  if (ev.xany.window == windows[i]->AskWindow())
	    {
	      if (ev.type == Expose)
		{
		  windows[i]->Redraw(ev.xexpose);
		  refresh_occured=true;
		}
	      else if (ev.type == KeyPress)
		return i;
	    }
	}
    }

  return -1;
}
