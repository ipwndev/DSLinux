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
// Custom Field Editor dialog.                                  //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/fl_ask.H>
#include "CustomFieldDB.h"
#include "CustomFieldEditor.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 200
#define DLG_WIDTH  400


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
CustomFieldEditor::CustomFieldEditor(PixilMainWnd * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Custom Field Editor"))
{
    CustomFieldDB *pCustomFieldDB = CustomFieldDB::GetCustomField();
    int i;
    int nRow;

    // Sort the InfoDB by its key field
    pCustomFieldDB->Sort(0);

    // Create the dialog widgets
    for (i = 0; i < 4; ++i) {
	sprintf(m_szLabel[i], _("Custom Field %d:"), i + 1);
	m_pInput[i] = new Fl_Input(150, 35 * i + 15, 100, 24, m_szLabel[i]);
	m_pInput[i]->textsize(INFOTYPE - 1);
	if (pCustomFieldDB->NumUndeletedRecs() > i) {
	    m_pInput[i]->value(pCustomFieldDB->GetName(i).c_str());
	}
    }

    m_pOKButton = new Fl_Button(310, 15, 80, 24, fl_ok);
    m_pHelpButton = new Fl_Button(310, 55, 80, 24, _("&Help"));
    m_pCancelButton = new Fl_Button(310, 95, 80, 24, fl_cancel);
    m_pCancelButton->shortcut("^[");

    // Finish the dialog
    end();

    // Set the button callbacks
    m_pHelpButton->callback(OnHelpButton);

    // Run this as a modal dialog
    if (DoModal(this, m_pOKButton, m_pCancelButton) == 1) {
	// OK Button was pressed, process the changes
	for (i = 0; i < 4; ++i) {
	    nRow = pCustomFieldDB->FindRow(CUSTOMID, i);
	    if (nRow < 0) {
		// Must insert a new row
		nRow = pCustomFieldDB->Insert(i);
	    }
	    pCustomFieldDB->SetColumn(nRow, CUSTOM, m_pInput[i]->value());
	}

	// Save these chages to disk
	pCustomFieldDB->Save();

	// Notify everyone of Address Book changes
	PixilDT::GetApp()->GetMainWindow()->Notify(ADDRESS_BOOK_CHANGED, 0);
    }
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
CustomFieldEditor::~CustomFieldEditor()
{
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
CustomFieldEditor::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_CUSTOM_FIELD_EDITOR);
}
