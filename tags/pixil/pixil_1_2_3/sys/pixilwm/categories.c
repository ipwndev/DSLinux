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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MWINCLUDECOLORS

#include <nano-X.h>
#include <par/par.h>

#include "apps.h"
#include "nanowm.h"
#include "categories.h"
#include "config.h"
#include "sys_menu.h"

/* A linked list of categories */
static category_t *categoryList = 0;

/* A pointer to the current list of categories */
static category_t *currentCategory = 0;

static GR_WINDOW_ID cbWindow = 0;
static GR_WINDOW_ID cbPixmap = 0;

int
category_getCount()
{
    int count = 0;

    category_t *ptr = categoryList;
    while (ptr) {
	count++;
	ptr = ptr->next;
    }

    return (count);
}

inline category_t *
category_getCurrent(void)
{
    return (currentCategory);
}

/* Each virtual window is defined by a index */

category_t *
category_getByIndex(int index)
{
    category_t *head = categoryList;

    while (head) {
	if (head->index == index)
	    return (head);
	head = head->next;
    }

    return (0);
}

inline int
category_getCurrentWin(void)
{
    if (currentCategory)
	return (currentCategory->index);
    return (0);
}

category_t *
category_getNext(category_t * cur)
{
    if (!cur)
	return (0);

    if (!cur->next)
	return (categoryList);
    else
	return (cur->next);
}

category_t *
category_getPrev(category_t * cur)
{

    category_t *head = 0;

    if (!cur)
	return (0);

    if (cur->prev)
	return (cur->prev);

    /* We need to find the tail.  A somewhat expensive operation */
    /* But easier than storing the tail */

    head = cur;

    while (head) {
	if (!head->next)
	    return (head);
	head = head->next;
    }

    return (0);
}

/* This just simply sets the current category to the specified index */

int
category_setCurrent(int index)
{

    category_t *head = categoryList;

    while (head) {
	if (head->index == index) {
	    currentCategory = head;
	    return (0);
	}
	head = head->next;
    }

    return (-1);
}

/* What is this used for? */

/*
   Given a name, return the index associated with that catagory
*/

#ifdef NOTUSED
int
get_catagory_index(char *name)
{
    PNXLIST p = catagory_list.head;
    char *dupname,		/* For strdup() of name */
     *dupcat;			/* For strdup() of cat->name */

    for (; p; p = p->next) {
	CATAGORY *cat = nxItemAddr(p, CATAGORY, next);

	if ((dupname = strdup(name)) != NULL &&
	    (dupcat = strdup(cat->name)) != NULL) {
	    /* Make all characters lower case */
	    string_tolower(strlen(dupname), dupname);
	    string_tolower(strlen(dupcat), dupcat);
	    if (!strcmp(dupname, dupcat))
		return (cat->index);

	    free(dupname);
	    free(dupcat);
	} /* end of if */
	else {
	    if (!strcmp(name, cat->name))
		return (cat->index);
	}			/* end of else */
    }

    return (-1);
}
#endif

/* switch_catagory() 
   
   This switches the current catagory to the one specified by index
   Unlike set_current_catagory, this function switches the icon lists,
   hides windows, and redraws the root window 
*/


int
switchCategory(int index)
{
    category_t *new = category_getByIndex(index);
    category_t *old = category_getCurrent();

    if (!new)
	return (-1);
    if (new == old)
	return (0);

    /* First, hide all of the running apps */
    if (old)
	hide_icon_list();

    category_setCurrent(new->index);

    /* Redraw the root window */
    /* This will automatically redraw the new icons */

    redraw_root_window();
    return (0);
}

/* This finds an application in the database, and loads it into memory */
/* Storing the resulting app pointer into the category                 */

