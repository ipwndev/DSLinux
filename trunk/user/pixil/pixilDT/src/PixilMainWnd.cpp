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
// Class for the main window for the Pixil Desktop.             //
//--------------------------------------------------------------//
#include "config.h"
#include <cstdio>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_message.H>
#include <FL/Fl_Return_Button.H>
#include "AddressBookDB.h"
#include "CategoryEditor.h"
#include "CustomFieldEditor.h"
#include "Dialog.h"
#include "FindDlg.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "LeftGroup.h"
#include "Options.h"
#include "OptionsDlg.h"
#include "PixilDT.h"
#include "PixilMainWnd.h"
#include "SplashDlg.h"

#include "VCMemoryLeak.h"
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT_WIDTH       64	// Size of the left portion of the main window
#define MAIN_WND_WIDTH  700	// Default window width
#define MAIN_WND_HEIGHT 500	// Default window height
#define MENU_HEIGHT      30	// Height of the menu

PixilMainWnd *g_MainWnd;


//--------------------------------------------------------------//
// Default Constructor.                                         //
//--------------------------------------------------------------//
PixilMainWnd::PixilMainWnd(int argc, char **argv)
    :
Fl_Window(MAIN_WND_WIDTH, MAIN_WND_HEIGHT, _("Pixil Desktop"))
{
    g_MainWnd = this;
    bool bTimedOut;
    PixilDTMessage nMessage;

    // Set that the original cursor is not known
    m_nCursor = (Fl_Cursor) (-1);

    // Show the splash window
    end();
    SplashDlg *pDlg = new SplashDlg(2.5, bTimedOut);
    begin();

    // Set up the Images object
    new Images;

    // Wait for the splash dialog before possibly queying the user on the location of the INI file
    while (bTimedOut == false)	// Wait for the splash window
    {
	Fl::check();
    }

    // Set op the Options object
    new Options;

    // Set the window icon
    PixilDT::GetApp()->SetPixilIcon(this);

    // Create the menu for the window
    Fl_Menu_Bar *pMenuBar =
	new Fl_Menu_Bar(0, 0, MAIN_WND_WIDTH, MENU_HEIGHT);
    m_pTranslatedMenuItems = TranslateMenuItems(m_MenuItems);

    // Temporary code until other functions are written
    //    EnableMenuItem(_("&File"),_("&Save All"),false,true);
    EnableMenuItem(_("&File"), _("Page Set&up"), false, true);
    EnableMenuItem(_("&Tools"), _("&Purge Events"), false, true);
    EnableMenuItem(_("&Tools"), _("P&urge Completed ToDo's"), false, true);
    //    EnableMenuItem(_("&Sync"),_("&Custom"),false,true);
    //    EnableMenuItem(_("&Sync"),_("&File Link"),false,true);
    //    EnableMenuItem(_("&Sync"),_("&View Log"),false,true);
    //    EnableMenuItem(_("&Sync"),_("&Setup"),false,true);
    EnableMenuItem(_("&Help"), _("&Quick Tour"), false, true);
    EnableMenuItem(_("&Help"), _("&Online Support"), false, true);
    EnableMenuItem(_("&Help"), _("&Pixil on the Web"), false, true);

    pMenuBar->menu(m_pTranslatedMenuItems);

    // Set the minimum size if needed
    size_range(MAIN_WND_WIDTH - 50, MAIN_WND_HEIGHT - 50);

    // Create the toolbar
    m_pToolbar =
	new Toolbar(0, MENU_HEIGHT, MAIN_WND_WIDTH, TOOLBAR_HEIGHT,
		    m_ToolbarItem);

    // Temporary code until other functionality is written
    //    m_pToolbar->Enable(Images::FILESAVE_ICON,false);

    // Add child widgets to the window (new widgets are added by default)
    m_pLeftGroup = new LeftGroup(0,
				 MENU_HEIGHT + TOOLBAR_HEIGHT,
				 LEFT_WIDTH,
				 MAIN_WND_HEIGHT - MENU_HEIGHT -
				 TOOLBAR_HEIGHT);
    m_pInfoGroup =
	new InfoGroup(LEFT_WIDTH, MENU_HEIGHT + TOOLBAR_HEIGHT,
		      MAIN_WND_WIDTH - LEFT_WIDTH,
		      MAIN_WND_HEIGHT - MENU_HEIGHT - TOOLBAR_HEIGHT);
    end();

    // Set up a custom callback to aid in closing the window
    callback(Callback);

    // Set that the window is resizable
    resizable(m_pInfoGroup);

    // Start with the page last used
    switch (Options::GetMainPage()) {
    case 1:
	nMessage = ADDRESS_BOOK_REQUESTED;
	break;

    case 2:
	nMessage = NOTES_REQUESTED;
	break;

    case 3:
	nMessage = TODO_LIST_REQUESTED;
	break;

    default:			// Includes a normal 0
	nMessage = SCHEDULER_REQUESTED;
    }
    Notify(nMessage, 0);

    // Show the window
    while (bTimedOut == false)	// Wait for the splash window
    {
	Fl::check();
    }
    delete pDlg;

    // Show the window
    show(argc, argv);
}


