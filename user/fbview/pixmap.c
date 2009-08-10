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

#include "pixmap.h"
#include "keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Function is scalling an image.
 * uses Blinear scale algorithm
 * @param fb_pixmap pixmap of image to scale
 * @param w width of image
 * @param h height of image
 * @param scale scale (MUST BE != 0.00)
 */
void fb_pixmap_scale(fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h, float scale)
{
	Uint16 i, j;
	Uint16 nw, nh;
	float x, y, f12, f34;
	Uint16 x1, y1, x2, y2;

	Uint8 p1R, p1G, p1B, p2R, p2G, p2B;
	Uint8 p3R, p3G, p3B, p4R, p4G, p4B;
	Uint8 new_R, new_G, new_B;

	fb_pixel * fb_pixmap_new;

	nw = (*w) * scale;
	nh = (*h) * scale;

	fb_pixmap_new = (fb_pixel *)malloc(nw * nh * pixelsize);
	if (!fb_pixmap_new) {
		printf ("Error while allocating memory for scaling.\n");
		return;
	}

	for (j = 0; j < nh; j++) {

		y = (float)j / scale;
		y1 = floor(y);
		y2 = y1+1 >= (*h) ? y1 : y1+1;

		for (i = 0; i < nw; i++) {

			x = (float)i / scale;
			x1 = floor(x);
			x2 = x1+1 >= (*w) ? x1 : x1+1;

			fb_get_pixel(*fb_pixmap + (x1 + y1*(*w))*pixelsize, &p1R, &p1G, &p1B);
			fb_get_pixel(*fb_pixmap + (x2 + y1*(*w))*pixelsize, &p2R, &p2G, &p2B);
			fb_get_pixel(*fb_pixmap + (x1 + y2*(*w))*pixelsize, &p3R, &p3G, &p3B);
			fb_get_pixel(*fb_pixmap + (x2 + y2*(*w))*pixelsize, &p4R, &p4G, &p4B);

			f12 = (float)p1R + (x - x1) * (p2R - p1R);
			f34 = (float)p3R + (x - x1) * (p4R - p3R);
			new_R = (Uint8)(f12 + (y - y1) * (f34 - f12));

			f12 = (float)p1G + (x - x1) * (p2G - p1G);
			f34 = (float)p3G + (x - x1) * (p4G - p3G);
			new_G = (Uint8)(f12 + (y - y1) * (f34 - f12));

			f12 = (float)p1B + (x - x1) * (p2B - p1B);
			f34 = (float)p3B + (x - x1) * (p4B - p3B);
			new_B = (Uint8)(f12 + (y - y1) * (f34 - f12));

			fb_set_pixel(fb_pixmap_new + (i + j*nw)*pixelsize, new_R, new_G, new_B);
		}
	}

	free(*fb_pixmap);
	*fb_pixmap = fb_pixmap_new;
	*w = nw;
	*h = nh;
}

/**
 * Function is rotating an image.
 * Only simples rotation are implemented (90 * x)
 * @param fb_pixmap pixmap of image to scale
 * @param w width of image
 * @param h height of image
 * @param rotate rotation (allowed only 0, 90, 180, 270)
 */
void fb_pixmap_rotate(fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h, Uint16 rotate)
{
	Uint16 nw, nh, i, j;
	fb_pixel * fb_pixmap_new;
	Uint8 R, G, B;

	if (rotate == 0) return;

	fb_pixmap_new = (fb_pixel *)malloc((*w) * (*h) * pixelsize);
	if (!fb_pixmap_new) {
		printf ("Error while allocating memory for rotation.\n");
		return;
	}

	switch (rotate) {
	case 90:
		nw = (*h);
		nh = (*w);

		for (j = 0; j < nw; j++)
		for (i = 0; i < nh; i++) {
			fb_get_pixel ((*fb_pixmap)+((i+j*nh)*pixelsize), &R, &G, &B);
			fb_set_pixel ((fb_pixmap_new)+((nw-j+i*nw)*pixelsize), R, G, B);
		}

		break;

	case 180:
		nw = (*w);
		nh = (*h);

		for (j = 0; j < nh; j++)
		for (i = 0; i < nw; i++) {
			fb_get_pixel ((*fb_pixmap)+((i+j*nw)*pixelsize), &R, &G, &B);
			fb_set_pixel ((fb_pixmap_new)+((nw-i+(nh-j)*nw)*pixelsize), R, G, B);
		}

		break;

	case 270:
		nw = (*h);
		nh = (*w);

		for (j = 0; j < nw; j++)
		for (i = 0; i < nh; i++) {
			fb_get_pixel ((*fb_pixmap)+((i+j*nh)*pixelsize), &R, &G, &B);
			fb_set_pixel ((fb_pixmap_new)+((j+(nh-i)*nw)*pixelsize), R, G, B);
		}

		break;
	}

	free(*fb_pixmap);
	*fb_pixmap = fb_pixmap_new;
	*w = nw;
	*h = nh;
}
