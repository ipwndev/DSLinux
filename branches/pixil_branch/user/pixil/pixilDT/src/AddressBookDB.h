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
// Address Book database class.                                 //
//--------------------------------------------------------------//
#ifndef ADDRESSBOOKDB_H_

#define ADDRESSBOOKDB_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <vector>
#include "InfoDB.h"
#include "Note.h"
#include "NxDbAccess.h"
#include <unistd.h>
using namespace std;

// Field References
#define AB_RECNO       0
#define AB_CAT         1
#define AB_SHOW        2
#define AB_LASTNAME    3
#define AB_FIRSTNAME   4
#define AB_COMPANY     5
#define AB_TITLE       6
#define AB_DEP1ID      7
#define AB_DEP2ID      8
#define AB_DEP3ID      9
#define AB_DEP4ID     10
#define AB_DEP5ID     11
#define AB_DEP6ID     12
#define AB_DEP7ID     13
#define AB_DEP1       14
#define AB_DEP2       15
#define AB_DEP3       16
#define AB_DEP4       17
#define AB_DEP5       18
#define AB_DEP6       19
#define AB_DEP7       20
#define AB_ADDRESS    21
#define AB_CITY       22
#define AB_REGION     23	// State, Province, Town, etc.
#define AB_POSTALCODE 24	// ZIP, Postal Code, etc.
#define AB_COUNTRY    25
#define AB_BDAY       26
#define AB_ANNIV      27
#define AB_CUST1      28
#define AB_CUST2      29
#define AB_CUST3      30
#define AB_CUST4      31
#define AB_NOTE       32
#define AB_FIELDS     33

#define AB_ID         4
#define AB_TEXT      50
#define AB_DBL_TEXT 100
#define AB_DATE      20
#define AB_NOTEDB   100

#define ADDRESS_BOOK_NOTES_PREFIX "add_"
class AddressBookDB:public NxDbAccess
{
  public:AddressBookDB();	// Constructor
    ~AddressBookDB();		// Destructor
    void Delete(int nRow);	// Delete the note for this item as well
    void Export(int nRow,	// Export to a set of strings
		vector < string > &vExportString);
    int FindRecno(int nRecno);	// Workaround for Recno being a character string (!?)
    static AddressBookDB *GetAddressBookDB();	// Retrieve the static pointer
    inline string GetAddress(int nRow) const	// Get the address
    {
	return (GetStringValue(nRow, AB_ADDRESS));
    }
    inline string GetAnniversary(int nRow) const	// Get the anniversary
    {
	return (GetStringValue(nRow, AB_ANNIV));
    }
    inline string GetBirthday(int nRow) const	// Get the birthday
    {
	return (GetStringValue(nRow, AB_BDAY));
    }
    inline int GetCategory(int nRow) const	// Get the category id
    {
	return (atoi(GetStringValue(nRow, AB_CAT).c_str()));
    }
    string GetCategoryName(int nRow) const;	// Get the name of the category
    inline string GetCity(int nRow) const	// Get the city
    {
	return (GetStringValue(nRow, AB_CITY));
    }
    inline string GetCompany(int nRow) const	// Get the company name
    {
	return (GetStringValue(nRow, AB_COMPANY));
    }
    inline string GetCountry(int nRow) const	// Get the country
    {
	return (GetStringValue(nRow, AB_COUNTRY));
    }
    inline string GetCustomField(int nRow,	// Get a custom field
				 int nIndex) const
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 4);

#endif				/*  */
	return (GetStringValue(nRow, AB_CUST1 + nIndex));
    }
    string GetCustomFieldName(int nIndex) const;	// Get one of the custom field names
    inline string GetFirstName(int nRow) const	// Get the first name
    {
	return (GetStringValue(nRow, AB_FIRSTNAME));
    }
    inline string GetInfo(int nRow,	// Get one of the seven Info fields
			  int nIndex) const
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 7);

#endif				/*  */
	return (GetStringValue(nRow, AB_DEP1 + nIndex));
    }
    inline int GetInfoID(int nRow,	// Get one of the seven Info fields
			 int nIndex) const
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 7);

