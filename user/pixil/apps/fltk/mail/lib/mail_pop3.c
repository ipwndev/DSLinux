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


/*
 * NXMAIL - MAIL_POP3.C
 * Routines to access POP3 mailboxes 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>

#include "nxmail.h"
#include "net_util.h"

#define POP3_PORT 110

static int
pop3_command(int fd, char *in, int len)
{
    int outlen;

#ifdef DEBUG
    printf("CLIENT OUT (%d):  %s", len, in);
#endif

    if (tcp_wait_for_socket(fd, 10, TCP_WRITE) <= 0)
	return (-1);

    outlen = write(fd, in, len);

    if (outlen != len) {
	if (outlen == -1)
	    perror("POP3_COMMAND (WRITE)");

	SET_NET_ERROR(errno);
	return (-1);
    }

    return (0);
}

static int
pop3_response(int fd, char *out, int len, int wait)
{
    int ret;
    int lindex;

    if (tcp_wait_for_socket(fd, 10, TCP_READ) <= 0)
	return (-1);

    ret = fdgets(fd, out, len - 1);

    if (ret <= 0) {
	printf("Got back %d from ret!\n", ret);
	return (-1);
    }

    /* Strip the CRLF and just turn it into a CR */
    lindex = (strlen(out) - 1);

    if (out[lindex] == '\n' && out[lindex - 1] == '\r') {
	out[lindex - 1] = '\n';
	out[lindex] = 0;
    }
#ifdef DEBUG
    printf("CLIENT IN: %s", out);
#endif

    return (0);
}

#ifdef LOG
#define pop3_log(str)  fprintf(stderr, "LOG %s", str)
#else
#define pop3_log(str)
#endif

static int
pop3_do_command(nxmail_stream * stream, char *command,
		char *response, int reslen)
{
    int cmdlen = strlen(command);

    if (pop3_command(stream->fd, command, cmdlen) == -1)
	return (NXMAIL_NETWORK_ERROR);

    if (pop3_response(stream->fd, response, reslen, 1) == -1)
	return (NXMAIL_NETWORK_ERROR);

#ifdef LOG
    pop3_log(response);
#endif

    return (NXMAIL_OK);
}

static int
pop3_is_ok(char *response)
{
    if (response[0] == '+')
	return (1);
    else
	return (0);
}

static int
pop3_get_data(nxmail_stream * stream, char *buffer, int maxlen)
{
    char *pos = buffer;
    int copied = 0;

    char linebuf[1024];

    while (1) {
	char *lpos = 0;
	int lsize = 0;

	if (pop3_response(stream->fd, linebuf, 1024, 0) != NXMAIL_OK)
	    break;

	if (!strlen(linebuf))
	    break;
	if (!strcmp(linebuf, ".\n"))
	    break;

	lpos = linebuf;
	lsize = strlen(linebuf);

	if (linebuf[0] == '.') {
	    lpos++;
	    lsize--;
	}

	if (copied + lsize > maxlen) {
	    strncpy(pos, lpos, maxlen - copied);
	    break;
	}

	strncpy(pos, lpos, lsize);

	pos += lsize;
	copied += lsize;
    }

    return (copied);
}

int
pop3_open(char *address, int port, nxmail_stream * stream)
{
    char response[200];
    int res;
    int fd = tcp_open_stream(address, port);

    if (fd == -1)
	return (NXMAIL_NETWORK_ERROR);

    stream->fd = fd;

    /* Now wait for the server to respond */
    res = pop3_response(stream->fd, response, 200, 1);

    if (res == -1) {
	close(fd);
	stream->fd = 0;
	return (NXMAIL_NETWORK_ERROR);
    }
#ifdef DEBUG
    pop3_log(response);
#endif

    return (NXMAIL_OK);
}

int
pop3_login(nxmail_stream * stream, char *user, char *password)
{
    char command[100];
    char response[100];

    /* Send the user string */

#ifdef DEBUG
    printf("LOG: Sending USER command\n");
#endif

    sprintf(command, "USER %s\n", user);

    pop3_do_command(stream, command, response, 100);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

#ifdef DEBUG
    printf("LOG: Sending PASS command\n");
#endif

    sprintf(command, "PASS %s\n", password);

    pop3_do_command(stream, command, response, 100);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

    return (NXMAIL_OK);
}

void
pop3_close(nxmail_stream * stream)
{
    char response[10];

    if (stream->fd != 0) {
	pop3_do_command(stream, "QUIT\n", response, 10);

	close(stream->fd);
	stream->fd = 0;
    }

}

int
pop3_stat(nxmail_stream * stream, int *msgcount, int *msgsize)
{
    char response[100];

    pop3_do_command(stream, "STAT\n", response, 100);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

    sscanf(response, "+OK %d %d", msgcount, msgsize);
    return (NXMAIL_OK);
}

int
pop3_msginfo(nxmail_stream * stream, int msg)
{
    char command[100];
    char response[100];
    int tmp, msgsize;

    sprintf(command, "LIST %d\n", msg);

    pop3_do_command(stream, command, response, 100);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

    sscanf(response, "+OK %d %d", &tmp, &msgsize);
    return (msgsize);
}

/* This just gets the header, nice for those quick grabs */
/* Returns the read size of the buffer */

int
pop3_getheader(nxmail_stream * stream, int msg, char **buffer)
{
    char command[100];
    char response[100];

    int hsize;

    hsize = pop3_msginfo(stream, msg);

    sprintf(command, "TOP %d 0\n", msg);

    if (pop3_command(stream->fd, command, strlen(command)) == -1)
	return (NXMAIL_NETWORK_ERROR);

    if (pop3_response(stream->fd, response, 100, 1) == -1)
	return (NXMAIL_NETWORK_ERROR);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

    /* sscanf(response, "+OK %d", &hsize); */

    *buffer = (char *) malloc(hsize);
    if (!*buffer)
	return (NXMAIL_ERROR);

    return (pop3_get_data(stream, *buffer, hsize));
}

int
pop3_getbody(nxmail_stream * stream, int msg, char **buffer)
{
    char command[100];
    char response[100];
    int hsize;

    hsize = pop3_msginfo(stream, msg);

    /* Get the message */
    sprintf(command, "RETR %d\n", msg);

    if (pop3_command(stream->fd, command, strlen(command)) == -1)
	return (NXMAIL_NETWORK_ERROR);

    if (pop3_response(stream->fd, response, 100, 1) == -1)
	return (NXMAIL_NETWORK_ERROR);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);

    /* sscanf(response, "+OK %d", &hsize); */

    /* Local buffer ensures that we have a big enough area */
    /* to copy in a message with no breaks in it */

    *buffer = (char *) malloc(hsize);

    if (!*buffer)
	return (NXMAIL_ERROR);

    return (pop3_get_data(stream, *buffer, hsize));
}

int
pop3_delete(nxmail_stream * stream, int msg)
{
    char command[100];
    char response[100];

    sprintf(command, "DELE %d\n", msg);

    pop3_do_command(stream, command, response, 100);

    if (!pop3_is_ok(response))
	return (NXMAIL_ERROR);
    return (NXMAIL_OK);
}

/* This is the structure that the main API accesses to do POP3 functions */

nxmail_driver_t nxmail_pop3_driver = {
    pop3_open,
    pop3_login,
    pop3_stat,
    pop3_msginfo,
    pop3_getheader,
    pop3_getbody,
    pop3_delete,
    pop3_close
};
