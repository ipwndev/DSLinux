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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef _SYNC_ERROR_H_
#define _SYNC_ERROR_H_

typedef struct sync_err_struct_
{
    char *msg;
}
sync_err_struct;

#define NO_ERR			0
#define EXP_ABORT 	        1
#define EXP_OK 			2
#define EXP_INFO		3
#define EXP_BP			4
#define EXP_EP			5
#define EXP_STATUS	        6
#define EXP_BS			7
#define EXP_ES			8
#define	EXP_TS			9
#define EXP_RD			10
#define EXP_ET			11
#define EXP_EOT			12
#define EXP_FLIP		13
#define EXP_COMMIT	        14
#define BAD_TS			15
#define SAVE_RD			16
#define UNEXP_ERROR	        17
#define USER_ABORT	        18
#define DT_BUSY                 19
#define CLOSE_CONN              20
#define NO_AGENT		21
#define AGENT_NS		22
#define NO_DB_REG		23
#define UNK_ERROR		24

#define ERR_MAX			24

extern sync_err_struct sync_err_msg[];

inline char *
get_error_msg(const int err_code)
{
    if (ERR_MAX < err_code)
	return sync_err_msg[ERR_MAX].msg;
    else if (0 > err_code)
	return sync_err_msg[ERR_MAX].msg;
    else
	return sync_err_msg[err_code].msg;
}
#endif
