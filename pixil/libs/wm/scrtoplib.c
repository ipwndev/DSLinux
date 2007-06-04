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


/* This sends a generic message to the screentop */

#include <stdio.h>
#include <string.h>

#include <ipc/colosseum.h>
#include <wm/scrtoplib.h>

static int scrtopId = 0;

static int
sendScrtopMessage(scrtop_message * msg, int size)
{

    int id;

    /* See if we need to resolve the screentop id */
    if (!scrtopId) {
	id = ClFindApp("nxscrtop");
	if (id <= 0) {
	    fprintf(stderr,
		    "Error - Got error %d while talking to the screentop\n",
		    id);
	    return (-1);
	}

	scrtopId = id;
    }

    return (ClSendMessage(scrtopId, (void *) msg, size));
}

int
scrtopDoAction(int type, char *app)
{

    scrtop_action action;
    action.type = type;

    strncpy(action.app, app, sizeof(action.app) - 1);
    return (sendScrtopMessage((scrtop_message *) & action, sizeof(action)));
}
