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

#include <pixil_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pixlib/pixlib.h>

#include "nano-X.h"
#include "apps.h"
#include "nanowm.h"
#include "config.h"
#include "categories.h"

#ifdef CONFIG_PIXILWM_MENUS
#include "sys_menu.h"
#endif

/* External functions called from within */


static GR_SCREEN_INFO si;

static void root_wndproc(win * window, GR_EVENT * ep);
static void root_prepare_background(void);
static void root_redraw();

#ifdef CONFIG_PIXILWM_MENUS

static void
root_reset(void *foo)
{
    /* wm_load_config_settings(); */
    root_prepare_background();
    root_redraw();
}

static wm_menu_t *root_sys_menu = 0;
static wm_menu_t *apps_sys_menu = 0;

void close_nxscrtop(int);

static void
shutdown_cb(void *foo)
{
    close_nxscrtop(0);
}

#endif

void
root_create(void)
{
    GR_SCREEN_INFO si;
    GR_WM_PROPERTIES props;

#ifdef CONFIG_PIXILWM_MENUS
    int i = 0;
#endif

    /* add root window */
    add_window(GR_ROOT_WINDOW_ID, GR_ROOT_WINDOW_ID, 0, root_wndproc);

    /*
     * update and child update events must be set to hook
     * top level window mappings
     */

    GrSelectEvents(GR_ROOT_WINDOW_ID, GR_EVENT_MASK_CHLD_UPDATE |
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN |
		   GR_EVENT_MASK_FDINPUT |
		   GR_EVENT_MASK_SCREENSAVER | GR_EVENT_MASK_TIMER | 
		   GR_EVENT_MASK_KEY_DOWN | GR_EVENT_MASK_KEY_UP);

    /* Platform specfic stuff */
    /* Ipaq: Grab the "power button" */

#ifdef CONFIG_PLATFORM_IPAQ
    GrGrabKey(GR_ROOT_WINDOW_ID, MWKEY_SUSPEND, GR_GRAB_HOTKEY_EXCLUSIVE);
#endif

    /* Zaurus:  Grab the Cancel button */
#ifdef CONFIG_PLATFORM_ZAURUS
    GrGrabKey(GR_ROOT_WINDOW_ID, MWKEY_ESCAPE, GR_GRAB_HOTKEY_EXCLUSIVE);
#endif

    /* Cache the background for speed */

    root_prepare_background();
    GrGetScreenInfo(&si);

    /* Step 1 - Get inital variables for the workspace width and height */
    setWorkspace(si.ws_width, si.ws_height);

    /* Step 2 - Make the input button */
    createInputButton();

    /* Step 3 - create the home button which aleways exists */
    home_create();

#ifdef NOTUSED
    date_create();

#ifdef CONFIG_PIXILWM_PM
    backlight_create();
#endif

    /* JHC - quick hack - we only bring up the battery if 
       APM is available
     */

#ifdef CONFIG_PIXILWM_PM
    if (!access("/proc/apm", R_OK))
	battery_create();
#endif
#endif

    /* Step 4 - Create the category browser */
    category_setCurrent(0);

#ifdef VIRTUAL_WINDOWS
    create_categoryBrowser();
#endif

    /* Step 5 - Set up the root window and get ready to draw! */
    props.flags = GR_WM_FLAGS_BACKGROUND;
    props.background = wm_getColor(WM_BGCOLOR);
    GrSetWMProperties(GR_ROOT_WINDOW_ID, &props);

    root_redraw();

#ifdef CONFIG_PIXILWM_MENUS

    /* Step 6 - Create the system menu (if applicable) */

    root_sys_menu = create_system_menu(6);

    if (create_apps_menu(&apps_sys_menu)) {
	ADD_SYS_MENU_CMENU(&root_sys_menu[0], "Quick Run...", apps_sys_menu);
	ADD_SYS_MENU_SEP(&root_sys_menu[1]);
	i = 2;
    } else {
	i = 0;
    }

    ADD_SYS_MENU_REGULAR(&root_sys_menu[i++], "Reset ScreenTop", root_reset,
			 0, 0);
    ADD_SYS_MENU_REGULAR(&root_sys_menu[i++], "About...", 0, 0, 0);
    ADD_SYS_MENU_REGULAR(&root_sys_menu[i++], "Shutdown", shutdown_cb, 0, 0);
    ADD_SYS_MENU_END(&root_sys_menu[i++]);
#endif

    /* Step 7 - There is no step 7 :) */

}

