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
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "net_util.h"

int nxmail_neterror = 0;

int
tcp_wait_for_socket(int fd, int len, int action)
{
    int ret;
    fd_set fdset;
    struct timeval timeval;

    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    timeval.tv_sec = len;
    timeval.tv_usec = 0;

    if (action == TCP_READ)
	ret = select(fd + 1, &fdset, 0, 0, &timeval);
    else
	ret = select(fd + 1, 0, &fdset, 0, &timeval);

    if (ret == -1)
	perror("TCP_WAIT_FOR_SOCKET (SELECT)");
    return (ret);
}

int
tcp_open_stream(char *address, int port)
{
    int ret;
    int fd;
    int serror;
    int slen = sizeof(serror);

    struct sockaddr_in outsock;
    struct hostent *host = gethostbyname(address);
    struct in_addr *hostaddr;

    if (!host)
	return (-1);
    hostaddr = (struct in_addr *) host->h_addr_list[0];

    fd = socket(PF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
	perror("TCP_OPEN_STREAM (SOCKET)");
	SET_NET_ERROR(errno);
	return (-1);
    }

    /* Set it non blocking */
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
	perror("TCP_OPEN_sTREAM (FCNTL)");
	SET_NET_ERROR(errno);
	close(fd);
	return (-1);
    }

    outsock.sin_family = AF_INET;
    outsock.sin_port = htons(port);
    outsock.sin_addr.s_addr = hostaddr->s_addr;

    /* A small change here.  A call to connect immediately */
    /* returns a EINPROGRESS, and then we wait for further events */
    /* to come in via poll or select */

    ret =
	connect(fd, (struct sockaddr *) &outsock, sizeof(struct sockaddr_in));

    if (ret == 0)
	return (fd);

    if (errno != EINPROGRESS) {
	perror("TCP_OPEN_STREAM (CONNECT)");
	SET_NET_ERROR(errno);
	close(fd);
	return (-1);
    }

    ret = tcp_wait_for_socket(fd, 5, TCP_WRITE);

    if (ret == -1) {
	SET_NET_ERROR(errno);
	close(fd);
	return (-1);
    }

    if (ret == 0) {
	printf("TCP_OPEN_STREAM - TIMEOUT ON CONNECT\n");
	close(fd);
	return (-1);
    }

    /* No error or timeout, check to see if there was an error */
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &serror, &slen);

    if (serror != 0) {
	printf("TCP_OPEN_STREAM:  %d\n", serror);
	SET_NET_ERROR(serror);
	close(fd);
	return (-1);
    }

    return (fd);
}

char *
tcp_get_hostname(int fd)
{
    struct sockaddr_in saddr;
    struct hostent *host;
    unsigned long addr;

    int slen = sizeof(struct sockaddr_in);

    if (getsockname(fd, (struct sockaddr *) &saddr, &slen) == -1) {
	perror("TCP_GET_HOSTNAME (GETSOCKNAME)");
	SET_NET_ERROR(errno);
	return (0);
    }

    addr = saddr.sin_addr.s_addr;

    host = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);

    if (!host) {
	return (0);
    }

    return (host->h_name);
}

/* Read up to len or until we are out of data, which */
/* ever comes first */

int
fdgets(int fd, char *buffer, int len)
{
    int outsize = 0;

    char *ptr = buffer;

    while (1) {
	int res = read(fd, ptr, 1);

	if (res <= 0) {
	    if (res == -1 && errno != EAGAIN) {
		perror("FDGETS (READ)");
		return (-1);
	    }

	    *ptr = 0;
	    return (outsize);
	}

	if (*ptr == '\n') {
	    if (outsize + 1 < len)
		*(ptr + 1) = 0;
	    return (outsize);
	}

	ptr++;
	if (outsize++ == len)
	    return (len);
    }
}
