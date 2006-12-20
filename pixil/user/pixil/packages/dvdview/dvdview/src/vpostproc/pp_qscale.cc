
#include "vpostproc/pp_qscale.hh"
#include "video12/constants.hh"

#include <iostream.h>


void VideoPostprocessor_QScale::ShowMBRows(DecodedImageData* decimg)
{
  DecodedImageData* dimg = Accumulate(decimg);
  if (!dimg)
    { return; }

  Assert(dimg->m_picdata1);
  Assert(dimg->m_picdata1->m_codedimage.IsInitialized());

  int blkheight;

  if (dimg->m_pichdr1.m_picture_structure == PICSTRUCT_FramePicture)
    blkheight=16;
  else
    blkheight=32;

  const Array2<Macroblock>& mbs = dimg->m_picdata1->m_codedimage;
  Pixel*const* yptr = dimg->m_image.AskFrameY();

  for (int y=0;y<mbs.AskHeight();y++)
    for (int x=0;x<mbs.AskWidth();x++)
      {
	int q=mbs.Ask_const(y,x).m_quantiser_scale_code;
	if (q>blkheight) q=blkheight;

	for (int i=1;i<=q;i++)
	  {
	    if ((y+1)*blkheight-i < decimg->m_height )
	      yptr[(y+1)*blkheight-i][x*16+15] = 255;
	  }
      }

  d_next->ShowMBRows(dimg);
}

void VideoPostprocessor_QScale::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,true);

  d_next->BeginPicture(dimg);
}

void VideoPostprocessor_QScale::FinishedPicture()
{
  d_next->FinishedPicture();
}
