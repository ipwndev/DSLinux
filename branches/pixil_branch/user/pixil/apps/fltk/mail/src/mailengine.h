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


#ifndef MAILENGINE_H
#define MAILENGINE_H

#include <string.h>
#include "nxmail.h"

typedef enum
{
    MAIL_POP3
}
MAILTYPE;

typedef enum
{
    MAIL_SUCCESS = 0,
    MAIL_OPEN_ERROR,
    MAIL_AUTH_ERROR,
    MAIL_SEND_ERROR
}
MAILERROR;

class mailEngine
{
  private:

    nxmail_stream * _stream;
    char _server[30];
    int _port;
    MAILTYPE _type;

  public:

      mailEngine(char *server, int port, MAILTYPE type);
     ~mailEngine();

    void reset_engine(char *, int, MAILTYPE);

    MAILERROR open_session(char *user, char *password);
    void close_session();

    int message_count();
    int active();

    nxmail_header_t *fetch_header(int);
    nxmail_body_t *fetch_message(int);
    int delete_message(int);

    void set_server(char *server)
    {
	strcpy(_server, server);
    }
    char *get_server(void)
    {
	return (_server);
    }

    void set_port(int port)
    {
	_port = port;
    }
    int get_port(void)
    {
	return (_port);
    }

    MAILERROR send_message(char *, int, nxmail_header_t *, char *, int);

    void set_type(MAILTYPE type)
    {
	_type = type;
    }
    MAILTYPE get_type(void)
    {
	return (_type);
    }
};

#endif
