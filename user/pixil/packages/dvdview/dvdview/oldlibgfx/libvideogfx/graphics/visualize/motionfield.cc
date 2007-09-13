/*
 *  motionfield.cc
 */

#include "motionfield.hh"

#include "libvideogfx/graphics/draw/draw.hh"


void MotionFieldVisualizer::Overlay(Image_YUV<Pixel>& img,const MotionVectorField& mvfld,const Bitmap<bool>* coloredbm) const
{
  const MotionVector*const* mvptr = mvfld.mv.AskFrame_const();

  Bitmap<Pixel>& bm  = img.AskBitmap(Image<Pixel>::Bitmap_Y);

  const bool*const* cbm;
  if (coloredbm) cbm = coloredbm->AskFrame_const();
  else cbm=NULL;

  ImageSpec_YUV spec;
  img.GetParam(spec);

  assert(spec.width  >  (mvfld.blksize_h*(mvfld.nblks_h-1)) &&
	 spec.width  <= (mvfld.blksize_h* mvfld.nblks_h   ));
  assert(spec.height >  (mvfld.blksize_v*(mvfld.nblks_v-1)) &&
	 spec.height <= (mvfld.blksize_v* mvfld.nblks_v   ));

  for (int y=0;y<mvfld.nblks_v;y++)
    for (int x=0;x<mvfld.nblks_h;x++)
      {
	int x0 = x*mvfld.blksize_h+mvfld.blksize_h/2;
	int y0 = y*mvfld.blksize_v+mvfld.blksize_v/2;

	float dx = mvptr[y][x].h;
	float dy = mvptr[y][x].v;

	dx *= d_scale;
	dy *= d_scale;

	float x1= x0+dx;
	float y1= y0+dy;

	Pixel color=d_vectorcolor;
	if (cbm && !cbm[y][x]) color=0;
	DrawLine(bm,x0,y0,(int)x1,(int)y1, color);

	if (d_drawarrows && (abs(mvptr[y][x].h)>0 || abs(mvptr[y][x].v)>0))
	  {
	    float len = sqrt(dx*dx+dy*dy);

	    dx /= len;
	    dy /= len;

	    DrawLine(bm,x0,y0,  (int)(x0+dy*3+dx*3),(int)(y0-dx*3+dy*3), color);
	    DrawLine(bm,x0,y0,  (int)(x0-dy*3+dx*3),(int)(y0+dx*3+dy*3), color);
	  }
      }
}
