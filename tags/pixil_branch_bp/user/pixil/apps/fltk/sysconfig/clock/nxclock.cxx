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

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include "nxsunclock.h"
#include "nxclock.h"

#define PLUGIN_CLASS nxSunclock

static PLUGIN_CLASS *plugin = 0;

static void
PLUGIN_CLASS_new(int x, int y, int w, int h)
{
    if (!plugin)
	plugin = new PLUGIN_CLASS(x, y, w, h, UTILITY_NAME);
}

static void
PLUGIN_CLASS_show(void)
{
    if (plugin)
	plugin->ShowWindow();
}

static void
PLUGIN_CLASS_hide(void)
{
    if (plugin)
	plugin->HideWindow();
}

static void
PLUGIN_CLASS_del(void)
{
    if (plugin)
	delete plugin;
}

extern "C" void
plugin_create(int x, int y, int w, int h)
{
    PLUGIN_CLASS_new(x, y, w, h);
}

extern "C" void
plugin_show(void)
{
    PLUGIN_CLASS_show();
}

extern "C" void
plugin_hide(void)
{
    PLUGIN_CLASS_hide();
}

extern "C" void
plugin_info(char *str, int size)
{
    strncpy(str, UTILITY_NAME, size);
}

extern "C" void
plugin_close(void)
{
    PLUGIN_CLASS_del();
}
