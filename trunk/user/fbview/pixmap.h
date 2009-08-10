/*
 *  (C) Copyright 2008-2009 Kamil Kopec <kamil_kopec@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ds_pixmap_h
#define ds_pixmap_h

#include "types.h"
#include "framebuffer.h"

/**
 * Function is scalling an image.
 * uses Blinear scale algorithm
 * @param fb_pixmap pixmap of image to scale
 * @param w width of image
 * @param h height of image
 * @param scale scale
 */
extern void fb_pixmap_scale(fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h, float scale);

/**
 * Function is rotating an image.
 * Only simples rotation are implemented (90 * x)
 * @param fb_pixmap pixmap of image to scale
 * @param w width of image
 * @param h height of image
 * @param rotate rotation (allowed only 0, 90, 180, 270)
 */
extern void fb_pixmap_rotate(fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h, Uint16 rotate);

#endif

