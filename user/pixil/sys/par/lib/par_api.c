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

#include <par/pardb.h>
#include <par/par.h>

#define APPLICATION_NAME "application"
#define CAPABILITY_NAME  "capabilities"
#define SCREENTOP_NAME   "screentop"

#define GLOBAL_PREF_NAME "global.preferences"

/* Get a global preference from the system */

/* Size here is really only for the text type */

int
par_getGlobalPref(db_handle * db, char *category, char *keyword, int type,
		  void *dest, int size)
{

    int ret;

    unsigned short gsize;
    unsigned short gtype;

    char *name =
	alloca(strlen(GLOBAL_PREF_NAME) + strlen(category) + strlen(keyword) +
	       2 + 1);
    if (!name)
	return (-1);

    sprintf(name, "%s.%s.%s", GLOBAL_PREF_NAME, category, keyword);

    switch (type) {
    case PAR_TEXT:
	gsize = size;
	break;

    case PAR_INT:
	gsize = sizeof(int);
	break;

    case PAR_FLOAT:
	gsize = sizeof(float);
	break;

    case PAR_BOOL:
	gsize = sizeof(unsigned char);
	break;

    default:
	gsize = size;
    }

    ret = db_findNode(db, name, dest, gsize, &gtype);

    if (ret < 0)
	return (ret);
    if (gtype != type)
	return (-1);

    return (ret);
}

/* Add/modify a global preference */

int
par_setGlobalPref(db_handle * db, char *category, char *keyword, int type,
		  char *value, int size)
{

    char *name =
	alloca(strlen(GLOBAL_PREF_NAME) + strlen(category) + strlen(keyword) +
	       2 + 1);
    if (!name)
	return (-1);
    sprintf(name, "%s.%s.%s", GLOBAL_PREF_NAME, category, keyword);

    return (db_addNode(db, name, value, size, type));
}

int
par_removeGlobalPref(db_handle * db, char *category, char *keyword,
		     char *value, int size, int type)
{

    char *name =
	alloca(strlen(GLOBAL_PREF_NAME) + strlen(category) + strlen(keyword) +
	       2 + 1);
    if (!name)
	return (-1);
    sprintf(name, "%s.%s.%s", GLOBAL_PREF_NAME, category, keyword);

    return (db_delNode(db, name));
}

inline int
par_getGlobalColor(db_handle * db, char *keyword, unsigned long *dest)
{
    return (par_getGlobalPref
	    (db, "appcolors", keyword, PAR_COLOR, (void *) dest,
	     sizeof(unsigned long)));
}

int
par_getAppField(db_handle * db, char *app, char *keyword, void *data,
		int size)
{

    int ret;

    char *local =
	alloca(strlen(APPLICATION_NAME) + strlen(app) + strlen(keyword) + 3);

    if (!local)
	return (-1);

    sprintf(local, "%s.%s.%s", APPLICATION_NAME, app, keyword);

    bzero((char *) data, size);

    ret = db_findNode(db, local, data, size - 1, 0);
    return (ret);
}

#define GET_FIELD(db, app, title, field) \
par_getAppField(db, app, title, field, sizeof(field))

int
par_getApplication(db_handle * db, char *app, par_app_t * local)
{
    int ret = 0;

    if (!app || !local)
      return -1;

    memset(local, 0, sizeof(par_app_t));

    /* It should not be an error not to have all the fields.  Really,
       isn't only exec needed? */

    /* So this has been changed.  If par_getAppField returns any error
       except "not found" then return an error.  Otherwise, ignore it.
    */

    ret = GET_FIELD(db, app, "title", local->title);
    if (ret == -1 && pardb_errno != PARDB_NOTFOUND) return -1;
   
    ret = GET_FIELD(db, app, "exec", local->path);
    if (ret == -1 && pardb_errno != PARDB_NOTFOUND) return -1;

    ret = GET_FIELD(db, app, "workdir", local->workdir);
    if (ret == -1 && pardb_errno != PARDB_NOTFOUND) return -1;

    ret = GET_FIELD(db, app, "icon", local->icon);
    if (ret == -1 && pardb_errno != PARDB_NOTFOUND) return -1;

    ret = GET_FIELD(db, app, "defargs", local->defargs);
    if (ret == -1 && pardb_errno != PARDB_NOTFOUND) return -1;

    return 0;
}

int
par_getAppPref(db_handle * db, char *application, char *category,
	       char *keyword, char *dest, unsigned short size)
{

    int len = strlen(APPLICATION_NAME) + strlen(application)
	+ strlen("preferences") + strlen(category) + strlen(keyword) + 4 + 1;

    char *name = alloca(len);

    if (!name)
	return (-1);

    sprintf(name, "%s.%s.preferences.%s.%s",
	    APPLICATION_NAME, application, category, keyword);

    return (db_findNode(db, name, dest, size, 0));
}

int
par_addAppPref(db_handle * db, char *application, char *category,
	       char *keyword, char *dest, unsigned short size,
	       unsigned short type)
{

    int len = strlen(APPLICATION_NAME) + strlen(application)
	+ strlen("preferences") + strlen(category) + strlen(keyword) + 4 + 1;

    char *name = alloca(len);

    if (!name)
	return (-1);

    sprintf(name, "%s.%s.preferences.%s.%s",
	    APPLICATION_NAME, application, category, keyword);

    return (db_addNode(db, name, dest, size, type));
}