#endif				/*  */
	return (GetIntValue(nRow, AB_DEP1ID + nIndex));
    }
    static string GetInfoName(int nIndex);	// Get one of the Info field names
    inline static int GetInfoNameCount()	// Get the number of info names
    {
	return (InfoDB::GetInfo()->NumUndeletedRecs());
    }
    string GetInfoName(int nRow,	// Get one of the seven Info field names
		       int nIndex) const;
    inline string GetLastName(int nRow) const	// Get the last name
    {
	return (GetStringValue(nRow, AB_LASTNAME));
    }
    Note *GetNewNote() const	// Get a note for a new row
    {
	return (new Note(0, ADDRESS_BOOK_NOTES_PREFIX));
    }
    Note *GetNote(int nRow) const;	// Get the notes for this row
    inline string GetNoteFile(int nRow) const	// Get the note file name
    {
	return (GetStringValue(nRow, AB_NOTE));
    }
    inline string GetPostalCode(int nRow) const	// Get the postal code
    {
	return (GetStringValue(nRow, AB_POSTALCODE));
    }
    inline int GetRecno(int nRow) const	// Get the recno column
    {
	return (atoi(GetStringValue(nRow, AB_RECNO).c_str()));
    }
    inline string GetRegion(int nRow) const	// Get the region
    {
	return (GetStringValue(nRow, AB_REGION));
    }
    inline int GetShowFlag(int nRow) const	// Get the show flag
    {
	return (GetIntValue(nRow, AB_SHOW));
    }
    inline string GetTitle(int nRow) const	// Get the title
    {
	return (GetStringValue(nRow, AB_TITLE));
    }
    int Import(const vector < string > &vExportString);	// Import a delimited string
    int Insert();		// Insert a row and set its key value
    int NumRecsByCategory(int nCategory);	// Get the number of records for a category
    inline void SetAddress(int nRow, const char *pszAddress)	// Set the address
    {
	SetColumn(nRow, AB_ADDRESS, pszAddress);
    }
    inline void SetAnniversary(int nRow, const char *pszAnniversary)	// Set the anniversary date
    {
	SetColumn(nRow, AB_ANNIV, pszAnniversary);
    }
    inline void SetBirthday(int nRow, const char *pszBirthday)	// Set the birthday
    {
	SetColumn(nRow, AB_BDAY, pszBirthday);
    }
    void SetCategory(int nRow, int nCategory)	// Set the Category
    {

	// Overcome database anomaly
	//              sprintf(szCategory,"%d",nCategory);
	//              SetColumn(nRow,AB_CAT,szCategory);
	SetColumn(nRow, AB_CAT, nCategory);
    }
    inline void SetCity(int nRow, const char *pszCity)	// Set the city
    {
	SetColumn(nRow, AB_CITY, pszCity);
    }
    inline void SetCompany(int nRow, const char *pszCompany)	// Set the company name
    {
	SetColumn(nRow, AB_COMPANY, pszCompany);
    }
    inline void SetCountry(int nRow, const char *pszCountry)	// Set the country
    {
	SetColumn(nRow, AB_COUNTRY, pszCountry);
    }
    inline void SetCustom(int nRow,	// Set one of the custom fields
			  int nIndex, const char *pszCustom)
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 4);

#endif /*  */
	SetColumn(nRow, AB_CUST1 + nIndex, pszCustom);
    }
    inline void SetCustom1(int nRow, const char *pszCustom)	// Set custom field 1
    {
	SetColumn(nRow, AB_CUST1, pszCustom);
    }
    inline void SetCustom2(int nRow, const char *pszCustom)	// Set custom field 2
    {
	SetColumn(nRow, AB_CUST2, pszCustom);
    }
    inline void SetCustom3(int nRow, const char *pszCustom)	// Set custom field 3
    {
	SetColumn(nRow, AB_CUST3, pszCustom);
    }
    inline void SetCustom4(int nRow, const char *pszCustom)	// Set custom field 4
    {
	SetColumn(nRow, AB_CUST4, pszCustom);
    }
    inline void SetInfo(int nRow,	// Set one of the seven Info fields
			int nIndex, const char *pszInfo)
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 7);

