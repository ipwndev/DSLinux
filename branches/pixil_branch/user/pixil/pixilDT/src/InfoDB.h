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
// "Info" database definition fields.                           //
//--------------------------------------------------------------//
#ifndef INFODB_H_

#define INFODB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include "NxDbAccess.h"

// Field References
#define INFOID   0
#define INFOTYPE 1
class InfoDB:public NxDbAccess
{
  public:InfoDB();		// Constructor
    ~InfoDB();			// Destructor
    static InfoDB *InfoDB::GetInfo();	// Get a pointer to this object
    inline string GetInfoType(int nRow) const	// Get the Info Type string
    {
	return (GetStringValue(nRow, INFOTYPE));
    }
    inline int GetInfoID(int nRow) const	// Get the recno column
    {
	return (GetIntValue(nRow, INFOID));
    }
    int Insert(int nInfoID);	// Insert a row and set its key id
    inline void SetInfoType(int nRow, const char *pszInfoType)	// Set the Info Type string
    {
	SetColumn(nRow, INFOTYPE, pszInfoType);
    }
  private:static const char *m_pszDefaultType[];
    // Initial rows
    static InfoDB *m_pThis;	// One and only address book object
    void Init();		// Initialize the database if empty
};


#endif /*  */
