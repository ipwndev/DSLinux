//
// "$Id$"
//
// Image drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// I hope a simple and portable method of drawing color and monochrome
// images.  To keep this simple, only a single storage type is
// supported: 8 bit unsigned data, byte order RGB, and pixels are
// stored packed into rows with the origin at the top-left.  It is
// possible to alter the size of pixels with the "delta" argument, to
// add alpha or other information per pixel.  It is also possible to
// change the origin and direction of the image data by messing with
// the "delta" and "linedelta", making them negative, though this may
// defeat some of the shortcuts in translating the image for X.

#ifdef WIN32
#include "fl_draw_image_win32.cxx"
#else

// A list of assumptions made about the X display:

// bits_per_pixel must be one of 8, 16, 24, 32.

// scanline_pad must be a power of 2 and greater or equal to 8.

// PsuedoColor visuals must have 8 bits_per_pixel (although the depth
// may be less than 8).  This is the only limitation that affects any
// modern X displays, you can't use 12 or 16 bit colormaps.

// The mask bits in TrueColor visuals for each color are
// contiguous and have at least one bit of each color.  This
// is not checked for.

// For 24 and 32 bit visuals there must be at least 8 bits of each color.

////////////////////////////////////////////////////////////////

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include "Fl_XColor.H"
#include <string.h>

#ifndef NANO_X //tanghao
static XImage i;	// template used to pass info to X

static int bytes_per_pixel;
static int scanline_add;
static int scanline_mask;

static void (*converter)(const uchar *from, uchar *to, int w, int delta);
static void (*mono_converter)(const uchar *from, uchar *to, int w, int delta);

static int dir;		// direction-alternator
static int ri,gi,bi;	// saved error-diffusion value

#if USE_COLORMAP
////////////////////////////////////////////////////////////////
// 8-bit converter with error diffusion

// I make a 16x16x16 cube of the closest colors in the fltk colormap
// we could allocate to each of the colors in a 4-bit image.  This is
// then used to find the pixel values and actual colors for error diffusion.
static uchar cube[16*16*16];

// calculate sum-of-squares error between 4-bit index and pixel colors:
static int calc_error(int r, int g, int b, int i) {
  int t; int s;
  t = ((r<<4)+8)-fl_xmap[0][i].r; s = t*t;
  t = ((g<<4)+8)-fl_xmap[0][i].g; s += t*t;
  t = ((b<<4)+8)-fl_xmap[0][i].b; s += t*t;
  return s;
}

// replace the color stored at a location with a better one:
static void improve(uchar *p, int& e, int r, int g, int b, int i) {
  if (i < FL_GRAY_RAMP || i > 255) return;
  int e1 = calc_error(r,g,b,i);
  if (e1 < e) {*p = i; e = e1;}
}

static int filled_color_cube;
static void fill_color_cube() {
  filled_color_cube = 1;
  int i;
  // allocate all the colors in the fltk color cube and gray ramp:
  // allocate widely seperated values first so that the bad ones are
  // distributed evenly through the colormap:
  for (i=0;;) {
    fl_xpixel((Fl_Color)(i+FL_COLOR_CUBE));
    i = (i+109)%(FL_NUM_RED*FL_NUM_GREEN*FL_NUM_BLUE); if (!i) break;
  }
  for (i=0;;) {
    fl_xpixel((Fl_Color)(i+FL_GRAY_RAMP));
    i = (i+7)%FL_NUM_GRAY; if (!i) break;
  }
  // fill in the 16x16x16 cube:
  uchar *p = cube;
  for (int r = 0; r<16; r++) {
    for (int g = 0; g<16; g++) {
      for (int b = 0; b<16; b++, p++) {
	// initial try is value from color cube:
	Fl_Color i = fl_color_cube(r*FL_NUM_RED/16, g*FL_NUM_GREEN/16,
				   b*FL_NUM_BLUE/16);
	int e = calc_error(r,g,b,i);
	*p = uchar(i);
	// try neighbor pixels in the cube to see if they are better:
	improve(p,e,r,g,b,i+FL_NUM_RED*FL_NUM_GREEN);
	improve(p,e,r,g,b,i-FL_NUM_RED*FL_NUM_GREEN);
	improve(p,e,r,g,b,i+FL_NUM_GREEN);
	improve(p,e,r,g,b,i-FL_NUM_GREEN);
	improve(p,e,r,g,b,i+1);
	improve(p,e,r,g,b,i-1);
	// try the gray ramp:
	i = fl_gray_ramp(g*FL_NUM_GRAY/15);
	improve(p,e,r,g,b,i);
	improve(p,e,r,g,b,i+1);
	improve(p,e,r,g,b,i-1);
      }
    }
  }
}

