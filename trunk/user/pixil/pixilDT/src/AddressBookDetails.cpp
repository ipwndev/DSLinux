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
// Class for the Address Book Details.                          //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <FL/fl_draw.H>
#include "AddressBookDB.h"
#include "AddressBookDetails.h"
#include "Note.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
AddressBookDetails::AddressBookDetails(int nX,
				       int nY, int nWidth, int nHeight)
    :
Fl_Browser(nX, nY, nWidth, nHeight)
{
    color(FL_GRAY);
    m_nRow = -1;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
AddressBookDetails::~AddressBookDetails()
{
}


//--------------------------------------------------------------//
// Display a row from the address book.                         //
//--------------------------------------------------------------//
void
AddressBookDetails::DisplayRow(int nRow)
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    char szLine[512];
    int i;
    string strData;
    string strData2;
    string strData3;

    // Erase the current display
    clear();

    // Only redisplay if the row number has been set
    if (nRow >= 0) {
	// Set the current row for refreshes
	m_nRow = nRow;

	// First line is the category
	sprintf(szLine, "@r@.%s",
		pAddressBookDB->GetCategoryName(nRow).c_str());
	add(szLine);

	// Second line is the name
	sprintf(szLine,
		"@m@.%s %s",
		pAddressBookDB->GetFirstName(nRow).c_str(),
		pAddressBookDB->GetLastName(nRow).c_str());
	add(szLine);

	// Next line is the title if present
	strData = pAddressBookDB->GetTitle(nRow);
	if (strData.length() > 0) {
	    sprintf(szLine, "@.%s", strData.c_str());
	    add(szLine);
	}
	// Next line is the company if present
	strData = pAddressBookDB->GetCompany(nRow);
	if (strData.length() > 0) {
	    sprintf(szLine, "@.%s", strData.c_str());
	    add(szLine);
	}
	// Next line is a blank line
	add("");		// Doc is incorrect, this cannot be NULL

	// Next 1 through 7 lines are the Dep's
	for (i = 0; i < 7; ++i) {
	    strData = pAddressBookDB->GetInfo(nRow, i);
	    if (strData.length() > 0) {
		sprintf(szLine,
			"@.%s: %s",
			pAddressBookDB->GetInfoName(nRow, i).c_str(),
			strData.c_str());
		add(szLine);
	    }
	}

	// Next line is a blank line
	add("");		// Doc is incorrect, this cannot be NULL

	// Next line is the address line
	strData = pAddressBookDB->GetAddress(nRow);
	if (strData.length() > 0) {
	    sprintf(szLine, "@.%s", strData.c_str());
	    add(szLine);
	}
	// Next line is the address line
	strData = pAddressBookDB->GetCity(nRow);
	strData2 = pAddressBookDB->GetRegion(nRow);
	strData3 = pAddressBookDB->GetPostalCode(nRow);
	if (strData.length() > 0
	    || strData2.length() > 0 || strData3.length() > 0) {
	    if (strData.length() > 0) {
		strcpy(szLine, strData.c_str());
	    }
	    if (strData2.length() > 0) {
		if (szLine[0] != '\0') {
		    strcat(szLine, ", ");
		}
		strcpy(szLine, strData2.c_str());
	    }
	    if (strData3.length() > 0) {
		if (szLine[0] != '\0') {
		    strcat(szLine, " ");
		}
		strcpy(szLine, strData.c_str());
	    }
	    add(szLine);
	}
	// Next line is the country line
	strData = pAddressBookDB->GetCountry(nRow);
	if (strData.length() > 0) {
	    sprintf(szLine, "@.%s", strData.c_str());
	    add(szLine);
	}
	// Next line is a blank line
	add("");		// Doc is incorrect, this cannot be NULL

	// Next 1 through 4 lines are the Dep's
	for (i = 0; i < 4; ++i) {
	    strData = pAddressBookDB->GetCustomField(nRow, i);
	    if (strData.length() > 0) {
		sprintf(szLine,
			"@.%s: %s",
			pAddressBookDB->GetCustomFieldName(i).c_str(),
			strData.c_str());
		add(szLine);
	    }
	}

	// Next line is a blank line
	add("");		// Doc is incorrect, this cannot be NULL

	// Next line is the Notes line(s)
	strData = pAddressBookDB->GetNoteFile(nRow);
	if (strData.length() > 0) {
	    sprintf(szLine, "@.%s", _("Notes:"));
	    add(szLine);

	    Note *pNote = pAddressBookDB->GetNote(nRow);
	    WrapText(pNote->GetText().c_str());
	    delete pNote;
	}
    } else {
	// No row selected, clear the details
	clear();
    }
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
AddressBookDetails::Message(PixilDTMessage nMessage, int nInfo)
{
    AddressBookDB *pAddressBookDB;
    int nReturn = 0;

    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:
	{
	    pAddressBookDB = AddressBookDB::GetAddressBookDB();
	    DisplayRow(m_nRow < pAddressBookDB->NumRecs()? m_nRow : -1);
	}
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Break note text into multiple lines for display.             //
//--------------------------------------------------------------//
void
AddressBookDetails::WrapText(const char *pszText)
{
    char *pszBufEnd;
    char *pszBuffer = NULL;
    char *pszBuffer2;
    char *pszBufStart;
    const char *pszData = pszText;
    const char *pszEnd;
    const char *pszEnd2;
    char *pszOldBufEnd;
    int i;
    int nChr;
    int nFlHeight = fl_height() + fl_descent();
    int nHeight;
    int nLength;
    int nMax;
    int nMaxBuffer = 0;
    int nMaxWidth = w() - 20;
    int nWidth;

    // Initialize the font for measurements
    fl_font(labelfont(), labelsize());

    while (*pszData != '\0') {
	// Get the next new-line delimited block of characters
	pszEnd = strchr(pszData, '\n');
	if (pszEnd == NULL) {
	    pszEnd = pszData + strlen(pszData);
	}
	pszEnd2 = strchr(pszData, '\r');
	if (pszEnd2 == NULL) {
	    pszEnd2 = pszData + strlen(pszData);
	}
	if (pszEnd2 < pszEnd) {
	    pszEnd = pszEnd2;
	}
	// Will this line fit into the browser
	nLength = pszEnd - pszData + 1;
	if (nMaxBuffer < nLength) {
	    delete[]pszBuffer;
	    pszBuffer = new char[nLength];
	    nMaxBuffer = nLength;
	}
	strncpy(pszBuffer, pszData, nLength - 1);
	pszBuffer[nLength - 1] = '\0';
	pszBufStart = pszBuffer;
	while (*pszBufStart == ' ') {
	    ++pszBufStart;
	}
	nWidth = nMaxWidth;
	fl_measure(pszBufStart, nWidth, nHeight);

	// Break out smaller portions of the line
	while (nHeight > nFlHeight || nWidth > nMaxWidth) {
	    // Find a small enough portion of this line to fit
	    pszBufEnd = pszBufStart;
	    do {
		pszOldBufEnd = pszBufEnd;
		pszBufEnd = strchr(pszBufEnd + 1, ' ');
		if (pszBufEnd == NULL) {
		    // Too long a word at the end of the line
		    break;
		}
		*pszBufEnd = '\0';
		nWidth = nMaxWidth;
		fl_measure(pszBufStart, nWidth, nHeight);
		*pszBufEnd = ' ';
	    } while (nHeight <= nFlHeight && nWidth <= nMaxWidth);

	    // Output the first portion of the remaining line
	    if (pszOldBufEnd == pszBufStart) {
		// Word too long, just output characters
		nMax = strlen(pszBufStart) - 1;
		nWidth = nMaxWidth + 1;
		for (i = nMax;
		     i > 1 && (nHeight > nFlHeight || nWidth > nMaxWidth);
		     --i) {
		    nChr = pszBufStart[i];
		    pszBufStart[i] = '\0';
		    nWidth = nMaxWidth;
		    fl_measure(pszBufStart, nWidth, nHeight);
		    pszBufStart[i] = nChr;
		}

		// Get these characters and add as a line
		pszBuffer2 = new char[i + 1];
		strncpy(pszBuffer2, pszBufStart, i);
		pszBuffer2[i] = '\0';
		add(pszBuffer2);
		delete[]pszBuffer2;
		pszBufStart += i;
	    } else {
		// Can break at white space (blank)
		*pszOldBufEnd = '\0';
		add(pszBufStart);
		pszBufStart = pszOldBufEnd + 1;
	    }
	    while (*pszBufStart == ' ') {
		++pszBufStart;
	    }

	    // Get the size of what's left
	    fl_measure(pszBufStart, nWidth, nHeight);
	}

	// Add the remainder of the line
	add(pszBufStart);

	// Update the next start of data
	pszData = pszEnd;
	if (*pszData == '\r' && pszData[1] == '\n') {
	    pszData += 2;
	} else if (*pszData != '\0') {
	    ++pszData;
	}
    }

    // Clean up
    delete[]pszBuffer;
}
