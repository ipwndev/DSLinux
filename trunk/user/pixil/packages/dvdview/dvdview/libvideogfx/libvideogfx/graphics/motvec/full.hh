/*********************************************************************
  full.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   25/May/99 - Dirk Farin
     - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_MOTVEC_FULL_HH
#define LIBVIDEOGFX_GRAPHICS_MOTVEC_FULL_HH

#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/graphics/motvec/mv.hh"


void CalcFullSearch(const Bitmap<Pixel>& img1,const Bitmap<Pixel>& img2,
		    Bitmap<FullSearchData>* searchdata,
		    Bitmap<MotVec>* motiondata,
		    int hblksize,int vblksize,int hrange,int vrange);

#endif
