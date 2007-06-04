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
// Main window for PixilDT prototype.                           //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/x.H>
#ifdef WIN32
#include "../getopt/getopt.h"
#endif
#include "HelpID.h"
#include "Images.h"
#include "Options.h"
#include "PixilDT.h"
#include "PixilMainWnd.h"
#ifdef WIN32
#include "resource.h"
#endif

#ifdef WIN32
#include <direct.h>
#endif

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Singleton pointer.                                           //
//--------------------------------------------------------------//
PixilDT *
    PixilDT::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
PixilDT::PixilDT(int argc, char **argv)
{
#ifdef DEBUG
    // Temp workaround, set the LANG environment variable to the language to translate to
    putenv("LANG=it");
#endif

    // Set the singleton pointer
    m_pThis = this;

    // Set up the proper locale and facets
    try {
	setlocale(LC_ALL, getenv("LANG"));
    }
    catch(...) {
	// Leave the locale at its default
    }

    // Determine the date formatting for validating entered dates
    DetermineDateFormat();

    // Translate button titles used in FLTK
    fl_no = _("&No");
    fl_yes = _("&Yes");
    fl_ok = _("&OK");
    fl_cancel = _("Cancel");

    // Set up the background color for FLTK
    Fl::get_system_colors();

    // Create the main window
    m_pMainWindow = new PixilMainWnd(argc, argv);
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
PixilDT::~PixilDT()
{
    delete m_pMainWindow;
}


//--------------------------------------------------------------//
// Determine the current local's date format based on the       //
// output of strftime.                                          //
//--------------------------------------------------------------//
void
PixilDT::DetermineDateFormat()
{
    char *pszSep[2];
    char szData[16];
    char szDate[64];
    int i;
    int nFirst;
    int nLength;
    int nMax;
    int nSecond;
    //int nThird;
    time_t nDate;
    struct tm TM;

    // Get February 1st, 2003 formatted by strftime, to determine the date formatting order
    memset(&TM, 0, sizeof(TM));
    TM.tm_year = 2003 - 1900;
    TM.tm_mon = 1;
    TM.tm_mday = 1;
    TM.tm_hour = 12;
    nDate = mktime(&TM);
    strftime(szDate, sizeof(szDate), "%x", localtime(&nDate));

    // Find separators in the formatted date
    pszSep[0] = NULL;
    pszSep[1] = NULL;
    nMax = strlen(szDate);
    m_cDateSeparator = '/';	// Just in case none found
    for (i = 0; i < nMax; ++i) {
	if (!isdigit(szDate[i])) {
	    if (pszSep[0] == NULL) {
		pszSep[0] = &szDate[i];

		// Save the date separator
		m_cDateSeparator = szDate[i];
	    } else {
		pszSep[1] = &szDate[i];
		break;
	    }
	}
    }

    // Two delimiters found ?
    if (pszSep[1] != NULL) {
	nLength = pszSep[0] - szDate;
	strncpy(szData, szDate, nLength);
	szData[nLength] = '\0';
	nFirst = atoi(szData);
	nLength = pszSep[1] - pszSep[0] - 1;
	strncpy(szData, pszSep[0] + 1, nLength);
	szData[nLength] = '\0';
	nSecond = atoi(szData);
	//nLength=nMax+szDate-pszSep[1]-1;
	//strncpy(szData,pszSep[1]+1,nLength);
	//szData[nLength]='\0';
	//nThird=atoi(szData);
	if (nFirst == 1) {
	    // First is day
	    if (nSecond == 2) {
		m_nDateFormat = DATE_DMY;
	    } else {
		m_nDateFormat = DATE_DYM;
	    }
	} else if (nFirst == 2) {
	    // First is month
	    if (nSecond == 1) {
		m_nDateFormat = DATE_MDY;
	    } else {
		m_nDateFormat = DATE_MYD;
	    }
	} else			//if (nFirst==3||nFirst==2003)
	{
	    // First is year
	    if (nSecond == 1) {
		m_nDateFormat = DATE_YDM;
	    } else {
		m_nDateFormat = DATE_YMD;
	    }
	}
    } else {
	// Two delimiters were not found
	m_nDateFormat = DATE_YMD;
    }

    // Determine the time separator character
    strftime(szDate, sizeof(szDate), "%X", localtime(&nDate));
    nMax = strlen(szDate);
    for (i = 0; i < nMax; ++i) {
	if (!isdigit(szDate[i])) {
	    m_cTimeSeparator = szDate[i];
	    break;
	}
    }

    // Get the strings used for AM/PM
    memset(&TM, 0, sizeof(TM));
    TM.tm_year = 80;
    //TM.tm_mon=0; // January
    TM.tm_mday = 2;
    TM.tm_hour = 6;
    nDate = mktime(&TM);
    strftime(szData, sizeof(szData), "%p", localtime(&nDate));
    m_strAM = szData;
    nDate += 12 * 60 * 60;
    strftime(szData, sizeof(szData), "%p", localtime(&nDate));
    m_strPM = szData;
}


//--------------------------------------------------------------//
// Set the Pixil Icon for a window.  This has to be done for    //
// the first window to be displayed since that window will      //
// register the FLTK window class for the WIN32 version of the  //
// code.                                                        //
//--------------------------------------------------------------//
void
PixilDT::SetPixilIcon(Fl_Window * pWindow)
{

#ifndef WIN32

    // Set the icon for X Windows
    pWindow->icon((char *) Images::GetPixmap(Images::PIXIL_ICON));

#else

    // Load the window icon from the resource file
    pWindow->
	icon((char *) LoadIcon(fl_display, MAKEINTRESOURCE(IDI_PIXIL_ICON)));

#endif

}


//--------------------------------------------------------------//
// Show help for a given help ID.                               //
//--------------------------------------------------------------//
void
PixilDT::ShowHelp(int nHelpID)
{
    static const char *pszHelpInfo[] = {
	"AddressBookCategory.html",
	"AddressBookDlg.html",
	"CategoryEditor.html",
	"CustomFieldEditor.html",
	"FindDlg.html",
	"ListByDlg.html",
	"NewUser.html",
	"NotesCategory.html",
	"NoteEditorDlg.html",
	"OptionsDlg.html",
	"SchedulerCategory.html",
	"SchedulerDlg.html",
	"SchedulerRepeatDlg.html",
	"SchedulerTimeDlg.html",
	"ToDoListCategory.html",
	"ToDoShowDlg.html",
    };
    static const HelpID nHelpIDList[] = {
	HELP_ADDRESS_BOOK_CATEGORY,	// Help on address book categories
	HELP_ADDRESS_BOOK_DLG,	// Help on the Address Book update dialog
	HELP_CATEGORY_EDITOR,	// Help on the Category Editor
	HELP_CUSTOM_FIELD_EDITOR,	// Help on the Custom Field Editor
	HELP_FIND_DLG,		// Help on the Find dialog
	HELP_LISTBY_DLG,	// Help on the Address Book/List By dialog
	HELP_NEW_USER,		// Help for the new user dialog
	HELP_NOTES_CATEGORY,	// Help on notes categories
	HELP_NOTE_EDITOR_DLG,	// Help on the Note Editor dialog
	HELP_OPTIONS_DLG,	// Help on the Options dialog
	HELP_SCHEDULER_CATEGORY,	// Help on scheduler categories
	HELP_SCHEDULER_DLG,	// Help on the Scheduler update dialog
	HELP_SCHEDULER_REPEAT_DLG,	// Help on the Scheduler Repeat entry dialog
	HELP_SCHEDULER_TIME_DLG,	// Help on the Scheduler time entry dialog
	HELP_TODO_LIST_CATEGORY,	// Help on ToDO list categories
	HELP_TODO_SHOW_DLG,	// Help on the ToDo List Show dialog
    };
    string strCmd;
    string strPath;
    unsigned int nIndex;

    // Find this help ID in the list
    for (nIndex = 0; nIndex < sizeof(nHelpIDList); ++nIndex) {
	if (nHelpIDList[nIndex] == nHelpID) {
	    break;
	}
    }

    if (nIndex >= sizeof(nHelpIDList)) {
	// Help not available
	fl_alert(_("Help is not yet available for this topic"));
	return;			// early exit
    }
#ifndef WIN32

    // Unix/Linux - no help yet
    fl_alert(_("Help is not yet implemented"));

#else

    bool bHaveBrowser = false;
    char szBrowser[256];
    char szData[256];
    char szKey[2];
    unsigned long nSize;
    unsigned long nType;
    HKEY hKey;

    // MS/Windows - find the browser from the registry and launch it
    if (::RegOpenKeyEx(HKEY_CURRENT_USER,
		       "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.htm\\OpenWithList",
		       0, KEY_READ, &hKey) == ERROR_SUCCESS) {
	// Get the Most recently used list for this file extension
	nSize = sizeof(szData);
	if (::
	    RegQueryValueEx(hKey, "MRUList", 0, &nType,
			    (unsigned char *) szData,
			    &nSize) == ERROR_SUCCESS) {
	    // Get the name of the executable
	    szKey[0] = szData[0];
	    szKey[1] = '\0';
	    nSize = sizeof(szData);
	    if (::
		RegQueryValueEx(hKey, szKey, 0, &nType,
				(unsigned char *) szBrowser,
				&nSize) == ERROR_SUCCESS) {
		bHaveBrowser = true;
	    }
	}
	// Close the registry key
	::RegCloseKey(hKey);
    }
    // Get the path to the browser
    if (bHaveBrowser == true) {
	bHaveBrowser = false;
	strCmd = "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
	strCmd += szBrowser;
	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			   strCmd.c_str(),
			   0, KEY_READ, &hKey) == ERROR_SUCCESS) {
	    // Get the Most recently used list for this file extension
	    nSize = sizeof(szData);
	    if (::
		RegQueryValueEx(hKey, NULL, 0, &nType,
				(unsigned char *) szData,
				&nSize) == ERROR_SUCCESS) {
		::GetShortPathName(szData, szBrowser, sizeof(szBrowser));
		bHaveBrowser = true;
	    }
	    // Close the registry key
	    ::RegCloseKey(hKey);
	}
    }
    // Now launch the browser if one is there
    if (bHaveBrowser == true) {
	strCmd = szBrowser;
	strCmd += " file://";

	strPath = Options::GetHelpPath();
	if (strPath == ".") {
	    char szPath[_MAX_PATH];

	    getcwd(szPath, sizeof(szPath));
	    strPath = szPath;
	}
	strCmd += strPath;
	strCmd += "/";
	strCmd += pszHelpInfo[nIndex];
	system(strCmd.c_str());
    }
#endif

}
