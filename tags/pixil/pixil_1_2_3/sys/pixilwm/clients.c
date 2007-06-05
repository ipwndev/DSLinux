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

/* Portions copyright (C) 2000 Alex Holden <alex@linuxhacker.org> */

#include <pixil_config.h>

#include <stdio.h>
#include <stdlib.h>

#define MWINCLUDECOLORS
#include <nano-X.h>

#include <wm/nxlib.h>
#include "nxdraw.h"

#include "nanowm.h"
#include "apps.h"
#include "categories.h"

#include "themes.h"

/* Where to place the first window on the screen */
#define FIRST_WINDOW_LOCATION 2

/* The distance to leave between windows when deciding where to place */
#define WINDOW_STEP 20

static GR_COORD lastx = FIRST_WINDOW_LOCATION;
static GR_COORD lasty = FIRST_WINDOW_LOCATION;

/* JHC 08/08/01 - Instead of using the previously provided */
/* workspace, we are going to go ahead and use our own variables */
/* so we can increase and shrink it at random */

static int workspaceWidth = 0;
static int workspaceHeight = 0;

void
setWorkspace(int width, int height)
{
    workspaceWidth = width;
    workspaceHeight = height;

    /* Now resize any windows that may be affected */
    resize_windows(workspaceWidth, workspaceHeight);
}

void
getWorkspace(int *width, int *height)
{
    if (width)
	*width = workspaceWidth;
    if (height)
	*height = workspaceHeight;
}

static void client_wndproc(win * window, GR_EVENT * ep);

/*
 * A new client window has been mapped, so we need to reparent and decorate it.
 * Returns -1 on failure or 0 on success.
 */
