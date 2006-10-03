//
// "$Id$"
//
// Color functions for the Fast Light Tool Kit (FLTK).
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

// Implementation of fl_color(i), fl_color(r,g,b).

#ifdef WIN32
#include "fl_color_win32.cxx"
#else

// Also code to look at the X visual and figure out the best way to turn
// a color into a pixel value.

// SGI compiler seems to have problems with unsigned char arguments
// being used to index arrays.  So I always copy them to an integer
// before use.

#include "Fl_XColor.H"
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

////////////////////////////////////////////////////////////////
// figure_out_visual() calculates masks & shifts for generating
// pixels in true-color visuals:

uchar fl_redmask, fl_greenmask, fl_bluemask;
int fl_redshift, fl_greenshift, fl_blueshift, fl_extrashift;
static uchar beenhere;

static void
figure_out_visual()
{
    beenhere = 1;
#ifndef NANO_X			//tanghao
    if (!fl_visual->red_mask || !fl_visual->green_mask
	|| !fl_visual->blue_mask) {
#if USE_COLORMAP
	fl_redmask = 0;
	return;
#else
	Fl::fatal("Requires true color visual");
#endif
    }
#endif //tanghao

    // get the bit masks into a more useful form:
    int i, j;
#ifndef NANO_X			//tanghao
    int m;
    for (i = 0, m = 1; m; i++, m <<= 1)
	if (fl_visual->red_mask & m)
	    break;
    for (j = i; m; j++, m <<= 1)
	if (!(fl_visual->red_mask & m))
	    break;
#else

    j = 64;
    i = 8;

#endif //tanghao
    fl_redshift = j - 8;
    fl_redmask = (j - i >= 8) ? 0xFF : 0xFF - (255 >> (j - i));

#ifndef NANO_X			//tanghao

    for (i = 0, m = 1; m; i++, m <<= 1)
	if (fl_visual->green_mask & m)
	    break;
    for (j = i; m; j++, m <<= 1)
	if (!(fl_visual->green_mask & m))
	    break;
#endif //tanghao

    fl_greenshift = j - 8;
    fl_greenmask = (j - i >= 8) ? 0xFF : 0xFF - (255 >> (j - i));

#ifndef NANO_X			//tanghao
    for (i = 0, m = 1; m; i++, m <<= 1)
	if (fl_visual->blue_mask & m)
	    break;
    for (j = i; m; j++, m <<= 1)
	if (!(fl_visual->blue_mask & m))
	    break;
#endif //tanghao

    fl_blueshift = j - 8;
    fl_bluemask = (j - i >= 8) ? 0xFF : 0xFF - (255 >> (j - i));

    i = fl_redshift;
    if (fl_greenshift < i)
	i = fl_greenshift;
    if (fl_blueshift < i)
	i = fl_blueshift;
    if (i < 0) {
	fl_extrashift = -i;
	fl_redshift -= i;
	fl_greenshift -= i;
	fl_blueshift -= i;
    } else
	fl_extrashift = 0;

}

////////////////////////////////////////////////////////////////
// Get an rgb color.  This is easy for a truecolor visual.  For
// colormapped it picks the closest color out of the fltk colormap
// but be warned that this results in *two* approximations: one
// to the fltk colormap, and another to whatever colors X allocates.

ulong
fl_xpixel(uchar r, uchar g, uchar b)
{
    if (!beenhere)
	figure_out_visual();
#if USE_COLORMAP
    if (!fl_redmask) {
	Fl_Color i;
	if (r == g && r == b) {	// get it out of gray ramp
	    i = fl_gray_ramp(r * FL_NUM_GRAY / 256);
	} else {		// get it out of color cube:
	    i = fl_color_cube(r * FL_NUM_RED / 256, g * FL_NUM_GREEN / 256,
			      b * FL_NUM_BLUE / 256);
	}
	return fl_xpixel(i);
    }
#endif
#ifdef NANO_X
    return GR_RGB(r, g, b);
#else
    return
	(((r & fl_redmask) << fl_redshift) +
	 ((g & fl_greenmask) << fl_greenshift) +
	 ((b & fl_bluemask) << fl_blueshift)
	) >> fl_extrashift;
#endif
}

void
fl_color(uchar r, uchar g, uchar b)
{
#ifdef NANO_X			//tanghao
    GrSetGCForeground(fl_gc, fl_xpixel(r, g, b));	//(GR_GC_ID gc, GR_COLOR foreground);
    GrSetGCBackground(fl_gc, GR_RGB(200, 200, 200));
#else
    XSetForeground(fl_display, fl_gc, fl_xpixel(r, g, b));
#endif //tanghao
}

////////////////////////////////////////////////////////////////
// Get a color out of the the fltk colormap.  Again for truecolor
// visuals this is easy.  For colormap this actually tries to allocate
// an X color, and does a least-squares match to find the closest
// color if X cannot allocate that color.

