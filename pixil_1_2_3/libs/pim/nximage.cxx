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


/* Derived from FLTK  fltk/src/Fl_Image.cxx */
/* Original copright:  Copyright 1998-1999 by Bill Spitzak and others. */

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <nximage.h>

void
NxImage::draw(int XP, int YP, int WP, int HP, int cx, int cy)
{
    // account for current clip region (faster on Irix):
    int X, Y, W, H;
    fl_clip_box(XP, YP, WP, HP, X, Y, W, H);
    cx += X - XP;
    cy += Y - YP;
    // clip the box down to the size of image, quit if empty:
    if (cx < 0) {
	W += cx;
	X -= cx;
	cx = 0;
    }
    if (cx + W > w)
	W = w - cx;
    if (W <= 0)
	return;
    if (cy < 0) {
	H += cy;
	Y -= cy;
	cy = 0;
    }
    if (cy + H > h)
	H = h - cy;
    if (H <= 0)
	return;

    if (!id) {
	id = (ulong) fl_create_offscreen(w, h);
	fl_begin_offscreen((Fl_Offscreen) id);
	fl_draw_image(array, 0, 0, w, h, d, ld);
	fl_end_offscreen();
    }
    fl_copy_offscreen(X, Y, W, H, (Fl_Offscreen) id, cx, cy);
    if (_volatile_data) {
	fl_delete_offscreen((Fl_Offscreen) id);
	id = 0;
    }				// end of if
}

NxImage::~NxImage()
{
    if (id)
	fl_delete_offscreen((Fl_Offscreen) id);
}

static void
image_labeltype(const Fl_Label * o, int x, int y, int w, int h, Fl_Align a)
{
    NxImage *b = (NxImage *) (o->value);
    int cx;

    if (a & FL_ALIGN_LEFT)
	cx = 0;
    else if (a & FL_ALIGN_RIGHT)
	cx = b->w - w;
    else
	cx = (b->w - w) / 2;
    int cy;
    if (a & FL_ALIGN_TOP)
	cy = 0;
    else if (a & FL_ALIGN_BOTTOM)
	cy = b->h - h;
    else
	cy = (b->h - h) / 2;
    b->draw(x, y, w, h, cx, cy);
}

static void
image_measure(const Fl_Label * o, int &w, int &h)
{
    NxImage *b = (NxImage *) (o->value);
    w = b->w;
    h = b->h;
}

void
NxImage::label(Fl_Widget * o)
{
    Fl::set_labeltype(_FL_IMAGE_LABEL, image_labeltype, image_measure);
    o->label(_FL_IMAGE_LABEL, (const char *) this);
}

void
NxImage::label(Fl_Menu_Item * o)
{
    Fl::set_labeltype(_FL_IMAGE_LABEL, image_labeltype, image_measure);
    o->label(_FL_IMAGE_LABEL, (const char *) this);
}
