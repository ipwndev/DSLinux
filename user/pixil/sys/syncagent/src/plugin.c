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
 * Use, duplication, or disc1losure by the government is subject to      
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
#include <libgen.h>

#include <dlfcn.h>

#ifdef CONFIG_PAR
#include <par/par.h>
#endif

#include "plugin.h"

#if 1	// xAMADEUS
#define dlopen(x,y) 0
#define dlsym(x,y)  0
#define dlclose(x)  0
#define dlerror()   0
#endif

static plugin_t *g_plugin = 0;

/* Fixme:  Just a bogus path right for the moment */

#define PLUGIN_PATH "../tcpip/"

plugin_t *
load_plugin(char *filename)
{
  char *data = 0;

  char path[128];

#ifdef CONFIG_PAR
  db_handle *handle = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
  
  if (handle) {
    (void) par_getCapability(handle, "syncagent", (void **) &data);
    db_closeDB(handle);
  }
#endif

  if (data)
    sprintf(path, "%s/%s.so", data, filename);
  else
    sprintf(path, "%s/%s.so", PLUGIN_PATH, filename);

  printf("Trying to load [%s]\n", path);

  if (g_plugin)
    return g_plugin;
  
  g_plugin = (plugin_t *) calloc(1, sizeof(plugin_t));

  if (!g_plugin)
    return 0;
  
  strcpy(g_plugin->name, basename(path));
    g_plugin->handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);

    if (g_plugin->handle) {
      g_plugin->init = (int (*)(int, char **))
	dlsym(g_plugin->handle, "pl_init");
      
      g_plugin->close = (int (*)(void))
	dlsym(g_plugin->handle, "pl_close");
      
      g_plugin->getfd = (int (*)(void))
	dlsym(g_plugin->handle, "pl_getfd");

      g_plugin->read = (int (*)(char **))
	dlsym(g_plugin->handle, "pl_read");
      
      g_plugin->write = (int (*)(char *, int))
	dlsym(g_plugin->handle, "pl_write");
      
      return g_plugin;
    }

    free(g_plugin);
    g_plugin = 0;
    
    return 0;
}

void free_plugin(plugin_t *plugin) {

  if (plugin != g_plugin) return;
  if (!g_plugin || !g_plugin->handle) return;

  dlclose(g_plugin->handle);
  free(g_plugin);

  g_plugin = 0;
}

