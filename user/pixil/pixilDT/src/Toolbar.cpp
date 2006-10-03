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
// Class for the main window's toolbar for the Pixil Desktop.   //
//--------------------------------------------------------------//
#include <cassert>
#include "Toolbar.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor.                                                 //
//--------------------------------------------------------------//
Toolbar::Toolbar(int nX,
		 int nY,
		 int nWidth, int nHeight, const ToolbarItem * pToolbarItem)
    :
Fl_Group(nX, nY, nWidth, nHeight, "")
{
    int i;
    int nXPos;

    // Get the number of buttons to be created
    for (i = 0; pToolbarItem[i].m_nType != TOOLBAR_END; ++i);

    // Get memory to cache the toolbar stuff
    m_ppToolbarButton = new PTOOLBARBUTTON[i];
    m_nCount = i;

    // Create each button in turn
    i = 0;
    nXPos = 10;
    do {
	// Create the button
	if (pToolbarItem[i].m_nType != TOOLBAR_GAP
	    && pToolbarItem[i].m_nType != TOOLBAR_END) {
	    m_ppToolbarButton[i] = new ToolbarButton(x() + nXPos,
						     y() + 3,
						     pToolbarItem[i].m_nImage,
						     pToolbarItem[i].
						     m_nDisabledImage,
						     pToolbarItem[i].
						     m_pszTooltip);
	    m_ppToolbarButton[i]->callback(Callback);
	    m_ppToolbarButton[i]->SetCallback(pToolbarItem[i].m_pfnCallback);
	    nXPos += ToolbarButton::GetWidth();
	} else {
	    // No button, just a null pointer and a gap
	    m_ppToolbarButton[i] = NULL;
	    nXPos += (ToolbarButton::GetWidth() >> 1);
	}
	++i;
    } while (pToolbarItem[i].m_nType != TOOLBAR_END);

    // Finish this widget
    end();
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
Toolbar::~Toolbar()
{
    int i;

    for (i = 0; i < m_nCount; ++i) {
	if (m_ppToolbarButton[i] != NULL) {
	    remove(m_ppToolbarButton[i]);
	    delete m_ppToolbarButton[i];
	}
    }
    delete[]m_ppToolbarButton;
}


//--------------------------------------------------------------//
// Translate the widget involved in a callback from a button.   //
//--------------------------------------------------------------//
void
Toolbar::Callback(Fl_Widget * pWidget, void *pUserData)
{
    ((ToolbarButton *) pWidget)->DoCallback();
}


//--------------------------------------------------------------//
// Enable or disable a toolbar button.  The button is located   //
// by the index to its "normal image".                          //
//--------------------------------------------------------------//
void
Toolbar::Enable(int nImage, bool bEnable)
{
    int i;

    // Find the button
    for (i = 0; i < m_nCount; ++i) {
	if (m_ppToolbarButton[i] != NULL) {
	    if (m_ppToolbarButton[i]->GetNormalImage() == nImage) {
		break;
	    }
	}
    }

#ifdef DEBUG
    // Test that the image was found
    assert(i < m_nCount);
#endif

    if (i < m_nCount) {
	// Enable or disable this button
	m_ppToolbarButton[i]->Enable(bEnable);
    }
}
