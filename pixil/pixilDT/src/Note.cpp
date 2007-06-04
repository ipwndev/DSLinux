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
// A note saved from any of the four info types                 //
//--------------------------------------------------------------//
#include <cstdio>
#include <ctime>
#include <fstream>
#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include "Note.h"
#include "Options.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
// Define these for the MS version of access
#define R_OK 4
#define W_OK 2
#endif


//--------------------------------------------------------------//
// Construct from a file.                                       //
//--------------------------------------------------------------//
Note::Note(const char *pszFileName, int nMaxLength, const char *pszPrefix)
{
    ifstream fileIn;

    m_strFileName = pszFileName;
    if (pszPrefix != NULL) {
	m_strPrefix = pszPrefix;
    }
    m_nMaxLength = nMaxLength;

    if (m_strFileName.length() > 0) {
	// Read in the file contents
	fileIn.open(m_strFileName.c_str());
	if (fileIn.is_open()) {
	    char *pszContents;
	    unsigned int nLength;

	    // The file can be read, get all of it
	    fileIn.seekg(0, ios::end);
	    nLength = fileIn.tellg();
	    fileIn.seekg(0);
	    pszContents = new char[nLength + 1];

	    // Preclear for MS/Windows translated mode (less chars are read in)
	    memset(pszContents, 0, nLength + 1);

	    // Continue onward
	    fileIn.read(pszContents, nLength);
	    m_strText = pszContents;
	    delete[]pszContents;
	} else {
	    // Cannot open the file, act as if there was no file
	    m_strFileName = "";
	}
    }
    m_bChanged = false;
}


//--------------------------------------------------------------//
// Construct as empty.                                          //
//--------------------------------------------------------------//
Note::Note(int nMaxLength, const char *pszPrefix)
{
    if (pszPrefix != NULL) {
	m_strPrefix = pszPrefix;
    }
    m_nMaxLength = nMaxLength;
    m_bChanged = false;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
Note::~Note()
{
}


//--------------------------------------------------------------//
// Get a file name for this note.  This code is written to be   //
// portable between MS/Windows and Linux.  The functioning of   //
// the tempnam API is different between these two.  This is a   //
// re-write of the function that will perform the same on each. //
//--------------------------------------------------------------//
void
Note::CreateFileName()
{
#ifdef WIN32
    static const char cDirChar = '\\';
#else
    static const char cDirChar = '/';
#endif
    char *pszFileName;
    int nFileNo = 0;
    static int nIncrement = 0;	// Used to speed file name searching
    string strPath = Options::GetDatabasePath();

    // Get a place for the file name
    pszFileName =
	new char[strPath.length() + 1 + m_strPrefix.length() + 6 + 4 + 1];

    // Initialize increment if needed
    if (nIncrement == 0) {
	nIncrement = (time(NULL) & (1024 * 1024 - 1));
    }
    // Fix the increment for faster searching
    if (++nIncrement > 1024 * 1024) {
	nIncrement = 1;
    }
    // Search until an unused file name is found
    while (1) {
	nFileNo += nIncrement;
	if (nFileNo > 1024 * 1024) {
	    nFileNo = 1;
	    if (++nIncrement > 1024 * 1024) {
		nIncrement = 1;
	    }
	}
	// Create the file name
	sprintf(pszFileName, "%s%c%s%06x.not", strPath.c_str(), cDirChar,
		m_strPrefix.c_str(), nFileNo);

	if (access(pszFileName, R_OK | W_OK) == -1) {
	    // The file does not exist, found a good name
	    break;
	}
    }

    // Save the file name
    m_strFileName = pszFileName;

    // Clean up
    delete[]pszFileName;
}


//--------------------------------------------------------------//
// Delete this note - delete its file.                          //
//--------------------------------------------------------------//
void
Note::Delete()
{
    if (m_strFileName.length() > 0) {
	unlink(m_strFileName.c_str());
	m_strFileName = "";
	m_strText = "";
	m_bChanged = true;
    }
}


//--------------------------------------------------------------//
// Save this note to disk.                                      //
//--------------------------------------------------------------//
void
Note::Save()
{
    if (m_bChanged == true) {
	if (m_strText.length() == 0) {
	    // Delete the notes file if no notes present
	    if (m_strFileName.length() > 0) {
		Delete();
	    }
	} else {
	    ofstream fileOut;

	    // Get a file name if one is not present
	    if (m_strFileName.length() == 0) {
		CreateFileName();
	    }
	    // Open the file for output
	    fileOut.open(m_strFileName.c_str());
	    if (!fileOut.is_open()) {
		CreateFileName();
		fileOut.open(m_strFileName.c_str());
	    }
	    // The file has to be open now or else
	    if (fileOut.is_open()) {
		fileOut.write(m_strText.c_str(), m_strText.length());
		fileOut.close();
	    }
	}
	m_bChanged = false;
    }
}


//--------------------------------------------------------------//
// Change the text of this note.                                //
//--------------------------------------------------------------//
void
Note::SetText(const char *pszText)
{
    if (pszText == NULL) {
	if (m_strText != "") {
	    m_strText = "";
	    m_bChanged = true;
	}
    } else {
	if (m_strText != pszText) {
	    m_strText = pszText;
	    m_bChanged = true;
	}
    }
}
