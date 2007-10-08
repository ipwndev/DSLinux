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
#include <unistd.h>

#include "apps.h"
#include "nanowm.h"
#include "categories.h"
#include "config.h"

#define XOFFSET 20
#define YOFFSET 20
#define XSPACING (ICONWIDTH+20)
#define YSPACING (ICONHEIGHT+ICONTEXTHEIGHT+20)

#define DEFAULT_ICON	"generic_app.gif"

GR_IMAGE_ID
loadIconImage(char *filename, char *app, int useDefault)
{

    GR_IMAGE_ID ret;

    char *f = 0;
    char *d = 0;
    char *basename = 0;

    if (filename && filename[0] != 0x0) {

	/* Append the path if nessesary */

	if (filename[0] != '/') {
	    f = alloca(strlen(wm_getDir(DESKTOP_IMAGEDIR)) +
		       strlen(filename) + 2);
	    if (!f)
		return (0);

	    sprintf(f, "%s/%s", wm_getDir(DESKTOP_IMAGEDIR), filename);

	    /* Check if the file exists */
	    if (access(f, F_OK) != 0) {
		warning("image '%s' does not exist.\n", f);
		goto useDefault;
	    }
	    ret = GrLoadImageFromFile(f, 0);
	    if (ret == 0)
		goto useDefault;
	    return ret;
	}

	if (access(filename, F_OK) != 0) {
	    warning("image '%s' does not exist.\n", filename);
	    goto useDefault;
	}

	/* Use the absolute path that was passed in */
	error("returning GrLoadImageFromFile() 2\n");
	ret = GrLoadImageFromFile(filename, 0);
	if (ret == 0)
	    goto useDefault;
	return ret;
    }

    /* Otherwise, we will use the basename of the application */

    if (!app)
	return (0);

    basename = strrchr(app, '/');
    if (!basename)
	basename = app;
    else
	basename = basename + 1;

    f = alloca(strlen(wm_getDir(DESKTOP_IMAGEDIR)) + strlen(basename) + 2);
    if (!f)
	return (0);

    sprintf(f, "%s/%s", wm_getDir(DESKTOP_IMAGEDIR), basename);

    if (access(f, F_OK) == 0)
	return (GrLoadImageFromFile(f, 0));
    warning("image '%s' does not exist.\n", filename);

  useDefault:
    if (!useDefault)
	return (0);

    /* If we get this far, then we need use the default icon */
    d = alloca(strlen(wm_getDir(DESKTOP_IMAGEDIR)) + strlen(DEFAULT_ICON) +
	       2);
    if (!d)
	return (0);

    sprintf(d, "%s/%s", wm_getDir(DESKTOP_IMAGEDIR), DEFAULT_ICON);

    ret = GrLoadImageFromFile(d, 0);

    if (!ret)
	error("Unable to load a default icon for app '%s'.\n", app);

    return (ret);
}

void
hide_icon_list(void)
{
    int i;

    category_t *cat = (category_t *) category_getCurrent();

    if (!cat)
	return;
    for (i = 0; i < cat->count; i++)
	GrUnmapWindow(cat->apps[i]->m_icon_wid);
}

/* Where size is the maximum width of the label (in pixels) */

