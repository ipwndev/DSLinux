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
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/poll.h>

#include <ipc/colosseum.h>

static int g_socket = 0;

/* Define malloc debug if you are afraid that you aren't freeing all your mallocs */

#ifdef MALLOC_DEBUG
static int g_malloc = 0;

#define CALLOC(size, count) debug_calloc(size, count)
#define FREE(ptr) debug_free(ptr)
#else
#define CALLOC(size, count) calloc(size, count)
#define FREE(ptr) free(ptr)
#endif

/* Define DEBUG to get lots of annoying printfs */

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: "str, ## args)
#else
#define DPRINT(str, args...)
#endif

#define ERROR(str, args...) printf("ERROR: "str, ## args)


#ifdef MALLOC_DEBUG

static inline void *
debug_calloc(int size, int count)
{
    g_malloc++;
    return (calloc(size, count));
}

static inline void
debug_free(void *ptr)
{
    g_malloc--;
    return (free(ptr));
}

#endif

/* To better handle dynamic packets, we use this structure instead of 
  the queue structure 
*/

typedef struct pbuffer
{
    cl_packet *packet;
    int size;
    struct pbuffer *next;
}
pkt_buff_t;

static pkt_buff_t *pkt_queue = 0;

static inline void
pkt_free(pkt_buff_t * buf)
{
    if (!buf)
	return;
    if (buf->packet)
	FREE(buf->packet);
    FREE(buf);
}

static int
client_ServerDied(void)
{
    printf("The Colosseum server closed unexpectedly\n");
    ClClose();
    return (CL_CLIENT_NOSRVR);
}

static void
client_QueuePacket(pkt_buff_t * packet)
{

    pkt_buff_t *head = pkt_queue;

    if (!head)
	pkt_queue = packet;
    else {
	for (; head->next; head = head->next);
	head->next = packet;
    }

    DPRINT("Queued a packet of size [%d]\n", packet->size);
    packet->next = 0;
}

static int
client_DequeuePacket(pkt_buff_t ** ret)
{

    if (pkt_queue) {
	*ret = pkt_queue;
	pkt_queue = pkt_queue->next;
	return (1);
    }

    return (0);
}

static int
client_SendToServer(int sock, unsigned char *data, int len)
{

    int ret;

    ret = write(sock, data, len);

    if (ret <= 0) {
	if (ret == 0)
	    return (1);
	return (-1);
    }

    return (0);
}

/* Grab a packet off of the server */

int
client_GetFromServer(int sock, pkt_buff_t ** ret_buffer)
{

    int ret;
    int dsize = 0;
    pkt_buff_t *buffer;
    cl_pkt_header header;

    struct pollfd pstruct;

    pstruct.fd = sock;
    pstruct.events = POLLIN;

    ret = poll(&pstruct, 1, 0);

    /* No data is available */

    if (ret <= 0) {
      if (ret == -1)
	perror("client_GetFromServer");

      return (ret);
    }

    /* Read the header */
    ret = read(sock, &header, sizeof(header));

    if (ret <= 0) {
	if (errno == EAGAIN || ret == 0) {
	  return (0);
	}
	perror("client_GetFromServer");
	return (-1);
    }

    if (!header.len)
	return (0);

    buffer = *ret_buffer = (pkt_buff_t *) CALLOC(1, sizeof(pkt_buff_t));

    if (!buffer) {
	ERROR("Error - Out of memory in client_GetFromServer\n");
	return (-1);
    }

    buffer->packet = (cl_packet *) CALLOC(header.len, 1);
    dsize = header.len - sizeof(header);

    if (!buffer->packet) {
	pkt_free(buffer);
	ERROR("Error - Out of memory in client_GetFromServer\n");
	return (-1);
    }

    buffer->size = header.len;
    memcpy(buffer->packet, &header, sizeof(header));

    /* Now, read in the rest of the packet */

    ret =
	read(sock, (unsigned char *) buffer->packet + sizeof(header), dsize);

    if (ret != dsize) {

	/* No matter what, free the existing packet buffer */
	pkt_free(buffer);

	if (ret < 0) {
	    if (errno == EAGAIN)
		return (0);

	    perror("client_GetFromServer");
	    return (-1);
	} else {
	    ERROR("bad packet size - expected [%d], got [%d].\n", dsize, ret);
	    return (-1);
	}
    }

    return (header.len);
}