//--------------------------------------------------------------//
// Destructor, close all open databases.                        //
//--------------------------------------------------------------//
PixilMainWnd::~PixilMainWnd()
{
#ifdef DEBUG
    NxDbAccess::DumpAll();
#endif

#ifndef WIN32
    fl_delete_offscreen((Fl_Offscreen) icon());
#endif

    // Close all data base files
    NxDbAccess::CloseAll();

    // Clean up the translated items
    FreeTranslatedMenu(m_pTranslatedMenuItems);

    // Clean up any images
    Images::Destroy();

    // Write the INI file back to disk
    Options::Destroy();
}


//--------------------------------------------------------------//
// Process a window closing event.                              //
//--------------------------------------------------------------//
void
PixilMainWnd::Callback(Fl_Widget * pWidget, void *pData)
{
    // Test whether a selection change is OK or not
    if (((PixilMainWnd *) pWidget)->m_pInfoGroup->
	Message(SELECTION_CHANGING, 0) == 1) {
	// Its OK, close the application
	// Notify the InfoGroup object that the application is closing
	((PixilMainWnd *) pWidget)->m_pInfoGroup->Message(APPLICATION_CLOSING,
							  0);

	// Perform the FLTK processing - close the application
	default_callback((Fl_Window *) pWidget, pData);
    }
}


//--------------------------------------------------------------//
// Paste a previously cut/copied item.                          //
//--------------------------------------------------------------//
void
PixilMainWnd::EditPaste(Fl_Widget * pWidget, void *pUserData)
{
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    pThis->m_pInfoGroup->Message(EDIT_PASTE, 0);
}


//--------------------------------------------------------------//
// Undo a previous cut/copy/past operation.                     //
//--------------------------------------------------------------//
void
PixilMainWnd::EditUndo(Fl_Widget * pWidget, void *pUserData)
{
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    pThis->m_pInfoGroup->Message(EDIT_UNDO, 0);
    pThis->FixMenu();
}


//--------------------------------------------------------------//
// Enable/Disable a menu item.                                  //
//--------------------------------------------------------------//
void
PixilMainWnd::EnableMenuItem(const char *pszTopMenu,
			     const char *pszSubMenu,
			     bool bEnable, bool bVisible)
{
    int nMenuItem;
    int nMenuItem2;
    int nNest;

    // Find this menu option
    nNest = 0;
    for (nMenuItem = 0;
	 m_pTranslatedMenuItems[nMenuItem].text != NULL || nNest > 0;
	 ++nMenuItem) {
	if (m_pTranslatedMenuItems[nMenuItem].text == NULL) {
	    // End of a submenu
	    --nNest;
	} else {
	    // This is not a NULL - End of SubMenu item

	    // Is this the top level menu item to search for
	    if (nNest == 0
		&& strcmp(pszTopMenu,
			  m_pTranslatedMenuItems[nMenuItem].text) == 0) {
		// This is the correct menu
		// Search for items below this one
		for (nMenuItem2 = nMenuItem + 1;
		     m_pTranslatedMenuItems[nMenuItem2].text != NULL;
		     ++nMenuItem2) {
		    if (strcmp
			(pszSubMenu,
			 m_pTranslatedMenuItems[nMenuItem2].text) == 0) {
			// This is the menu item to change
			if (bVisible == true) {
			    // Show the item
			    m_pTranslatedMenuItems[nMenuItem2].flags &=
				~FL_MENU_INVISIBLE;

			    // Should it be enabled or not
			    if (bEnable == true) {
				// Show the menu item
				m_pTranslatedMenuItems[nMenuItem2].flags &=
				    ~FL_MENU_INACTIVE;
			    } else {
				// Hide the menu item
				m_pTranslatedMenuItems[nMenuItem2].flags |=
				    FL_MENU_INACTIVE;
			    }
			} else {
			    // Hide the menu item (and make it inactive)
			    m_pTranslatedMenuItems[nMenuItem2].flags |=
				FL_MENU_INVISIBLE | FL_MENU_INACTIVE;
			}
			break;	// Out of the inner for loop
		    }
		}

#ifdef DEBUG
		// Was the second level menu item found
		if (m_pTranslatedMenuItems[nMenuItem2].text == NULL) {
		    assert(false);	// Second level menu item not found
		}
#endif
		// Get out of the outer for loop
		break;
	    } else		// Not the correct first level menu item
	    {
		// Bump the nesting level if needed
		if ((m_pTranslatedMenuItems[nMenuItem].flags & FL_SUBMENU) !=
		    0) {
		    ++nNest;
		}
	    }
	}
    }

#ifdef DEBUG
    // Was the first level menu item found ?
    if (m_pTranslatedMenuItems[nMenuItem].text == NULL) {
	assert(false);		// First level menu item not found
    }
#endif
}


