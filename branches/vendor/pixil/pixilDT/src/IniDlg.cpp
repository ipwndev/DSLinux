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
// Dialog for a missing INI file or a new user.                 //
//--------------------------------------------------------------//
#include "config.h"
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <FL/filename.H>
#include <FL/fl_message.H>
#include <FL/Fl_Return_Button.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "IniDlg.h"
#include "InputBox.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define PROMPT_WIDTH 140
#define DLG_HEIGHT   (4*DLG_BORDER+4*DLG_BUTTON_HEIGHT)
#define DLG_WIDTH    (2*DLG_BORDER+PROMPT_WIDTH+400+DLG_BUTTON_WIDTH)


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
IniDlg::IniDlg()
:  Fl_Window(DLG_WIDTH, DLG_HEIGHT, _("New User Settings"))
{
    Fl_Box *pBox;

    // Create the widgets
    pBox = new Fl_Box(DLG_BORDER,
		      DLG_BORDER,
		      w() - 2 * DLG_BORDER, 2 * DLG_BUTTON_HEIGHT);
    pBox->align(Fl_Align(FL_ALIGN_INSIDE + FL_ALIGN_WRAP));
    pBox->
	label(_
	      ("You are a new user.  Please enter the directory where you would to have the Pixil Desktop files stored:"));
    m_pDataDir =
	new Fl_Input(PROMPT_WIDTH, 2 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT,
		     w() - 2 * DLG_BORDER - DLG_BUTTON_WIDTH - PROMPT_WIDTH,
		     DLG_INPUT_HEIGHT, _("Data Directory:"));
    m_pDataDir->maximum_size(256);
    m_pBrowseButton = new Fl_Button(w() - DLG_BORDER - DLG_BUTTON_WIDTH,
				    2 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, _("&Browse"));
    m_pBrowseButton->callback(OnBrowseButton);

    // Create the buttons
    m_pOKButton =
	new Fl_Return_Button(w() - 3 * DLG_BORDER - 3 * DLG_BUTTON_WIDTH,
			     h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
			     DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton =
	new Fl_Button(w() - 2 * DLG_BORDER - 2 * DLG_BUTTON_WIDTH,
		      h() - DLG_BUTTON_HEIGHT - DLG_BORDER, DLG_BUTTON_WIDTH,
		      DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BORDER - DLG_BUTTON_WIDTH,
				  h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);

    // Finish the dialog
    end();

    // The DoModal method will show this dialog
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
IniDlg::~IniDlg()
{
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
IniDlg::DoModal()
{
    bool bGood;
    int nResult;

    // Keep doing the dialog until a good directory is entered
    // or the cancel button is pressed
    do {
	nResult =::DoModal(this, m_pOKButton, m_pCancelButton);
	if (nResult == 0) {
	    // Cancel button
	    break;
	}
	bGood = ValidateDirectory(m_pDataDir->value(), m_strPath);
    } while (bGood == false);

    return (bGood);
}


//--------------------------------------------------------------//
// Browse button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
IniDlg::OnBrowseButton(Fl_Widget * pWidget, void *pUserData)
{
    InputBox *pInputBox;
    IniDlg *pThis = (IniDlg *) (pWidget->parent());

    // Get the directory name
    pInputBox = new InputBox(_("Data Directory"),
			     pThis,
			     pThis->m_pDataDir->value(),
			     256,
			     HELP_NEW_USER,
			     Validate,
			     pThis,
			     NULL, _("Please enter the data directory:"));
    if (pInputBox->GetEntry().length() > 0) {
	pThis->m_pDataDir->value(pInputBox->GetEntry().c_str());
    }
    delete pInputBox;
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
IniDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_NEW_USER);
}


//--------------------------------------------------------------//
// Validate a new data directory (static callback).             //
//--------------------------------------------------------------//
bool
IniDlg::Validate(const char *pszString, Fl_Widget * pThis, void *pUserData)
{
    bool bNotBlank = false;
    int i;

    // The directory cannot start or end with a blank
    if (!isspace(pszString[0]) && !isspace(pszString[strlen(pszString) - 1])) {
	// The directory cannot be completely blank
	for (i = 0; pszString[i] != '\0'; ++i) {
	    if (!isspace(pszString[i])) {
		bNotBlank = true;
		break;
	    }
	}
    }
    return (bNotBlank);
}


//--------------------------------------------------------------//
// Validate an entered directory.                               //
//--------------------------------------------------------------//
bool
IniDlg::ValidateDirectory(const char *pszDirectory, string & strPath)
{
    bool bCreated;
#ifndef WIN32
    static const char cSlash = '/';
#else
    static const char cSlash = '\\';
#endif
    char szDirectory[1024];
    char szPath[1024];
    bool bReturn = false;
    FILE *file;
#ifdef WIN32
    int nDrive;
    int nOldDrive;
#endif
    int nResult;
#ifndef WIN32
    string strCommand;
#endif
    string strDir;
    string strFile;
    string strFile2;
    unsigned int nPos;

    // Save the current directory
    getcwd(szDirectory, sizeof(szDirectory));

#ifdef WIN32
    // Save the current drive letter
    nOldDrive = _getdrive();
    nDrive = nOldDrive;
#endif

    // Is there a file name
    if (strlen(pszDirectory) > 0) {
	// Is this a directory
	if (filename_isdir(pszDirectory)) {
	    // Does it already contain a data base
	    strFile = pszDirectory;
	    if (strFile[strFile.length() - 1] == cSlash
		&& strFile.length() > 1) {
		strFile = strFile.substr(0, strFile.length() - 1);
	    }
	    strFile += cSlash;
	    strFile += "PixilDT.ini";
	    file = fopen(strFile.c_str(), "r");
	    if (file != NULL) {
		// A database already exists in this directory,
		// ask the user whether to use it or not
		fclose(file);
		nResult =
		    fl_ask(_
			   ("A database already exists in this directory, use the data in this existing data base?"));
		bReturn = (nResult == 1);
	    } else {
		bReturn = true;
	    }
	} else			// Not an existing directory
	{
	    // Is this an existing file ?
	    file = fopen(pszDirectory, "r");
	    if (file != NULL) {
		// This is a file and cannot be used
		fclose(file);
		fl_message(_
			   ("This name already exists as a file so it cannot be used as the target directory"));
	    } else		// Not an existing file
	    {
		// Ask if it should be created
		nResult =
		    fl_ask(_
			   ("This directory does not exist yet, should it be created?"));
		bReturn = (nResult == 1);

		// Now create the directory if needed
		if (bReturn == true) {
		    strFile = pszDirectory;

#ifndef WIN32
		    if (strFile[0] != cSlash) {
			getcwd(szDirectory, sizeof(szDirectory));
			strFile2 = strFile;
			strFile = szDirectory;
			strFile += '/';
			strFile += strFile2;
		    }
#else
		    if (strFile[1] == ':'
			&& ((strFile[0] >= 'A' && strFile[0] <= 'Z')
			    || (strFile[0] >= 'a' && strFile[0] <= 'z'))) {
			if ((strFile[0] >= 'A' && strFile[0] <= 'Z')) {
			    nDrive = strFile[0] - 'A' + 1;
			} else {
			    nDrive = strFile[0] - 'a' + 1;
			}
			strFile = strFile.substr(2);
		    } else {
			nDrive = _getdrive();
		    }
		    if (strFile[0] != cSlash) {
			getcwd(szDirectory, sizeof(szDirectory));
			strFile2 = strFile;
			strFile = szDirectory;
			strFile += '\\';
			strFile += strFile2;
		    }
#endif

#ifdef WIN32
		    // Switch to the selected drive
		    _chdrive(nDrive);
#endif

		    // Go to the root directory
		    strDir = cSlash;
		    chdir(strDir.c_str());

		    // Create each directory in turn
		    bCreated = false;
		    while (strFile.length() > 0) {
			// Remove the leading slash
			strFile = strFile.substr(1);

			// Find the next slash
			nPos = strFile.find_first_of(cSlash);

			// Was one found
			if (nPos != string::npos) {
			    // Get the next directory level
			    strDir = strFile.substr(0, nPos);
			    strFile = strFile.substr(nPos);

			    // Go to this directory
			    if (chdir(strDir.c_str()) != 0) {
				// Create this directory level
#ifndef WIN32
				strCommand = "mkdir ";
				strCommand += strDir.c_str();
				if (system(strCommand.c_str()) != 0)
#else
				if (mkdir(strDir.c_str()) != 0)
#endif
				{
				    break;
				}
				chdir(strDir.c_str());
			    }
			} else {
			    // Last level of sub-directories
#ifndef WIN32
			    strCommand = "mkdir ";
			    strCommand += strFile.c_str();
			    if (system(strCommand.c_str()) != 0)
#else
			    if (mkdir(strFile.c_str()) != 0)
#endif
			    {
				break;
			    }
			    strFile = "";
			    bCreated = true;
			}
		    }

		    // Was everything created
		    if (bCreated == true) {
			bReturn = true;
		    } else {
			fl_message(_("Unable to create the directory."));
		    }
		}
	    }
	}
    }
#ifdef WIN32
    // Switch back to the original drive
    _chdrive(nOldDrive);
#endif

    // Switch back to the original directory
    chdir(szDirectory);

    // Fix up the output path if all is well
    if (bReturn == true) {
	filename_absolute(szPath, pszDirectory);

#ifndef WIN32
	// Save the final path
	strPath = szPath;
#else
	// Add the drive letter to the path if needed
	if (strFile[1] != ':'
	    || ((strFile[0] < 'A' || strFile[0] > 'Z')
		&& (strFile[0] < 'a' || strFile[0] > 'z'))) {
	    strPath = nDrive + 'A' - 1;
	    strPath += ':';
	    strPath += szPath;
	} else {
	    strPath = szPath;
	}
#endif

    }

    return (bReturn);
}
