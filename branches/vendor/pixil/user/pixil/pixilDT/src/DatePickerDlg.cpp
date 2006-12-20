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
// Date Picker Dialog.                                          //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/Fl_Return_Button.H>
#include "DatePickerDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT(nType) (DLG_BUTTON_HEIGHT+DLG_BORDER+DatePicker::GetHeight(nType))
#define DLG_WIDTH (2*DLG_BORDER+DatePicker::GetWidth())


//--------------------------------------------------------------//
// Dialog titles based on the type of the dialog.  These must   //
// be in the same order as the Datepicker::Type enumeration.    //
//--------------------------------------------------------------//
const char *
    DatePickerDlg::m_pszTitle[3] = {
    "Select Date",
    "Select Week",
    "Select Month",
};


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
DatePickerDlg::DatePickerDlg(time_t nTime,
			     DatePicker::Type nType, Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT(nType)) >> 1),
	  DLG_WIDTH,
	  DLG_HEIGHT(nType),
	  _(nType >= 0
	      && nType < 3 ? m_pszTitle[nType] : m_pszTitle[0]))
{
    int nPushButtonWidth = (DatePicker::GetWidth() - 2 * DLG_BORDER) / 3;

    // Set the preselected date
    m_nTime = nTime;

    // Set the dialog type
    if (nType < 0 || nType >= 3) {
	// Reset to daily
	nType = DatePicker::Daily;
    }

    m_pDatePicker = new DatePicker(DLG_BORDER, DLG_BORDER, nType, nTime);

    // Create the buttons
    m_pOKButton = new Fl_Return_Button(DLG_BORDER,
				       h() - DLG_BUTTON_HEIGHT - DLG_BORDER,
				       w() - 4 * DLG_BORDER -
				       2 * nPushButtonWidth,
				       DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton =
	new Fl_Button(w() - 2 * (nPushButtonWidth + DLG_BORDER),
		      h() - DLG_BUTTON_HEIGHT - DLG_BORDER, nPushButtonWidth,
		      DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pTodayButton = new Fl_Button(w() - nPushButtonWidth - DLG_BORDER,
				   h() - (DLG_BUTTON_HEIGHT + DLG_BORDER),
				   nPushButtonWidth,
				   DLG_BUTTON_HEIGHT, _("&Today"));
    m_pTodayButton->callback(TodayButtonCallback);

    // Finish the dialog
    end();

    // The DoModal method will show this dialog
}


//--------------------------------------------------------------//
// Run the modal dialog.                                        //
//--------------------------------------------------------------//
int
DatePickerDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// OK button was pressed, get the updated note
	m_nTime = m_pDatePicker->GetDate();
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Process a click on the Today button.                         //
//--------------------------------------------------------------//
void
DatePickerDlg::TodayButtonCallback(Fl_Widget * pWidget, void *pUserData)
{
    DatePickerDlg *pThis =
	reinterpret_cast < DatePickerDlg * >(pWidget->parent());
    time_t nToday = time(NULL);

    pThis->m_pDatePicker->SetDate(nToday);
}
