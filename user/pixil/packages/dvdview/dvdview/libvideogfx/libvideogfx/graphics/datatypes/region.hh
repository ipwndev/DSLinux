/*********************************************************************
  libvideogfx/graphics/datatypes/region.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    17/Nov/1999 - Dirk Farin - first revision
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_DATATYPES_REGION_HH
#define LIBVIDEOGFX_GRAPHICS_DATATYPES_REGION_HH

#include "libvideogfx/graphics/basic/bitmap.hh"


class RegionStripe
{
public:
  int x,y;
  int xrun;

  bool operator==(const RegionStripe& s) const { return x==s.x && y==s.y && xrun==s.xrun; }
};


class RegionStripeIterator
{
  friend class Region;
public:
  RegionStripe Get();
  bool NoMoreStripes();

private:
  RegionStripeIterator(bool sglpixel,const class Region*);

  const class Region* d_region;
  bool d_sglPixel;
  int  d_nextIdx;
  int  d_currRun;
};


class Region
{
  friend class RegionStripeIterator;

public:
  Region();
  Region(const Region&);
  ~Region();

  const Region& operator=(const Region&);

  void AddPixel(int x,int y) { AddHRun(x,y,1); }
  void AddHRun(int x,int y,int xrun);
  void AddStripe(const RegionStripe& s) { AddHRun(s.x,s.y,s.xrun); }

  void RemovePixel(int x,int y);

  RegionStripeIterator GetStripeIterator(bool sglpixel=true) const;
  bool ContainsPixel(int x,int y) const;
  //void GetPixelList(Point2D<int>*);

  void Shift(int x,int y); // Shift region x (y) pixels right (down).

  void   Union(const Region&);
  void   Cut  (const Region&);
  Region Intersection(const Region&);

  bool operator<=(const Region&) const;
  bool operator>=(const Region& r) const { return  r <= *this; }
  bool operator==(const Region& r) const { return (r <= *this) && (*this <= r); }

private:
  RegionStripe* d_stripes;
  int d_nStripes;
  int d_nSize;
};

/* String example:
   "  X  \n"
   " X0XX\n"  // Use '.' for '0', when the pixel is not included in the mask.
   "  X  \n";
 */
Region RegionFromString(const char*);
template <class T> void DrawRegion(Bitmap<T>&,const Region&,T color);

#endif

