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

#include <stdio.h>
#include <stdlib.h>
#include "nano-X.h"

#include "nanowm.h"

static win *windows = NULL;
static win *zHead = NULL;

/*
 * Find the windowlist entry for the specified window ID and return a pointer
 * to it, or NULL if it isn't in the list.
 */

win *
find_window(GR_WINDOW_ID wid)
{
    win *w = windows;

    debug("Looking for window %d...\n", wid);

    while (w) {
	if (w->wid == wid) {
	    debug("found it!\n");
	    return w;
	}
	w = w->next;
    }

    debug("Nope, %d is not in the list\n", wid);
    return NULL;
}

/*
 * Add a new entry to the front of the windowlist.
 * Returns -1 on failure or 0 on success.
 */
win *
add_window(GR_WINDOW_ID wid, GR_WINDOW_ID pid, GR_WINDOW_ID clid,
	   wndproc proc)
{
    win *w;

    debug("Adding window %d\n", wid);

    if (!(w = calloc(sizeof(win), 1)))
	return NULL;

    w->pos.xorig = -1;
    w->pos.yorig = -1;
    w->wid = wid;
    w->pid = pid;
    w->clientid = clid;
    w->proc = proc;
    w->mousedn = GR_FALSE;

    w->zorder.next = 0;
    w->zorder.prev = 0;

    w->next = windows;
    windows = w;

    return w;
}

/* This will resize any maximized windows to the specified width and height */
/* I didn't know where else to put this function */

void
resize_windows(int width, int height)
{

    win *w = windows;

    for (w = windows; w; w = w->next) {

	GR_WM_PROPERTIES props;

	/* Only resize the containers */
	if (!w->wid || !w->clientid)
	    continue;

	GrGetWMProperties(w->wid, &props);

	if ((props.props & GR_WM_PROPS_NOAUTORESIZE) ==
	    GR_WM_PROPS_NOAUTORESIZE)
	    continue;

	if ((props.props & GR_WM_PROPS_MAXIMIZE) == GR_WM_PROPS_MAXIMIZE)
	    GrResizeWindow(w->wid, width, height);

    }
}


/*
 * Remove an entry from the windowlist.
 * We must search through the list for it so that we can find the previous
 * entry in the list and fix the next pointer. The alternative is to add a
 * prev pointer to the structure which would increase the memory usage.
 * Returns -1 on failure or 0 on success.
 */
int
remove_window(win * window)
{
    win *w = windows;
    win *prev = NULL;

    while (w) {
	if (w == window) {
	    if (!prev)
		windows = w->next;
	    else
		prev->next = w->next;

	    /* Set the zorder */

	    if (w->zorder.prev)
		w->zorder.prev->zorder.next = w->zorder.next;

	    if (w->zorder.next)
		w->zorder.next->zorder.prev = w->zorder.prev;

	    if (w == zHead)
		zHead = w->zorder.next;

	    free(w);
	    return 0;
	}
	prev = w;
	w = w->next;
    }

    return -1;
}

/*
 * Remove an entry and all it's children from the windowlist.
 * Returns -1 on failure or 0 on success.
 */
int
remove_window_and_children(win * window)
{
    win *t, *w = windows;
    win *prev = NULL;
    GR_WINDOW_ID pid = window->wid;

    debug("Removing window %d and children\n", window->wid);

    while (w) {
	if ((w->pid == pid) || (w == window)) {

	    if (prev)
		prev->next = w->next;
	    else
		windows = w->next;
	    t = w->next;

	    /* Set the zorder */

	    if (w->zorder.prev)
		w->zorder.prev->zorder.next = w->zorder.next;

	    if (w->zorder.next)
		w->zorder.next->zorder.prev = w->zorder.prev;

	    if (w == zHead)
		zHead = w->zorder.next;

	    free(w);
	    w = t;
	    continue;
	}
	prev = w;
	w = w->next;
    }

    return -1;
}

#ifdef ZDEBUG

static void
zorder_print()
{

    win *win = zHead;

    fprintf(stderr, "---- Z-Order List ----\n");

    if (!zHead) {
	fprintf(stderr, "<none>\n");
    } else {
	while (win) {
	    fprintf(stderr, "%p <-- %d --> %p\n",
		    win->zorder.prev, win->wid, win->zorder.next);

	    win = win->zorder.next;
	}
    }

    fprintf(stderr, "-------------\n");
}

#endif

/* Push this window to the top of the zorder */

int
zorder_push(GR_WINDOW_ID id)
{

    win *w = find_window(id);
    if (!w)
	return (-1);

    if (!zHead) {
	zHead = w;
	w->zorder.prev = w->zorder.next = 0;
    }

    else {
	w->zorder.next = zHead;
	w->zorder.prev = 0;

	zHead->zorder.prev = w;
	zHead = w;
    }

#ifdef ZDEBUG
    zorder_print();
#endif

    return (0);
}

/* Get the top window from the zorder */

GR_WINDOW_ID
zorder_peek_top(void)
{

    if (zHead)
	return (zHead->wid);
    else
	return (0);

}

/* Remove an abitrary window from the linked list */

void
zorder_remove(GR_WINDOW_ID id)
{

    win *w = find_window(id);

    if (!w)
	return;

    if (w->zorder.prev)
	w->zorder.prev->zorder.next = w->zorder.next;

    if (w->zorder.next)
	w->zorder.next->zorder.prev = w->zorder.prev;

    if (w == zHead)
	zHead = w->zorder.next;

    w->zorder.prev = w->zorder.next = 0;

#ifdef ZDEBUG
    zorder_print();
#endif

}

void
zorder_raise(GR_WINDOW_ID id)
{

    win *w = find_window(id);
    if (!w)
	return;

    if (w == zHead)
	return;

    /* First, pull it out of the list */

    if (w->zorder.prev)
	w->zorder.prev->zorder.next = w->zorder.next;

    if (w->zorder.next)
	w->zorder.next->zorder.prev = w->zorder.prev;

    /* Now, push it on top */
    w->zorder.next = w->zorder.prev = 0;

    w->zorder.next = zHead;
    zHead->zorder.prev = w;

    zHead = w;

#ifdef ZDEBUG
    zorder_print();
#endif

}

#ifdef NOTUSED

/* Pop the top window from the zorder */
/* And return the new top window      */

GR_WINDOW_ID
zorder_pop(void)
{

    GR_WINDOW_ID ret = 0;
    win *ptr = zHead;

    if (!ptr)
	return (0);

    ret = ptr->wid;
    next = ptr->zorder.next;

    if (next)
	next->zorder.prev = 0;

    zHead = ptr->zorder.next;

    ptr->zorder.next = 0;
    ptr->zorder.prev = 0;


    return (ret);
}

#endif