static int
loadApplication(db_handle * db, category_t * category, char *app)
{

    /* The database structure */
    app_info_t ainfo;

    if (par_getAppTitle(db, app, ainfo.title, sizeof(ainfo.title)) < 0)
	return (-1);
    if (par_getAppPath(db, app, ainfo.path, sizeof(ainfo.path)) < 0)
	return (-1);

    /* These are not required, but they are nice to have */

    par_getAppIcon(db, app, ainfo.icon, sizeof(ainfo.icon));
    par_getAppWorkDir(db, app, ainfo.workdir, sizeof(ainfo.workdir));
    par_getAppArgs(db, app, ainfo.args, sizeof(ainfo.args));

    /* No flags, this is a normal application */
    ainfo.flags = 0;

    /* Now, add the application */

    if (!apps_add_application(app, &ainfo, &category->apps[category->count])) {
	category->count++;
	return (0);
    } else {
	category->apps[category->count] = 0;
	return (-1);
    }
}

/* Load a category from the database */

static int
loadCategory(db_handle * db, char *category, int index)
{

    char *app;
    char *applist;

    int appcount;

    category_t *ptr = 0;

    /* First, allocate a new category structure */

    if (!categoryList) {
	ptr = categoryList = (category_t *) calloc(sizeof(category_t), 1);
    } else {
	category_t *n = categoryList;
	while (n->next)
	    n = n->next;
	ptr = n->next = (category_t *) calloc(sizeof(category_t), 1);
	ptr->prev = n;
    }

    par_getScreentopCategory(db, category, &ptr->title, &applist);

    if (!applist)
	return (0);

    ptr->index = index;

    /* First, get a count of the number of applications in the list */
    app = applist;
    appcount = par_getStringListCount(applist, ' ');

    /* Allocate enough room for the applications */
    ptr->apps = (APP **) malloc(appcount * sizeof(APP *));

    while (app) {
	char *head = par_parseStringList(&app, ' ');
	if (!head)
	    break;
	loadApplication(db, ptr, head);
    }

    /* Clean up after ourselves */
    free(applist);
    return (0);
}

/* Given a database handle, load all of the categories into memory */

int
nxLoadCategories(db_handle * db)
{

    int count, i;
    itemlist_t *categories;

    count = par_getScreentopCatList(db, &categories);

    if (count <= 0)
	return (-1);

    for (i = 0; i < count; i++) {
	char cat[25];
	int size = sizeof(cat);

	bzero(cat, sizeof(cat));

	par_getListItem(categories, i, cat, &size);
	loadCategory(db, cat, i);
    }

    par_freeItemList(categories);
    return (0);
}

void
nxFreeCategories(void)
{

    category_t *head = categoryList;

    while (head) {
	category_t *ptr = head->next;

	/* Free the title */
	if (head->title)
	    free(head->title);

	/* The apps will be freed elsewhere */
	/* Free the app list */
	if (head->apps)
	    free(head->apps);

	free(head);
	head = ptr;
    }

    categoryList = 0;
    currentCategory = 0;
}

/* This is where we draw the category browser */

void
draw_categoryBrowser(GR_REGION_ID r)
{

    GR_WINDOW_INFO winfo;
    GR_GC_ID gc;

    /* Draw as much as we need from the pixmap */

    GrGetWindowInfo(cbWindow, &winfo);

    gc = GrNewGC();

    /* Set the clip region */
    GrSetGCRegion(gc, r);

    GrCopyArea(GR_ROOT_WINDOW_ID, gc,
	       winfo.x, winfo.y, winfo.width, winfo.height,
	       cbPixmap, 0, 0, MWROP_SRCCOPY);

    /* Now, draw the appropriate category name centered within the box */

    if (currentCategory) {
	int x, y;
	int width, height, baseline;
	GR_FONT_ID font = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);

	GrSetGCForeground(gc, WHITE);
	GrSetGCUseBackground(gc, GR_FALSE);

	GrSetGCFont(gc, font);

	GrGetGCTextSize(gc, currentCategory->title, -1,
			GR_TFASCII | GR_TFTOP, &width, &height, &baseline);

	x = (winfo.width - width) / 2;
	y = (winfo.height - height) / 2;

	GrText(GR_ROOT_WINDOW_ID, gc,
	       winfo.x + x, winfo.y + y, currentCategory->title,
	       -1, GR_TFASCII | GR_TFTOP);

	GrDestroyFont(font);
    }

    GrDestroyGC(gc);
}

