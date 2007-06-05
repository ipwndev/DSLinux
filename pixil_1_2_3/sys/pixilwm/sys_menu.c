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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MWINCLUDECOLORS
#include <nano-X.h>

#include "nanowm.h"
#include "apps.h"
#include "sys_menu.h"

/* Drawing defines */

#define XPADDING 3

typedef struct wm_menu_st
{
    GR_WINDOW_ID wid;
    GR_WINDOW_ID pid;
    GR_COLOR bcolor;
    GR_COLOR tcolor;
    wm_menu_t *data;
    struct wm_menu_st *next;
    unsigned char selected;
    unsigned char btndown;

    unsigned char icon_align;
}
wm_menu_struct;

static wm_menu_struct *wm_menu_head = 0;
static wm_menu_struct *wm_menu_active = 0;

static GR_FONT_ID sys_fontid = 0;

static int YITEMSIZE = 0;

/* Forward declarations */
void menu_wndproc(win *, GR_EVENT *);
static void close_menu(wm_menu_struct *);

static wm_menu_struct *
create_new_menu(GR_WINDOW_ID parent, wm_menu_t * data,
		int x, int y, int dir, GR_COLOR bcolor, GR_COLOR tcolor)
{
    int use_icon = 0;

    int count = 0;
    int xsize = 0, ysize = 0;
    win *window;
    GR_WINDOW_INFO winfo;
    GR_GC_ID gc;
    wm_menu_struct *ptr;

    if (!data)
	return (0);

    GrGetWindowInfo(parent, &winfo);

    /* Ok, first of all determine the size of the window */

    gc = GrNewGC();
    GrSetGCFont(gc, sys_fontid);

    for (count = 0; data[count].type != MENU_TYPE_END; count++) {
	int wi, hi, ba;

	if (data[count].icon)
	    use_icon = 1;

	if (data[count].type == MENU_TYPE_SEP)
	    ysize += 6;
	else {
	    ysize += YITEMSIZE + 2;

	    GrGetGCTextSize(gc, (void *) data[count].value,
			    strlen(data[count].value), 0, &wi, &hi, &ba);

	    if (data[count].icon)
		wi += YITEMSIZE;

	    if (wi > xsize) {
		switch (data[count].type) {
		case MENU_TYPE_CHECK:
		case MENU_TYPE_CMENU:
		    xsize = wi + 10 + XPADDING;
		    break;
		default:
		    xsize = wi + (XPADDING * 2);
		    if (data[count].icon)
			xsize += YITEMSIZE;
		    break;
		}
	    }
	}
    }

    /* Put some padding around it */
    xsize += 4;

    if (ysize > winfo.height)
	return (0);

    GrDestroyGC(gc);

    /* Now, figure out which direction the sucker should point */
    if (dir == MENU_DIR_BEST) {
	/* We would prefer to go down, but if there is no room */
	if ((winfo.height - y) < ysize)
	    dir = MENU_DIR_UP;
	else
	    dir = MENU_DIR_DOWN;
    }

    /* Adjust the X and Y to fit into the screen */
    if ((winfo.width - x) < xsize)
	x = winfo.width - xsize;

    /* Reposition the menu if needed to fit it in the window */

    if (dir == MENU_DIR_DOWN) {
	if ((winfo.height - y) < ysize)
	    y = winfo.height - ysize;
    } else {
	if ((y < ysize))
	    y = 0;
	else
	    y = y - ysize;
    }

    /* Ok, lets start allocating some room, eh? */
    ptr = (wm_menu_struct *) malloc(sizeof(wm_menu_struct));

    if (!ptr)
	return (0);

    ptr->data = data;
    ptr->selected = -1;
    ptr->btndown = 0;
    ptr->bcolor = bcolor;
    ptr->tcolor = tcolor;
    ptr->next = 0;
    ptr->pid = parent;
    ptr->icon_align = use_icon;

    /* Now, do the window stuff */
    ptr->wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE,
			     NULL, parent, x, y, xsize, ysize, bcolor);

    GrSelectEvents(ptr->wid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN |
		   GR_EVENT_MASK_BUTTON_UP);

    window = add_window(ptr->wid, parent, 0, menu_wndproc);
    GrMapWindow(ptr->wid);

    return (ptr);
}

/* 
 * This is the routine that actually handles the drawing for a given 
 * menu and wid
 */

