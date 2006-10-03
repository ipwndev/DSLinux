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
// Class for the left portion of the main window for PixilDT.   //
//--------------------------------------------------------------//
#include "config.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "LeftGroup.h"
#include "PixilMainWnd.h"

#include "VCMemoryLeak.h"


#define BUTTON_HEIGHT 52
#define BUTTON_WIDTH  50
#define FIRST_BUTTON  15


//--------------------------------------------------------------//
// Default constructor                                          //
//--------------------------------------------------------------//
LeftGroup::LeftGroup(int nX, int nY, int nWidth, int nHeight)
    :
Fl_Group(nX, nY, nWidth, nHeight, "")
{
    int nLeft = ((nWidth - BUTTON_WIDTH) >> 1);

    // Set the background color
    box(FL_ENGRAVED_BOX);

    // Create each of four boxes, these are added to the current group by default
    new ImageBox(x() + nLeft,
		 y() + FIRST_BUTTON,
		 BUTTON_WIDTH,
		 BUTTON_HEIGHT,
		 Images::GetSchedulerImage(), SCHEDULER_REQUESTED, _("Date"));
    new ImageBox(x() + nLeft,
		 y() + FIRST_BUTTON + 1 * (BUTTON_HEIGHT + 10),
		 BUTTON_WIDTH,
		 BUTTON_HEIGHT,
		 Images::GetAddressBookImage(),
		 ADDRESS_BOOK_REQUESTED, _("Address"));
    new ImageBox(x() + nLeft,
		 y() + FIRST_BUTTON + 2 * (BUTTON_HEIGHT + 10),
		 BUTTON_WIDTH,
		 BUTTON_HEIGHT,
		 Images::GetToDoListImage(), TODO_LIST_REQUESTED, _("To Do"));
    new ImageBox(x() + nLeft,
		 y() + FIRST_BUTTON + 3 * (BUTTON_HEIGHT + 10),
		 BUTTON_WIDTH,
		 BUTTON_HEIGHT,
		 Images::GetNotesImage(), NOTES_REQUESTED, _("Memo"));

    resizable(new
	      Fl_Box(x(), y() + FIRST_BUTTON + 4 * (BUTTON_HEIGHT + 10), w(),
		     h() - FIRST_BUTTON - 4 * (BUTTON_HEIGHT + 10)));

    end();

    // Show this group
    show();
}


//--------------------------------------------------------------//
// Process a message from the parent widget.                    //
//--------------------------------------------------------------//
int
LeftGroup::Message(PixilDTMessage nMessage, int nInfo)
{
    ImageBox *pImageBox;
    int i;
    int nReturn = 0;		// Default return value

    // Call each child with this message (any that are down will pop-up like a radio button)
    for (i = 0; i < children(); ++i) {
	pImageBox = dynamic_cast < ImageBox * >(child(i));
	if (pImageBox != NULL) {
	    pImageBox->Message(nMessage, 0);
	}
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Called by one of the child widgets to indicate that a button //
// was pressed.                                                 //
//--------------------------------------------------------------//
void
LeftGroup::Notify(PixilDTMessage nMessage, int nInfo)
{
    // Notify the parent
    dynamic_cast < PixilMainWnd * >(parent())->Notify(nMessage, 0);

//      // Call each child with this message (any that are down will pop-up like a radio button)
//      Message(nMessage,0);
}
