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
// A class that will present a dialog just like "fl_input"      //
// except that the size of the input field can be limited.      //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <FL/fl_ask.H>
#include "FLTKUtil.h"
#include "HelpID.h"
#include "InputBox.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT 103
#define DLG_WIDTH  410


//--------------------------------------------------------------//
// Constructor                                                  //
// Just like "fl_input" but will limit the text size of the     //
// input field.                                                 //
//--------------------------------------------------------------//
InputBox::InputBox(const char *pszTitle,
		   Fl_Widget * pParent,
		   const char *pszDefault,
		   int nMaxSize,
		   enum HelpID nHelpID,
		   VALIDATE_FUNC pfnValidate,
		   Fl_Widget * pValidateWidget,
		   void *pValidateData, const char *pszFmt, ...)
    :
Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, pszTitle)
{
    va_list ap;

    m_nMaxSize = nMaxSize;
    m_nHelpID = nHelpID;
    m_pfnValidate = pfnValidate;
    m_pValidateWidget = pValidateWidget;
    m_pValidateData = pValidateData;
    va_start(ap, pszFmt);
    FormatPrompt(pszFmt, ap);
    va_end(ap);
    RunInput(pszDefault);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
InputBox::~InputBox()
{
}


//--------------------------------------------------------------//
// Format the prompt for the input box, almost the same as      //
// vsprintf.                                                    //
//--------------------------------------------------------------//
void
InputBox::FormatPrompt(const char *pszFmt, va_list ap)
{
    char *pszPrompt = new char[1024];

#ifdef DEBUG
    int nResult = vsprintf(pszPrompt, pszFmt, ap);
    assert(nResult < 1023);	// Too much output from vsprintf
#else
    vsprintf(pszPrompt, pszFmt, ap);
#endif
    m_strPrompt = pszPrompt;
    delete[]pszPrompt;
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
InputBox::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    InputBox *pThis = reinterpret_cast < InputBox * >(pWidget->parent());

    PixilDT::GetApp()->ShowHelp(pThis->m_nHelpID);
}


//--------------------------------------------------------------//
// Run the dialog                                               //
//--------------------------------------------------------------//
void
InputBox::RunInput(const char *pszDefault)
{
    char *pszString = new char[m_nMaxSize + 1];
    int nHeight;
    int nWidth;

    // Set the size of the input field based on the maximum number of characters
    fl_font(labelfont(), labelsize());
    memset(pszString, 'M', m_nMaxSize);
    pszString[m_nMaxSize] = '\0';
    fl_measure(pszString, nWidth, nHeight);
    delete[]pszString;
    if (nWidth > 340) {
	nWidth = 340;
    }

    m_pMessage = new Fl_Box(60, 10, 340, 20);
    m_pMessage->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_pMessage->label(m_strPrompt.c_str());
    m_pInput = new Fl_Input(60, 37, nWidth, 23);
    m_pInput->hide();
    m_pInput->type(FL_NORMAL_INPUT);
    m_pInput->show();
    m_pInput->value(pszDefault);
    m_pInput->maximum_size(m_nMaxSize);
    m_pIcon = new Fl_Box(10, 10, 50, 50);
    m_pIcon->box(FL_THIN_UP_BOX);
    m_pIcon->label("?");
    m_pIcon->labelfont(FL_TIMES_BOLD);
    m_pIcon->labelsize(34);
    m_pIcon->color(FL_WHITE);
    m_pIcon->labelcolor(FL_BLUE);
    m_pButton[0] = new Fl_Button(310, 70, 90, 23);
    m_pButton[0]->shortcut("^[");
    m_pButton[0]->label(fl_cancel);
    m_pButton[1] = new Fl_Return_Button(210, 70, 90, 23);
    m_pButton[1]->label(fl_ok);
    m_pButton[2] = new Fl_Button(110, 70, 90, 23);
    m_pButton[2]->label(_("&Help"));
    m_pButton[2]->callback(OnHelpButton);
    resizable(new Fl_Box(60, 10, 110 - 60, 27));
    end();

#ifdef WIN32
    MessageBeep(MB_ICONQUESTION);
#endif // WIN32

    // Stay modal until a good string is entered
    while (DoModal(this, m_pButton[1], m_pButton[0]) == 1) {
	// Validate the string if a validation function exists
	if (m_pfnValidate == NULL
	    || m_pfnValidate(m_pInput->value(), m_pValidateWidget,
			     m_pValidateData) == true) {
	    // Save the input string and quit this loop
	    m_strReply = m_pInput->value();
	    break;
	}
    }
}
