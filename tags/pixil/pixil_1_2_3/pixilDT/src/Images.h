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
// Class used to access all images in the application.          //
//--------------------------------------------------------------//
#ifndef IMAGES_H_

#define IMAGES_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <cassert>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>
#ifndef WIN32
#include <FL/x.H>		// For Pixmap
#endif /*  */
class Images
{
  public:enum ImageID
    { ADDRESSBOOK_IMAGE = 0, BIGLEFT_ICON, BIGRIGHT_ICON, BOX_ICON, CALENDAR_ICON, CHECKBOX_ICON, CURHELP_ICON, DISABLED_CALENDAR_ICON, DISABLED_COPY_ICON, DISABLED_CUT_ICON, DISABLED_NOTES, DISABLED_PASTE_ICON, DISABLED_PRINT_ICON, DISABLED_SAVE_ICON, DISABLED_UNDO_ICON, EDITCOPY_ICON, EDITCUT_ICON, EDITPASTE_ICON, EDITUNDO_ICON, FILEPRINT_ICON, FILESAVE_ICON, FIND_ICON, LEFT_ICON, NEWITEM_ICON, NOTES_IMAGE, NOTES_ICON, PIXIL_ICON, PRIVATE_ICON, REPEAT_ICON, RIGHT_ICON, SCHEDULER_IMAGE, SMALL_DOWN_ICON, SMALL_UP_ICON, TIME_ICON, TODOLIST_IMAGE, MAX_IMAGES,	// Not an image, used for range checking
    };
      Images();			// Default constructor
     ~Images();			// Destructor
    static void Destroy();	// Destroy all image storage
    inline static Fl_Pixmap *GetAddressBookImage()	// Get the Address Book button image
    {
	return (m_pThis->GetImage(ADDRESSBOOK_IMAGE));
    }
    inline static Fl_Pixmap *GetBigLeftIcon()	// Get the big left arrow icon
    {
	return (m_pThis->GetImage(BIGLEFT_ICON));
    }
    inline static Fl_Pixmap *GetBigRightIcon()	// Get the big right arrow icon
    {
	return (m_pThis->GetImage(BIGRIGHT_ICON));
    }
    inline static Fl_Pixmap *GetBoxIcon()	// Get the unchecked box icon
    {
	return (m_pThis->GetImage(BOX_ICON));
    }
    inline static Fl_Pixmap *GetCalendarIcon()	// Get the date/calendar icon
    {
	return (m_pThis->GetImage(CALENDAR_ICON));
    }
    inline static Fl_Pixmap *GetCheckboxIcon()	// Get the checked box icon
    {
	return (m_pThis->GetImage(CHECKBOX_ICON));
    }
    static Fl_Pixmap *GetDisabledImage(int nImage);	// Get the image with possible color changes
    static Fl_Pixmap *GetImage(int nImage);	// Get an image
    inline static Fl_Pixmap *GetLeftIcon()	// Get the Left arrow icon
    {
	return (m_pThis->GetImage(LEFT_ICON));
    }
    inline static Fl_Pixmap *GetNotesImage()	// Get the Notes button image
    {
	return (m_pThis->GetImage(NOTES_IMAGE));
    }
    inline static Fl_Pixmap *GetNotesIcon()	// Get the small "notes available" icon
    {
	return (m_pThis->GetImage(NOTES_ICON));
    }
    inline static Fl_Pixmap *GetPixilIcon()	// Get the Pixil window icon
    {
	return (m_pThis->GetImage(PIXIL_ICON));
    }

#ifndef WIN32
    static unsigned long GetPixmap(int nIndex);	// Get an X Pixmap for an image
#endif /*  */
    inline static Fl_Pixmap *GetPrivateIcon()	// Get the small "private entry" icon
    {
	return (m_pThis->GetImage(PRIVATE_ICON));
    }
    inline static Fl_Pixmap *GetRepeatIcon()	// Get the Repeat arrow icon
    {
	return (m_pThis->GetImage(REPEAT_ICON));
    }
    inline static Fl_Pixmap *GetRightIcon()	// Get the Right arrow icon
    {
	return (m_pThis->GetImage(RIGHT_ICON));
    }
    inline static Fl_Pixmap *GetSchedulerImage()	// Get the Scheduler button image
    {
	return (m_pThis->GetImage(SCHEDULER_IMAGE));
    }
    inline static Fl_Pixmap *GetSmallDownIcon()	// Get the Small Down arrow icon
    {
	return (m_pThis->GetImage(SMALL_DOWN_ICON));
    }
    inline static Fl_Pixmap *GetSmallUpIcon()	// Get the Small Up arrow icon
    {
	return (m_pThis->GetImage(SMALL_UP_ICON));
    }
    inline static Fl_Pixmap *GetTimeIcon()	// Get the Time icon
    {
	return (m_pThis->GetImage(TIME_ICON));
    }
    inline static Fl_Pixmap *GetToDoListImage()	// Get ToDo List button image
    {
	return (m_pThis->GetImage(TODOLIST_IMAGE));
    }
  private:static char **m_ppszDisabledXPM[];
    // Disabled XPM's with different colors from the original XPM
    static char const *const *m_ppszXPM[];	// Source XPM's
    static Images *m_pThis;	// Singleton pointer
};


#endif /*  */
