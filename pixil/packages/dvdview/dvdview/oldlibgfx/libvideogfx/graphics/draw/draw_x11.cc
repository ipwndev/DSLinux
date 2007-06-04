
#include "libvideogfx/graphics/draw/draw_x11.hh"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream.h>
#include "libvideogfx/x11/server.hh"

void WriteText(Bitmap<Pixel>& bm,const char* txt,int x0,int y0,const char* fontname,
	       HTextAlign halign,VTextAlign valign,
	       TextDrawMode mode,Pixel front,Pixel back)
{
  Pixel*const* yp = bm.AskFrame();

  // Get access to font.

  Display* display = default_x11server.AskDisplay();

  Font font = XLoadFont(display,fontname);
  XFontStruct* fontstr = XQueryFont(display,font);


  // Calculate pixel dimensions of text if written in the font specified.

  int dummy;
  XCharStruct overallsize;
  XTextExtents(fontstr,txt,strlen(txt),&dummy,&dummy,&dummy,&overallsize);

  const int w = overallsize.width;
  const int h = overallsize.ascent+overallsize.descent;


  // Create a pixmap to draw the text into.

  Pixmap pixmap = XCreatePixmap(display,RootWindow(display,DefaultScreen(display)),w,h,1);
  GC gc = XCreateGC(display,pixmap,0,NULL);


  // Clear pixmap.

  XSetForeground(display,gc,0);
  XSetBackground(display,gc,0);
  XFillRectangle(display,pixmap,gc,0,0,w,h);


  // Draw text.

  XSetForeground(display,gc,1);
  XSetFont(display,gc,font);
  XDrawString(display,pixmap,gc,0,overallsize.ascent,txt,strlen(txt));


  // Transfer pixmap data into X11-client XImage.

  XImage* ximg = XGetImage(display,pixmap,0,0,w,h,0x01,ZPixmap);

  if (halign==HAlign_Center) x0 -= w/2;
  if (halign==HAlign_Right)  x0 += (bm.AskWidth()-w);

  if (valign==VAlign_Center) y0 -= h/2;
  if (valign==VAlign_Bottom) y0 += (bm.AskHeight()-h);

  if (mode==TextDrawMode_Opaque)
    {
      for (int y=0;y<ximg->height;y++)
	for (int x=0;x<ximg->width;x++)
	  {
	    unsigned char bitpos = 0x80>>(x%8);
	    yp[y+y0][x+x0] = ((ximg->data[ximg->bytes_per_line*y+x/8]&bitpos) ? front : back );
	  }
    }
  else
    {
      assert(mode==TextDrawMode_Transparent);

      for (int y=0;y<ximg->height;y++)
	for (int x=0;x<ximg->width;x++)
	  {
	    unsigned char bitpos = 0x80>>(x%8);
	    if (ximg->data[ximg->bytes_per_line*y+x/8]&bitpos) yp[y+y0][x+x0] = front;
	  }
    }


  // Clean up

  XDestroyImage(ximg);
}

