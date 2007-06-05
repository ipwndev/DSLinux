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
// Address Book database definition fields.                     //
//--------------------------------------------------------------//
#ifndef ADDRESSBOOKDBDEF_H_

#define ADDRESSBOOKDBDEF_H_

// Contacts Fields
field cFields[] = {
    {
     'i', 1, 0}
    ,				// Field 0:RECNO
    {
     'i', 1, 0}
    ,				//       1:CAT
    {
     'i', 1, 0}
    ,				//          2:SHOW
    {
     'c', AB_TEXT, 0}
    ,				//       3:LASTNAME
    {
     'c', AB_TEXT, 0}
    ,				//       4:FIRSTNAME
    {
     'c', AB_TEXT, 0}
    ,				//       5:COMPANY
    {
     'c', AB_TEXT, 0}
    ,				//       6:TITLE
    {
     'i', 1, 0}
    ,				//          7:DEP1ID
    {
     'i', 1, 0}
    ,				//          8:DEP2ID
    {
     'i', 1, 0}
    ,				//          9:DEP3ID
    {
     'i', 1, 0}
    ,				//         10:DEP4ID
    {
     'i', 1, 0}
    ,				//         11:DEP5ID
    {
     'i', 1, 0}
    ,				//         12:DEP6ID
    {
     'i', 1, 0}
    ,				//         13:DEP7ID
    {
     'c', AB_TEXT, 0}
    ,				//      14:DEP1
    {
     'c', AB_TEXT, 0}
    ,				//      15:DEP2
    {
     'c', AB_TEXT, 0}
    ,				//      16:DEP3
    {
     'c', AB_TEXT, 0}
    ,				//      17:DEP4
    {
     'c', AB_TEXT, 0}
    ,				//      18:DEP5
    {
     'c', AB_TEXT, 0}
    ,				//      19:DEP6
    {
     'c', AB_TEXT, 0}
    ,				//      20:DEP7
    {
     'c', AB_DBL_TEXT, 0}
    ,				//      21:ADDRESS
    {
     'c', AB_TEXT, 0}
    ,				//      22:CITY
    {
     'c', AB_TEXT, 0}
    ,				//      23:REGION
    {
     'c', AB_TEXT, 0}
    ,				//      24:POSTALCODE
    {
     'c', AB_TEXT, 0}
    ,				//      25:COUNTRY
    {
     'c', AB_DATE, 0}
    ,				//      26:BDAY
    {
     'c', AB_DATE, 0}
    ,				//      27:ANNIV
    {
     'c', AB_TEXT, 0}
    ,				//      28:CUST1
    {
     'c', AB_TEXT, 0}
    ,				//      29:CUST2
    {
     'c', AB_TEXT, 0}
    ,				//      30:CUST3
    {
     'c', AB_TEXT, 0}
    ,				//      31:CUST4
    {
     'c', AB_NOTEDB, 0}
    ,				//      32:NOTE
    {
     0}
};


// Database
fildes cFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			// extension
    33,				// nfields
    &cFields[0]			// fieldlist
};


#endif /*  */
