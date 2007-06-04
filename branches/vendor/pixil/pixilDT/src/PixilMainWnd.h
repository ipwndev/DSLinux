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
#ifndef PIXILMAINWND_H_

#define PIXILMAINWND_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>
#include "InfoGroup.h"
#include "LeftGroup.h"
#include "Toolbar.h"
class PixilMainWnd:public Fl_Window
{
  public:PixilMainWnd(int argc,
		 // Constructor
		 char **argv);
     ~PixilMainWnd();		// Destructor
    void Notify(PixilDTMessage nMessage,	// Process a notification from a child widget
		int nInfo);
    inline void ResetCursor()	// Set the cursor back to its original value
    {
	cursor(FL_CURSOR_DEFAULT);
    }
    inline void SetCursor(Fl_Cursor nCursor)	// Set the cursor for this window
    {
	cursor(nCursor);
    }
  protected:int handle(int nEvent);
    // Event handler, keep escape key from closing this window
  private:Fl_Cursor m_nCursor;
    // The original cursor for this window
    static const Fl_Menu_Item m_MenuItems[];	// Menu definition
    Fl_Menu_Item *m_pTranslatedMenuItems;	// Translated menu definition
    InfoGroup *m_pInfoGroup;	// Main sub-window
    LeftGroup *m_pLeftGroup;	// Group for the left of the main window
    PixilDTMessage m_nCurrentDisplay;	// Type of info currently being displayed
    Toolbar *m_pToolbar;	// Toolbar for the window
    static const ToolbarItem m_ToolbarItem[];	// Toolbar definitions
    static void Callback(Fl_Widget * pWidget, void *pData);	// Used to close the program nicely
    static void EditCopy(Fl_Widget * pWidget, void *pData);	// Edit/Copy menu selection
    static void EditCut(Fl_Widget * pWidget, void *pData);	// Edit/Cut menu selection
    static void EditDelete(Fl_Widget * pWidget, void *pData);	// Edit/Delete menu selection
    static void EditFind(Fl_Widget * pWidget, void *pData);	// Edit/Find menu selection
    static void EditNew(Fl_Widget * pWidget, void *pData);	// Edit/New menu selection
    static void EditPaste(Fl_Widget * pWidget, void *pData);	// Edit/Paste menu selection
    static void EditUndo(Fl_Widget * pWidget, void *pData);	// Edit/Undo menu selection
    void EnableMenuItem(const char *pszTopMenu,	// Enable/Disable Show/Hide a menu item
			const char *pszSubMenu, bool bEnable, bool bVisible);
    static void FileExit(Fl_Widget * pWidget, void *pData);	// File/Exit menu selection
    static void FilePageSetup(Fl_Widget * pWidget, void *pData)	// File/Page Setup menu selection
    {
    }
    static void FilePrint(Fl_Widget * pWidget, void *pData);	// File/Print menu selection
    static void FileSaveAll(Fl_Widget * pWidget, void *pData)	// File/Save All menu selection
    {
    }
    void FixMenu();		// Enable/disable menu selection based on current display
    static void HelpAbout(Fl_Widget * pWidget, void *pData);	// Help/About Pixil Desktop menu selection
    static void HelpHelp(Fl_Widget * pWidget, void *pData);	// Help/PixilDesktopHelp menu selection
    static void HelpOnlineSupport(Fl_Widget * pWidget, void *pData)	// Help/Online Support menu selection
    {
    }
    static void HelpQuickTour(Fl_Widget * pWidget, void *pData)	// Help/QuickTour menu selection
    {
    }
    static void HelpPixilOnWeb(Fl_Widget * pWidget, void *pData)	// Help/Pixil on the Web menu selection
    {
    }
    int SetCurrentDisplay(PixilDTMessage nMessage);	// Set the current display type
    static void SyncPush(Fl_Widget * pWidget, void *pData);
    static void SyncPull(Fl_Widget * pWidget, void *pData);
    static void SyncMerge(Fl_Widget * pWidget, void *pData);
    static void SyncSetup(Fl_Widget * pWidget, void *pData)	// Sync/Setup menu selection
    {
    }
    static void ToolsCategories(Fl_Widget * pWidget, void *pData);	// Tools/Categories menu selection
    static void ToolsCustomFields(Fl_Widget * pWidget, void *pData);	// Tools/Custom Fields menu selection
    static void ToolsOptions(Fl_Widget * pWidget, void *pData);	// Tools/Options menu selection
    static void ToolsPurgeCompletedToDos(Fl_Widget * pWidget, void *pData)	// Tools/Purge Completed ToDo's menu selection
    {
    }
    static void ToolsPurgeEvents(Fl_Widget * pWidget, void *pData)	// Tools/Purge Events menu selection
    {
    }
    static void ViewAddressBook(Fl_Widget * pWidget, void *pData);	// View/Address Book menu selection
    static void ViewHidePrivate(Fl_Widget * pWidget, void *pData)	// View/Hide Private Records menu selection
    {
    }
    static void ViewListBy(Fl_Widget * pWidget, void *pData);	// View/List By menu selection
    static void ViewNotes(Fl_Widget * pWidget, void *pData);	// View/Notes menu selection
    static void ViewScheduler(Fl_Widget * pWidget, void *pData);	// View/Scheduler menu selection
    static void ViewShow(Fl_Widget * pWidget, void *pData);	// View/Show (for ToDo List) menu selection
    static void ViewShowPrivate(Fl_Widget * pWidget, void *pData)	// View/Show Private Records menu selection
    {
    }
    static void ViewTodoList(Fl_Widget * pWidget, void *pData);	// View/ToDo List menu selection
};


#endif /*  */
