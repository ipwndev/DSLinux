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
// Class to access an NxDb database for desktop applications.   //
// This keeps the contents of the database (file) in memory and //
// will write out changes only to disk as needed.               //
//--------------------------------------------------------------//
#include <cassert>
#include <ctime>
#include <fstream>
#include "NxDbAccess.h"
#include "Options.h"
#include <algorithm>

#include "VCMemoryLeak.h"

#ifdef WIN32
// Visual C mis-named it
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif


//--------------------------------------------------------------//
// Pointer to the one and only NxDb object.                     //
//--------------------------------------------------------------//
NxDb *
    NxDbAccess::m_pNxDb =
    NULL;


//--------------------------------------------------------------//
// Command line arguments.                                      //
//--------------------------------------------------------------//
int
    NxDbAccess::m_nArgc =
    0;
char **
    NxDbAccess::m_ppszArgv =
    NULL;


//--------------------------------------------------------------//
// Set of all open databases.                                   //
//--------------------------------------------------------------//
set < NxDbAccess * >NxDbAccess::m_setOpenDB;


//--------------------------------------------------------------//
// Column number currently being sorted on.                     //
// No other way to get it to the comparison routines.           //
//--------------------------------------------------------------//
int
    NxDbAccess::m_nSortColumn = -
    1;
bool(*NxDbAccess::m_pfnCompareRoutine) (NxDbRow *, NxDbRow *);


//--------------------------------------------------------------//
// Constructor - opens a database                               //
//--------------------------------------------------------------//
NxDbAccess::NxDbAccess(const char *pszFileName,
		       /*const */ fildes * pDescription,
		       /*const */ field * pField)
{
    m_pszFileName = strdup(pszFileName);
    m_pDescription = pDescription;
    m_pField = pField;

    //m_pNxDb = NULL;  

    Open();
}

void
NxDbAccess::Open()
{
    m_bOpen = false;
    m_bClosed = false;

    if (m_pNxDb == NULL) {
	m_pNxDb = new NxDb(m_nArgc, m_ppszArgv);
	m_pNxDb->SetPath(Options::GetDatabasePath().c_str());
    }
    // Open or Create the database
    m_strDbName = m_pszFileName;
    if (!m_pNxDb->Open(m_pszFileName, m_pDescription, m_pField, 0)) {
	if (m_pNxDb->Create(m_pszFileName, m_pDescription, m_pField, 0)) {
	    if (m_pNxDb->Open(m_pszFileName, m_pDescription, m_pField, 0)) {
		m_bOpen = true;
	    }
	}
    } else {
	m_bOpen = true;
    }

    // Read all rows from the data base if open
    if (m_bOpen == true) {
	// Read all rows from this data base
	ReadAll();
    }
    // Add this to the set of open data bases
    m_setOpenDB.insert(this);
}


//--------------------------------------------------------------//
// Destructor - closes the database                             //
//--------------------------------------------------------------//
NxDbAccess::~NxDbAccess()
{
    // Close this database
    Close();

    // Destroy the NxDb object if this is the last NxDbAccess object
    if (m_setOpenDB.size() == 0) {
	delete m_pNxDb;
	m_pNxDb = NULL;
    }
}


//--------------------------------------------------------------//
// Close this data base, used by some derived constructors.     //
//--------------------------------------------------------------//
void
NxDbAccess::Close()
{
    // Close this data base if not already closed
    if (m_bClosed == false) {
	set < NxDbAccess * >::iterator iter;
	unsigned int i;

	// Destroy the rows saved for this data base
	for (i = 0; i < m_vpRow.size(); ++i) {
	    delete m_vpRow[i];
	}

	// Remove this object from the set of open data bases
	iter = m_setOpenDB.find(this);
#ifdef DEBUG
	assert(iter != m_setOpenDB.end());	// Must have found this
#endif
	m_setOpenDB.erase(iter);

	// Close this data base
	if (m_bClosed == false) {
	    m_pNxDb->Close(m_strDbName.c_str());
	}

	m_vpRow.erase(m_vpRow.begin(), m_vpRow.end());

	// Set the closed flag
	m_bClosed = true;
    }
}


