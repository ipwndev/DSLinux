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
// ToDo List database class.                                    //
//--------------------------------------------------------------//
#ifndef TODOLISTDB_H_

#define TODOLISTDB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <vector>
#include "NxDbAccess.h"
using namespace std;

// Field References
#define TODO_ID         0
#define TODO_CAT        1
#define TODO_COMPLETE   2
#define TODO_PRIORITY   3
#define TODO_TITLE      4
#define TODO_DESC       5
#define TODO_TIME       6
#define TODO_NUM_FIELDS 7

#define TODO_TITLE_LEN 100
#define TODO_DESC_LEN  100
class ToDoListDB:public NxDbAccess
{
  public:ToDoListDB();		// Constructor
    ~ToDoListDB();		// Destructor
    inline int GetCategory(int nRow) const	// Get the Category
    {
	return (GetIntValue(nRow, TODO_CAT));
    }
    string GetCategoryName(int nRow) const;	// Get the name of the category
    inline int GetComplete(int nRow) const	// Get the complete flag
    {
	return (GetIntValue(nRow, TODO_COMPLETE));
    }
    inline string GetDescription(int nRow) const	// Get the description
    {
	return (GetStringValue(nRow, TODO_DESC));
    }
    inline int GetID(int nRow) const	// Get the ID
    {
	return (GetIntValue(nRow, TODO_ID));
    }
    inline int GetPriority(int nRow) const	// Get the priority
    {
	return (GetIntValue(nRow, TODO_PRIORITY));
    }
    inline int GetTime(int nRow) const	// Get the time
    {
	return (GetIntValue(nRow, TODO_TIME));
    }
    string GetTimeString(int nRow) const;	// Get the time as a string
    inline string GetTitle(int nRow) const	// Get the title
    {
	return (GetStringValue(nRow, TODO_TITLE));
    }
    static ToDoListDB *GetToDoListDB();	// Get the singleton pointer
    int Import(const vector < string > &strData);	// Import a delimited string
    int Insert(int nCategory);	// Insert a row and set its key value
    inline void SetCategory(int nRow, int nCategory)	// Set the category
    {
	SetColumn(nRow, TODO_CAT, nCategory);
    }
    inline void SetComplete(int nRow, int nComplete)	// Set the complete flag
    {
	SetColumn(nRow, TODO_COMPLETE, nComplete);
    }
    inline void SetDescription(int nRow, const char *pszDesc)	// Set the description
    {
	SetColumn(nRow, TODO_DESC, pszDesc);
    }
    inline void SetID(int nRow, int nID)	// Set the ID
    {
	SetColumn(nRow, TODO_ID, nID);
    }
    inline void SetPriority(int nRow, int nPriority)	// Set the priority
    {
	SetColumn(nRow, TODO_PRIORITY, nPriority);
    }
    inline void SetTime(int nRow, int nTime)	// Set the time
    {
	SetColumn(nRow, TODO_TIME, nTime);
    }
    inline void SetTitle(int nRow, const char *pszTitle)	// Set the title
    {
	SetColumn(nRow, TODO_TITLE, pszTitle);
    }
  private:static ToDoListDB *m_pThis;
    // One and only object
};


#endif /*  */
