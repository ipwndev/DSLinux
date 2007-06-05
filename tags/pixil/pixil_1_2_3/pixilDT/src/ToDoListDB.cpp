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
// ToDo List database definition fields.                        //
//--------------------------------------------------------------//
#include <ctime>
#include "FLTKUtil.h"
#include "TimeFunc.h"
#include "ToDoListCategoryDB.h"
#include "ToDoListDB.h"
#include "ToDoListDBDef.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
ToDoListDB *
    ToDoListDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ToDoListDB::ToDoListDB()
:  NxDbAccess("td", &tdFile, tdFields)
{
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ToDoListDB::~ToDoListDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Get the name of a category.                                  //
//--------------------------------------------------------------//
string
ToDoListDB::GetCategoryName(int nRow) const
{
    ToDoListCategoryDB *pCategory =
	ToDoListCategoryDB::GetToDoListCategoryDB();
    int nCategory = GetCategory(nRow);
    int nCategoryRow = pCategory->FindRow(CATID, nCategory);

    return (pCategory->GetCategory(nCategoryRow));
}


//--------------------------------------------------------------//
// Get the time as a date string.                               //
//--------------------------------------------------------------//
string
ToDoListDB::GetTimeString(int nRow) const
{
    string strReturn;
    time_t nDate = GetTime(nRow);

    if (nDate >= 24 * 60 * 60) {
	strReturn =::FormatDate(nDate);
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
ToDoListDB *
ToDoListDB::GetToDoListDB()
{
    if (m_pThis == NULL) {
	m_pThis = new ToDoListDB;
    }
    return (m_pThis);
}


//--------------------------------------------------------------//
// Import from a set of strings.                                //
//--------------------------------------------------------------//
int
ToDoListDB::Import(const vector < string > &vExportString)
{
    int nRow;

    // Import the data
    nRow = NxDbAccess::Import(vExportString);

    // Now fix the record number
    SetColumn(nRow, TODO_ID, 0);
    SetHighKey(nRow, TODO_ID);

    return (nRow);
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
ToDoListDB::Insert(int nCategory)
{
    int nRow = NxDbAccess::Insert();

    // Set other values as needed
    SetCategory(nRow, nCategory >= 0 ? nCategory : 0);
    SetTitle(nRow, _("(new item)"));

    // Now set a unique key value
    SetHighKey(nRow, TODO_ID);

    return (nRow);
}
