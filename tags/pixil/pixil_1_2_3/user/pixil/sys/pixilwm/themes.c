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


/* VERY SUPER SECRET */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nano-X.h>

#include "nanowm.h"
#include "themes.h"
#include "config.h"

static theme_t *activeTheme = 0;

static void
getWidgetWidth(theme_t * theme, int item, int state, int *minw, int *maxw)
{

    component_t *component = &theme->components[GET_COMPONENT(item)];
    widget_t *widget = component->widgets[GET_WIDGET(item)];

    if (minw)
	*minw = widget->width.min;
    if (maxw)
	*maxw = widget->width.max;
}

static void
getWidgetHeight(theme_t * theme, int item, int state, int *minh, int *maxh)
{

    component_t *component = &theme->components[GET_COMPONENT(item)];
    widget_t *widget = component->widgets[GET_WIDGET(item)];

    if (minh)
	*minh = widget->height.min;
    if (maxh)
	*maxh = widget->height.max;
}

static void
getWidgetColor(theme_t * theme, int item, int s,
	       GR_COLOR * fgcolor, GR_COLOR * bgcolor)
{

    component_t *component = &theme->components[GET_COMPONENT(item)];
    widget_t *widget = component->widgets[GET_WIDGET(item)];

    widget_state_t *state = &widget->states[s];

    /* If the state isn't set, then default to active (active must exist) */
    if (!state->flags)
	state = &widget->states[STATE_ACTIVE];

    if (bgcolor)
	*bgcolor = state->bgcolor;
    if (fgcolor)
	*fgcolor = state->fgcolor;
}

static void
drawWidget(GR_WINDOW_ID id, GR_GC_ID gc,
	   theme_t * theme, int item, int s, int x, int y, int w, int h)
{

    component_t *component;
    widget_t *widget;
    widget_state_t *state;

    if (!theme) {
	error("Invalid theme specified to drawWidget()\n");
	return;
    }

    if (GET_COMPONENT(item) >= THEME_COMPONENT_COUNT) {
	warning("Component %d does not exist in the current theme\n",
		GET_COMPONENT(item));
	return;
    }

    component = &theme->components[GET_COMPONENT(item)];

    if (!component->widgets) {
	warning("The current theme has no active widgets\n",
		GET_COMPONENT(item));
	return;
    }

    widget = component->widgets[GET_WIDGET(item)];

    if (!widget) {
	warning("Widget %d does not exist in the current theme\n", item);
	return;
    }

    state = &widget->states[s];

    /* If the state isn't set, then default to active (active must exist) */
    if (!state->flags)
	state = &widget->states[STATE_ACTIVE];

    if (state->image && !state->imageid) {

	GR_IMAGE_ID image;
	GR_IMAGE_INFO info;

	char *filename =
	    alloca(strlen(wm_getDir(DESKTOP_THEMEDIR)) +
		   strlen(state->image) + 2);
	sprintf(filename, "%s/%s", wm_getDir(DESKTOP_THEMEDIR), state->image);


	image = GrLoadImageFromFile(filename, 0);

	if (image) {

	    GrGetImageInfo(image, &info);
	    state->imageWidth = info.width;
	    state->imageHeight = info.height;

	    state->imageid =
		GrNewPixmap(state->imageWidth, state->imageHeight, 0);
	    if (state->imageid) {
		GR_GC_ID pgc = GrNewGC();
		GrDrawImageToFit(state->imageid, pgc, 0, 0, -1, -1, image);
		GrDestroyGC(pgc);
		GrFreeImage(image);
	    }
	}
    }

    if (!state->imageid) {
	if (w == -1)
	    w = widget->width.min;
	if (h == -1)
	    h = widget->height.min;
    } else {
	if (w == -1)
	    w = state->imageWidth;
	if (h == -1)
	    h = state->imageHeight;
    }

    /* If no image is available, go for the default colors */

    if (!state->imageid) {
	GrSetGCForeground(gc, state->bgcolor);
	GrFillRect(id, gc, x, y, w, h);
    } else {
	int xpos = x, ypos = y;
	int left = w;

	/* FIXME:  Handle both horizontal and vertical tiling */

	while (left) {
	    int cwidth;

	    if (left < state->imageWidth)
		cwidth = left;
	    else
		cwidth = state->imageWidth;

	    GrCopyArea(id, gc, xpos, ypos, cwidth, h, state->imageid, 0, 0,
		       MWROP_SRCCOPY);

	    xpos += cwidth;
	    left -= cwidth;
	}
    }
}

/* GLOBAL FUNCTIONS */

