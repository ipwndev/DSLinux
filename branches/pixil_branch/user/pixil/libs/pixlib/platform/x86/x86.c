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



/* System header files */


/* Local header files */
#include <pixlib/pixlib.h>


/* Typedef, macros, enum/struct/union definitions */


/* Global scope variables */


/* Local scope variables */
static unsigned long f_capmask = 0;	/* No hardware specific is supported */

/* Static function prototypes */


/*******************************************************************************\
**
**	Static function definitions
**
\*******************************************************************************/


/*******************************************************************************\
**
**	Extern function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	Function:	int pix_PDSupprt()
**	Desc:		Determines if the given platform support the specified capabilites
**	Accepts:	int capability = Flag for a given capability
**	Returns:	int; 0 = not supported, non-zero for capabilties supported
**	
\*******************************************************************************/
int
pix_PDSupport(int capability)
{
    return (capability & f_capmask);
}				/* end of pix_PDSupport() */
