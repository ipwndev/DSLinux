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
#include <par/par.h>

#define MWINCLUDECOLORS

#include <nano-X.h>
#include "apps.h"
#include "nanowm.h"
#include "config.h"

typedef struct input_struct
{
    char *title;
    int visible;
    APP *app;
    struct input_struct *next;
}
input_t;

static input_t *inputList = 0;
static input_t *defaultInput = 0;

static GR_WINDOW_ID defaultWid;
static GR_WINDOW_ID menuWid;
static GR_WINDOW_ID menubarWid;

static int menubarVisible = 0;

/* This may be a very stupid idea */

static int defaultWidth = 0;
static int defaultHeight = 0;

static void inputProc(win * window, GR_EVENT * ep);

#ifdef NOTUSED
static input_t *
input_findByWindow(GR_WINDOW_ID wid)
{

    input_t *input = inputList;
    while (input) {
	if (input->wid == wid)
	    return (input);
	input = input->next;
    }

    return (0);
}
#endif

static int
loadInput(db_handle * db, char *input)
{

    par_input_t istruct;
    app_info_t ainfo;

    input_t *ptr;

    bzero(&istruct, sizeof(par_input_t));
    bzero(&ainfo, sizeof(app_info_t));

    par_getScreentopInput(db, input, &istruct);

    /* These are absolutely required */

    if (par_getAppTitle(db, istruct.app, ainfo.title, sizeof(ainfo.title)) <
	0) {
	error("Couldn't find input '%s'.\n", istruct.app);
	return (-1);
    }

    if (par_getAppPath(db, istruct.app, ainfo.path, sizeof(ainfo.path)) < 0) {
	error("Couldn't get a path for input '%s'.\n", istruct.app);
	return (-1);
    }

    /* These are not required, but they are nice to have */

    par_getAppIcon(db, istruct.app, ainfo.icon, sizeof(ainfo.icon));
    par_getAppWorkDir(db, istruct.app, ainfo.workdir, sizeof(ainfo.workdir));
    par_getAppArgs(db, istruct.app, ainfo.args, sizeof(ainfo.args));

    ainfo.flags = FL_INPUT;

    if (!inputList) {
	ptr = inputList = (input_t *) calloc(sizeof(input_t), 1);
    } else {
	input_t *head = inputList;
	while (head->next)
	    head = head->next;
	ptr = head->next = (input_t *) calloc(sizeof(input_t), 1);
    }

    if (!ptr)
	return (-1);
    ptr->next = 0;

    /* Add the application to the list */
    if (apps_add_application(istruct.app, &ainfo, &ptr->app) != 0)
	return (-1);
    if (!ptr->app)
	return (-1);


    /* Set up the icon */
    if (strlen(ainfo.icon)) {
	ptr->app->m_icon_iid = loadIconImage(ainfo.icon, ainfo.path, 1);
    } else {
	ptr->app->m_icon_iid = loadIconImage(istruct.icon, ainfo.path, 1);
    }

    if (!defaultInput)
	defaultInput = ptr;
    return (0);
}

int
nxLoadInputs(db_handle * db)
{

    int i;

    int count;
    itemlist_t *inputs;

    count = par_getScreentopInputList(db, &inputs);
    if (count <= 0)
	return (-1);

    for (i = 0; i < count; i++) {
	char input[25];
	int size = sizeof(input);

	bzero(input, sizeof(input));
	par_getListItem(inputs, i, input, &size);

	if (loadInput(db, input) == -1) {
	}
    }

    par_freeItemList(inputs);
    return (0);
}

void
nxFreeInputs(void)
{

    input_t *head = inputList;

    while (head) {
	input_t *next = head->next;

	if (head->title)
	    free(head->title);

	free(head);
	head = next;
    }
}

static void
drawDefaultButton(GR_WINDOW_ID wid)
{

    int xpos, ypos;

    GR_WINDOW_INFO info;
    GR_IMAGE_INFO iinfo;

    GR_GC_ID gc = GrNewGC();

    GrGetWindowInfo(wid, &info);
    GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));

    GrFillRect(wid, gc, 0, 0, info.width, info.height);

    /* Draw the default image in here */
    if (!defaultInput)
	return;
    if (!defaultInput->app || !defaultInput->app->m_icon_iid)
	return;

    GrGetImageInfo(defaultInput->app->m_icon_iid, &iinfo);

    xpos = (info.width - iinfo.width) / 2;
    ypos = (info.height - iinfo.height) / 2;

    GrDrawImageToFit(wid, gc, xpos, ypos, -1, -1,
		     defaultInput->app->m_icon_iid);

    /* Draw a black border around it for looks */

    GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
    GrRect(wid, gc, 0, 0, info.width, info.height - 2);

    GrDestroyGC(gc);
}

