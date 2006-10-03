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
#include "nano-X.h"
#include "apps.h"
#include "nanowm.h"
#include "categories.h"

#include "config.h"

#define HOMEICON "home.gif"

static GR_BOOL g_clear_window = 0;

static GR_IMAGE_ID imageid;

static void home_wndproc(win * window, GR_EVENT * ep);

void
home_create(void)
{
    GR_WINDOW_ID wid;
    GR_SCREEN_INFO si;

    GrGetScreenInfo(&si);
    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
			si.ws_width - 62, si.ws_height + 4, 13, 14,
			wm_getColor(WM_TASKBAR));
    add_window(wid, GR_ROOT_WINDOW_ID, 0, home_wndproc);

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);
}

static void
home_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    GR_GC_ID gc = GrNewGC();

    if (!imageid)
	imageid = loadIconImage(HOMEICON, 0, 0);

    GrDrawImageToFit(ep->wid, gc, 0, 0, -1, -1, imageid);
    GrDestroyGC(gc);
}

static void
home_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
    if (g_clear_window && g_last_app) {

	if (!g_last_app->m_container_wid)
	    return;
	container_activate(g_last_app->m_container_wid);

	g_clear_window = 0;

	/* Reset the workspace.  This is a major hack, but the best we can do */
	inputResetWorkspace();
	return;
    }

    /* Make sure we hide all the inputs */
    hideInputs();

#ifdef VIRTUAL_WINDOWS
    app_hide_windows(category_getCurrentWin());
#else
    app_hide_windows();
#endif

    g_clear_window = 1;
}

static void
home_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	home_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	home_buttondown(window, &ep->button);
	break;
    }
}
