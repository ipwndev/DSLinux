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


#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_

// msg_defs.h
//
// Description: Definitions of the message protocol to be passed back
//                              and forth between PIM applications and the desktop. 
//                              Some of these messages are also used for the sync.
//                              application to begin syncronization with the sync
//                              agent.

/*********** Old stuff
#define ACK		0	
#define BEG_SYNC 	1
#define ACK_SYNC 	2
#define BEG_COMP 	3
#define ACK_COMP 	4
#define BEG_APP 	100
#define ACK_APP 	101
#define BEG_TABLE 	102
#define ACK_TABLE 	103
#define ROW_DATA 	104
#define ACK_ROW 	105
#define END_TABLE 	190
#define END_APP 	192
#define RCV_COMM 	193
#define SND_COMM 	194
#define ABORT 		199
#define ACK_ABORT 	198
#define STATUS		80
#define ACK_STATUS 	81
#define END_COMP 	90
#define END_SYNC	91
#define ACK_END 	200
#define ERR 		99
***************/

// New stuff 12/13/01
#define ERR	100
#define ABORT   150
#define OK	200
#define INFO	250
#define BP	300
#define EP	350
#define STATUS  400
#define BS	500
#define ES	550
#define TS	600
#define RD	700
#define ET	800
#define EOT	850
#define FLIP	900
#define COMMIT	950

/* Anything 1000 and above are destined only for the sync agent */
#define CONNECT    1000
#define DISCONNECT 1005

#endif
