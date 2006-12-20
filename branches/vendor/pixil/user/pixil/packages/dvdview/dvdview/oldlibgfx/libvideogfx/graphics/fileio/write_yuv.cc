/*
 *  write_yuv.cc
 */

#include "write_yuv.hh"


FileWriter_YUV1::FileWriter_YUV1()
  : d_yuvstr(NULL),
    d_alphastr(NULL),
    d_write_greyscale_as_color(false),
    d_write_interleaved(false)
{
}


void FileWriter_YUV1::WriteImage(const Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  const Pixel*const* Y = img.AskFrameY_const();
  const Pixel*const* U = (param.nocolor ? NULL : img.AskFrameU_const());
  const Pixel*const* V = (param.nocolor ? NULL : img.AskFrameV_const());

  // write Y

  for (int y=0;y<param.height;y++)
    d_yuvstr->write(Y[y],param.width);

  // write chrominance

  int cw,ch;
  param.GetChromaSizes(cw,ch);

  if (d_write_greyscale_as_color && param.nocolor)
    {
      uint8* buf;
      buf = new uint8[2*cw];

      for (int i=0;i<2*cw;i++)
	buf[i]=128;

      for (int y=0;y<ch;y++)
	d_yuvstr->write(buf,2*cw);

      delete[] buf;
    }
  else if (param.nocolor)
    {
      // write no color information
    }
  else
    {
      if (d_write_interleaved)
	{
	  uint8* buf;
	  buf = new uint8[2*cw];
	  
	  for (int y=0;y<ch;y++)
	    {
	      for (int i=0;i<cw;i++)
		{
		  buf[2*i  ] = U[y][i];
		  buf[2*i+1] = V[y][i];
		}
	      
	      d_yuvstr->write(buf,2*cw);
	    }

	  delete[] buf;
	}
      else
	{
	  for (int y=0;y<ch;y++)
	    d_yuvstr->write(U[y],cw);
	  for (int y=0;y<ch;y++)
	    d_yuvstr->write(V[y],cw);
	}
    }

  // alpha

  if (d_alphastr && param.has_alphamask)
    {
      const Pixel*const* A = img.AskFrameA_const();

      for (int y=0;y<param.height;y++)
	d_alphastr->write(A[y],param.width);
    }
}
