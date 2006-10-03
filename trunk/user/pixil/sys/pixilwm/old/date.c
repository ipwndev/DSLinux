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

#include <langinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <nano-X.h>
#include "nanowm.h"
#include "apps.h"
#include "config.h"
#include <par/par.h>

#ifdef CONFIG_PIXILWM_MENUS
#include "sys_menu.h"
#endif

#define DATE_12_HOUR_MODE 0x00
#define DATE_24_HOUR_MODE 0x01
#define BUFFER_LENGTH 20

static int DateToggle = 0;
static int DateMode = DATE_12_HOUR_MODE;

#ifdef NOTUSED

static char *DAY[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static char *MON[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#endif

static void date_wndproc(win * window, GR_EVENT * ep);

int check_par_tz_flag = 1;
static GR_FONT_ID g_fontid;

void
date_create(void)
{
    win *window;
    GR_WINDOW_ID wid;
    GR_SCREEN_INFO si;

    GrGetScreenInfo(&si);

    g_fontid = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);

    wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
			32, si.ws_height + 4, 70, 16,
			wm_getColor(WM_TASKBAR));
    window = add_window(wid, GR_ROOT_WINDOW_ID, 0, date_wndproc);

    GrSelectEvents(wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP |
		   GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(wid);

    timeout_setproc(date_wndproc, window);
}

static void
date_exposure(win * window, GR_EVENT_EXPOSURE * ep)
{
    char hmtz_flg = 0, tmzone[64 + 1] = { '\0' };
    time_t t;
    struct tm *tv;
    char buf[BUFFER_LENGTH];
    GR_GC_ID gc = GrNewGC();
    db_handle *pardb;

    GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));
    GrSetGCBackground(gc, wm_getColor(WM_BGCOLOR));
    GrFillRect(ep->wid, gc, ep->x, ep->y, ep->width, ep->height);

    /* open par */
    if (check_par_tz_flag != 0) {
	char *tzone;

	if ((pardb =
	     db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY)) != NULL) {
	    /* Get the values from par */
	    par_getGlobalPref(pardb, "timezone", "home_active", PAR_BOOL,
			      &hmtz_flg, sizeof(char));
	    if (hmtz_flg)
		par_getGlobalPref(pardb, "timezone", "home_tz", PAR_TEXT,
				  tmzone, sizeof(tmzone) - 1);
	    else
		par_getGlobalPref(pardb, "timezone", "visit_tz", PAR_TEXT,
				  tmzone, sizeof(tmzone) - 1);

	    if (tmzone[0] > ' ') {
		if ((tzone = strrchr(tmzone, ',')) != NULL)
		    tzone++;
		else
		    tzone = tmzone;

		setenv("TZ", tzone, 1);
		tzset();
	    }			/* end of if */
	    db_closeDB(pardb);
	}			/* end of if */
	check_par_tz_flag = 0;
    }
    /* end of if  */
    t = time(0);
    tv = localtime(&t);

    if (DateToggle) {
	/* 
	 * jeffm:
	 * %X = Show localized date w/ out time.
	 */
//        strftime(buf, BUFFER_LENGTH, "%x", tv);
	strftime(buf, BUFFER_LENGTH, nl_langinfo(D_FMT), tv);
    } else {
	if (DateMode == DATE_24_HOUR_MODE) {
	    /*
	     * jeffm: 
	     * %k = 24 hour mode from 0 - 23.
	     * %M = Minutes
	     */
//                strftime(buf, BUFFER_LENGTH, "%k:%M", tv); 
	    strftime(buf, BUFFER_LENGTH, nl_langinfo(T_FMT), tv);
	} else {
	    /*
	     * jeffm:
	     * %l = 12 hour mode from 1 - 12.
	     * %M = Minutes
	     * %P = 'am' or 'pm' or a corresponding string for the current local.
	     */
//                 strftime(buf, BUFFER_LENGTH, "%l:%M %P", tv);
	    strftime(buf, BUFFER_LENGTH, nl_langinfo(T_FMT_AMPM), tv);
	}
    }

    GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
    GrSetGCBackground(gc, wm_getColor(WM_TASKBAR));
    GrSetGCFont(gc, g_fontid);
    GrText(ep->wid, gc, 0, 1, buf, -1, GR_TFTOP);

    GrDestroyGC(gc);
}

static void
date_redraw(win * window)
{
    GR_EVENT_EXPOSURE expose;
    GR_WINDOW_INFO info;

    /* dummy full area expose event */
    GrGetWindowInfo(window->wid, &info);
    expose.type = GR_EVENT_TYPE_EXPOSURE;
    expose.wid = window->wid;
    expose.x = 0;
    expose.y = 0;
    expose.width = info.width;
    expose.height = info.height;
    date_exposure(window, &expose);
}

#ifdef CONFIG_PIXILWM_MENUS

static void
date_set_format(void *val)
{
    if ((int) val == DATE_24_HOUR_MODE)
	DateMode = DATE_24_HOUR_MODE;
    else
	DateMode = DATE_12_HOUR_MODE;
}

static wm_menu_t date_sys_menu[] = {
    {"12 Hour", MENU_TYPE_CHECK, date_set_format, (void *) DATE_12_HOUR_MODE},
    {"24 Hour", MENU_TYPE_CHECK, date_set_format, (void *) DATE_24_HOUR_MODE},
    {"", MENU_TYPE_END, 0}
};

#endif

static void
date_buttondown(win * window, GR_EVENT_BUTTON * ep)
{
#ifdef CONFIG_PIXILWM_MENUS
    if (ep->buttons & GR_BUTTON_R) {
	GR_SCREEN_INFO si;
	GrGetScreenInfo(&si);

	date_sys_menu[0].checked = GR_FALSE;
	date_sys_menu[1].checked = GR_FALSE;

	if (DateMode == DATE_12_HOUR_MODE)
	    date_sys_menu[0].checked = GR_TRUE;
	else
	    date_sys_menu[1].checked = GR_TRUE;

	if (open_system_menu(GR_ROOT_WINDOW_ID, 32, si.ws_height,
			     MENU_DIR_UP, date_sys_menu))

	    return;
    }
#endif

    DateToggle = !DateToggle;
    date_redraw(window);
}

static void
date_wndproc(win * window, GR_EVENT * ep)
{
    switch (ep->type) {
    case GR_EVENT_TYPE_EXPOSURE:
	date_exposure(window, &ep->exposure);
	break;
    case GR_EVENT_TYPE_BUTTON_DOWN:
	date_buttondown(window, &ep->button);
	break;
    case GR_EVENT_TYPE_TIMEOUT:
	date_redraw(window);
	break;
    }
}
