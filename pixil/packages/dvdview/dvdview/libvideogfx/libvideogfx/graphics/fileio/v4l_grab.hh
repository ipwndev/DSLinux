/*********************************************************************
  v4l_grab.hh

  purpose:
    Interface to Video4Linux-grabbing interfaces.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   11/Jul/2000 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILEIO_V4L_GRAB_HH
#define LIBVIDEOGFX_GRAPHICS_FILEIO_V4L_GRAB_HH

#include "libvideogfx/graphics/basic/image.hh"


class V4L_Grabber
{
public:
   V4L_Grabber();
  ~V4L_Grabber();

  // initialization

  void SetDevice(const char* device);
  void SetAlignment(const ImageInfo_Alignment& align) { d_align=align; }
  void SetBorder(int border) { d_border=border; }
  void SetResolution(int w,int h);
  void AskResolutin(int& w,int& h) { w = d_width; h = d_height; }

  void StartGrabbing(bool greyscale);

  void Grab(Image_YUV<Pixel>&);

private:
  // bool d_initialized;

  const char* d_device;
  int  d_fd;

  int d_width,d_height;
  bool d_greyscale;
  ImageInfo_Alignment d_align;
  int d_border;

  struct GrabData* d_grabdata;

  int d_nextbuf;
};

#endif
