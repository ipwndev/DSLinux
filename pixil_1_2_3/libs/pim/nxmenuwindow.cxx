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

// This is the window type used by Fl_Menu to make the pop-ups.
// It draws in the overlay planes if possible.

// Also here is the implementation of the mouse & keyboard grab,
// which are used so that clicks outside the program's windows
// can be used to dismiss the menus.

#include <pixil_config.h>

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <nxmenuwindow.h>

// WIN32 note: HAVE_OVERLAY is false
#if HAVE_OVERLAY
extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static GC gc;			// the GC used by all X windows
extern uchar fl_overlay;	// changes how fl_color(x) works
#endif

#include <stdio.h>

void
NxMenuWindow::show()
{

#if HAVE_OVERLAY
    if (!shown() && overlay() && fl_find_overlay_visual()) {
	XInstallColormap(fl_display, fl_overlay_colormap);
	fl_background_pixel = int (fl_transparent_pixel);
	Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
	fl_background_pixel = -1;
    } else
#endif
#ifdef CONFIG_NANOX
	Fl_X::mw_parent = 0;
#endif
    Fl_Single_Window::show();
}

void
NxMenuWindow::flush()
{
#if HAVE_OVERLAY
    if (!fl_overlay_visual || !overlay()) {
	Fl_Single_Window::flush();
	return;
    }
    Fl_X *i = Fl_X::i(this);
    fl_window = i->xid;
    if (!gc)
	gc = XCreateGC(fl_display, i->xid, 0, 0);
    fl_gc = gc;
    fl_overlay = 1;
    fl_clip_region(i->region);
    i->region = 0;
    draw();
    fl_overlay = 0;
#else
    Fl_Single_Window::flush();
#endif
}

void
NxMenuWindow::erase()
{
#if HAVE_OVERLAY
    if (!gc || !shown())
	return;
//XSetForeground(fl_display, gc, 0);
//XFillRectangle(fl_display, fl_xid(this), gc, 0, 0, w(), h());
    XClearWindow(fl_display, fl_xid(this));
#endif
}

// Fix the colormap flashing on Maximum Impact Graphics by erasing the
// menu before unmapping it:
void
NxMenuWindow::hide()
{
    erase();
    Fl_Single_Window::hide();
}

NxMenuWindow::~NxMenuWindow()
{
    hide();
}
