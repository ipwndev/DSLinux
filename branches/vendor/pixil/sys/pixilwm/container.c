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


/* Portions Copyright (C) 2000 Alex Holden <alex@linuxhacker.org> */

#include <pixil_config.h>

#include <stdio.h>
#include <stdlib.h>
#define MWINCLUDECOLORS
#include "nano-X.h"
#include "nxdraw.h"

#include "nanowm.h"
#include "apps.h"
#include "themes.h"

//#define OUTLINE_MOVE 1
//#define OUTLINE_RESIZE 1

static void
container_redraw(win * window)
{
    GR_WINDOW_INFO info;
    GR_WM_PROPERTIES props;
    GR_BOOL active;

    debug("container_exposure window %d\n", window->wid);

    GrGetWindowInfo(window->wid, &info);
    GrGetWMProperties(window->clientid, &props);

    /* Check for invalid window */
    if (props.flags == 0)
	return;

    active = (window->clientid == GrGetFocus());

#ifdef CONFIG_PIXILWM_THEMES
    if (get_activeTheme())
	themeDrawContainer(window->wid, info.width, info.height, props.title,
			   active, props.props, get_activeTheme());
    else
#endif
	nxPaintNCArea(window->wid, info.width, info.height, props.title,
		      active, props.props);

    /* Ack, this was malloced, and it is our responsibility to free it */
    if (props.title)
	free(props.title);
}

static void
container_exposure(win * window, GR_EVENT_EXPOSURE * event)
{
    debug("container_exposure window %d\n", window->wid);
    container_redraw(window);
}

static GR_BOOL
PtInRect(GR_RECT * prc, GR_SIZE x, GR_SIZE y)
{
    return (x >= prc->x && x < (prc->x + prc->width) &&
	    y >= prc->y && y < (prc->y + prc->height));
}

/* NOTE:  This is the non-themed closebox handler */

static int
check_closebox(GR_WINDOW_INFO * info, int ex, int ey)
{

    int cxborder = 0, cyborder = 0;

    GR_RECT r;

    if (info->props & GR_WM_PROPS_BORDER) {
	cxborder = 1;
	cyborder = 1;
    }
    if (info->props & GR_WM_PROPS_APPFRAME) {
	cxborder = CXBORDER;
	cyborder = CYBORDER;
    }

    r.x = info->width - CXCLOSEBOX - cxborder - 2;
    r.y = cyborder + 2;
    r.width = CXCLOSEBOX;
    r.height = CYCLOSEBOX;

    /* Check mousedn in close box */
    return (PtInRect(&r, ex, ey));
}

static int
check_titlebar(GR_WINDOW_INFO * info, int ex, int ey)
{
    GR_RECT r;
    int cxborder = 0, cyborder = 0;

    if (info->props & GR_WM_PROPS_BORDER) {
	cxborder = 1;
	cyborder = 1;
    }
    if (info->props & GR_WM_PROPS_APPFRAME) {
	cxborder = CXBORDER;
	cyborder = CYBORDER;
    }

    r.x = cxborder;
    r.y = cyborder;
    r.width = info->width - cxborder * 2;
    r.height = CYCAPTION;

    /* Check for mousedn in caption box */
    return (PtInRect(&r, ex, ey));
}