#endif /*  */
	SetColumn(nRow, AB_DEP1 + nIndex, pszInfo);
    }
    inline void SetInfo1(int nRow, const char *pszInfo)	// Set the Info 1
    {
	SetColumn(nRow, AB_DEP1, pszInfo);
    }
    inline void SetInfo1ID(int nRow, int nInfoID)	// Set the Info 1 ID
    {
	SetColumn(nRow, AB_DEP1ID, nInfoID);
    }
    inline void SetInfo2(int nRow, const char *pszInfo)	// Set the Info 2
    {
	SetColumn(nRow, AB_DEP2, pszInfo);
    }
    inline void SetInfo2ID(int nRow, int nInfoID)	// Set the Info 2 ID
    {
	SetColumn(nRow, AB_DEP2ID, nInfoID);
    }
    inline void SetInfo3(int nRow, const char *pszInfo)	// Set the Info 3
    {
	SetColumn(nRow, AB_DEP3, pszInfo);
    }
    inline void SetInfo3ID(int nRow, int nInfoID)	// Set the Info 3 ID
    {
	SetColumn(nRow, AB_DEP3ID, nInfoID);
    }
    inline void SetInfo4(int nRow, const char *pszInfo)	// Set the Info 4
    {
	SetColumn(nRow, AB_DEP4, pszInfo);
    }
    inline void SetInfo4ID(int nRow, int nInfoID)	// Set the Info 4 ID
    {
	SetColumn(nRow, AB_DEP4ID, nInfoID);
    }
    inline void SetInfo5(int nRow, const char *pszInfo)	// Set the Info 5
    {
	SetColumn(nRow, AB_DEP5, pszInfo);
    }
    inline void SetInfo5ID(int nRow, int nInfoID)	// Set the Info 5 ID
    {
	SetColumn(nRow, AB_DEP5ID, nInfoID);
    }
    inline void SetInfo6(int nRow, const char *pszInfo)	// Set the Info 6
    {
	SetColumn(nRow, AB_DEP6, pszInfo);
    }
    inline void SetInfo6ID(int nRow, int nInfoID)	// Set the Info 6 ID
    {
	SetColumn(nRow, AB_DEP6ID, nInfoID);
    }
    inline void SetInfo7(int nRow, const char *pszInfo)	// Set the Info 7
    {
	SetColumn(nRow, AB_DEP7, pszInfo);
    }
    inline void SetInfo7ID(int nRow, int nInfoID)	// Set the Info 7 ID
    {
	SetColumn(nRow, AB_DEP7ID, nInfoID);
    }
    inline void SetInfoID(int nRow,	// Set any of the Info ID's
			  int nIndex, int nInfoID)
    {

#ifdef DEBUG
	assert(nIndex >= 0 && nIndex < 7);

#endif /*  */
	SetColumn(nRow, AB_DEP1ID + nIndex, nInfoID);
    }
    inline void SetFirstName(int nRow, const char *pszFirstName)	// Set the first name
    {
	SetColumn(nRow, AB_FIRSTNAME, pszFirstName);
    }
    inline void SetLastName(int nRow, const char *pszLastName)	// Set the last name
    {
	SetColumn(nRow, AB_LASTNAME, pszLastName);
    }
    inline void SetNoteFile(int nRow, const char *pszNoteFile)	// Set the note file name
    {
	SetColumn(nRow, AB_NOTE, pszNoteFile);
    }
    void SetNote(int nRow,	// Set the note
		 Note * pNote);
    inline void SetPostalCode(int nRow, const char *pszPostalCode)	// Set the postal code
    {
	SetColumn(nRow, AB_POSTALCODE, pszPostalCode);
    }
    inline void SetRegion(int nRow, const char *pszRegion)	// Set the region
    {
	SetColumn(nRow, AB_REGION, pszRegion);
    }
    inline void SetShowFlag(int nRow, int nShow)	// Set the Show Flag
    {
	SetColumn(nRow, AB_SHOW, nShow);
    }
    inline void SetTitle(int nRow, const char *pszTitle)	// Set the Title
    {
	SetColumn(nRow, AB_TITLE, pszTitle);
    }
  private:static AddressBookDB *m_pThis;
    // One and only address book object
};


#endif /*  */
