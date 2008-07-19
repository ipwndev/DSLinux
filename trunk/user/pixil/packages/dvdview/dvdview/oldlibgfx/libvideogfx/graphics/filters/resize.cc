
#include "libvideogfx/graphics/filters/resize.hh"

void HalfSize_Subsample(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest)
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

