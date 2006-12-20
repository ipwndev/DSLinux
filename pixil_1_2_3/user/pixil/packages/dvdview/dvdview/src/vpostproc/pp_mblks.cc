
#include "vpostproc/pp_mblks.hh"
#include "video12/constants.hh"


Pixel maximize(Pixel v)
{
  if (v<128) return 255;
  else       return   0;
}


void VideoPostprocessor_MBBoundaries::ShowMBRows(DecodedImageData* decimg)
{
  DecodedImageData* dimg = Accumulate(decimg);
  if (!dimg)
    { return; }

  int blkheight;
  if (dimg->m_pichdr1.m_picture_structure == PICSTRUCT_FramePicture)
    blkheight=16;
  else
    blkheight=32;


  Pixel*const* pix = dimg->m_image.AskFrameY();
  ImageParam_YUV param;
  dimg->m_image.GetParam(param);

  if (d_maxmode)
    {
      for (int y=0;y<param.height;y+=blkheight)
	for (int x=0;x<param.width;x+=16)
	  {
	    pix[y][x] = maximize(pix[y][x]);
	    if (d_bigmarks)
	      {
		pix[y            ][x+15] = maximize(pix[y            ][x+15]);
		pix[y+blkheight-1][x   ] = maximize(pix[y+blkheight-1][x   ]);
		pix[y+blkheight-1][x+15] = maximize(pix[y+blkheight-1][x+15]);
	      }
	  }
    }
  else
    {
      Assert(d_staticmode);

      for (int y=0;y<param.height;y+=blkheight)
	for (int x=0;x<param.width;x+=16)
	  {
	    pix[y][x] = d_val;
	    if (d_bigmarks)
	      {
		pix[y            ][x+15] = d_val;
		pix[y+blkheight-1][x   ] = d_val;
		pix[y+blkheight-1][x+15] = d_val;
	      }
	  }
    }

  d_next->ShowMBRows(dimg);
}

void VideoPostprocessor_MBBoundaries::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,true);

  d_next->BeginPicture(dimg);
}

void VideoPostprocessor_MBBoundaries::FinishedPicture()
{
  d_next->FinishedPicture();
}
