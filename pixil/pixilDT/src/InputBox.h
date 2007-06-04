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
#ifndef INPUTBOX_H_

#define INPUTBOX_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
using namespace std;
typedef bool(*VALIDATE_FUNC) (const char *pszString, Fl_Widget * pWidget,
			      void *pUserData);
class InputBox:public Fl_Window
{
  public:InputBox(const char *pszTitle,
	     // Constructor
	     Fl_Widget * pParent, const char *pszDefault, int nMaxSize,
	     enum HelpID nHelpID, VALIDATE_FUNC pfnValidate,
	     Fl_Widget * pValidateWidget, void *pValidateData,
	     const char *pszFmt, ...);
     ~InputBox();		// Destructor
    inline const string & GetEntry() const	// Get the string keyed in
    {
	return (m_strReply);
    }
  private:  Fl_Box * m_pIcon;
    // Area for the icon
    Fl_Box *m_pMessage;		// Area for the prompt
    Fl_Button *m_pButton[3];	// Buttons on the dialog
    Fl_Input *m_pInput;		// Input widget
    Fl_Widget *m_pValidateWidget;	// Widget for validation call
    enum HelpID m_nHelpID;	// Help for this input dialog
    int m_nMaxSize;		// Maximum size of the entry field
    string m_strPrompt;		// The prompt for the input field
    string m_strReply;		// The entry field
    VALIDATE_FUNC m_pfnValidate;	// Entry validation function
    void *m_pValidateData;	// Data passed to the validation routine
    void FormatPrompt(const char *pszFmt,	// Similar to vsprintf
		      va_list ap);
    static void OnHelpButton(Fl_Widget * pWidget,	// Help button callback
			     void *pUserData);
    void RunInput(const char *pszDefault);	// Actually run the modal dialog
};


#endif /*  */