static unsigned fl_cmap[256] = {
#include "fl_cmap.h"		// this is a file produced by "cmap.C":
};

#if HAVE_OVERLAY
Fl_XColor fl_xmap[2][256];
uchar fl_overlay;
Colormap fl_overlay_colormap;
XVisualInfo *fl_overlay_visual;
ulong fl_transparent_pixel;
#else
Fl_XColor fl_xmap[1][256];
#endif

// calculate what color is actually on the screen for a mask:
static inline uchar
realcolor(uchar color, uchar mask)
{
#if 1
    // accurate version if the display has linear gamma, but fl_draw_image
    // works better with the simpler version on most screens...
    uchar m = mask;
    uchar result = color & m;
    for (;;) {
	while (m & mask) {
	    m >>= 1;
	    color >>= 1;
	}
	if (!m)
	    break;
	mask = m;
	result |= color & m;
    }
    return result;
#else
    return (color & mask) | (~mask) & (mask >> 1);
#endif
}

ulong
fl_xpixel(Fl_Color i)
{

#if HAVE_OVERLAY
    Fl_XColor & xmap = fl_xmap[fl_overlay][i];
#else
    Fl_XColor & xmap = fl_xmap[0][i];
#endif
    if (xmap.mapped)
	return xmap.pixel;

    if (!beenhere)
	figure_out_visual();
    uchar r, g, b;
    {
	unsigned c = fl_cmap[i];
	r = uchar(c >> 24);
	g = uchar(c >> 16);
	b = uchar(c >> 8);
    }

#if USE_COLORMAP
    Colormap colormap;
#if HAVE_OVERLAY
    if (fl_overlay) {
	colormap = fl_overlay_colormap;
	goto J1;
    }
#endif
    if (!fl_redmask) {
	colormap = fl_colormap;
#if HAVE_OVERLAY
      J1:
	static XColor *ac[2];
	XColor *&allcolors = ac[fl_overlay];
	static int nc[2];
	int &numcolors = nc[fl_overlay];
#else

#ifdef NANO_X
	//static GR_COLOR *allcolors;
	static int numcolors;
#else
	static XColor *allcolors;
	static int numcolors;
#endif
#endif

	// I don't try to allocate colors with XAllocColor once it fails
	// with any color.  It is possible that it will work, since a color
	// may have been freed, but some servers are extremely slow and this
	// avoids one round trip:
	if (!numcolors) {	// don't try after a failure
#ifdef NANO_X
	    GR_COLOR xcol;
	    xcol = GR_RGB(r, g, b);
	    xmap.mapped = 1;
	    xmap.r = r >> 8;
	    xmap.g = g >> 8;
	    xmap.b = b >> 8;
	    return xcol;
#else
	    XColor xcol;
	    xcol.red = r << 8;
	    xcol.green = g << 8;
	    xcol.blue = b << 8;
	    if (XAllocColor(fl_display, colormap, &xcol)) {
		xmap.mapped = 1;
		xmap.r = xcol.red >> 8;
		xmap.g = xcol.green >> 8;
		xmap.b = xcol.blue >> 8;
		return xmap.pixel = xcol.pixel;
#endif
	    }
#ifndef NANO_X
	    // I only read the colormap once.  Again this is due to the slowness
	    // of round-trips to the X server, even though other programs may alter
	    // the colormap after this and make decisions here wrong.
#if HAVE_OVERLAY
	    if (fl_overlay)
		numcolors = fl_overlay_visual->colormap_size;
	    else
#endif

		numcolors = fl_visual->colormap_size;
	    if (!allcolors)
		allcolors = new XColor[numcolors];
	    for (int p = numcolors; p--;)
		allcolors[p].pixel = p;
	    XQueryColors(fl_display, colormap, allcolors, numcolors);
	}
	// find least-squares match:
	int mindist = 0x7FFFFFFF;
	unsigned int bestmatch = 0;
	for (unsigned int n = numcolors; n--;) {
#if HAVE_OVERLAY
	    if (fl_overlay && n == fl_transparent_pixel)
		continue;
#endif
	    XColor & a = allcolors[n];
	    int d, t;
	    t = int (r) - int (a.red >> 8);
	    d = t * t;
	    t = int (g) - int (a.green >> 8);
	    d += t * t;
	    t = int (b) - int (a.blue >> 8);
	    d += t * t;
	    if (d <= mindist) {
		bestmatch = n;
		mindist = d;
	    }
	}
	XColor & p = allcolors[bestmatch];

	// It appears to "work" to not call this XAllocColor, which will
	// avoid another round-trip to the server.  But then X does not
	// know that this program "owns" this value, and can (and will)
	// change it when the program that did allocate it exits:
	if (XAllocColor(fl_display, colormap, &p)) {
	    xmap.mapped = 1;
	    xmap.pixel = p.pixel;
	} else {
	    // However, if that XAllocColor fails, I have to give up and
	    // assumme the pixel is ok for the duration of the program.  This
	    // is due to bugs (?) in the Solaris X and some X terminals
	    // where XAllocColor *always* fails when the colormap is full,
	    // even if we ask for a color already in it...
	    xmap.mapped = 2;	// 2 prevents XFreeColor from being called
	    xmap.pixel = bestmatch;
	}
	xmap.r = p.red >> 8;
	xmap.g = p.green >> 8;
	xmap.b = p.blue >> 8;
	return xmap.pixel;
#endif //tanghao

    }
#endif
    // return color for a truecolor visual:
    xmap.mapped = 2;		// 2 prevents XFreeColor from being called
    xmap.r = realcolor(r, fl_redmask);
    xmap.g = realcolor(g, fl_greenmask);
    xmap.b = realcolor(b, fl_bluemask);
    return xmap.pixel = fl_xpixel(r, g, b);

}

