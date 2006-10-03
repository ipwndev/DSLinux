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
#ifndef NXDBROW_H_

#define NXDBROW_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <cassert>
#include <string>
#include <vector>
#include <nxdb.h>
#include "NxDbColumn.h"
using namespace std;
class NxDbRow
{
  public:NxDbRow();		// Default constructor
    NxDbRow(field * pField);	// Construct with field definitions
    NxDbRow(const string & strDbName,	// Construct from data base info
	    int nRow, NxDb * pNxDb);
    inline NxDbRow(const NxDbRow & Other)	// Copy constructor
    {
	*this = Other;
    }
     ~NxDbRow();		// Destructor, closes the database
    NxDbRow & operator=(const NxDbRow & Other);	// Assignment operator
    void Clear();		// Clear all columns in this row
    inline void Delete()	// Mark this row for deletion
    {
	m_bNeedDelete = true;
    }
    void Export(vector < string > &vExportString);	// Export this row to a delimited string
    inline int GetIntValue(int nCol)	// Get the value of this column as an integer
    {

#ifdef DEBUG
	// Must exist within this row
	assert(nCol < (int) m_vField.size() && nCol >= 0);

#endif /*  */
	return (m_vField[nCol].GetIntValue());
    }
    inline int GetRecordNumber() const	// Get the physical record number
    {
	return (m_nRecordNumber);
    }
    inline string GetStringValue(int nCol) const	// Get the value of this column as a string
    {

#ifdef DEBUG
	// Must exist within this row
	assert(nCol < (int) m_vField.size() && nCol >= 0);

#endif				/*  */
	return (m_vField[nCol].GetStringValue());
    }
    void Import(const vector < string > &vExportString);	// Import data into this row
    inline bool IsDeleted() const	// Return whether this row has been deleted or is scheduled for deletion
    {
	return (m_bDeleted || m_bNeedDelete);
    }

    inline int GetFlags() const
    {
	return (m_iFlags);
    }

    inline void SetFlags(int flags)
    {
	m_iFlags = flags;
    }

    bool Save(const string & strDbName,	// Save this row to disk, may delete or insert
	      NxDb * pNxDb);

    void UpdateFlags(const string &, NxDb *);

    bool SetColumn(int nCol,	// Set the value of a column
		   const char *pszValue);
    bool SetColumn(int nCol,	// Set the value of a column
		   int nValue);
    void SetColumns(field * pField);	// Set the field definitions
    inline void SetRecordNumber(int nRecordNumber)	// Set the external database record number
    {
	m_nRecordNumber = nRecordNumber;
    };
    inline void SetStringValue(int nCol,	// Set the value of a column from a string
			       const char *pszValue)
    {

#ifdef DEBUG
	// Must exist within this row
	assert(nCol < (int) m_vField.size() && nCol >= 0);

#endif /*  */
	m_vField[nCol].SetStringValue(pszValue);
	m_bChanged = true;	// Always count as changed
    }
  private:bool m_bChanged;	// Has the data in this row been changed or not
    bool m_bDeleted;		// Has this row been deleted and is no longer active
    bool m_bNeedDelete;		// This row needs to be deleted
    bool m_bNew;		// Is this a new row
    int m_iFlags;		// The actual flags associated iwth the row 
    field *m_pField;

    int m_nRecordNumber;	// The record number in the external data base
    vector < NxDbColumn > m_vField;	// The fields in this row
    unsigned char *CreateRecord(string & strDbName,	// Build the binary record/row to go to the database
				NxDb * pNxDb);
};
typedef vector < NxDbRow * >ROW_VECTOR;

#endif /*  */
