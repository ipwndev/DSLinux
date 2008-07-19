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


#ifndef SYS_MENU_H
#define SYS_MENU_H

#include <nano-X.h>

#define MENU_DIR_DOWN      0x01
#define MENU_DIR_UP        0x02
#define MENU_DIR_BEST      0x03

#define MENU_TYPE_REGULAR  0x00
#define MENU_TYPE_SEP      0x01
#define MENU_TYPE_CHECK    0x02
#define MENU_TYPE_APP      0x03
#define MENU_TYPE_CMENU    0x04
#define MENU_TYPE_END      0x05

typedef void (MENU_CALLBACK) (void *);

typedef struct
{
    char value[24];
    int type;
    void *action;
    void *cbdata;
    GR_BOOL checked;
    GR_IMAGE_ID icon;
}
wm_menu_t;

GR_BOOL system_menu_active();
int handle_system_menu(GR_EVENT * ep);
int open_system_menu(GR_WINDOW_ID parent, int x, int y,
		     int dir, wm_menu_t * menudata);

wm_menu_t *create_system_menu(int);
void destroy_system_menu(wm_menu_t *);

inline void sys_menu_add_item(wm_menu_t *, int, char *,
			      void *, void *, int, GR_IMAGE_ID iid);

/* Defined in virtwin.c */
wm_menu_t *create_catagory_menu(int index, char *name);

/* Defined in apps.c */
int create_apps_menu(wm_menu_t **);

/* Macros that abstract creating menu items */

#define ADD_SYS_MENU_REGULAR(ptr, value, action, data, icon) \
(sys_menu_add_item(ptr, MENU_TYPE_REGULAR, (char *) value, (void *) action, (void *) data, 0, icon))

#define ADD_SYS_MENU_SEP(ptr) (sys_menu_add_item(ptr, MENU_TYPE_SEP, 0, 0, 0, 0, 0))

#define ADD_SYS_MENU_CHECK(ptr, value, action, data, checked) \
(sys_menu_add_item(ptr, MENU_TYPE_CHECK, (char *) value, (void *) action, (void *) data, checked, 0))

#define ADD_SYS_MENU_APP(ptr, value, path, icon) \
(sys_menu_add_item(ptr, MENU_TYPE_APP, (char *) value, (void *) path, 0, 0, icon))

#define ADD_SYS_MENU_CMENU(ptr, value, menu) \
(sys_menu_add_item(ptr, MENU_TYPE_CMENU, (char *) value, (void *) menu, 0, 0, 0))

#define ADD_SYS_MENU_END(ptr) (sys_menu_add_item(ptr, MENU_TYPE_END, 0, 0, 0, 0,0))


#endif