static void
drawMenuButton(win * window, GR_EVENT_EXPOSURE * e)
{

    GR_POINT points[3];

    GR_WINDOW_INFO info;
    GR_GC_ID gc = GrNewGC();

    GrGetWindowInfo(e->wid, &info);

    /* This will eventually be an image, but for now, just put a caret.. :) */

    GrSetGCBackground(gc, wm_getColor(WM_TASKBAR));
    GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));

    points[0].x = 3;
    points[0].y = 11;

    points[1].x = 7;
    points[1].y = 6;

    points[2].x = 12;
    points[2].y = 11;

    GrFillPoly(e->wid, gc, 3, points);

    GrRect(e->wid, gc, 0, 0, info.width, info.height - 2);

    GrDestroyGC(gc);
}

static void
drawMenu(win * window, GR_EVENT_EXPOSURE * e)
{

    input_t *head = inputList;
    int x = 0, y = 0;

    GR_WINDOW_INFO info;
    GR_IMAGE_INFO iinfo;

    GR_GC_ID gc = GrNewGC();

    GrGetWindowInfo(e->wid, &info);
    GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));

    while (head) {
	int ypos = 0;
	if (!head->app || !head->app->m_icon_iid) {
	    head = head->next;
	    continue;
	}

	GrGetImageInfo(head->app->m_icon_iid, &iinfo);

	x = (info.width - iinfo.width) / 2;
	ypos = y + (20 - iinfo.height) / 2;

	GrDrawImageToFit(e->wid, gc, x, ypos, -1, -1, head->app->m_icon_iid);
	/* GrRect(e->wid, gc, 0, y, info.width, 20); */

	y += 19;
	head = head->next;
    }


    /* GrRect(e->wid, gc, 0, 0, info.width, info.height); */

    GrDestroyGC(gc);
}

void
inputResizeWorkspace(APP * app)
{
    GR_WINDOW_INFO info;
    if (!app)
	return;

    /* Figure out the size of the window */
    GrGetWindowInfo(app->m_container_wid, &info);

    setWorkspace(defaultWidth, defaultHeight - info.height + 1);
}

void
inputResetWorkspace(void)
{
    int cw, ch;
    getWorkspace(&cw, &ch);

    if ((defaultWidth == cw) && (defaultHeight == ch))
	return;

    setWorkspace(defaultWidth, defaultHeight);
}

int
isInputVisible(void)
{
    input_t *input = inputList;

    while (input) {
	if (input->visible)
	    return (1);
	input = input->next;
    }

    return (0);
}

static void
fireInput(input_t * input)
{

    APP *p = 0;
    if (!input)
	return;

    p = input->app;

    if (!p)
	return;

    if (is_app_running(p)) {
	if (input->visible) {
	    hide_application(p);
	    input->visible = 0;
	} else {
	    show_application(p);
	    input->visible = 1;
	}
    } else {
	launch_application(p);
	input->visible = 1;
    }

    if (input->visible)
	inputResizeWorkspace(input->app);
    else
	setWorkspace(defaultWidth, defaultHeight);
}

static void
launchDefault(win * window, GR_EVENT_BUTTON * e)
{
    fireInput(defaultInput);
}

static void
launchInput(win * window, GR_EVENT_BUTTON * e)
{

    input_t *head = inputList;

    int index = e->y / 20;
    int i;

    for (i = 0; i < (index); i++) {
	if (!head)
	    return;
	head = head->next;
    }

    if (head != defaultInput && defaultInput) {

	if (defaultInput->visible)
	    hide_application(defaultInput->app);

	defaultInput->visible = 0;
    }

    /* This is the new default! */
    fireInput(head);

    defaultInput = head;
    drawDefaultButton(defaultWid);
}

/* Called when an input is killed */

void
killInput(APP * app)
{

    int state = 0;
    input_t *head = inputList;

    /* Try to figure out which input this was for */
    while (head) {
	if (head->app == app) {
	    if (head->visible)
		head->visible = 0;
	} else if (head->visible)
	    state = 1;

	head = head->next;
    }

    if (!state)
	setWorkspace(defaultWidth, defaultHeight);
}

/* Hide all the inputs */

void
hideInputs(void)
{

    input_t *head = inputList;

    while (head) {
	if (!head->app) {
	    head = head->next;
	    continue;
	}

	if (head->visible)
	    hide_application(head->app);

	head->visible = 0;

	head = head->next;
    }
}

