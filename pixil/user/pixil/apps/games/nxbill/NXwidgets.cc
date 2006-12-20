 /* Copyright (C) 200-2002 Century Embedded Techonlogies
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

/* This is an attempt to put some method to the maddness */
/* of the various widgets created and handled by NXBill  */

/* It is my sincere hope that will will be merged with   */
/* any standard Microwindows widgets at some point in    */
/* the future                                            */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NXwidgets.h"

typedef struct nx_callback_t
{
    GR_WINDOW_ID wid;
    int event;
    GR_FNCALLBACKEVENT callback;
    struct nx_callback_t *next;
}
NX_CALLBACK;

typedef struct nx_dialog_t
{
    char str[255];
}
NX_DIALOG;

typedef struct nx_widget_t
{
    int x;			/* Position of the widget */
    int y;

    GR_WINDOW_ID wid;
    GR_WINDOW_ID pid;		/* Parent of the widget */

    NX_DIALOG dialog;

    NX_CALLBACK *callback_list;
    struct nx_widget_t *next;
}
NX_WIDGET;

typedef struct nx_app_t
{
    int wid;
    NX_WIDGET *widgets;
}
NX_APP;

static NX_APP global_app;
extern GR_FONT_ID global_font;

static int handle_dialog_up(GR_WINDOW_ID wid);
static void handle_dialog_expose(GR_WINDOW_ID wid, char *str);

static NX_WIDGET *
find_widget(GR_WINDOW_ID wid)
{
    NX_WIDGET *ptr = global_app.widgets;

    while (ptr) {
	if (ptr->wid == wid)
	    return (ptr);

	ptr = ptr->next;
    }

    return (0);
}

GR_WINDOW_ID
createNXApp(char *title, int xsize, int ysize)
{
    global_app.wid =
	GrNewWindowEx(GR_WM_PROPS_APPWINDOW, (GR_CHAR *) "nxBill",
		      GR_ROOT_WINDOW_ID, 0, 0, xsize, ysize, WHITE);

    GrSelectEvents(global_app.wid, GR_EVENT_MASK_CLOSE_REQ);

    GrMapWindow(global_app.wid);
    global_app.widgets = 0;

    return (global_app.wid);
}

void
killNXApp(GR_WINDOW_ID app)
{
    NX_WIDGET *ptr = global_app.widgets;
    NX_WIDGET *nxt;

    while (ptr) {
	NX_CALLBACK *n, *t;

	nxt = ptr->next;

	if ((n = ptr->callback_list))
	    while (n) {
		t = n->next;
		free(n);
		n = t;
	    }

	free(ptr);
	ptr = nxt;
    }
}

GR_WINDOW_ID
createNXWidget(GR_WINDOW_ID parent, int x, int y, int width, int height)
{
    NX_WIDGET *ptr;

    /* Create a entry for the widget in the list */
    if (global_app.widgets == 0) {
	global_app.widgets = (NX_WIDGET *) malloc(sizeof(NX_WIDGET));
	ptr = global_app.widgets;
    } else {
	ptr = global_app.widgets;
	while (ptr->next)
	    ptr = ptr->next;
	ptr->next = (NX_WIDGET *) malloc(sizeof(NX_WIDGET));
	ptr = ptr->next;
    }

    ptr->x = x;
    ptr->y = y;

    ptr->pid = parent;

    ptr->next = 0;
    ptr->callback_list = 0;

    ptr->wid = GrNewWindow(parent, x, y, width, height, 0, WHITE, WHITE);

    return (ptr->wid);
}

void
registerNXWidgetCallback(GR_WINDOW_ID wid, int event,
			 GR_FNCALLBACKEVENT callback)
{
    GR_WINDOW_INFO iinfo;
    NX_WIDGET *ptr = find_widget(wid);
    NX_CALLBACK *n;

    if (!ptr)
	return;

    if (!ptr->callback_list) {
	ptr->callback_list = (NX_CALLBACK *) malloc(sizeof(NX_CALLBACK));
	n = ptr->callback_list;
    } else {
	n = ptr->callback_list;

	while (n->next)
	    n = n->next;

	n->next = (NX_CALLBACK *) malloc(sizeof(NX_CALLBACK));
	n = n->next;
    }

    n->event = event;
    n->callback = callback;
    n->next = 0;

    /* Get the current event mask */
    GrGetWindowInfo(wid, &iinfo);

    /* Set this window to respond to the event in question */
    GrSelectEvents(wid, GR_EVENTMASK(event) | iinfo.eventmask);
}

