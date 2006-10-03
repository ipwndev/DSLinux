/*********************************************************************
  draw/draw_x11.hh

  purpose:
    Drawing routines that need X11 but that do not create any
    GUI windows in that sense. X11 is needed for example to get
    access to the fonts that are installed on the system.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    21/Jul/1999 - Dirk Farin - cleanup
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_DRAW_DRAW_X11_HH
#define LIBVIDEOGFX_GRAPHICS_DRAW_DRAW_X11_HH

#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/graphics/basic/image.hh"


/* Draw text into the bitmap. */
enum HTextAlign   { HAlign_Left,HAlign_Center,HAlign_Right  };
enum VTextAlign   { VAlign_Top ,VAlign_Center,VAlign_Bottom };
enum TextDrawMode { TextDrawMode_Transparent,TextDrawMode_Opaque };

void WriteText(Bitmap<Pixel>&,const char* txt,int x,int y,const char* x11fontname="9x15",
	       HTextAlign halign=HAlign_Left,VTextAlign valign=VAlign_Top,
	       TextDrawMode mode = TextDrawMode_Transparent,Pixel front=255,Pixel back=0);

#endif
