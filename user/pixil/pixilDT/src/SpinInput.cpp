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
// A class for what looks like a spin control.                  //
//--------------------------------------------------------------//
#include <cstdio>
#include <FL/Fl.H>
#include <FL/Fl_Repeat_Button.H>
#include "Dialog.h"
#include "Images.h"
#include "SpinInput.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SpinInput::SpinInput(int nX,
		     int nY,
		     int nWidth,
		     int nHeight,
		     const char *pszLabel,
		     int nMaxSize, int nMinimum, int nMaximum)
    :
Fl_Group(nX, nY, nWidth, nHeight)
{
    Fl_Repeat_Button *pButton;

    // Create what looks like a spin control with some surrounding text
    m_pInput =
	new Fl_Int_Input(nX, nY, nWidth - IMAGE_BUTTON_WIDTH, nHeight,
			 pszLabel);
    m_pInput->maximum_size(nMaxSize);
    m_nMin = nMinimum;
    if (nMaximum > nMinimum) {
	m_nMax = nMaximum;
    } else {
	m_nMax = m_nMin;
    }
    if (m_nMin > 0) {
	value(m_nMin);
    } else if (m_nMax < 0) {
	value(m_nMax);
    } else {
	value(0);
    }
    pButton = new Fl_Repeat_Button(nX + nWidth - IMAGE_BUTTON_WIDTH,
				   nY, IMAGE_BUTTON_WIDTH, nHeight / 2);
    pButton->callback(OnUpButton);
    m_pUpPixmap = Images::GetSmallUpIcon();
    m_pUpPixmap->label(pButton);
    pButton = new Fl_Repeat_Button(nX + nWidth - IMAGE_BUTTON_WIDTH,
				   nY + nHeight / 2,
				   IMAGE_BUTTON_WIDTH, nHeight - nHeight / 2);
    pButton->callback(OnDownButton);
    m_pDownPixmap = Images::GetSmallDownIcon();
    m_pDownPixmap->label(pButton);

    // Finish this widget
    end();
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SpinInput::~SpinInput()
{
    delete m_pUpPixmap;
    delete m_pDownPixmap;
}


//--------------------------------------------------------------//
// Down button callback                                         //
//--------------------------------------------------------------//
void
SpinInput::OnDownButton(Fl_Widget * pWidget, void *pUserData)
{
    int nValue;
    SpinInput *pThis = (SpinInput *) (pWidget->parent());

    // Decrement the value of this widget
    nValue = pThis->value() - 1;
    if (nValue >= pThis->m_nMin) {
	pThis->value(nValue);
    }
}


//--------------------------------------------------------------//
// Up button callback                                           //
//--------------------------------------------------------------//
void
SpinInput::OnUpButton(Fl_Widget * pWidget, void *pUserData)
{
    int nValue;
    SpinInput *pThis = (SpinInput *) (pWidget->parent());

    // Increment the value of this widget
    nValue = pThis->value() + 1;
    if (nValue <= pThis->m_nMax) {
	pThis->value(nValue);
    }
}


//--------------------------------------------------------------//
// Set the value of this widget                                 //
//--------------------------------------------------------------//
void
SpinInput::value(int nValue)
{
    char szData[16];

    if (nValue >= m_nMin && nValue <= m_nMax) {
	sprintf(szData, "%d", nValue);
	m_pInput->value(szData);
    }
}
