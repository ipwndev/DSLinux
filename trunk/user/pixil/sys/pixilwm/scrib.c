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
#include <unistd.h>
#include "nano-X.h"
#include "apps.h"
#include "nanowm.h"

#define APPICON		"nxscribble.gif"

static GR_IMAGE_ID imageid;
static int appvisible = 0;

static void scribble_wndproc(win * window, GR_EVENT * ep);

void
scribble_create(void)
{
    GR_WINDOW_ID wid;
    APP *p;
    GR_SCREEN_INFO si;

    GrGetScreenInfo(&si);
    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
			si.ws_width - 20, si.ws_height + 2, 20, 20,
			COLOR_TASKBAR);
    add_window(wid, GR_ROOT_WINDOW_ID, 0, scribble_wndproc);

    p = find_app_flags(FL_SCRIBBLE);
    if (p)
	p->m_icon_wid = wid;

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);
}

static void
scribble_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    GR_GC_ID gc = GrNewGC();

    if (!imageid) {
	char path[256];

	sprintf(path, "%s/%s", get_desktop_config(DESKTOP_IMAGEDIR), APPICON);
	imageid = GrLoadImageFromFile(path, 0);
    }

    GrDrawImageToFit(ep->wid, gc, 0, 0, -1, -1, imageid);

    GrDestroyGC(gc);
}

static void
scribble_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
    APP *p = find_app_flags(FL_SCRIBBLE);

    if (!p) {
	printf("Unable to find the scribble app!\n");
	return;
    }

    if (is_app_running(p)) {
	if (appvisible) {
	    hide_application(p);
	    appvisible = 0;
	} else {
	    show_application(p);
	    appvisible = 1;
	}
    } else {
	launch_application(p);
	appvisible = 1;
    }

#ifdef NOTUSED
    if (p->m_process_id && p->m_container_wid) {

	appvisible = !appvisible;

	if (appvisible) {
	    GrMapWindow(p->m_container_wid);
	    GrRaiseWindow(p->m_container_wid);
	} else {
	    GrUnmapWindow(p->m_container_wid);
	}
    } else if (!(p->m_process_id = vfork())) {
	printf("execing %s\n", p->m_exec);
	execlp(p->m_exec, p->m_exec, 0);
	exit(0);
    } else {
	g_last_window = p->m_icon_wid;
	LastLaunchedApp = p;
	appvisible = 1;
    }
#endif

}

static void
scribble_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	scribble_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	scribble_buttondown(window, &ep->button);
	break;
    }
}
