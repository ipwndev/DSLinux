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
// Help ID's passed back to the main application.               //
//--------------------------------------------------------------//
#ifndef HELPID_H_

#define HELPID_H_
enum HelpID
{ HELP_NO_TOPIC = 0,		// No topic specified
    HELP_ADDRESS_BOOK_CATEGORY,	// Help on address book categories
    HELP_ADDRESS_BOOK_DLG,	// Help on the Address Book update dialog
    HELP_CATEGORY_EDITOR,	// Help on the Category Editor
    HELP_CUSTOM_FIELD_EDITOR,	// Help on the Custom Field Editor
    HELP_FIND_DLG,		// Help on the Find dialog
    HELP_LISTBY_DLG,		// Help on the Address Book/List By dialog
    HELP_NEW_USER,		// Help for the new user dialog
    HELP_NOTES_CATEGORY,	// Help on notes categories
    HELP_NOTE_EDITOR_DLG,	// Help on the Note Editor dialog
    HELP_OPTIONS_DLG,		// Help on the Options dialog
    HELP_SCHEDULER_CATEGORY,	// Help on scheduler categories
    HELP_SCHEDULER_DLG,		// Help on the Scheduler update dialog
    HELP_SCHEDULER_REPEAT_DLG,	// Help on the Scheduler Repeat entry dialog
    HELP_SCHEDULER_TIME_DLG,	// Help on the Scheduler time entry dialog
    HELP_TODO_LIST_CATEGORY,	// Help on ToDO list categories
    HELP_TODO_SHOW_DLG,		// Help on the ToDo List Show dialog
};

#endif /*  */
