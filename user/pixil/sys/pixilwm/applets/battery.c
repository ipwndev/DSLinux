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

/* Battery monitor for those apps with APM support */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <nano-X.h>
#include <nxcolors.h>

#include <pixlib/pixlib.h>

#include "nanowm.h"
#include "applets.h"

#ifdef STATIC_LINK
#define applet_init battery_applet_init
#define applet_close battery_applet_close
#endif

static GR_WINDOW_ID wid;
static int g_w = 0, g_h = 0;
static int applet_id;

#define APPICON "battery.gif"

struct battery_t
{
  unsigned short percent;
  unsigned short ttl;
  unsigned char status;
};

static struct battery_t global_battery;
static GR_WINDOW_ID ttl_wid = 0;
static int ttl_shown = 0;
static GR_IMAGE_ID batimage;

#ifdef NOTUSED
static void show_ttlwindow(void) {
  char buf[64];
  int tw, th, tb;
  GR_GC_ID gc;

  static GR_FONT_ID fontid = 0;
  
  if (!ttl_wid) {
    GR_WINDOW_INFO wi;
    GrGetWindowInfo(wid, &wi);

    ttl_wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID, 
			    wi.x, wi.y - 10, 45, 15, wm_getColor(WM_DIALOG));
  }

  GrSelectEvents(ttl_wid, GR_EVENT_MASK_EXPOSURE);
  GrMapWindow(ttl_wid);

  while(1) {
    GR_EVENT event;
    GrGetNextEvent(&event);

    if (event.type == GR_EVENT_TYPE_EXPOSURE && event.exposure.wid == ttl_wid) 
      break;
  }
  
  if (!fontid)
    fontid = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);

  if (global_battery.status || global_battery.ttl == ((unsigned short)-1)) 
    sprintf(buf, "AC Attached");
  else 
    sprintf(buf, "%d:%02d left", global_battery.ttl / 60, 
	    global_battery.ttl % 60);

  gc = GrNewGC();
  GrSetGCFont(gc, fontid);
  
  GrGetGCTextSize(gc, buf, -1, GR_TFTOP, &tw, &th, &tb);

  GrResizeWindow(ttl_wid, tw + 10, 15);

  GrSetGCForeground(gc, GR_COLOR_BLACK);
  GrRect(ttl_wid, gc, 0,0, tw + 10, 15);

  GrSetGCForeground(gc, wm_getColor(WM_ICONTEXT));
  GrSetGCUseBackground(gc, GR_FALSE);

  GrText(ttl_wid, gc, 5, 1, buf, -1, GR_TFTOP);
  GrDestroyGC(gc);

  ttl_shown = 2;
}
#endif
  
  static void draw_battery(GR_WINDOW_ID wid) {
  GR_GC_ID gc = GrNewGC();

  GR_COLOR color;
  int perc = global_battery.percent;
  int height = 0;

  GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));
  GrFillRect(wid, gc, 0, 0, 7, 14);

  GrSetGCBackground(gc, wm_getColor(WM_TASKBAR));
  GrDrawImageToFit(wid, gc, 0, 0, -1, -1, batimage);

  /* If we are charging, the percentage is always 100% */
  if (global_battery.status) {
    perc = 100;
    color = GR_COLOR_WHITE;
  }
  else {
    if (perc > 100) perc = 100;
    if (perc < 0) perc = 0;
    
    if (perc > 50) color = GR_COLOR_WHITE;
    else if (perc <= 50 && perc > 10) color = GR_COLOR_YELLOW;
    else color = GR_COLOR_RED;
  }

  height = (11 * perc) / 100;
  GrSetGCForeground(gc, color);

  GrFillRect(wid, gc, 1, 12 - height, 5, height);
  GrDestroyGC(gc);
}

static int 
get_battery_info(struct battery_t *bptr) {

  bptr->percent = pix_pwr_getbat(PWR_BAT_PERCENT);
  bptr->ttl = pix_pwr_getbat(PWR_BAT_SECONDS);

  /* Returns 1 if charging, 0 if not, or -1 if unknown */
  bptr->status = pix_pwr_isCharging();
  return 0;
}

static void event_callback(GR_WINDOW_ID window, GR_EVENT *event) {

  switch(event->type) {
#ifdef NOTUSED
  case GR_EVENT_TYPE_BUTTON_DOWN:
    show_ttlwindow();
    break;
#endif
  case GR_EVENT_TYPE_EXPOSURE:
    draw_battery(window);
    break;
  }
}

static void timeout_callback(void) {

  int pper, psta;

  /* If the TTL window is showing, then hide it */

  if (ttl_shown) {
    ttl_shown--;
    
    if (!ttl_shown)
      GrUnmapWindow(ttl_wid);
  }

  /* Only redraw the battery if there is a change */

  pper = global_battery.percent;
  psta = global_battery.status;

  get_battery_info(&global_battery);

  if (global_battery.percent != pper || global_battery.status != psta)
    draw_battery(wid);
}

int applet_init(int id, int *x, int y, int h) {

  GR_IMAGE_INFO iinfo;

  int tid;
  applet_id = id;

  /* Get the battery info to start things off */
  get_battery_info(&global_battery);

  /* Load the image */
  batimage = loadIconImage(APPICON, 0, 0);
  if (!batimage) return -1;

  GrGetImageInfo(batimage, &iinfo);
  
  /* Create the main window */

  wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0, GR_ROOT_WINDOW_ID,
		      *x, y, iinfo.width, h, wm_getColor(WM_TASKBAR));

  if (!wid) return -1;

  /* Register the applet */
  wm_applet_register(id, wid, 
		     GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP | 
		     GR_EVENT_MASK_EXPOSURE, event_callback);
  
  /* Register the timer */

  tid = wm_applet_add_timer(id, APPLET_TIMER_PERIODIC, 1000, timeout_callback);

  GrMapWindow(wid);

  /* Update the coordinates */
  g_w = iinfo.width;
  g_h = h;
  
  *x += iinfo.width;

  return 0;
}

int applet_close(void) {
  wm_applet_del_timer(applet_id, 0);
  return 0;
}
  


