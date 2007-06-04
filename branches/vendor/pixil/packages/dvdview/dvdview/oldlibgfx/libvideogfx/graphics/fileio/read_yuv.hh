/*********************************************************************
  readyuv.hh

  purpose:
    Read in YUV-Files. The format of the files is as follows:
    First the complete Y-plane is continuously saved. The
    following U- and V-planes are either saved continuously
    one after the other (U comes first) or interleaved.
    That is the bytes are taken alternatingly from the U- and
    V-planes. The chrominance channels may be horizontally or
    vertically (or both) subsampled by a factor of two.

    Additionally a separate alpha-mask file can be read in. 
    This mask always has full resolution.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   15/Jul/99 - Dirk Farin - complete rework, greyscale input
   25/May/99 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILEIO_READYUV_HH
#define LIBVIDEOGFX_GRAPHICS_FILEIO_READYUV_HH

#include <fstream.h>

#include "libvideogfx/graphics/basic/image.hh"


class FileReader_YUV1
{
public:
   FileReader_YUV1();
  ~FileReader_YUV1() { }

  // initialization

  void SetYUVStream  (istream& yuvstream)   { d_yuvstr = &yuvstream; d_initialized=false; }
  void SetAlphaStream(istream& alphastream) { d_alphastr = &alphastream; d_initialized=false; }

  void SetImageSpec(const ImageSpec_YUV& spec) { d_spec = spec; d_initialized=false; }
  void SetInterleavedUV(bool flag=true)    { d_interleavedUV = flag; d_initialized=false; }
  void SetInputIsGreyscale(bool flag=true) { d_greyscale_input = flag; d_initialized=false; }

  // usage

  int  AskNFrames() const;
  bool IsEOF() const;

  void SkipToImage(int nr);
  void ReadImage(Image_YUV<Pixel>&);

private:
  istream* d_yuvstr;
  istream* d_alphastr;

  ImageSpec_YUV d_spec;

  bool  d_interleavedUV;
  bool  d_greyscale_input;

  int   d_nFrames;
  int   d_Framesize;
  int   d_nextFrame;

  void Init();
  bool d_initialized;
};

#endif
