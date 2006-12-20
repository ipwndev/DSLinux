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


#include <par/par.h>
#include <pixlib/pixlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nano-X.h>
#include "nxdraw.h"

#include "apps.h"
#include "nanowm.h"
#include "config.h"
#include "powerman.h"

static GR_IMAGE_ID idON;
static GR_IMAGE_ID idOFF;
static GR_BOOL blON = 1;
static GR_WINDOW_ID blWid;

static void backlight_wndproc(win * window, GR_EVENT * ep);

#define BLOFFICON "bloff.gif"
#define BLONICON  "blon.gif"

void
backlight_create(void)
{
    int ih, iw, ypos;

    GR_WINDOW_ID wid;
    GR_SCREEN_INFO si;
    GR_IMAGE_INFO iinfo;

    /* Load the images first so I can center the window on the screen */
    idON = loadIconImage(BLONICON, 0, 0);
    idOFF = loadIconImage(BLOFFICON, 0, 0);

    if (!idON || !idOFF)
	return;

    GrGetImageInfo(idON, &iinfo);
    ih = iinfo.height;
    iw = iinfo.width;


    GrGetImageInfo(idOFF, &iinfo);
    if (iinfo.height > ih)
	ih = iinfo.height;
    if (iinfo.width > iw)
	iw = iinfo.width;

    GrGetScreenInfo(&si);

    ypos = si.ws_height + (((20 - ih) / 2) + 1);

    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
			2, ypos, iw, ih, wm_getColor(WM_TASKBAR));

    add_window(wid, GR_ROOT_WINDOW_ID, 0, backlight_wndproc);

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);
    blWid = wid;
}

static void
backlight_draw(GR_WINDOW_ID id) {
  GR_GC_ID gc = GrNewGC();
  
  GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));
  GrFillRect(id, gc, 0, 0, 10, 16);
  
  /* draw image in actual size */
  GrDrawImageToFit(id, gc, 0, 0, -1, -1, blON ? idON : idOFF);
  
  GrDestroyGC(gc);
}

static void
backlight_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
  backlight_draw(ep->wid);
}

static void
backlight_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
    GR_EVENT_EXPOSURE expose;
    GR_WINDOW_INFO info;

    blON = !blON;

    /* dummy full area expose event */
    GrGetWindowInfo(ep->wid, &info);
    expose.type = GR_EVENT_TYPE_EXPOSURE;
    expose.wid = ep->wid;
    expose.x = 0;
    expose.y = 0;
    expose.width = info.width;
    expose.height = info.height;
    backlight_exposure(window, &expose);

    pm_backlight(blON ? BL_ON : BL_OFF);
}

static void
backlight_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	backlight_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	backlight_buttondown(window, &ep->button);
	break;
    }
}

void
pm_set_bl_state(int state) {
  blON = (state == BL_ON) ? 1 : 0;
  backlight_draw(blWid);
}

int
pm_get_bl_state(void) {
  return (blON ? BL_ON :BL_OFF);
}

