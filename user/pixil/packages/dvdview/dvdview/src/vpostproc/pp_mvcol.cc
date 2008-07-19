
#include "vpostproc/pp_mvcol.hh"
#include "video12/constants.hh"
#include "libvideogfx/graphics/color/colorspace.hh"
#include <math.h>

#include <iostream.h>

#ifndef PI
#define PI 3.1415926535
#endif


bool VideoPostprocessor_MVCol::NeedsPictureData(uint3 pictype) const
{
  return true;

  assert(d_next);

  return ((pictype == PICTYPE_B && d_showb) ||
	  (pictype == PICTYPE_P && d_showp) ||
	  d_next->NeedsPictureData(pictype));
}

bool VideoPostprocessor_MVCol::NeedsMBData(uint3 pictype) const
{
  return NeedsPictureData(pictype);
}

void VideoPostprocessor_MVCol::ShowMBRows(DecodedImageData* decimg)
{
  DecodedImageData* dimg = Accumulate(decimg);
  if (!dimg)
    { return; }

  Pixel*const* yy = dimg->m_image.AskFrameY();
  Pixel*const* cb = dimg->m_image.AskFrameU();
  Pixel*const* cr = dimg->m_image.AskFrameV();

  ImageParam_YUV param;
  dimg->m_image.GetParam(param);

  int cw = ((param.chroma==Chroma444)?16:8);
  int ch = ((param.chroma==Chroma420)?8:16);

  if (!dimg->m_picdata1)
    { if (d_hold) RedrawHold(yy,cb,cr,cw,ch); d_next->ShowMBRows(decimg); return; }

  // Skip drawing in unselected picture types. If hold mode is selected, draw
  // old vectors again instead.

  if (dimg->m_pichdr1.m_picture_coding_type==PICTYPE_I)
    { if (d_hold) RedrawHold(yy,cb,cr,cw,ch); d_next->ShowMBRows(decimg); return; }

  if (dimg->m_pichdr1.m_picture_coding_type==PICTYPE_P && !d_showp)
    { if (d_hold) RedrawHold(yy,cb,cr,cw,ch); d_next->ShowMBRows(decimg); return; }

  if (dimg->m_pichdr1.m_picture_coding_type==PICTYPE_B && !d_showb)
    { if (d_hold) RedrawHold(yy,cb,cr,cw,ch); d_next->ShowMBRows(decimg); return; }


  if (!d_blks.IsInitialized())
    {
      int w = param.width;
      int h = param.height;
      
      d_blks.Create((w+15)/16,(h+15)/16);
    }

  if (dimg->m_pichdr1.m_picture_structure == PICSTRUCT_FramePicture)
    d_fields=false;
  else
    d_fields=true;


  // Draw and store new vectors.

  int h = dimg->m_picdata1->m_codedimage.AskHeight();
  int w = dimg->m_picdata1->m_codedimage.AskWidth();

  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      {
        const Macroblock& mb = dimg->m_picdata1->m_codedimage.Ask_const(y,x);

	BlockColorInfo bci;

	float magnitude=0.0;
	float angle;

	{
	  // forward MV

	  int h,v;

	  float m;

	  if (mb.m_HasMotionForward && d_fwd)
	    {
	      h = abs(mb.m_forward1.m_habs);
	      v = abs(mb.m_forward1.m_vabs);

	      m = sqrt((float)(h*h+v*v));
	      magnitude=m;
	    }
	    

	  // backward MV
	    
	  if (mb.m_HasMotionBackward && d_bkw)
	    {
	      h = abs(mb.m_backward1.m_habs);
	      v = abs(mb.m_backward1.m_vabs);

	      m = sqrt((float)(h*h+v*v));
	      magnitude += m;
	      magnitude /= 2;
	    }
	}

	{
	  int h=0,v=0;
	    
	  if (mb.m_HasMotionForward && d_fwd)
	    {
	      h += mb.m_forward1.m_habs;
	      v += mb.m_forward1.m_vabs;
	    }
	    

	  // backward MV
	    
	  if (mb.m_HasMotionBackward && d_bkw)
	    {
	      h -= mb.m_backward1.m_habs;
	      v -= mb.m_backward1.m_vabs;
	    }

	  if (h!=0 || v!=0)
	    {
	      float w;
	      if (h==0)
		{
		  if (v<0) w=PI/2.0;
		  else     w=PI*3.0/2.0;
		}
	      else
		{
		  if (h>0)
		    {
		      w = atan(((float)-v)/h);
		      if (w<0) w+= 2*PI;
		    }
		  else
		    {
		      w = -atan(((float)v)/h);
		      w+= PI;
		    }
		}

	      angle=w;
	    }
	}

	magnitude /= 32; if (magnitude>1.0) magnitude=1.0;
	angle /= 2*PI;   if (angle>2*PI) angle=2*PI;

	bci.hue = angle;
	if (magnitude==0)
	  bci.sat=0;
	else
	  bci.sat = magnitude*0.5 + 0.5;

	d_blks.Ask(y,x)=bci;
      }

  RedrawHold(yy,cb,cr,cw,ch);

  d_next->ShowMBRows(dimg);
}


void VideoPostprocessor_MVCol::RedrawHold(Pixel*const* yp,Pixel*const* cb,Pixel*const* cr,
					  int cw,int ch)
{
  if (!d_blks.IsInitialized())
    return;

  // Redraw stored vectors.

  int h = d_blks.AskHeight();
  int w = d_blks.AskWidth();

  int blkh = 16;
  if (d_fields) blkh*=2;

  if (d_fields) h/=2;

  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      {
	BlockColorInfo bci;

	bci=d_blks.Ask(y,x);
	for (int yy=0;yy<ch;yy++)
	  for (int xx=0;xx<cw;xx++)
	    {
	      float rd,gd,bd;
	      uint8 r,g,b;
	      uint8 yc,uc,vc;

	      float hue = bci.hue;
	      float sat = bci.sat;
	      float val = ((float)(yp[blkh*y+yy][16*x+xx]))/255;

	      HSB2RGB(hue,sat,val, rd,gd,bd);
	      r = (uint8)(255*rd);
	      g = (uint8)(255*gd);
	      b = (uint8)(255*bd);
	      RGB2YUV(r,g,b,yc,uc,vc);

	      //yp[16*y+yy][16*x+xx] = yc;
	      cb[ch*y+yy][cw*x+xx] = uc;
	      cr[ch*y+yy][cw*x+xx] = vc;
	    }
      }
}


void VideoPostprocessor_MVCol::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,true);

  d_next->BeginPicture(dimg);
}

void VideoPostprocessor_MVCol::FinishedPicture()
{
  d_next->FinishedPicture();
}


#include "libvideogfx/containers/array2.cc"
template class Array2<BlockColorInfo>;
