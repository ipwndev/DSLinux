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

#ifndef ds_bmp_h
#define ds_bmp_h

#include "types.h"
#include "framebuffer.h"

#define BMP_SUPPORT

/**
 * load bmp file to fb_pixmap
 * @param filename name of bmp file on the disk to load
 * @param fb_pixmap pointer to the memory for pixmap
 * @param w width of loaded image
 * @param h height of loaded image
 * @return 0 if success, 1 if failure.
 */
extern int bmp2fb_pixmap(char * filename, fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h);

/**
 * save pixmap to bmp file 
 * @param filename name of bmp file on the disk to save
 * @param fb_pixmap pixmap of image to save
 * @param w width
 * @param h height
 * @return 0 if success, 1 if failure
 */
extern int fb_pixmap2bmp(char * filename, fb_pixel * fb_pixmap, Uint16 w, Uint16 h);

#endif
