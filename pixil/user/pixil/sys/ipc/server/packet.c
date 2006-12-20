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
#include <errno.h>
#include <unistd.h>

#include <ipc/colosseum.h>
#include "server.h"

/* Actually write a packet out to the socket */

int
cl_SendError(cl_app_struct * app, int err, cl_packet * pkt, int len)
{

    cl_pkt_header *head = &pkt->header;
    head->resp = err;

    return (cl_ClientWrite(app->cl_socket, (unsigned char *) pkt, len));
}

int
cl_SendPacket(cl_app_struct * app, cl_packet * pkt, int len)
{

    cl_pkt_header *head = &pkt->header;
    head->resp = 0;

    return (cl_ClientWrite(app->cl_socket, (unsigned char *) pkt, len));
}

int
cl_HandleClientReq(cl_app_struct * app)
{

    int result = -1;

    cl_pkt_buff *data;
    cl_packet *pkt;

    if (!app)
	return (-1);
    if (cl_ClientRead(app->cl_socket, &data))
	return (-1);

    if (!data) {
	DO_LOG(CL_LOG_ERROR, "Error - Bad cl_clientRead\n");
	return (-1);
    }

    pkt = (cl_packet *) data->data;

    DPRINT("IN (%s):  Size [%d] Type [%d]\n", app->cl_name, pkt->header.len,
	   pkt->header.type);

    switch (pkt->header.type) {

    case CL_PKT_MESSAGE:

	/* No error messages on send message for now */

#ifdef NOTUSED
	if (!ISSET_FLAG(app->cl_flags, CL_F_ACTIVE))
	    CL_ERROR(app, CL_E_APPUNKWN, (cl_pkt_message *) & pkt->message);
	else
#endif

	    result = cl_SendMessage(app, data);
	break;

    case CL_PKT_REGISTER:

	/* This packet is an error if it is already active */

	if (ISSET_FLAG(app->cl_flags, CL_F_ACTIVE))
	    CL_ERROR(app, CL_E_APPACTIVE, (cl_pkt_reg *) & pkt->reg);
	else
	    result = cl_HandleRegisterApp(app, (cl_pkt_reg *) & pkt->reg);

	break;

    case CL_PKT_STARTAPP:

	if (!ISSET_FLAG(app->cl_flags, CL_F_ACTIVE))
	    CL_ERROR(app, CL_E_APPUNKWN, (cl_pkt_start *) & pkt->start);
	else
	    result = cl_HandleStartApp(app, (cl_pkt_start *) & pkt->start);

	break;

    case CL_PKT_SPAWNAPP:
	result = cl_HandleSpawnApp(app, (cl_pkt_spawn *) & pkt->spawn);
	break;

    case CL_PKT_APP_INFO:
	result = cl_HandleAppInfo(app, (cl_pkt_appinfo *) & pkt->appinfo);
	break;

    case CL_PKT_FINDAPP:

	if (!ISSET_FLAG(app->cl_flags, CL_F_ACTIVE))
	    CL_ERROR(app, CL_E_APPUNKWN, (cl_pkt_findapp *) & pkt->findapp);
	else
	    result = cl_HandleFindApp(app, (cl_pkt_findapp *) & pkt->findapp);

	break;

#ifdef HAVE_LOGGING

    case CL_PKT_LOG:{
	    cl_pkt_log *log = &pkt->log;
	    cl_doLog(log->level, app->cl_name, log->message);
	}

	break;
#endif

    default:
	DO_LOG(CL_LOG_MESSAGE, "Unknown IPC packet %d\n", pkt->header.type);
	CL_ERROR(app, CL_E_BADCMD, pkt);
	break;
    }

    if (result != 1)
	pkt_buff_free(data);

    return (0);			/* No fatal errors, so return 0 */
}