static void color8_converter(const uchar *from, uchar *to, int w, int delta) {
  if (!filled_color_cube) fill_color_cube();
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    to = to+(w-1);
    d = -delta;
    td = -1;
  } else {
    dir = 1;
    d = delta;
    td = 1;
  }
  for (; w--; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[2]; if (b < 0) b = 0; else if (b>255) b = 255;
    Fl_XColor* x = fl_xmap[0] + cube[((r<<4)&0xf00)+(g&0xf0)+(b>>4)];
    r -= x->r;
    g -= x->g;
    b -= x->b;
    *to = uchar(x->pixel);
  }
  ri = r; gi = g; bi = b;
}

static void mono8_converter(const uchar *from, uchar *to, int w, int delta) {
  if (!filled_color_cube) fill_color_cube();
  int r=ri;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    to = to+(w-1);
    d = -delta;
    td = -1;
  } else {
    dir = 1;
    d = delta;
    td = 1;
  }
  for (; w--; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    Fl_XColor* x = fl_xmap[0] + cube[(r>>4)*0x111];
    r -= x->g;
    *to = uchar(x->pixel);
  }
  ri = r;
}

#endif

////////////////////////////////////////////////////////////////
// 16 bit TrueColor converters with error diffusion
// Cray computers have no 16-bit type, so we use character pointers
// (which may be slow)

#ifdef U16
#define OUTTYPE U16
#define OUTSIZE 1
#define OUTASSIGN(v) *t = v
#else
#define OUTTYPE uchar
#define OUTSIZE 2
#define OUTASSIGN(v) int tt=v; t[0] = uchar(tt>>8); t[1] = uchar(tt)
#endif

static void color16_converter(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (; w--; from += d, t += td) {
    r = (r&~fl_redmask)  +from[0]; if (r>255) r = 255;
    g = (g&~fl_greenmask)+from[1]; if (g>255) g = 255;
    b = (b&~fl_bluemask) +from[2]; if (b>255) b = 255;
    OUTASSIGN((
      ((r&fl_redmask)<<fl_redshift)+
      ((g&fl_greenmask)<<fl_greenshift)+
      ((b&fl_bluemask)<<fl_blueshift)
      ) >> fl_extrashift);
  }
  ri = r; gi = g; bi = b;
}

static void mono16_converter(const uchar *from,uchar *to,int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  uchar mask = fl_redmask & fl_greenmask & fl_bluemask;
  int r=ri;
  for (; w--; from += d, t += td) {
    r = (r&~mask) + *from; if (r > 255) r = 255;
    uchar m = r&mask;
    OUTASSIGN((
      (m<<fl_redshift)+
      (m<<fl_greenshift)+
      (m<<fl_blueshift)
      ) >> fl_extrashift);
  }
  ri = r;
}

// special-case the 5r6g5b layout used by XFree86:

static void c565_converter(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (; w--; from += d, t += td) {
    r = (r&7)+from[0]; if (r>255) r = 255;
    g = (g&3)+from[1]; if (g>255) g = 255;
    b = (b&7)+from[2]; if (b>255) b = 255;
    OUTASSIGN(((r&0xf8)<<8) + ((g&0xfc)<<3) + (b>>3));
  }
  ri = r; gi = g; bi = b;
}

static void m565_converter(const uchar *from,uchar *to,int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri;
  for (; w--; from += d, t += td) {
    r = (r&7) + *from; if (r > 255) r = 255;
    OUTASSIGN((r>>3) * 0x841);
  }
  ri = r;
}

////////////////////////////////////////////////////////////////
// 24bit TrueColor converters:

static void rgb_converter(const uchar *from, uchar *to, int w, int delta) {
  int d = delta-3;
  for (; w--; from += d) {
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
  }
}

static void bgr_converter(const uchar *from, uchar *to, int w, int delta) {
  for (; w--; from += delta) {
    uchar r = from[0];
    uchar g = from[1];
    *to++ = from[2];
    *to++ = g;
    *to++ = r;
  }
}

