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



/* Portions Copyright (C) 2000 Alex Holden <alex@linuxhacker.org> */

#ifndef _NANOWM_H_
#define _NANOWM_H_

#include "apps.h"
#include "categories.h"
#include "sys_menu.h"

#ifdef DEBUG
#define Dprintf printf
#else
#define Dprintf(ignore...)
#endif

#include <par/pardb.h>

typedef struct windowlist *winptr;
typedef void (*wndproc) (struct windowlist * window, GR_EVENT * ep);

struct pos_size
{
    GR_COORD xoff;
    GR_COORD yoff;
    GR_COORD xorig;
    GR_COORD yorig;
    GR_SIZE width;
    GR_SIZE height;
};

/* all root-mapped windows are kept track of with this structure*/
struct windowlist
{
    GR_WINDOW_ID wid;		/* The ID of this window */
    GR_WINDOW_ID pid;		/* The ID of this window's parent */
    GR_WINDOW_ID clientid;	/* clientid for container window */
    GR_BOOL mousedn;		/* TRUE when mouse is down in window */
    GR_COORD x;			/* mouse down coordinates */
    GR_COORD y;
    wndproc proc;		/* callback associated with window */
    struct windowlist *next;	/* The next window in the list */
    int sizing;			/* used during resize handling */
    struct pos_size pos;	/* used during resize/move handling */

    struct
    {
	struct windowlist *next;
	struct windowlist *prev;
    }
    zorder;
};
typedef struct windowlist win;


/* LOGGING MACROS */

#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_DEBUG 3

void scrtop_log_message(int level, char *fmt, ...);

#define debug(fmt, args...) scrtop_log_message(LOG_DEBUG, fmt, ## args)
#define warning(fmt, args...) scrtop_log_message(LOG_WARNING, fmt, ## args)
#define error(fmt, args...) scrtop_log_message(LOG_ERROR, fmt, ## args)

/* wlist.c*/
win *find_window(GR_WINDOW_ID wid);
win *add_window(GR_WINDOW_ID wid, GR_WINDOW_ID pid, GR_WINDOW_ID clid,
		wndproc proc);
int remove_window(win * window);
int remove_window_and_children(win * window);
void resize_windows(int width, int height);

int zorder_push(GR_WINDOW_ID id);
GR_WINDOW_ID zorder_peek_top(void);
void zorder_remove(GR_WINDOW_ID id);
void zorder_raise(GR_WINDOW_ID id);


/* main.c*/
typedef void (*scrtop_timer_cb)(void);

void scrtop_register_timer(GR_TIMER_ID id, void (*callback)(void));
void scrtop_unregister_timer(GR_TIMER_ID id);

void timeout_setproc(wndproc proc, win * window);

/* categories.c */
wm_menu_t *create_categoryMenu(int index, char *name);

/* container.c*/
void container_wndproc(win * window, GR_EVENT * ep);
void container_activate(GR_WINDOW_ID wid);
void container_hide(GR_WINDOW_ID wid);

/* clients.c*/
int client_updatemap_new(GR_WINDOW_ID wid);
void setWorkspace(int width, int height);
void getWorkspace(int *width, int *height);

/* apps.c */
void apps_free_memory();

/* root.c */
void root_free_memory();
inline void redraw_root_window();

/* icons.c */
void hide_icon_list(void);
void draw_current_iconlist(GR_REGION_ID r);
GR_WINDOW_ID create_icon_window();

GR_IMAGE_ID loadIconImage(char *filename, char *app, int useDefault);

/* inputs.c */
int nxLoadInputs(db_handle * db);
void createInputButton(void);

/* settings.c */
int nxLoadConfig(void);

/* ipc.c */
void startIPC(void);
void handleIPC(GR_EVENT_FDINPUT * e);
int ipcSpawnApp(char *app, char *str);
int ipcActive(void);
int ipcGetAppName(int processid, char *name, int size);

/* input.c */
void inputResetWorkspace(void);
void hideInputs(void);
int isInputVisible(void);

#endif
