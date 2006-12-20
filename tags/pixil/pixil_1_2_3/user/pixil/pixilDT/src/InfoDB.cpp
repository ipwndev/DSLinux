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
#include "InfoDB.h"
#include "InfoDBDef.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
InfoDB *
    InfoDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Default info typess if this is a new data base.              //
//--------------------------------------------------------------//
const char *
    InfoDB::m_pszDefaultType[7] = {
    N_("Work"),
    N_("Home"),
    N_("Fax"),
    N_("Mobile"),
    N_("Pager"),
    N_("E-Mail"),
    N_("Web Page"),
};


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
InfoDB::InfoDB()
:  NxDbAccess("add_info", &iFile, iFields)
{
    // Init the database if needed
    Init();
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
InfoDB::~InfoDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
InfoDB *
InfoDB::GetInfo()
{
    if (m_pThis == NULL) {
	m_pThis = new InfoDB;
    }
    return (m_pThis);
}


//--------------------------------------------------------------//
// Initialize the database at open.                             //
//--------------------------------------------------------------//
void
InfoDB::Init()
{
    // Init the data base if needed
    if (NumRecs() == 0) {
	int i;
	int nRow;

	for (i = 0; i < 7; ++i) {
	    nRow = Insert(i);
	    SetInfoType(nRow, _(m_pszDefaultType[i]));
	}
	Save();
    }
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
InfoDB::Insert(int nInfoID)
{
    int nRow = NxDbAccess::Insert();

    // Now set the key value requested by the caller
    SetColumn(nRow, INFOID, nInfoID);

    // Save this row so that an uninitialized key
    // is not left in the data base if the program fails
    Save();

    return (nRow);
}
