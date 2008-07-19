/*
 *  linear.cc
 */

#include <math.h>
#include <iostream.h>

#include "linear.hh"


void LowPass_5pt(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest)
{
  // Get image parameters and assure that they are in the right format.
  ImageParam_YUV param;
  img.GetParam(param);

  ImageParam_YUV param2;
  dest.GetParam(param2);

  assert(param.chroma ==Chroma444);
  assert(param2.chroma==Chroma444);
  assert(&img != &dest);  // Lowpass needs two image buffers for correct operation.

  // Give hint as the destination image will be completely overwritten.
  dest.Hint_ContentsIsNotUsedAnymore();


  Pixel*const* yp2  = dest.AskFrameY();
  Pixel*const* up2  = dest.AskFrameU();
  Pixel*const* vp2  = dest.AskFrameV();

  const Pixel*const* yp  = img.AskFrameY_const();
  const Pixel*const* up  = img.AskFrameU_const();
  const Pixel*const* vp  = img.AskFrameV_const();

  /* Do lowpass filtering.
     We filter all of the image except a one pixel wide border because the
     filter size is 3x3. This border will simply be copied from the original
     image.
  */
     
  int w = param.width;
  int h = param.height;

  for (int y=1;y<param.height-1;y++)
    for (int x=1;x<param.width-1;x++)
      {
	yp2[y][x] = (  yp[y-1][x  ] +
		       yp[y+1][x  ] +
		       yp[y  ][x-1] +
		       yp[y  ][x+1] +
		     4*yp[y  ][x  ]    +4)/8;
	up2[y][x] = (  up[y-1][x  ] +
 		       up[y+1][x  ] +
		       up[y  ][x-1] +
		       up[y  ][x+1] +
		     4*up[y  ][x  ]    +4)/8;
	vp2[y][x] = (  vp[y-1][x  ] +
		       vp[y+1][x  ] +
		       vp[y  ][x-1] +
		       vp[y  ][x+1] +
		     4*vp[y  ][x  ]    +4)/8;
      }

  // Copy border from old image to filtered one.

  for (int x=0;x<param.width;x++)
    {
      yp2[  0][x]=yp[  0][x]; up2[  0][x]=up[  0][x]; vp2[  0][x]=vp[  0][x];
      yp2[h-1][x]=yp[h-1][x]; up2[h-1][x]=up[h-1][x]; vp2[h-1][x]=vp[h-1][x];
    }

  for (int y=0;y<param.height;y++)
    {
      yp2[y][  0]=yp[y][  0]; up2[y][0  ]=up[y][  0]; vp2[y][  0]=vp[y][  0];
      yp2[y][w-1]=yp[y][w-1]; up2[y][w-1]=up[y][w-1]; vp2[y][w-1]=vp[y][w-1];
    }
}



void LowPass_3x3mean(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest)
{
  // Get image parameters and assure that they are in the right format.
  ImageParam_YUV param;
  img.GetParam(param);

  ImageParam_YUV param2;
  dest.GetParam(param2);

  assert(param.chroma ==Chroma444);
  assert(param2.chroma==Chroma444);
  assert(&img != &dest);  // Lowpass needs two image buffers for correct operation.

  // Give hint as the destination image will be completely overwritten.
  dest.Hint_ContentsIsNotUsedAnymore();


  Pixel*const* yp2  = dest.AskFrameY();
  Pixel*const* up2  = dest.AskFrameU();
  Pixel*const* vp2  = dest.AskFrameV();

  const Pixel*const* yp  = img.AskFrameY_const();
  const Pixel*const* up  = img.AskFrameU_const();
  const Pixel*const* vp  = img.AskFrameV_const();

  /* Do lowpass filtering.
     We filter all of the image except a one pixel wide border because the
     filter size is 3x3. This border will simply be copied from the original
     image.
  */
     
  int w = param.width;
  int h = param.height;

  for (int y=1;y<param.height-1;y++)
    for (int x=1;x<param.width-1;x++)
      {
	yp2[y][x] = (  yp[y-1][x-1] +
		       yp[y-1][x  ] +
		       yp[y-1][x+1] +
		       yp[y  ][x-1] +
		       yp[y  ][x  ] +
		       yp[y  ][x+1] +
		       yp[y+1][x-1] +
		       yp[y+1][x  ] +
		       yp[y+1][x+1]    +4)/9;
	up2[y][x] = (  up[y-1][x-1] +
		       up[y-1][x  ] +
		       up[y-1][x+1] +
		       up[y  ][x-1] +
		       up[y  ][x  ] +
		       up[y  ][x+1] +
		       up[y+1][x-1] +
		       up[y+1][x  ] +
		       up[y+1][x+1]    +4)/9;
	vp2[y][x] = (  vp[y-1][x-1] +
		       vp[y-1][x  ] +
		       vp[y-1][x+1] +
		       vp[y  ][x-1] +
		       vp[y  ][x  ] +
		       vp[y  ][x+1] +
		       vp[y+1][x-1] +
		       vp[y+1][x  ] +
		       vp[y+1][x+1]    +4)/9;
      }

  // Copy border from old image to filtered one.

  for (int x=0;x<param.width;x++)
    {
      yp2[  0][x]=yp[  0][x]; up2[  0][x]=up[  0][x]; vp2[  0][x]=vp[  0][x];
      yp2[h-1][x]=yp[h-1][x]; up2[h-1][x]=up[h-1][x]; vp2[h-1][x]=vp[h-1][x];
    }

  for (int y=0;y<param.height;y++)
    {
      yp2[y][  0]=yp[y][  0]; up2[y][0  ]=up[y][  0]; vp2[y][  0]=vp[y][  0];
      yp2[y][w-1]=yp[y][w-1]; up2[y][w-1]=up[y][w-1]; vp2[y][w-1]=vp[y][w-1];
    }
}


