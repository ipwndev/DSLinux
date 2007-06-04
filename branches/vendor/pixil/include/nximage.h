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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


/* Derived from fltk/FL/Fl_Image. */
/* Original copyright:  Copyright 1998-1999 by Bill Spitzak and others. */

#ifndef NXIMAGE_INCLUDED
#define NXIMAGE_INCLUDED

class Fl_Widget;
struct Fl_Menu_Item;

struct NxImage
{
    const uchar *array;
    int w, h, d, ld;
    ulong id;			// for internal use
    int _volatile_data;
    FL_EXPORT NxImage(const uchar * bits, int W, int H, int D = 3, int LD =
		      0):array(bits), w(W), h(H), d(D), ld(LD), id(0)
    {
	_volatile_data = 0;
    }
    FL_EXPORT ~ NxImage();
    FL_EXPORT void label(Fl_Widget *);
    FL_EXPORT void label(Fl_Menu_Item *);
    FL_EXPORT void draw(int X, int Y, int W, int H, int cx = 0, int cy = 0);
    FL_EXPORT void draw(int X, int Y)
    {
	draw(X, Y, w, h, 0, 0);
    }
    FL_EXPORT void setvdata(int flag)
    {
	_volatile_data = flag;
    }
};

#endif
