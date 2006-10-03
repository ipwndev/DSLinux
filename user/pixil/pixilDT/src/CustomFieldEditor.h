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
#ifndef CUSTOMFIElDEDITOR_H_

#define CUSTOMFIELDEDITOR_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>
#include "CustomFieldDB.h"
#include "PixilMainWnd.h"
class CustomFieldEditor:public Fl_Window
{
  public:CustomFieldEditor(PixilMainWnd * pParent);
    // Constructor
    ~CustomFieldEditor();	// Destructor
  private:char m_szLabel[4][32];
    // Labels for the input fields
    Fl_Button *m_pCancelButton;	// The Cancel button
    Fl_Button *m_pHelpButton;	// The Help button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Input *m_pInput[4];	// Input fields for each of the 4 custom fields
    CustomFieldDB *m_pDB;	// Pointer to the database to be used for this dialog
    PixilMainWnd *m_pParent;	// The parent window
    static void OnHelpButton(Fl_Widget * pWidget,	// Help Button press
			     void *pUserData);
    static void OnOKButton(Fl_Widget * pWidget,	// OK Button press
			   void *pUserData);
};


#endif /*  */
