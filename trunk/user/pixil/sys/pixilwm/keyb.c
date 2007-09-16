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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "nano-X.h"
#include "apps.h"
#include "nanowm.h"
#define APPICON		"nxkbd.gif"

static GR_IMAGE_ID imageid;
static GR_BOOL appvisible = 0;

static void keyboard_wndproc(win * window, GR_EVENT * ep);

void
keyboard_create()
{
    GR_WINDOW_ID wid;
    APP *p;
    GR_SCREEN_INFO si;

    GrGetScreenInfo(&si);
    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
			si.ws_width - 40, si.ws_height + 2, 20, 20,
			COLOR_TASKBAR);
    add_window(wid, GR_ROOT_WINDOW_ID, 0, keyboard_wndproc);

    p = find_app_flags(FL_KEYBOARD);
    if (p)
	p->m_icon_wid = wid;

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);
}

static void
keyboard_exposure(win * window, GR_EVENT_EXPOSURE * ep)
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
keyboard_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
    APP *p = find_app_flags(FL_KEYBOARD);

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
}

#ifdef NOTUSED

static void
keyboard_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
    char *kbdArgv[4] = { 0 };	/* Arg vector */
    static char kymap[256] = { 0 },	/* Path to the keymap files */
      kyword[32] =
    {
    0};				/* Keyboard keyword */
    APP *p = find_app_flags(FL_KEYBOARD);
    GR_SCREEN_INFO si;		/* Screen info */

    GrGetScreenInfo(&si);
    if (!p)
	return;

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
	/* Set up the keyboard environment for execvp */
	if (!kymap[0] && !kyword[0]) {
	    char cfgpath[255] = { '\0' };	/* Config path */

	    sprintf(cfgpath, "%s", nxGetConfigFile(NX_CONFIG_MAIN));

	    /* Get the path to the keymap files */

	    IniGetString("nxkeyboard", "kymap", "", kymap, sizeof(kymap),
			 cfgpath);
	    IniGetString("nxkeyboard", "kybsize", "", kyword, sizeof(kyword),
			 cfgpath);
	    if (kymap[0] > ' ') {
		memset(cfgpath, 0, sizeof(cfgpath));
		desktop_expand_path(kymap, cfgpath, sizeof(cfgpath));
		memcpy(kymap, cfgpath, strlen(cfgpath));
	    } /* end of if */
	    else {
		/* Get the default path */
		sprintf(kymap, "%s", get_desktop_config(DESKTOP_NXAPPDIR));
		printf("set kymap to %s\n", kymap);
	    }			/* end of else */

	    /* Get the mapset/size variable from the config */
	    if (kyword[0] < ' ') {
		printf("kyword not found\n");
		/* Use the defaults */
		printf("Cols=%d Rows=%d\n", si.cols, si.rows);
		if (si.rows <= 240)
		    sprintf(kyword, "sml");
		else if (si.rows <= 480)
		    sprintf(kyword, "mid");
		else
		    sprintf(kyword, "big");

		printf("kyword set to %s\n", kyword);
	    }			/* end of if */
	}

	/* end of if */
	/* Put the name of the executable in kbdArgv[0] */
	if ((kbdArgv[0] =
	     (char *) calloc(sizeof(char), strlen(p->m_exec) + 1)) == NULL
	    || memcpy(kbdArgv[0], p->m_exec, strlen(p->m_exec)) == NULL) {
	    printf("Error allocating memory for %s\n", p->m_exec);
	} /* end of if */
	else if ((kbdArgv[1] =
		  (char *) calloc(sizeof(char),
				  strlen(kymap) + 1 + 3)) == NULL
		 || !sprintf(kbdArgv[1], "-d%s", kymap)) {
	    printf("Error allocating memory for %s %s\n", p->m_exec, kymap);
	} /* end of else-if */
	else if ((kbdArgv[2] =
		  (char *) calloc(sizeof(char),
				  strlen(kyword) + 1 + 3)) == NULL
		 || !sprintf(kbdArgv[2], "-m%s", kyword)) {
	    printf("Error allocating memory for %s %s -k %s\n", p->m_exec,
		   kymap, kyword);
	} /* end of else-if */
	else {
	    int i = 0;
	    while (kbdArgv[i]) {
		printf("kbdArgv[%d]=%s\n", i, kbdArgv[i]);
		i++;
	    }			/* end of while */
	    execvp(p->m_exec, kbdArgv);
	}			/* end of else */
	exit(0);
    } else {
	g_last_window = p->m_icon_wid;
	LastLaunchedApp = p;
	appvisible = 1;
    }
}
#endif

static void
keyboard_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	keyboard_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	keyboard_buttondown(window, &ep->button);
	break;
    }
}