static void
draw_menu(GR_WINDOW_ID wid, wm_menu_t * data, int selected,
	  GR_COLOR back, GR_COLOR text, int icon_align)
{
    GR_GC_ID gc = GrNewGC();
    GR_WINDOW_INFO winfo;
    int i, ypos, xpos;

    int menu_width;

    GrGetWindowInfo(wid, &winfo);
    GrRaiseWindow(wid);		/* Always raise system menus to the top */

    menu_width = winfo.width - 2;

    ypos = 0;

    /* The main loop */
    for (i = 0; data[i].type != MENU_TYPE_END; i++) {
	GrSetGCBackground(gc, back);
	GrSetGCForeground(gc, text);

	/* Now, based on each menu type, do the drawing */
	switch (data[i].type) {
	case MENU_TYPE_SEP:
	    /* Seperator line 2 pixels thick spanning entire window */
	    /* FIXME:  This should come out of PAR */

	    GrSetGCForeground(gc, MWRGB(0x00, 0x66, 0xCC));
	    GrLine(wid, gc, 2, ypos + 2, winfo.width - 4, ypos + 2);
	    GrLine(wid, gc, 2, ypos + 4, winfo.width - 4, ypos + 4);

	    GrSetGCForeground(gc, text);
	    GrLine(wid, gc, 2, ypos + 3, winfo.width - 4, ypos + 3);

	    ypos += 6;
	    break;

	case MENU_TYPE_CHECK:
	    /* Check box with a indicator before the text */

#ifdef NOTUSED
	    if (data[i].checked == GR_TRUE)
		GrFillEllipse(wid, gc, 5, ypos + (YITEMSIZE / 2), 3, 3);	/* Filled */
	    else
		GrEllipse(wid, gc, 5, ypos + (YITEMSIZE / 2), 3, 3);	/* Empty */
#endif

	    if (data[i].checked == GR_TRUE)
		GrFillRect(wid, gc, 4, ypos + 6, 4, 4);
#ifdef NOTUSED
	    else
		GrRect(wid, gc, 4, ypos + 6, 4, 4);
#endif

	    GrSetGCFont(gc, sys_fontid);
	    GrText(wid, gc, 10, ypos + 1, data[i].value, -1, GR_TFTOP);

	    ypos += YITEMSIZE + 2;
	    break;

	case MENU_TYPE_CMENU:
	    {
		GR_POINT points[] = {
		    {winfo.width - 10, ypos + 2}
		    ,
		    {winfo.width - 3, ypos + (YITEMSIZE / 2)}
		    ,
		    {winfo.width - 10, ypos + YITEMSIZE - 1}
		};

		/* The parent of a cascading menu.  Arrow after the text */

		GrSetGCFont(gc, sys_fontid);
		GrText(wid, gc, 1 + XPADDING, ypos + 1,
		       data[i].value, -1, GR_TFTOP);

		GrSetGCForeground(gc, text);
		GrFillPoly(wid, gc, 3, points);

		GrSetGCForeground(gc, GRAY);
		GrPoly(wid, gc, 3, points);

		ypos += YITEMSIZE + 2;
		break;
	    }

	case MENU_TYPE_APP:
	case MENU_TYPE_REGULAR:	/* These differ only by callback */

	    if (i == selected) {
		GrFillRect(wid, gc, 1, ypos, winfo.width - 2, YITEMSIZE + 1);

		GrSetGCForeground(gc, back);
		GrSetGCBackground(gc, text);

	    }

	    if (icon_align)
		xpos = YITEMSIZE + XPADDING;
	    else
		xpos = 1 + XPADDING;

	    if (data[i].icon)
		GrDrawImageToFit(wid, gc, 1, ypos + 1, YITEMSIZE,
				 YITEMSIZE, data[i].icon);

	    GrSetGCFont(gc, sys_fontid);

	    GrText(wid, gc, xpos, ypos + 1, data[i].value, -1, GR_TFTOP);

	    ypos += YITEMSIZE + 2;
	    break;
	}
    }

    /* Finally, draw a border around the window */
    GrSetGCForeground(gc, text);
    GrRect(wid, gc, 0, 0, winfo.width, winfo.height);

    GrDestroyGC(gc);
}

