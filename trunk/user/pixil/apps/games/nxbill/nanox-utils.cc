 /*
 * Copyright (C) 200-2002 Century Embedded Techonlogies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* These are NanoX utilties designed to act */
/* like the Xt counterparts (but better) */

#include "objects.h"
#include "NXUI.h"

GR_GC_ID menubargc;

typedef struct wcallback
{
    GR_WINDOW_ID wid;
    int event;
    GR_FNCALLBACKEVENT callback;
    struct wcallback *next;
}
widget_callback;

widget_callback *callback_list;




GR_WINDOW_ID
NXAppInit(char *title, int xsize, int ysize)
{
    GR_WINDOW_ID newwin;

    newwin =
	GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, xsize, ysize, 0, BLACK, WHITE);

    GrSelectEvents(newwin, GR_EVENT_MASK_CLOSE_REQ);

    /* Create a top level window with the appropriate title */
    return (newwin);
}

void
NXRealizeWidget(GR_WINDOW_ID widget)
{
    GR_WINDOW_INFO ptr;
    GR_WINDOW_ID child;

    /* First show the parent */
    GrMapWindow(widget);

    GrGetWindowInfo(widget, &ptr);

    if (!ptr.child)
	return;


    child = ptr.child;

    GrMapWindow(child);

    GrGetWindowInfo(child, &ptr);

    while (ptr.sibling) {
	GrMapWindow(ptr.sibling);

	GrGetWindowInfo(ptr.sibling, &ptr);
    }
}


void
check_menubar(GR_EVENT * event)
{
    int x, y;
    GR_RECT rect;

    /* Check to see if we have to do any events */

    rect.x = 2;
    rect.y = 2;
    rect.width = 40;
    rect.height = 23;


    x = event->button.x;
    y = event->button.y;

    if (PtInRect(&rect, event->button.x, event->button.y))
	game.warp_to_level(1);
}


void
redraw_menubar(GR_EVENT * event)
{

}

GR_WINDOW_ID
CreateMenuBar(char *name, GR_WINDOW_ID parent)
{
    /* Create a menu bar with three beautiful buttons */

    menubargc = GrNewGC();

    return (GrNewWindow(parent, 0, 0, 240, 35, 0, BLACK, GRAY));
}

GR_WINDOW_ID
CreatePixmapBox(const char *name, GR_WINDOW_ID parent,
		GR_WINDOW_ID pixmap, const char *text)
{
    printf("I would create a pixmap box here!\n");
}



GR_WINDOW_ID
CreateDrawingArea(char *name, GR_WINDOW_ID parent, int width, int height)
{
    return GrNewWindow(parent, 0, 30, width, height, 0, BLACK, WHITE);
}

GR_WINDOW_ID
CreateRowCol(char *name, GR_WINDOW_ID parent)
{
    printf("I would be creating something here\n");
    return (0);
}

int
NXGetWidgetCallback(GR_WINDOW_ID wid, GR_EVENT * event)
{
    widget_callback *n = callback_list;

    if (!n)
	return (0);

    while (n) {
	if (n->wid == wid && n->event == event->type) {
	    n->callback(event);
	    return (1);
	}

	n = n->next;
    }

    return (0);
}

void
NXRegisterWidgetCallback(GR_WINDOW_ID wid, int event,
			 GR_FNCALLBACKEVENT callback)
{
    widget_callback *n;

    if (!callback_list) {
	callback_list = (widget_callback *) malloc(sizeof(widget_callback));
	n = callback_list;
    } else {
	n = callback_list;
	while (n->next)
	    n = n->next;

	n->next = (widget_callback *) malloc(sizeof(widget_callback));
	n = n->next;
    }

    n->wid = wid;
    n->event = event;
    n->callback = callback;
    n->next = 0;

}

void
NXPopup(GR_WINDOW_ID wid)
{
    printf("Poping up a dialog box!\n");
    /* Pop me up, give me the focus and don't do a damn thing until */
    /* I respond */

    GrMapWindow(wid);
    GrRaiseWindow(wid);
    GrSetFocus(wid);
}


GR_WINDOW_ID
NXCreateDialog(GR_WINDOW_ID parent, char *name, char *str)
{
    /* By default, we are popped up right smack in the middle of the screen */


    return (GrNewWindow(parent, 0, 0, 10, 10, 0, BLACK, WHITE));
}