int
client_updatemap_new(GR_WINDOW_ID wid)
{
    APP *lapp = 0;

    GR_WINDOW_ID pid;
    GR_COORD x, y, width, height, xoffset, yoffset;
    GR_SIZE w, h;
    GR_WM_PROPS style;
    GR_WINDOW_INFO winfo;
    GR_WM_PROPERTIES props;
    static GR_SCREEN_INFO si;

    /* get screen/workspace information */
    if (!si.rows)
	GrGetScreenInfo(&si);

    /* get client window information */
    GrGetWindowInfo(wid, &winfo);
    style = winfo.props;

    /* if not redecorating or not child of root window, return */
    if (winfo.parent != GR_ROOT_WINDOW_ID || (style & GR_WM_PROPS_NODECORATE))
	return 0;

    /* deal with replacing borders with window decorations */
    if (winfo.bordersize) {
	/*
	 * For complex reasons, it's easier to unmap,
	 * remove the borders, and then map again,
	 * rather than try to recalculate the window
	 * position in the server w/o borders.  By
	 * the time we get this event, the window has
	 * already been painted with borders...
	 * This currently causes a screen flicker as
	 * the window is painted twice.  The workaround
	 * is to create the window without borders in
	 * the first place.
	 */
	GrUnmapWindow(wid);

	/* remove client borders, if any */
	props.flags = style | GR_WM_FLAGS_BORDERSIZE;
	props.bordersize = 0;
	GrSetWMProperties(wid, &props);

	/* remap the window without borders, call this routine again */
	GrMapWindow(wid);
	return 0;
    }

    /* if default decoration style asked for, set real draw bits */
    if ((style & GR_WM_PROPS_APPMASK) == GR_WM_PROPS_APPWINDOW) {
	GR_WM_PROPERTIES pr;

	style = (style & ~GR_WM_PROPS_APPMASK) | nxGetDefaultWindowStyle();

	/* A quick hack - if NOAUTORESIZE is enabled */
	/* then clear the NOMOVE flag */

	if (style & GR_WM_PROPS_NOAUTORESIZE)
	    style &= ~GR_WM_PROPS_NOMOVE;

	pr.flags = GR_WM_FLAGS_PROPS;
	pr.props = style;
	GrSetWMProperties(wid, &pr);
    }
#ifdef CONFIG_PIXILWM_THEMES
    if (get_activeTheme())
	themeContainerSize(get_activeTheme(),
			   style, winfo.width, winfo.height,
			   &xoffset, &yoffset, &width, &height);
    else
#endif

	/* determine container size and client child window offsets */
	nxCalcNCSize(style, winfo.width, winfo.height, &xoffset, &yoffset,
		     &width, &height);

    /* determine x,y window location and width,height size */
    if ((style & (GR_WM_PROPS_NOAUTORESIZE | GR_WM_PROPS_MAXIMIZE)) ==
	GR_WM_PROPS_MAXIMIZE) {

	/* force to maximized position */
	x = 0;
	y = 0;
	width = workspaceWidth;
	height = workspaceHeight;
    } else if (style & GR_WM_PROPS_NOAUTOMOVE) {
	x = winfo.x;
	y = winfo.y;
    } else {

	/* We could probably use a more intelligent algorithm here */
	x = lastx + WINDOW_STEP;
	if ((x + width) > si.ws_width)
	    x = FIRST_WINDOW_LOCATION;
	lastx = x;
	y = lasty + WINDOW_STEP;
	if ((y + height) > si.ws_height)
	    y = FIRST_WINDOW_LOCATION;
	lasty = y;
    }

    /* create container window */
    pid = GrNewWindow(GR_ROOT_WINDOW_ID, x, y, width, height,
		      0, LTGRAY, BLACK);

    add_window(pid, GR_ROOT_WINDOW_ID, wid, container_wndproc);

    /* don't erase background of container window */
    props.flags = GR_WM_FLAGS_PROPS;
    props.props = style | GR_WM_PROPS_NOBACKGROUND;
    GrSetWMProperties(pid, &props);

    debug("New client window %d container %d\n", wid, pid);

    GrSelectEvents(pid, GR_EVENT_MASK_CHLD_UPDATE | GR_EVENT_MASK_UPDATE
		   | GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN
		   | GR_EVENT_MASK_MOUSE_POSITION | GR_EVENT_MASK_EXPOSURE);

    /* reparent client to container window */
    /* must map before reparent (nano-X bug) */

    GrMapWindow(pid);
    GrReparentWindow(wid, pid, xoffset, yoffset);

    /* resize client window to container */

#ifdef CONFIG_PIXILWM_THEMES
    if (get_activeTheme())
	themeClientSize(get_activeTheme(), style, width, height, &w, &h);
    else
#endif
	nxCalcClientSize(style, width, height, NULL, NULL, &w, &h);

    if (w != winfo.width || h != winfo.height)
	GrResizeWindow(wid, w, h);

    /* Get the application record for the new app */

    if (winfo.processid) {
	lapp = find_app_pid(winfo.processid);

	if (!lapp) {
	    char name[32] = { 0 };
	    if (ipcActive()
		&& ipcGetAppName(winfo.processid, name, sizeof(name)) == 0)
		lapp = find_app_name(name);
	}
    }

    if (lapp) {
	lapp->m_container_wid = pid;
	if (lapp->m_flags & FL_INPUT)
	    inputResizeWorkspace(lapp);
	lapp->m_virtual_wid = category_getCurrentWin();

	/* Record this for future HOME actions */
	g_last_app = lapp;
    }
#ifdef NOTUSED
    if (LastLaunchedApp) {
	LastLaunchedApp->m_container_wid = pid;

	if (LastLaunchedApp->m_flags & FL_INPUT)
	    inputResizeWorkspace(LastLaunchedApp);

	LastLaunchedApp->m_virtual_wid = category_getCurrentWin();
	LastLaunchedApp = NULL;
    }
#endif

    /*GrMapWindow(pid); */
    GrSetFocus(wid);		/* force fixed focus */

    /* add client window */
    add_window(wid, pid, 0, client_wndproc);

    /* Push this container window to the very top */
    zorder_push(pid);

#ifdef VIRTUAL_WINDOWS
    /* Redraw the virtual window switcher to handle the missing window */
    /* virtswitch_redraw(); */
#endif

    return 0;
}

/*
 * We've just received an event notifying us that a client window has been
 * unmapped, so we need to destroy all of the decorations.
 */
static void
client_destroy(win * window)
{
    APP *app;
    win *pwin;
    GR_WINDOW_ID pid, zwid;

    debug("Client window %d has been destroyed\n", window->wid);

    if (!(pwin = find_window(window->pid))) {
	error("Couldn't find the parent of window %d.\n", window->wid);
	return;
    }

    pid = pwin->wid;

    /* The client just went away, see if we can clear */
    /* the outstanding values                         */

    app = find_app_container(pid);

    /* If the app exists, then clear the values */

    if (app) {
	app->m_container_wid = 0;
	app->m_process_id = 0;
    } else
	error("No application was associated with container %d.\n", pid);

    /* Remove this window from the zorder */
    zorder_remove(pid);

    /* Activate the next window in the list */
    if ((zwid = zorder_peek_top()))
	container_activate(zwid);

    /* Remove the entire window from the window list */
    remove_window_and_children(pwin);

    debug("Destroying container %d\n", pid);
    GrDestroyWindow(pid);
}

static void
client_update(win * window, GR_EVENT_UPDATE * ep)
{
    switch (ep->utype) {
    case GR_UPDATE_DESTROY:
	client_destroy(window);
	break;
    }
}

static void
client_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_CHLD_UPDATE:
	client_update(window, &ep->update);
	break;
    }
}
