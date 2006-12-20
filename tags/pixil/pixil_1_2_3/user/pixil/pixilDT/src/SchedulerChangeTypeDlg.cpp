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
// Address Book Dialog.                                         //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "SchedulerChangeTypeDlg.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT       (5*DLG_BUTTON_HEIGHT)
#define DLG_WIDTH        (5*DLG_BORDER+4*DLG_BUTTON_WIDTH)


//--------------------------------------------------------------//
// Constructor, nRow is the row number in the address book      //
// database as currently sorted or a -1 if this is to be a new  //
// row.                                                         //
//--------------------------------------------------------------//
SchedulerChangeTypeDlg::SchedulerChangeTypeDlg(Fl_Widget * pParent,
					       bool bDelete)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Repeating Event"))
{
    const char *pszMessage;
    Fl_Box *pBox;

    // Create the box for the message
    if (bDelete == false) {
	pszMessage =
	    _
	    ("Should these changes be made to the current instance of this event, "
	     "to the current and future instances of this event, "
	     "or to all instances of this event ?");
    } else {
	pszMessage =
	    _("Should the current instance of this event be deleted, "
	      " the current instance and all future events be deleted, "
	      " or all instances of this event be deleted ?");
    }
    pBox = new Fl_Box(DLG_BORDER,
		      DLG_BORDER,
		      w() - 2 * DLG_BORDER,
		      h() - DLG_BUTTON_HEIGHT - 2 * DLG_BORDER, pszMessage);
    pBox->
	align(FL_ALIGN_LEFT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);

    // Create the buttons
    m_pCurrentButton = new Fl_Button(DLG_BORDER,
				     h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				     DLG_BUTTON_WIDTH,
				     DLG_BUTTON_HEIGHT, _("&Current"));
    m_pCurrentButton->callback(OnCurrentButton);
    m_pFutureButton = new Fl_Button(2 * DLG_BORDER + DLG_BUTTON_WIDTH,
				    h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, _("+ &Future"));
    m_pFutureButton->callback(OnFutureButton);
    m_pAllButton = new Fl_Button(3 * DLG_BORDER + 2 * DLG_BUTTON_WIDTH,
				 h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				 DLG_BUTTON_WIDTH,
				 DLG_BUTTON_HEIGHT, _("All"));
    m_pAllButton->callback(OnAllButton);
    m_pCancelButton = new Fl_Button(4 * DLG_BORDER + 3 * DLG_BUTTON_WIDTH,
				    h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");

    end();

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
SchedulerChangeTypeDlg::~SchedulerChangeTypeDlg()
{
}


//--------------------------------------------------------------//
// Run the dialog.                                              //
//--------------------------------------------------------------//
int
SchedulerChangeTypeDlg::DoModal()
{
    // Preset to the window being closed early
    m_nChangeType = SCHEDULER_CHANGE_TYPE_NONE;

    // Run the dialog
    ::DoModal(this, NULL, m_pCancelButton);

    // Indicate that the (non-existant) OK button was pressed
    return (1);
}


//--------------------------------------------------------------//
// All button was clicked (static callback).                    //
//--------------------------------------------------------------//
void
SchedulerChangeTypeDlg::OnAllButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerChangeTypeDlg *pThis =
	(SchedulerChangeTypeDlg *) (pWidget->parent());

    pThis->m_nChangeType = SCHEDULER_CHANGE_TYPE_ALL;
    pThis->do_callback();
}


//--------------------------------------------------------------//
// Current button was clicked (static callback).                //
//--------------------------------------------------------------//
void
SchedulerChangeTypeDlg::OnCurrentButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerChangeTypeDlg *pThis =
	(SchedulerChangeTypeDlg *) (pWidget->parent());

    pThis->m_nChangeType = SCHEDULER_CHANGE_TYPE_CURRENT_ONLY;
    pThis->do_callback();
}


//--------------------------------------------------------------//
// Future button was clicked (static callback).                 //
//--------------------------------------------------------------//
void
SchedulerChangeTypeDlg::OnFutureButton(Fl_Widget * pWidget, void *pUserData)
{
    SchedulerChangeTypeDlg *pThis =
	(SchedulerChangeTypeDlg *) (pWidget->parent());

    pThis->m_nChangeType = SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE;
    pThis->do_callback();
}