static int
client_WaitForData(int sock, int timeout)
{

    fd_set iset, eset;

    FD_ZERO(&iset);
    FD_ZERO(&eset);

    FD_SET(sock, &iset);
    FD_SET(sock, &eset);

    if (timeout) {
	struct timeval tv;
	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;

	return (select(sock + 1, &iset, 0, &eset, &tv));
    } else
	return (select(sock + 1, &iset, 0, &eset, 0));
}

static int
client_WaitForServer(int sock, int packet_type,
		     pkt_buff_t ** pkt, int timeout)
{

    int qcount = 0;
    *pkt = 0;

    do {
	pkt_buff_t *buffer = 0;
	cl_packet *data;
	int ret;

	/* Wait for the data to come in on the line */
	ret = client_WaitForData(sock, timeout);

	if (ret == 0)
	    return (CL_CLIENT_TIMEOUT);
	else if (ret == -1) {
	    return (CL_CLIENT_ERROR);
	}

	ret = client_GetFromServer(sock, &buffer);

	if (ret < 0) {
	    return (CL_CLIENT_ERROR);
	}

	else if (ret == 0)
	    return (CL_CLIENT_TIMEOUT);

	data = (cl_packet *) buffer->packet;

	if (data->header.type == packet_type) {
	    *pkt = buffer;
	    if (qcount)
		write(sock, 0, 0);
	    return (0);
	}

	if (data->header.type == CL_PKT_MESSAGE) {
	    client_QueuePacket(buffer);
	    DPRINT("While waiting for a %d, a Message packet was queued\n",
		   packet_type);
	    qcount++;
	} else
	    ERROR("Got type [%d] when expecting type [%d]\n",
		  data->header.type, packet_type);

    } while (1);
}

static int
client_MakeRequest(int sock, int packet_type,
		   cl_packet * packet, int len, int timeout)
{

    pkt_buff_t *buffer = 0;

    int ret;
    int size = 0;

    packet->header.type = packet_type;
    packet->header.len = len;

    ret = client_SendToServer(sock, (unsigned char *) packet, len);

    if (ret == -1) {
	return (CL_CLIENT_ERROR);
    } else if (ret == 1)
	return (client_ServerDied());

    bzero(packet, len);
    ret = client_WaitForServer(sock, packet_type, &buffer, timeout);

    if (ret < 0 || buffer == 0) {
	ERROR("Got an error from client_WaitForServer\n");
	pkt_free(buffer);
	return (ret);
    }

    /* This is where we actually transfer the packet */

    if (buffer->size != len)
	ERROR("Unexpected size [%d] in client_MakeRequest\n", buffer->size);

    if (buffer->size > CL_MAX_PKT_SIZE) {
	ERROR("I refuse to try and handle a packet of size [%d]\n",
	      buffer->size);
	pkt_free(buffer);
	return (CL_CLIENT_INVALID);
    }

    size = (len > buffer->size) ? buffer->size : len;

    bzero(packet, len);
    memcpy(packet, buffer->packet, size);

    pkt_free(buffer);		/* Free the buffer - very important */
    return (size);
}

static int
local_Register(unsigned char *name, int inflags, int *outflags)
{

    struct sockaddr_un saddr;
    cl_pkt_reg pkt;
    int ret;

    if (!name)
	return (CL_CLIENT_INVALID);
    if (g_socket)
	return (CL_CLIENT_CONNECTED);

    /* First, create a socket and try to connect */

    g_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (!g_socket) {
	perror("Socket Error: ");
	return (CL_CLIENT_SOCK_ERROR);
    }

    /* Try to connect to the named socket */

    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, CL_NAMED_SOCKET, sizeof(saddr.sun_path));

    /* Try to connect.  If the connection is refused, then we will */
    /* assume that no server is available */

    if (connect(g_socket, (struct sockaddr *) &saddr, sizeof(saddr)) == -1) {
	close(g_socket);
	g_socket = 0;
	if ((errno == ECONNREFUSED) || (errno == ENOENT))
	    return (CL_CLIENT_NOSRVR);
	else
	    return (CL_CLIENT_SOCK_ERROR);
    }