static void rrr_converter(const uchar *from, uchar *to, int w, int delta) {
  for (; w--; from += delta) {
    *to++ = *from;
    *to++ = *from;
    *to++ = *from;
  }
}

////////////////////////////////////////////////////////////////
// 32bit TrueColor converters on a 32 or 64-bit machine:

#ifdef U64
#define STORETYPE U64
#if WORDS_BIGENDIAN
#define INNARDS32(f) \
  U64 *t = (U64*)to; \
  int w1 = (w+1)/2; \
  for (; w1--; from += delta) {U64 i = f; from += delta; *t++ = (i<<32)|(f);}
#else
#define INNARDS32(f) \
  U64 *t = (U64*)to; \
  int w1 = (w+1)/2; \
  for (; w1--; from += delta) {U64 i=f; from+= delta; *t++ = ((U64)(f)<<32)|i;}
#endif
#else
#define STORETYPE U32
#define INNARDS32(f)   U32 *t = (U32*)to; for(; w--; from += delta) *t++ = f;
#endif

static void rgbx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((unsigned(from[0])<<24)+(from[1]<<16)+(from[2]<<8));
}

static void xbgr_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0])+(from[1]<<8)+(from[2]<<16));
}

static void xrgb_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0]<<16)+(from[1]<<8)+(from[2]));
}

static void bgrx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32((from[0]<<8)+(from[1]<<16)+(unsigned(from[2])<<24));
}

static void rrrx_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(unsigned(*from) * 0x1010100U);
}

static void xrrr_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(*from * 0x10101U);
}

static void
color32_converter(const uchar *from, uchar *to, int w, int delta) {
  INNARDS32(
    (from[0]<<fl_redshift)+(from[1]<<fl_greenshift)+(from[2]<<fl_blueshift));
}

static void
mono32_converter(const uchar *from,uchar *to,int w, int delta) {
  INNARDS32(
    (*from << fl_redshift)+(*from << fl_greenshift)+(*from << fl_blueshift));
}

////////////////////////////////////////////////////////////////

static void figure_out_visual() {
  static XPixmapFormatValues *pfvlist;
  static int FL_NUM_pfv;
  if (!pfvlist) pfvlist = XListPixmapFormats(fl_display,&FL_NUM_pfv);
  XPixmapFormatValues *pfv;
  for (pfv = pfvlist; pfv < pfvlist+FL_NUM_pfv; pfv++)
    if (pfv->depth == fl_visual->depth) break;
  i.format = ZPixmap;
  i.byte_order = ImageByteOrder(fl_display);
//i.bitmap_unit = 8;
//i.bitmap_bit_order = MSBFirst;
//i.bitmap_pad = 8;
  i.depth = fl_visual->depth;
  i.bits_per_pixel = pfv->bits_per_pixel;

  if (i.bits_per_pixel & 7) bytes_per_pixel = 0; // produce fatal error
  else bytes_per_pixel = i.bits_per_pixel/8;

  unsigned int n = pfv->scanline_pad/8;
  if (pfv->scanline_pad & 7 || (n&(n-1)))
    Fl::fatal("Can't do scanline_pad of %d",pfv->scanline_pad);
  if (n < sizeof(STORETYPE)) n = sizeof(STORETYPE);
  scanline_add = n-1;
  scanline_mask = -n;

#if USE_COLORMAP
  if (bytes_per_pixel == 1) {
    converter = color8_converter;
    mono_converter = mono8_converter;
    return;
  }
  if (!fl_visual->red_mask)
    Fl::fatal("Can't do %d bits_per_pixel colormap",i.bits_per_pixel);
#endif

  // otherwise it is a TrueColor visual:
  fl_xpixel(0,0,0); // setup fl_redmask, etc, in fl_color.C

  int rs = fl_redshift;
  int gs = fl_greenshift;
  int bs = fl_blueshift;

  switch (bytes_per_pixel) {

  case 2:
    // All 16-bit TrueColor visuals are supported on any machine with
    // 24 or more bits per integer.
#ifdef U16
    ::i.byte_order = WORDS_BIGENDIAN;
#else
    ::i.byte_order = 1;
#endif
    if (rs == 11 && gs == 6 && bs == 0 && fl_extrashift == 3) {
      converter = c565_converter;
      mono_converter = m565_converter;
    } else {
      converter = color16_converter;
      mono_converter = mono16_converter;
    }
    break;

  case 3:
    if (::i.byte_order) {rs = 16-rs; gs = 16-gs; bs = 16-bs;}
    if (rs == 0 && gs == 8 && bs == 16) {
      converter = rgb_converter;
      mono_converter = rrr_converter;
    } else if (rs == 16 && gs == 8 && bs == 0) {
      converter = bgr_converter;
      mono_converter = rrr_converter;
    } else {
      Fl::fatal("Can't do arbitrary 24bit color");
    }
    break;

  case 4:
    if ((::i.byte_order!=0) != WORDS_BIGENDIAN)
      {rs = 24-rs; gs = 24-gs; bs = 24-bs;}
    if (rs == 0 && gs == 8 && bs == 16) {
      converter = xbgr_converter;
      mono_converter = xrrr_converter;
    } else if (rs == 24 && gs == 16 && bs == 8) {
      converter = rgbx_converter;
      mono_converter = rrrx_converter;
    } else if (rs == 8 && gs == 16 && bs == 24) {
      converter = bgrx_converter;
      mono_converter = rrrx_converter;
    } else if (rs == 16 && gs == 8 && bs == 0) {
      converter = xrgb_converter;
      mono_converter = xrrr_converter;
    } else {
      ::i.byte_order = WORDS_BIGENDIAN;
      converter = color32_converter;
      mono_converter = mono32_converter;
    }
    break;

  default:
    Fl::fatal("Can't do %d bits_per_pixel",i.bits_per_pixel);
  }
}
#endif
#define MAXBUFFER 0x40000 // 256k

