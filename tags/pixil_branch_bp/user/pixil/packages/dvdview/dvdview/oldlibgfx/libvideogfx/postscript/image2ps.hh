/*********************************************************************
  postscript/image2ps.hh

  purpose:
    Print image as postscript.

  notes:

  to do:
   - a unified filter approach could be useful. This would allow
     to build a filter chain like "RunLength"->"Ascii85".

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    28/Jul/99 - Dirk Farin - RGB color image output
    26/Jul/99 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_POSTSCRIPT_IMAGE2PS_HH
#define LIBVIDEOGFX_POSTSCRIPT_IMAGE2PS_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "libvideogfx/postscript/layout.hh"
#include <iostream.h>


class Image2Postscript
{
public:
  enum PSImageEncoding { Hex, Ascii85 };


   Image2Postscript() : d_ostr(NULL), d_enc(Ascii85) { }
  ~Image2Postscript() { }

  void SetOutput(ostream& ostr) { d_ostr=&ostr; }

  void SetPrintingArea(const PrintingArea& a) { d_area=a; }
  void SetEncodingMethod(PSImageEncoding enc) { d_enc=enc; }
  void DrawImage(const Image_YUV<Pixel>&);
  void DrawImage(const Image_RGB<Pixel>&);

private:
  void WriteLine_ASCIIHex(const Pixel*,int len);
  void WriteLine_ASCII85 (const Pixel*,int len);

  PrintingArea    d_area;
  PSImageEncoding d_enc;
  ostream*        d_ostr;
};

#endif
