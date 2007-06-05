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
// Class for a button on a toolbar object.                      //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/Fl.H>
#include "Images.h"
#include "ToolbarButton.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ToolbarButton::ToolbarButton(int nX,
			     int nY,
			     int nImage,
			     int nDisabledImage, const char *pszTooltip)
    :
Fl_Button(nX, nY, TOOLBAR_BUTTON_WIDTH, TOOLBAR_BUTTON_WIDTH, "")
{
    m_bEnabled = true;
    m_nNormalImage = nImage;
    m_nDisabledImage = nDisabledImage;
    m_pPixmap = Images::GetImage(nImage);
    m_pPixmap->label(this);
    align(FL_ALIGN_CENTER);
    m_strTooltip = _(pszTooltip);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ToolbarButton::~ToolbarButton()
{
    delete m_pPixmap;
}


//--------------------------------------------------------------//
// Enable or disable this button.                               //
//--------------------------------------------------------------//
void
ToolbarButton::Enable(bool bEnable)
{
    // Only change if the states are different
    if (m_bEnabled != bEnable) {
	if (bEnable == true) {
	    // Enable the button
	    delete m_pPixmap;
	    m_pPixmap = Images::GetImage(m_nNormalImage);
	    m_pPixmap->label(this);
	    activate();
	    m_bEnabled = true;
	} else {
	    // Disable this button, but only if there is a disabled image
	    if (m_nDisabledImage >= 0) {
		delete m_pPixmap;
		m_pPixmap = Images::GetDisabledImage(m_nDisabledImage);
		m_pPixmap->label(this);
		deactivate();
		m_bEnabled = false;
	    }
	}
    }
}