int
fireNXWidgetCallback(GR_WINDOW_ID wid, GR_EVENT * event)
{
    NX_WIDGET *ptr = find_widget(wid);
    NX_CALLBACK *n;

    if (!ptr)
	return (0);

    n = ptr->callback_list;

    if (!n)
	return (0);

    while (n) {
	if (n->event == event->type) {
	    n->callback(event);
	    return (1);
	}

	n = n->next;
    }

    return (0);
}

void
realizeNXWidget(GR_WINDOW_ID widget)
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

GR_WINDOW_ID
createNXDialog(GR_WINDOW_ID parent, char *str)
{
    NX_WIDGET *ptr;

    GR_WINDOW_ID wid = createNXWidget(parent, 5, 100, 220, 125);

    ptr = find_widget(wid);

    if (!ptr)
	return (0);

    if (str) {
	int count, i;

	/* Count the carriage returns so we know how big to resize the window */

	for (i = 0, count = 0; i < (int) strlen(str); i++)
	    if (str[i] == '\n')
		count++;

	GrResizeWindow(wid, 220, ((count + 1) * 15) + 20);

	strncpy(ptr->dialog.str, str, 254);
    } else
	ptr->dialog.str[0] = 0;

    return (wid);
}

void
popupNXDialog(GR_WINDOW_ID dialog, char *str)
{
    GR_EVENT event;

    GrSelectEvents(dialog,
		   GR_EVENT_MASK_BUTTON_DOWN |
		   GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_EXPOSURE);

    GrMapWindow(dialog);
    GrRaiseWindow(dialog);

    /* Here we wait for a button down */

    while (1) {
	GrGetNextEvent(&event);

	switch (event.type) {
	case GR_EVENT_TYPE_BUTTON_UP:
	    if (event.button.wid != dialog)
		break;

	    if (handle_dialog_up(event.button.wid)) {
		GrUnmapWindow(dialog);
		return;
	    }

	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);

	case GR_EVENT_TYPE_EXPOSURE:
	    handle_dialog_expose(dialog, str);
	    fireNXWidgetCallback(event.exposure.wid, &event);
	    break;
	}
    }
}

void
handle_dialog_expose(GR_WINDOW_ID wid, char *str)
{
    char *c;
    GR_GC_ID newgc;
    GR_WINDOW_INFO iinfo;
    int ypos;

    /* Show the dialog box */

    NX_WIDGET *ptr = find_widget(wid);

    if (!ptr)
	return;

    /* If we are using a dynamic string, then resize the window */

    if (ptr->dialog.str[0] == 0) {
	int count = 0, i;

	/* Count the carriage returns so we know how big to resize the window */
	for (i = 0; i < (int) strlen(str); i++)
	    if (str[i] == '\n')
		count++;

	GrResizeWindow(wid, 220, ((count + 1) * 15) + 20);
    }

    GrGetWindowInfo(wid, &iinfo);

    newgc = GrNewGC();

    GrSetGCMode(newgc, GR_MODE_SET);
    GrSetGCFont(newgc, GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0));

    /* Do some pretty drawing */

    GrSetGCForeground(newgc, LTGRAY);
    GrFillRect(wid, newgc, 0, 0, iinfo.width, iinfo.height);

    nxDraw3dBox(ptr->wid, 0, 0, iinfo.width, iinfo.height, GRAY, BLACK);

    GrSetGCBackground(newgc, LTGRAY);
    GrSetGCForeground(newgc, BLACK);

    if (ptr->dialog.str[0] != 0)
	c = ptr->dialog.str;
    else
	c = str;

    ypos = 20;

    while (*c != 0) {
	char *s = c;

	for (; *c != 0 && *c != '\n'; c++);

	GrText(wid, newgc, 10, ypos, s, (int) (c - s), 0);

	if (*c)
	    c++;

	/* Figure this out responsibly! */
	ypos += 15;
    }

    GrDestroyGC(newgc);
}

int
handle_dialog_up(GR_WINDOW_ID wid)
{
    return (1);
}