static void
cbProc(win * window, GR_EVENT * ep)
{
    GR_WINDOW_INFO winfo;

    category_t *curcat = category_getCurrent();
    category_t *newcat = 0;

    switch (ep->type) {
    case GR_EVENT_TYPE_BUTTON_DOWN:
	GrGetWindowInfo(ep->button.wid, &winfo);

	/* Changed this so if the pointer is on the right half of the window it
	   ** will fetch the next catagory, and if on the left half of the window it
	   ** will fetch the previous catagory --- j webb (01/19/01)
	 */

	if (ep->button.x <= (winfo.width / 2))
	    newcat = category_getPrev(curcat);
	else if (ep->button.x > (winfo.width / 2))
	    newcat = category_getNext(curcat);

	switchCategory(newcat->index);
	break;
    }
}

void
create_categoryBrowser(void)
{

    char file[256];

    GR_SCREEN_INFO si;
    GR_IMAGE_ID bImage = 0;
    GR_GC_ID gc;

    int bWidth = 0, bHeight = 0;

    sprintf(file, "%s/catbrowser.gif", wm_getDir(DESKTOP_IMAGEDIR));

    if (access(file, F_OK) == 0) {
	bImage = GrLoadImageFromFile(file, 0);

	if (bImage) {
	    GR_IMAGE_INFO info;
	    GrGetImageInfo(bImage, &info);
	    bWidth = info.width;
	    bHeight = info.height;
	}
    }

    if (!bImage) {
	bWidth = 80;
	bHeight = 20;
    }

    GrGetScreenInfo(&si);

    /* create an input window */
    cbWindow = GrNewInputWindow(GR_ROOT_WINDOW_ID,
				(si.ws_width - bWidth) / 2, 0,
				bWidth, bHeight);

    cbPixmap = GrNewPixmap(bWidth, bHeight, 0);
    gc = GrNewGC();

    if (bImage) {
	GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
	GrFillRect(cbPixmap, gc, 0, 0, bWidth, bHeight);

	/* Now draw the category browser image or (ack!) drawing */
	GrDrawImageToFit(cbPixmap, gc, 0, 0, -1, -1, bImage);
	GrFreeImage(bImage);
    } else {
	GR_POINT points[3];

	GrSetGCForeground(gc, LTGRAY);
	GrFillRect(cbPixmap, gc, 0, 0, bWidth, bHeight);

	GrSetGCForeground(gc, BLACK);
	GrRect(cbPixmap, gc, 0, 0, bWidth, bHeight);

	points[0].x = 1;
	points[0].y = bHeight / 2;
	points[1].x = 10;
	points[1].y = 1;
	points[2].x = 10;
	points[2].y = bHeight - 2;

	GrFillPoly(cbPixmap, gc, 3, points);

	points[0].x = bWidth - 2;
	points[0].y = bHeight / 2;
	points[1].x = bWidth - 10;
	points[1].y = 1;
	points[2].x = bWidth - 10;
	points[2].y = bHeight - 2;

	GrFillPoly(cbPixmap, gc, 3, points);
    }

    GrDestroyGC(gc);

    /* Finally, set up the input window and leave */
    GrSelectEvents(cbWindow,
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);
    add_window(cbWindow, GR_ROOT_WINDOW_ID, 0, cbProc);

    GrMapWindow(cbWindow);
}

#ifdef CONFIG_PIXILWM_MENUS

wm_menu_t *
create_categoryMenu(int index, char *name)
{

    int i;
    wm_menu_t *ptr;

    category_t *cat = category_getByIndex(index);

    if (!cat)
	return (0);

    ptr = create_system_menu(cat->count + 1);

    if (!ptr)
	return (0);

    for (i = 0; i < cat->count; i++) {
	APP *p = cat->apps[i];
	if (!p)
	    continue;

	ADD_SYS_MENU_APP(&ptr[i], p->m_icontitle, p->m_exec, p->m_icon_iid);
    }

    ADD_SYS_MENU_END(&ptr[i]);
    strcpy(name, cat->title);

    return (ptr);
}

#endif
