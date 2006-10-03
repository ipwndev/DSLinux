/*
 *  markblks.cc
 */

#include <iostream.h>

#include "markblks.hh"

#include "libvideogfx/graphics/draw/draw.hh"


void BlockMarker::Overlay(Image_YUV<Pixel>& img,const Bitmap<bool>& markerbm) const
{
  Bitmap<Pixel>& bm  = img.AskBitmap(Image<Pixel>::Bitmap_Y);

  const bool*const* markp = markerbm.AskFrame_const();

  ImageSpec_YUV spec;
  img.GetParam(spec);

  int w = markerbm.AskWidth();
  int h = markerbm.AskHeight();

  cout << spec.width << " " << d_size_h << " " << w << endl;

  //352 8 22

  assert(spec.width  >  (d_size_h*(w-1)) &&
	 spec.width  <= (d_size_h* w  ));
  assert(spec.height >  (d_size_v*(h-1)) &&
	 spec.height <= (d_size_v* h  ));

  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      {
	if (markp[y][x] != d_inverse)
	  {
	    int x0 = x*d_size_h;
	    int y0 = y*d_size_v;
	    int x1 = x0+d_size_h-1;
	    int y1 = y0+d_size_v-1;

	    DrawLine(bm,x0,y0,x1,y0, d_color);
	    DrawLine(bm,x0,y1,x1,y1, d_color);
	    DrawLine(bm,x0,y0,x0,y1, d_color);
	    DrawLine(bm,x1,y0,x1,y1, d_color);
	  }
      }
}