void
set_activeTheme(theme_t * theme)
{
    activeTheme = theme;
}
inline theme_t *
get_activeTheme(void)
{
    return (activeTheme);
}


static void
drawTitlebar(theme_t * theme, GR_DRAW_ID id, GR_GC_ID gc, int w, int h,
	     GR_CHAR * title, GR_BOOL active, GR_WM_PROPS props)
{


    int themeState = active ? STATE_ACTIVE : STATE_INACTIVE;
    int x = 0, y = 0;

    int captionWidth = 0;
    int closeWidth = 0;
    int paddingWidth = 0;

    /* Calculate the various sizes */

    if (title != 0) {
	int strw, strh, strb;
	int minw, maxw;

	getWidgetWidth(theme, TITLEBAR_WIDGET_CAPTION, themeState, &minw,
		       &maxw);

	GrSetGCFont(gc, theme->font);
	GrGetGCTextSize(gc, title, -1, GR_TFTOP, &strw, &strh, &strb);

	/* Now, figure out the size */
	if (strw < minw)
	    captionWidth = minw;
	else if (strw > maxw)
	    captionWidth = maxw;
	else
	    captionWidth = strw + 5;	/* Padding */
    }

    /* Close box */
    if ((props & GR_WM_PROPS_CLOSEBOX) == GR_WM_PROPS_CLOSEBOX) {
	int minw, maxw;

	getWidgetWidth(theme, TITLEBAR_WIDGET_CLOSEBUTTON, themeState, &minw,
		       &maxw);
	closeWidth = minw;
    }

    paddingWidth = w - (captionWidth + closeWidth);

    if (title != 0) {
	GR_COLOR fgcolor;

	getWidgetColor(theme, TITLEBAR_WIDGET_CAPTION, themeState, &fgcolor,
		       0);
	drawWidget(id, gc, theme, TITLEBAR_WIDGET_CAPTION, themeState, x, y,
		   captionWidth, -1);

	GrSetGCForeground(gc, fgcolor);
	GrSetGCUseBackground(gc, GR_FALSE);

	/* FIXME:  Text pos should be configurable */
	GrText(id, gc, x + 4, y, title, -1, GR_TFASCII | GR_TFTOP);
    }

    if (props & GR_WM_PROPS_CLOSEBOX)
	drawWidget(id, gc, theme, TITLEBAR_WIDGET_CLOSEBUTTON, themeState,
		   x + w - closeWidth, y, closeWidth, -1);

    if (paddingWidth)
	drawWidget(id, gc, theme, TITLEBAR_WIDGET_TITLEBAR, themeState,
		   x + captionWidth, y, paddingWidth, -1);
}

static void
drawBorder(theme_t * theme, GR_DRAW_ID id, GR_GC_ID gc, int cw, int ch,
	   GR_BOOL active, GR_WM_PROPS style, int item)
{

    int themeState = active ? STATE_ACTIVE : STATE_INACTIVE;
    int x, y, w, h;
    int tbarh;

    int widget = COMPONENT_WIDGET(item, 0);

    if ((style & GR_WM_PROPS_CAPTION) == GR_WM_PROPS_CAPTION)
	getWidgetHeight(theme, TITLEBAR_WIDGET_CAPTION, 0, 0, &tbarh);
    else
	tbarh = 0;

    switch (item) {

    case COMPONENT_TOP:
	x = 0;
	y = 0;
	w = cw - (theme->client.left - theme->client.right);
	h = theme->client.top;
	break;

    case COMPONENT_LEFT:
	x = 0;
	y = tbarh;
	w = theme->client.left;
	h = ch - tbarh;
	break;

    case COMPONENT_RIGHT:
	x = cw - theme->client.right;
	y = tbarh;
	w = theme->client.right;
	h = ch - tbarh;
	break;

    case COMPONENT_BOTTOM:
	x = theme->client.left;
	y = ch - theme->client.right;
	w = cw - (theme->client.left - theme->client.right);
	h = theme->client.bottom;
	break;
    }

    drawWidget(id, gc, theme, widget, themeState, x, y, w, h);
}

void
themeDrawContainer(GR_DRAW_ID id, int w, int h, GR_CHAR * title,
		   GR_BOOL active, GR_WM_PROPS props, theme_t * theme)
{

    GR_GC_ID gc;

    gc = GrNewGC();

    /* first draw the titlebar (if we have it) */

    if ((props & GR_WM_PROPS_CAPTION) == GR_WM_PROPS_CAPTION) {
	drawTitlebar(theme, id, gc, w, h, title, active, props);
    } else {
	drawBorder(theme, id, gc, w, h, active, props, COMPONENT_TOP);
    }

    /* Now draw the other three borders */
    if ((props & GR_WM_PROPS_BORDER) == GR_WM_PROPS_BORDER) {

	drawBorder(theme, id, gc, w, h, active, props, COMPONENT_LEFT);

	drawBorder(theme, id, gc, w, h, active, props, COMPONENT_RIGHT);

	drawBorder(theme, id, gc, w, h, active, props, COMPONENT_BOTTOM);
    }

    GrDestroyGC(gc);
}