//--------------------------------------------------------------//
// Copy the currently selected item.                            //
//--------------------------------------------------------------//
void
PixilMainWnd::EditCopy(Fl_Widget * pWidget, void *pUserData)
{
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    pThis->m_pInfoGroup->Message(EDIT_COPY, 0);
    pThis->FixMenu();
}


//--------------------------------------------------------------//
// Cut the currently selected item.                             //
//--------------------------------------------------------------//
void
PixilMainWnd::EditCut(Fl_Widget * pWidget, void *pUserData)
{
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    pThis->m_pInfoGroup->Message(EDIT_CUT, 0);
    pThis->FixMenu();
}


//--------------------------------------------------------------//
// Delete the current item based on what type of info is being  //
// displayed                                                    //
//--------------------------------------------------------------//
void
PixilMainWnd::EditDelete(Fl_Widget * pWidget, void *pUserData)
{
    PixilDTMessage nMessage;
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    switch (pThis->m_nCurrentDisplay) {
    case ADDRESS_BOOK_REQUESTED:
	nMessage = ADDRESS_BOOK_DELETE;
	break;

    case NOTES_REQUESTED:
	nMessage = NOTES_DELETE;
	break;

    case SCHEDULER_REQUESTED:
	nMessage = SCHEDULER_DELETE;
	break;

    case TODO_LIST_REQUESTED:
	nMessage = TODO_LIST_DELETE;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown display type
#endif
	;
    }

    pThis->m_pInfoGroup->Message(nMessage, 0);
}


//--------------------------------------------------------------//
// Process the Edit/Find menu selection                         //
//--------------------------------------------------------------//
void
PixilMainWnd::EditFind(Fl_Widget * pWidget, void *pUserData)
{
    FindDlg *pDlg = new FindDlg(pWidget->parent());

    if (pDlg->DoModal() == 1) {
	int nAction = pDlg->GetAction();
	int nRecno = pDlg->GetRecno();
	PixilDTMessage nType = pDlg->GetType();

	// Remove the dialog from the screen
	delete pDlg;

	// Now perform the action
	if (nAction == FindDlg::ActionGoTo) {
	    PixilDT::GetApp()->GetMainWindow()->Notify(nType, nRecno);
	}
    } else {
	// Remove the dialog from the screen
	delete pDlg;
    }
}


//--------------------------------------------------------------//
// Add a new item based on what type of info is being displayed //
//--------------------------------------------------------------//
void
PixilMainWnd::EditNew(Fl_Widget * pWidget, void *pUserData)
{
    PixilDTMessage nMessage;
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    switch (pThis->m_nCurrentDisplay) {
    case ADDRESS_BOOK_REQUESTED:
	nMessage = ADDRESS_BOOK_NEW;
	break;

    case NOTES_REQUESTED:
	nMessage = NOTES_NEW;
	break;

    case SCHEDULER_REQUESTED:
	nMessage = SCHEDULER_NEW;
	break;

    case TODO_LIST_REQUESTED:
	nMessage = TODO_LIST_NEW;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown display type
#endif
	;
    }

    pThis->m_pInfoGroup->Message(nMessage, 0);
}


//--------------------------------------------------------------//
// File/Exit menu selection                                     //
//--------------------------------------------------------------//
void
PixilMainWnd::FileExit(Fl_Widget * pWidget, void *pUserData)
{
    // Do this window's callback, the window will close
    pWidget->parent()->do_callback(pWidget->parent(), pUserData);
}


//--------------------------------------------------------------//
// File/Print menu selection.                                   //
//--------------------------------------------------------------//
void
PixilMainWnd::FilePrint(Fl_Widget * pWidget, void *pData)
{
    PixilMainWnd *pThis = (PixilMainWnd *) (pWidget->parent());

    switch (pThis->m_nCurrentDisplay) {
    case ADDRESS_BOOK_REQUESTED:
	pThis->m_pInfoGroup->Message(ADDRESS_BOOK_PRINT, 0);
	break;

    case NOTES_REQUESTED:
	pThis->m_pInfoGroup->Message(NOTES_PRINT, 0);
	break;

    case SCHEDULER_REQUESTED:
	pThis->m_pInfoGroup->Message(SCHEDULER_PRINT, 0);
	break;

    case TODO_LIST_REQUESTED:
	pThis->m_pInfoGroup->Message(TODO_LIST_PRINT, 0);
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown page
#endif
	;
    }
}


