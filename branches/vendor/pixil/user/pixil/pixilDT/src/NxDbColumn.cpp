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
#include <cassert>
#include <climits>
#include <cstdio>
#include <FL/fl_ask.H>
#include <FL/x.H>
extern "C"
{
#include <file.h>
}
#include "NxDbColumn.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
NxDbColumn::NxDbColumn(int nType, int nMaxLength)
{
    m_ucType = nType;
    m_nMaxLength = nMaxLength;
    switch (m_ucType) {
    case DataTypeChar:		// String
	m_Union.m_pszString = NULL;
	break;

    default:			// Integer types
	m_Union.m_nInt = 0;
    }
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
NxDbColumn::~NxDbColumn()
{
    if (m_ucType == DataTypeChar) {
	delete[]m_Union.m_pszString;
    }
}


//--------------------------------------------------------------//
// Assignment operator.                                         //
//--------------------------------------------------------------//
NxDbColumn & NxDbColumn::operator=(const NxDbColumn & Other)
{
    // Copy only if needed
    if (this != &Other) {
	// Free up the old character string if needed
	if (m_ucType == DataTypeChar) {
	    delete[]m_Union.m_pszString;
	}
	// Copy to here
	m_ucType = Other.m_ucType;
	m_nMaxLength = Other.m_nMaxLength;
	switch (m_ucType) {
	case DataTypeChar:	// String
	    if (Other.m_Union.m_pszString != NULL) {
		m_Union.m_pszString =
		    new char[strlen(Other.m_Union.m_pszString) + 1];
		strcpy(m_Union.m_pszString, Other.m_Union.m_pszString);
	    } else {
		m_Union.m_pszString = NULL;
	    }
	    break;

	default:		// Integer types
	    m_Union.m_nInt = Other.m_Union.m_nInt;
	}
    }
    return (*this);
}


//--------------------------------------------------------------//
// Clear this column                                            //
//--------------------------------------------------------------//
void
NxDbColumn::Clear()
{
    switch (m_ucType) {
    case DataTypeChar:
	delete[]m_Union.m_pszString;
	m_Union.m_pszString = NULL;
	break;

    case DataTypeInt16:
    case DataTypeInt32:
	m_Union.m_nInt = 0;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown column type
#endif
	;
    }
}


//--------------------------------------------------------------//
// Export this column as a string.                              //
//--------------------------------------------------------------//
string
NxDbColumn::Export()
{
    char szData[16];
    string strReturn;

    switch (m_ucType) {
    case DataTypeChar:
	strReturn = m_Union.m_pszString;
	break;

    case DataTypeInt16:
    case DataTypeInt32:
	sprintf(szData, "%d", m_Union.m_nInt);
	strReturn = szData;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Get the value of this column as an integer                   //
//--------------------------------------------------------------//
int
NxDbColumn::GetIntValue() const
{
    int nReturn;

    switch (m_ucType) {
    case DataTypeChar:		// Character string
#ifdef DEBUG
	assert(false);		// cannot get the integer value of a string
#endif
	nReturn = 0;
	break;

    case DataTypeInt16:
    case DataTypeInt32:
	nReturn = m_Union.m_nInt;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown data type
#endif
	;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Get the value of this column as a string.                    //
//--------------------------------------------------------------//
string
NxDbColumn::GetStringValue() const
{
    string strReturn;

    switch (m_ucType) {
    case DataTypeChar:		// Character string
	if (m_Union.m_pszString != NULL) {
	    strReturn = m_Union.m_pszString;
	}
	break;

    case DataTypeInt16:
    case DataTypeInt32:
	char szString[16];

	sprintf(szString, "%d", m_Union.m_nInt);
	strReturn = szString;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown data type
#endif
	;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Import an escaped string.                                    //
//--------------------------------------------------------------//
void
NxDbColumn::Import(const string & strData)
{
    int nChar;
    string strData2;
    unsigned int nOffset;

    switch (m_ucType) {
    case DataTypeChar:
	// Unescape the string
	if (strData[0] == '\"' && strData[strData.length() - 1] == '\"') {
	    nOffset = 1;
	    while (nOffset < strData.length() - 2) {
		if (strData[nOffset] == '\\') {
		    // Escape sequence
#ifdef DEBUG
		    assert(nOffset < strData.length() - 3);	// Single backslash at end of string
#endif
		    switch (strData[nOffset + 1]) {
		    case 'a':
			strData2 += '\a';
			break;

		    case 'b':
			strData2 += '\b';
			break;

		    case 'f':
			strData2 += '\f';
			break;

		    case 'n':
			strData2 += '\n';
			break;

		    case 'r':
			strData2 += '\r';
			break;

		    case 't':
			strData2 += '\t';
			break;

		    case 'v':
			strData2 += '\v';
			break;

		    case '\'':
			strData2 += '\'';
			break;

		    case '\"':
			strData2 += '\"';
			break;

		    case '\\':
			strData2 += '\\';
			break;

		    case 'x':
#ifdef DEBUG
			assert(nOffset < strData.length() - 5);	// Not enough room for hex escape
#endif
			if (strData[nOffset + 2] >= '0'
			    && strData[nOffset + 2] <= '9') {
			    nChar = 16 * (strData[nOffset + 2] - '0');
			} else if (strData[nOffset + 2] >= 'a'
				   && strData[nOffset + 2] <= 'f') {
			    nChar = 16 * (strData[nOffset + 2] - 'a' + 10);
			} else if (strData[nOffset + 2] >= 'A'
				   && strData[nOffset + 2] <= 'F') {
			    nChar = 16 * (strData[nOffset + 2] - 'A' + 10);
			}
#ifdef DEBUG
			else {
			    assert(false);	// Invalid hex digit
			}
#endif

			if (strData[nOffset + 3] >= '0'
			    && strData[nOffset + 3] <= '9') {
			    nChar += strData[nOffset + 3] - '0';
			} else if (strData[nOffset + 3] >= 'a'
				   && strData[nOffset + 3] <= 'f') {
			    nChar += strData[nOffset + 3] - 'a' + 10;
			} else if (strData[nOffset + 3] >= 'A'
				   && strData[nOffset + 3] <= 'F') {
			    nChar += strData[nOffset + 3] - 'A' + 10;
			}
#ifdef DEBUG
			else {
			    assert(false);	// Invalid hex digit
			}
#endif

			strData2 += nChar;
			nOffset += 2;	// Fix for 4 digit excape
			break;
		    }

		    // Fix for the escape length
		    nOffset += 1;
		} else {
		    // Normal character
		    strData2 += strData[nOffset];
		}

		// Go to the next character
		nOffset += 1;
	    }
	}
#ifdef DEBUG
	else {
	    assert(false);	// Incorrect string on import
	}
#endif

	// Move this string into the column
	SetValue(strData2.c_str());
	break;

    case DataTypeInt16:
    case DataTypeInt32:
	// Move the numeric value into the column
	SetStringValue(strData.c_str());
    }
}


//--------------------------------------------------------------//
// Output this column                                           //
//--------------------------------------------------------------//
void
NxDbColumn::Output(unsigned char *ucPtr)
{
    switch (m_ucType) {
    case DataTypeChar:		// Character string
	if (m_Union.m_pszString != NULL) {
	    strcpy((char *) ucPtr, m_Union.m_pszString);
	} else {
	    *ucPtr = '\0';
	}
	break;

    case DataTypeInt16:
	put16((char *) ucPtr, m_Union.m_nInt);
	break;

    case DataTypeInt32:
	put32((char *) ucPtr, m_Union.m_nInt);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown data type
#endif
	;
    }
}


//--------------------------------------------------------------//
// Set the initial value from a string.                         //
//--------------------------------------------------------------//
void
NxDbColumn::SetStringValue(const char *pszValue)
{
    switch (m_ucType) {
    case DataTypeChar:		// character string
#ifdef DEBUG
	// Cannot exceed the maximum length
	assert(strlen(pszValue) <= m_nMaxLength);
#endif
	// Free the old value
	delete[]m_Union.m_pszString;

	// Copy the new one
	if (pszValue != NULL) {
	    m_Union.m_pszString = new char[strlen(pszValue) + 1];
	    strcpy(m_Union.m_pszString, pszValue);
	} else {
	    m_Union.m_pszString = NULL;
	}
	break;

    case DataTypeInt16:	// Short integer
	{
	    int nValue;

	    // Fix the value for discrepencies between signed and unsigned shorts
	    nValue = atoi(pszValue);	// No bad character checking
	    if (nValue > 0x7fff && nValue < 0x10000) {
		nValue -= 0x10000;
	    }
#ifdef DEBUG
	    assert(nValue >= SHRT_MIN && nValue <= SHRT_MAX);
#endif

	    m_Union.m_nInt = nValue;
	}
	break;

    case DataTypeInt32:	// Long integer
	m_Union.m_nInt = atoi(pszValue);	// No bad character checking
	break;

    default:			// Unknown type
#ifdef DEBUG
	assert(false);		// Unknown data type
#endif
	;
    }
}


//--------------------------------------------------------------//
// Reset the type of the column                                 //
//--------------------------------------------------------------//
void
NxDbColumn::SetType(int nType, int nMaxLength)
{
    // Free the old string if needed
    if (m_ucType == DataTypeChar) {
	delete[]m_Union.m_pszString;
    }

    m_ucType = nType;
    m_nMaxLength = nMaxLength;
    switch (m_ucType) {
    case DataTypeChar:		// String
	m_Union.m_pszString = NULL;
	break;

    default:			// Integer types
	m_Union.m_nInt = 0;
    }
}


//--------------------------------------------------------------//
// Set to a string value.                                       //
//--------------------------------------------------------------//
bool
NxDbColumn::SetValue(const char *pszValue)
{
    bool bReturn;

    /* FIXME:  Major hack alert - this just gets around idiocricy */

    if (m_ucType == 'i' || m_ucType == 'l') {
	int val = atoi(pszValue);
	return SetValue(val);
    }
    // Are the old and new strings equal

    if (m_Union.m_pszString == NULL) {
	bReturn = (pszValue != NULL);
    } else {
	if (pszValue != NULL) {
	    bReturn = (strcmp(m_Union.m_pszString, pszValue) != 0);
	} else {
	    bReturn = true;
	}
    }

    // Change if the values are different
    if (bReturn == true) {
	// Free the old value
	delete[]m_Union.m_pszString;

	// Copy the new one
	if (pszValue != NULL) {
	    m_Union.m_pszString = new char[strlen(pszValue) + 1];
	    strcpy(m_Union.m_pszString, pszValue);
	} else {
	    m_Union.m_pszString = NULL;
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Set to an integer value                                      //
//--------------------------------------------------------------//
bool
NxDbColumn::SetValue(int nValue)
{
    bool bReturn;


#ifdef DEBUG
    // This must be an integer column
    assert(m_ucType == 'i' || m_ucType == 'l');

    // This must be in range for a 16-bit integer
    if (m_ucType == 'i') {
	assert(nValue >= SHRT_MIN && nValue <= SHRT_MAX);
    }
#endif

    bReturn = (m_Union.m_nInt != nValue);
    m_Union.m_nInt = nValue;
    return (bReturn);
}


//--------------------------------------------------------------//
// Set to a string value                                        //
//--------------------------------------------------------------//
bool
NxDbColumn::SetValue(const string & strValue)
{
    bool bReturn;

#ifdef DEBUG
    // This must be a character column
    assert(m_ucType == 'c');
    assert(strValue.length() < m_nMaxLength);
#endif

    // Are the old and new strings different
    if (m_Union.m_pszString == NULL) {
	bReturn = true;
    } else {
	bReturn = (strValue != m_Union.m_pszString);
    }

    // Change the string if needed
    if (bReturn == true) {
	// Free the old value
	delete[]m_Union.m_pszString;

	// Copy the new one
	m_Union.m_pszString = new char[strValue.length() + 1];
	strcpy(m_Union.m_pszString, strValue.c_str());
    }
    return (bReturn);
}
