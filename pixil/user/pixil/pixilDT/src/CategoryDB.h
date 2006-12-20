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
#ifndef CATEGORYDB_H_

#define CATEGORYDB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Menu_Item.H>
#include "NxDbAccess.h"

// Field References
#define CATID 0
#define CAT   1
class CategoryDB:public NxDbAccess
{
  public:CategoryDB(const char *pszFileName,
	       // Constructor
	       int nCategoryLength);
     ~CategoryDB();		// Destructor
    inline string GetCategory(int nRow) const	// Get the category string
    {
	return (GetStringValue(nRow, CAT));
    }
    Fl_Menu_Item *GetCategoryMenu(bool bIncludeAll);	// Create a menu for a category choice widget
    Fl_Menu_Item *GetCategoryMenuWithFont(bool bIncludeAll,	// Create a menu for a category choice widget
					  Fl_Font nFont,
					  unsigned char nFontSize);
    inline int GetCategoryID(int nRow) const	// Get the recno column
    {
	return (GetIntValue(nRow, CATID));
    }
    int Insert(int nID);	// Insert a row and set its key id
    inline void SetCategory(int nRow, const char *pszCategory)	// Set the category string
    {
	SetColumn(nRow, CAT, pszCategory);
    }
    bool TestDuplicate(const char *pszString,	// Test a name for duplicates
		       void *pData);
  private:static bool m_bFieldInUse[4];
    // Indicators for static field definitions bing in use
    static const char *m_pszDefaultCategory[3];	// Default categories for database initialization
    static field *m_pStaticField[4];	// Field definitions used during construction
    static fildes *m_pStaticFildes[4];	// Data base definition used during custruction
    int m_nIndex;		// Index to the field definitions used for this data base
    static int m_nLastIndexUsed;	// Used during construction process
    void Init();		// Initialize the data base at open
    static field *CreateFields(int nCategoryLength);	// Create the database fields definition
};


#endif /*  */