void
themeContainerSize(theme_t * theme,
		   GR_WM_PROPS style, int clientw, int clienth,
		   int *xoff, int *yoff, int *containerw, int *containerh)
{

    int tbarh;

    if ((style & GR_WM_PROPS_CAPTION) == GR_WM_PROPS_CAPTION)
	getWidgetHeight(theme, TITLEBAR_WIDGET_CAPTION, 0, 0, &tbarh);
    else
	tbarh = theme->client.top;

    if ((style & GR_WM_PROPS_BORDER) == GR_WM_PROPS_BORDER) {
	*containerw = clientw + theme->client.left + theme->client.right;
	*containerh = clienth + tbarh + theme->client.bottom;
	*xoff = theme->client.left;
    } else {
	*containerw = clientw;
	*containerh = clienth + tbarh;
	*xoff = 0;
    }

    *yoff = tbarh;
}

void
themeClientSize(theme_t * theme, GR_WM_PROPS style, int containerw,
		int containerh, int *clientw, int *clienth)
{

    int tbarh;

    if ((style & GR_WM_PROPS_CAPTION) == GR_WM_PROPS_CAPTION)
	getWidgetHeight(theme, TITLEBAR_WIDGET_CAPTION, 0, 0, &tbarh);
    else
	tbarh = theme->client.top;

    if ((style & GR_WM_PROPS_BORDER) == GR_WM_PROPS_BORDER) {
	*clientw = containerw - (theme->client.left + theme->client.right);
	*clienth = containerh - (tbarh + theme->client.bottom);
    } else {
	*clientw = containerw;
	*clienth = containerh - tbarh;
    }
}

static void
themeGetWidgetOffset(theme_t * theme, GR_WM_PROPS style,
		     int index, int w, int h, int *x, int *y)
{

    int cl, cr, ct, cb;
    component_t *component = &theme->components[GET_COMPONENT(index)];
    widget_t *widget = component->widgets[GET_WIDGET(index)];

    if ((style & GR_WM_PROPS_BORDER) == GR_WM_PROPS_BORDER) {
	cr = w - theme->client.right;
	cl = theme->client.left;

	if ((style & GR_WM_PROPS_CAPTION) == GR_WM_PROPS_CAPTION)
	    ct = 0;
	else
	    ct = theme->client.top;

	cb = h - theme->client.bottom;
    } else {
	cl = 0;
	cr = w;
	ct = 0;
	cb = h;
    }

    switch (index) {
    case TITLEBAR_WIDGET_CLOSEBUTTON:
	*x = cr - widget->width.min;
	*y = ct;
	break;
    }
}

static GR_BOOL
PtInRect(GR_RECT * prc, GR_SIZE x, GR_SIZE y)
{
    return (x >= prc->x && x < (prc->x + prc->width) &&
	    y >= prc->y && y < (prc->y + prc->height));
}

int
widgetCheckBounds(theme_t * theme, GR_WM_PROPS style, int index, int x, int y,
		  int w, int h)
{

    component_t *component = &theme->components[GET_COMPONENT(index)];
    widget_t *widget = component->widgets[GET_WIDGET(index)];

    int xpos, ypos;

    themeGetWidgetOffset(theme, style, index, w, h, &xpos, &ypos);

    if (widget->hotspot.w || widget->hotspot.h) {
	GR_RECT r;
	r.x = xpos + widget->hotspot.x;
	r.y = ypos + widget->hotspot.y;
	r.width = widget->hotspot.w;
	r.height = widget->hotspot.h;

	return (PtInRect(&r, x, y));
    }

    return (0);
}

int
themeCheckBounds(theme_t * theme, GR_WM_PROPS style, int index, int x, int y,
		 int w, int h)
{

    GR_RECT r;
    int th;

    switch (index) {
    case COMPONENT_TITLEBAR:
	getWidgetHeight(theme, TITLEBAR_WIDGET_CAPTION, 0, 0, &th);
	r.x = 0;
	r.y = 0;
	r.width = w;
	r.height = th;
	break;

    default:
	return (0);
    }
    return (PtInRect(&r, x, y));
}
