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


#include <pixlib/pixlib.h>
#include <pixlib/pix_sys_proto.h>

/* This is a structure (defined elsewhere for each 
 * individual platform), that give us a gateway to the
 * actualy system dependent functions for each API 
 * call 
 */

extern pix_sys_func_t pix_sys_functions;

/******** SYSTEM SETTINGS **********/

/* 
 * pix_sys_set_date()
 * This function sets the date / time on the system
 */

int
pix_sys_set_date(pix_sys_date_t * date_struct)
{
    if (pix_sys_functions.sysdep_set_date)
	return (pix_sys_functions.sysdep_set_date(date_struct));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/* 
 * pix_sys_get_date()
 * This function get the date / time from the system
 */

int
pix_sys_get_date(pix_sys_date_t * date_struct)
{
    if (pix_sys_functions.sysdep_get_date)
	return (pix_sys_functions.sysdep_get_date(date_struct));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/******** POWER MANAGEMENT **********/

/* 
 * pix_sys_set_backlight(brightness)
 * This function sets the backlight on the system
 * where brigtness is a percentage of the brightness
 */

int
pix_sys_set_backlight(unsigned char brightness, unsigned char power)
{
    if (brightness > 100)
	return (PIX_SYS_ERROR);

    if (pix_sys_functions.sysdep_set_backlight)
	return (pix_sys_functions.sysdep_set_backlight(brightness, power));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/******** STATISTICS **********/

/* 
 * pix_sys_get_cpu_load()
 * This function gets the current CPU load 
 */

int
pix_sys_get_cpu_load(pix_sys_cpu_t * cpu_struct)
{
    if (pix_sys_functions.sysdep_get_cpu_load)
	return (pix_sys_functions.sysdep_get_cpu_load(cpu_struct));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/* 
 * pix_sys_get_memory_usage()
 * This function gets the current CPU memory usage 
 */

int
pix_sys_get_memory_usage(pix_sys_memory_t * mem_struct)
{
    if (pix_sys_functions.sysdep_get_memory_usage)
	return (pix_sys_functions.sysdep_get_memory_usage(mem_struct));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/* 
 * pix_sys_get_battery()
 * This function gets the current battery life
 */

int
pix_sys_get_battery(pix_sys_battery_t * battery_struct)
{
    if (pix_sys_functions.sysdep_get_battery)
	return (pix_sys_functions.sysdep_get_battery(battery_struct));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/*
 * pix_sys_get_net_value
 * This function get the value specified by key
 * 
 */
int
pix_sys_get_net_value(char *key, char *ret)
{
    if (pix_sys_functions.sysdep_get_net_value)
	return (pix_sys_functions.sysdep_get_net_value(key, ret));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}

/*
 * pix_sys_write_net_values
 * This function writes the values to the correct file
 *
 */
int
pix_sys_write_net_values(pix_sys_ipaddr_str_t ip_info, pix_sys_dns_t dns_info)
{
    if (pix_sys_functions.sysdep_write_net_values)
	return (pix_sys_functions.sysdep_write_net_values(ip_info, dns_info));
    else
	return (PIX_SYS_NOT_IMPLEMENTED);
}
