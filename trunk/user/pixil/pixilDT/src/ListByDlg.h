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
#ifndef LISTBYDLG_H_

#define LISTBYDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Window.H>
class ListByDlg:public Fl_Window
{
  public:ListByDlg(bool bSelection,
	      // Constructor
	      Fl_Widget * pParent);
    ~ListByDlg();		// Destructor
    int DoModal();		// Run the modal dialog
    inline bool GetSelection() const	// Get the selection, true = sort by last name
    {
	return (m_bSelection);
    }
  private:  bool m_bSelection;
    // The selection, true = sort by last name
    Fl_Button *m_pCancelButton;	// Cancel button
    Fl_Button *m_pHelpButton;	// Help button
    Fl_Button *m_pOKButton;	// OK button
    Fl_Check_Button *m_pRadioButton[2];	// The radio buttons
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
};


#endif /*  */
