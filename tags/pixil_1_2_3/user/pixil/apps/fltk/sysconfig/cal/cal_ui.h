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



#ifndef		CAL_UI_INCLUDED
#define		CAL_UI_INCLUDED		1

/* System header files */


/* Local header files */
#include <nano-X.h>

typedef enum
{
    amNONE = 0,			/* No rotation needed */
    amXFLIP,			/* Only X is flipped */
    amYFLIP,			/* Only Y is flipped */
    amXYFLIP			/* Both X & Y are flipped */
}
AxisMode_t;

/* Definitions for calibration */
#define			CAL_MAX_CTRL_PTS			5	/* Number of control points to gather */
#define			CAL_OFFSET1					30	/* Offset #1 value */
#define			CAL_OFFSET2					50	/* Offset #2 value */
#define			SZ_XHAIR					50	/* Size of the cross-hair */
#define			START_TEXT_Y				95

/* Color scheme (foreground and background) */
#define			NXCAL_BACKGROUND			(GR_RGB(0, 0, 0))
#define			NXCAL_FOREGROUND			(GR_RGB(255, 255, 255))

/* Flag definitions for nxcal flags arg */
#define			NXCAL_NONINT_MODE			0x01

/* Function prototypes */
#ifdef		__cplusplus
extern "C"
{
#endif				/*      __cplusplus */
    int nxcal(int flags, char *calfile);
#ifdef		__cplusplus
}
#endif				/*      __cplusplus */

#endif				/*      CAL_UI_INCLUDED */
