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


#ifndef LINUX_IF_H
#define LINUX_IF_H

/*
 * LINUX_IF.H
 * Copyright 2000, Century Software Embedded Technologies
 */

/* 
 * DESCRIPTION:  This file contains the prototypes for
 *               the Linux OS specific functions
 *               
 * AUTHOR:       First draft, Jordan Crouse, 11/27/00
 */

/****** THIS FILE IS FOR INTERNAL LIBRARY USE ONLY ******/

int linux_do_ip_address(int, char *, pix_comm_ipaddr_t *);
int linux_do_default_gateway(int, unsigned long *, char *);

int linux_set_nameserver(char *, char *, char *, unsigned long *, int);
int linux_get_if_list(int, pix_comm_interface_t *, int *);
int linux_if_active(char *);
int linux_if_dhcp(char *);
int linux_set_if_status(int, char *);
int linux_set_netscript_values(char *file, const char *keys[],
			       const char *vals[], int size);
int linux_get_netscript_values(char *file, char *keys[], char *vals[],
			       int size);

#endif
