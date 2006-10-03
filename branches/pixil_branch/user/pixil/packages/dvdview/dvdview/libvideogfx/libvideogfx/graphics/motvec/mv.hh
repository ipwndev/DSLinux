/*********************************************************************
  mv.hh

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

#ifndef LIBVIDEOGFX_GRAPHICS_MOTVEC_MV_HH
#define LIBVIDEOGFX_GRAPHICS_MOTVEC_MV_HH

#include "libvideogfx/graphics/basic/bitmap.hh"


struct FullSearchData
{
  Bitmap<int> error;
};

struct MotVec
{
  int h,v;
};

#endif