/* The following are special caching     */
/* routines to make the background image */
/* rendering run faster                  */

static GR_IMAGE_ID lastBGImage = 0;
static GR_WINDOW_ID lastBGPixmap = 0;
static int lastBGWidth = 0;
static int lastBGHeight = 0;

static void
root_prepare_background(void)
{
    GR_GC_ID gc = GrNewGC();
    GR_IMAGE_INFO iinfo;
    GR_IMAGE_ID iid;
    int style = 0;
    int wpactive = wm_getWallpaper(&iid, &style);
    int xpos, ypos, wi, hi;
    if (!si.rows)
	GrGetScreenInfo(&si);

    if (!wpactive || lastBGImage == iid)
	return;

    GrGetImageInfo(iid, &iinfo);

    /* Remove the last cached BG */

    if (lastBGPixmap)
	GrDestroyWindow(lastBGPixmap);

    /* Figure out the size of the cached pixmap */

    if (style == WALLPAPER_TILED || style == WALLPAPER_FULLSCREEN) {
	lastBGPixmap = GrNewPixmap(si.ws_width, si.ws_height, NULL);
	lastBGWidth = si.ws_width;
	lastBGHeight = si.ws_height;
    } else {			/* CENTERED */

	lastBGPixmap = GrNewPixmap(iinfo.width, iinfo.height, NULL);
	lastBGWidth = iinfo.width;
	lastBGHeight = iinfo.height;
    }

    GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
    GrFillRect(lastBGPixmap, gc, 0, 0, lastBGWidth, lastBGHeight);

    lastBGImage = iid;


    /* Now draw into the pixmap.  If it is fullscreen or centered,
       thats trival.  Tiled is a little more bad */

    switch (style) {
    case WALLPAPER_TILED:

	for (ypos = 0; ypos < si.ws_height; ypos += iinfo.width)
	    for (xpos = 0; xpos < si.ws_width; xpos += iinfo.height) {
		if (si.ws_width - xpos < iinfo.width)
		    wi = si.ws_width - xpos;
		else
		    wi = iinfo.width;

		if (si.ws_height - ypos < iinfo.height)
		    hi = si.ws_height - ypos;
		else
		    hi = iinfo.height;

		GrDrawImageToFit(lastBGPixmap, gc, xpos, ypos, wi, hi, iid);
	    }

	break;

    case WALLPAPER_FULLSCREEN:
    case WALLPAPER_CENTERED:
    default:
	GrDrawImageToFit(lastBGPixmap, gc, 0, 0, -1, -1, iid);
	break;
    }

    GrDestroyGC(gc);
}

static void
root_draw_background(GR_WINDOW_ID wid, GR_REGION_ID r)
{
    GR_GC_ID gc = GrNewGC();
    GR_IMAGE_ID iid;
    int style = 0;

    int wpactive = wm_getWallpaper(&iid, &style);

    /* Set the clip region */
    GrSetGCRegion(gc, r);

    if (!si.rows)
	GrGetScreenInfo(&si);

    /* First, draw the background (if it is visible) */

    if (!wpactive || style == WALLPAPER_BOTTOM || style == WALLPAPER_CENTERED
	|| !lastBGPixmap) {
	GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
	GrFillRect(wid, gc, 0, 0, si.ws_width, si.ws_height);
    }

    /* Now draw the wall paper */
    if (wpactive && lastBGPixmap) {
	int xpos, ypos, wi, hi;

	switch (style) {
	case WALLPAPER_TILED:
	case WALLPAPER_FULLSCREEN:
	    GrCopyArea(wid, gc, 0, 0, si.ws_width, si.ws_height,
		       lastBGPixmap, 0, 0, MWROP_SRCCOPY);

	    break;

	case WALLPAPER_BOTTOM:
	    xpos = (si.ws_width - lastBGWidth) / 2;
	    ypos = (si.ws_height - lastBGHeight);
	    wi = lastBGWidth;
	    hi = lastBGHeight;

	    GrCopyArea(wid, gc, xpos, ypos, wi, hi,
		       lastBGPixmap, 0, 0, MWROP_SRCCOPY);
	    break;

	case WALLPAPER_CENTERED:
	default:
	    xpos = (si.ws_width - lastBGWidth) / 2;
	    ypos = (si.ws_height - lastBGHeight) / 2;
	    wi = lastBGWidth;
	    hi = lastBGHeight;

	    GrCopyArea(wid, gc, xpos, ypos, wi, hi,
		       lastBGPixmap, 0, 0, MWROP_SRCCOPY);
	    break;
	}
    }

    GrDestroyGC(gc);
}

