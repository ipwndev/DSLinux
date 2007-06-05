/*********************************************************************
  fileio/writeyuv.hh

  purpose:
    Write an YUV image to a ostream as follows: First the complete
    Y plane is written. Then the U and V components are written
    either one after the other or interleaved.
    If you're saving a greyscale only image you can choose between
    saving dummy color information or omitting the U,V data.

    The alpha mask that may be available in the image will be saved
    into a separate ostream if specified.

  notes:
    - You may save entire YUV sequences into a single file by simply
      calling WriteImage() several times.

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   15/Jul/99 - Dirk Farin - complete rewrite, interleaved output,
                            greyscale output
   25/May/99 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILEIO_WRITEYUV_HH
#define LIBVIDEOGFX_GRAPHICS_FILEIO_WRITEYUV_HH

#include <fstream.h>

#include "libvideogfx/graphics/basic/image.hh"


class FileWriter_YUV1
{
public:
   FileWriter_YUV1();
  ~FileWriter_YUV1() { }

  void SetYUVStream(ostream& str)   { d_yuvstr   = &str; }
  void SetAlphaStream(ostream& str) { d_alphastr = &str; }

  void SetWriteGreyscaleAsColor(bool flag=true) { d_write_greyscale_as_color=flag; }
  void WriteInterleaved(bool flag=true)         { d_write_interleaved=flag; }

  void WriteImage(const Image_YUV<Pixel>&);

private:
  ostream* d_yuvstr;
  ostream* d_alphastr;

  bool d_write_greyscale_as_color;
  bool d_write_interleaved;
};

#endif