#ifdef NOTUSED
    /* Now, make the socket non blocking */
    if (fcntl(g_socket, F_SETFL, O_NONBLOCK)) {
	close(g_socket);
	g_socket = 0;
	return (CL_CLIENT_SOCK_ERROR);
    }
#endif

    bzero(&pkt, sizeof(pkt));
    strncpy(pkt.name, name, CL_MAX_NAME_LEN);

    pkt.start_flags = inflags;

    ret = client_MakeRequest(g_socket, CL_PKT_REGISTER, (cl_packet *) & pkt,
			     sizeof(pkt), 0);

    if (ret < 0)
	return (ret);

    switch (pkt.header.resp) {
    case CL_E_APPEXISTS:
	return (CL_CLIENT_BADNAME);

    case 0:
	if (outflags)
	    *outflags = pkt.start_flags;
	return (g_socket);

    default:
	printf("Error %d returned from server\n", pkt.header.resp);
	return (CL_CLIENT_ERROR);
    }
}

int
ClRegister(unsigned char *name, int *flags)
{
    return (local_Register(name, CL_NORMAL_CLIENT, flags));
}

int
ClReconnect(unsigned char *name)
{
    g_socket = 0;
    return (local_Register(name, CL_NORMAL_CLIENT, 0));
}

int
ClClose(void)
{

    pkt_buff_t *head = pkt_queue;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);

    /* Free any queued packets */

    while (head) {
	pkt_buff_t *next = head->next;
	pkt_free(head);
	head = next;
    }

    close(g_socket);
    g_socket = 0;

#ifdef MALLOC_DEBUG
    printf("MALLOC_DEBUG:  The end result was %d\n", g_malloc);
#endif

    return (0);
}

int
ClFindApp(unsigned char *name)
{

    cl_pkt_findapp pkt;
    int ret;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!name)
	return (CL_CLIENT_INVALID);

    bzero(&pkt, sizeof(pkt));

    /* Construct the find app packet */
    strncpy(pkt.name, name, CL_MAX_NAME_LEN);

    ret = client_MakeRequest(g_socket, CL_PKT_FINDAPP,
			     (cl_packet *) & pkt, sizeof(pkt), 0);

    if (ret < 0)
	return (ret);

    switch (pkt.header.resp) {
    case CL_E_NOAPP:
	return (CL_CLIENT_NOTFOUND);

    case 0:
	return (pkt.ipc_id);

    default:
	printf("Error %d returned from server\n", pkt.header.resp);
	return (CL_CLIENT_ERROR);
    }
}

int
ClStartApp(unsigned char *name, unsigned char *args, int flags, int timeout)
{

    cl_pkt_start pkt;
    int ret;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!name)
	return (CL_CLIENT_INVALID);

    bzero(&pkt, sizeof(pkt));

    /* Construct the find app packet */
    strncpy(pkt.name, name, CL_MAX_NAME_LEN);

    if (args)
	strncpy(pkt.argstr, args, CL_MAX_ARG_LEN);

    pkt.timeout = timeout;
    pkt.start_flags = flags;

    /* Wait up to timeout seconds */
    ret = client_MakeRequest(g_socket, CL_PKT_STARTAPP,
			     (cl_packet *) & pkt, sizeof(pkt),
			     timeout * 1000000);

    if (ret < 0)
	return (ret);

    switch (pkt.header.resp) {
    case CL_E_NOAPP:
    case CL_E_APPERR:
	return (CL_CLIENT_NOTFOUND);

    case 0:
	return (pkt.ipc_id);

    default:
	printf("Error %d returned from server\n", pkt.header.resp);
	return (CL_CLIENT_ERROR);
    }

}

