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


#ifndef _SCRTOPLIB_H_
#define _SCRTOPLIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
	int type;
	char app[30];
    }
    scrtop_action;

    typedef union
    {
	int type;
	scrtop_action action;
    }
    scrtop_message;

#define CATEGORY_ACTION 1
#define CATEGORY_NA_ACTION 2

#define CATEGORY_ITEM(category, item) ((category << 8) | item)
#define GET_CATEGORY(value) ( (value >> 8) & 0xFF )

#define ACTION_RAISE CATEGORY_ITEM(CATEGORY_ACTION, 1)
#define ACTION_LOWER CATEGORY_ITEM(CATEGORY_ACTION, 2)
#define ACTION_SHOW  CATEGORY_ITEM(CATEGORY_ACTION, 3)
#define ACTION_HIDE  CATEGORY_ITEM(CATEGORY_ACTION, 4)
#define ACTION_CLOSE CATEGORY_ITEM(CATEGORY_ACTION, 5)

#define NA_ACTION_BLON	CATEGORY_ITEM(CATEGORY_NA_ACTION, 1)
#define NA_ACTION_BLOFF CATEGORY_ITEM(CATEGORY_NA_ACTION, 2)

    int scrtopDoAction(int type, char *app);

#define scrtopRaiseApp(app)  (scrtopDoAction(ACTION_RAISE, app))
#define scrtopLowerApp(app)  (scrtopDoAction(ACTION_LOWER, app))
#define scrtopShowApp(app)   (scrtopDoAction(ACTION_SHOW, app))
#define scrtopHideApp(app)   (scrtopDoAction(ACTION_HIDE, app))
#define scrtopCloseApp(app)  (scrtopDoAction(ACTION_CLOSE, app))

#ifdef __cplusplus
}
#endif

#endif
