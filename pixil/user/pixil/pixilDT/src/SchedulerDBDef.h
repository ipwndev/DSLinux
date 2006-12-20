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

//--------------------------------------------------------------//
// Scheduler database definition fields.                        //
//--------------------------------------------------------------//
#ifndef SCHEDULERDBDEF_H_

#define SCHEDULERDBDEF_H_

// Schedule
field sFields[] = {
    {
     'i', 1, 0}
    ,				// Field 0:id
    {
     'i', 1, 0}
    ,				//        1:categoryId
    {
     'l', 1, 0}
    ,				//        2:startTime
    {
     'l', 1, 0}
    ,				//        3:endTime
    {
     'i', 1, 0}
    ,				//        4:allDayFlag
    {
     'i', 1, 0}
    ,				//        5:repeatFlag 1 (day, week, month, year)
    {
     'i', 1, 0}
    ,				//        6:repeatFlag 2
    {
     'l', 1, 0}
    ,				//        7:repeatFlag 3
    {
     'l', 1, 0}
    ,				//        8:repeatWkMonFlag
    {
     'i', 1, 0}
    ,				//        9:entryType
    {
     'c', SCHED_DESC_LEN, 0}
    ,				// 10:description
    {
     'i', 1, 0}
    ,				//        11 exception flag
    {
     'i', 1, 0}
    ,				//        12 recno pointer
    {
     'i', 1, 0}
    ,				//        13:alarm interval
    {
     'i', 1, 0}
    ,				//        14:alarm_flags
    {
     0}
};


// Database
fildes sFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			// extension
    SCHED_NUM_FIELDS,		// nfields
    &sFields[0]			// fieldlist
};


#endif /*  */
