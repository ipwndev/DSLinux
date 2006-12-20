/*
 *  regions.cc
 */

#include <stdlib.h>

#include "regions.hh"


Regions2FalseColors::Regions2FalseColors()
{
  d_regioncolor = new Color[c_InitialNRegions+1];
  d_nRegions=c_InitialNRegions;

  d_regioncolor[0].r = 
  d_regioncolor[0].g = 
  d_regioncolor[0].b = 0;

  AssignColors(1,c_InitialNRegions);
}


void Regions2FalseColors::DrawRegions(const Bitmap<int>& bm,Image_RGB<Pixel>& img)
{
  const int*const* reg = bm.AskFrame_const();

  ImageSpec spec;
  spec.width  = bm.AskWidth();
  spec.height = bm.AskHeight();
  spec.halign = 8;

  img.Create(spec);

  Pixel*const* rp = img.AskFrameR();
  Pixel*const* gp = img.AskFrameG();
  Pixel*const* bp = img.AskFrameB();

  for (int y=0;y<spec.height;y++)
    for (int x=0;x<spec.width;x++)
      {
	int region = reg[y][x];

	if (region<0) region=0;

	if (region > d_nRegions)
	  {
	    int nNewRegions = region+c_Increment;
	    Color* newmap = new Color[nNewRegions+1];

	    // keep old colors

	    for (int i=0;i<=d_nRegions;i++)
	      newmap[i] = d_regioncolor[i];

	    // assign new colors to new entries

	    delete[] d_regioncolor;
	    d_regioncolor = newmap;

	    AssignColors(d_nRegions+1,nNewRegions);

	    d_nRegions = nNewRegions;
	  }

	rp[y][x] = d_regioncolor[region].r;
	gp[y][x] = d_regioncolor[region].g;
	bp[y][x] = d_regioncolor[region].b;
      }
}


void Regions2FalseColors::AssignColors(int from,int to)
{
  for (int i=from;i<=to;i++)
    {
      d_regioncolor[i].r = rand()&255;
      d_regioncolor[i].g = rand()&255;
      d_regioncolor[i].b = rand()&255;
    }
}







void Regions2BoundaryImage::DrawRegions(const Bitmap<int>& regions,Image_YUV<Pixel>& img)
{
  const int*const* sp = regions.AskFrame_const();

  ImageSpec_YUV spec;
  spec.width   = regions.AskWidth();
  spec.height  = regions.AskHeight();
  spec.nocolor = true;
  spec.halign  = 8;
  img.Create(spec);

  Pixel*const* dp = img.AskFrameY();
  for (int y=0;y<spec.height-1;y++)
    for (int x=0;x<spec.width-1;x++)
      {
	if (sp[y][x] != sp[y+1][x]) dp[y][x]=0; else
	if (sp[y][x] != sp[y][x+1]) dp[y][x]=0; else dp[y][x]=255;
      }

  int x,y;
  x=spec.width-1;
  for (y=0;y<spec.height-1;y++)
    {
      if (sp[y][x] != sp[y+1][x]) dp[y][x]=0; else dp[y][x]=255;
    }
  
  y=spec.height-1;
  for (int x=0;x<spec.width-1;x++)
    {
      if (sp[y][x] != sp[y][x+1]) dp[y][x]=0; else dp[y][x]=255;
    }


  // draw image border

  for (int x=0;x<spec.width;x++)
    dp[0][x]=dp[spec.height-1][x]=0;
  for (int y=0;y<spec.height;y++)
    dp[y][0]=dp[y][spec.width-1]=0;
}


template <class Pel,class APel> void OverlayAlphaMask(Bitmap<Pel>& bm,const Bitmap<APel>& alphamap,
						      Pixel val)
{
  Pel*const* p = bm.AskFrame();
  const APel*const* ap = alphamap.AskFrame_const();

  assert(bm.AskWidth() ==alphamap.AskWidth());
  assert(bm.AskHeight()==alphamap.AskHeight());

  for (int y=0;y<bm.AskHeight();y++)
    for (int x=0;x<bm.AskWidth();x++)
      {
	if (ap[y][x]<128 && ((x+y)%4)==0)
	  { p[y][x]=val; }
	else if (x>0 && ap[y][x] != ap[y][x-1])
	  {
	    if (ap[y][x]<ap[y][x-1]) p[y][x]=val; else p[y][x-1]=val;
	  }
	else if (y>0 && ap[y][x] != ap[y-1][x])
	  {
	    if (ap[y][x]<ap[y-1][x]) p[y][x]=val; else p[y-1][x]=val;
	  }
      }
}


template void OverlayAlphaMask(Bitmap<Pixel>&,const Bitmap<Pixel>&,Pixel);

//template class Bitmap<int>;

//#include "libvideogfx/graphics/basic/bitmap.cc"
