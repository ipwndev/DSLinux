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


#ifndef SETTINGS_H
#define SETTINGS_H

#include "nano-X.h"

/* These are the default settings */
/* Change them as you see fit     */

#define DEFAULT_WALLPAPER	"cenlogo.gif"
#define DEFAULT_BGCOLOR         COLOR_DESKTOP
#define DEFAULT_ICONCOLOR       COLOR_DESKTOP
#define DEFAULT_ICONTEXT        COLOR_ICONTEXT

#define WALLPAPER_NONE       0x00
#define WALLPAPER_TILED      0x01
#define WALLPAPER_CENTERED   0x02
#define WALLPAPER_FULLSCREEN 0x04
#define WALLPAPER_BOTTOM     0x05

#define WM_BGCOLOR         0
#define WM_ICONCOLOR       1
#define WM_ICONTEXT        2
#define WM_TASKBAR         3
#define WM_DIALOG          5
#define WM_COLOR_SIZE      6


#define DESKTOP_PREFIX   0
#define DESKTOP_IMAGEDIR 1
#define DESKTOP_NXAPPDIR 2
#define DESKTOP_SOUNDDIR 3
#define DESKTOP_THEMEDIR 4

#define WM_DIR_SIZE      5

/* These are functions to get the various window manager settings */

void wm_setColor(int color, GR_COLOR value);
GR_COLOR wm_getColor(int color);
void wm_setDir(int dir, char *value);
const char *wm_getDir(int dir);
int wm_getWallpaper(GR_IMAGE_ID * iid, int *flags);

#endif
