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


#ifndef PIX_COMM_PROTO_H
#define PIX_COMM_PROTO_H

/*
 * PIX_COMM_PROTO.H
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

#define PIX_COMM_PROTO_NO_FUNCTION 0x0	/* Use this when no function is available */

typedef struct _pix_commproto
{
    int (*sysdep_do_ip_address) (int, char *, pix_comm_ipaddr_t *);
    int (*sysdep_do_default_gateway) (int, unsigned long *, char *);
    int (*sysdep_set_nameserver) (char *, char *, unsigned long *, int);
    int (*sysdep_ppp_connect) (int, pix_comm_ppp_options_t *, char **, int,
			       int, char *);
    int (*sysdep_ppp_disconnect) (char *);
    int (*sysdep_get_if_list) (int, pix_comm_interface_t *, int *);
    int (*sysdep_if_active) (char *);
    int (*sysdep_set_if_status) (int, char *);
    int (*sysdep_if_dhcp) (char *);
    int (*sysdep_ppp_get_stats) (char *, pix_comm_ppp_stats_t *);
    int (*sysdep_ppp_write_info) (char *, pix_comm_ppp_info_t *);
    int (*sysdep_ppp_read_info) (char *, pix_comm_ppp_info_t *);
    int (*sysdep_ppp_get_ip_info) (char *, pix_comm_ppp_options_t *);
    int (*sysdep_wireless_name) (char *, char *);
    int (*sysdep_wireless_encode) (int, char *, char *);
    int (*sysdep_wireless_essid) (int, char *, char *);
    int (*sysdep_wireless_get_stats) (char *, pix_comm_wirestats_t *);
    int (*sysdep_wireless_get_if_list) (pix_comm_interface_t *, int *);
    int (*sysdep_get_net_value) (char *, char *);
    int (*sysdep_write_net_values) (pix_comm_ipaddr_str_t, pix_comm_dns_t);
}
pix_comm_func_t;

#endif