void
NxDbAccess::RefreshAll()
{
    // The destructor for NxDbAccess removes entries from the set m_setOpenDB
    set < NxDbAccess * >::iterator iter;
    set < NxDbAccess * >tmp;

#ifdef DEBUG
    printf("refreshing %d databases\n", m_setOpenDB.size());
#endif

    for (iter = m_setOpenDB.begin(); iter != m_setOpenDB.end(); ++iter) {
	tmp.insert(*(iter));
	(*(iter))->Close();
    }

    for (iter = tmp.begin(); iter != tmp.end(); ++iter) {
	(*(iter))->Open();
    }
}

//--------------------------------------------------------------//
// Close all open data bases                                    //
//--------------------------------------------------------------//
void
NxDbAccess::CloseAll()
{
    NxDbAccess *pNxDbAccess;

    // The destructor for NxDbAccess removes entries from the set m_setOpenDB
    while (m_setOpenDB.size() > 0) {
	pNxDbAccess = *(m_setOpenDB.begin());
	delete pNxDbAccess;
    }
}


//--------------------------------------------------------------//
// Compare rows using a custom comparison routine               //
//--------------------------------------------------------------//
bool
NxDbAccess::CompareCustom(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    switch (CompareDeleted(pRow1, pRow2)) {
    case -1:			// First row was deleted, but not the second
	bReturn = false;
	break;

    case 0:			// Both have same status
	bReturn = (*m_pfnCompareRoutine) (pRow1, pRow2);
	break;

    case 1:			// Second row was deleted, but not the first
	bReturn = true;
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare the deleted status of two rows                       //
// Returns: 0 if both rows are deleted or not deleted           //
//          1 if the second row is deleted                      //
//          -1 if the first row is deleted                      //
//--------------------------------------------------------------//
int
NxDbAccess::CompareDeleted(NxDbRow * pRow1, NxDbRow * pRow2)
{
    int nReturn;

    if (pRow1->IsDeleted() == pRow2->IsDeleted()) {
	// Same status
	nReturn = 0;
    } else if (pRow1->IsDeleted()) {
	// First row is deleted - higher
	nReturn = -1;
    } else			// if (pRow2->IsDeleted())
    {
	// Second row is deleted - higher
	nReturn = +1;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Compare rows by contents of an integer column                //
//--------------------------------------------------------------//
bool
NxDbAccess::CompareInteger(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    switch (CompareDeleted(pRow1, pRow2)) {
    case -1:			// First row was deleted, but not the second
	bReturn = false;
	break;

    case 0:			// Both have same status
	bReturn =
	    pRow1->GetIntValue(m_nSortColumn) <
	    pRow2->GetIntValue(m_nSortColumn);
	break;

    case 1:			// Second row was deleted, but not the first
	bReturn = true;
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Compare rows by physical record number                       //
//--------------------------------------------------------------//
bool
NxDbAccess::ComparePhysicalRecord(NxDbRow * pRow1, NxDbRow * pRow2)
{
    return (pRow1->GetRecordNumber() < pRow2->GetRecordNumber());
}


//--------------------------------------------------------------//
// Compare rows by contents of a string column                  //
//--------------------------------------------------------------//
bool
NxDbAccess::CompareString(NxDbRow * pRow1, NxDbRow * pRow2)
{
    bool bReturn;

    switch (CompareDeleted(pRow1, pRow2)) {
    case -1:			// First row was deleted, but not the second
	bReturn = false;
	break;

    case 0:			// Both have same status
	bReturn =
	    pRow1->GetStringValue(m_nSortColumn) <
	    pRow2->GetStringValue(m_nSortColumn);
	break;

    case 1:			// Second row was deleted, but not the first
	bReturn = true;
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Delete a row from the data base.                             //
//--------------------------------------------------------------//
void
NxDbAccess::Delete(int nRow)
{
#ifdef DEBUG
    assert(nRow < (int) m_vpRow.size() && nRow >= 0);
#endif

    // Delete the row
    m_vpRow[nRow]->Delete();

    // Delete this row immediately
    Save();

    // Mark as not sorted
    if (m_nSortOrder != -1) {
	// Set as unsorted only if not in physical record order
	m_nSortOrder = -2;
    }
}


//--------------------------------------------------------------//
// Workaround to avoid MS Visual C linker error LNK1179 - call  //
// stable_sort from only one place in the source module.        //
//--------------------------------------------------------------//
void
NxDbAccess::DoSort(FNCUSTOMCOMPARE pfnCompareRoutine, ROW_VECTOR & vpRow)
{
    // Now sort the rows
    stable_sort(vpRow.begin(), vpRow.end(), pfnCompareRoutine);
}


//--------------------------------------------------------------//
// Find the real row number for a local row.                    //
//--------------------------------------------------------------//
int
NxDbAccess::FindRealRow(int nRow, ROW_VECTOR & vpRow)
{
#ifdef DEBUG
    assert(nRow >= 0 && (unsigned int) nRow < vpRow.size());
#endif

    return (FindPhysicalRecord(vpRow[nRow]->GetRecordNumber()));
}


//--------------------------------------------------------------//
// Find a row by key value.                                     //
//--------------------------------------------------------------//
int
NxDbAccess::FindRow(int nCol, int nValue)
{
    int i;
    int nMax = m_vpRow.size();

    // Serial search ...
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false
	    && m_vpRow[i]->GetIntValue(nCol) == nValue) {
	    break;
	}
    }

    // Reset the return value if not found
    if (i >= nMax) {
	i = -1;
    }

    return (i);
}


//--------------------------------------------------------------//
// Dump the contents of a database to a text file.              //
//--------------------------------------------------------------//
void
NxDbAccess::Dump(const char *pszFileName)
{
    char szTime[24];
    field *pField = m_pNxDb->GetField(m_strDbName);
    fildes *pDesc = m_pNxDb->GetFilDes(m_strDbName);
    int nCol;
    int nMax;
    int nRow;
    ofstream fileOut;
    string strData;
    string strFormatted;
    time_t time_value;
    unsigned int i;

    fileOut.open(pszFileName);
    if (fileOut.is_open()) {
	// Dump the file name
	fileOut << m_strDbName.c_str()
	    << endl;

	// Dump each undeleted row
	nMax = NumRecs();
	for (nRow = 0; nRow < nMax; ++nRow) {
	    if (!IsDeleted(nRow)) {
		// Dump the physical record number
		fileOut << "#" << GetRecordNumber(nRow)
		    << " ";

		// Dump each field
		for (nCol = 0; nCol < pDesc->nfields; ++nCol) {
		    if (nCol > 0) {
			fileOut << ",";
		    }
		    switch (pField[nCol].type) {
		    case 'c':	// Character string
			strData = GetStringValue(nRow, nCol);
			strFormatted = "";
			for (i = 0; i < strData.length(); ++i) {
			    if (strData[i] >= ' ' && strData[i] <= '~') {
				strFormatted += strData[i];
			    } else if (strData[i] == '\"') {
				strFormatted += "\\\"";
			    } else if (strData[i] == '\'') {
				strFormatted += "\\\'";
			    } else if (strData[i] == '\n') {
				strFormatted += "\\n";
			    } else if (strData[i] == '\r') {
				strFormatted += "\\r";
			    } else if (strData[i] == '\t') {
				strFormatted += "\\t";
			    } else {
				char szData[6];

				sprintf(szData, "\\0x%02x",
					(unsigned) strData[i]);
				strFormatted += szData;
			    }
			}
			fileOut << "\"" << strFormatted.c_str()
			    << "\"";
			break;

		    case 'd':	// Date field
			time_value = GetIntValue(nRow, nCol);
			strftime(szTime, sizeof(szTime), "%Y/%m/%d %H:%M:%S",
				 localtime(&time_value));
			fileOut << "\"" << szTime << "\"";
			break;

		    default:	// All others must be numeric
			// Catch date fields marked as longs
			if (GetIntValue(nRow, nCol) > 1000000000) {
			    time_value = GetIntValue(nRow, nCol);
			    strftime(szTime, sizeof(szTime),
				     "%Y/%m/%d %H:%M:%S",
				     localtime(&time_value));
			    fileOut << "\"" << szTime << "\"";
			} else {
			    // Treat as a normal integer
			    fileOut << GetIntValue(nRow, nCol);
			}
		    }
		}

		// End this line
		fileOut << endl;
	    }
	}

	// Close the file
	fileOut.close();
    }
}


//--------------------------------------------------------------//
// Dump all open databases to text files.                       //
//--------------------------------------------------------------//
void
NxDbAccess::DumpAll()
{
    char szFileName[16];
    int i = 0;
    set < NxDbAccess * >::iterator iter;

    for (iter = m_setOpenDB.begin(); iter != m_setOpenDB.end(); ++iter) {
	sprintf(szFileName, "NxDb%04d.txt", ++i);
	(*(iter))->Dump(szFileName);
    }
}


//--------------------------------------------------------------//
// Find the first row with a column like "pszValue".            //
//--------------------------------------------------------------//
int
NxDbAccess::FindFirst(int nCol, const char *pszValue)
{
    int i;
    int nLength = strlen(pszValue);
    int nMax = m_vpRow.size();

    // Serial search ...
    for (i = 0; i < nMax; ++i) {
	if (strncasecmp
	    (m_vpRow[i]->GetStringValue(nCol).c_str(), pszValue,
	     nLength) == 0) {
	    break;
	}
    }

    // Reset the return value if not found
    if (i >= nMax) {
	i = -1;
    }

    return (i);
}


//--------------------------------------------------------------//
// Find a row by key value.                                     //
//--------------------------------------------------------------//
int
NxDbAccess::FindRow(int nCol, const char *pszValue)
{
    int i;
    int nMax = m_vpRow.size();

    // Serial search ...
    for (i = 0; i < nMax; ++i) {
	if (strcasecmp(m_vpRow[i]->GetStringValue(nCol).c_str(), pszValue) ==
	    0) {
	    break;
	}
    }

    // Reset the return value if not found
    if (i >= nMax) {
	i = -1;
    }

    return (i);
}


//--------------------------------------------------------------//
// Find a row by physical record number.                        //
//--------------------------------------------------------------//
int
NxDbAccess::FindPhysicalRecord(int nRecordNumber, ROW_VECTOR vpRow)
{
    int i;
    int nMax = vpRow.size();

    for (i = 0; i < nMax; ++i) {
	if (vpRow[i]->GetRecordNumber() == nRecordNumber) {
	    break;
	}
    }

    // Was it found ?
    if (i >= nMax) {
	// Not found
	i = -1;
    }
    return (i);
}


//--------------------------------------------------------------//
// Get the size of a column.  The returned size is size-1 for   //
// character string columns as NxDb must have room for a null   //
// terminator in its buffers.                                   //
//--------------------------------------------------------------//
int
NxDbAccess::GetColumnSize(int nCol)
{
    field *pField = m_pNxDb->GetField(m_strDbName);
#ifdef DEBUG
    fildes *pFileDesc = m_pNxDb->GetFilDes(m_strDbName);

    assert(nCol >= 0 && nCol < pFileDesc->nfields);
#endif

    return (pField[nCol].size);
}


//--------------------------------------------------------------//
// Insert a new row into the database.  Returns the row number. //
//--------------------------------------------------------------//
int
NxDbAccess::Insert()
{
    int i;
    int j;
    int nMax = m_vpRow.size();
    int nRecno = -1;
    int nRow;
    NxDbRow *pRow;

    // Find the lowest numbered deleted record
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == true) {
	    j = m_vpRow[i]->GetRecordNumber();
	    if (nRecno > j || nRecno == -1) {
		nRecno = j;
		nRow = i;
	    }
	}
    }
    if (nRecno == -1) {
	// Create the new row
	pRow = new NxDbRow(m_pNxDb->GetField(m_strDbName));
	//pRow=new NxDbRow(m_pField);
	nRow = m_vpRow.size();
	pRow->SetRecordNumber(nRow + 1);
	m_vpRow.push_back(pRow);
    } else {
	// Reuse an old row
	m_vpRow[nRow]->Clear();
    }

    // Create this row immediately
    Save();

    // Mark as not sorted
    if (m_nSortOrder != -1) {
	// Set as unsorted only if not in physical record order
	m_nSortOrder = -2;
    }

    return (nRow);
}


//--------------------------------------------------------------//
// Get the number of undeleted rows with a matching key value   //
//--------------------------------------------------------------//
int
NxDbAccess::NumRecsByKey(int nCol, const char *pszValue)
{
    int i;
    int nCount = 0;
    int nMax = m_vpRow.size();

    // Count records
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false
	    && m_vpRow[i]->GetStringValue(nCol) == pszValue) {
	    ++nCount;
	}
    }

    return (nCount);
}


//--------------------------------------------------------------//
// Get a local row number given a real row number.              //
//--------------------------------------------------------------//
int
NxDbAccess::GetLocalRow(int nRow, ROW_VECTOR & vpRow)
{
    int i;
    int nMax;
    int nRecord;

#ifdef DEBUG
    assert((unsigned int) nRow < m_vpRow.size());
#endif

    nRecord = m_vpRow[nRow]->GetRecordNumber();
    nMax = vpRow.size();
    for (i = 0; i < nMax; ++i) {
	if (vpRow[i]->GetRecordNumber() == nRecord) {
	    break;
	}
    }

#ifdef DEBUG
    assert(i < nMax);		// Or the row was not found
#endif

    return (i);
}


//--------------------------------------------------------------//
// Get an array of row pointers, this array is used by some of  //
// the display widgets to set up a custom ordering of the rows  //
// being displayed.  This allows multiple widgets to have       //
// different orderings of the rows.                             //
//--------------------------------------------------------------//
ROW_VECTOR
NxDbAccess::GetRows()
{
    ROW_VECTOR vReturn = m_vpRow;

    return (vReturn);
}


//--------------------------------------------------------------//
// Import a row from a string                                   //
//--------------------------------------------------------------//
int
NxDbAccess::Import(const vector < string > &vExportString)
{
    int nRow;

    // Insert a new row
    nRow = Insert();

    // Move the column data into it
    m_vpRow[nRow]->Import(vExportString);

    return (nRow);
}


//--------------------------------------------------------------//
// Get the number of undeleted rows with a matching key value   //
//--------------------------------------------------------------//
int
NxDbAccess::NumRecsByKey(int nCol, int nValue)
{
    int i;
    int nCount = 0;
    int nMax = m_vpRow.size();

    // Count records
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false
	    && m_vpRow[i]->GetIntValue(nCol) == nValue) {
	    ++nCount;
	}
    }

    return (nCount);
}


