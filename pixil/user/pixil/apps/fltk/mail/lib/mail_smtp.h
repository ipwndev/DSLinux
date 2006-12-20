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


#ifndef MAIL_SMTP_H
#define MAIL_SMTP_H

typedef enum
{
    SMTP_LOGON = 0,
    SMTP_HELLO,
    SMTP_MAIL,
    SMTP_RECPT,
    SMTP_DATA,
    SMTP_DATA_END,
    SMTP_QUIT
}
SMTPCommands;

/* SMTP ERROR MESSAGES */

#define SMTP_NET_ERROR        1
#define SMTP_READY            220
#define SMTP_CLOSED           221
#define SMTP_OK               250
#define SMTP_FORWARDED        251
#define SMTP_READY_FOR_DATA   354
#define SMTP_NO_SERVICE       421
#define SMTP_MAILBOX_UNAVAIL  450
#define SMTP_LOCAL_ERROR      451
#define SMTP_SYSTEM_FULL      452
#define SMTP_SYNTAX_ERROR     500
#define SMTP_BAD_ARGUMENT     501
#define SMTP_NO_COMMAND       502
#define SMTP_BAD_SEQUENCE     503
#define SMTP_PARAM_NOT_AVAIL  504
#define SMTP_NO_MAILBOX       550
#define SMTP_USR_NOT_LOCAL    551
#define SMTP_MAILBOX_FULL     552
#define SMTP_BAD_MAILBOX_NAME 553
#define SMTP_FAILURE          554

int smtp_send_message(char *, int, nxmail_header_t *, char *, int);
#endif