//--------------------------------------------------------------//
// Fix the menu items based on what is being currently          //
// displayed.                                                   //
//--------------------------------------------------------------//
void
PixilMainWnd::FixMenu()
{
    bool bEnable;
    const char *pszMenu[] =	// These must match text in the menu (must be translated here)
    {
	_("&View"),
	_("&Tools"),
	_("Show &Private Records"),
	_("&Hide Private Records"),
	_("&List By"),
	_("S&how..."),
	_("Custom &Fields"),
	_("&Purge Events"),
	_("P&urge Completed ToDo's"),
    };
    int i;
    int nIndex;
    static const int nMatrix[7][6] = {
	// Contents:
	//  Menu selection 1 in pszMenu
	//  Menu selection 2 in pszMenu
	//  Hide/show for Address Book
	//  Hide/show for Notes
	//  Hide/show for Scheduler
	//  Hide/show for ToDo List
	{0, 2, 0, 0, 0, 0},	// {0,2,0,1,1,1}, // Don't show "hide/show" private records
	{0, 3, 0, 0, 0, 0},	// {0,3,0,1,1,1}, // Don't show "hide/show" private records
	{0, 4, 1, 0, 0, 0},
	{0, 5, 0, 0, 0, 1},
	{1, 6, 1, 0, 0, 0},
	{1, 7, 0, 0, 0, 0},	// {1,7,0,0,1,0}, // Changed for no purging allowed yet
	{1, 8, 0, 0, 0, 0},	// {1,8,0,0,0,1}, // Changed for no purging allowed yet
    };

    // Enable and disable menu items with static status
    switch (m_nCurrentDisplay) {
    case ADDRESS_BOOK_REQUESTED:
	// Hide the "View/Show Private Records" selection
	// Hide the "View/Hide Private Records" selection
	// Show the "View/List By" selection
	// Hide the "View/Show" selection
	// Show the "Tools/Custom Fields" selection
	// Hide the "Tools/Purge Events" selection
	// Hide the "Tools/Purge Completed ToDo's" selection
	nIndex = 0;
	break;

    case NOTES_REQUESTED:
	// Show the "View/Show Private Records" selection
	// Show the "View/Hide Private Records" selection
	// Show the "View/List By" selection
	// Hide the "View/Show" selection
	// Hide the "Tools/Custom Fields" selection
	// Hide the "Tools/Purge Events" selection
	// Hide the "Tools/Purge Completed ToDo's" selection
	nIndex = 1;
	break;

    case SCHEDULER_REQUESTED:
	// Show the "View/Show Private Records" selection
	// Show the "View/Hide Private Records" selection
	// Hide the "View/List By" selection
	// Hide the "View/Show" selection
	// Hide the "Tools/Custom Fields" selection
	// Show the "Tools/Purge Events" selection (temp hidden for no purging)
	// Hide the "Tools/Purge Completed ToDo's" selection
	nIndex = 2;
	break;

    case TODO_LIST_REQUESTED:
	// Show the "View/Show Private Records" selection
	// Show the "View/Hide Private Records" selection
	// Hide the "View/List By" selection
	// Show the "View/Show" selection
	// Hide the "Tools/Custom Fields" selection
	// Hide the "Tools/Purge Events" selection
	// Show the "Tools/Purge Completed ToDo's" selection (temp hidden for no purging)
	nIndex = 3;
	break;

#ifdef DEBUG
    default:
	assert(false);		// Unknown display type
#endif
    }

    // Now reconfigure the menu using these settings
    for (i = 0; i < 7; ++i) {
	// Go hide/show each menu item in question
	EnableMenuItem(pszMenu[nMatrix[i][0]],
		       pszMenu[nMatrix[i][1]],
		       nMatrix[i][nIndex + 2] == 1,
		       nMatrix[i][nIndex + 2] == 1);
    }

    // Now enable and disable menu/toolbar items with dynamic status
    bEnable = (m_pInfoGroup->Message(EDIT_CUT_AVAILABLE, 0) == 1);
    EnableMenuItem(_("&Edit"), _("Cu&t"), bEnable, true);
    m_pToolbar->Enable(Images::EDITCUT_ICON, bEnable);
    bEnable = (m_pInfoGroup->Message(EDIT_COPY_AVAILABLE, 0) == 1);
    EnableMenuItem(_("&Edit"), _("&Copy"), bEnable, true);
    m_pToolbar->Enable(Images::EDITCOPY_ICON, bEnable);
    bEnable = (m_pInfoGroup->Message(EDIT_PASTE_AVAILABLE, 0) == 1);
    EnableMenuItem(_("&Edit"), _("&Paste"), bEnable, true);
    m_pToolbar->Enable(Images::EDITPASTE_ICON, bEnable);
    bEnable = (m_pInfoGroup->Message(EDIT_UNDO_AVAILABLE, 0) == 1);
    EnableMenuItem(_("&Edit"), _("&Undo"), bEnable, true);
    m_pToolbar->Enable(Images::EDITUNDO_ICON, bEnable);
}


