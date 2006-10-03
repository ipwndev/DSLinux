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
// Global application options.                                  //
//--------------------------------------------------------------//
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <FL/fl_ask.H>
#include <FL/x.H>
#include "HelpID.h"
#include "IniDlg.h"
#include "Options.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
#define strcasecmp stricmp	// MS Visual C renames this
#endif


using namespace std;


#ifndef WIN32_PROFILE_API
// Linux/Unix based INI file classes


//--------------------------------------------------------------//
// Class for individual INI file categories.                    //
//--------------------------------------------------------------//
class INICategory
{
  public:

    INICategory(const string & strName);	// Constructor

    inline INICategory(const INICategory & Other)	// Copy constructor
    {
	*this = Other;
    }

    INICategory & operator=(const INICategory & Other);	// Assignment operator

    bool operator==(const INICategory & Other) const;	// Comparison operator

    bool operator==(const string & strName) const;	// Comparison operator

    bool operator<(const INICategory & Other) const;	// Comparison operator

    inline const string & GetName() const	// Get the category name
    {
	return (m_strName);
    }

    string GetValue(const string & strName,	// Get the value of a setting
		    const string & strDefault);

    void SetOption(const string & strLine);	// Set a setting from "name=value"

    bool SetOption(const string & strName,	// Set a setting
		   const string & strValue);

    void Write(ostream & fileOut);	// Write this category to a stream


  private:

    map < string, string > m_mSetting;	// Settings for this category

    string m_strName;		// Name of this category

    map < string, string >::iterator FindSetting(const string & strValue);	// Find a setting

};


//--------------------------------------------------------------//
// Class to read and process an INI file.                       //
//--------------------------------------------------------------//
class INIFileReader
{
  public:

    INIFileReader(const string & strFileName);	// Construct from a file

      inline ~ INIFileReader()	// Destructor
    {
	Write();
    }

    string GetOption(const string & strCategory,	// Get an option from the file
		     const string & strName, const string & strDefault);

    void SetOption(const string & strCategory,	// Set an option in the file
		   const string & strName, const string & strValue);

    void Write();		// Write the INI file back to the file it was read from

    void Write(const string & strFileName);	// Write the INI file back to disk


  private:

    bool m_bModifiedFlag;	// Modified flag

    string m_strFileName;	// Name of the INI file

    vector < INICategory > m_vCategory;	// Categories in this INI file

    INICategory *FindCategory(const string & strCategory,	// Find or create a category
			      bool bCreate);

};


#endif


#ifdef WIN32
#define strcasecmp stricmp
#endif


//--------------------------------------------------------------//
// Singleton pointer.                                           //
//--------------------------------------------------------------//
Options *
    Options::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