//--------------------------------------------------------------//
// Get the number of undeleted rows                             //
//--------------------------------------------------------------//
int
NxDbAccess::NumUndeletedRecs()
{
    int i;
    int nCount = 0;
    int nMax = m_vpRow.size();

    // Count records
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false) {
	    ++nCount;
	}
    }

    return (nCount);
}


//--------------------------------------------------------------//
// Read all rows from the database into memory.                 //
//--------------------------------------------------------------//
void
NxDbAccess::ReadAll()
{
    int i;
    int nMax = m_pNxDb->NumRecs(m_strDbName);
    NxDbRow *pRow;

    // Assume that the database is open
    // Read each record
    for (i = 0; i < nMax; ++i) {
	pRow = new NxDbRow(m_strDbName, i + 1, m_pNxDb);
	m_vpRow.push_back(pRow);
    }

    // Set that the sort order is by physical placement
    m_nSortOrder = -1;
}


//--------------------------------------------------------------//
// Write all changed rows to disk.                              //
//--------------------------------------------------------------//
bool
NxDbAccess::Save()
{
    bool bReturn = true;
    int i;
    int nMax = m_vpRow.size();

    // Save each row
    for (i = 0; i < nMax && bReturn; ++i) {
	bReturn &= (m_vpRow[i]->Save(m_strDbName, m_pNxDb));
    }
    return (bReturn);
}

