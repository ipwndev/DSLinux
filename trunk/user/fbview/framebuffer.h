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

#ifndef ds_framebuffer_h
#define ds_framebuffer_h

#include "types.h"

/* 4MB memery limit */
#define MEMSIZE (1024*1024*4)

/* byte per pixel for framebuffer device */
#define FB_BPP (sizeof(fb_pixel)*8)

/** Type of data represents pixels in framebuffer device. */
typedef Uint8 fb_pixel;

/** Pixmap of screen. Every change of this variable will be imediatly seen by user */
extern fb_pixel * screen;

/** Pixel size in bytes. Usually it is 1 or 2 */
extern Uint8 pixelsize;

/** Memory size of one row. This is fb_xres * pixelsize */
extern Uint32 fb_line_size;

/** Resolution of screen, in X axis */
extern Uint16 fb_xres;

/** Resolution of screen, in Y axis */
extern Uint16 fb_yres;

/**
 * Initializes the framebuffer.
 * @return 0 if success, 1 if failure
 */
extern int fb_init();

/**
 * Uninitializes the framebuffer
 * @return always 0
 */
extern int fb_uninit();

/**
 * Set black colour to all pixels of the screen.
 * @param screen pointer to pixmap with width and height same as screen.
 * Usually is it variable screen.
 */
extern void fb_clear_screen(fb_pixel * screen);

/**
 * Paints a part of image on the screen. Id image is smaller than screen, function paints entire image.
 * @param fb_pixel pixmap of image to paint on the screen
 * @param width width od image
 * @param height height of image
 * @param pos_x position of the image to paint. Pixel pointed by pos_x will be drawn in the top, left corner.
 * @param pos_y position of the image to paint. Pixel pointed by pos_y will be drawn in the top, left corner.
 */
extern void fb_pixmap_render(fb_pixel * fb_pixmap, Uint16 width , Uint16 height, Uint16 pos_x, Uint16 pos_y);

/**
 * Sets colour of pixel on the pixmap.
 * @param dest memory adress of pixel
 * @param R red value [0; 255];
 * @param G green value [0; 255];
 * @param B blue value [0; 255];
 */
extern inline void fb_set_pixel(fb_pixel * dest, Uint8 R, Uint8 G, Uint8 B);

/**
 * Reads colour of pixel and saves it to RGB variables.
 * @param dest memory adress of pixel
 * @param R red value [0; 255];
 * @param G green value [0; 255];
 * @param B blue value [0; 255];
 */
extern inline void fb_get_pixel(fb_pixel * dest, Uint8 * R, Uint8 * G, Uint8 * B);

#endif
