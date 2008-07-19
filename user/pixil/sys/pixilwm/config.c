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


/* For now we try to play nicely with the rest of the system */
/* API that has been established */

#include <pixil_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include <par/par.h>

#include <nano-X.h>

#include "nanowm.h"
#include "apps.h"
#include "categories.h"
#include "config.h"

#ifdef CONFIG_PIXILWM_THEMES
#include "themes.h"
#endif

#include "applets.h"

/* This local structure maintains our current settings */

static struct
{

    /* Wallpaper settings */

    struct
    {
	GR_IMAGE_ID bgImage;
	int flags;
    }
    wallpaper;

    /* Colors */
    GR_COLOR colorArray[WM_COLOR_SIZE];
    char *dirArray[WM_DIR_SIZE];

    char *theme;
}
wmSettings;

/* These are a series of "default" values */
static const char *defaultDirs[WM_DIR_SIZE] =
    { ".", "./images", "./bin", "./sounds", "./themes" };

static void
strToLower(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++)
	tolower(str[i]);
}

void
wm_setColor(int color, GR_COLOR value)
{
    wmSettings.colorArray[color] = value;
}

GR_COLOR
wm_getColor(int color)
{
    return (wmSettings.colorArray[color]);
}

void
wm_setDir(int dir, char *value)
{

    if (wmSettings.dirArray[dir]) {
	if (strcmp(wmSettings.dirArray[dir], value) == 0)
	    return;

	free(wmSettings.dirArray[dir]);
	wmSettings.dirArray[dir] = 0;
    }

    wmSettings.dirArray[dir] = strdup(value);
}

inline const char *
wm_getDir(int dir)
{
    if (!wmSettings.dirArray[dir])
	return (defaultDirs[dir]);
    return (wmSettings.dirArray[dir]);
}

int
wm_getWallpaper(GR_IMAGE_ID * iid, int *flags)
{
    if (!wmSettings.wallpaper.bgImage ||
	wmSettings.wallpaper.flags == WALLPAPER_NONE)
	return (0);

    *iid = wmSettings.wallpaper.bgImage;
    *flags = wmSettings.wallpaper.flags;
    return (1);
}

static int
nxLoadDefaults(void)
{

    wmSettings.wallpaper.flags = WALLPAPER_NONE;

    /* NOTE:  The GrGetSysColor here is not freeing its nxAllocReq */

    wm_setColor(WM_BGCOLOR, COLOR_DESKTOP);
    wm_setColor(WM_ICONCOLOR, COLOR_DESKTOP);
    wm_setColor(WM_ICONTEXT, COLOR_ICONTEXT);
    wm_setColor(WM_TASKBAR, COLOR_TASKBAR);
    wm_setColor(WM_DIALOG, COLOR_TASKBAR);

    return (0);
}

static void
getColor(db_handle * db, int index, char *keyword)
{

    int val;
    char *name = alloca(strlen("colors") + strlen(keyword) + 2);

    sprintf(name, "colors.%s", keyword);

    if (par_getScreentopSetting(db, name, &val, sizeof(val)) <= 0) {
	error("Couldn't get the color '%s' from the database.\n", keyword);
	return;
    }

    wm_setColor(index,
		GR_RGB((val >> 16) & 0xFF, (val >> 8) & 0xFF, (val) & 0xFF));
}

static void
getDirectory(db_handle * db, int index, char *keyword)
{

    char str[512];
    int size = par_getScreentopDir(db, keyword, str, sizeof(str));
    if (size > 0)
	wm_setDir(index, str);
    else
	error("Couldn't get the directory '%s' from the database.\n",
	      keyword);
}

static void
nxLoadApplets(db_handle *db) {
  
  itemlist_t *list;
  int i;

  if (par_getList(db, "screentop.settings.applets", &list) <= 0) {
    printf("screentop.settings.applets doesn't exist\n");
    return;
  }

  for(i = 0; i < list->count; i++) {
    char plugin[128];
    char name[64];

    sprintf(name, "screentop.settings.applets.%s", list->list[i]);

    if (db_findNode(db, name, plugin, sizeof(plugin), 0) < 0)
      continue;

    wm_applet_load(plugin);
  }

  par_freeItemList(list);
}
  
static int
nxLoadSettings(db_handle * db)
{

    char in_string[512];
    int style, size;

    /* First get the directories */

    getDirectory(db, DESKTOP_THEMEDIR, "themedir");
    getDirectory(db, DESKTOP_NXAPPDIR, "bindir");
    getDirectory(db, DESKTOP_IMAGEDIR, "icondir");

  /*--- Get the background image and style ---*/

    size =
	par_getScreentopSetting(db, "bgimage", in_string, sizeof(in_string));

    if (size > 0) {
	wmSettings.wallpaper.bgImage = loadIconImage(in_string, 0, 0);

	if (wmSettings.wallpaper.bgImage) {
	    style = WALLPAPER_CENTERED;

	    size = par_getScreentopSetting(db, "bgstyle", in_string,
					   sizeof(in_string));

	    if (size > 0) {
		strToLower(in_string);

		if (!strcmp(in_string, "bottom"))
		    style = WALLPAPER_BOTTOM;
		else if (!strcmp(in_string, "tiled"))
		    style = WALLPAPER_TILED;
		else if (!strcmp(in_string, "fullscreen"))
		    style = WALLPAPER_FULLSCREEN;
	    }
	} else
	    style = WALLPAPER_NONE;

	wmSettings.wallpaper.flags = style;
    }

  /*--- Now load the color settings ---*/

    getColor(db, WM_BGCOLOR, "bgcolor");
    getColor(db, WM_ICONCOLOR, "iconbgcolor");
    getColor(db, WM_ICONTEXT, "iconfgcolor");
    getColor(db, WM_TASKBAR, "taskbar");

    /* Get the theme information */

#ifdef CONFIG_PIXILWM_THEMES
    if (wm_getDir(DESKTOP_THEMEDIR)) {
	const char *dir = wm_getDir(DESKTOP_THEMEDIR);
	set_activeTheme(createTheme(dir));
    }
#endif

    /* Lastly, load the taskbar applets */
    nxLoadApplets(db);

    return (0);
}

/* Fixme:  How can we handle reloading settings? */

int
nxLoadConfig(void)
{

    db_handle *db = 0;

    /* Load any defaults */
    bzero(&wmSettings, sizeof(wmSettings));
    nxLoadDefaults();

    /* Now, open the PAR and get ready to read! */
    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (!db) {
	error("PAR database open returned error %d\n", pardb_errno);
	return (-1);
    }

    /* Now load in steps.  First the settings */
    nxLoadSettings(db);

    /* Next, get the categories and applications */
    nxLoadCategories(db);

    /* Finally, load the input tools */
    nxLoadInputs(db);

    /* We're done, go ahead and close the database */
    db_closeDB(db);
    return (0);
}