/* New fangled way to set things up.  We have two buttons - One with */
/* the most current input and one that will bring up the menu        */

void
createInputButton(void)
{

    int count = 0;

    input_t *head = inputList;

    GR_SCREEN_INFO si;
    int x, y;

    GrGetScreenInfo(&si);

    getWorkspace(&defaultWidth, &defaultHeight);

    x = si.ws_width - 46;
    y = si.ws_height + 2;

    while (head) {
	count++;
	head = head->next;
    }

    if (!count) {
	return;
    }

    /* Make the two buttons */

    defaultWid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL,
			       GR_ROOT_WINDOW_ID,
			       x, y, 32, 20, wm_getColor(WM_TASKBAR));

    menuWid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL,
			    GR_ROOT_WINDOW_ID,
			    x + 33, y, 13, 20, wm_getColor(WM_TASKBAR));


    menubarWid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL,
			       GR_ROOT_WINDOW_ID,
			       x + 20, si.ws_height - (20 * count),
			       25, (20 * count) - 1, wm_getColor(WM_TASKBAR));

    GrSelectEvents(menuWid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);
    GrSelectEvents(defaultWid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);
    GrSelectEvents(menubarWid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);

    add_window(defaultWid, GR_ROOT_WINDOW_ID, 0, inputProc);
    add_window(menuWid, GR_ROOT_WINDOW_ID, 0, inputProc);

    add_window(menubarWid, GR_ROOT_WINDOW_ID, 0, inputProc);

    GrMapWindow(defaultWid);
    GrMapWindow(menuWid);
}

#ifdef NOTUSED

/* Currently, the inputs are added as they are found */
/* from right to left (of course this may change)  */

int
createInputs(int start)
{

    int x, y;
    GR_SCREEN_INFO si;

    input_t *head = inputList;

    GrGetScreenInfo(&si);

    x = start;
    y = si.ws_height + 2;

    while (head) {
	if (!head->app || !head->app->m_icon_iid) {
	    head = head->next;
	    continue;
	}

	head->wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL,
				  GR_ROOT_WINDOW_ID,
				  x, y, 30, 15, wm_getColor(WM_TASKBAR));

	add_window(head->wid, GR_ROOT_WINDOW_ID, 0, input_wndproc);

	GrSelectEvents(head->wid,
		       GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);

	GrMapWindow(head->wid);

	x -= 20;
	head = head->next;
    }

    return (x);
}

static void
input_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{

    GR_GC_ID gc;
    input_t *input = input_findByWindow(ep->wid);

    if (!input)
	return;
    if (!input->app || !input->app->m_icon_iid)
	return;

    gc = GrNewGC();
    GrDrawImageToFit(ep->wid, gc, 0, 0, -1, -1, input->app->m_icon_iid);
    GrDestroyGC(gc);
}

static void
input_buttondown(win * window, GR_EVENT_BUTTON * ep)
{

    APP *p = 0;
    input_t *input = input_findByWindow(ep->wid);
    if (!input)
	return;

    p = input->app;

    if (!p) {
	return;
    }

    if (is_app_running(p)) {
	if (input->visible) {
	    hide_application(p);
	    input->visible = 0;
	} else {
	    show_application(p);
	    input->visible = 1;
	}
    } else {
	launch_application(p);
	input->visible = 1;
    }
}

#endif

static void
inputProc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {

    case GR_EVENT_TYPE_EXPOSURE:
	if (ep->exposure.wid == defaultWid)
	    drawDefaultButton(ep->exposure.wid);

	if (ep->exposure.wid == menuWid)
	    drawMenuButton(window, &ep->exposure);

	if (ep->exposure.wid == menubarWid)
	    drawMenu(window, &ep->exposure);

	break;

    case GR_EVENT_TYPE_BUTTON_DOWN:

	if (ep->button.wid == menuWid) {

	    if (menubarVisible) {
		GrUnmapWindow(menubarWid);
		menubarVisible = 0;
	    } else {
		GrMapWindow(menubarWid);
		GrRaiseWindow(menubarWid);
		menubarVisible = 1;
	    }
	}

	if (ep->button.wid == defaultWid) {

	    launchDefault(window, &ep->button);
	    if (menubarVisible) {
		GrUnmapWindow(menubarWid);
		menubarVisible = 0;
	    }
	}

	if (ep->button.wid == menubarWid) {
	    launchInput(window, &ep->button);
	    menubarVisible = 0;
	    GrUnmapWindow(menubarWid);
	}

	break;
    }
}