//--------------------------------------------------------------//
// Event handler, keep the ESC key from closing the window.     //
//--------------------------------------------------------------//
int
PixilMainWnd::handle(int nEvent)
{
    int nReturn;

    if (nEvent == FL_KEYBOARD && Fl::event_key() == FL_Escape) {
	nReturn = 1;
    } else {
	nReturn = Fl_Window::handle(nEvent);
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Context sensitive help                                       //
//--------------------------------------------------------------//
void
PixilMainWnd::HelpAbout(Fl_Widget * pWidget, void *pUserData)
{
    Fl_Button *pButton;
    Fl_Browser *pBrowser;
    Fl_Window *pAbout = new Fl_Window(300, 150, _("About PixilDT"));

    pBrowser = new Fl_Browser(DLG_BORDER,
			      DLG_BORDER,
			      pAbout->w() - 2 * DLG_BORDER,
			      pAbout->h() - DLG_BUTTON_HEIGHT -
			      2 * DLG_BORDER);
    pBrowser->box(FL_FLAT_BOX);
    pBrowser->color(FL_GRAY);
    pBrowser->add(_("Pixil Desktop"));
    pBrowser->add("");		// Documentation is incorrect, NULL will not add a line
    pBrowser->add(_("Version: 0.10"));
    pBrowser->add(_("Copyright 2001 Century Software"));

    pButton =
	new Fl_Return_Button(pAbout->w() - DLG_BUTTON_WIDTH - DLG_BORDER,
			     pAbout->h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
			     DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, fl_ok);

    pAbout->end();
    ::DoModal(pAbout, pButton, NULL);
    delete pAbout;
}


//--------------------------------------------------------------//
// Context sensitive help                                       //
//--------------------------------------------------------------//
void
PixilMainWnd::HelpHelp(Fl_Widget * pWidget, void *pUserData)
{
#ifndef WIN32

    // Unix/Linux - no help yet
    fl_alert(_("Contextual Help is not yet implemented"));

#else

    // MS/Windows - no help yet
    fl_alert(_("Contextual Help is not yet implemented"));

#endif
}


//--------------------------------------------------------------//
// Process a notification from a child widget.                  //
//--------------------------------------------------------------//
void
PixilMainWnd::Notify(PixilDTMessage nMessage, int nInfo)
{
    switch (nMessage) {
    case ADDRESS_BOOK_CHANGED:
    case BEGIN_WEEK_CHANGED:	// Beginning day of week has changed
    case NOTES_CHANGED:
    case SCHEDULER_CHANGED:
    case TODO_LIST_CHANGED:
	// Send these messages back down to the center and right portions of the window
	m_pInfoGroup->Message(nMessage, nInfo);

	// Then fix the menu and toolbar
	FixMenu();
	break;

    case ADDRESS_BOOK_GOTO:
    case NOTES_GOTO:
    case SCHEDULER_GOTO:
    case TODO_LIST_GOTO:
	if (SetCurrentDisplay(nMessage) == 1) {
	    m_pInfoGroup->Message(nMessage, nInfo);
	}
	break;

    case ADDRESS_BOOK_REQUESTED:
    case NOTES_REQUESTED:
    case SCHEDULER_REQUESTED:
    case TODO_LIST_REQUESTED:
	SetCurrentDisplay(nMessage);
	break;

    case CATEGORY_ADDED:
	switch (m_nCurrentDisplay) {
	case ADDRESS_BOOK_REQUESTED:
	    m_pInfoGroup->Message(ADDRESS_BOOK_CATEGORY_ADDED, nInfo);
	    break;

	case NOTES_REQUESTED:
	    m_pInfoGroup->Message(NOTES_CATEGORY_ADDED, nInfo);
	    break;

	case SCHEDULER_REQUESTED:
	    m_pInfoGroup->Message(SCHEDULER_CATEGORY_ADDED, nInfo);
	    break;

	case TODO_LIST_REQUESTED:
	    m_pInfoGroup->Message(TODO_LIST_CATEGORY_ADDED, nInfo);
	}
	break;

    case CATEGORY_DELETED:
	switch (m_nCurrentDisplay) {
	case ADDRESS_BOOK_REQUESTED:
	    m_pInfoGroup->Message(ADDRESS_BOOK_CATEGORY_DELETED, nInfo);
	    break;

	case NOTES_REQUESTED:
	    m_pInfoGroup->Message(NOTES_CATEGORY_DELETED, nInfo);
	    break;

	case SCHEDULER_REQUESTED:
	    m_pInfoGroup->Message(SCHEDULER_CATEGORY_DELETED, nInfo);
	    break;

	case TODO_LIST_REQUESTED:
	    m_pInfoGroup->Message(TODO_LIST_CATEGORY_DELETED, nInfo);
	}
	break;

    case CATEGORY_RENAMED:
	switch (m_nCurrentDisplay) {
	case ADDRESS_BOOK_REQUESTED:
	    m_pInfoGroup->Message(ADDRESS_BOOK_CATEGORY_RENAMED, nInfo);
	    break;

	case NOTES_REQUESTED:
	    m_pInfoGroup->Message(NOTES_CATEGORY_RENAMED, nInfo);
	    break;

	case SCHEDULER_REQUESTED:
	    m_pInfoGroup->Message(SCHEDULER_CATEGORY_RENAMED, nInfo);
	    break;

	case TODO_LIST_REQUESTED:
	    m_pInfoGroup->Message(TODO_LIST_CATEGORY_RENAMED, nInfo);
	}
	break;

    case ENABLE_TOOLBAR_BUTTON:	// Enable a toolbar button
	m_pToolbar->Enable(nInfo, true);
	break;

    case DISABLE_TOOLBAR_BUTTON:	// Disable a toolbar button
	m_pToolbar->Enable(nInfo, false);
	break;

    case FIX_MENU:		// Fix the menu and toolbar for the current status
	FixMenu();
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown message
#endif
	;
    }
}


//--------------------------------------------------------------//
// Set the current display.                                     //
//--------------------------------------------------------------//
int
PixilMainWnd::SetCurrentDisplay(PixilDTMessage nMessage)
{
    int nReturn;

    // Notify everyone that the selection is changing
    // Refuse the change if the return is 0
    nReturn = m_pInfoGroup->Message(SELECTION_CHANGING, 0);
    if (nReturn == 1) {
	// Set the type of display to use
	switch (nMessage) {
	case ADDRESS_BOOK_REQUESTED:
	case ADDRESS_BOOK_GOTO:
	    nMessage = ADDRESS_BOOK_REQUESTED;
	    break;

	case NOTES_REQUESTED:
	case NOTES_GOTO:
	    nMessage = NOTES_REQUESTED;
	    break;

	case SCHEDULER_REQUESTED:
	case SCHEDULER_GOTO:
	    nMessage = SCHEDULER_REQUESTED;
	    break;

	case TODO_LIST_REQUESTED:
	case TODO_LIST_GOTO:
	    nMessage = TODO_LIST_REQUESTED;
	    break;

	default:
#ifdef DEBUG
	    assert(false);	// Unknown message
#endif
	    ;
	}

	// Save the type of info being displayed
	m_nCurrentDisplay = nMessage;

	// Notify the left side buttons of the selection
	m_pLeftGroup->Message(nMessage, 0);

	// Send these messages back down to the center and right portions of the window
	m_pInfoGroup->Message(nMessage, 0);

	// Fix the current menu items
	FixMenu();
    } else {
	// Reset the left side buttons if the request has been cancelled
	m_pLeftGroup->Message(m_nCurrentDisplay, 0);
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Process the Tool/Categories selection from the main menu.    //
//--------------------------------------------------------------//
void
PixilMainWnd::ToolsCategories(Fl_Widget * pWidget, void *pData)
{
    CategoryEditor *pDlg;
    CategoryEditor::Type nType;
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    switch (pThis->m_nCurrentDisplay) {
    case ADDRESS_BOOK_REQUESTED:
	nType = CategoryEditor::AddressBook;
	break;

    case NOTES_REQUESTED:
	nType = CategoryEditor::Notes;
	break;

    case SCHEDULER_REQUESTED:
	nType = CategoryEditor::Scheduler;
	break;

    case TODO_LIST_REQUESTED:
	nType = CategoryEditor::ToDoList;
	break;

    default:
#ifdef DEBUG
	assert(false);		// Unknown display type
#endif
	;
    }

    // Invoke the Category Editor for that set of categories
    pDlg = new CategoryEditor(nType, pThis);
    delete pDlg;
}


//--------------------------------------------------------------//
// Process the Tool/Custom Fields selection from the main menu. //
//--------------------------------------------------------------//
void
PixilMainWnd::ToolsCustomFields(Fl_Widget * pWidget, void *pData)
{
    CustomFieldEditor *pDlg;
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    // Invoke the Custom Field Editor
    pDlg = new CustomFieldEditor(pThis);
    delete pDlg;
}


//--------------------------------------------------------------//
// Process the Tool/Options selection from the main menu.       //
//--------------------------------------------------------------//
void
PixilMainWnd::ToolsOptions(Fl_Widget * pWidget, void *pData)
{
    OptionsDlg *pDlg;
    PixilMainWnd *pThis = PixilDT::GetApp()->GetMainWindow();

    // Invoke the global options dialog
    pDlg = new OptionsDlg(pThis);
    pDlg->DoModal();
    delete pDlg;
}


//--------------------------------------------------------------//
// View/Address Book menu selection                             //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewAddressBook(Fl_Widget * pWidget, void *pData)
{
    ((PixilMainWnd *) (pWidget->parent()))->
	SetCurrentDisplay(ADDRESS_BOOK_REQUESTED);
}


//--------------------------------------------------------------//
// View/List By menu selection                                  //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewListBy(Fl_Widget * pWidget, void *pData)
{
    // Send a message that the List By dialog is needed
    ((PixilMainWnd *) (pWidget->parent()))->m_pInfoGroup->
	Message(SHOW_LIST_BY, 0);
}


//--------------------------------------------------------------//
// View/Notes menu selection                                    //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewNotes(Fl_Widget * pWidget, void *pData)
{
    ((PixilMainWnd *) (pWidget->parent()))->
	SetCurrentDisplay(NOTES_REQUESTED);
}


//--------------------------------------------------------------//
// View/Scheduler menu selection                                //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewScheduler(Fl_Widget * pWidget, void *pData)
{
    ((PixilMainWnd *) (pWidget->parent()))->
	SetCurrentDisplay(SCHEDULER_REQUESTED);
}


//--------------------------------------------------------------//
// View/Show menu selection (ToDo List show options)            //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewShow(Fl_Widget * pWidget, void *pData)
{
    // Send a message that the Show dialog is needed
    ((PixilMainWnd *) (pWidget->parent()))->m_pInfoGroup->
	Message(SHOW_TODO_OPTIONS, 0);
}


//--------------------------------------------------------------//
// View/ToDo List menu selection                                //
//--------------------------------------------------------------//
void
PixilMainWnd::ViewTodoList(Fl_Widget * pWidget, void *pData)
{
    ((PixilMainWnd *) (pWidget->parent()))->
	SetCurrentDisplay(TODO_LIST_REQUESTED);
}


//--------------------------------------------------------------//
// The main window menu                                         //
//--------------------------------------------------------------//
const Fl_Menu_Item
    PixilMainWnd::m_MenuItems[] = {
    {N_("&File"), 0, 0, 0, FL_SUBMENU},
//              {N_("&Save All"),               0, PixilMainWnd::FileSaveAll},
    {N_("Page Set&up"), 0, PixilMainWnd::FilePageSetup},
    {N_("&Print"), 0, PixilMainWnd::FilePrint},
    {N_("E&xit"), 0, PixilMainWnd::FileExit},
    {NULL},
    {N_("&Edit"), 0, 0, 0, FL_SUBMENU},
    {N_("&Undo"), 0, PixilMainWnd::EditUndo},
    {N_("Cu&t"), 0, PixilMainWnd::EditCut},
    {N_("&Copy"), 0, PixilMainWnd::EditCopy},
    {N_("&Paste"), 0, PixilMainWnd::EditPaste},
    {N_("&Delete"), 0, PixilMainWnd::EditDelete},
    {N_("&New"), 0, PixilMainWnd::EditNew},
    {N_("&Find..."), 0, PixilMainWnd::EditFind},
    {NULL},
    {N_("&View"), 0, 0, 0, FL_SUBMENU},
    {N_("&Scheduler"), 0, PixilMainWnd::ViewScheduler},
    {N_("&Address Book"), 0, PixilMainWnd::ViewAddressBook},
    {N_("&Todo List"), 0, PixilMainWnd::ViewTodoList},
    {N_("&Notes"), 0, PixilMainWnd::ViewNotes},
    {N_("Show &Private Records"), 0, PixilMainWnd::ViewShowPrivate},
    {N_("&Hide Private Records"), 0, PixilMainWnd::ViewHidePrivate},
    {N_("&List By"), 0, PixilMainWnd::ViewListBy},
    {N_("S&how..."), 0, PixilMainWnd::ViewShow},
    {NULL},
    {N_("&Tools"), 0, 0, 0, FL_SUBMENU},
    {N_("&Categories"), 0, PixilMainWnd::ToolsCategories},
    {N_("Custom &Fields"), 0, PixilMainWnd::ToolsCustomFields},
    {N_("&Purge Events"), 0, PixilMainWnd::ToolsPurgeEvents},
    {N_("P&urge Completed ToDo's"), 0,
     PixilMainWnd::ToolsPurgeCompletedToDos},
    {N_("&Options"), 0, PixilMainWnd::ToolsOptions},
    {NULL},
    {N_("&Sync"), 0, 0, 0, FL_SUBMENU},
    {N_("&Merge"), 0, PixilMainWnd::SyncMerge},
    {N_("Overwrite &PDA"), 0, PixilMainWnd::SyncPush},
    {N_("Overwrite &Desktop"), 0, PixilMainWnd::SyncPull},
    //              {N_("&Setup"),                  0, PixilMainWnd::SyncSetup},
    {NULL},
    {N_("&Help"), 0, 0, 0, FL_SUBMENU},
    {N_("Pixil Desktop Help"), 0, PixilMainWnd::HelpHelp},
    {N_("&Quick Tour"), 0, PixilMainWnd::HelpQuickTour},
    {N_("&Online Support"), 0, PixilMainWnd::HelpOnlineSupport},
    {N_("&Pixil on the Web"), 0, PixilMainWnd::HelpPixilOnWeb},
    {N_("&About Pixil Desktop"), 0, PixilMainWnd::HelpAbout},
    {NULL},
    {NULL},
};


//--------------------------------------------------------------//
// The toolbar definition structure.                            //
//--------------------------------------------------------------//
const ToolbarItem
    PixilMainWnd::m_ToolbarItem[] = {
//      {TOOLBAR_BUTTON, N_("Save All"), FileSaveAll, Images::FILESAVE_ICON,  Images::DISABLED_SAVE_ICON},
    {TOOLBAR_BUTTON, N_("Print"), FilePrint, Images::FILEPRINT_ICON,
     Images::DISABLED_PRINT_ICON},
    {TOOLBAR_GAP, NULL, NULL, 0, 0},
    {TOOLBAR_BUTTON, N_("Cut"), EditCut, Images::EDITCUT_ICON,
     Images::DISABLED_CUT_ICON},
    {TOOLBAR_BUTTON, N_("Copy"), EditCopy, Images::EDITCOPY_ICON,
     Images::DISABLED_COPY_ICON},
    {TOOLBAR_BUTTON, N_("Paste"), EditPaste, Images::EDITPASTE_ICON,
     Images::DISABLED_PASTE_ICON},
    {TOOLBAR_BUTTON, N_("Undo"), EditUndo, Images::EDITUNDO_ICON,
     Images::DISABLED_UNDO_ICON},
    {TOOLBAR_GAP, NULL, NULL, 0, 0},
    {TOOLBAR_BUTTON, N_("Find"), EditFind, Images::FIND_ICON, -1},
    {TOOLBAR_GAP, NULL, NULL, 0, 0},
    {TOOLBAR_BUTTON, N_("New Item"), EditNew, Images::NEWITEM_ICON, -1},
    {TOOLBAR_GAP, NULL, NULL, 0, 0},
    {TOOLBAR_BUTTON, N_("Help"), HelpHelp, Images::CURHELP_ICON, -1},
    {TOOLBAR_END, NULL, NULL, 0, 0},
};

/** 
 * @brief Perform a merge operation.
 * 
 * @param pWidget Unused.
 * @param pData Unused.
 */
void
PixilMainWnd::SyncMerge(Fl_Widget * pWidget, void *pData)
{
    char *pixil_bin = getenv("PIXIL_BIN");
    char *pixil_data = getenv("PIXIL_DATA");
    char cmd_line[4096];

    if (!pixil_bin || !pixil_data)
	return;

    sprintf(cmd_line, "%s/pull.sh", pixil_bin);
#ifdef DEBUG
    printf("Running [%s]\n", cmd_line);
#endif
    execl(cmd_line, cmd_line, 0);
}

void
PixilMainWnd::SyncPush(Fl_Widget * pWidget, void *pData)
{
    char *pixil_bin = getenv("PIXIL_BIN");
    char *pixil_data = getenv("PIXIL_DATA");
    char cmd_line[4096];

    if (!pixil_bin || !pixil_data)
	return;

    sprintf(cmd_line, "%s/push.sh", pixil_bin);
    int pid = vfork();

    if (!pid) {
	execl(cmd_line, cmd_line, 0);
    } else {
	wait(NULL);
	g_MainWnd->m_pInfoGroup->Refresh();
    }

}

void
PixilMainWnd::SyncPull(Fl_Widget * pWidget, void *pData)
{
    //  g_MainWnd->m_pInfoGroup->Refresh();
    //  return;
    char *pixil_bin = getenv("PIXIL_BIN");
    char *pixil_data = getenv("PIXIL_DATA");
    char cmd_line[4096];

    if (!pixil_bin || !pixil_data)
	return;

    sprintf(cmd_line, "%s/pull.sh", pixil_bin);

    int pid = vfork();

    if (!pid) {
	execl(cmd_line, cmd_line, 0);
    } else {
	wait(NULL);
	g_MainWnd->m_pInfoGroup->Refresh();
    }

}
