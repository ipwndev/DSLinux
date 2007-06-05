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
// List By Dialog.                                              //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "ListByDlg.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 200
#define DLG_WIDTH  400


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ListByDlg::ListByDlg(bool bSelection, Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Change Address Book Order"))
{
    m_pRadioButton[0] = new Fl_Check_Button(DLG_BORDER,
					    DLG_BORDER,
					    DLG_RADIO_SIZE,
					    DLG_RADIO_SIZE,
					    _
					    ("Sort by &Last Name then First Name"));
    m_pRadioButton[0]->type(FL_RADIO_BUTTON);
    m_pRadioButton[0]->down_box(FL_ROUND_DOWN_BOX);
    m_pRadioButton[0]->align(FL_ALIGN_RIGHT);
    m_pRadioButton[1] = new Fl_Check_Button(DLG_BORDER,
					    DLG_BORDER +
					    2 * DLG_BUTTON_HEIGHT,
					    DLG_RADIO_SIZE, DLG_RADIO_SIZE,
					    _
					    ("Sort by &Company Name then Last Name"));
    m_pRadioButton[1]->type(FL_RADIO_BUTTON);
    m_pRadioButton[1]->down_box(FL_ROUND_DOWN_BOX);
    m_pRadioButton[1]->align(FL_ALIGN_RIGHT);
    m_pOKButton = new Fl_Button(w() - 3 * (DLG_BUTTON_WIDTH + DLG_BORDER),
				h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
				DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton = new Fl_Button(w() - 2 * (DLG_BUTTON_WIDTH + DLG_BORDER),
				    h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - 1 * (DLG_BUTTON_WIDTH + DLG_BORDER),
				  h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);
    end();

    m_bSelection = bSelection;
    m_pRadioButton[m_bSelection == true ? 0 : 1]->value(1);

    // The DoModal method will show this dialog
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ListByDlg::~ListByDlg()
{
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
ListByDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// OK button was pressed, update the selection
	m_bSelection = (m_pRadioButton[0]->value() == 1);
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
ListByDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_LISTBY_DLG);
}
