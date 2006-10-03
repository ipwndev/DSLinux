/*
 * imgwin.cc
 */
#include "config.h"

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>

#include <iostream.h>
#include <iomanip.h>

#include <nano-X.h>

#include "libvideogfx/init.hh"

#include "server.hh"
#include "imgwin.hh"

#include "libvideogfx/graphics/color/colorspace.hh"

bool fullscreen;

struct NXSpecificData
{
  int		d_display;
  GR_WINDOW_ID	d_win;
};


ImageWindow_X11::ImageWindow_X11()
  : d_initialized(false),
    d_xpos(-1),d_ypos(-1)
{
  d_nxdata = new NXSpecificData;
}


ImageWindow_X11::~ImageWindow_X11()
{
  Close();

  if (d_nxdata) { delete d_nxdata; d_nxdata=NULL; }
}

void ImageWindow_X11::Close()
{
  if (!d_initialized)
    return;

  GrUnmapWindow(d_nxdata->d_win);
  GrFlush();
  d_initialized=false;
}


GR_WINDOW_ID ImageWindow_X11::AskWindow()  { assert(d_initialized); return d_nxdata->d_win; }
int          ImageWindow_X11::AskDisplay() { return d_nxdata->d_display; }


void ImageWindow_X11::Create(int w,int h,const char* title,const X11Server* server,GR_WINDOW_ID parent)
{
  assert(!d_initialized);

  d_nxdata->d_display = default_x11server.AskDisplay();

  // Create window
  GR_WINDOW_ID parent_window;
  if (parent)
    parent_window = parent;
  else
    parent_window = GR_ROOT_WINDOW_ID;

  int flags = 0;
  if (::fullscreen == true) {
    flags = GR_WM_PROPS_NODECORATE | GR_WM_PROPS_NOAUTOMOVE |
    GR_WM_PROPS_NOAUTORESIZE;
    d_xpos = 0;
    d_ypos = 0;
  } else {
    flags = GR_WM_PROPS_APPWINDOW | GR_WM_PROPS_NOAUTORESIZE;
    if (d_xpos >= 0 && d_ypos >= 0)
      flags |= GR_WM_PROPS_NOAUTOMOVE;
  }
  d_nxdata->d_win = GrNewWindowEx(flags, (unsigned char *)title,
  	parent_window, d_xpos, d_ypos, w, h, MWRGB(0,0,0));
  
  GrSelectEvents(d_nxdata->d_win, GR_EVENT_MASK_EXPOSURE|GR_EVENT_MASK_KEY_DOWN);
  GrMapWindow(d_nxdata->d_win);
  GrFlush();

#if 0
  while (1)
    {
      XEvent xev;
      XNextEvent(d_x11data->d_display,&xev);

      if (xev.type == Expose)
        break;
    }
#endif
  d_initialized = true; 
}


ImageWindow_Autorefresh_X11::ImageWindow_Autorefresh_X11()
  : d_lastimg_was_RGB(false),
    d_lastimg_was_YUV(false)
{
}

ImageWindow_Autorefresh_X11::~ImageWindow_Autorefresh_X11()
{
}

void ImageWindow_Autorefresh_X11::Create(int w,int h,const char*title,const X11Server* server,GR_WINDOW_ID win)
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
#if 0
  XEvent event;
  while (XCheckWindowEvent(AskDisplay(),AskWindow(),ExposureMask,&event))
      Redraw(event.xexpose);
#endif
}


void ImageWindow_Autorefresh_X11::RedrawForever()
{
#if 0
  XEvent event;
  for (;;)
    {
      XWindowEvent(AskDisplay(),AskWindow(),ExposureMask,&event);
      Redraw(event.xexpose);
    }
#endif
}


char ImageWindow_Autorefresh_X11::CheckForKeypress()
{
#if 0
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
#endif
}


char ImageWindow_Autorefresh_X11::WaitForKeypress()
{
#if 0
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
#endif
}


void ImageWindow_Autorefresh_X11::Redraw(GR_EVENT_EXPOSURE& ev)
{
  PutImage(ev.x     , ev.y,
	   ev.width , ev.height,
	   ev.x     , ev.y);
}


/* All windows have to be on the same X-server.
 */
int MultiWindowRefresh(ImageWindow_Autorefresh_X11*const* windows,int nWindows)
{
#if 0
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

#endif
  return -1;
}
