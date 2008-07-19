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

#ifndef POWERMAN_H_
#define POWERMAN_H_

#define BL_OFF 0
#define BL_ON  1

#define DISABLED -1
#define AC_ON     0
#define BAT_ON    1

/* General functions */

void pm_init(void);
void pm_reload(void);

/* Callback functions */

#define PM_CALLBACK_BL 0x0

typedef void (*pm_callback)(int, void *);

void pm_register_callback(int type, pm_callback cb);
void pm_unregister_callback(int type, pm_callback cb);

/* APM functions */
GR_TIMER_ID pm_get_timer_id(void);
void pm_suspend(void);

/* Backlight functions */

void pm_backlight(int mode);

void pm_bltimer_on(void);
void pm_bltimer_off(void);

int pm_get_bl_state(void);
void pm_bl_toggle(void);

#endif
