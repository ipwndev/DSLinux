/*
 *  full.cc
 */

#include <iostream.h>

#include "full.hh"


inline int Abs(int a)
{
  if (a<0) return -a; else return a;
}

void CalcFullSearch(const Bitmap<Pixel>& img1,const Bitmap<Pixel>& img2,
		    Bitmap<FullSearchData>* searchdata,
		    Bitmap<MotVec>* motiondata,
		    int hblksize,int vblksize,int hrange,int vrange)
{
  assert(img1.AskWidth()  == img2.AskWidth());
  assert(img1.AskHeight() == img2.AskHeight());

  int width  = img1.AskWidth();
  int height = img1.AskHeight();

  int hsize = (img1.AskWidth() +hblksize-1)/hblksize;
  int vsize = (img1.AskHeight()+vblksize-1)/vblksize;

  FullSearchData*const* fsdptr = NULL;
  MotVec*const* mvptr = NULL;
  if (searchdata) { searchdata->Create(hsize,vsize); fsdptr = searchdata->AskFrame(); }
  if (motiondata) { motiondata->Create(hsize,vsize); mvptr = motiondata->AskFrame(); }

  const Pixel*const* i1 = img1.AskFrame_const();
  const Pixel*const* i2 = img2.AskFrame_const();

  for (int y=0;y<vsize;y++)
    { cout << '.'; cout.flush();
    for (int x=0;x<hsize;x++)
      {
	int*const* fsderr = NULL;
	if (fsdptr)
	  {
	    FullSearchData& fsd = fsdptr[y][x];
	    fsd.error.Create(2*hrange+1,2*vrange+1);
	    fsderr = fsd.error.AskFrame();
	  }

	int x0 = x*hblksize;
	int y0 = y*vblksize;

	int besterror = 0x7FFFFFFF;
	int besth=0,bestv=0;

	for (int dy=-vrange;dy<=vrange;dy++)
	  for (int dx=-hrange;dx<=hrange;dx++)
	    {
	      int error;

	      if (x0+dx < 0 || y0+dy < 0 ||
		  x0+hblksize+dx > width ||
		  y0+vblksize+dy > height)
		{
		  error = 0x7FFFFFFF;
		}
	      else
		{
		  error = 0;
		  for (int yy=0;yy<vblksize;yy++)
		    for (int xx=0;xx<hblksize;xx++)
		      error += Abs(i1[y0+yy][x0+xx] - i2[y0+yy+dy][x0+xx+dx]);
		}

	      if ((error<besterror) ||
		  (error==besterror && Abs(dy)+Abs(dx) < Abs(besth)+Abs(bestv)))
		{
		  besterror = error;
		  besth = dx;
		  bestv = dy;
		}

	      if (fsderr) fsderr[dy+vrange][dx+hrange] = error;
	    }

	if (mvptr)
	  {
	    mvptr[y][x].h = besth;
	    mvptr[y][x].v = bestv;
	  }
      }
    }
}
