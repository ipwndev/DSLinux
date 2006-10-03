/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


// System header files

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>


// Local header files
#define	MWINCLUDECOLORS
#include <nano-X.h>

#include "scfxn.h"


// Typedef, macros, enum/struct/union defintions


// Global variables


// Local variables


// Static function prototypes


//------------------------------------------------------------------------------
//
//      Static function definitions
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
//      Extern function definitions
//
//------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void darken_pixel()
**	Desc:		Darkens the cooresponding pixel value
**	Accepts:	rgbTriplet *pix = Ptr to the pixel values
**				double adj = value adjustment multiplier
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
darken_pixel(rgbTriplet * pix, double adj)
{
    pix->red = (int) (((double) pix->red) * adj);
    pix->green = (int) (((double) pix->green) * adj);
    pix->blue = (int) (((double) pix->blue) * adj);
    return;
}				// end of darken_pixel(rgbTriplet *, double)


/*******************************************************************************\
**
**	Function:	void nxsunimage()
**	Desc:		Generates the sunimage using native Microwindow's calls.
**	Accepts:	unsigned char *srcpix = Source image
**				unsigned char *dstpix = Destination image
**				int w = Width of the image
**				int h = height of the image
**				int d = Depth of the image
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxsunimage(unsigned char *srcpix, unsigned char *dstpix, int w, int h, int d)
{
    double cd, *daywave, fnoon, f1, f2, f3, gmttm, jultm, quot, sd, sdec, shft, sra, srv, slong;	// Sun longitude
    int i,			// Loop iterator
      j,			// 2nd loop iterator
      north,			// North value
      south,			// South value
      tr1;			// ????
    rgbTriplet pix;		// Current pixel
    time_t now = time(NULL);	// Current time
    struct tm *utctm;		// UTC time structure

    // Allocate memory for the day/night wave line
    daywave = (double *) malloc(w * sizeof(double));

    utctm = gmtime(&now);
    jultm = jtime(utctm);

    sunpos(jultm, 0, &sra, &sdec, &srv, &slong);

    gmttm = gmst(jultm);
    slong = fixangle(180.0 + (sra - (gmttm * 15)));
    fnoon = slong * (w / 360.0);
    slong -= 180;

    if ((quot = dtr(sdec)) > 0)
	south = 0;
    else
	south = -1;

    north = -1 - south;

    cd = cos(quot);
    sd = sin(quot);
    quot = (2.0 * M_PI) / (double) w;
    shft = 0.5 * (h + 1);

    f1 = M_PI / ((double) h);
    f2 = ((double) h) / M_PI;
    f3 = 1E-8 * f2;

    for (i = 0; i < w; i++) {
	double val = (double) i;
	daywave[i] = cos(quot * (val - fnoon));
    }				// end of for

    // Start manipulating the image
    for (i = 0; i < w; i++) {
	if (fabs(sd) > f3)
	    tr1 = (int) (shft + f2 * atan(daywave[i] * (cd / sd)));
	else
	    tr1 = 0;

	if (tr1 < 0)
	    tr1 = 0;

	// Figure where to darken....
	if (south == -1) {
	    // Draw below the line
	    for (j = 0; j <= tr1; j++) {
		read_pixel(srcpix, i, j, d, w, &pix);
		darken_pixel(&pix, 0.5);
		write_pixel(dstpix, i, j, d, w, &pix);
	    }			// end of for
	}			// end of if
	else {
	    // Draw above the line
	    for (j = tr1; j < h; j++) {
		read_pixel(srcpix, i, j, d, w, &pix);
		darken_pixel(&pix, 0.5);
		write_pixel(dstpix, i, j, d, w, &pix);
	    }			// end of for
	}			// end of else
    }				// end of for

    free(daywave);

    return;
}				// end of nxsunimage(unsigned char *, unsigned char *, int, int, int)

/*******************************************************************************\
**
**	Function:	void read_pixel()
**	Desc:		Reads the current pixel (determined by i,j) from the array
**				and stores it into the rgbTriplet
**	Accepts:	unsigned char *src = Image array source
**				int x = x location of current pixel
**				int y = y location of current pixel
**				int d = depth (img depth)
**				int xsize = img width
**				rgbTriplet *pix = Store the data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
read_pixel(unsigned char *src, int x, int y, int d, int xsize,
	   rgbTriplet * pix)
{
    unsigned short wv;		// word pixel value (for 16 bpp images)
    unsigned long lv;		// long pixel value (for 24/32 bpp images)

    switch (d) {
    case 2:
	// 16 bpp images
	wv = *((unsigned short *) src + (y * (xsize * 2)) + (x * 2));
	pix->red = (wv & 0xF800) >> 11;	// Red pixel component
	pix->green = (wv & 0x07E0) >> 5;	// Blue pixel component
	pix->blue = (wv & 0x001F);	// Green pixel component
	break;
    case 3:
	// 24 bpp images
	lv = *(((unsigned long *) src) + (y * xsize) + x);
	pix->red = (lv & 0xFF000000) >> 24;	// Red pixel component
	pix->green = (lv & 0xFF0000) >> 16;	// Blue pixel component
	pix->blue = (lv & 0xFF00) >> 8;	// Green pixel component
	break;
    case 4:
	// 32 bpp images
	lv = *((((unsigned long *) src) + (y * xsize) + x));
	pix->red = (lv & 0xFF0000) >> 16;	// Red pixel component
	pix->green = (lv & 0xFF00) >> 8;	// Blue pixel component
	pix->blue = (lv & 0xFF);	// Green pixel component
	break;
    }				// end of switch

    return;
}				// end of read_pixel(unsigned char *, int, int, int, int, rgbTriplet *)

/*******************************************************************************\
**
**	Function:	void write_pixel()
**	Desc:		Writes the current pixel value to the destination image array
**	Accepts:	unsigned char *dst = Destination image array
**				int x = x pixel coordinate
**				int y = y pixel coordiante
**				int d = Depth value
**				int xsize = image width
**				rgbTriplet *pix = Ptr to the pixel structure
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
write_pixel(unsigned char *dst, int x, int y, int d, int xsize,
	    rgbTriplet * pix)
{
    unsigned short wv;		// Word pixel value
    unsigned long lv;		// Long pixel value

    unsigned long offset = 0;

    switch (d) {
    case 2:
	// 16 bpp image
	wv = RGB2PIXEL565(pix->red, pix->green, pix->blue);
	offset = (y * (xsize * 2)) + (x * 2);
	*((unsigned short *) (dst + offset)) = wv;
	break;
    case 3:
	// 24 bpp image
	lv = RGB2PIXEL888(pix->red, pix->green, pix->blue);
	offset = (y * (xsize * 3)) + (x * 3);
	*((unsigned long *) (dst + offset)) = lv;
	break;
    case 4:
	// 32 bpp image
	lv = (pix->red << 16) | (pix->green << 8) | pix->blue;
	offset = (y * (xsize * 4)) + (x * 4);
	*((unsigned long *) (dst + offset)) = lv;
	break;
    }				// end of switch

    return;
}				// end of write_pixel(unsigned char *, int, int, int, int, rgbTriplet *)
