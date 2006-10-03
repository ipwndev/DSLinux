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
#ifndef NXDBACCESS_H_

#define NXDBACCESS_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <cassert>
#include <set>
#include <string>
#include <vector>
#include <nxdb.h>
#include "NxDbRow.h"
using namespace std;
typedef bool(*FNCUSTOMCOMPARE) (NxDbRow * pRow1,	// Custom comparison routine for sorting
				NxDbRow * pRow2);
class NxDbAccess
{
  public:NxDbAccess(const char *pszFileName,
	       // Constructor, opens a database
	       /*const */ fildes * pDescription,
	       /*const */ field * pField);
      virtual ~ NxDbAccess();	// Destructor, closes the database
    static void CloseAll();	// Close all open data bases
    static void RefreshAll();	// Close and reopen all data bases
    virtual void Delete(int nRow);	// Delete a row from the data base
    void Dump(const char *pszFileName);	// Dump the database contents to a file
    static void DumpAll();	// Dump all open databases
    virtual inline void Export(int nRow,	// Convert a row to a set of strings
			       vector < string > &vExportString)
    {

#ifdef DEBUG
	assert(nRow >= 0 && nRow < NumRecs());

#endif				/*  */
	m_vpRow[nRow]->Export(vExportString);
    }
    int FindFirst(int nCol,	// Find the first row with a column like pszValue
		  const char *pszValue);
    inline int FindPhysicalRecord(int nRecordNumber)	// Find a row by physical record number
    {
	return (FindPhysicalRecord(nRecordNumber, m_vpRow));
    }
    static int FindPhysicalRecord(int nRecordNumber,	// Get the physical record number for a local row
				  ROW_VECTOR vpRow);
    int FindRealRow(int nRow,	// Find the real row number for a local row
		    ROW_VECTOR & vpRow);
    int FindRow(int nCol,	// Find a row by key value
		int nValue);
    int FindRow(int nCol,	// Find a row by key value
		const char *pszValue);
    int GetColumnSize(int nCol);	// Get the maximum size of a column
    inline int GetIntValue(int nRow, int nCol) const	// Get the value of this column as an integer
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif				/*  */
	return (m_vpRow[nRow]->GetIntValue(nCol));
    }

    inline int GetFlags(int nRow)
    {
#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);
#endif

	return m_vpRow[nRow]->GetFlags();
    }

    inline void SetFlags(int nRow, int flags)
    {
#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);
#endif

	m_vpRow[nRow]->SetFlags(flags);
    }

    int GetLocalRow(int nRow,	// Get the local row number for a real row
		    ROW_VECTOR & vpRow);
    inline int GetRecordNumber(int nRow,	// Get the physical record number for this row
			       ROW_VECTOR & vpRow)
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	return (vpRow[nRow]->GetRecordNumber());
    }
    inline int GetRecordNumber(int nRow)	// Get the physical record number for this row
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	return (m_vpRow[nRow]->GetRecordNumber());
    }
    ROW_VECTOR GetRows();	// Get an array of row pointers
    inline string GetStringValue(int nRow, int nCol) const	// Get the value of this column as a string
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif				/*  */
	return (m_vpRow[nRow]->GetStringValue(nCol));
    }
    virtual int Import(const vector < string > &strData);	// Import a delimited string as a new row

    virtual int Insert();	// Insert a new row into the data base, derived classes must set any key value as needed
    inline bool IsDeleted(int nRow)	// Determine if this is a deleted row
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	return (m_vpRow[nRow]->IsDeleted());
    }
    static inline bool IsDeleted(int nRow,	// Determine if a local copy of a row is deleted
				 ROW_VECTOR & vpRow)
    {

#ifdef DEBUG
	assert(nRow < (int) vpRow.size() && nRow >= 0);

#endif /*  */
	return (vpRow[nRow]->IsDeleted());
    }
    inline int NumRecs()	// Get the row count
    {
	return (m_pNxDb->NumRecs(m_strDbName));
    }
    int NumRecsByKey(int nCol,	// Get the row count for rows with a particular value
		     const char *pszValue);
    int NumRecsByKey(int nCol,	// Get the row count for rows with a particular value
		     int nValue);
    int NumUndeletedRecs();	// Get the number of undeleted rows
    bool Save();		// Write all changed rows to disk
    void UpdateFlags();         // Update the flags of the saved objects

    static void SaveCmdLine(int argc,	// Save pointers to the command line arguments
			    char **argv);
    bool Search(int nRow,	// Search all character strings in a row for a match to another string
		const char *pszString, bool bMatchWholeWord, bool bMatchCase);
    inline bool SetColumn(int nRow,	// Set the value of a column
			  int nCol, const char *pszValue)
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	return (m_vpRow[nRow]->SetColumn(nCol, pszValue));
    }
    inline bool SetColumn(int nRow,	// Set the value of a column
			  int nCol, int nValue)
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	return (m_vpRow[nRow]->SetColumn(nCol, nValue));
    }
    void SetHighKey(int nRow,	// Set a unique key value for a column
		    int nKeyColumn);
    void SetHighStringKey(int nRow,	// Set a unique key value for a character string column
			  int nKeyColumn);
    void Sort(FNCUSTOMCOMPARE pfnCompareRoutine);	// Sort the rows in memory with a custom comparison routine
    void Sort(FNCUSTOMCOMPARE pfnCompareRoutine,	// Sort a local copy of the rows with a custom comparison routine
	      ROW_VECTOR & vRow);
    void Sort(int nCol);	// Sort the rows in memory
    bool TestDuplicate(const char *pszString,	// Test for a duplicate string in the database
		       int nExistingKey, int nStringColumn, int nKeyColumn);
    void Close();		// Close this data base (used in some derived constructors)
    void Open();

    void GetSchema(vector < char >&, vector < int >&);

  protected:bool m_bClosed;	// True if an early close was performed in a derived constructor
    inline void SetStringValue(int nRow,	// Set the value of a column
			       int nCol, const char *pszValue)
    {

#ifdef DEBUG
	assert(nRow < (int) m_vpRow.size() && nRow >= 0);

#endif /*  */
	m_vpRow[nRow]->SetStringValue(nCol, pszValue);
    }
  private:char *m_pszFileName;
    fildes *m_pDescription;
    field *m_pField;
    bool m_bOpen;		// Is the database currently open
    static char **m_ppszArgv;	// Command line arguments
    static int m_nArgc;		// Command line arguments
    static int m_nSortColumn;	// Kludge - the column currently being sorted on
    int m_nSortOrder;		// Last column sorted on
    static set < NxDbAccess * >m_setOpenDB;	// List of pointers to currently open data bases
    static NxDb *m_pNxDb;	// The database object, kept open to prevent file sharing
    ROW_VECTOR m_vpRow;		// Rows for this data base
    string m_strDbName;		// Name of the data base for NxDb
    static bool(*m_pfnCompareRoutine) (NxDbRow *, NxDbRow *);	// Kludge - Custom comparison routine currently in use
    static bool CompareCustom(NxDbRow * pRow1,	// Compare rows using a custom comparison routine
			      NxDbRow * pRow2);
    static int CompareDeleted(NxDbRow * pRow1,	// Compare the deleted status of two rows
			      NxDbRow * pRow2);
    static bool CompareInteger(NxDbRow * pRow1,	// Compare rows by contents of an integer column
			       NxDbRow * pRow2);
    static bool ComparePhysicalRecord(NxDbRow * pRow1,	// Compare rows by physical record number
				      NxDbRow * pRow2);
    static bool CompareString(NxDbRow * pRow1,	// Compare rows by contents of a string column
			      NxDbRow * pRow2);
    static void DoSort(FNCUSTOMCOMPARE pfnCompareRoutine,	// Workaround for Visual C/STL problem
		       ROW_VECTOR & vpRow);
    void ReadAll();		// Read all rows from the database into memory
    bool SearchString(const char *pszHaystack,	// Search for one string in another
		      const char *pszNeedle, bool bMatchWholeWord,
		      bool bMatchCase);
};


#endif /*  */
