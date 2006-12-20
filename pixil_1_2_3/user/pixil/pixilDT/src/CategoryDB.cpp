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
#include <FL/fl_ask.H>
#include "config.h"
#include "CategoryDB.h"
#include "CategoryDBDef.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default categories if this is a new data base.               //
//--------------------------------------------------------------//
const char *
    CategoryDB::m_pszDefaultCategory[3] = {
    N_("Unfiled"),
    N_("Business"),
    N_("Personal")
};


//--------------------------------------------------------------//
// Field definitions used during construction.  (Kludge for     //
// different sized category names.)                             //
//--------------------------------------------------------------//
bool
    CategoryDB::m_bFieldInUse[4] = {
	false,
	false,
	false,
false };
field *
    CategoryDB::m_pStaticField[4];
fildes *
    CategoryDB::m_pStaticFildes[4];
int
    CategoryDB::m_nLastIndexUsed;


//--------------------------------------------------------------//
// Constructor (kludged due to varying lengths of category      //
// fields.                                                      //
//--------------------------------------------------------------//
CategoryDB::CategoryDB(const char *pszFileName, int nCategoryLength)
    :
NxDbAccess(pszFileName, m_pStaticFildes[m_nLastIndexUsed],
	   CreateFields(nCategoryLength))
{
    // Save the field definitions used by the construction process
    m_nIndex = m_nLastIndexUsed;

    // Init the database if needed
    Init();
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
CategoryDB::~CategoryDB()
{
    // Close this data base "early" so that the remainder of this
    // destructor can run without causing problems
    Close();

    // Free up the field definitions
    delete[]m_pStaticField[m_nIndex];
    delete[]m_pStaticFildes[m_nIndex];
    m_bFieldInUse[m_nIndex] = false;
}


//--------------------------------------------------------------//
// Called by the constructor to set up the fields for this data //
// base.                                                        //
//--------------------------------------------------------------//
field *
CategoryDB::CreateFields(int nCategoryLength)
{
    int i;

    // Find a static field definition not already in use
    for (i = 0; i < 4; ++i) {
	if (m_bFieldInUse[i] == false) {
	    break;
	}
    }

#ifdef DEBUG
    assert(i < 4);		// No open slots
#endif

    // Save for post constructor code
    m_nLastIndexUsed = i;

    m_bFieldInUse[i] = true;
    m_pStaticField[i] = new field[3];
    memcpy(m_pStaticField[i], catFields, 3 * sizeof(field));
    m_pStaticField[i][1].size = nCategoryLength;
    m_pStaticFildes[i] = new fildes;
    memcpy(m_pStaticFildes[i], &catFile, sizeof(fildes));
    m_pStaticFildes[i]->fieldp = m_pStaticField[i];
    return (m_pStaticField[i]);
}


//--------------------------------------------------------------//
// Create a menu for a category selection Fl_Choice widget.     //
//--------------------------------------------------------------//
Fl_Menu_Item *
CategoryDB::GetCategoryMenu(bool bIncludeAll)
{
    Fl_Menu_Item *pMenuItem;
    int i;
    int nMax = NumUndeletedRecs() + 1;

    // Create the menu
    pMenuItem = new Fl_Menu_Item[nMax + (bIncludeAll == true ? 1 : 0)];
    memset(pMenuItem, 0,
	   (nMax + (bIncludeAll == true ? 1 : 0)) * sizeof(Fl_Menu_Item));

    // Set up the all selection if needed
    if (bIncludeAll == true) {
	// Use strdup so that FreeTranslatedMenu will work correctly
	pMenuItem[0].text = strdup(_("All"));
    }
    // Add the categories to the menu
    Sort(CATID);
    for (i = 0; i < nMax - 1; ++i) {
	// Use strdup so that FreeTranslatedMenu will work correctly
	pMenuItem[i + (bIncludeAll == true ? 1 : 0)].text =
	    strdup(GetCategory(i).c_str());
    }
    return (pMenuItem);
}


//--------------------------------------------------------------//
// Create a menu for a category selection Fl_Choice widget      //
// using a preset font.                                         //
//--------------------------------------------------------------//
Fl_Menu_Item *
CategoryDB::GetCategoryMenuWithFont(bool bIncludeAll,
				    Fl_Font nFont, unsigned char nFontSize)
{
    Fl_Menu_Item *pMenuItem = GetCategoryMenu(bIncludeAll);
    int i;
    int nMax = NumUndeletedRecs() + (bIncludeAll ? 1 : 0);

    // Fix the font in the menu
    for (i = 0; i < nMax; ++i) {
	pMenuItem[i].labelfont(nFont);
	pMenuItem[i].labelsize(nFontSize);
    }
    return (pMenuItem);
}


//--------------------------------------------------------------//
// Initialize the database at open.                             //
//--------------------------------------------------------------//
void
CategoryDB::Init()
{
    // Init the data base if needed
    if (NumRecs() == 0) {
	int i;
	int nRow;

	for (i = 0; i < 3; ++i) {
	    nRow = Insert(i);
	    SetCategory(nRow, _(m_pszDefaultCategory[i]));
	}
	Save();
    }
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
CategoryDB::Insert(int nID)
{
    int nRow = NxDbAccess::Insert();

    // Now set the key value requested by the caller
    if (nID < 0) {
	// Caller did not request one, set next highest
	SetHighKey(nRow, CATID);
    } else {
	// Caller requested a specific key
	SetColumn(nRow, CATID, nID);

	// Save this change so that an uninitialized key is
	// not left in the data base if the program fails
	Save();
    }

    return (nRow);
}


//--------------------------------------------------------------//
// Test a category name for duplicates.                         //
// pUserData starts out in the InputBox class as a void pointer //
// but is really the category ID of an existing category.  It   //
// is used here to prevent calling an unchanged name a          //
// duplicate when renaming categories.  Returns false if this   //
// is a duplicate category name.                                //
//--------------------------------------------------------------//
bool
CategoryDB::TestDuplicate(const char *pszString, void *pUserData)
{
    bool bReturn;
    int nThisCategory = reinterpret_cast < int >(pUserData);

    // Test if this is a duplicate string
    bReturn = NxDbAccess::TestDuplicate(pszString, nThisCategory, CAT, CATID);

    // Issue an error message if not valid
    if (bReturn == false) {
	fl_alert(_
		 ("This Category Name duplicates an existing category, please re-enter."));
    }

    return (bReturn);
}
