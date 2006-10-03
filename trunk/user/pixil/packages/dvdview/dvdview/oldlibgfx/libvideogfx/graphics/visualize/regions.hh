/*********************************************************************
  libvideogfx/graphics/visualize/regions.hh

  purpose:
    Create false color image of segmentation regions.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    1999/Nov/02 - Dirk Farin - rewritten because original source
                               was lost :(((
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_VISUALIZE_REGIONS_HH
#define LIBVIDEOGFX_GRAPHICS_VISUALIZE_REGIONS_HH

#include "libvideogfx/graphics/basic/image.hh"


class Regions2FalseColors
{
public:
  Regions2FalseColors();

  void DrawRegions(const Bitmap<int>&,Image_RGB<Pixel>&);

private:
  static const int c_InitialNRegions = 100;
  static const int c_Increment       = 100;

  struct Color { Pixel r,g,b; };

  Color* d_regioncolor;
  int    d_nRegions;    // Region 0 is background, other regions: [1 ... d_nRegions].

  void AssignColors(int from,int to);
};



class Regions2BoundaryImage
{
public:
  void DrawRegions(const Bitmap<int>&,Image_YUV<Pixel>&);
};


template <class Pel,class APel> void OverlayAlphaMask(Bitmap<Pel>& bm,const Bitmap<APel>& alphamap,
						      Pixel val);

#endif

