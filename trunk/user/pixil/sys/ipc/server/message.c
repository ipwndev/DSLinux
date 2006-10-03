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
#include <string.h>
#include <errno.h>

#include <ipc/colosseum.h>
#include "server.h"

extern int cl_chatty;

int
handle_server_message(cl_app_struct * app, cl_pkt_message * msg)
{
    return (0);
}

static int
deliver_message(cl_app_struct * dest, cl_pkt_buff * pkt)
{

    if (!pkt || !dest)
	return (-1);

    if (!data_in_queue(dest)) {
	int ret = cl_ClientWrite(dest->cl_socket, pkt->data, pkt->len);

	/* If the write was blocked, then queue the packet for later */

	if (ret == 1) {
	    queue_add_packet(dest, pkt);
	    dest->cl_flags |= CL_F_QUEUE_MSG;
	    return (1);
	}

	return (0);		/* Tell the calling function to free the buffer */
    }

    queue_add_packet(dest, pkt);
    return (1);
}

/* Try to send as many queued messages as possible */

int
cl_HandleQueuedMessages(cl_app_struct * app)
{

    cl_pkt_buff *pkt;

    if (!app)
	return (-1);

    /* Temporary clear the block */
    app->cl_flags &= ~CL_F_QUEUE_MSG;

    while ((pkt = queue_peek_packet(app))) {
	int ret = cl_ClientWrite(app->cl_socket, pkt->data, pkt->len);

	/* If there is an error, then reinstate the block and bail */
	if (ret == 1) {
	    app->cl_flags |= CL_F_QUEUE_MSG;
	    return (0);
	}

	/* If it sent succesfully, then actually pull the packet */
	/* and free it */

	pkt = queue_get_packet(app);
	pkt_buff_free(pkt);
    }

    return (0);
}

int
cl_SendMessage(cl_app_struct * req, cl_pkt_buff * pkt)
{

    int stored = 0;
    int ret = 0;

    cl_app_struct *dest;
    cl_pkt_message *msg = pkt->data;

    cl_pkt_msg_response response;

    bzero(&response, sizeof(response));

    response.header.type = CL_PKT_MSG_RESPONSE;
    response.header.len = sizeof(cl_pkt_msg_response);

    if (msg->dest == CL_MSG_SERVER_ID) {
	if (cl_chatty)
	    DO_LOG(CL_LOG_DEBUG, "SERVER MSG [%s]", req->cl_name);

	handle_server_message(req, msg);

	/* It is important to send *something* back to the client. */
	cl_SendPacket(req, (cl_packet *) & response,
		      sizeof(cl_pkt_msg_response));

	return (0);
    }

    /* Broadcast */

    if (msg->dest == CL_MSG_BROADCAST_ID) {

	if (cl_chatty)
	    DO_LOG(CL_LOG_DEBUG, "BROADCAST MSG [%s]\n", req->cl_name);

	dest = get_first_app();

	while (dest) {

	    if (dest != req && ISSET_FLAG(dest->cl_flags, CL_F_ACTIVE)) {
		if (deliver_message(dest, pkt) == 1) {
		    stored++;
		    PKT_BUFF_ADD_OWNER(pkt);
		}
	    }

	    dest = get_next_app(dest);
	}

	/* It is important to send *something* back to the client. */
	cl_SendPacket(req, (cl_packet *) & response,
		      sizeof(cl_pkt_msg_response));
	return ((stored ? 1 : 0));
    }

    /* Regular delivery */

    dest = get_app_by_id(msg->dest);

    if (!dest) {
	DO_LOG(CL_LOG_ERROR, "WARNING:  IPC id [%d] doesn't exist!",
	       msg->dest);

	/* On a bad destination, we inform the source.  This is used to detect
	   clients that may have gone away.  This is also why we send a dummy
	   response on all the other paths - the client is waiting for a response
	 */

	printf("Seending a CL_E_NODEST to the client\n");
	CL_ERROR(req, CL_E_NODEST, &response);
	return (-1);
    }

    DO_LOG(CL_LOG_DEBUG, "MSG [%s] to [%d]\n", req->cl_name, msg->dest);

    msg->src = req->cl_id;
    ret = deliver_message(dest, pkt);

    /* It is important to send *something* back to the client. */
    cl_SendPacket(req, (cl_packet *) & response, sizeof(cl_pkt_msg_response));
    return (ret);
}
