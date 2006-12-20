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


#ifndef LINUX_PPP_H
#define LINUX_PPP_H

/*
 * LINUX_PPP.H
 * Copyright 2000, Century Software Embedded Technologies
 */

/* 
 * DESCRIPTION:  This file contains the prototypes for
 *               The linux specific PPP functions 
 *               
 * AUTHOR:       First draft, Jordan Crouse, 11/29/00
 */

/****** THIS FILE IS FOR INTERNAL LIBRARY USE ONLY ******/

int linux_ppp_connect(int, pix_comm_ppp_options_t *, char **, int, int,
		      char *);
int linux_ppp_disconnect(char *);
int linux_ppp_get_stats(char *, pix_comm_ppp_stats_t *);
int linux_ppp_write_info(char *, pix_comm_ppp_info_t *);
int linux_ppp_read_info(char *, pix_comm_ppp_info_t *);
int linux_ppp_ip_address(char *, pix_comm_ppp_options_t *);

#endif
