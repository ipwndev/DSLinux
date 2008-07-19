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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

#include <unistd.h>

#include "apps.h"
#include "nanowm.h"
#include "categories.h"
#include "config.h"

#ifdef CONFIG_PIXILWM_MENUS
#include "sys_menu.h"
#endif

/* Globals */
APP *g_last_app;		/* The last known application that was started */

/* A local list of applications */
NXLISTHEAD apphead;

static int
searchPath(char *in, char *out, int size, int flags)
{

    char *path;
    char *p, *temp;
    int csize;

    if (!in || !out)
	return (-1);

    if (in[0] == '/') {

	/* Check to see if the file exists */
	if (access(in, flags) != 0)
	    return (-1);
	bzero(out, size);
	csize = (strlen(in) > size ? size : strlen(in));

	strncpy(out, in, csize);
	return (csize);
    }

    if (!(temp = getenv("PATH")))
	return (-1);

    p = path = alloca(strlen(temp) + 1);
    strcpy(path, temp);

    while (p) {
	char *n = strchr(p, ':');
	char *f;

	if (n)
	    *n = 0;
	f = alloca(strlen(p) + strlen(in) + 2);
	if (!f)
	    continue;

	sprintf(f, "%s/%s", p, in);

	if (access(f, flags) == 0) {
	    bzero(out, size);
	    csize = (strlen(in) > size ? size : strlen(f));

	    strncpy(out, f, csize);
	    return (csize);
	}

	if (n)
	    p = n + 1;
	else
	    p = 0;
    }

    return (-1);
}

/* This function takes the passed information and turns it into */
/* something useful We maintain the legacy APP list to avoid    */
/* borking the old launch system.                               */

int
apps_add_application(char *name, app_info_t * app_info, APP ** application)
{

    char filename[512];
    APP *app;
    char *p;

    if (!strlen(app_info->path))
	return (1);

    /* Hmmm... is this really an error, or an annoying warning? */

    if (searchPath(app_info->path, filename, sizeof(filename), X_OK) == -1) {
      //error("Application '%s' does not exist\n", app_info->path);
      return (-1);
    }

    app = nxItemNew(APP);

    if (!app) {
	error("Unable to allocate enough memory\n");
	return (-1);
    }

    strzcpy(app->m_name, name, sizeof(app->m_name));

    /* set application flags */
    app->m_flags = app_info->flags;

    /* Check for virtual window flags */
    app->m_virtual_flags = 0;

    /* set executable path */
    strzcpy(app->m_exec, filename, sizeof(app->m_exec));
    strzcpy(app->m_workdir, app_info->workdir, sizeof(app->m_workdir));

    /* If no working directory was specified, try to develop one from the path */

    if (strlen(app->m_workdir) == 0) {
	char *p = strrchr(filename, '/');
	if (p) {
	    strzcpy(app->m_workdir, filename, (int) (p - filename));
	} else
	    app->m_workdir[0] = 0;
    }

    /* We used to parse the arguments, but that we before we had */
    /* colosseum do most of the heavy lifting                    */

    /* We still keep the legacy code around for backup purposes  */
    /* but to avoid allocating unused memory, we will parse the  */
    /* argc, argv list as each app is executed (see exec.c)      */

    strzcpy(app->m_args, app_info->args, sizeof(app->m_args));

    if (app_info->flags & FL_INPUT) {
	/* Add this application to the main list */
	nxListAdd(&apphead, &app->link);

	/* Save the link for the calling function */
	if (application)
	    *application = app;
	return (0);
    }

    if (strlen(app_info->title))
	strzcpy(app->m_icontitle, app_info->title, sizeof(app->m_icontitle));
    else {
	/* use basename if no title override */
	p = strrchr(app_info->path, '/');
	strzcpy(app->m_icontitle, p ? p + 1 : app_info->path,
		sizeof(app->m_icontitle));
    }

    /* user passed icon, path.gif or default */
    /* This will soon be attacked with some fury */

    app->m_icon_iid = loadIconImage(app_info->icon, filename, 1);

    if (!app->m_icon_iid) {

	warning("Unable to find an icon for '%s'.  Skipping...\n", filename);

	free(app);
	return (-1);		/* Error */
    }

    app->m_process_id = 0;
    app->m_container_wid = 0;

    app->m_icon_wid = create_icon_window();

    /* Add this application to the main list */
    nxListAdd(&apphead, &app->link);

    /* Pass the newly created application back to the calling app */
    if (application)
	*application = app;

    return (0);
}

