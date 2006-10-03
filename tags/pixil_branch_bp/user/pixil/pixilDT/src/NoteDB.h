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
// Notes database class.                                        //
//--------------------------------------------------------------//
#ifndef NOTEDB_H_

#define NOTEDB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <vector>
#include "Note.h"
#include "NxDbAccess.h"
using namespace std;

// Field References
#define NOTE_INDEX      0
#define NOTE_CAT        1
#define NOTE_FILE       2
#define NOTE_DESC       3
#define NOTE_ARCH       4
#define NOTE_NUM_FIELDS 5

#define NOTE_TITLE_LEN 50
#define NOTE_FILE_LEN 100

#define NOTE_NOTES_PREFIX "not_"
class NoteDB:public NxDbAccess
{
  public:NoteDB();		// Constructor
    ~NoteDB();			// Destructor
    void Delete(int nRow);	// Delete the note file for this note as well
    void Export(int nRow,	// Export to a set of strings
		vector < string > &vExportString);
    inline int GetArchiveFlag(int nRow) const	// Get the archive flag
    {
	return (GetIntValue(nRow, NOTE_ARCH));
    }
    inline int GetCategory(int nRow) const	// Get the category
    {
	return (GetIntValue(nRow, NOTE_CAT));
    }
    string GetCategoryName(int nRow) const;	// Get the category name
    inline string GetFile(int nRow) const	// Get the note file
    {
	return (GetStringValue(nRow, NOTE_FILE));
    }
    inline int GetIndex(int nRow) const	// Get the index
    {
	return (GetIntValue(nRow, NOTE_INDEX));
    }
    static NoteDB *GetNoteDB();	// Get the singleton pointer
    Note *GetNote(int nRow) const;	// Get the actual note
    inline string GetTitle(int nRow) const	// Get the title
    {
	return (GetStringValue(nRow, NOTE_DESC));
    }
    int Import(const vector < string > &vExportString);	// Import a delimited string
    int Insert(int nCategory);	// Insert a row and set its key value
    inline void SetArchiveFlag(int nRow, int nArchiveFlag)	// Set the archive flag
    {
	SetColumn(nRow, NOTE_ARCH, nArchiveFlag);
    }
    inline void SetCategory(int nRow, int nCategory)	// Set the category
    {
	SetColumn(nRow, NOTE_CAT, nCategory);
    }
    inline void SetFile(int nRow, const char *pszFile)	// Set the note file
    {
	SetColumn(nRow, NOTE_FILE, pszFile);
    }
    inline void SetIndex(int nRow, int nIndex)	// Set the index
    {
	SetColumn(nRow, NOTE_INDEX, nIndex);
    }
    void SetNote(int nRow,	// Set the note
		 Note * pNote);
    inline void SetTitle(int nRow, const char *pszTitle)	// Set the title
    {
	SetColumn(nRow, NOTE_DESC, pszTitle);
    }
  private:static NoteDB *m_pThis;
    // One and only object
};


#endif /*  */
