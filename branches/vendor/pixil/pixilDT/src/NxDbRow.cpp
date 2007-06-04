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
// Class Used to represent a row in an NxDB database.           //
//--------------------------------------------------------------//
#include "NxDbRow.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Default constructor.                                         //
//--------------------------------------------------------------//
NxDbRow::NxDbRow()
{
    m_bChanged = true;
    m_bDeleted = false;
    m_bNeedDelete = false;
    m_bNew = true;
    m_nRecordNumber = 0;
    m_iFlags = NxDb::NEW;
}


//--------------------------------------------------------------//
// Construct with field definitions                             //
//--------------------------------------------------------------//
NxDbRow::NxDbRow(field * pField)
{
    SetColumns(pField);

    m_nRecordNumber = 0;
    m_iFlags = NxDb::NEW;
}


//--------------------------------------------------------------//
// Construct from data base info.                               //
//--------------------------------------------------------------//
NxDbRow::NxDbRow(const string & strDbName,	// Construct from data base info
		 int nRow, NxDb * pNxDb)
{
    char szBuffer[MAXRECSIZ];
    field *pField;
    int i;
    string strCopy = strDbName;

    pField = pNxDb->GetField(strCopy);
    SetColumns(pField);

    /* Grab the record flags first */
    int test;

    pNxDb->GetFlags(strCopy, nRow, test);
    m_iFlags = test;

    // Preset to not deleted
    m_bDeleted = false;

    // Read data from database
    /* FIXME:  Can we detect new and changed records here too? */

    for (i = 0; pField[i].type != '\0'; ++i) {
	if (pNxDb->Extract(strCopy, nRow, i, szBuffer) == 0) {
	    // This is a deleted record
	    m_bDeleted = true;
	    break;
	}
	if (pField[i].type == 'c') {
	    szBuffer[pField[i].size] = '\0';	// Workaround to error in NxDb class Extract method
	}
	m_vField[i].SetStringValue(szBuffer);
    }

    // Set that the data has not been changed
    m_bChanged = false;
    m_bNeedDelete = false;
    m_bNew = false;

    // Set the record number
    m_nRecordNumber = nRow;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NxDbRow::~NxDbRow()
{
}


//--------------------------------------------------------------//
// Assignment operator                                          //
//--------------------------------------------------------------//
NxDbRow & NxDbRow::operator=(const NxDbRow & Other)
{
    if (this != &Other) {
	m_bChanged = Other.m_bChanged;
	m_bDeleted = Other.m_bDeleted;
	m_bNeedDelete = Other.m_bNeedDelete;
	m_bNew = Other.m_bNew;
	m_nRecordNumber = Other.m_nRecordNumber;
	m_vField = Other.m_vField;
	m_iFlags = Other.m_iFlags;
    }

    return (*this);
}


//--------------------------------------------------------------//
// Clear all columns in this row, set up for re-use of the row. //
//--------------------------------------------------------------//
void
NxDbRow::Clear()
{
    int i;
    int nMax = m_vField.size();

    // Output each field
    for (i = 0; i < nMax; ++i) {
	m_vField[i].Clear();
    }

    // Reset the status
    m_bChanged = true;
    m_bDeleted = false;
    m_bNeedDelete = false;
    m_bNew = true;

    if (!(m_iFlags & NxDb::NEW))
	m_iFlags = NxDb::CHANGED;
    else
	m_iFlags = NxDb::NEW;

}


//--------------------------------------------------------------//
// Create the binary record to be written to the data base.     //
// The caller must delete the returned memory from here.        //
//--------------------------------------------------------------//
unsigned char *
NxDbRow::CreateRecord(string & strDbName, NxDb * pNxDb)
{
    field *pField = pNxDb->GetField(strDbName);

    int i;
    int nMax = m_vField.size();
    int nRecSize = pNxDb->GetFilDes(strDbName)->recsiz;
    unsigned char *ucPtr;
    unsigned char *ucRecord = new unsigned char[nRecSize + 1];	// One extra for unused terminator

#ifdef WIN32
    // Clear the record
    memset(ucRecord, 0, nRecSize);
#endif

    // Output each field
    for (i = 0, ucPtr = ucRecord; i < nMax; ++i) {
	m_vField[i].Output(ucPtr + pField[i].offset);
    }

    return (ucRecord);
}


//--------------------------------------------------------------//
// Export this row to a delimited string.                       //
//--------------------------------------------------------------//
void
NxDbRow::Export(vector < string > &vExportString)
{
    int i;
    int nMax = m_vField.size();
    string strReturn;

    vExportString.clear();
    for (i = 0; i < nMax; ++i) {
	vExportString.push_back(m_vField[i].Export());
    }
}


//--------------------------------------------------------------//
// Import data to this row from a delimited string.             //
//--------------------------------------------------------------//
void
NxDbRow::Import(const vector < string > &vExportString)
{
    int nColumn;
    int nMax = vExportString.size();

    for (nColumn = 0; nColumn < nMax; ++nColumn) {
	m_vField[nColumn].SetStringValue(vExportString[nColumn].c_str());
    }
}


//--------------------------------------------------------------//
// Save this row to disk if needed                              //
//--------------------------------------------------------------//
bool
NxDbRow::Save(const string & strDbName, NxDb * pNxDb)
{
    bool bReturn = true;
    string strName = strDbName;
    unsigned char *m_ucRecord = NULL;

#ifndef DEBUG
    // The external record number must have been set
    assert(m_nRecordNumber > 0);
#endif

    if (m_bDeleted == false) {
	if (m_bNeedDelete == true) {
	    // Delete this record
	    pNxDb->DeleteRec(strName, m_nRecordNumber);
	    m_bDeleted = true;
	} else if (m_bNew == true) {
	    // Insert this record
	    m_ucRecord = CreateRecord(strName, pNxDb);
	    pNxDb->Insert(strName, (char *) m_ucRecord);
	    m_bChanged = false;
	    m_bNew = false;
	} else if (m_bChanged == true) {
	    m_ucRecord = CreateRecord(strName, pNxDb);
	    pNxDb->Edit(strName, m_nRecordNumber, (char *) m_ucRecord);
	    m_bChanged = false;
	}
	// Clean up
	delete[]m_ucRecord;
    }

    /* Get the real flags */
    pNxDb->GetFlags(strName, m_nRecordNumber, m_iFlags);

    return (bReturn);
}


//--------------------------------------------------------------//
// Set the value of a column                                    //
//--------------------------------------------------------------//
bool
NxDbRow::SetColumn(int nCol, const char *pszValue)
{
    bool bReturn;

#ifdef DEBUG
    // Must fit within this row
    assert(nCol < (int) m_vField.size() && nCol >= 0);
#endif

    bReturn = m_vField[nCol].SetValue(pszValue);
    m_bChanged |= bReturn;

    if (!m_iFlags)
	m_iFlags = NxDb::CHANGED;

    return (bReturn);
}


//--------------------------------------------------------------//
// Set the value of a column                                    //
//--------------------------------------------------------------//
bool
NxDbRow::SetColumn(int nCol, int nValue)
{
    bool bReturn;

#ifdef DEBUG
    // Must fix within this row
    assert(nCol < (int) m_vField.size() && nCol >= 0);
#endif

    bReturn = m_vField[nCol].SetValue(nValue);
    m_bChanged |= bReturn;

    if (!m_iFlags)
	m_iFlags = NxDb::CHANGED;
    
    return (bReturn);
}


//--------------------------------------------------------------//
// Set the field definitions for this row.                      //
//--------------------------------------------------------------//
void
NxDbRow::SetColumns(field * pField)
{
    int i;
    NxDbColumn column(NxDbColumn::DataTypeChar);

    m_vField.clear();
    for (i = 0; pField[i].type != '\0'; ++i) {
	column.SetType(pField[i].type, pField[i].size);
	m_vField.push_back(column);
    }

    // Set that the data is new
    m_bChanged = true;
    m_bDeleted = false;
    m_bNeedDelete = false;
    m_bNew = true;
}

void
NxDbRow::UpdateFlags(const string & strDbName, NxDb * pNxDb) {
  pNxDb->SetFlags(strDbName, m_nRecordNumber, NxDb::NONE);
}

  
  
