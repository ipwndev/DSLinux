/*********************************************************************
  libvideogfx/graphics/datatypes/motionfield.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    1999/Oct/19 - Dirk Farin - integration into CVS, mostly rewritten
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_DATATYPES_MOTIONFIELD_HH
#define LIBVIDEOGFX_GRAPHICS_DATATYPES_MOTIONFIELD_HH

#include "libvideogfx/graphics/basic/bitmap.hh"


struct MotionVector
{
  float h,v;
};


struct MotionVectorField
{
  MotionVectorField(int p_nblks_h,
                    int p_nblks_v,
                    int p_blksize_h = 16,
                    int p_blksize_v = 16)
  {
    blksize_h = p_blksize_h;
    blksize_v = p_blksize_v;
    nblks_h   = p_nblks_h;
    nblks_v   = p_nblks_v;
    mv.Create(nblks_h, nblks_v);
  }

  Bitmap<MotionVector> mv;
  int blksize_h,blksize_v;
  int nblks_h  ,nblks_v;
};

#endif

