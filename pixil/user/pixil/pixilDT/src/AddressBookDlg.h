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
// Address Book Dialog.                                         //
//--------------------------------------------------------------//
#ifndef ADDRESSBOOKDLG_H_

#define ADDRESSBOOKDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Window.H>
#include "NoteEditor.h"
class AddressBookDlg:public Fl_Window
{
  public:AddressBookDlg(int nRow,
		   // Constructor
		   Fl_Widget * pParent);
     ~AddressBookDlg();		// Destructor
    int DoModal();		// Run the modal dialog
    inline int GetRecno() const	// Get the record number just updated/inserted
    {
	return (m_nRecno);
    }
  private:  Fl_Box * m_pMessage;
    // Static message
    Fl_Button *m_pCancelButton;	// The Cancel button
    Fl_Button *m_pHelpButton;	// The Help Button
    Fl_Button *m_pOKButton;	// The OK button
    Fl_Button *m_pRadioButton[7];	// Show radio buttons
    Fl_Choice *m_pCategory;	// Category Choice widget
    Fl_Choice *m_pInfoChoice[7];	// Info field choices
    Fl_Group *m_pPage[3];	// Tab pages
    Fl_Input *m_pAddress;	// Address widget
    Fl_Input *m_pAnniversary;	// Anniversary
    Fl_Input *m_pBirthday;	// Birthday
    Fl_Input *m_pCity;		// City widget
    Fl_Input *m_pCompany;	// The company widget
    Fl_Input *m_pCountry;	// Country widget
    Fl_Input *m_pCustom[4];	// Custom field widgets
    Fl_Input *m_pInfo[7];	// Info field widgets
    Fl_Input *m_pFirstName;	// The first name widget
    Fl_Input *m_pLastName;	// The last name widget
    Fl_Input *m_pPostalCode;	// Postal code widget
    Fl_Input *m_pRegion;	// Region widget
    Fl_Input *m_pTitle;		// The title widget
    Fl_Menu_Item *m_pTranslatedCategoryMenu;	// Menu for the category choice widget
    Fl_Menu_Item *m_pTranslatedMenu[7];	// Menus for each of m_pInfoChoice above
    Fl_Tabs *m_pTabs;		// The tab widget
    int m_nRecno;		// The record number just updated/inserted
    int m_nRow;			// Row in the Address Book database
    NoteEditor *m_pNoteEditor;	// Note editor
    string m_strCustomField[4];	// Custom field names
    void CreateCategoryChoice();	// Create the category choice widget
    void CreatePage1();		// Create the first tab page
    void CreatePage2();		// Create the first tab page
    void CreatePage3();		// Create the first tab page
    void FillInfoChoice(Fl_Choice * pChoice,	// Fill in an Info field choice widget
			int nIndex);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
};


#endif /*  */