static int
get_item_pos(wm_menu_t * data, int selected)
{
    int ypos = 0;
    int i;

    for (i = 0; data[i].type != MENU_TYPE_END; i++) {
	if (i == selected)
	    return (ypos);

	switch (data[i].type) {
	case MENU_TYPE_SEP:
	    ypos += 6;
	    break;

	default:
	    ypos += YITEMSIZE + 2;
	}
    }

    return (0);
}

static int
check_mouse_pos(wm_menu_t * data, int x, int y)
{
    int i;
    int ypos = 0;

    for (i = 0; data[i].type != MENU_TYPE_END; i++)
	switch (data[i].type) {
	case MENU_TYPE_SEP:
	    /* Ignore seperators */
	    ypos += 6;
	    break;

	case MENU_TYPE_CMENU:
	case MENU_TYPE_APP:
	case MENU_TYPE_REGULAR:
	case MENU_TYPE_CHECK:
	    /* All these have the same ysize */
	    if (y >= ypos && y <= ypos + YITEMSIZE)
		return (i);

	    ypos += YITEMSIZE + 2;
	    break;
	}

    return (-1);
}

static void
expose_menu(GR_WINDOW_ID wid)
{
    wm_menu_struct *menu_ptr;

    if (!wm_menu_head)
	return;

    menu_ptr = wm_menu_head;

    /* Now go through and draw all of the active menus */

    while (menu_ptr) {
	/* Only draw those menus that require it */

	if (menu_ptr->wid == wid)
	    draw_menu(menu_ptr->wid, menu_ptr->data, menu_ptr->selected,
		      menu_ptr->bcolor, menu_ptr->tcolor,
		      menu_ptr->icon_align);

	menu_ptr = menu_ptr->next;
    }
}

static int
menu_buttonup(GR_EVENT_BUTTON * ep)
{
    int i;
    void *data;
    GR_WINDOW_INFO winfo;
    MENU_CALLBACK *cb;
    int ypos;

    if (!wm_menu_active)
	return (0);

    if (!wm_menu_active->btndown)
	return (0);

    /* Reset the buttondown flag */
    wm_menu_active->btndown = 0;

    /* Get the current menu selection */
    i = wm_menu_active->selected;

#ifdef SYSTEM_SOUNDS
    if (wm_menu_active->data[i].type != MENU_TYPE_SEP)
	play_sound(SOUND_CLICKED);
#endif

    switch (wm_menu_active->data[i].type) {
    case MENU_TYPE_REGULAR:
    case MENU_TYPE_CHECK:
	/* Do a callback */
	cb = (MENU_CALLBACK *) wm_menu_active->data[i].action;
	data = wm_menu_active->data[i].cbdata;

	/* Close the menu before we do the callback, just to be safe */
	close_menu(wm_menu_head);

	if (cb)
	    cb(data);

	return (1);

    case MENU_TYPE_APP:
	/* Do an app */
	if (wm_menu_active->data[i].action) {
	    APP *app;

	    /* Editorial:  I don't like this, but it seems to be the best way */
	    if (!
		(app =
		 find_app_path((char *) wm_menu_active->data[i].action)))
		app =
		    apps_add_onetime((char *) wm_menu_active->data[i].action);

	    if (app)
		launch_application(app);
	}

	close_menu(wm_menu_head);
	return (1);

    case MENU_TYPE_CMENU:
	/* Do a new child menu */
	/* Get some stats */

	GrGetWindowInfo(wm_menu_active->wid, &winfo);

	ypos = get_item_pos(wm_menu_active->data, wm_menu_active->selected);

	/* Make a new menu! */
	wm_menu_active->next =
	    create_new_menu(wm_menu_active->pid,
			    (wm_menu_t *) wm_menu_active->data[i].action,
			    winfo.x + winfo.width + 2, winfo.y + ypos,
			    MENU_DIR_BEST, WHITE, MWRGB(0x00, 0x66, 0xCC));

	if (wm_menu_active->next)
	    wm_menu_active = wm_menu_active->next;

	return (1);
    }

    return (0);
}

