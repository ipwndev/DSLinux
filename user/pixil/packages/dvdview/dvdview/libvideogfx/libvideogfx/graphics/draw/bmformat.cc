/*
 *  bmformat.cc
 */

#include <string.h>

#include "bmformat.hh"


template <class A,class B> void ConvertBitmap(const Bitmap<A>& src,Bitmap<B>& dst)
{
  int w=src.AskWidth();
  int h=src.AskHeight();

  dst.Create(w,h,1,1,src.AskBorderWidth());

  const A*const* ap = src.AskFrame_const();
        B*const* bp = dst.AskFrame();

  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      bp[y][x] = static_cast<B>(ap[y][x]);
}


template <class T> void CopyInnerBorder(const Image<T>& src,Image<T>& dst,int hwidth,int vwidth)
{
  for (int i=0;i<4;i++)
    {
            Bitmap<T>& dstbm = dst.AskBitmap      ((Image<T>::BitmapChannel)i);
      const Bitmap<T>& srcbm = src.AskBitmap_const((Image<T>::BitmapChannel)i);


      // Skip empty bitmaps.

      assert(srcbm.IsEmpty() == dstbm.IsEmpty());

      if (srcbm.IsEmpty())
	continue;


      // Copy border.

      int w = srcbm.AskWidth();
      int h = srcbm.AskHeight();

      assert(dstbm.AskWidth()  == w);
      assert(dstbm.AskHeight() == h);

            T*const* dp = dstbm.AskFrame();
      const T*const* sp = srcbm.AskFrame_const();

      // Horizontal stripes.

      for (int x=0;x<w;x++)
	for (int y=0;x<vwidth;y++)
	  {
	    dp[    y][x]=sp[    y][x];
	    dp[h-1-y][x]=sp[h-1-y][x];
	  }

      // Vertical stripes.
      
      for (int y=vwidth;y<h-vwidth;y++)
	for (int x=0;x<hwidth;x++)
	  {
	    dp[y][    x]=sp[y][    x];
	    dp[y][w-1-x]=sp[y][w-1-x];
	  }
    }
}

template <class T> void ExtrudeIntoBorder (Image<T>&  img)
{
  for (int i=0;i<4;i++)
    {
      Bitmap<T>& bm = img.AskBitmap((Image<T>::BitmapChannel)i);

      if (bm.IsEmpty())
	continue;

      CopyBitmapIntoBorder(bm);
    }
}


template <class T> void ExtrudeIntoBorder(Bitmap<T>& bm)
{
  int w = bm.AskWidth(), h = bm.AskHeight();
  int borderwidth = bm.AskBorderWidth();

  T*const* p = bm.AskFrame();

  for (int y=0;y<h;y++)
    for (int x=1;x<=borderwidth;x++)
      {
	p[y][-x]   =p[y][0];
	p[y][w-1+x]=p[y][w-1];
      }

  for (int x=-borderwidth;x<w+borderwidth;x++)
    for (int y=1;y<=borderwidth;y++)
      {
	p[-y][x]   =p[0][x];
	p[h-1+y][x]=p[h-1][x];
      }
}


template <class T> void SetBitmapBorder(Bitmap<T>& bm,const T& val)
{
  int w = bm.AskWidth(), h = bm.AskHeight();
  int borderwidth = bm.AskBorderWidth();

  T*const* p = bm.AskFrame();

  for (int y=0;y<h;y++)
    for (int x=1;x<=borderwidth;x++)
      {
	p[y][-x] = p[y][w-1+x] = val;
      }

  for (int x=-borderwidth;x<w+borderwidth;x++)
    for (int y=1;y<=borderwidth;y++)
      {
	p[-y][x] = p[h-1+y][x] = val;
      }
}

template <class T> void RemoveColor(Image_YUV<T>& img)
{
  img.AskBitmap(Image<Pixel>::Bitmap_U).Destroy();
  img.AskBitmap(Image<Pixel>::Bitmap_V).Destroy();

  ImageParam_YUV param;
  img.GetParam(param);
  param.nocolor=true;
  img.SetParam(param);
}

template <class T> void ConvertGreyToRGB(const Image_YUV<T>& src,Image_RGB<T>& dst)
{
  ImageParam_YUV yuvparam;
  src.GetParam(yuvparam);

  assert(yuvparam.nocolor);

  ImageSpec spec; 
  spec.ImageParam::operator=(yuvparam);
  dst.Create(spec);

  const T*const* yp = src.AskFrameY_const();

  T*const* rp = dst.AskFrameR();
  T*const* gp = dst.AskFrameG();
  T*const* bp = dst.AskFrameB();

  for (int y=0;y<spec.height;y++)
    for (int x=0;x<spec.width;x++)
      {
	rp[y][x] = gp[y][x] = bp[y][x] = yp[y][x];
      }
}

template void CopyInnerBorder(const Image<Pixel>& src,Image<Pixel>& dst,int hwidth,int vwidth);
template void ExtrudeIntoBorder (Image<Pixel>&);
template void ExtrudeIntoBorder(Bitmap<Pixel>&);
template void RemoveColor(Image_YUV<Pixel>&);
template void ConvertGreyToRGB(const Image_YUV<Pixel>&,Image_RGB<Pixel>&);
