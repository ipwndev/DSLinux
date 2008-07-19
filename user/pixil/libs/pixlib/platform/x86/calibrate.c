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
#include <string.h>

/* Local header files */
#include <pixlib/pixlib.h>

/* Typedef, macros, enum/struct/union defintions */


/* Global scope variables */


/* Local scope variables */


/* Static function prototypes */

/*-----------------------------------------------------------------------------*\
--
--	Static function definitions
--
\*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*\
--
--	External function definitions
--
\*-----------------------------------------------------------------------------*/

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDataPt()
**	Desc:		Gets the raw data (point data) from the device file and stores
**				it into the CalPt_t structure passed in
**	Accepts:	CalPt_t *ptdata = Ptr to storage of x/y coordinates
**	Returns:	int; 0 on successful read, -1 on error
**
\*******************************************************************************/
int
pix_cal_GetDataPt(CalPt_t * ptdata)
{
    int retval = PIXLIB_STUB_VAL;	/* Default to stubbed function */

    return (retval);
}				/* end of pix_cal_GetDataPt() */

/*******************************************************************************\
**
**	Function:	int pix_cal_Calibrate()
**	Desc:		ipaq calibration routine which calculates the calibration values
**				based on the control and user specific data points
**	Accepts:	int npts = Number of control/data points to process
**				CalPt_t *ctrldata = Array of npts control points
**				CalPt_t *userdata = Array of npts user points
**	Returns:	int; 0 on success, -1 on error, PIXLIB_STUB_VAL for a stubbed
**				function.
**
\*******************************************************************************/
int
pix_cal_Calibrate(int npts, CalPt_t * ctrldata, CalPt_t * userdata)
{
    int retval = PIXLIB_STUB_VAL;	/* Default to stubbed function */

    return (retval);
}				/* end of pix_cal_Calibrate() */

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDrawPt()
**	Desc:		Gets the draw point (in case of any swapping/rotation that is
**				device specific)
**	Accepts:	CalPt_t *ctrldata = Ptr to the control points
**				CalPt_t *drawdata = Ptr to the draw points storage
**	Returns:	int; 0 on success, -1 on error, PIXLIB_STUB_VAL for stubbed fxn
**	
\*******************************************************************************/
int
pix_cal_GetDrawPt(CalPt_t * ctrldata, CalPt_t * drawdata)
{
    if (ctrldata == NULL || drawdata == NULL)
	return (-1);

    memcpy(drawdata, ctrldata, sizeof(CalPt_t));

    return (0);
}				/* end of pix_cal_GetDrawPt() */

/*******************************************************************************\
**
**	Function:	int pix_cal_GetCtrlPts()
**	Desc:		Creates the control point array and fills it in with the control
**				points for this platform
**	Acecpts:	int *npts = Ptr to the number of points
**				CalPt_t **ctrldata = Ptr to Ptr of array to be dynamically created
**				int w = screen width
**				int h = screen height
**				int bpp = bits per pixel
**	Returns:	int; 0 on success, -1 on error, PIXLIB_STUB_VAL for stubbed fxn
**
\*******************************************************************************/
int
pix_cal_GetCtrlPts(int *npts, CalPt_t ** ctrldata, int w, int h, int bpp)
{
    int retval = PIXLIB_STUB_VAL;

    return (retval);
}				/* end of pix_cal_GetCtrlPts() */
