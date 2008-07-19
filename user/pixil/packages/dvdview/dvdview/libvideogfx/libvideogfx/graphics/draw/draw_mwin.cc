
#include "libvideogfx/graphics/draw/draw_x11.hh"

#include <microwin/nano-X.h>
#include <iostream.h>
#include "libvideogfx/x11/server.hh"

void WriteText(Bitmap<Pixel>& bm,const char* txt,int x0,int y0,const char* fontname,
	       HTextAlign halign,VTextAlign valign,
	       TextDrawMode mode,Pixel front,Pixel back)
{
  Pixel*const* yp = bm.AskFrame();

  // Get access to font.
  //Font font = XLoadFont(display,fontname);
  //XFontStruct* fontstr = XQueryFont(display,font);
  GR_GC_ID gc = GrNewGC();
  GR_FONT_ID font = GrCreateFont(0, MWFONT_SYSTEM_FIXED, 0);
  GrSetGCFont(gc, font);

  // Calculate pixel dimensions of text if written in the font specified.

  int width, height, base;
  //XCharStruct overallsize;
  //XTextExtents(fontstr,txt,strlen(txt),&dummy,&dummy,&dummy,&overallsize);
  //const int w = overallsize.width;
  //const int h = overallsize.ascent+overallsize.descent;

  GrGetGCTextSize(gc, txt, strlen(txt), GR_TFASCII|GR_TFTOP, &width,
  	&height, &base);
  const int w = width;
  const int h = height;

  // Create a pixmap to draw the text into.

  GR_WINDOW_ID pixmap = GrCreatePixmap(w,h,NULL);

  // Clear pixmap.
  //XSetForeground(display,gc,0);
  //XSetBackground(display,gc,0);
  //XFillRectangle(display,pixmap,gc,0,0,w,h);

  // Draw text.

  //XSetForeground(display,gc,1);
  //XSetFont(display,gc,font);
  //XDrawString(display,pixmap,gc,0,overallsize.ascent,txt,strlen(txt));
  GrText(pixmap, gc, 0, 0, txt, strlen(txt), GR_TFASCII|GR_TFTOP);

  // Transfer pixmap data into X11-client XImage.
  //XImage* ximg = XGetImage(display,pixmap,0,0,w,h,0x01,ZPixmap);
  GR_PIXELVAL bits[w*h];
  GrReadArea(pixmap, 0, 0, w, h, bits);

  if (halign==HAlign_Center) x0 -= w/2;
  if (halign==HAlign_Right)  x0 += (bm.AskWidth()-w);

  if (valign==VAlign_Center) y0 -= h/2;
  if (valign==VAlign_Bottom) y0 += (bm.AskHeight()-h);

  if (mode==TextDrawMode_Opaque)
    {
      for (int y=0;y<h;y++)
	for (int x=0;x<w;x++)
	  {
	    //unsigned char bitpos = 0x80>>(x%8);
	    //yp[y+y0][x+x0] = ((ximg->data[ximg->bytes_per_line*y+x/8]&bitpos) ? front : back );
	    yp[y+y0][x+x0] = ((bits[w*y+x]) ? front : back );
	  }
    }
  else
    {
      assert(mode==TextDrawMode_Transparent);

      for (int y=0;y<h;y++)
	for (int x=0;x<w;x++)
	  {
	    //unsigned char bitpos = 0x80>>(x%8);
	    //if (ximg->data[ximg->bytes_per_line*y+x/8]&bitpos) yp[y+y0][x+x0] = front;
	    if (bits[w*y+x]) yp[y+y0][x+x0] = front;
	  }
    }

  GrDestroyGC(gc);
  GrDestroyFont(font);
  GrDestroyWindow(pixmap);
}
