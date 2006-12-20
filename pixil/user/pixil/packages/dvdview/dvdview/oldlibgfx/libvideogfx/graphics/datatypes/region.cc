/*
 *  region.cc
 */

#include "region.hh"


RegionStripeIterator::RegionStripeIterator(bool sglpixel,const class Region* region)
  : d_region(region),
    d_sglPixel(sglpixel),
    d_nextIdx(0),
    d_currRun(0)
{
}


RegionStripe RegionStripeIterator::Get()
{
  if (d_sglPixel)
    {
      RegionStripe s = d_region->d_stripes[d_nextIdx];
      s.x += d_currRun;
      s.xrun = 1;
      d_currRun++;

      if (d_currRun == d_region->d_stripes[d_nextIdx].xrun)
	{
	  d_currRun=0;
	  d_nextIdx++;
	}

      return s;
    }
  else
    {
      return d_region->d_stripes[d_nextIdx++];
    }
}


bool RegionStripeIterator::NoMoreStripes()
{
  return d_nextIdx == d_region->d_nStripes;
}




Region::Region()
{
  d_stripes = new RegionStripe[1000];
  d_nStripes = 0;
  d_nSize = 1000;
}


Region::~Region()
{
  delete[] d_stripes;
}


Region::Region(const Region& r)
{
  d_nSize = max(r.d_nStripes,10);
  d_stripes = new RegionStripe[d_nSize];
  d_nStripes = r.d_nStripes;

  for (int i=0;i<d_nStripes;i++)
    d_stripes[i] = r.d_stripes[i];
}


const Region& Region::operator=(const Region& r)
{
  delete[] d_stripes;
  d_nSize = max(r.d_nStripes,10);
  d_stripes = new RegionStripe[d_nSize];
  d_nStripes = r.d_nStripes;

  for (int i=0;i<d_nStripes;i++)
    d_stripes[i] = r.d_stripes[i];
}


void Region::AddHRun(int newx,int newy,int newxrun)
{
  for(int i=0;i<newxrun;i++)
    {
      if (d_nStripes == d_nSize)
	{
	  RegionStripe* s = new RegionStripe[d_nSize*2];
	  for (int n=0;n<d_nSize;n++)
	    s[n]=d_stripes[n];

	  delete[] d_stripes;
	  d_stripes=s;
	  d_nSize *= 2;
	}

      RegionStripe s;
      s.x    = newx+i;
      s.y    = newy;
      s.xrun = 1;

      bool alreadycontained=false;

      for (int n=0;n<d_nStripes && !alreadycontained;n++)
	{
	  if (d_stripes[n] == s)
	    { alreadycontained=true; }
	}

      if (!alreadycontained)
	{
	  d_stripes[d_nStripes] = s;
	  d_nStripes++;
	}
    }

#if 0
  // Check if we can append the new stripe to an existing one.

  for (int i=d_nStripes-1;i>=0 && d_stripes[i].y>=newy ;i--)
    {
      if (d_stripes[i].y == newy)
	{
	  RegionStripe& s = d_stripes[i];

	  // 3 combination cases: new stripe connects to the front, to the end or is in the middle

	  if (newx < s.x && newx+newxrun>=s.x)
	    {
	      int newend = max(s.x+s.xrun , newx+xrun);
	      s.x = newx;
	      s.xrun = newend-s.x;
	    }

	  // Handle this case:
	  //     XXXXXXXXXXXXXX                   <- new
	  // AAAAAAAA      BBBBBBBBBBBBBB
	}
    }
#endif
}


void Region::RemovePixel(int x,int y)
{
  for (int i=0;i<d_nStripes;i++)
    {
      if (d_stripes[i].y == y &&
	  d_stripes[i].x == x)
	{
	  assert(d_stripes[i].xrun==1);
	  d_nStripes--;
	  d_stripes[i]=d_stripes[d_nStripes];
	  return;
	}
    }
}


bool Region::ContainsPixel(int x,int y) const
{
  for (int i=0;i<d_nStripes;i++)
    if (d_stripes[i].y == y &&
	d_stripes[i].x <= x &&
	d_stripes[i].x+d_stripes[i].xrun-1 >= x)
      return true;

  return false;
}


void Region::Shift(int x,int y) // Shift region x,y pixels right,down.
{
  for (int i=0;i<d_nStripes;i++)
    {
      d_stripes[i].x += x;
      d_stripes[i].y += y;
    }
}


void Region::Union(const Region& r)
{
  RegionStripeIterator iter=r.GetStripeIterator(false);

  while (!iter.NoMoreStripes())
    {
      AddStripe(iter.Get());
    }
}

void Region::Cut(const Region& r)
{
  RegionStripeIterator iter=r.GetStripeIterator(false);

  while (!iter.NoMoreStripes())
    {
      RegionStripe s = iter.Get();
      RemovePixel(s.x,s.y);
    }
}


Region Region::Intersection(const Region& r)
{
  Region newreg;
  RegionStripeIterator iter=r.GetStripeIterator(true);

  while (!iter.NoMoreStripes())
    {
      RegionStripe s = iter.Get();
      if (ContainsPixel(s.x,s.y))
	newreg.AddPixel(s.x,s.y);
    }

  return newreg;
}


bool Region::operator<=(const Region& r) const
{
  RegionStripeIterator iter=GetStripeIterator(true);

  while (!iter.NoMoreStripes())
    {
      RegionStripe s=iter.Get();
      if (!r.ContainsPixel(s.x,s.y))
	return false;
    }

  return true;
}


RegionStripeIterator Region::GetStripeIterator(bool sglpixel) const
{
  return RegionStripeIterator(sglpixel,this);
}


Region RegionFromString(const char* str)
{
  // Find zero point.

  int x0=0,y0=0;

  for (const char* p=str;*p!='0' && *p!='.';p++,x0++)
    {
      if (*p=='\n') { x0=-1; y0++; }
    }

  int x=0,y=0;
  Region r;
  for (const char* p=str;*p!=0;p++,x++)
    {
      if (*p=='X' || *p=='0') { r.AddPixel(x-x0,y-y0); }
      if (*p=='\n') { x=-1; y++; }
    }

  return r;
}


template <class T> void DrawRegion(Bitmap<T>& bm,const Region& r,T color)
{
  RegionStripeIterator iter=r.GetStripeIterator(false);
  T*const* p = bm.AskFrame();

  while (!iter.NoMoreStripes())
    {
      RegionStripe s = iter.Get();
      for (int x=0;x<s.xrun;x++)
	p[s.y][s.x+x]=color;
    }
}


template void DrawRegion(Bitmap<bool>& ,const Region&,bool);
template void DrawRegion(Bitmap<Pixel>&,const Region&,Pixel);
