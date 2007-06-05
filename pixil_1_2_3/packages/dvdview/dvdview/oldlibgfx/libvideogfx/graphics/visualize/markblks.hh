/*********************************************************************
  libvideogfx/graphics/visualize/markblks.hh

  purpose:
    Mark some blocks.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    30.Nov.1999 - Dirk Farin - first revision
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_VISUALIZE_MARKBLKS_HH
#define LIBVIDEOGFX_GRAPHICS_VISUALIZE_MARKBLKS_HH

#include "libvideogfx/graphics/datatypes/motionfield.hh"
#include "libvideogfx/graphics/basic/image.hh"


class BlockMarker
{
public:
  BlockMarker() : d_color(255), d_size_h(16), d_size_v(16), d_inverse(false) { }

  void SetLumaVal(Pixel p) { d_color=p; }
  void SetSize(int h,int v) { d_size_h=h; d_size_v=v; }
  void InverseMarkers(bool flag=true) { d_inverse=flag; }

  void Overlay(Image_YUV<Pixel>&,const Bitmap<bool>&) const;

private:
  Pixel  d_color;
  int    d_size_h,d_size_v;
  bool   d_inverse;
};

#endif

