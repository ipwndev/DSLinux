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



/* Feature test switches */


/* System header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local header files */
#include <pixlib/pixlib.h>


/* Typedef, macro, enum/struct/union definitions */


/* Global scope variables */


/* Static scope variables */


/* Static function prototypes */

/*******************************************************************************\
**
**	Static function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	External Function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	Function:	int pix_sys_meminfo()
**	Desc:		Gets the system memory information from the /proc/meminfo file
**	Accepts:	struct pixMemInfo_t *pmi = Ptr to memory information
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
pix_sys_meminfo(pixMemInfo_t * pmi)
{
    char membuf[512];		/* Memory Buffer */
    int rc = -1;		/* Return code */
    unsigned long a, b, c, d, e, f;
    FILE *fp;			/* File pointer */

    if (pmi == NULL)
	return (rc);

    memset(pmi, 0, sizeof(pixMemInfo_t));

    if ((fp = fopen("/proc/meminfo", "r")) == NULL)
	return (rc);


    /* Parse the data */
    fgets(membuf, sizeof(membuf), fp);
    if (fscanf(fp, "%*s %ld %ld %ld %ld %ld %ld", &a, &b, &c, &d, &e, &f) > 0) {
	pmi->mtotal = a;
	pmi->mused = b;
	pmi->mfree = c;
	pmi->mshare = d;
	pmi->mbuffer = e;
	pmi->mcache = f;
	if (fscanf(fp, "%*s %ld %ld %ld", &a, &b, &c) > 0) {
	    pmi->stotal = a;
	    pmi->sused = b;
	    pmi->sfree = c;
	    rc = 0;
	}			/* end of if */
    }
    /* end of if */
    fclose(fp);

    return (rc);
}				/* end of pix_sys_meminfo() */

/*******************************************************************************\
**
**	Function:	int pix_sys_osinfo()
**	Desc:		Returns OS information
**	Accepts:	struct utsname *punm = Ptr to the structure to fill in
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
pix_sys_osinfo(struct utsname *punm)
{
    if (punm == NULL)
	return (-1);

    return (uname(punm));
}				/* end of pix_osinfo() */

/*******************************************************************************\
**
**	Function:	int pix_sys_pcmciainfo()
**	Desc:		Returns information from the card manager (cardmgr) concerning the
**				status of the PCMIA sockets
**	Accepts:	pixPCMCIAInfo_t *ppi = Ptr to the PCMCIA info struct
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
pix_sys_pcmciainfo(pixPCMCIAInfo_t * ppi)
{
    char cardbuf[256],		/* read buffer */
     *cardpath = NULL,		/* Ptr to the card path */
     *cardopts[2] = { "/var/run/stab",
	"/var/lib/pcmcia/stab"
    };
    int rc = -1;		/* Default return code */
    FILE *fp = 0;		/* File pointer */

    /* Determine the validity of the arguments */
    if (ppi == NULL)
	return (rc);

    /* Determine the path of the file */
    if ((cardpath = getenv("CARD_STAB")) == NULL
	|| (fp = fopen(cardpath, "r")) == NULL) {
	int idx;

	for (idx = 0; idx < 2; idx++) {
	    cardpath = cardopts[idx];
	    if ((fp = fopen(cardpath, "r")) != NULL)
		break;
	}			/* end of for */
    }
    /* end of if */
    if (fp == NULL)
	return (rc);

    /* Now parse the data */
    while (fgets(cardbuf, sizeof(cardbuf), fp) != NULL) {
	int len = strlen(cardbuf);

	if (cardbuf[len - 1] == '\n')
	    cardbuf[--len] = '\0';

	if (!memcmp(cardbuf, "Socket 0:", 9)) {
	    strcpy(ppi->socket0, cardbuf);
	    rc++;
	} /* end if of */
	else if (!memcmp(cardbuf, "Socket 1:", 9)) {
	    strcpy(ppi->socket1, cardbuf);
	    rc++;
	}			/* end of else if */
    }				/* end of while */

    fclose(fp);

    /* Normalize the return value */
    if (rc > -1)
	rc = 0;

    return (rc);
}				/* end of pix_sys_pcmciainfo() */
