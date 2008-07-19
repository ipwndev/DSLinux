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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/fb.h>

#include <pixlib/pixlib.h>

static int open_device(void) {
  int fd = open("/dev/fb0", O_RDWR);
  return fd;
}

/* 
 pix_bl_ctrl() - Controls the backlight.  
 pwr:  0 = off, 1 = on
 level:  brightness level between 0 and the device maximum
*/

int
pix_bl_ctrl(int pwr, int level)
{
  struct bl_setting set;

  fd = open_device();
  if (!fd) return -1;

  
  ioctl(fd, FB_BACKLIGHT_GET_POWER, &set);

  if (set.cur != (pwr) ? 1 : 0) {
    set.curr = (pwr) ? 1 : 0;
    ioctl(fd, FB_BACKLIGHT_SET_POWER, &set); 
  }

  ioctl(fd, FB_BACKIGHT_GET_BRIGHTNESS, &set);

  if (level != set.cur) {
    set.curr = (level > set.max) ? set.max : level;
    ioctl(fd, FB_BACKLIGHT_SET_BRIGHTNESS, &set);
  }

  close(fd);
  return 0;
}		

int
pix_bl_getmxval(void)
{
  struct bl_setting set;

  fd = open_device();
  if (!fd) return -1;

  ioctl(fd, FB_BACKLIGHT_GET_BRIGHTNESS, &set);
  close(fd);

  return set.max;
}

