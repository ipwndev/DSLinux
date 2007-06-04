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
 // Address Book database definition fields.                     //
 //--------------------------------------------------------------//
#include "config.h"
#include "AddressBookCategoryDB.h"
#include "AddressBookDB.h"
#include "AddressBookDBDef.h"
#include "CustomFieldDB.h"

#include "VCMemoryLeak.h"


 //--------------------------------------------------------------//
 // Pointer to the only one of these objects.                    //
 //--------------------------------------------------------------//
AddressBookDB *
    AddressBookDB::m_pThis =
    NULL;


 //--------------------------------------------------------------//
 // Constructor                                                  //
 //--------------------------------------------------------------//
AddressBookDB::AddressBookDB()
:  NxDbAccess("add", &cFile, cFields)
{
    printf("ADDRESSBOOKDB:  Created (%x)\n", this);
}


 //--------------------------------------------------------------//
 // Destructor                                                   //
 //--------------------------------------------------------------//
AddressBookDB::~AddressBookDB()
{
    m_pThis = NULL;
}


 //--------------------------------------------------------------//
 // Delete an address line including deleting the note if any    //
 // exists.                                                      //
 //--------------------------------------------------------------//
void
AddressBookDB::Delete(int nRow)
{
    Note *pNote = GetNote(nRow);

    pNote->Delete();
    delete pNote;

    NxDbAccess::Delete(nRow);
}


 //--------------------------------------------------------------//
 // Export to a delimited string including the notes.            //
 //--------------------------------------------------------------//
void
AddressBookDB::Export(int nRow, vector < string > &vExportString)
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
 // Workaround for the RECNO column being a character string.    //
 //--------------------------------------------------------------//
int
AddressBookDB::FindRecno(int nRecno)
{
    char szRecno[16];

    sprintf(szRecno, "%d", nRecno);
    return (FindRow(AB_RECNO, szRecno));
}


 //--------------------------------------------------------------//
 // Get a pointer to an open data base.                          //
 //--------------------------------------------------------------//
AddressBookDB *
AddressBookDB::GetAddressBookDB()
{
    if (m_pThis == NULL) {
	m_pThis = new AddressBookDB;
    }
    return (m_pThis);
}


 //--------------------------------------------------------------//
 // Get the name of a category.                                  //
 //--------------------------------------------------------------//
string
AddressBookDB::GetCategoryName(int nRow) const
{
    AddressBookCategoryDB *pCategory =
	AddressBookCategoryDB::GetAddressBookCategoryDB();
    int nCategory = GetCategory(nRow);
    int nCategoryRow = pCategory->FindRow(CATID, nCategory);

    return (pCategory->GetCategory(nCategoryRow));
}


 //--------------------------------------------------------------//
 // Get the name of a custom field.                              //
 //--------------------------------------------------------------//
string
AddressBookDB::GetCustomFieldName(int nIndex) const
{
    CustomFieldDB *pCustomFieldDB = CustomFieldDB::GetCustomField();
    int nRow;

#ifdef DEBUG
    assert(nIndex >= 0 && nIndex < 4);
#endif

    nRow = pCustomFieldDB->FindRow(CUSTOMID, nIndex);
    return (pCustomFieldDB->GetName(nRow));
}


 //--------------------------------------------------------------//
 // Get the name of an Info field.                               //
 //--------------------------------------------------------------//
string
AddressBookDB::GetInfoName(int nIndex)
{
    InfoDB *pInfoDB = InfoDB::GetInfo();
    int nInfoRow = pInfoDB->FindRow(INFOID, nIndex);

    return (pInfoDB->GetInfoType(nInfoRow));
}


 //--------------------------------------------------------------//
 // Get the name of an Info field.                               //
 //--------------------------------------------------------------//
string
AddressBookDB::GetInfoName(int nRow, int nIndex) const
{
    int nNameIndex;

#ifdef DEBUG
    assert(nIndex >= 0 && nIndex < 7);
#endif

    nNameIndex = GetInfoID(nRow, nIndex);
    return (GetInfoName(nNameIndex));
}


 //--------------------------------------------------------------//
 // Get the notes for a row.                                     //
 //--------------------------------------------------------------//
Note *
AddressBookDB::GetNote(int nRow) const
{
    Note *pReturn;
    string strNoteFile = GetNoteFile(nRow);

    if (strNoteFile.length() == 0) {
	// No file name
	pReturn = new Note(0, ADDRESS_BOOK_NOTES_PREFIX);
    } else {
	// Already have a file
	pReturn = new Note(strNoteFile.c_str(), 0, ADDRESS_BOOK_NOTES_PREFIX);
    }
    return (pReturn);
}


 //--------------------------------------------------------------//
 // Import from a delimited string.                              //
 //--------------------------------------------------------------//
int
AddressBookDB::Import(const vector < string > &vExportString)
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
    pNote = new Note(0, ADDRESS_BOOK_NOTES_PREFIX);
    pNote->SetText(strNote.c_str());
    SetNote(nRow, pNote);
    delete pNote;

    // Now fix the record number
    SetStringValue(nRow, AB_RECNO, "0");
    SetHighStringKey(nRow, AB_RECNO);

    return (nRow);
}


 //--------------------------------------------------------------//
 // Insert a new row and set its key id.                         //
 //--------------------------------------------------------------//
int
AddressBookDB::Insert()
{
    printf("DEBUG (%x)->Insert()\n", this);
    int nRow = NxDbAccess::Insert();

    // Now set a unique key value
    //      SetHighStringKey(nRow,AB_RECNO);
    SetHighKey(nRow, AB_RECNO);

    return (nRow);
}


//--------------------------------------------------------------//
// Get the number of records for a category.                    //
//--------------------------------------------------------------//
int
AddressBookDB::NumRecsByCategory(int nCategory)
{
    char szCategory[16];

    sprintf(szCategory, "%d", nCategory);
    return (NumRecsByKey(AB_CAT, szCategory));
}


//--------------------------------------------------------------//
// Set the notes for this address book line.                    //
//--------------------------------------------------------------//
void
AddressBookDB::SetNote(int nRow, Note * pNote)
{
    // Save to set the file name
    pNote->Save();

    // Save the note file name
    SetNoteFile(nRow, pNote->GetFileName().c_str());
}
