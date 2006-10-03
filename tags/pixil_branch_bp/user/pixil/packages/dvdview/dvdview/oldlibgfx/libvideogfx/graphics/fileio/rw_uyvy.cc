/*
 *  rw_uyvy.cc
 */

#include "rw_uyvy.hh"


bool CheckImageSize(ifstream& istr,const ImageInfo_Base& sizespec)
{
  long pos = istr.tellg();
  istr.seekg(0,ios::end);
  long filelength = istr.tellg();
  istr.seekg(pos,ios::beg);

  return filelength == sizespec.width*sizespec.height*2;
}


void ReadImage_UYVY(Image_YUV<Pixel>& img,ifstream& istr,const ImageSpec_YUV& spec)
{
  assert(istr.is_open());

  ImageSpec_YUV myspec = spec;

  myspec.chroma  = Chroma422;
  myspec.nocolor = false;

  img.Create(myspec);

  Pixel*const* yp = img.AskFrameY();
  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  uint8* linebuf = new uint8[spec.width*2];

  for (int y=0;y<spec.height;y++)
    {
      istr.read(linebuf,spec.width*2);

      uint8* lp = linebuf;

      for (int x=0;x<spec.width/2;x++)
	{
	  // This ugly piece of code helps the compiler to optimize
	  // this a bit further as he doesn't have to mind the pointers
	  // pointing to the same memory locations.
	  // Thus all four assignments could be performed in parallel.

	  uint8 a,b,c,d;

	  a = *lp++;
	  b = *lp++;
	  c = *lp++;
	  d = *lp++;

	  up[y][x]     = a;
	  yp[y][2*x  ] = b;
	  vp[y][x]     = c;
	  yp[y][2*x+1] = d;
	}
    }

  delete[] linebuf;
}





void WriteImage_UYVY(Image_YUV<Pixel>& img,ofstream& ostr)
{
  ImageParam_YUV param;

  img.GetParam(param);

  assert(param.chroma  == Chroma422);
  assert(param.nocolor == false);

  // Write file

  const Pixel*const* yp = img.AskFrameY_const();
  const Pixel*const* up = img.AskFrameU_const();
  const Pixel*const* vp = img.AskFrameV_const();

  uint8* linebuf = new uint8[param.width*2];

  for (int y=0;y<param.height;y++)
    {
#if 1
      uint8* lp;
      const uint8* p;

      // luminance

      lp = &linebuf[1];
      p = yp[y];

      for (int x=0;x<param.width;x++)
	{
	  *lp = *p++;
	  lp+=2;
	}

      // chrominance

      for (int x=0;x<param.width/2;x++)
	{
	  // The same ugly code as in ReadImage_UYVY().

	  uint8 a,b;
	  a = up[y][x];
	  b = vp[y][x];
	  linebuf[4*x  ] = a;
	  linebuf[4*x+2] = b;
	}
#endif
      ostr.write(linebuf,param.width*2);
    }

  delete[] linebuf;
}
