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


#include <string.h>
#include "mime.h"

static struct
{
    char mimetype[50];
    MIMESUPPORT supported;
}
supported_mime[] =
{
    {
    "application/octet-stream", MIME_SAVE}
    ,				/* Save programs externally */
    {
    "application/x-gzip", MIME_SAVE}
    , {
    "message/delivery-status", MIME_TEXT}
    , {
    "message/http", MIME_TEXT}
    , {
    "message/partial", MIME_TEXT}
    , {
    "message/rfc822", MIME_TEXT}
    , {
    "multipart/report", MIME_TEXT}
    , {
    "text/plain", MIME_TEXT}
    , {
    "text/rfc822-headers", MIME_TEXT}
    , {
    "text/html", MIME_TEXT}
    , {
    "%%%", MIME_NOT_SUPPORTED}
};

MIMESUPPORT
check_mime_support(char *mimestr)
{
    int count = 0;

    while (1) {
	if (!strcmp(supported_mime[count].mimetype, "%%%"))
	    return (MIME_NOT_SUPPORTED);

	if (!strcmp(supported_mime[count].mimetype, mimestr))
	    return (supported_mime[count].supported);

	count++;
    }
}