void
apps_buttondown(GR_WINDOW_ID id)
{
    APP *app;

    if (!(app = find_app_icon(id)))
	return;
    launch_application(app);
}

#ifdef NOTUSED

/* 
 * This hides all of the windows for the given virtual window 
 */


void
app_hide_windows(int virt)
{
    PNXLIST p;

    for (p = apphead.head; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_container_wid && app->m_virtual_wid == virt)
	    if (!(app->m_virtual_flags & VW_STICKY))	/* Check for stickyness */
		container_hide(app->m_container_wid);
    }
}

/*
 * This will map all the active windows for a given virtual
 * window.  
 * FIXME:  We need a flag to indicate those apps that have been 
 * minimized or otherwise hidden 
 */

void
app_show_windows(int virt, int focus)
{
    PNXLIST p;

    for (p = apphead.head; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_container_wid && app->m_virtual_wid == virt) {
	    if (!(app->m_virtual_flags & VW_STICKY))
		GrMapWindow(app->m_container_wid);

	    /* If the new window has a stored focus, set it back again */
#ifdef NOTUSED
	    if (focus) {
		win *window = find_window(app->m_container_wid);
		if (window)
		    if (window->clientid == focus)
			GrSetFocus(window->clientid);
	    }
#endif
	}
    }
}

#else /* VIRTUAL_WINDOWS */

void
app_hide_windows()
{
    PNXLIST p;

    for (p = apphead.head; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);
	if (app->m_container_wid)
	    container_hide(app->m_container_wid);
    }
}

#endif /* VIRTUAL_WINDOWS */

APP *
find_app_pid(int pid)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_process_id == pid)
	    return app;
    }
    return NULL;
}

APP *
find_app_path(char *path)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (!strcmp(path, app->m_exec))
	    return app;
    }
    return NULL;
}

APP *
find_app_flags(int flags)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_flags & flags)
	    return app;
    }
    return NULL;
}

APP *
find_app_icon(GR_WINDOW_ID id)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_icon_wid == id) {
	    return app;
	}
    }
    return NULL;

}

APP *
find_app_container(GR_WINDOW_ID id)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (app->m_container_wid == id) {
	    return app;
	}
    }
    return NULL;

}

APP *
find_app_name(char *name)
{
    PNXLIST p = apphead.head;

    for (; p; p = p->next) {
	APP *app = nxItemAddr(p, APP, link);

	if (strcmp(app->m_name, name) == 0) {
	    return app;
	}
    }
    return NULL;

}


/* JHC 11/14 - Added this to allow apps that aren't registered in the normal */
/* manner to run.  */

APP *
apps_add_onetime(char *path)
{
    char *name;

    app_info_t ainfo;

    bzero(&ainfo, sizeof(ainfo));
    strcpy(ainfo.path, path);
    ainfo.flags = FL_NOICON | FL_ONETIME;

    name = strrchr(path, '/');
    if (!name)
	name = path;
    else
	name = name + 1;

    if (apps_add_application(name, &ainfo, 0) == -1)
	return (0);

    return (find_app_path(path));
}

void
apps_free_memory()
{
    /* Only called when we are going down.  */
    /* Free all the memory we have allocated */

    PNXLIST p = apphead.head;
    PNXLIST t;

    while (p) {
	APP *app = nxItemAddr(p, APP, link);
	t = p->next;

	nxListRemove(&apphead, &app->link);
	free(app);

	p = t;
    }
}

#ifdef CONFIG_PIXILWM_MENUS
int
create_apps_menu(wm_menu_t ** data)
{
    int i, index = 0;
    int count = category_getCount();
    wm_menu_t *ptr;

    count++;
    *data = create_system_menu(count);

    if (!*data)
	return (0);
    else
	ptr = *data;

    for (i = 0; i < count; i++) {
	char name[25];

	wm_menu_t *cmenu = create_categoryMenu(i, name);

	if (cmenu)
	    ADD_SYS_MENU_CMENU(&ptr[index++], name, cmenu);
    }

    ADD_SYS_MENU_END(&ptr[index++]);
    return (index);
}

#endif