Options::Options()
{
    m_pThis = this;

#ifndef WIN32_PROFILE_API	// Linux/Unix only code

    // Read in the INI file
    SetIniFileName();
    m_pINIFileReader = new INIFileReader(m_strIniFile);

#endif
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
Options::~Options()
{
#ifndef WIN32_PROFILE_API

    // Write out the INI file
    delete((INIFileReader *) m_pINIFileReader);

#endif
}


//--------------------------------------------------------------//
// Destroy the Options object.                                  //
//--------------------------------------------------------------//
void
Options::Destroy()
{
    delete m_pThis;
    m_pThis = NULL;
}


//--------------------------------------------------------------//
// Get a boolean value from the INI file                        //
//--------------------------------------------------------------//
bool
Options::GetBooleanValue(const char *pszSection,
			 const char *pszSetting, bool bDefault)
{
    bool bReturn;
    string strValue;

#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    strValue =
	((INIFileReader *) (m_pThis->m_pINIFileReader))->GetOption(pszSection,
								   pszSetting,
								   "false");

#else

    char szValue[32];

    // Get the INI file name
    SetIniFileName();

    // Windows variation, get from the INI file
    ::GetPrivateProfileString(pszSection,
			      pszSetting,
			      "*",
			      szValue,
			      sizeof(szValue), m_pThis->m_strIniFile.c_str());
    strValue = szValue;

#endif

    if (strcasecmp(strValue.c_str(), "yes") == 0
	|| strcasecmp(strValue.c_str(), "true") == 0) {
	bReturn = true;
    } else if (strcmp(strValue.c_str(), "*") == 0) {
	bReturn = bDefault;
    } else {
	bReturn = false;
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Get an integer value from the INI file                       //
//--------------------------------------------------------------//
int
Options::GetIntValue(const char *pszSection, const char *pszSetting)
{
    int nReturn;
    string strValue;

#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    strValue =
	((INIFileReader *) (m_pThis->m_pINIFileReader))->GetOption(pszSection,
								   pszSetting,
								   "0");

#else

    char szValue[32];

    // Get the INI file name
    SetIniFileName();

    // Windows variation, get from the INI file
    ::GetPrivateProfileString(pszSection,
			      pszSetting,
			      "0",
			      szValue,
			      sizeof(szValue), m_pThis->m_strIniFile.c_str());
    strValue = szValue;

#endif

    nReturn = atoi(strValue.c_str());

    return (nReturn);
}


//--------------------------------------------------------------//
// Get a string value from the INI file                         //
//--------------------------------------------------------------//
string
Options::GetStringValue(const char *pszSection,
			const char *pszSetting, const char *pszDefault)
{
    string strValue;

#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    strValue =
	((INIFileReader *) (m_pThis->m_pINIFileReader))->GetOption(pszSection,
								   pszSetting,
								   pszDefault);

#else

    char szValue[512];

    // Get the INI file name
    SetIniFileName();

    // Windows variation, get from the INI file
    ::GetPrivateProfileString(pszSection,
			      pszSetting,
			      pszDefault,
			      szValue,
			      sizeof(szValue), m_pThis->m_strIniFile.c_str());
    strValue = szValue;

#endif

    return (strValue);
}


//--------------------------------------------------------------//
// Set a boolean value into the INI file                        //
//--------------------------------------------------------------//
void
Options::SetBooleanValue(const char *pszSection,
			 const char *pszSetting, bool bValue)
{
    const char *pszValue = (bValue == true ? "Yes" : "No");


#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    ((INIFileReader *) (m_pThis->m_pINIFileReader))->SetOption(pszSection,
							       pszSetting,
							       pszValue);

#else

    // Get the INI file name
    SetIniFileName();

    // Windows variation, save it
    ::WritePrivateProfileString(pszSection,
				pszSetting,
				pszValue, m_pThis->m_strIniFile.c_str());

#endif

}


//--------------------------------------------------------------//
// Set the name of the INI file.                                //
//--------------------------------------------------------------//
void
Options::SetIniFileName()
{
    if (m_pThis->m_strIniFile.length() == 0) {

#ifndef WIN32

	/* Linux */
	/* See if this user already has an INI file.  If so, use that */
	/* Otherwise, load the default ini file, and write it to the home directory */

	struct passwd *pwd = getpwuid(getuid());

	m_pThis->m_UserIniFile = 0;

	if (pwd) {
	    char path[128];
	    snprintf(path, 128, "%s/.pixildt/PixilDT.ini", pwd->pw_dir);

	    if (!access(path, F_OK)) {
		m_pThis->m_strIniFile = path;
		m_pThis->m_UserIniFile = 1;
		printf("Using INI file %s\n", path);
	    }
	}

	if (!m_pThis->m_UserIniFile) {
	    printf("Using the default INI file\n");
	    m_pThis->m_strIniFile = "./PixilDT.ini";
	}
#else

	char *pszPath;
	HKEY hKey;
	HKEY hKey2;
	HKEY hKey3;
	HKEY hKey4;
	unsigned long dwDisposition;
	unsigned long dwSize;
	unsigned long dwType;
	string strPath;

	// Get the INI file name from the registry if it is there
	if (::RegOpenKeyEx(HKEY_CURRENT_USER,
			   "Software\\Century Software\\Pixil Desktop",
			   0, KEY_READ, &hKey) == ERROR_SUCCESS) {
	    // Get the INI file path
	    if (::RegQueryValueEx(hKey,
				  "INIFile",
				  NULL,
				  &dwType, NULL, &dwSize) == ERROR_SUCCESS) {
		// Get the actual path data
		pszPath = new char[dwSize];
		::RegQueryValueEx(hKey,
				  "INIFile",
				  NULL,
				  &dwType,
				  (unsigned char *) pszPath, &dwSize);
		m_pThis->m_strIniFile = pszPath;
		delete[]pszPath;
	    }

	    ::RegCloseKey(hKey);
	}

	if (m_pThis->m_strIniFile.length() == 0) {
	    IniDlg *pDlg;

	    // No INI file available in the registry,
	    // Ask the user where to put the file
	    pDlg = new IniDlg();
	    if (pDlg->DoModal() != 1) {
		// Just quit if the user does not want to enter a directory for the files
		exit(EXIT_FAILURE);
	    }
	    strPath = pDlg->GetDataPath();
	    m_pThis->m_strIniFile = strPath;
	    if (m_pThis->m_strIniFile.length() == 0) {
		// Just quit if the user does not want to enter a directory for the files
		exit(EXIT_FAILURE);
	    }
	    m_pThis->m_strIniFile += "\\PixilDT.ini";
	    delete pDlg;

	    // Set the data base path
	    SetDatabasePath(strPath.c_str());

	    // Create these keys if they don't already exist
	    if (::RegCreateKeyEx(HKEY_CURRENT_USER,
				 "Software",
				 0,
				 NULL,
				 REG_OPTION_NON_VOLATILE,
				 KEY_WRITE,
				 NULL,
				 &hKey2, &dwDisposition) == ERROR_SUCCESS) {
		if (::RegCreateKeyEx(hKey2,
				     "Century Software",
				     0,
				     NULL,
				     REG_OPTION_NON_VOLATILE,
				     KEY_WRITE,
				     NULL,
				     &hKey3,
				     &dwDisposition) == ERROR_SUCCESS) {
		    if (::RegCreateKeyEx(hKey3,
					 "Pixil Desktop",
					 0,
					 NULL,
					 REG_OPTION_NON_VOLATILE,
					 KEY_WRITE,
					 NULL,
					 &hKey4,
					 &dwDisposition) == ERROR_SUCCESS) {
			// Set the INI file path
			long x =::RegSetValueEx(hKey4,
						"INIFile",
						NULL,
						REG_SZ,
						(const unsigned char
						 *) (m_pThis->m_strIniFile.
						     c_str()),
						m_pThis->m_strIniFile.
						length());
			::RegCloseKey(hKey4);
		    }
		    ::RegCloseKey(hKey3);
		}
		::RegCloseKey(hKey2);
	    }
	}
#endif

    }
}


//--------------------------------------------------------------//
// Set an integer value into the INI file                       //
//--------------------------------------------------------------//
void
Options::SetIntValue(const char *pszSection,
		     const char *pszSetting, int nValue)
{
    char szValue[16];

    // Get the value of the setting
    sprintf(szValue, "%d", nValue);

#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    ((INIFileReader *) (m_pThis->m_pINIFileReader))->SetOption(pszSection,
							       pszSetting,
							       szValue);

#else

    // Get the INI file name
    SetIniFileName();

    // Windows variation, save it
    ::WritePrivateProfileString(pszSection,
				pszSetting,
				szValue, m_pThis->m_strIniFile.c_str());

#endif

}


//--------------------------------------------------------------//
// Set astring value into the INI file                          //
//--------------------------------------------------------------//
void
Options::SetStringValue(const char *pszSection,
			const char *pszSetting, const char *pszValue)
{

#ifndef WIN32_PROFILE_API

    // Unix/Linux variation
    ((INIFileReader *) (m_pThis->m_pINIFileReader))->SetOption(pszSection,
							       pszSetting,
							       pszValue);

#else

    // Get the INI file name
    SetIniFileName();

    // Windows variation, save it
    ::WritePrivateProfileString(pszSection,
				pszSetting,
				pszValue, m_pThis->m_strIniFile.c_str());

#endif

}


#ifndef WIN32_PROFILE_API
// Linux/Unix based INI file classes


//--------------------------------------------------------------//
// Construct from an INI file.                                  //
//--------------------------------------------------------------//
INIFileReader::INIFileReader(const string & strFileName)
{
    char szBuffer[512];
    ifstream fileIn;
    INICategory *pINICategory = NULL;
    int i;
    string strName = "(none)";

    // Initialize instance variables
    m_bModifiedFlag = false;
    m_strFileName = strFileName;

    // Open the INI file
    fileIn.open(m_strFileName.c_str());

    // Continue if the file was found
    if (fileIn.is_open()) {
	// Read each line from the file
	while (!fileIn.eof()) {
	    fileIn.getline(szBuffer, sizeof(szBuffer));

	    // Trim trailing spaces from the line
	    i = strlen(szBuffer) - 1;
	    while (i >= 0 && isspace(szBuffer[i])) {
		szBuffer[i--] = '\0';
	    }

	    // Process the line if not blank
	    if (szBuffer[0] != '\0') {
		if (szBuffer[0] == '[') {
		    // New category, get the category name
		    strName = &szBuffer[1];
		    if (strName[strName.length() - 1] == ']') {
			strName = strName.substr(0, strName.length() - 1);
		    }
		    // Create the new category
		    FindCategory(strName, true);
		} else {
		    // This is a new option for the category
		    pINICategory = FindCategory(strName, true);

		    // Add this option to the category
		    pINICategory->SetOption(string(szBuffer));
		}
	    }
	}
    }
    // Reset the modified flag
    m_bModifiedFlag = false;

    // Clean up
    fileIn.close();
}


//--------------------------------------------------------------//
// Find a category or create one.                               //
//--------------------------------------------------------------//
INICategory *
INIFileReader::FindCategory(const string & strCategory, bool bCreate)
{
    bool bFound = false;
    INICategory *pINICategory = NULL;
    int i;
    int nMax = m_vCategory.size();

    // Find this category
    for (i = 0; i < nMax; ++i) {
	if (m_vCategory[i] == strCategory) {
	    pINICategory = &m_vCategory[i];
	    bFound = true;
	    break;
	}
    }

    // Was it found ?
    if (bFound == false && bCreate == true) {
	// Create a new one
	pINICategory = new INICategory(strCategory);
	m_vCategory.push_back(*pINICategory);
	delete pINICategory;
	pINICategory = &m_vCategory[m_vCategory.size() - 1];
	m_bModifiedFlag = true;
    }

    return (pINICategory);
}


//--------------------------------------------------------------//
// Retrieve an INI file setting.                                //
//--------------------------------------------------------------//
string
INIFileReader::GetOption(const string & strCategory,
			 const string & strName, const string & strDefault)
{
    INICategory *pINICategory = FindCategory(strCategory, false);
    string strReturn;

    if (pINICategory != NULL) {
	strReturn = pINICategory->GetValue(strName, strDefault);
    } else {
	strReturn = strDefault;
    }

    return (strReturn);
}


//--------------------------------------------------------------//
// Set or create an option.                                     //
//--------------------------------------------------------------//
void
INIFileReader::SetOption(const string & strCategory,
			 const string & strName, const string & strValue)
{
    bool bModifiedFlag;
    INICategory *pINICategory = FindCategory(strCategory, true);

    bModifiedFlag = pINICategory->SetOption(strName, strValue);
    if (bModifiedFlag == true) {
	m_bModifiedFlag = true;
    }
}


//--------------------------------------------------------------//
// Write the INI file back to disk overwriting the file it was  //
// read from.                                                   //
//--------------------------------------------------------------//
void
INIFileReader::Write()
{
    if (m_bModifiedFlag == true) {
	struct passwd *pwd = getpwuid(getuid());
	if (!pwd) {
	    printf("Error - unable to get the user name for id %d\n",
		   getuid());
	    return;
	}

	/* Create the user ini file if we need to */

	if (strncmp(pwd->pw_dir, m_strFileName.c_str(), strlen(pwd->pw_dir))) {
	    char path[128];
	    snprintf(path, 128, "%s/.pixildt", pwd->pw_dir);

	    if (access(path, F_OK)) {
		if (mkdir(path, 0755)) {
		    printf("Error - unable to create %s\n", path);
		    return;
		}
	    }

	    strcat(path, "/PixilDT.ini");
	    m_strFileName = path;

	    printf("Saving INI file %s\n", path);
	}


	Write(m_strFileName);
	m_bModifiedFlag = false;
    }
}


//--------------------------------------------------------------//
// Write the INI file back to disk.                             //
//--------------------------------------------------------------//
void
INIFileReader::Write(const string & strFileName)
{
    ofstream fileOut;
    vector < INICategory >::iterator iter;

    // Open the output file
    fileOut.open(strFileName.c_str());

    // Continue if open
    if (fileOut.is_open()) {
	for (iter = m_vCategory.begin(); iter != m_vCategory.end(); ++iter) {
	    iter->Write(fileOut);
	}
	fileOut.close();
    }
}


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
INICategory::INICategory(const string & strName)
{
    m_strName = strName;
}


//--------------------------------------------------------------//
// Assignment operator.                                         //
//--------------------------------------------------------------//
INICategory & INICategory::operator=(const INICategory & Other)
{
    m_mSetting = Other.m_mSetting;
    m_strName = Other.m_strName;
    return (*this);
}


//--------------------------------------------------------------//
// Comparison operator.                                         //
//--------------------------------------------------------------//
bool
INICategory::operator==(const INICategory & Other) const
{
    return (strcasecmp(m_strName.c_str(), Other.m_strName.c_str()) == 0);
}


//--------------------------------------------------------------//
// Comparison operator.                                         //
//--------------------------------------------------------------//
bool
INICategory::operator==(const string & strOther) const
{
    return (strcasecmp(m_strName.c_str(), strOther.c_str()) == 0);
}


//--------------------------------------------------------------//
// Comparison operator.                                         //
//--------------------------------------------------------------//
bool
INICategory::operator<(const INICategory & Other) const
{
    return (strcasecmp(m_strName.c_str(), Other.m_strName.c_str()) < 0);
}


//--------------------------------------------------------------//
// Find a setting by name                                       //
//--------------------------------------------------------------//
map < string,
    string >::iterator INICategory::FindSetting(const string & strName)
{
    map < string, string >::iterator iter;

    for (iter = m_mSetting.begin(); iter != m_mSetting.end(); ++iter) {
	if (strcasecmp(strName.c_str(), iter->first.c_str()) == 0) {
	    break;
	}
    }
    return (iter);
}


//--------------------------------------------------------------//
// Get the value for a name                                     //
//--------------------------------------------------------------//
string
INICategory::GetValue(const string & strName, const string & strDefault)
{
    map < string, string >::iterator iter = FindSetting(strName);
    string strReturn;

    if (iter != m_mSetting.end()) {
	strReturn = iter->second;
    } else {
	strReturn = strDefault;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Add an option from a string in the format name=value.        //
//--------------------------------------------------------------//
void
INICategory::SetOption(const string & strLine)
{
    string strName;
    string strValue;
    unsigned int nPos;

    // Get the option name and value from the string
    nPos = strLine.find("=");
    if (nPos >= 0) {
	// Get the name
	if (nPos > 0) {
	    strName = strLine.substr(0, nPos);
	} else {
	    strName = "";
	}

	// Get the value
	if (nPos < strLine.length() - 1) {
	    strValue = strLine.substr(nPos + 1, strLine.length());
	} else {
	    strValue = "";
	}
    } else {
	// No equal sign, assume all is name
	strName = strLine;
	strValue = "";
    }

    SetOption(strName, strValue);
}


//--------------------------------------------------------------//
// Set or create an option.                                     //
//--------------------------------------------------------------//
bool
INICategory::SetOption(const string & strName, const string & strValue)
{
    bool bReturn = true;
    map < string, string >::iterator iter;
    pair < string, string > prElement;

    // Is the name in the list
    iter = FindSetting(strName);

    if (iter == m_mSetting.end()) {
	// Name not found, create it
	prElement.first = strName;
	prElement.second = strValue;
	m_mSetting.insert(prElement);
    } else if (iter->second != strValue) {
	// Different value, set to the new one
	iter->second = strValue;
    } else {
	// Same value, no need to set
	bReturn = false;
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Write this category to disk.                                 //
//--------------------------------------------------------------//
void
INICategory::Write(ostream & fileOut)
{
    map < string, string >::iterator iter;

    // Output the category name
    fileOut << "[" << m_strName.c_str()
	<< "]" << endl;

    // Output the values within
    for (iter = m_mSetting.begin(); iter != m_mSetting.end(); ++iter) {
	fileOut << iter->first.c_str()
	    << "=" << iter->second.c_str()
	    << endl;
    }

    // Output a blank line
    fileOut << endl;
}


#endif
