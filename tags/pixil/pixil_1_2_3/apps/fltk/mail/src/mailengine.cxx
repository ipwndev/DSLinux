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


/* MAILENGINE.CXX
   
   WRITTEN BY:  Jordan Crouse
                Century Software Embedded Technologies		
*/

#include <stdio.h>

#include "nxmail.h"
#include "mailengine.h"

int
mailEngine::message_count(void)
{
    int mcount;

    if (!_stream)
	return (0);

    if (nxmail_status(_stream, &mcount) != NXMAIL_OK)
	return (0);
    else
	return (mcount);
}

int
mailEngine::active(void)
{
    if (_stream)
	return (1);
    else
	return (0);
}

MAILERROR mailEngine::open_session(char *user, char *password)
{
    int
	ret;
    int
	mailtype =
	0;

    /* If no stream, then start a new one */
    if (!_stream) {
	_stream = nxmail_init_stream();

	if (!_stream) {
	    fprintf(stderr, "MAIL:  Unable to open a new stream\n");
	    return (MAIL_OPEN_ERROR);
	}
    }

    if (_type == MAIL_POP3)
	mailtype = NXMAIL_TYPE_POP3;

    ret = nxmail_open(_stream, _server, _port, mailtype);

    if (ret != NXMAIL_OK) {
	fprintf(stderr, "MAIL:  Unable to open %s:%d\n", _server, _port);
	_stream = 0;

	return (MAIL_OPEN_ERROR);
    }

    if (nxmail_auth(_stream, user, password) != NXMAIL_OK) {
	fprintf(stderr, "MAIL:  Unable to authorize %s!\n", user);
	nxmail_close(_stream);

	_stream = 0;
	return (MAIL_AUTH_ERROR);
    }

    return (MAIL_SUCCESS);
}

void
mailEngine::close_session(void)
{
    if (_stream)
	nxmail_close(_stream);
}

void
mailEngine::reset_engine(char *newserver, int newport, MAILTYPE newtype)
{
    _type = newtype;
    _port = newport;
    strcpy(_server, newserver);

    if (_stream)
	nxmail_close_stream(_stream);
}

nxmail_header_t *
mailEngine::fetch_header(int message)
{
    return (nxmail_fetchheader(_stream, message));
}

nxmail_body_t *
mailEngine::fetch_message(int message)
{
    return (nxmail_fetchbody(_stream, message));
}


int
mailEngine::delete_message(int message)
{
    return (nxmail_delete(_stream, message));
}

MAILERROR
    mailEngine::send_message(char *server, int port,
			     nxmail_header_t * header, char *body, int size)
{
    int ret;

    ret = nxmail_sendmsg(server, port, header, body, size);

    if (ret != NXMAIL_OK)
	return (MAIL_SEND_ERROR);
    else
	return (MAIL_SUCCESS);
}


mailEngine::mailEngine(char *server, int port, MAILTYPE type)
{
    _stream = 0;
    _type = type;
    _port = port;
    strcpy(_server, server);
}

mailEngine::~mailEngine()
{
    if (_stream)
	nxmail_close_stream(_stream);
    _stream = 0;
}
