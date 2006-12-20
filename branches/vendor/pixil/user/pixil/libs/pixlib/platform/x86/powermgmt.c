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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Local header files */
#include <pixlib/pixlib.h>


/* Typedef, macros, enum/struct/union definitions */
typedef struct
{
    char ldrvr_v[4 + 1], bios_v[4 + 1], metric[4 + 1];
    unsigned char apmflg, linests, batsts, batflg;
    int batper, batunit;
}
apm_values_t;


/* Global scope variables */


/* File scope variables */


/* Static function prototypes */
static int parseApm(apm_values_t * apmval);


/*******************************************************************************\
**
**	Static function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	Function:	static int parseApm()
**	Desc:		Reads the /proc/apm file and fills in the apm_values_t struct
**	Accepts:	apm_values_t *apmval = Ptr to the struct to fill in
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
static int
parseApm(apm_values_t * apmval)
{
    int apmflg, batsts, batflg, linests, retval = -1;	/* Return value */
    char apmbuf[255];		/* Buffer */
    FILE *fin;			/* File ptr */

    if (apmval == NULL)
	return (retval);

    if ((fin = fopen("/proc/apm", "r")) == NULL)
	return (retval);

    /* Get the first line */
    if (fgets(apmbuf, sizeof(apmbuf), fin) != NULL) {
	sscanf(apmbuf, "%s %s %i %i %i %i %d%*c %d %s", apmval->ldrvr_v,
	       apmval->bios_v, &apmflg, &linests, &batsts, &batflg,
	       &apmval->batper, &apmval->batunit, apmval->metric);
	apmval->apmflg = (unsigned char) apmflg;
	apmval->linests = (unsigned char) linests;
	apmval->batsts = (unsigned char) batsts;
	apmval->batflg = (unsigned char) batflg;
	retval = 0;
    }
    /* end of if */
    fclose(fin);

    return (retval);
}				/* end of parseApm() */


/******************************************************************************\
**
**	Externally callable function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	Function:	int pix_pwr_getbat()
**	Desc:		Returns the current battery level from the device
**	Acecpts:	int flags = Flag indicating how to return the value;
**					PWR_BAT_PERCENT = return as a percent (int)
**					PWR_BAT_SECONDS = return as # of seconds remaining
**	Returns:	int; value as described (>= 0), -1 on unknown, PIXLIB_STUB_VAL
**					for stubbed functions
**
\*******************************************************************************/

int
pix_pwr_getbat(int flags)
{
    int retval = -1;		/* Return value */
    apm_values_t apmval;	/* Apm values from the device */

    int ret = parseApm(&apmval);
    if (ret == -1) return -1;  /* No APM available */   

    if (apmval.linests == 0x00) {
      if (flags == PWR_BAT_PERCENT) retval = apmval.batper;
    }
    else {
      if (!memcmp(apmval.metric, "min", 3))
	retval = apmval.batunit * 60;
      else if (!memcmp(apmval.metric, "sec", 3))
	retval = apmval.batunit;
    }	

    return retval;
}	

/*******************************************************************************\
**
**	Function:	int pix_pwr_isCharging()
**	Desc:		Attempts to determine if the device's battery is charging
**	Accepts:	Nothing (void)
**	Returns:	int; 1 for charging, 0 for not charging, -1 on unknown, PIXLIB_STUB_VAL
**					if stubbed
**
\*******************************************************************************/

int
pix_pwr_isCharging(void)
{
    int retval = 0;		/* Default to Not charging */
    apm_values_t apmval;	/* Apm values */

    int ret = parseApm(&apmval);
    if (ret == -1) return -1;  /* No APM available */   

    if (apmval.batflg != 0x80) {
      if (apmval.batsts == 0x03 || apmval.batflg == 0x03) 
	    retval = 1;
    }

    return retval;
} 

/*******************************************************************************\
**
**	Function:	int pix_pwr_OnBattery()
**	Desc:		Determines if the device is on batter power
**	Accepts:	Nothing (void)
**	Returns:	int; 1 for on battery, 0 on line power, -1 on unknown, PIXLIB_STUB_VAL
**					if stubbed.
**
\*******************************************************************************/
int
pix_pwr_onBattery(void)
{
    int retval = 0;		/* Default to unknown */
    apm_values_t apmval;	/* apm values */
    
    int ret = parseApm(&apmval); 
    if (ret == -1) return -1;

    if (apmval.linests == 0x00)
      retval = 1;
    else if (apmval.linests == 0xff)
      retval = -1;

    return (retval);
}	

/*******************************************************************************\
**
**	Function:	int pix_pwr_suspend()
**	Desc:	suspend the device	
**	Accepts:	Nothing (void)
**	Returns:	int; 0 on succes -1 on failure, PIXLIB_STUB_VAL
**					if stubbed.
**
\*******************************************************************************/
int
pix_pwr_suspend(void)
{
    int retval = -1;		/* Default to unknown */
    char *command = "/usr/bin/apm --suspend";

    printf("running [%s]\n", command);
    retval = system(command);
    if (127 == retval || 0 > retval) {
	retval = -1;
    } else
	retval = 0;
    return (retval);
}				/* end of pix_pwr_suspend() */
