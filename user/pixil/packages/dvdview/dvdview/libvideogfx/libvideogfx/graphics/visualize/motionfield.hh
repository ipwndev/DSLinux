/*********************************************************************
  libvideogfx/graphics/visualize/motionfield.hh

  purpose:
    Overlay of motion vector field.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    1999/Oct/29 - Dirk Farin - first revision
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_VISUALIZE_MOTIONFIELD_HH
#define LIBVIDEOGFX_GRAPHICS_VISUALIZE_MOTIONFIELD_HH

#include "libvideogfx/graphics/datatypes/motionfield.hh"
#include "libvideogfx/graphics/basic/image.hh"


class MotionFieldVisualizer
{
public:
  MotionFieldVisualizer() : d_vectorcolor(255), d_drawarrows(false), d_scale(1.0) { }

  void SetLumaVal(Pixel p) { d_vectorcolor=p; }
  void DrawArrows(bool flag) { d_drawarrows=flag; }
  void SetScale(float s) { d_scale=s; }

  void Overlay(Image_YUV<Pixel>&,const MotionVectorField&,const Bitmap<bool>* colored=NULL) const;

private:
  Pixel  d_vectorcolor;
  bool   d_drawarrows;
  float d_scale;
};

#endif