int
par_delAppPref(db_handle * db, char *application, char *category,
	       char *keyword)
{

    int len = strlen(APPLICATION_NAME) + strlen(application)
	+ strlen("preferences") + strlen(category) + strlen(keyword) + 4 + 1;

    char *name = alloca(len);

    if (!name)
	return (-1);

    sprintf(name, "%s.%s.preferences.%s.%s",
	    APPLICATION_NAME, application, category, keyword);

    return (db_delNode(db, name));
}

int
par_getCapability(db_handle * db, char *capability, void **dest)
{

    int size;

    char *name = alloca(strlen(CAPABILITY_NAME) + strlen(capability) + 2);
    if (!name)
	return (-1);

    sprintf(name, "%s.%s", CAPABILITY_NAME, capability);

    size = db_getDataSize(db, name);

    if (size <= 0) {
	*dest = 0;
	return (size);
    }

    *dest = (char *) malloc(size);
    db_findNode(db, name, *dest, size, 0);

    return (size);
}

int
par_getScreentopSetting(db_handle * db, char *setting, void *dest, int size)
{

    char *name = alloca(strlen("screentop.settings.") + strlen(setting) + 1);
    if (!name)
	return (-1);

    sprintf(name, "screentop.settings.%s", setting);

    return (db_findNode(db, name, dest, size, 0));
}

int
par_getScreentopDir(db_handle * db, char *dir, char *dest, int size)
{

    int len = strlen("screentop.directories") + strlen(dir);
    char *name = alloca(len + 2);

    if (!name)
	return (-1);
    sprintf(name, "screentop.directories.%s", dir);

    return (db_findNode(db, name, dest, size, 0));
}

int
par_getScreentopInput(db_handle * db, char *input, par_input_t * data)
{

    int len = strlen("screentop.inputs") + strlen(input) + strlen("title");
    char *name = alloca(len + 3);

    if (!name)
	return (-1);

    sprintf(name, "screentop.inputs.%s.title", input);
    if (db_findNode(db, name, data->title, sizeof(data->title), 0) < 0)
	return (-1);

    sprintf(name, "screentop.inputs.%s.app", input);
    if (db_findNode(db, name, data->app, sizeof(data->app), 0) < 0)
	return (-1);

    sprintf(name, "screentop.inputs.%s.icon", input);
    if (db_findNode(db, name, data->icon, sizeof(data->icon), 0) < 0)
	return (-1);

    return (0);
}

int
par_getScreentopCategory(db_handle * db, char *category,
			 char **title, char **applist)
{

    int size;
    int len =
	strlen("screentop.categories") + strlen(category) + strlen("applist");
    char *name = alloca(len + 3);

    if (!name)
	return (-1);

    /* First, get the title */

    sprintf(name, "screentop.categories.%s.title", category);

    size = db_getDataSize(db, name);
    if (size <= 0)
	return (-1);

    *title = (char *) malloc(size);
    db_findNode(db, name, *title, size, 0);

    /* Next, get the full app list */
    sprintf(name, "screentop.categories.%s.applist", category);
    size = db_getDataSize(db, name);

    if (size <= 0) {
	*applist = 0;
	return (0);
    }

    *applist = (char *) malloc(size);
    db_findNode(db, name, *applist, size, 0);
    return (0);
}

/* List functions */

static int
local_getList(db_handle * db, char *name, itemlist_t * local)
{

    int count = db_getChildCount(db, name);

    if (count <= 0)
	return (count);

    local->list = (char **) calloc(count * sizeof(char *), 1);
    if (!local->list)
	return (-1);

    /* Now, send the list in and get the child names */

    local->count = db_getChildList(db, name, local->list, count);

    if (local->count == -1) {
	int i;

	for (i = 0; i < count; i++) {
	    if (local->list[i])
		free(local->list[i]);
	}

	free(local->list);
	local->list = 0;
    }

    return (local->count);
}

int
par_getList(db_handle * db, char *name, itemlist_t ** list)
{

    itemlist_t *local = (itemlist_t *) malloc(sizeof(itemlist_t));
    int ret = local_getList(db, name, local);

    if (ret == -1)
	free(local);
    else
	*list = local;

    return (ret);
}

void
par_freeItemList(itemlist_t * list)
{
    int i;

    if (!list)
	return;

    for (i = 0; i < list->count; i++) {
	if (list->list[i])
	    free(list->list[i]);
    }

    if (list->list)
	free(list->list);
    free(list);
}

int
par_getListItem(itemlist_t * list, int index, char *item, int *size)
{

    int lsize;

    if (!list)
	return (0);
    if (index >= list->count)
	return (0);

    if (*size > strlen(list->list[index]))
	lsize = strlen(list->list[index]);
    else
	lsize = *size;

    strncpy(item, list->list[index], lsize);

    *size = lsize;
    return (list->count > (index + 1) ? index + 1 : 0);
}

int
par_getStringListCount(char *str, char delim)
{

    int count = 1;
    char *p = str;

    if (!*p)
	return (0);

    while (1) {
	p = strchr(p, delim);
	if (!p)
	    return (count);

	while (*p == delim)
	    p++;
	if (!*p)
	    return (count);

	count++;
    }
}

char *
par_parseStringList(char **str, char delim)
{

    char *start, *n;

    if (!&str)
	return (0);

    start = *str;
    n = strchr(*str, delim);

    if (!n)
	*str = 0;
    else {
	char *p = n + 1;

	*n = 0;
	while (*p == delim)
	    p++;

	if (!*p)
	    *str = 0;
	else
	    *str = p;
    }

    return (start);
}