static void innards(const uchar *buf, int X, int Y, int W, int H,
		    int delta, int linedelta, int mono,
		    Fl_Draw_Image_Cb cb, void* userdata)
{
#ifndef NANO_X //tanghao
  if (!linedelta) linedelta = W*delta;

  int dx, dy, w, h;
  fl_clip_box(X,Y,W,H,dx,dy,w,h);
  if (w<=0 || h<=0) return;
  dx -= X;
  dy -= Y;

  if (!bytes_per_pixel) figure_out_visual();
  i.width = w;
  i.height = h;

  void (*conv)(const uchar *from, uchar *to, int w, int delta) = converter;
  if (mono) conv = mono_converter;

  // See if the data is already in the right format.  Unfortunately
  // some 32-bit x servers (XFree86) care about the unknown 8 bits
  // and they must be zero.  I can't confirm this for user-supplied
  // data, so the 32-bit shortcut is disabled...
  // This can set bytes_per_line negative if image is bottom-to-top
  // I tested it on Linux, but it may fail on other Xlib implementations:
  if (buf && (
#if 0	// set this to 1 to allow 32-bit shortcut
      delta == 4 &&
#if WORDS_BIGENDIAN
      conv == rgbx_converter
#else
      conv == xbgr_converter
#endif
      ||
#endif
      conv == rgb_converter && delta==3
      ) && !(linedelta&scanline_add)) {
    i.data = (char *)(buf+delta*dx+linedelta*dy);
    i.bytes_per_line = linedelta;

  } else {
    int linesize = ((w*bytes_per_pixel+scanline_add)&scanline_mask)/sizeof(STORETYPE);
    int blocking = h;
    static STORETYPE *buffer;	// our storage, always word aligned
    static long buffer_size;
    {int size = linesize*h;
    if (size > MAXBUFFER) {
      size = MAXBUFFER;
      blocking = MAXBUFFER/linesize;
    }
    if (size > buffer_size) {
      delete[] buffer;
      buffer_size = size;
      buffer = new STORETYPE[size];
    }}
    i.data = (char *)buffer;
    i.bytes_per_line = linesize*sizeof(STORETYPE);
    if (buf) {
      buf += delta*dx+linedelta*dy;
      for (int j=0; j<h; ) {
	STORETYPE *to = buffer;
	int k;
	for (k = 0; j<h && k<blocking; k++, j++) {
	  conv(buf, (uchar*)to, w, delta);
	  buf += linedelta;
	  to += linesize;
	}
#ifndef NANO_X //tanghao
	XPutImage(fl_display,fl_window,fl_gc, &i, 0, 0, X+dx, Y+dy+j-k, w, k);
#endif //tanghao
      }
    } else {
#ifdef __GNUC__
      STORETYPE linebuf[(W*delta+(sizeof(STORETYPE)-1))/sizeof(STORETYPE)];
#else
      STORETYPE* linebuf = new STORETYPE[(W*delta+(sizeof(STORETYPE)-1))/sizeof(STORETYPE)];
#endif
      for (int j=0; j<h; ) {
	STORETYPE *to = buffer;
	int k;
	for (k = 0; j<h && k<blocking; k++, j++) {
	  cb(userdata, dx, dy+j, w, (uchar*)linebuf);
	  conv((uchar*)linebuf, (uchar*)to, w, delta);
	  to += linesize;
	}
#ifndef NANO_X
	XPutImage(fl_display,fl_window,fl_gc, &i, 0, 0, X+dx, Y+dy+j-k, w, k);
#endif //tanghao
      }
#ifndef __GNUC__
      delete[] linebuf;
#endif
    }
  }
#else
  if (linedelta == 0) linedelta = W*delta;

  if (buf)
  {
    bool flip_h = (    delta < 0); if (flip_h)     delta = -    delta;
    bool flip_v = (linedelta < 0); if (flip_v) linedelta = -linedelta;
    if (delta >= 1)
    {
      GR_WINDOW_ID id = fl_window;
      GR_GC_ID     gc = GrNewGC();
      U32          prev_fg = (U32) -1;
      GR_POINT     plist[1000];
      int          pcnt = 0;

      for ( int y = 0; y < H; ++y )
      {
        const uchar * pixel = buf;
        buf += linedelta;
        for ( int x = 0; x < W; ++x )
        {
          U32 fg;

	  // calculate the foreground color
	  if (delta >= 3) // presume it's RGB...
            fg = GR_RGB(pixel[0],pixel[1],pixel[2]);
          else // presume it's greyscale
            fg = GR_RGB(pixel[0],pixel[0],pixel[0]);

	  // if the foreground color changed, dump any queued pixels remaining
	  // for the old color, then change the color
          if (fg != prev_fg) {
            if (pcnt) {
              GrPoints(id,gc,pcnt,plist);
              pcnt = 0;
            }
            GrSetGCForeground(gc,fg);
            prev_fg = fg;
          }

          // queue the next pixel value
          plist[pcnt].x = X+(flip_h?(W-x-1):x);
          plist[pcnt].y = Y+(flip_v?(H-y-1):y);
          pcnt++;

          // if we've reached the max. number of queued pixels, dump them
          if (pcnt == sizeof(plist)/sizeof(*plist)) {
            GrPoints(id,gc,pcnt,plist);
            pcnt = 0;
          }

          pixel += delta;
        }
      }

      // dump any remaining queued points
      if (pcnt)
        GrPoints(id,gc,pcnt,plist);

      GrDestroyGC(gc);
    }
    else
    {
      Fl::fatal("Can't do a delta value of %d",delta);
    }
  } 
  else
  {
    int linesize = ((linedelta>0) ? linedelta : -linedelta);
    int blocking = H;
    static uchar * buffer = 0;
    static long buffer_size = 0;
    {
      int size = linesize*H;
      if (size > MAXBUFFER) 
      {
        size = MAXBUFFER;
        blocking = MAXBUFFER/linesize;
      }
      if (size > buffer_size) 
      {
        delete[] buffer;
        buffer_size = size;
        buffer = new uchar[size];
      }
    }
    uchar * linebuf = new uchar[linesize];
    for (int j=0; j<H; ) 
    {
      uchar *to = buffer;
      int k;
      for (k = 0; j<H && k<blocking; k++, j++) 
      {
        cb(userdata, X, Y+j, W, linebuf);
        memcpy(to,linebuf,linesize);
        to += linesize;
      }
      innards(buffer,X,Y+j-k,W,blocking,delta,linedelta,0,0,0);
    }
    delete [] linebuf;
  }
#endif //tanghao
}

void fl_draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0);
}
void fl_draw_image(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data);
}
void fl_draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,1,0,0);
}
void fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,1,cb,data);
}

void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
#ifndef NANO_X //tanghao
  if (fl_visual->depth > 16) {
    fl_color(r,g,b);
    fl_rectf(x,y,w,h);
  } else {
    uchar c[3];
    c[0] = r; c[1] = g; c[2] = b;
    innards(c,x,y,w,h,0,0,0,0,0);
  }
#else
  // this may need speeded up sometime, as in the above code .. jsk
    fl_color(r,g,b);
    fl_rectf(x,y,w,h);

#endif //tanghao
}

#endif

//
// End of "$Id$".
//