//--------------------------------------------------------------//
// Update all of the flags to be zero                           //
//--------------------------------------------------------------//

void
NxDbAccess::UpdateFlags()
{
    int i;
    int nMax = m_vpRow.size();

    /* Update the rows */

    for (i = 0; i < nMax; ++i) {
      m_vpRow[i]->UpdateFlags(m_strDbName, m_pNxDb);
    }
    
}

//--------------------------------------------------------------//
// Save a copy of the command line arguments for use when       //
// opening NxDb objects                                         //
//--------------------------------------------------------------//
void
NxDbAccess::SaveCmdLine(int argc, char **argv)
{
    m_nArgc = argc;
    m_ppszArgv = argv;
}


//--------------------------------------------------------------//
// Search all character string columns for a match to a string  //
//--------------------------------------------------------------//
bool
NxDbAccess::Search(int nRow,
		   const char *pszString,
		   bool bMatchWholeWord, bool bMatchCase)
{
    bool bReturn = false;
    int i;

    // Deleted records do not match
    if (IsDeleted(nRow) == false) {
	field *pField = m_pNxDb->GetField(m_strDbName);
	fildes *pFileDesc = m_pNxDb->GetFilDes(m_strDbName);

	// Search all character columns
	for (i = 0; i < pFileDesc->nfields && bReturn == false; ++i) {
	    if (pField[i].type == 'c') {
		bReturn = SearchString(GetStringValue(nRow, i).c_str(),
				       pszString,
				       bMatchWholeWord, bMatchCase);
	    }
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Search for one string in another. complicated search because //
// of the match whole word and match case qualifiers.           //
//--------------------------------------------------------------//
bool
NxDbAccess::SearchString(const char *pszHaystack,
			 const char *pszNeedle,
			 bool bMatchWholeWord, bool bMatchCase)
{
    bool bMatch;
    bool bReturn = false;
    const char *pszData;
    int nChr;
    int nHaystackLen = strlen(pszHaystack);
    int nNeedleLen = strlen(pszNeedle);

    // Set up for the first case comparison
    if (bMatchCase == false) {
	nChr = tolower(pszNeedle[0]);
    } else {
	nChr = pszNeedle[0];
    }

    pszData = pszHaystack;
    while (pszData != NULL && bReturn == false) {
	bMatch = true;
	pszData = strchr(pszData, nChr);
	if (pszData != NULL
	    && (pszData - pszHaystack) + nNeedleLen <= nHaystackLen) {
	    if (bMatchWholeWord == true) {
		if (pszData != pszHaystack && isalnum(*(pszData - 1)) == 1
		    && (pszData - pszHaystack) + nNeedleLen != nHaystackLen
		    && isalnum(*pszData + nNeedleLen == 1)) {
		    bMatch = false;
		}
	    }

	    if (bMatch == true) {
		if (bMatchCase == true) {
		    bMatch = (strncmp(pszData, pszNeedle, nNeedleLen) == 0);
		} else {
		    bMatch =
			(strncasecmp(pszData, pszNeedle, nNeedleLen) == 0);
		}
	    }
	    // Set whether this matches or not
	    bReturn = bMatch;
	}
	// Fix for the next loop
	if (pszData != NULL) {
	    ++pszData;
	}
    }

    // If no match, try the other case
    if (bReturn == false && bMatchCase == false && toupper(nChr) != nChr) {
	// Set up for the second case comparison
	nChr = toupper(pszNeedle[0]);

	pszData = pszHaystack;
	while (pszData != NULL && bReturn == false) {
	    bMatch = true;
	    pszData = strchr(pszData, nChr);
	    if (pszData != NULL
		&& (pszData - pszHaystack) + nNeedleLen <= nHaystackLen) {
		if (bMatchWholeWord == true) {
		    if ((pszData != pszHaystack
			 && isalnum(*(pszData - 1)) != 0)
			|| ((pszData - pszHaystack) + nNeedleLen !=
			    nHaystackLen
			    && isalnum(pszData[nNeedleLen]) != 0)) {
			bMatch = false;
		    }
		}

		if (bMatch == true) {
		    if (bMatchCase == true) {
			bMatch =
			    (strncmp(pszData, pszNeedle, nNeedleLen) == 0);
		    } else {
			bMatch =
			    (strncasecmp(pszData, pszNeedle, nNeedleLen) ==
			     0);
		    }
		}
		// Set whether this matches or not
		bReturn = bMatch;
	    }
	    // Fix for the next loop
	    if (pszData != NULL) {
		++pszData;
	    }
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Set a unique key value for a column                          //
//--------------------------------------------------------------//
void
NxDbAccess::SetHighKey(int nRow, int nKeyColumn)
{
    int i;
    int nHighKey = 0;
    int nMax = m_vpRow.size();
    int nValue;

    // Now find the highest key value so far
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false) {
	    nValue = GetIntValue(i, nKeyColumn);
	    if (nValue > nHighKey) {
		nHighKey = nValue;
	    }
	}
    }

    // Now set the key id for this record
    SetColumn(nRow, nKeyColumn, nHighKey + 1);

    // Save this change special as otherwise an unkeyed row
    // may be left in the data base if the program fails
    Save();
}


//--------------------------------------------------------------//
// Set a unique key value for a character string column         //
//--------------------------------------------------------------//
void
NxDbAccess::SetHighStringKey(int nRow, int nKeyColumn)
{
    char szKey[16];
    int i;
    int nHighKey = 0;
    int nMax = m_vpRow.size();
    int nValue;

    // Now find the highest key value so far
    for (i = 0; i < nMax; ++i) {
	if (m_vpRow[i]->IsDeleted() == false) {
	    nValue = atoi(GetStringValue(i, nKeyColumn).c_str());
	    if (nValue > nHighKey) {
		nHighKey = nValue;
	    }
	}
    }

    // Now set the key id for this record
    sprintf(szKey, "%d", nHighKey + 1);
    SetColumn(nRow, nKeyColumn, szKey);

    // Save this change special as otherwise an unkeyed row
    // may be left in the data base if the program fails
    Save();
}


//--------------------------------------------------------------//
// Sort the rows in memory using a custom sort routine.         //
//--------------------------------------------------------------//
void
NxDbAccess::Sort(FNCUSTOMCOMPARE pfnCompareRoutine)
{
    // Save the custom sort routine
    m_pfnCompareRoutine = pfnCompareRoutine;

    // Now sort the rows
    DoSort(CompareCustom, m_vpRow);

    // Set the last sort order to unsorted/custom
    m_nSortOrder = -2;
}


//--------------------------------------------------------------//
// Sort a local copy of the rows using a custom sort routine.   //
//--------------------------------------------------------------//
void
NxDbAccess::Sort(FNCUSTOMCOMPARE pfnCompareRoutine, ROW_VECTOR & vpRow)
{
    // Save the custom sort routine
    m_pfnCompareRoutine = pfnCompareRoutine;

    // Now sort the rows
    DoSort(CompareCustom, vpRow);

    // Set the last sort order to unsorted/custom
    m_nSortOrder = -2;
}


//--------------------------------------------------------------//
// Sort the rows in memory.  A request to sort by column number //
// -1 will sort by the physical order in the original database. //
//--------------------------------------------------------------//
void
NxDbAccess::Sort(int nCol)
{
    bool(*pfnPr) (NxDbRow *, NxDbRow *);

    // Only sort if needed
    if (nCol != m_nSortOrder) {
	const field *pField = m_pNxDb->GetField(m_strDbName);
#ifdef DEBUG
	const fildes *pFileDescription = m_pNxDb->GetFilDes(m_strDbName);

	// The column must be available
	assert(nCol >= -1 && nCol < pFileDescription->nfields);
#endif

	// Determine the comparison routine to be used
	if (nCol == -1) {
	    // Sort by physical placement in the database
	    pfnPr = ComparePhysicalRecord;
	} else {
	    switch (pField[nCol].type) {
	    case 'c':		// Character string
		pfnPr = CompareString;
		break;

	    case 'd':		// dates
	    case 'i':		// 16-bit integers
	    case 'l':		// 32-bit integers
		pfnPr = CompareInteger;
		break;

	    default:
#ifdef DEBUG
		assert(false);	// Unknown field type
#endif
		pfnPr = NULL;	// Will GPF
	    }
	}

	// Now sort the rows
	m_nSortColumn = nCol;
	DoSort(pfnPr, m_vpRow);

	// Set the last sort order
	m_nSortOrder = nCol;
    }
}


//--------------------------------------------------------------//
// Test a string column for duplicates.  This function will     //
// test all non-deleted rows in a database to see if the string //
// duplicates an existing row.  It will not compare to the row  //
// whose key is given in the nExistingKey argument.  Returns    //
// false if this is a duplicate string.                         //
//--------------------------------------------------------------//
bool
NxDbAccess::TestDuplicate(const char *pszString,
			  int nExistingKey, int nStringColumn, int nKeyColumn)
{
    bool bReturn = true;
    int i;
    int nMax = NumRecs();

    for (i = 0; i < nMax; ++i) {
	// Only look at un-deleted rows
	if (IsDeleted(i) == false) {
	    // Don't compare if this is the particular key to ignore
	    if (GetIntValue(i, nKeyColumn) != nExistingKey) {
		// Note - case sensitive comparison...
		if (GetStringValue(i, nStringColumn).compare(pszString) == 0) {
		    // Duplicate found
		    bReturn = false;
		    break;
		}
	    }
	}
    }
    return (bReturn);
}

void
NxDbAccess::GetSchema(vector < char >&col_type, vector < int >&col_size)
{
    int idx = 0;

    field *pField = m_pNxDb->GetField(m_strDbName);
    field *p_field;

    for (p_field = pField; p_field->type != 0; p_field = &(pField[++idx])) {

      col_size.push_back(p_field->size);
      col_type.push_back(p_field->type);
    }
}