void
drawLabel(GR_GC_ID gc, char *label, int x, int y, int icon)
{

    int dy = y;
    int size = icon + 20;

    char *p = alloca(strlen(label) + 1);
    if (!p)
	return;

    strcpy(p, label);

    GrSetGCUseBackground(gc, GR_FALSE);
    while (1) {
	int w, h, b;
	char *n;

	GrGetGCTextSize(gc, p, -1, GR_TFASCII | GR_TFTOP, &w, &h, &b);

	/* if it fits, then go with it */

	if (w <= size) {
	    int dx = x + ((icon - w) / 2);
	    GrText(GR_ROOT_WINDOW_ID, gc, dx, dy, p, -1,
		   GR_TFASCII | GR_TFTOP);
	    GrSetGCUseBackground(gc, GR_TRUE);
	    return;
	}

	/* Look for a break in the text */
	n = strchr(p, ' ');
	if (n)
	    *n = 0;

	GrGetGCTextSize(gc, p, -1, GR_TFASCII | GR_TFTOP, &w, &h, &b);

	if (w < size) {
	    int dx = x + ((icon - w) / 2);

	    GrText(GR_ROOT_WINDOW_ID, gc, dx, dy, p, -1,
		   GR_TFASCII | GR_TFTOP);

	    dy += h;
	    p = n + 1;
	} else {		/* Truncate it */
	    int avg = w / strlen(p);
	    int delta = (w - size) / avg;
	    int pos = strlen(p) - (delta + 4);
	    int dx, i;

	    /* We need to cut off delta + 4 chars */
	    for (i = pos; i < pos + 3; i++)
		p[i] = '.';

	    p[i] = 0;

	    GrGetGCTextSize(gc, p, -1, GR_TFASCII | GR_TFTOP, &w, &h, &b);
	    dx = x + ((icon - w) / 2);

	    GrText(GR_ROOT_WINDOW_ID, gc, dx, dy, p, -1,
		   GR_TFASCII | GR_TFTOP);
	    GrSetGCUseBackground(gc, GR_TRUE);
	    return;
	}
    }
}

/* The passed GC is appropriately clipped */

void
draw_current_iconlist(GR_REGION_ID r)
{
    int i;
    GR_GC_ID gc;

    GR_IMAGE_INFO iinfo;

    int xpos = XOFFSET;
    int ypos = YOFFSET;

    GR_FONT_ID fnt = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);
    GR_SCREEN_INFO si;

    category_t *cat = (category_t *) category_getCurrent();

    if (!cat)
	return;

    gc = GrNewGC();
    GrGetScreenInfo(&si);

    /* Set the clip region */
    GrSetGCRegion(gc, r);

    GrSetGCForeground(gc, wm_getColor(WM_ICONTEXT));

    /* If we have a background color for the icon, then use it, otherwise */
    /* don't use the background */

    if (wm_getColor(WM_ICONCOLOR) >= 0)
	GrSetGCBackground(gc, wm_getColor(WM_ICONCOLOR));
    else
	GrSetGCUseBackground(gc, 0);

    GrSetGCFont(gc, fnt);

    for (i = 0; i < cat->count; i++) {
	APP *cur = cat->apps[i];

	if (!cur->m_icon_iid)
	    continue;
	if (cur->m_flags & FL_INPUT)
	    continue;

	GrMoveWindow(cur->m_icon_wid, xpos, ypos);
	GrMapWindow(cur->m_icon_wid);

	GrGetImageInfo(cur->m_icon_iid, &iinfo);
	if ((iinfo.width > ICONWIDTH) || (iinfo.height > ICONHEIGHT)) {
	    GrDrawImageToFit(GR_ROOT_WINDOW_ID, gc, xpos, ypos,
			     ICONWIDTH, ICONHEIGHT, cur->m_icon_iid);
	} else {
	    GrDrawImageToFit(GR_ROOT_WINDOW_ID, gc,
			     // xpos, ypos, 
			     xpos + ((ICONWIDTH - iinfo.width) / 2),
			     ypos + (ICONHEIGHT - iinfo.height),
			     // ICONWIDTH, ICONHEIGHT, 
			     iinfo.width, iinfo.width, cur->m_icon_iid);
	}
	drawLabel(gc, cur->m_icontitle, xpos, ypos + ICONHEIGHT + 5,
		  ICONWIDTH);

	if (xpos + XSPACING + XSPACING > si.ws_width) {
	    xpos = XOFFSET;
	    ypos += YSPACING;
	} else
	    xpos += XSPACING;
    }

    GrDestroyFont(fnt);
    GrDestroyGC(gc);
}

static void
icon_handler(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_BUTTON_DOWN:
	apps_buttondown(window->wid);
	break;
    }
}

GR_WINDOW_ID
create_icon_window()
{
    GR_WINDOW_ID wid = GrNewInputWindow(GR_ROOT_WINDOW_ID, 20, 20,
					ICONWIDTH,
					ICONHEIGHT + ICONTEXTHEIGHT);

    add_window(wid, GR_ROOT_WINDOW_ID, 0, icon_handler);

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);

    return (wid);
}
