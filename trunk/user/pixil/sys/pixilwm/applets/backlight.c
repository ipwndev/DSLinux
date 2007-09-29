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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nano-X.h>

#include "nanowm.h"
#include "nxdraw.h"
#include "powerman.h"
#include "applets.h"

#define BLOFFICON "bloff.gif"
#define BLONICON  "blon.gif"

#ifdef STATIC_LINK
#define applet_init backlight_applet_init
#define applet_close backlight_applet_close
#endif

static GR_IMAGE_ID idON, idOFF;
static GR_WINDOW_ID wid;
static int g_w = 0, g_h = 0;
static int applet_id;

static void
draw_backlight(GR_WINDOW_ID id) {
  GR_GC_ID gc = GrNewGC();  
  GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));
  GrFillRect(id, gc, 0, 0, 10, 16); 
  GrDrawImageToFit(id, gc, 0, 0, -1, -1, pm_get_bl_state() ? idON : idOFF);  
  GrDestroyGC(gc);
}

static void
event_callback(GR_WINDOW_ID window, GR_EVENT *event) {
  switch(event->type) {
  case GR_EVENT_TYPE_BUTTON_DOWN:
    pm_bl_toggle();  /* Toggle the backlight status */
    break;
  }
  
  draw_backlight(window);
}

/* This is called when there is a change to the status of the backlight */

static void backlight_callback(int type, void *data) {
  draw_backlight(wid);
}

int applet_init(int id, int *x, int y, int h) {

  GR_IMAGE_INFO iinfo;
  int iw;
  applet_id = id;
  
  /* Create the local images */
  
  idON = loadIconImage(BLONICON, 0, 0);
  idOFF = loadIconImage(BLOFFICON, 0, 0);

  if (!idON || !idOFF) return -1;

  GrGetImageInfo(idON, &iinfo);
  iw = iinfo.width;

  GrGetImageInfo(idOFF, &iinfo);
  iw = (iinfo.width > iw) ? iinfo.width : iw;
  
  /* Make a new window */

  wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0, GR_ROOT_WINDOW_ID,
		      *x, y, iw, h, wm_getColor(WM_TASKBAR));
  
  if (!wid) return -1;

  /* Register for some events */

  wm_applet_register(id, wid, 
		     GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP | 
		     GR_EVENT_MASK_EXPOSURE, event_callback);

  /* Register for a power managment status change */
  pm_register_callback(PM_CALLBACK_BL, backlight_callback);

  /* Map the window */
  GrMapWindow(wid);

  /* And update the coordinates */

  g_w = iw;  
  g_h = h;
  
  *x += iw;

  return 0;
}

int applet_close(void) {
  pm_unregister_callback(PM_CALLBACK_BL, backlight_callback);
  return 0;
}
  