Fl_Color fl_color_;
#include <stdio.h>
void
fl_color(Fl_Color i)
{
    fl_color_ = i;
#ifdef NANO_X
    //GR_COLOR c = fl_xpixel(i);
    GrSetGCForeground(fl_gc, fl_xpixel(i));	//(GR_GC_ID gc, GR_COLOR foreground);
    GrSetGCBackground(fl_gc, GR_RGB(255, 255, 255));
#else
    XSetForeground(fl_display, fl_gc, fl_xpixel(i));
#endif //tanghao
}

void
Fl::free_color(Fl_Color i, int overlay)
{
#if HAVE_OVERLAY
#else
    if (overlay)
	return;
#endif
    if (fl_xmap[overlay][i].mapped) {
#if USE_COLORMAP
#if HAVE_OVERLAY
	Colormap colormap = overlay ? fl_overlay_colormap : fl_colormap;
#else
#ifndef NANO_X
	Colormap colormap = fl_colormap;
#endif
#endif
#ifndef NANO_X
	if (fl_xmap[overlay][i].mapped == 1)
	    XFreeColors(fl_display, colormap, &(fl_xmap[overlay][i].pixel), 1,
			0);
#endif //tanghao
#endif
	fl_xmap[overlay][i].mapped = 0;
    }
}

void
Fl::set_color(Fl_Color i, unsigned c)
{
    if (fl_cmap[i] != c) {
	free_color(i, 0);
#if HAVE_OVERLAY
	free_color(i, 1);
#endif
	fl_cmap[i] = c;
    }
}

#endif // end of X-specific code

unsigned
Fl::get_color(Fl_Color i)
{
    return fl_cmap[i];
}

void
Fl::set_color(Fl_Color i, uchar red, uchar green, uchar blue)
{
    Fl::set_color(i,
		  ((unsigned) red << 24) + ((unsigned) green << 16) +
		  ((unsigned) blue << 8));
}

void
Fl::get_color(Fl_Color i, uchar & red, uchar & green, uchar & blue)
{
    unsigned c = fl_cmap[i];
    red = uchar(c >> 24);
    green = uchar(c >> 16);
    blue = uchar(c >> 8);
}

Fl_Color
fl_color_average(Fl_Color color1, Fl_Color color2, float weight)
{
    Fl_Color avg;
    unsigned rgb1 = fl_cmap[color1];
    unsigned rgb2 = fl_cmap[color2];
    uchar r, g, b;

    r = (uchar) (((uchar) (rgb1 >> 24)) * weight +
		 ((uchar) (rgb2 >> 24)) * (1 - weight));
    g = (uchar) (((uchar) (rgb1 >> 16)) * weight +
		 ((uchar) (rgb2 >> 16)) * (1 - weight));
    b = (uchar) (((uchar) (rgb1 >> 8)) * weight +
		 ((uchar) (rgb2 >> 8)) * (1 - weight));

    if (r == g && r == b) {	// get it out of gray ramp
	avg = fl_gray_ramp(r * FL_NUM_GRAY / 256);
    } else {			// get it out of color cube:
	avg =
	    fl_color_cube(r * FL_NUM_RED / 256, g * FL_NUM_GREEN / 256,
			  b * FL_NUM_BLUE / 256);
    }

    return avg;
}

Fl_Color
inactive(Fl_Color c)
{
    return fl_color_average(c, FL_GRAY, .33f);
}

Fl_Color
contrast(Fl_Color fg, Fl_Color bg)
{
    int c1 = int (fl_cmap[fg]);
    int c2 = int (fl_cmap[bg]);

    if ((c1 ^ c2) & 0x80800000)
	return fg;
    else if (c2 & 0x80800000)
	return FL_GRAY_RAMP;	// black from gray ramp
    else
	return (Fl_Color) (FL_COLOR_CUBE - 1);	// white from gray ramp
}

//
// End of "$Id$".
//