int
ClSpawnApp(unsigned char *name, unsigned char *args)
{

    cl_pkt_spawn pkt;
    int ret;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!name)
	return (CL_CLIENT_INVALID);

    bzero(&pkt, sizeof(pkt));

    /* Construct the find app packet */
    strncpy(pkt.name, name, CL_MAX_NAME_LEN);

    if (args)
	strncpy(pkt.argstr, args, CL_MAX_ARG_LEN);

    /* Wait up to timeout seconds */

    ret = client_MakeRequest(g_socket, CL_PKT_SPAWNAPP,
			     (cl_packet *) & pkt, sizeof(pkt), 0);

    if (ret < 0)
	return (ret);

    switch (pkt.header.resp) {
    case CL_E_SPAWNERR:
	return (CL_CLIENT_NOTFOUND);

    case 0:
	return (pkt.pid);

    default:
	printf("Error %d returned from server\n", pkt.header.resp);
	return (CL_CLIENT_ERROR);
    }
}

int
clGetAppInfo(cl_app_info * info)
{

    int ret;
    cl_pkt_appinfo pkt;

    pkt.flags = info->flags;

    switch (info->flags) {
    case CL_APP_INFO_NAME:
	strcpy(pkt.name, info->name);
	break;
    case CL_APP_INFO_PID:
	pkt.processid = info->processid;
	break;
    }

    /* Wait 500 ms for a response */
    ret = client_MakeRequest(g_socket, CL_PKT_APP_INFO,
			     (cl_packet *) & pkt, sizeof(pkt), 0);

    if (ret < 0)
	return (ret);

    if (pkt.header.resp == CL_E_NOSUCHAPP)
	return (CL_CLIENT_NOTFOUND);

    info->flags = pkt.flags;
    info->processid = pkt.processid;
    strcpy(info->name, pkt.name);

    return (0);
}

int
ClSendMessage(int id, void *message, int len)
{

    pkt_buff_t *buffer = 0;
    cl_packet *response = 0;
    cl_pkt_message *pkt = 0;
    int ret = 0;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!message)
	return (CL_CLIENT_INVALID);

    if (!len)
	return (0);

    /* Allocate enough room for the packet */
    /* Note:  This must be on the heap because it can be pretty big */

    if (MESSAGE_PKT_SIZE(len) > CL_MAX_PKT_SIZE) {
	ERROR("Message size [%d] is too big - Rejecting it\n", len);
	return (CL_CLIENT_INVALID);
    }

    pkt = CALLOC(MESSAGE_PKT_SIZE(len), 1);

    pkt->dest = id;
    pkt->msglen = len;

    memcpy(&pkt->message, message, len);

    pkt->header.type = CL_PKT_MESSAGE;
    pkt->header.len = MESSAGE_PKT_SIZE(len);

    /* Send the packet */

    ret =
	client_SendToServer(g_socket, (unsigned char *) pkt,
			    MESSAGE_PKT_SIZE(len));

    if (ret == -1) {
	FREE(pkt);
	return (CL_CLIENT_ERROR);
    }

    else if (ret == 1) {
	FREE(pkt);
	return (client_ServerDied());
    }

    ret = client_WaitForServer(g_socket, CL_PKT_MSG_RESPONSE, &buffer, 0);

    if (ret < 0 || buffer == 0) {
	pkt_free(buffer);
	FREE(pkt);
	return (CL_CLIENT_ERROR);
    }

    response = (cl_packet *) buffer->packet;

    switch (response->header.resp) {
    case CL_E_NODEST:
	ret = CL_CLIENT_NODEST;
	break;

    case 0:
	ret = 0;
	break;

    default:
	ret = CL_CLIENT_ERROR;
	break;
    }

    FREE(pkt);
    pkt_free(buffer);

    return (ret);
}

