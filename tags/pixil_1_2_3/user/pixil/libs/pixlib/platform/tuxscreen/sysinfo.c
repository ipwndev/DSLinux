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
#include <string.h>

#include <pixlib/pixlib.h>


/*******************************************************************************\
**
**	Function:	int pix_sys_cpuinfo()
**	Desc:		Parses /proc/cpu info and organizes the data in a system independant
**				way
**	Accepts:	pixCpuInfo_t *pcpu = Ptr to the cpu info
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
pix_sys_cpuinfo(pixCpuInfo_t * pcpu)
{
    char buffer[512],		/* Buffer */
      model_name[80];
    float cpu_speed = 0.0;
    int rc = -1;		/* Return code */
    FILE *fp;			/* File pointer */

    /* Validate the incoming arguments */
    if (pcpu == NULL)
	return (rc);

    if ((fp = fopen("/proc/cpuinfo", "r")) == NULL)
	return (rc);

    /*
       ** Start parsing the file.  The only fields that are interesting are 
       ** "model name" and "cpu MHZ"
     */
    while (fgets(buffer, sizeof(buffer), fp)) {
	char *cp;
	int len = strlen(buffer);

	if (buffer[len - 1] == '\n')
	    buffer[--len] = '\0';

	/* Set up cp to point to the data */
	if ((cp = strchr(buffer, ':')) == NULL)
	    continue;
	if (!memcmp(buffer, "model name", 10)) {
	    cp += 2;		/* Skip the space */
	    strcpy(model_name, cp);
	    rc++;
	} /* end of if */
	else if (!memcmp(buffer, "cpu MHz", 7)) {
	    cp += 2;
	    cpu_speed = atof(cp);
	    rc++;
	}			/* end of if */
    }				/* end of while */

    if (rc > -1) {
	sprintf(pcpu->cpu, "%s %.0f MHz", model_name, cpu_speed);
	rc = 0;
    }
    /* end of if */
    return (rc);
}				/* end of pix_sys_cpuinfo() */
