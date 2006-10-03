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

#include <getopt.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include <sysconfig.h>

/* These functions allow the plugins to use the SysConfig
   ipc connection */

SysConfig *g_sysconf = 0;

void
sysconf_ipc_write(int id, char *data, int len)
{
    if (!g_sysconf)
	return;
    g_sysconf->Write_Fd(id, data, len);
}

int
sysconf_ipc_find(char *str)
{
    if (!g_sysconf)
	return -1;
    return g_sysconf->Find_Fd(str);
}

NxApp *
sysconf_get_instance(void)
{
    return g_sysconf->Instance();
}

int
main(int argc, char *argv[])
{
    extern char *optarg;

    g_sysconf = new SysConfig;

    while (1) {
	signed char ch = getopt(argc, argv, "f:");
	if (ch == -1)
	    break;
	if (ch == 'f')
	    g_sysconf->ForcePlugin(optarg);
    }

    g_sysconf->showMain();

    Fl::run();
    return 0;
}
