
#include "libvideogfx/postscript/regions.hh"


void Regions2Postscript::DrawRegions(const Bitmap<int>& bm)
{
  const int*const* p = bm.AskFrame_const();

  const int w = bm.AskWidth();
  const int h = bm.AskHeight();

  assert(d_ostr);


  // Draw horizontal boundaries.

  *d_ostr << "0 setlinewidth\n";
  for (int y=0;y<h-1;y++)
    {
      for (int x=0;x<w;x++)
	{
	  if (p[y][x] != p[y+1][x])
	    {
	      // begin line
	      
	      *d_ostr << ((area.xoffs+area.width *x/w)*cm2pts) << ' '
		      << ((area.yoffs-area.height*(y+1)/h)*cm2pts) << " moveto ";


	      // find end

	      int n=1;

	      x++;
	      while (x<w && p[y][x] != p[y+1][x])
		{
		  x++; n++;
		}

	      // draw line

	      *d_ostr << n*area.width/w*cm2pts << " 0 rlineto\n";
	    }
	}
      *d_ostr << "stroke\n";
    }


  // Draw vertical boundaries.

  for (int x=0;x<w-1;x++)
    {
      for (int y=0;y<h;y++)
	{
	  if (p[y][x] != p[y][x+1])
	    {
	      // begin line

	      *d_ostr << ((area.xoffs+area.width *(x+1)/w)*cm2pts) << ' '
		      << ((area.yoffs-area.height*y/h)*cm2pts) << " moveto 0 ";

	      // find end

	      int n=1;

	      y++;
	      while (y<h && p[y][x] != p[y][x+1])
		{
		  y++; n++;
		}

	      // draw line

	      *d_ostr << -n*area.height/h*cm2pts << " rlineto\n";
	    }
	}
      *d_ostr << "stroke\n";
    }
}
