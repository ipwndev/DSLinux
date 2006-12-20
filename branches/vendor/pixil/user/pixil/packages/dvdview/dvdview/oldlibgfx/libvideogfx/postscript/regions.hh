/*********************************************************************
  postscript/regions.hh

  purpose:
    Draw segmented regions.
    Takes a bitmap a input that has a region id assigned to each
    element. The visualizer class will then draw the borders between
    adjacent regions.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    23/Jul/99 - Dirk Farin - speedup and output file size reduction
      by combining adjacent line segments into a single line.
    21/Jul/99 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_POSTSCRIPT_REGIONS_HH
#define LIBVIDEOGFX_POSTSCRIPT_REGIONS_HH

#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/postscript/layout.hh"
#include <iostream.h>


class Regions2Postscript
{
public:
   Regions2Postscript() : d_ostr(NULL) { }
  ~Regions2Postscript() { }

  void SetOutput(ostream& ostr) { d_ostr=&ostr; }

  void SetPrintingArea(const PrintingArea& a) { area=a; }
  void DrawRegions(const Bitmap<int>&);

private:
  PrintingArea area;
  ostream* d_ostr;
};

#endif