static void
container_buttondown(win * window, GR_EVENT_BUTTON * event)
{
    GR_RECT r;

    int ret;
    GR_WINDOW_INFO info;

    struct pos_size *pos = &window->pos;

    debug("container_buttondown window %d\n", window->wid);

    if (window->mousedn)
	return;

    GrGetWindowInfo(window->wid, &info);

    /* Check for close box press */
    if ((info.props & (GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX)) ==
	(GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX)) {

#ifdef CONFIG_PIXILWM_THEMES
	if (get_activeTheme())
	    ret = widgetCheckBounds(get_activeTheme(),
				    info.props, TITLEBAR_WIDGET_CLOSEBUTTON,
				    event->x, event->y,
				    info.width, info.height);
	else
#endif
	    ret = check_closebox(&info, event->x, event->y);

	if (ret) {
	    GrCloseWindow(window->clientid);
	    return;
	}
    }

    /* Set focus on button down */
    GrSetFocus(window->clientid);

    /* check for corner resize */
    r.x = info.width - 5;
    r.y = info.height - 5;
    r.width = 5;
    r.height = 5;

    if ((info.props & GR_WM_PROPS_APPFRAME)
	&& PtInRect(&r, event->x, event->y)) {

	window->sizing = GR_TRUE;
	pos = &window->pos;

	// save off the width/height offset from the window manager
	GrGetWindowInfo(window->clientid, &info);
	pos->xoff = -info.width;
	pos->yoff = -info.height;

	GrGetWindowInfo(window->wid, &info);
	pos->xoff += info.width;
	pos->yoff += info.height;


#ifdef OUTLINE_RESIZE
	// let mouse move know that we havn't drawn the rect yet
	pos->width = -1;
	pos->height = -1;
#endif
	window->mousedn = GR_TRUE;
	return;
    }

    /* if not in caption, return (FIXME, not calc'd exactly) */
    if (!(info.props & GR_WM_PROPS_CAPTION))
	return;

    /* Check the bounds of the whole shooting titlebar */

#ifdef CONFIG_PIXILWM_THEMES
    if (get_activeTheme())
	ret = themeCheckBounds(get_activeTheme(),
			       info.props, COMPONENT_TITLEBAR,
			       event->x, event->y, info.width, info.height);
#endif
    else
	ret = check_titlebar(&info, event->x, event->y);

    if (!ret)
	return;

    /* Raise window if mouse down and allowed */
    if (!(info.props & GR_WM_PROPS_NORAISE)) {
	GrRaiseWindow(window->wid);
	zorder_raise(window->wid);
    }

    /* Don't allow window move if NOMOVE property set */
    if (info.props & GR_WM_PROPS_NOMOVE)
	return;

    window->x = event->x;
    window->y = event->y;

    pos->xoff = event->x;
    pos->yoff = event->y;

#ifdef OUTLINE_MOVE
    pos->xorig = -1;
    pos->yorig = -1;
    pos->width = info.width;
    pos->height = info.height;
#endif
    window->mousedn = GR_TRUE;
}

static void
container_buttonup(win * window, GR_EVENT_BUTTON * event)
{
#if OUTLINE_RESIZE | OUTLINE_MOVE
    GR_GC_ID gc;
#endif
    struct pos_size *pos = &window->pos;

    debug("container_buttonup window %d\n", window->wid);

    if (window->mousedn && !window->sizing &&
	pos->xorig != -1 && pos->yorig != -1) {
#ifdef OUTLINE_MOVE
	gc = GrNewGC();
	GrSetGCMode(gc, GR_MODE_XOR | GR_MODE_EXCLUDECHILDREN);
	GrRect(GR_ROOT_WINDOW_ID, gc, pos->xorig, pos->yorig,
	       pos->width, pos->height);
	GrMoveWindow(window->wid, pos->xorig, pos->yorig);

#endif
    } else if (window->sizing && pos->width != -1 && pos->height != -1) {
	GR_WINDOW_INFO info;

	GrGetWindowInfo(window->wid, &info);

#ifdef OUTLINE_RESIZE
	gc = GrNewGC();
	GrSetGCMode(gc, GR_MODE_XOR | GR_MODE_EXCLUDECHILDREN);

	GrRect(GR_ROOT_WINDOW_ID, gc, info.x, info.y, pos->width,
	       pos->height);
	GrDestroyGC(gc);
#endif
	GrResizeWindow(window->wid, event->rootx - info.x,
		       event->rooty - info.y);
    }

    window->sizing = GR_FALSE;
    window->mousedn = GR_FALSE;
}

