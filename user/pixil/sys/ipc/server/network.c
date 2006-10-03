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
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <ipc/colosseum.h>
#include "server.h"

/* This is a wrapper that handles any exceptions */

void
cl_CloseClient(int sock)
{
    cl_CloseApp(get_app_by_socket(sock));
    close(sock);
}

int
cl_ClientWrite(int fd, unsigned char *buffer, int len)
{

    int out = len;

    while (out) {
	int ret = write(fd, buffer, len);

	if (ret <= 0) {
	    if (errno == EAGAIN || errno == EINTR)
		continue;

	    /* Invalid agument messages are expected when clients go away */
	    /* Don't log the message (or we'll see tons of messages) */

	    if (errno != EINVAL) {
		DO_LOG(CL_LOG_ERROR, "Linux error [%s] while writing [%d]\n",
		       strerror(errno), fd);
	    }

	    cl_CloseClient(fd);
	    return (-1);
	}

	out -= ret;
    }

    return (0);
}

int
cl_ClientRead(int fd, cl_pkt_buff ** pkt)
{

    cl_pkt_header header;
    unsigned char *buffer = 0;
    int ret;
    int dsize = 0;

    /* Read the header first to get the size */
    ret = read(fd, &header, sizeof(header));

    if (ret <= 0) {
	switch (errno) {
	case EAGAIN:
	    return -1;		/* Dont do anything rash, just try to read again */

	    /* Normally, this is due to a SIGPIPE.  */

	    /* Note: We could do a fancy signal handler scheme to verify
	       that this EINTR was due to a "bad" signal, but I'm going
	       to go ahead and assume that any EINTR at this point is 
	       not good.  I am going to stick a debug message in though,
	       so we can determine what is happening */

	case EINTR:
	    printf
		("Got an EINTR.  This is probably a SIGPIPE.  Closing client.\n");
	    break;

	case EINVAL:
	case EBADF:
	case ECONNRESET:
	    /* Common messages when a client disapears */
	    break;

	default:
	    DO_LOG(CL_LOG_ERROR, "Linux error [%d] [%s] while reading [%d]\n",
		   errno, strerror(errno), fd);
	}

	cl_CloseClient(fd);

	*pkt = 0;

	return -1;
    }

    /* Now, allocate enough temporary room to hold the whole packet */
    buffer = (unsigned char *) malloc(header.len);

    if (!buffer) {
	DO_LOG(CL_LOG_ERROR, "Out of memory in cl_ClientRead\n");
	return (-1);
    }

    dsize = header.len - sizeof(header);

    /* Copy the header over */
    memcpy(buffer, (void *) &header, sizeof(header));

    /* And read the rest of the packet */
    ret = read(fd, (void *) buffer + sizeof(header), dsize);

    if (ret != dsize) {
	if (ret < 0)
	    DO_LOG(CL_LOG_ERROR, "Error %d on read.  Shutting down fd %d.\n",
		   errno, fd);
	else
	    DO_LOG(CL_LOG_ERROR, "Bad packet size (got %d, expected %d).\n",
		   ret, dsize);

	free(buffer);
	cl_CloseClient(fd);
	return (-1);
    }

    *pkt = pkt_buff_alloc(buffer, header.len);
    free(buffer);
    return (0);
}

int
cl_InitServerSocket(void)
{

    int lsock;
    struct sockaddr_un saddr;

    /* Check if the temporary file already exists. */

    if (!access(CL_NAMED_SOCKET, F_OK))
	if (unlink(CL_NAMED_SOCKET))
	    return (-1);

    if ((lsock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	perror("create socket");
	return (-1);
    }

    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, CL_NAMED_SOCKET, sizeof(saddr.sun_path));

    if (bind(lsock, (struct sockaddr *) &saddr, SUN_LEN(&saddr)) < 0) {
	perror("bind socket");
	goto exit_init;
    }

    if (listen(lsock, 5) == -1) {
	perror("listen socket");
	goto exit_init;
    }

    return (lsock);

  exit_init:

    if (lsock)
	close(lsock);
    return (-1);
}

int
cl_HandleNewClient(int srvsock)
{

    struct sockaddr_un saddr;
    int slen = sizeof(saddr);
    int clsock = 0;

    if ((clsock = accept(srvsock, (struct sockaddr *) &saddr, &slen)) == -1) {
	perror("accept");
	return (-1);
    }

    /* Allocate the application structure */
    if (!cl_AllocNewApp(clsock))
	return (-1);

    return (0);
}
