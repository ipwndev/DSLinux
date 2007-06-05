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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "nxmail.h"
#include "net_util.h"
#include "mail_smtp.h"

#define OK      0
#define FAILURE 1
#define ERROR   2

static char *SMTPProtoArray[] = { NULL, "HELO %s\n", "MAIL FROM:<%s>\n",
    "RCPT TO:<%s>\n", "DATA\n", "\n.\n", "QUIT\n"
};

static int smtp_handle_response(SMTPCommands state, int response);

static int
smtp_send_command(int fd, char *command, int len)
{
#ifdef DEBUG
    printf("SMTP:  Sending <%s> <%d>\n", command, len);
#endif

    if (tcp_wait_for_socket(fd, 5, TCP_WRITE) <= 0)
	return (-1);

    return (write(fd, command, len));
}

static int
smtp_get_response(int fd, char *response, int len)
{
    int lindex;
    int ret;

    if (tcp_wait_for_socket(fd, 5, TCP_READ) <= 0)
	return (-1);

    bzero(response, len);

    ret = fdgets(fd, response, len - 1);

    if (ret <= 0)
	return (-1);

    /* Strip the CRLF and just turn it into a CR */
    lindex = (strlen(response) - 1);

    if (response[lindex] == '\n' && response[lindex - 1] == '\r') {
	response[lindex - 1] = '\n';
	response[lindex] = 0;
    }
#ifdef DEBUG
    printf("SMTP:  Got <%s>", response);
#endif

    return (0);
}

static int
smtp_command(int fd, SMTPCommands cmd, char *data)
{
    char cmdstr[1024];
    char response[1024];

    if (SMTPProtoArray[cmd]) {
	bzero(cmdstr, 1024);

	if (data)
	    sprintf(cmdstr, SMTPProtoArray[cmd], data);
	else
	    strcpy(cmdstr, SMTPProtoArray[cmd]);

	if (smtp_send_command(fd, cmdstr, strlen(cmdstr)) == -1)
	    return (SMTP_NET_ERROR);
    }

    do {
	if (smtp_get_response(fd, response, 1024) == -1)
	    return (SMTP_NET_ERROR);
    }

    while (atoi(response) == 0);

    return (atoi(response));
}

static int
smtp_send_data(int fd, char *buffer, int bufferlen)
{
    int len;
    int outlen = 0;

    while (1) {
#ifdef DEBUG
	printf("SMTP:  Sending %s\n", buffer + outlen);
#endif

	if (tcp_wait_for_socket(fd, 5, TCP_WRITE) <= 0)
	    return (-1);

	len = write(fd, buffer + outlen, bufferlen - outlen);

	if (len <= 0) {
	    perror("SMTP_SEND_DATA (WRITE)\n");
	    return (-1);
	}

	if (len == (bufferlen - outlen))
	    break;
	outlen += len;
    }

    return (0);
}

int
smtp_send_message(char *server, int port, nxmail_header_t * header,
		  char *body, int bodylen)
{
    SMTPCommands state = 0;
    int addrok = 0;

    nxmail_address_t *tolist = &header->to;

    char *hostname;
    char str[100];
    int ret;

    int fd = tcp_open_stream(server, port);

    if (fd == -1)
	return (NXMAIL_SEND_ERROR);

    printf("SEND:  Opened up %d for socket\n", fd);

    hostname = tcp_get_hostname(fd);

    if (!hostname) {
	printf("SMTP:  Couldn't resolve the local host\n");
	close(fd);
	return (NXMAIL_SEND_ERROR);
    }

    state = SMTP_LOGON;
    ret = smtp_command(fd, state, 0);
    if (smtp_handle_response(state, ret) != OK)
	goto end_session;

    /* Send HELLO */
    state = SMTP_HELLO;

    ret = smtp_command(fd, state, hostname);
    if (smtp_handle_response(state, ret) != OK)
	goto end_session;

    /* Send MAIL FROM */
    state = SMTP_MAIL;

    sprintf(str, "%s@%s", header->from.mailbox, header->from.host);
    ret = smtp_command(fd, state, str);
    if (smtp_handle_response(state, ret) != OK)
	goto end_session;

    /* Send RECT TO: for all names on the list */
    state = SMTP_RECPT;

    while (tolist) {
	sprintf(str, "%s@%s", tolist->mailbox, tolist->host);
	ret = smtp_command(fd, state, str);

	switch (smtp_handle_response(state, ret)) {
	case ERROR:
	    goto end_session;
	case OK:
	    addrok++;
	}

	tolist->result = ret;
	tolist = tolist->next;
    }

    if (!addrok) {
	ret = SMTP_NO_MAILBOX;
	goto end_session;
    }

    /* Send DATA */
    state = SMTP_DATA;
    ret = smtp_command(fd, state, 0);
    if (smtp_handle_response(state, ret) != OK)
	goto end_session;

    smtp_send_data(fd, body, bodylen);

    state = SMTP_DATA_END;
    ret = smtp_command(fd, state, 0);

  end_session:

    printf("SEND: Sending quit!\n");

    smtp_command(fd, SMTP_QUIT, 0);
    close(fd);

    switch (smtp_handle_response(state, ret)) {
    case OK:
	return (NXMAIL_SEND_OK);
    case FAILURE:
	return (NXMAIL_SEND_FAILURE);
    default:
	return (NXMAIL_SEND_ERROR);
    }
}

/* This returns a OK, FAILURE, or ERROR depending on the result */

static int
smtp_handle_response(SMTPCommands state, int response)
{
    switch (state) {
    case SMTP_LOGON:
	if (response != SMTP_READY)
	    return FAILURE;
	return OK;

    case SMTP_HELLO:
	if (response != SMTP_OK)
	    return ERROR;
	return OK;

    case SMTP_MAIL:
	switch (response) {
	case SMTP_OK:
	    return (OK);
	case SMTP_MAILBOX_FULL:
	case SMTP_LOCAL_ERROR:
	case SMTP_SYSTEM_FULL:
	    return (FAILURE);
	default:
	    return (ERROR);
	}

    case SMTP_RECPT:
	switch (response) {
	case SMTP_OK:
	case SMTP_FORWARDED:
	    return (OK);
	case SMTP_SYNTAX_ERROR:
	case SMTP_BAD_ARGUMENT:
	case SMTP_BAD_SEQUENCE:
	case SMTP_NO_SERVICE:
	    return (ERROR);
	default:
	    return (FAILURE);
	}

    case SMTP_DATA:
	if (response != SMTP_READY_FOR_DATA)
	    return (ERROR);
	return (OK);

    case SMTP_DATA_END:
	if (response == SMTP_OK)
	    return (OK);
	return (ERROR);

    default:
	return (FAILURE);
    }
}