int
ClLookupName(int id, unsigned char *name, int *len)
{

    int ret;
    cl_pkt_findname pkt;

    bzero(&pkt, sizeof(pkt));

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!name || !len || !*len)
	return (CL_CLIENT_INVALID);

    /* Construct the find app packet */
    pkt.id = id;

    /* Wait 500 ms for a response */
    ret = client_MakeRequest(g_socket, CL_PKT_FINDNAME,
			     (cl_packet *) & pkt, sizeof(pkt), 0);

    if (ret < 0)
	return (ret);

    strncpy(name, pkt.name, *len);

    *len = strlen(pkt.name);
    return (0);
}

int
ClGetMessage(void *msg, int *len, unsigned short *src)
{

    pkt_buff_t *buffer = 0;
    cl_pkt_message *msgpkt;

    int wlen;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!msg || !len || !*len)
	return (CL_CLIENT_INVALID);

    /* It there is something in the queue, get it there first */
    if (client_DequeuePacket(&buffer) == 0) {
	int rlen = client_GetFromServer(g_socket, &buffer);

	if (rlen == 0) {
	  printf("CLCLIENT:  client_GetFromServer() returned 0\n");
	  return (CL_CLIENT_NODATA);
	}
	else if (rlen == -1)
	    return (CL_CLIENT_ERROR);
    }

    msgpkt = (cl_pkt_message *) buffer->packet;

    /* This shouldn't happen, but it occasionally does */

    if (msgpkt->header.type == CL_PKT_MSG_RESPONSE) {
      printf("CLCLIENT:  Invalid header type\n");
      return (CL_CLIENT_NODATA);
    }

    wlen = (msgpkt->msglen > *len ? *len : msgpkt->msglen);

    memcpy(msg, &msgpkt->message, wlen);
    *len = wlen;

    *src = msgpkt->src;

    pkt_free(buffer);

    return (msgpkt->dest == CL_MSG_BROADCAST_ID)
	? CL_CLIENT_BROADCAST : CL_CLIENT_SUCCESS;
}

int
ClGetNextMessage(void *msg, int *len)
{

    int msgsrc = 0;

    pkt_buff_t *buffer = 0;
    cl_pkt_message *pkt;
    int wlen;

    /* It there is something in the queue, get it there first */

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!msg || !len || !*len)
	return (CL_CLIENT_INVALID);

    if (!client_DequeuePacket(&buffer)) {
	int ret = client_WaitForServer(g_socket, CL_PKT_MESSAGE, &buffer, 0);

	if (ret == 1)
	    return (client_ServerDied());
	else if (ret == -1)
	    return (CL_CLIENT_ERROR);
    }

    pkt = (cl_pkt_message *) buffer->packet;

    wlen = (pkt->msglen > *len ? *len : pkt->msglen);
    memcpy(msg, &pkt->message, wlen);
    *len = wlen;
    msgsrc = pkt->src;

    pkt_free(buffer);
    return (msgsrc);
}

#ifdef HAVE_LOGGING

int
ClLogMessage(int level, unsigned char *message)
{

    cl_pkt_log pkt;
    int ret;

    if (!g_socket)
	return (CL_CLIENT_NOCONN);
    if (!message)
	return (CL_CLIENT_INVALID);

    bzero(&pkt, sizeof(pkt));

    pkt.level = level;
    strncpy(pkt.message, message, CL_MAX_LOG_LEN);

    pkt.header.type = CL_PKT_LOG;
    pkt.header.len = sizeof(pkt);

    ret = client_SendToServer(g_socket, (unsigned char *) &pkt, sizeof(pkt));

    return (ret);
}

int
ClRegisterLogger(char *name)
{
    return (local_Register(name, CL_LOG_CLIENT, 0));
}

#else /* HAVE_LOGGING */

int
ClLogMessage(int level, unsigned char *name)
{
    return (CL_E_NOTIMPLEMENT);
}

int
ClRegisterLogger(char *name)
{
    return (CL_E_NOTIMPLEMENT);
}

#endif /* HAVE_LOGGING */
