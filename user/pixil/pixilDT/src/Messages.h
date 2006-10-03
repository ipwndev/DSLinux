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
// Messages passed between widgets with do_callback             //
//--------------------------------------------------------------//
#ifndef MESSAGES_H_

#define MESSAGES_H_
enum PixilDTMessage
{ ADDRESS_BOOK_REQUESTED = 1, NOTES_REQUESTED, SCHEDULER_REQUESTED, TODO_LIST_REQUESTED, ADDRESS_BOOK_CHANGED, NOTES_CHANGED, SCHEDULER_CHANGED, TODO_LIST_CHANGED, ADDRESS_BOOK_DELETE, NOTES_DELETE, SCHEDULER_DELETE, TODO_LIST_DELETE, ADDRESS_BOOK_GOTO,	// Requires second parameter - physical record number to go to
    NOTES_GOTO,			// Requires second parameter - physical record number to go to
    SCHEDULER_GOTO,		// Requires second parameter - physical record number to go to
    TODO_LIST_GOTO,		// Requires second parameter - physical record number to go to
    ADDRESS_BOOK_NEW, NOTES_NEW, SCHEDULER_NEW, TODO_LIST_NEW, ADDRESS_BOOK_CATEGORY_DELETED,	// Requires second parameter - category ID of deleted record
    NOTES_CATEGORY_DELETED,	// Requires second parameter - category ID of deleted record
    SCHEDULER_CATEGORY_DELETED,	// Requires second parameter - category ID of deleted record
    TODO_LIST_CATEGORY_DELETED,	// Requires second parameter - category ID of deleted record
    CATEGORY_DELETED,		// Requires second parameter - category ID of deleted record
    ADDRESS_BOOK_CATEGORY_ADDED, NOTES_CATEGORY_ADDED, SCHEDULER_CATEGORY_ADDED, TODO_LIST_CATEGORY_ADDED, CATEGORY_ADDED, ADDRESS_BOOK_CATEGORY_RENAMED, NOTES_CATEGORY_RENAMED, SCHEDULER_CATEGORY_RENAMED, TODO_LIST_CATEGORY_RENAMED, CATEGORY_RENAMED, FIND_ITEM_REQUESTED,	// Requires second parameter - line in the FindDlg results
    ENABLE_TOOLBAR_BUTTON,	// Requires seocnd parameter - the normal image of the toolbar button
    DISABLE_TOOLBAR_BUTTON,	// Requires seocnd parameter - the normal image of the toolbar button
    SHOW_LIST_BY,		// Address Book/List By dialog has been requested
    SHOW_TODO_OPTIONS,		// ToDo List/Show Options dialog has been requested
    SELECTION_CHANGING,		// The selection between the four main parts of the application is changing
    APPLICATION_CLOSING,	// The application is closing, data must be saved
    BEGIN_WEEK_CHANGED,		// The beginning day of week has been changed
    ADDRESS_BOOK_PRINT,		// Print the address book
    NOTES_PRINT,		// Print the notes
    SCHEDULER_PRINT,		// Print the current scheduler page
    TODO_LIST_PRINT,		// Print the to do list
    EDIT_CUT,			// Perform an edit cut operation
    EDIT_COPY,			// Perform an edit copy operation
    EDIT_PASTE,			// Perform an edit paste operation
    EDIT_UNDO,			// Perform an edit undo operation
    EDIT_CUT_AVAILABLE,		// Is an edit/cut operation available, returns 1 or 0
    EDIT_COPY_AVAILABLE,	// Is an edit/copy operation available, returns 1 or 0
    EDIT_PASTE_AVAILABLE,	// Is an edit/paste operation available, returns 1 or 0
    EDIT_UNDO_AVAILABLE,	// Is an edit/undo operation available, returns 1 or 0
    FIX_MENU,			// Fix the main window's menu and toolbar
};

#endif /*  */
