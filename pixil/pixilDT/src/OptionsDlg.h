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
// Global Options Dialog.                                       //
//--------------------------------------------------------------//
#ifndef OPTIONSDLG_H_

#define OPTIONSDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>
class OptionsDlg:public Fl_Window
{
  public:OptionsDlg(Fl_Widget * pParent);
    // Constructor
    ~OptionsDlg();		// Destructor
    int DoModal();		// Run the modal dialog
  private:  Fl_Button * m_pBrowseButton;
    // Browse for data directory
    Fl_Button *m_pCancelButton;	// Cancel button
    Fl_Button *m_pHelpButton;	// Help button
    Fl_Button *m_pOKButton;	// OK Button
    Fl_Check_Button *m_pConfirmDelete;	// Confirm deletes check button
    Fl_Choice *m_pAppChoice;	// Startup application
    Fl_Choice *m_pDayBegins;	// Hour the day begins
    Fl_Choice *m_pWeekBegins;	// Day the week begins
    Fl_Input *m_pDataDir;	// Data directory
    Fl_Menu_Item *m_pMenuApp;	// Menu for m_pAppChoice
    Fl_Menu_Item *m_pMenuDayBegins;	// Menu for m_pDayBegins
    Fl_Menu_Item *m_pMenuWeekBegins;	// Menu for m_pWeekBegins
    static void OnBrowseButton(Fl_Widget * pWidget,	// Process a click on the browse button
			       void *pUserData);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process a click on the help button
			     void *pUserData);
    static bool Validate(const char *pszString,	// Validate a new data directory
			 Fl_Widget * pThis, void *pData);
};


#endif /*  */
