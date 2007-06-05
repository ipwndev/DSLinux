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
// Notes database definition fields.                            //
//--------------------------------------------------------------//
#include "NoteDB.h"
#include "NoteDBDef.h"
#include "NotesCategoryDB.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Pointer to the only one of these objects.                    //
//--------------------------------------------------------------//
NoteDB *
    NoteDB::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NoteDB::NoteDB()
:  NxDbAccess("not", &noteFile, noteFields)
{
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NoteDB::~NoteDB()
{
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Delete a note line including deleting the note if any        //
// exists.                                                      //
//--------------------------------------------------------------//
void
NoteDB::Delete(int nRow)
{
    Note *pNote = GetNote(nRow);

    pNote->Delete();
    delete pNote;

    NxDbAccess::Delete(nRow);
}


//--------------------------------------------------------------//
// Export a row to a set of strings.                            //
//--------------------------------------------------------------//
void
NoteDB::Export(int nRow, vector < string > &vExportString)
{
    Note *pNote;

    // Export the data base record
    NxDbAccess::Export(nRow, vExportString);

    // Get the notes for this row
    pNote = GetNote(nRow);
    vExportString.push_back(pNote->GetText());
    delete pNote;
}


//--------------------------------------------------------------//
// Get the name of a category.                                  //
//--------------------------------------------------------------//
string
NoteDB::GetCategoryName(int nRow) const
{
    NotesCategoryDB *pCategory = NotesCategoryDB::GetNotesCategoryDB();
    int nCategory = GetCategory(nRow);
    int nCategoryRow = pCategory->FindRow(CATID, nCategory);

    return (pCategory->GetCategory(nCategoryRow));
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
Note *
NoteDB::GetNote(int nRow) const
{
    Note *pReturn;
    string strNoteFile = GetFile(nRow);

    if (strNoteFile.length() == 0) {
	// No file name
	pReturn = new Note(0, NOTE_NOTES_PREFIX);
    } else {
	// Already have a file
	pReturn = new Note(strNoteFile.c_str(), 0, NOTE_NOTES_PREFIX);
    }
    return (pReturn);
}


//--------------------------------------------------------------//
// Get a pointer to an open data base.                          //
//--------------------------------------------------------------//
NoteDB *
NoteDB::GetNoteDB()
{
    if (m_pThis == NULL) {
	m_pThis = new NoteDB;
    }
    return (m_pThis);
}


//--------------------------------------------------------------//
// Import a row from a set of strings.                          //
//--------------------------------------------------------------//
int
NoteDB::Import(const vector < string > &vExportString)
{
    int nRow;
    Note *pNote;
    string strNote;
    vector < string > vExportString2 = vExportString;

    // Save the last string as the notes for this row
    strNote = vExportString2[vExportString2.size() - 1];
    vExportString2.pop_back();

    // Import the data
    nRow = NxDbAccess::Import(vExportString2);

    // Create the note for this row
    pNote = new Note(0, NOTE_NOTES_PREFIX);
    pNote->SetText(strNote.c_str());
    SetNote(nRow, pNote);
    delete pNote;

    // Now fix the record number
    SetIndex(nRow, 0);
    SetHighKey(nRow, NOTE_INDEX);

    return (nRow);
}


//--------------------------------------------------------------//
// Insert a new row and set its key id.                         //
//--------------------------------------------------------------//
int
NoteDB::Insert(int nCategory)
{
    int nRow = NxDbAccess::Insert();

    // Set the category for this row
    SetCategory(nRow, nCategory >= 0 ? nCategory : 0);

    // Now set a unique key value
    SetHighKey(nRow, NOTE_INDEX);

    return (nRow);
}


//--------------------------------------------------------------//
// Set the notes for this note line.                            //
//--------------------------------------------------------------//
void
NoteDB::SetNote(int nRow, Note * pNote)
{
    // Save to set the file name
    pNote->Save();

    // Save the note file name
    SetFile(nRow, pNote->GetFileName().c_str());
}