static void
root_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    GR_GC_ID gc = GrNewGC();
    GR_REGION_ID r;
    GR_RECT rc;


    if (!si.rows)
	GrGetScreenInfo(&si);

    /* 
     * Create clip region based on exposed rectangle.
     * This keeps flicker way down.
     */
    r = GrNewRegion();
    rc.x = ep->x;
    rc.y = ep->y;
    rc.width = ep->width;
    rc.height = ep->height;
    GrUnionRectWithRegion(r, &rc);
    GrSetGCRegion(gc, r);

    /* draw taskbar */
    GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));

    GrFillRect(ep->wid, gc, 0, si.ws_height, si.ws_width,
	       si.vs_height - si.ws_height);

    /* draw background (but only the rectangle) */
    root_draw_background(ep->wid, r);

    /* Draw the current list of icons */
    draw_current_iconlist(r);

    /* Draw the category browser */
    draw_categoryBrowser(r);

    GrDestroyGC(gc);
    GrDestroyRegion(r);
}

static void
root_redraw()
{
    GR_EVENT_EXPOSURE expose;
    GR_WINDOW_INFO info;

    /* dummy full area expose event */
    GrGetWindowInfo(GR_ROOT_WINDOW_ID, &info);
    expose.type = GR_EVENT_TYPE_EXPOSURE;
    expose.wid = GR_ROOT_WINDOW_ID;
    expose.x = 0;
    expose.y = 0;
    expose.width = info.width;
    expose.height = info.height;
    root_exposure(0, &expose);
}

inline void
redraw_root_window()
{
    root_redraw();
}

#ifdef CONFIG_PLATFORM_ZAURUS

static GR_TIMER_ID suspend_timer_id = 0;

static void handle_zaurus_suspend(void) {
  suspend_timer_id = 0;
  pm_suspend();
}

#endif

static void
root_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	root_exposure(window, &ep->exposure);
	break;

    case GR_EVENT_TYPE_KEY_DOWN:
#ifdef CONFIG_PLATFORM_ZAURUS
      if (ep->keystroke.ch == MWKEY_ESCAPE) {
	/* The key must be pressed for 4 seconds before the suspend happens */
	suspend_timer_id = GrCreateTimer(GR_ROOT_WINDOW_ID, 4000);
	
	if (suspend_timer_id) 
	  scrtop_register_timer(suspend_timer_id, handle_zaurus_suspend);      
      }
#endif
      break;

    case GR_EVENT_TYPE_KEY_UP:
#ifdef CONFIG_PLATFORM_ZAURUS
      /* If the key is released before the timer, then get rid of the timer */

      if (ep->keystroke.ch == MWKEY_ESCAPE) {
	if (suspend_timer_id) {
	  GrDestroyTimer(suspend_timer_id);
	  
	  scrtop_unregister_timer(suspend_timer_id);
	  suspend_timer_id = 0;
	}
      }
#endif
      
#ifdef CONFIG_PLATFORM_IPAQ
	if (ep->keystroke.ch == MWKEY_SUSPEND)
	   pm_suspend();
#endif

	break;

#ifdef CONFIG_PIXILWM_MENUS
#ifdef NOTUSED
    case GR_EVENT_TYPE_BUTTON_LONG_CLICK:
#endif
    case GR_EVENT_TYPE_BUTTON_DOWN:
	if (ep->button.buttons & GR_BUTTON_R) {
	    open_system_menu(GR_ROOT_WINDOW_ID,
			     ep->button.x, ep->button.y, MENU_DIR_BEST,
			     root_sys_menu);
	}

	break;
#endif
    }
}

void
root_free_memory()
{

#ifdef CONFIG_PIXILWM_MENUS
    destroy_system_menu(root_sys_menu);
#endif

}
