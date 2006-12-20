
#include "vpostproc/pp_mv.hh"
#include "video12/constants.hh"
#include "libvideogfx/graphics/draw/draw.hh"

#include <iostream.h>


inline bool VideoPostprocessor_MotionVector::NeedsPictureData(uint3 pictype) const
{
  assert(d_next);

  return ((pictype == PICTYPE_B && d_show_b) ||
	  (pictype == PICTYPE_P && d_show_p) ||
	  d_next->NeedsPictureData(pictype));
}

bool VideoPostprocessor_MotionVector::NeedsMBData(uint3 pictype) const
{
  return NeedsPictureData(pictype);
}


void VideoPostprocessor_MotionVector::ShowMBRows(DecodedImageData* decimg)
{
  if (!decimg->m_picdata1)
    {
      d_next->ShowMBRows(decimg);
      return;
    }

  if (decimg->m_pichdr1.m_picture_coding_type == PICTYPE_B && !d_show_b)
    {
      d_next->ShowMBRows(decimg);
      return;
    }

  if (decimg->m_pichdr1.m_picture_coding_type == PICTYPE_P && !d_show_p)
    {
      d_next->ShowMBRows(decimg);
      return;
    }

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

  Bitmap<Pixel>& bm_y = dimg->m_image.AskBitmap(Image<Pixel>::Bitmap_Y);
  Bitmap<Pixel>& bm_u = dimg->m_image.AskBitmap(Image<Pixel>::Bitmap_U);
  Bitmap<Pixel>& bm_v = dimg->m_image.AskBitmap(Image<Pixel>::Bitmap_V);

  for (int y=0;y<mbs.AskHeight();y++)
    for (int x=0;x<mbs.AskWidth();x++)
      {
        const Macroblock& mb = dimg->m_picdata1->m_codedimage.Ask_const(y,x);

	int x0 = x*16+8;
	int y0 = y*blkheight+blkheight/2;

        if (mb.m_HasMotionForward && d_show_forward)
          {
	    int x1 = x0+mb.m_forward1.m_habs/2;
	    int y1 = y0+mb.m_forward1.m_vabs/2;

	    DrawLine(bm_y,x0,y0,x1,y1,(Pixel)255);
	    if (d_colored)
	      {
		DrawLine(bm_u,x0/2,y0/2,x1/2,y1/2,(Pixel)250);
		DrawLine(bm_v,x0/2,y0/2,x1/2,y1/2,(Pixel)250);
	      }
	  }

        if (mb.m_HasMotionBackward && d_show_backward)
          {
	    int x1 = x0+mb.m_backward1.m_habs/2;
	    int y1 = y0+mb.m_backward1.m_vabs/2;

	    DrawLine(bm_y,x0,y0,x1,y1,(Pixel)255);
	    if (d_colored)
	      {
		DrawLine(bm_u,x0/2,y0/2,x1/2,y1/2,(Pixel)10);
		DrawLine(bm_v,x0/2,y0/2,x1/2,y1/2,(Pixel)10);
	      }
	  }
      }

  d_next->ShowMBRows(dimg);
}


void VideoPostprocessor_MotionVector::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,true);

  d_next->BeginPicture(dimg);
}

void VideoPostprocessor_MotionVector::FinishedPicture()
{
  d_next->FinishedPicture();
}
