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
// Class Used to represent a column in an NxDb database.        //
//--------------------------------------------------------------//
#ifndef NXDBCOLUMN_H_

#define NXDBCOLUMN_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
using namespace std;
class NxDbColumn
{
  public:enum DataType
    { DataTypeChar = 'c', DataTypeInt16 = 'i', DataTypeInt32 = 'l',
    };
      NxDbColumn(int nType,	// Constructor
		 int nMaxLength = 1);
    inline NxDbColumn(const NxDbColumn & Other)	// Copy constructor
    {
	m_Union.m_pszString = NULL;	// Avoid GPF
	*this = Other;
    }
     ~NxDbColumn();		// Destructor
    NxDbColumn & operator=(const NxDbColumn & Other);	// Assignment operator
    void Clear();		// Clear this column
    string Export();		// Export this data as a string
    int GetIntValue() const;	// Get the value of this column as an integer
    string GetStringValue() const;	// Get the value of this column as a string
    void Import(const string & strData);	// Import an escaped string
    void Output(unsigned char *ucPtr);	// Output the contents of this column
    void SetStringValue(const char *pszValue);	// Set the initial value of the field
    bool SetValue(const char *pszValue);	// Set to a string value
    bool SetValue(int nValue);	// Set to an integer value
    bool SetValue(const string & strValue);	// set to a string value
    void SetType(int nType,	// Reset the type of the column
		 int m_nMaxLength = 1);
  private:unsigned char m_ucType;
    // The type of field stored here
    unsigned int m_nMaxLength;	// Maximum array size (for strings includes the null terminator)
    union tagField		// Three types of fields allowed
    {
	int m_nInt;		// 32 bit integer - also contains 16 bit integers
	char *m_pszString;	// Character string (string class cannot be used in a union)
    }
    m_Union;
};


#endif /*  */
