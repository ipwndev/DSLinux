/*********************************************************************
  mvfield.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   02/Jun/99 - Dirk Farin
     - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_POSTSCRIPT_MVFIELD_HH
#define LIBVIDEOGFX_POSTSCRIPT_MVFIELD_HH

#include "libvideogfx/graphics/motvec/mv.hh"
#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/postscript/layout.hh"
#include <iostream.h>


class MotionField2Postscript
{
public:
  MotionField2Postscript();
  ~MotionField2Postscript();

  void SetOutput(ostream& ostr) { d_ostr=&ostr; }

  void SetPrintingArea(const PrintingArea& a) { d_area=a; }

  void DrawMotionField(const Bitmap<MotVec>&,int hperblk,int vperblk);

private:
  PrintingArea d_area;
  ostream*     d_ostr;
};

#endif
