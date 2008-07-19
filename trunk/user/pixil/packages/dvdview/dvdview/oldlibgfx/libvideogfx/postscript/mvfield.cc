
#include "libvideogfx/postscript/mvfield.hh"

MotionField2Postscript::MotionField2Postscript()
  : d_ostr(NULL)
{
}

MotionField2Postscript::~MotionField2Postscript()
{
}

void MotionField2Postscript::DrawMotionField(const Bitmap<MotVec>& mv,int hperblk,int vperblk)
{
  float blkwidth  = d_area.width /mv.AskWidth();
  float blkheight = d_area.height/mv.AskHeight();

  const MotVec*const* mvptr = mv.AskFrame_const();

  *d_ostr << "0 setlinewidth\n";
  for (int y=0;y<=mv.AskHeight();y++)
    {
      *d_ostr << (d_area.xoffs*cm2pts) << ' ' << (d_area.yoffs-y*blkheight)*cm2pts << " moveto " << d_area.width*cm2pts << " 0 rlineto\n";
    }
  *d_ostr << "stroke\n";

  for (int x=0;x<=mv.AskWidth();x++)
    {
      *d_ostr << (d_area.xoffs+x*blkwidth)*cm2pts << ' ' << (d_area.yoffs*cm2pts) << " moveto 0 " << -d_area.height*cm2pts << " rlineto\n";
    }
  *d_ostr << "stroke\n";

  *d_ostr << "1 setlinewidth\n";

  for (int y=0;y<mv.AskHeight();y++)
    {
      for (int x=0;x<mv.AskWidth();x++)
	{
	  *d_ostr << (d_area.xoffs+x*blkwidth+blkwidth/2)*cm2pts << ' ' << (d_area.yoffs-y*blkheight-blkheight/2)*cm2pts << " moveto "
		  << mvptr[y][x].h*blkwidth*cm2pts/hperblk       << ' ' << -mvptr[y][x].v*blkheight*cm2pts/vperblk << " rlineto\n";
	}
      *d_ostr << "stroke\n";
    }
}
