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


#ifndef PIX_SYS_PROTO_H
#define PIX_SYS_PROTO_H

/*
 * PIX_SYS_PROTO.H
 * Copyright 2000, Century Software Embedded Technologies
 */

/* 
 * DESCRIPTION:  This file defines the prototype structure
 *               used to define the system dependent functions
 *               that must be called for each API function
 *
 * AUTHOR:       First draft, Jordan Crouse, 11/27/00
 */

/****** THIS FILE IS FOR INTERNAL LIBRARY USE ONLY ******/

#include <pixlib/pixlib.h>

#define PIX_SYS_PROTO_NO_FUNCTION 0x0	/* Use this when no function is available */

typedef struct _pix_sysproto
{
    int (*sysdep_set_date) (pix_sys_date_t *);
    int (*sysdep_get_date) (pix_sys_date_t *);
    int (*sysdep_set_backlight) (unsigned char, unsigned char);
    int (*sysdep_get_cpu_load) (pix_sys_cpu_t *);
    int (*sysdep_get_memory_usage) (pix_sys_memory_t *);
    int (*sysdep_get_battery) (pix_sys_battery_t *);
    int (*sysdep_get_net_value) (char *, char *);
    int (*sysdep_write_net_values) (pix_sys_ipaddr_str_t, pix_sys_dns_t);
}
pix_sys_func_t;

#endif
