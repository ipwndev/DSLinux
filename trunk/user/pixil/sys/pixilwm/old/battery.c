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
#include "apps.h"
#include "config.h"

#define APPICON "battery.gif"

/* This is how many times a second the timer refreshes */
#define TIMERRATE 2
#define TTL_TIMEOUT 2

#define TTL_WIN 1
#define BAT_WIN 2

struct battery_t
{
  unsigned short percent;
  unsigned short ttl;
  unsigned char status;
};

static struct battery_t global_battery;

/* Some local globals */

static GR_WINDOW_ID ttl_wid = 0;
static unsigned char ttl_timer = 0;
static GR_IMAGE_ID batimage = 0;
static unsigned long timer = 0;
static int apm_fd;

static void
ttl_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    int c;
    int swidth;
    GR_FONT_INFO info;
    char buf[20];
    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID fontid = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);

    /* If we are charging, indicate that fact */
    
    if (global_battery.status) 
      sprintf(buf, "Charging");
    else 
      sprintf(buf, "%d:%02d left", global_battery.ttl / 60, global_battery.ttl % 60);
    
    swidth = 0;

    GrGetFontInfo(fontid, &info);

    for (c = 0; c < strlen(buf); c++) {
	swidth += info.widths[(unsigned char) buf[c]];
    }

    GrResizeWindow(ttl_wid, swidth + 4, 15);

    GrSetGCForeground(gc, wm_getColor(WM_ICONTEXT));
    GrSetGCUseBackground(gc, GR_FALSE);
    GrSetGCFont(gc, fontid);
    GrText(ep->wid, gc, 1, 1, buf, -1, GR_TFTOP);

    GrDestroyFont(fontid);
    GrDestroyGC(gc);
}

static void
battery_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    GR_GC_ID batgc = GrNewGC();

    int perc = global_battery.percent;
    int height = 0;

    GrSetGCForeground(batgc, wm_getColor(WM_TASKBAR));
    GrFillRect(ep->wid, batgc, 0, 0, 7, 14);

    GrSetGCBackground(batgc, wm_getColor(WM_TASKBAR));

    /* Draw the battery */
    GrDrawImageToFit(ep->wid, batgc, 0, 0, -1, -1, batimage);

    /* If we are charging, the percentage is always 100% */
    if (global_battery.status) {
      perc = 100;
      color = GR_COLOR_GREEN;
    }
    else {
      if (perc > 100) perc = 100;
      if (perc < 0) perc = 0;
      
      if (perc > 50) color = GR_COLOR_WHITE;
      else if (perc <= 50 && perc > 10) color = GR_COLOR_YELLOW;
      else color = GR_COLOR_RED;
    }

    height = (11 * perc) / 100;
    GrSetGCForeground(batgc, color);

    GrFillRect(ep->wid, batgc, 1, 12 - height, 5, height);

    GrDestroyGC(batgc);
}

int 
get_battery_info(struct battery_t *bptr) {

  bptr->percent = pix_pwr_getbat(PWR_BAT_PERCENT);
  bptr->ttl = pix_pwr_getbat(PWR_BAT_SECONDS);

  /* Returns 1 if charging, 0 if not, or -1 if unknown */
  bptr->status = pix_pwr_isCharging();
  return 0;
}

static void
redraw_window(win * window, int type)
{
    GR_EVENT_EXPOSURE expose;
    GR_WINDOW_INFO info;

    /* dummy full area expose event */
    GrGetWindowInfo(window->wid, &info);
    expose.type = GR_EVENT_TYPE_EXPOSURE;
    expose.wid = window->wid;
    expose.x = 0;
    expose.y = 0;
    expose.width = info.width;
    expose.height = info.height;

    if (type == TTL_WIN)
	ttl_exposure(window, &expose);
    else
	battery_exposure(window, &expose);
}

static void
battery_wndproc(win * window, GR_EVENT * ep)
{
    GR_EVENT_MOUSE *em;

    if (ep->type == GR_EVENT_TYPE_TIMER)
	printf("go timer event\n");

    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:

	if (ep->exposure.wid == ttl_wid && ttl_timer)
	    ttl_exposure(window, &ep->exposure);
	else
	    battery_exposure(window, &ep->exposure);

	break;

    case GR_EVENT_TYPE_BUTTON_DOWN:

	if (ttl_timer) {
	    ttl_timer = 0;
	    GrUnmapWindow(ttl_wid);
	} else {
	    ttl_timer = 1;

	    get_battery_info(&global_battery);
	    em = (GR_EVENT_MOUSE *) ep;
	    GrMoveWindow(ttl_wid, em->rootx, em->rooty - 15);
	    GrRaiseWindow(ttl_wid);
	    GrMapWindow(ttl_wid);
	}
	break;

    case GR_EVENT_TYPE_TIMEOUT:

	if (ttl_timer) {
	    ttl_timer++;
	    /* Hide the ttl timer after a short timeout */
	    if (ttl_timer == (TTL_TIMEOUT * TIMERRATE)) {
		ttl_timer = 0;
		GrUnmapWindow(ttl_wid);
	    }
	}

	if (timer++ % (TIMERRATE * 30)) {
	    unsigned short pperc = global_battery.percent;

	    if (!get_battery_info(&global_battery))
		break;

	    /* Only redraw if the value has changed */
	    if (pperc != global_battery.percent)
		redraw_window(window, BAT_WIN);
	}

	break;
    }
}

void
battery_create(void)
{
    int ypos;
    GR_IMAGE_INFO iinfo;

    win *window, *ttlwindow;

    GR_WINDOW_ID wid;
    GR_SCREEN_INFO si;
    GR_WM_PROPERTIES props;

    /* Open up a link to the apm data file */
    apm_fd = open("/proc/apm", O_RDONLY);

    if (apm_fd == -1) {
	//fprintf(stderr, "Couldn't open /proc/apm\n");
	//return;
    }

    if (!get_battery_info(&global_battery))
	return;

    batimage = loadIconImage(APPICON, 0, 0);
    if (!batimage)
	return;

    GrGetScreenInfo(&si);
    GrGetImageInfo(batimage, &iinfo);

    ypos = si.ws_height + (((20 - iinfo.height) / 2) + 1);

    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE,
			NULL, GR_ROOT_WINDOW_ID,
			18, ypos, 7, 14, wm_getColor(WM_TASKBAR));

    window = add_window(wid, GR_ROOT_WINDOW_ID, 0, battery_wndproc);

    ttl_wid =
	GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID, 22,
		      si.ws_height - 17, 45, 15, wm_getColor(WM_DIALOG));

    props.flags = GR_WM_FLAGS_BORDERSIZE;
    props.bordersize = 1;
    GrSetWMProperties(ttl_wid, &props);

    ttlwindow = add_window(ttl_wid, GR_ROOT_WINDOW_ID, 0, battery_wndproc);

    GrSelectEvents(wid, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_TIMEOUT |
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);

    GrSelectEvents(ttl_wid, GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);

    timeout_setproc(battery_wndproc, window);
}