static int
menu_buttondown(GR_EVENT_BUTTON * ep)
{
    int ret;
    int backtrack = 0;

    if (!wm_menu_active)
	return (0);

    if (ep->wid != wm_menu_active->wid) {
	wm_menu_struct *ptr = wm_menu_head;

	/* Look to see if the click was anywhere in the tree */
	while (ptr) {
	    if (ptr->wid == ep->wid) {
		close_menu(ptr->next);
		wm_menu_active = ptr;
		backtrack = 1;
		break;
	    }
	    ptr = ptr->next;
	}

	if (!ptr) {
	    /* Close all of the menus */
	    close_menu(wm_menu_head);
	    return (1);
	}
    }

    ret = check_mouse_pos(wm_menu_active->data, ep->x, ep->y);

    if (ret != -1 && !backtrack) {
	wm_menu_active->selected = ret;
	wm_menu_active->btndown = 1;
	expose_menu(wm_menu_active->wid);
    }

    return (1);
}

static void
close_menu(wm_menu_struct * ptr)
{
    wm_menu_struct *mptr = wm_menu_head;
    wm_menu_struct *pptr = 0;

    if (!ptr && !wm_menu_head)
	return;

    mptr = wm_menu_head;

    while (mptr && mptr != ptr) {
	pptr = mptr;
	mptr = mptr->next;
    }

    if (!mptr)			/* Couldn't find it! */
	return;

    if (pptr) {
	pptr->next = 0;
	wm_menu_active = pptr;
    } else {
	wm_menu_head = 0;
	wm_menu_active = 0;
    }

    while (mptr) {
	win *window;

	/* If we didn't allocate it, why do we try to free it? */

	/* if (mptr->data)
	   free(mptr->data);
	 */

	window = find_window(mptr->wid);
	remove_window(window);
	GrDestroyWindow(mptr->wid);

	pptr = mptr;
	mptr = mptr->next;
	free(pptr);
    }
}

int
handle_system_menu(GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	expose_menu(ep->exposure.wid);
	return (0);

    case GR_EVENT_TYPE_BUTTON_DOWN:
	return (menu_buttondown(&ep->button));

    case GR_EVENT_TYPE_BUTTON_UP:
	return (menu_buttonup(&ep->button));
    }

    return (0);
}

void
menu_wndproc(win * window, GR_EVENT * ep)
{
    handle_system_menu(ep);
}

GR_BOOL
system_menu_active()
{
    if (wm_menu_active)
	return (1);
    else
	return (0);
}

int
open_system_menu(GR_WINDOW_ID parent, int x, int y,
		 int dir, wm_menu_t * menudata)
{
    GR_FONT_INFO finfo;

    if (wm_menu_head) {
	printf("Only one menu at a time!\n");
	return (0);
    }

    /* Allocate the font if not already done */

    if (!sys_fontid) {
	sys_fontid = GrCreateFont(GR_FONT_GUI_VAR, 0, 0);
	GrGetFontInfo(sys_fontid, &finfo);

	YITEMSIZE = finfo.height + 2;
    }

    wm_menu_head = create_new_menu(parent,
				   (wm_menu_t *) menudata,
				   x, y, dir, WHITE, MWRGB(0x00, 0x66, 0xCC));

#ifdef SYSTEM_SOUNDS
    play_sound(SOUND_MENU_OPENED);
#endif

    if (!wm_menu_head)
	return (0);
    else {
	wm_menu_active = wm_menu_head;
	return (1);
    }
}

void
destroy_system_menu(wm_menu_t * menu)
{
    int i;

    if (!menu)
	return;

    /* Go through and free as much as possible */

    for (i = 0; menu[i].type != MENU_TYPE_END; i++) {
	if (menu[i].type == MENU_TYPE_CMENU)
	    if (menu[i].action)
		destroy_system_menu(menu[i].action);
    }

    free(menu);
}

wm_menu_t *
create_system_menu(int size)
{
    return ((wm_menu_t *) malloc(size * sizeof(wm_menu_t)));
}


/* This is a simple utility function that will fill in a particular entry */
/* in the wm_menu_t structure.  This is defined inline for speed          */

/* There are macros in sys_menu.h that abstract this for each menu type   */

inline void
sys_menu_add_item(wm_menu_t * ptr, int tp, char *value, void *action,
		  void *data, int checked, GR_IMAGE_ID iid)
{
    ptr->type = tp;

    if (value) {
	strncpy(ptr->value, value, 24);

	if (strlen(value) > 24)
	    ptr->value[24] = 0;
    } else
	ptr->value[0] = 0;

    ptr->action = action;
    ptr->cbdata = data;
    ptr->checked = checked;
    ptr->icon = iid;
}