template <class Pel> void ConvolveH(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				    const Array<float>& filter,bool useborder)
{
  int left = -filter.AskBase();
  int right=  filter.AskSize()-left-1;

  int border=0;
  if (useborder) border=src.AskBorderWidth();

  int xs = -border+left; if (xs<0) xs=0;
  int xe = src.AskWidth()-1+border-right; if (xe>src.AskWidth()-1) xe=src.AskWidth()-1;

  // cout << "H-Filtering from " << xs << " to " << xe << endl;

  int h = src.AskHeight();
  int borderw = src.AskBorderWidth();

  dst.Create(xe-xs+1,h,1,1,borderw);

  const float* f = filter.Data_const();

  const Pel*const* sp = src.AskFrame_const();
        Pel*const* dp = dst.AskFrame();

  for (int y=-borderw;y<h+borderw;y++)
    for (int x=xs;x<=xe;x++)
      {
	float sum=0.0;
	for (int xx=-left;xx<=right;xx++)
	  sum += f[xx]*sp[y][x+xx];
    
	dp[y][x-xs] = (Pel)sum;
      }
}


template <class Pel> void ConvolveV(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				    const Array<float>& filter,bool useborder)
{
  int top   = -filter.AskBase();
  int bottom=  filter.AskSize()-top-1;

  int border=0;
  if (useborder) border=src.AskBorderWidth();

  int ys = -border+top; if (ys<0) ys=0;
  int ye = src.AskHeight()-1+border-bottom; if (ye>src.AskHeight()-1) ye=src.AskHeight()-1;

  // cout << "V-Filtering from " << ys << " to " << ye << endl;

  int w = src.AskWidth();
  int borderw = src.AskBorderWidth();

  dst.Create(w,ye-ys+1,1,1,borderw);

  const float* f = filter.Data_const();

  const Pel*const* sp = src.AskFrame_const();
        Pel*const* dp = dst.AskFrame();

  for (int y=ys;y<=ye;y++)
    for (int x=-borderw;x<w+borderw;x++)
      {
	float sum=0.0;
	for (int yy=-top;yy<=bottom;yy++)
	  sum += f[yy]*sp[y+yy][x];
    
	dp[y-ys][x] = (Pel)sum;
      }
}


template <class Pel> void ConvolveHV(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				     const Array<float>& filter,bool useborder)
{
  Bitmap<Pel> tmpbm;
  ConvolveH(src,tmpbm,filter,useborder);
  ConvolveV(tmpbm,dst,filter,useborder);
}


void NormalizeFilter(Array<float>& filter)
{
  float sum=0.0;
  int i0 = filter.AskStartIdx();
  int i1 = filter.AskEndIdx();

  float* f = filter.Data();

  for (int i=i0;i<=i1;i++)
    sum += f[i];

  const float fact = 1.0/sum;

  for (int i=i0;i<=i1;i++)
    f[i] *= fact;
}


void CreateGaussFilter(Array<float>& filter,float sigma,float cutoffval,bool normalize)
{
#define MAXRANGE 100
  float filt[MAXRANGE];

  float minus_twosigma2inv = -1.0/(2*sigma*sigma);

  int lastidx=MAXRANGE-1;
  for (int i=0;i<MAXRANGE;i++)
    {
      filt[i] = exp(i*i*minus_twosigma2inv);

      if (filt[i] < cutoffval)
	{ lastidx = i-1; break; }
    }

  if (lastidx==MAXRANGE-1)
    throw "CreateGaussFilter(): Gauss filter is too wide.";

  filter.Create(2*lastidx+1 , -lastidx);
  float* f = filter.Data();

  for (int i=0;i<=lastidx;i++)
    f[-i]=f[i]=filt[i];

  if (normalize) NormalizeFilter(filter);
}


void CreateGaussDerivFilter(Array<float>& filter,float sigma,float cutoffval=0.01)
{
  CreateGaussFilter(filter,sigma,cutoffval,false);

  for (int i=filter.AskStartIdx();i<=filter.AskEndIdx();i++)
    filter.Data()[i] *= i;

  // normalize

  float sum=0.0;

  int i0 = filter.AskStartIdx();
  int i1 = filter.AskEndIdx();

  float* f = filter.Data();

  for (int i=i0;i<=i1;i++)
    sum += i*f[i];

  const float fact = 1.0/sum;

  for (int i=i0;i<=i1;i++)
    f[i] *= fact;
}


template class Array<float>;
template void ConvolveH (const Bitmap<Pixel>&,Bitmap<Pixel>&,const Array<float>&,bool);
template void ConvolveV (const Bitmap<Pixel>&,Bitmap<Pixel>&,const Array<float>&,bool);
template void ConvolveHV(const Bitmap<Pixel>&,Bitmap<Pixel>&,const Array<float>&,bool);

template void ConvolveH (const Bitmap<float>&,Bitmap<float>&,const Array<float>&,bool);
template void ConvolveV (const Bitmap<float>&,Bitmap<float>&,const Array<float>&,bool);
template void ConvolveHV(const Bitmap<float>&,Bitmap<float>&,const Array<float>&,bool);

#include "libvideogfx/containers/array.cc"
