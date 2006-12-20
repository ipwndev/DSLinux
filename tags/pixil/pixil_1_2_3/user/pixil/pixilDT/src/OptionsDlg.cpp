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
// Options Dialog.                                              //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_message.H>
#include <FL/Fl_Return_Button.H>
#include "OptionsDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "InputBox.h"
#include "Options.h"
#include "PixilDT.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
#define max __max
#define min __min
#endif


#define CHOICE_WIDTH 150
#define PROMPT_WIDTH 150
#define TAB_HEIGHT   DLG_BUTTON_HEIGHT
#define DLG_HEIGHT   (6*DLG_BORDER+4*DLG_BUTTON_HEIGHT+DLG_INPUT_HEIGHT+TAB_HEIGHT)
#define DLG_WIDTH    (5*DLG_BORDER+PROMPT_WIDTH+150+DLG_BUTTON_WIDTH)


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
OptionsDlg::OptionsDlg(Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Options"))
{
    static const char *pszApp[4] = {
	"Scheduler",
	"Address Book",
	"Notes",
	"ToDo List",
    };
    static const char *pszDay[2] = {
	"Sunday",
	"Monday",
    };
    char *pszHour[23];
    Fl_Group *pGroup;
    Fl_Tabs *pTabs;
    int i;
    int j;
    string strAM;
    string strPM;

    // Create the tab widget
    pTabs = new Fl_Tabs(DLG_BORDER,
			DLG_BORDER,
			w() - 2 * DLG_BORDER,
			h() - 3 * DLG_BORDER - DLG_BUTTON_HEIGHT);

    // Create the first page for the tab
    pGroup = new Fl_Group(2 * DLG_BORDER,
			  DLG_BORDER + TAB_HEIGHT,
			  w() - 4 * DLG_BORDER,
			  h() - 5 * DLG_BORDER - DLG_BUTTON_HEIGHT,
			  _("General"));
    m_pAppChoice = new Fl_Choice(2 * DLG_BORDER + PROMPT_WIDTH,
				 TAB_HEIGHT + 2 * DLG_BORDER,
				 CHOICE_WIDTH,
				 DLG_BUTTON_HEIGHT,
				 _("Startup Application:"));
    m_pMenuApp = CreateChoice(4, pszApp, true);
    m_pAppChoice->menu(m_pMenuApp);
    m_pAppChoice->value(Options::GetMainPage());
    m_pDataDir = new Fl_Input(2 * DLG_BORDER + PROMPT_WIDTH,
			      TAB_HEIGHT + 3 * DLG_BORDER + DLG_BUTTON_HEIGHT,
			      w() - 5 * DLG_BORDER - DLG_BUTTON_WIDTH -
			      PROMPT_WIDTH, DLG_INPUT_HEIGHT,
			      _("Data Directory:"));
    m_pDataDir->maximum_size(256);
    m_pDataDir->value(Options::GetDatabasePath().c_str());
    m_pBrowseButton = new Fl_Button(w() - 2 * DLG_BORDER - DLG_BUTTON_WIDTH,
				    TAB_HEIGHT + 3 * DLG_BORDER +
				    DLG_BUTTON_HEIGHT, DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, _("&Browse"));
    m_pBrowseButton->callback(OnBrowseButton);
    m_pConfirmDelete = new Fl_Check_Button(2 * DLG_BORDER + PROMPT_WIDTH,
					   TAB_HEIGHT + 4 * DLG_BORDER +
					   2 * DLG_BUTTON_HEIGHT,
					   w() - 4 * DLG_BORDER,
					   DLG_INPUT_HEIGHT,
					   _("Confirm before Delete:"));
    m_pConfirmDelete->align(FL_ALIGN_LEFT);
    m_pConfirmDelete->value(Options::GetConfirmDelete() == true ? 1 : 0);
    pGroup->end();

    // Create the second page for the tab
    pGroup = new Fl_Group(2 * DLG_BORDER,
			  DLG_BORDER + TAB_HEIGHT,
			  w() - 4 * DLG_BORDER,
			  h() - 5 * DLG_BORDER - DLG_BUTTON_HEIGHT,
			  _("Date Book"));
    m_pDayBegins = new Fl_Choice(2 * DLG_BORDER + PROMPT_WIDTH,
				 TAB_HEIGHT + 2 * DLG_BORDER,
				 CHOICE_WIDTH,
				 DLG_BUTTON_HEIGHT, _("Workday Begins:"));
    PixilDT::GetAMPM(strAM, strPM);
    for (i = 0; i < 23; ++i) {
	if (strAM.length() != 0) {
	    // Twelve hour clock
	    j = ((i + 1) % 12);
	    if (j == 0) {
		j = 12;
	    }
	    pszHour[i] =
		new char[3 + max(strAM.length(), strPM.length()) + 1];
	    sprintf(pszHour[i], "%d %s", j,
		    (i + 1) / 12 == 0 ? strAM.c_str() : strPM.c_str());
	} else {
	    // 24 hour clock
	    pszHour[i] = new char[3];
	    sprintf(pszHour[i], "%d", i + 1);
	}
    }
    m_pMenuDayBegins = CreateChoice(23, pszHour, false);
    m_pDayBegins->menu(m_pMenuDayBegins);
    m_pDayBegins->value(Options::GetDayBegins() ==
			0 ? 8 - 1 : Options::GetDayBegins() - 1);
    for (i = 0; i < 23; ++i) {
	delete pszHour[i];
    }
    m_pWeekBegins = new Fl_Choice(2 * DLG_BORDER + PROMPT_WIDTH,
				  TAB_HEIGHT + 3 * DLG_BORDER +
				  DLG_BUTTON_HEIGHT, CHOICE_WIDTH,
				  DLG_BUTTON_HEIGHT, _("Week Begins:"));
    m_pMenuWeekBegins = CreateChoice(2, pszDay, true);
    m_pWeekBegins->menu(m_pMenuWeekBegins);
    m_pWeekBegins->value(Options::GetWeekBegins());
    pGroup->end();		// End the second page

    // End the tab widget
    pTabs->end();

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
OptionsDlg::~OptionsDlg()
{
    FreeTranslatedMenu(m_pMenuApp);
    FreeTranslatedMenu(m_pMenuDayBegins);
    FreeTranslatedMenu(m_pMenuWeekBegins);
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
OptionsDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// Get the values from this dialog
	Options::SetMainPage(m_pAppChoice->value());
	Options::SetConfirmDelete(m_pConfirmDelete->value() == 1);
	Options::SetDayBegins(m_pDayBegins->value() + 1);
	if (Options::GetWeekBegins() != m_pWeekBegins->value()) {
	    // Change the setting
	    Options::SetWeekBegins(m_pWeekBegins->value());

	    // Refresh displays
	    PixilDT::GetApp()->GetMainWindow()->Notify(BEGIN_WEEK_CHANGED, 0);
	}
	if (Options::GetDatabasePath() != m_pDataDir->value()) {
	    // Change the database path
	    Options::SetDatabasePath(m_pDataDir->value());

	    // Close all open databases
	    NxDbAccess::CloseAll();

	    // Refresh all data
	    PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED,
						       0);
	    PixilDT::GetApp()->GetMainWindow()->Notify(NOTES_CHANGED, 0);
	    PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);
	    PixilDT::GetApp()->GetMainWindow()->Notify(TODO_LIST_CHANGED, 0);
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Browse button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
OptionsDlg::OnBrowseButton(Fl_Widget * pWidget, void *pUserData)
{
    InputBox *pInputBox;
    OptionsDlg *pThis =
	(OptionsDlg *) (pWidget->parent()->parent()->parent());

    // Get the directory name
    pInputBox = new InputBox(_("Data Directory"),
			     PixilDT::GetApp()->GetMainWindow(),
			     pThis->m_pDataDir->value(),
			     256,
			     HELP_OPTIONS_DLG,
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
OptionsDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_OPTIONS_DLG);
}


//--------------------------------------------------------------//
// Validate a new data directory (static callback).             //
//--------------------------------------------------------------//
bool
OptionsDlg::Validate(const char *pszString,
		     Fl_Widget * pThis, void *pUserData)
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
