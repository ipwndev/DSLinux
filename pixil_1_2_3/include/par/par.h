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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef _PAR_H_
#define _PAR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <par/pardb.h>

    typedef struct
    {
	char title[25];
	char path[128];
	char workdir[128];
	char icon[64];
	char defargs[64];
    }
    par_app_t;

    typedef struct
    {
	char title[30];
	char app[25];
	char icon[64];
    }
    par_input_t;

    typedef struct
    {
	char **list;
	int count;
    }
    itemlist_t;

    int par_getGlobalPref(db_handle * db, char *category, char *keyword,
			  int type, void *dest, int size);

    int par_setGlobalPref(db_handle * db, char *category, char *keyword,
			  int type, char *value, int size);

    int par_removeGlobalPref(db_handle * db, char *category,
			     char *keyword, char *value, int size, int type);

/* These are some commonly used items defined here for simplicity */
    int par_getGlobalColor(db_handle * db, char *keyword,
			   unsigned long *dest);

    int par_getAppField(db_handle * db, char *app, char *keyword, void *data,
			int size);

    int par_getApplication(db_handle * db, char *app, par_app_t * astruct);

#define par_getAppTitle(db, app, name, size) (par_getAppField(db, app, "title", name, size))
#define par_getAppPath(db, app, name, size) (par_getAppField(db, app, "exec", name, size))
#define par_getAppWorkDir(db, app, name, size) (par_getAppField(db, app, "workdir", name, size))
#define par_getAppIcon(db, app, name, size) (par_getAppField(db, app, "icon", name, size))
#define par_getAppArgs(db, app, name, size) (par_getAppField(db, app, "defargs", name, size))

    int par_getAppPref(db_handle * db, char *application, char *category,
		       char *keyword, char *dest, unsigned short size);

    int par_addAppPref(db_handle * db, char *application, char *category,
		       char *keyword, char *dest, unsigned short size,
		       unsigned short type);

    int par_delAppPref(db_handle * db, char *application, char *category,
		       char *keyword);

    int par_getScreentopSetting(db_handle * db, char *setting, void *dest,
				int size);
    int par_getScreentopDir(db_handle * db, char *dir, char *dest, int size);
    int par_getScreentopInput(db_handle * db, char *input,
			      par_input_t * data);
    int par_getScreentopCategory(db_handle * db, char *category, char **title,
				 char **applist);

    int par_getCapability(db_handle * db, char *capability, void **dest);

/* This is the generic get list function */

    int par_getList(db_handle * db, char *name, itemlist_t ** list);

/* These are some convienent defines */
#define par_getAppList(db, list) (par_getList(db, "application", list))

#define par_getScreentopCatList(db, list) (par_getList(db, "screentop.categories", list))
#define par_getScreentopInputList(db, list) (par_getList(db, "screentop.inputs", list))

/* These are some generic itemlist utility functions */

    int par_getListItem(itemlist_t * list, int index, char *item, int *size);
    void par_freeItemList(itemlist_t * list);

    int par_getStringListCount(char *str, char delim);
    char *par_parseStringList(char **str, char delim);

#ifdef __cplusplus
}
#endif

#endif
