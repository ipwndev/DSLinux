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


#ifndef _APPS_H_
#define _APPS_H_

#include <nano-X.h>
#include <par/par.h>
#include <wm/nxlib.h>

#define ICONWIDTH	48
#define ICONHEIGHT	48
#define ICONTEXTHEIGHT	13

#define COLOR_TASKBAR 		GrGetSysColor(GR_COLOR_3DLIGHT)
#define COLOR_TASKBARHILIGHT	GrGetSysColor(GR_COLOR_BTNHIGHLIGHT)
#define COLOR_ICONTEXT		GrGetSysColor(GR_COLOR_ACTIVECAPTIONTEXT)
#define COLOR_DESKTOP 		GrGetSysColor(GR_COLOR_DESKTOP)
#define COLOR_REALDESKTOP 	GR_RGB(1,1,1)

void root_create(void);
void load_applications(char *);
void apps_buttondown(GR_WINDOW_ID id);
void home_create(void);
void date_create(void);
void backlight_create(void);
void scribble_create(void);
void keyboard_create(void);
void battery_create(void);

#ifdef VIRTUAL_WINDOWS
void cat_switch_create(void);
#endif

#define FL_INPUT        0x001
#define FL_NOICON	0x004	/* don't create icon or icon window */
#define FL_ONETIME      0x008	/* Only run once */
#ifdef VIRTUAL_WINDOWS
#define FL_STICKY       0x010
#endif

#ifdef VIRTUAL_WINDOWS
#define VW_STICKY       0x01
#endif


/* desktop and taskbar applications kept here*/
typedef struct
{
    NXLIST link;

    char m_name[30];

    char m_exec[64];		/* executable filename */
    char m_icontitle[64];	/* displayed icon title */
    char m_workdir[64];
    char m_args[64];
    int m_flags;		/* application flags */
    int m_icon_wid;		/* icon input window id */
    int m_icon_iid;		/* icon image id */
    int m_container_wid;	/* container window id */
    int m_process_id;		/* linux process id of running app */
#ifdef VIRTUAL_WINDOWS
    int m_virtual_wid;		/* The current virtual window that this app is associated with */
    int m_virtual_flags;	/* Virtual window flags */
#endif

}
APP;

typedef struct
{
    NXLIST next;
    int pid;
    APP *app;
}
PROCESS;

typedef struct app_info_struct
{
    int flags;

    char title[25];
    char path[128];
    char workdir[128];
    char icon[64];
    char args[64];
}
app_info_t;

extern NXLISTHEAD apphead;

extern APP *g_last_app;

/* Search functions */

APP *find_app_flags(int flags);
APP *find_app_path(char *path);	/* Find a app based on the path */
APP *find_app_pid(int pid);
APP *find_app_name(char *name);
APP *find_app_icon(GR_WINDOW_ID id);
APP *find_app_container(GR_WINDOW_ID id);

/* Old style process handlers */

void application_handler(int id);
APP *apps_add_onetime(char *path);

/* Application list managment */
int apps_add_application(char *, app_info_t * app_info, APP ** application);
int launch_application(APP * app);
void hide_application(APP * app);
void show_application(APP * app);
int is_app_running(APP * app);

/* Window managment */
void app_show_windows(int, int);
void app_hide_windows();

/* Process management */
void kill_running_processes();

/* input.c */

void killInput(APP * app);
void inputResizeWorkspace(APP * app);

#endif
