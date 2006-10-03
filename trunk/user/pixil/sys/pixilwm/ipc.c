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

#ifdef CONFIG_COLOSSEUM
#include <ipc/colosseum.h>
#endif

#include <nano-X.h>

#include "nanowm.h"
#include "apps.h"
#include <wm/scrtoplib.h>

#ifdef CONFIG_PIXILWM_PM
#include "powerman.h"
#endif

//extern int check_par_tz_flag;

static int cl_fd = 0;

int
ipcActive(void)
{
    return ((cl_fd > 0) ? 1 : 0);
}

void
startIPC(void)
{
#ifdef CONFIG_COLOSSEUM
    int cl_flags;

    cl_fd = ClRegister("nxscrtop", &cl_flags);

    if (cl_fd <= 0)
	warning("Unable to locate the Colosseum server.\n");
    else
	GrRegisterInput(cl_fd);
#endif
}

int
ipcGetAppName(int processid, char *name, int size)
{
#ifndef CONFIG_COLOSSEUM
  return -1;
#else
    cl_app_info info;

    info.flags = CL_APP_INFO_PID;
    info.processid = processid;

    if (clGetAppInfo(&info) == -1) {
	error("Unable to get information for process '%d'.\n", processid);
	return (-1);
    }

    strncpy(name, info.name, size);
    return (0);
#endif
}

static void
handleAction(scrtop_action * action)
{

    APP *app = find_app_name(action->app);
    win *window;

    if (!app) {
	error("Couldn't locate application '%s'.\n", action->app);
	return;
    }

    if (!app->m_container_wid)
	return;
    window = find_window(app->m_container_wid);

    if (!window) {
	error("Unable to find the window for container %d.\n",
	      app->m_container_wid);
	return;
    }

    switch (action->type) {

    case ACTION_SHOW:
	container_activate(app->m_container_wid);
	break;

    case ACTION_HIDE:
	container_hide(app->m_container_wid);
	break;

    case ACTION_CLOSE:
	window = find_window(app->m_container_wid);
	if (!window || !window->clientid)
	    break;

	GrCloseWindow(window->clientid);
	break;

    default:
	warning("Unknown IPC command %d\n", action->type);
	break;
    }
}

void
handleNAAction(scrtop_action * action)
{
    /*
       ** This function handles the "Non-Application" related actions, i.e. things
       ** that pertain to the screen top only
     */

    switch (action->type) {
#ifdef CONFIG_PIXILWM_PM
    case NA_ACTION_BLON:
      pm_backlight(1);
      break;

    case NA_ACTION_BLOFF:
      pm_backlight(0);
      break;
#endif
    default:
      warning("Unknown NA_ACTION IPC command %d\n", action->type);
      break;
    }				/* end of switch */
}				/* end of handleNAAction() */

void
handleTextIpc(scrtop_message * msg)
{

    char *t_msg;
    char *c = 0;

    t_msg = (char *) malloc(CL_MAX_MSG_LEN);

    if (t_msg == 0)
	return;

    memcpy(t_msg, msg, sizeof(scrtop_message));

    c = strchr(t_msg, '^');

    if (c != 0)
	*c++ = 0;

#ifdef CONFIG_PIXILWM_PM
    if (!(strcmp(t_msg, "sc_power")))
      pm_reload();

    if (!(strcmp(t_msg, "sc_backlite")))
      pm_reload();
#endif

    if (!(strcmp(t_msg, "sc_clock"))) {
	/*
	   ** The user has changed the system time/date (possibly changed the 
	   ** timezone, etc, get values from par and set time accordingly
	 */
      //check_par_tz_flag = 1;
    }
    /* end of if */
    free(t_msg);
}

void
handleIPC(GR_EVENT_FDINPUT * e)
{
#ifdef CONFIG_COLOSSEUM

    scrtop_message msg;
    int size = sizeof(msg);
    int ack = 0;
    unsigned short src = 0;

    /* We have a message waiting for us, lets grab it and do some stuff */

    ack = ClGetMessage(&msg, &size, &src);

    if (ack < 0)
	return;

    if (ack == CL_CLIENT_BROADCAST) {
	handleTextIpc(&msg);
	return;
    }

    switch (GET_CATEGORY(msg.type)) {

    case CATEGORY_ACTION:
	handleAction(&msg.action);
	break;

    case CATEGORY_NA_ACTION:
	handleNAAction(&msg.action);
	break;

    default:
	warning("Unknown IPC message category %d.\n", GET_CATEGORY(msg.type));
	break;
    }
#endif
}

/* Send off an application for the IPC to spawn */

int
ipcSpawnApp(char *app, char *str)
{
#ifdef CONFIG_COLOSSEUM
    if (!cl_fd)
	return (-1);

    return (ClSpawnApp(app, str));
#endif
}
