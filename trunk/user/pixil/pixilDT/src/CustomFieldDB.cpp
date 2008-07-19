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
// Base class for all Category database classes.                //
//--------------------------------------------------------------//
#include "config.h"
#include "CustomFieldDB.h"
#include "CustomFieldDBDef.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
CustomFieldDB *
    CustomFieldDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Default info typess if this is a new data base.              //
//--------------------------------------------------------------//
const char *
    CustomFieldDB::m_pszDefaultName[4] = {
    N_("Custom1"),
    N_("Custom2"),
    N_("Custom3"),
    N_("Custom4"),
};


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
CustomFieldDB::CustomFieldDB()
:  NxDbAccess("add_custfields", &custFile, custFields)
{
    // Init the database if needed
    Init();
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
CustomFieldDB::~CustomFieldDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
CustomFieldDB *
CustomFieldDB::GetCustomField()
{
    if (m_pThis == NULL) {
	m_pThis = new CustomFieldDB;
    }
    return (m_pThis);
}


//--------------------------------------------------------------//
// Initialize the database at open.                             //
//--------------------------------------------------------------//
void
CustomFieldDB::Init()
{
    // Init the data base if needed
    if (NumRecs() == 0) {
	int i;
	int nRow;

	for (i = 0; i < 4; ++i) {
	    nRow = Insert(i);
	    SetName(nRow, _(m_pszDefaultName[i]));
	}
	Save();
    }
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
CustomFieldDB::Insert(int nID)
{
    int nRow = NxDbAccess::Insert();

    // Now set the key value requested by the caller
    SetColumn(nRow, CUSTOMID, nID);

    return (nRow);
}
