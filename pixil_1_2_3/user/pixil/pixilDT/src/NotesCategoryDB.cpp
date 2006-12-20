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
// Notes Category database definition fields.                   //
//--------------------------------------------------------------//
#include "NotesCategoryDB.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
NotesCategoryDB *
    NotesCategoryDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NotesCategoryDB::NotesCategoryDB()
:  CategoryDB("notcat", 13)
{
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NotesCategoryDB::~NotesCategoryDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
NotesCategoryDB *
NotesCategoryDB::GetNotesCategoryDB()
{
    if (m_pThis == NULL) {
	m_pThis = new NotesCategoryDB;
    }
    return (m_pThis);
}