static void
container_mousemoved(win * window, GR_EVENT_MOUSE * event)
{
    GR_WINDOW_INFO info;
    struct pos_size *pos;
#if OUTLINE_RESIZE | OUTLINE_MOVE
    GR_GC_ID gc;
#endif
    debug("container_mousemoved window %d\n", window->wid);

    if (!window->mousedn)
	return;

    pos = (struct pos_size *) &window->pos;

    if (window->sizing) {

	GrGetWindowInfo(window->wid, &info);

#ifdef OUTLINE_RESIZE
	gc = GrNewGC();
	GrSetGCMode(gc, GR_MODE_XOR | GR_MODE_EXCLUDECHILDREN);

	// erase old rectangle if necessary
	if (pos->width != -1 && pos->height != -1)
	    GrRect(GR_ROOT_WINDOW_ID, gc, info.x, info.y, pos->width,
		   pos->height);

	// draw new one
	GrRect(GR_ROOT_WINDOW_ID, gc, info.x, info.y,
	       event->rootx - info.x, event->rooty - info.y);

	GrDestroyGC(gc);
	// save this new rectangle's width, height
	// I know, this shouldn't be stored in x/y, but...
	pos->width = event->rootx - info.x;
	pos->height = event->rooty - info.y;
#else
	GrResizeWindow(window->wid, event->rootx - info.x,
		       event->rooty - info.y);
#endif
	return;
    }
#ifdef OUTLINE_MOVE
    gc = GrNewGC();
    GrSetGCMode(gc, GR_MODE_XOR | GR_MODE_EXCLUDECHILDREN);

    if (pos->xorig != -1 && pos->yorig != -1)
	GrRect(GR_ROOT_WINDOW_ID, gc, pos->xorig, pos->yorig,
	       pos->width, pos->height);

    GrRect(GR_ROOT_WINDOW_ID, gc, event->rootx - pos->xoff,
	   event->rooty - pos->yoff, pos->width, pos->height);

    pos->xorig = event->rootx - pos->xoff;
    pos->yorig = event->rooty - pos->yoff;

    GrDestroyGC(gc);
#else
    GrMoveWindow(window->wid, event->rootx - pos->xoff,
		 event->rooty - pos->yoff);
#endif

#ifdef VIRTUAL_WINDOWS
    /* Redraw the window box so it knows whats up */
    /* virtswitch_redraw(); */
#endif
}

static void
container_child_update(win * window, GR_EVENT_UPDATE * ep)
{
    switch (ep->utype) {
    case GR_UPDATE_ACTIVATE:
	container_redraw(window);
	break;
    }
}



static void
container_resize(win * window, GR_EVENT_UPDATE * ep)
{
    int w, h;

    GR_WINDOW_INFO info;
    GrGetWindowInfo(window->wid, &info);

    // calculate new size for the client window


#ifdef CONFIG_PIXILWM_THEMES
    if (get_activeTheme())
	themeClientSize(get_activeTheme(), info.props, ep->width, ep->height,
			&w, &h);
    else
#endif
	nxCalcClientSize(info.props, ep->width, ep->height, NULL, NULL, &w,
			 &h);

    // resize client window
    GrResizeWindow(window->clientid, w, h);
}

static void
container_update(win * window, GR_EVENT_UPDATE * ep)
{
    switch (ep->utype) {
    case GR_UPDATE_SIZE:
	container_resize(window, ep);
	break;
    }
}

void
container_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	container_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	container_buttondown(window, &ep->button);
	break;
    case GR_EVENT_TYPE_BUTTON_UP:
	container_buttonup(window, &ep->button);
	break;
    case GR_EVENT_TYPE_UPDATE:
	container_update(window, &ep->update);
	break;
    case GR_EVENT_TYPE_MOUSE_POSITION:
	container_mousemoved(window, &ep->mouse);
	break;
    case GR_EVENT_TYPE_CHLD_UPDATE:
	container_child_update(window, &ep->update);
	break;
    }
}

/* GLOBAL CONTAINER MANAGMENT */
/* This is an effort to consolidate the various container functions */
/* (map, unmap, etc...) */

/* This activates the given container */
/* By mapping it, raising it and giving it focus */

void
container_activate(GR_WINDOW_ID wid)
{

    GR_WINDOW_INFO info;
    win *window;

    if (wid == 0) {
	debug("Passed 0 to container_set_focus()\n");
	return;
    }

    if (!(window = find_window(wid))) {
	error("Couldn't find window '%d' in the window list\n", wid);
	return;
    }

    GrGetWindowInfo(wid, &info);

    if (!info.mapped) {
	zorder_push(wid);
	GrMapWindow(wid);
    } else {
	zorder_raise(wid);
	GrRaiseWindow(wid);
    }

    if (info.props & GR_WM_PROPS_NOFOCUS)
	return;
    GrSetFocus(window->clientid);
}

/* This hides the specified container */

void
container_hide(GR_WINDOW_ID wid)
{

    if (!wid)
	return;

    /* Pull it out of the visible zorder */
    zorder_remove(wid);

    /* And unmap it */
    GrUnmapWindow(wid);
}
