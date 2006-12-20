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
#include "config.h"
#include <FL/fl_ask.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include "AddressBookCategoryDB.h"
#include "AddressBookDB.h"
#include "AddressBookDlg.h"
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "Messages.h"
#include "PixilDT.h"

#include "VCMemoryLeak.h"


#define DLG_HEIGHT       480
#define DLG_INDENT       100
#define DLG_WIDTH        520


//--------------------------------------------------------------//
// Constructor, nRow is the row number in the address book      //
// database as currently sorted or a -1 if this is to be a new  //
// row.                                                         //
//--------------------------------------------------------------//
AddressBookDlg::AddressBookDlg(int nRow, Fl_Widget * pParent)
    :
Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Edit Address"))
{
    // Get the record number for this address book key
    if (nRow < 0) {
	// New row
	m_nRow = -1;
    } else {
	m_nRow = nRow;
    }

    // Create the dialog widgets
    // Tab/notebook widget first
    m_pTabs = new Fl_Tabs(DLG_BORDER,
			  DLG_BORDER,
			  w() - 3 * DLG_BORDER - DLG_BUTTON_WIDTH,
			  h() - 2 * DLG_BORDER);

    // Add the first page to the tab
    CreatePage1();

    // Add the second page to the tab
    CreatePage2();

    // Add the third page to the tab
    CreatePage3();

    // Finish with the tab widget
    m_pTabs->end();

    // Create the buttons
    m_pOKButton = new Fl_Return_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				       DLG_BORDER + DLG_TAB_HEIGHT,
				       DLG_BUTTON_WIDTH,
				       DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				    2 * DLG_BORDER + DLG_BUTTON_HEIGHT +
				    DLG_TAB_HEIGHT, DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  3 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT +
				  DLG_TAB_HEIGHT, DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);
    CreateCategoryChoice();

    m_pTabs->show();
    end();

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
AddressBookDlg::~AddressBookDlg()
{
    int i;

    // Free the Info choices
    for (i = 0; i < 7; ++i) {
	FreeTranslatedMenu(m_pTranslatedMenu[i]);
    }

    // Free the Category choice
    FreeTranslatedMenu(m_pTranslatedCategoryMenu);
}


//--------------------------------------------------------------//
// Create the category selection widget                         //
//--------------------------------------------------------------//
void
AddressBookDlg::CreateCategoryChoice()
{
    AddressBookCategoryDB *pAddressBookCategoryDB =
	AddressBookCategoryDB::GetAddressBookCategoryDB();

    // Create the widget
    m_pCategory = new Fl_Choice(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				h() - DLG_BORDER - DLG_BUTTON_HEIGHT,
				DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT);

    // Create the menu
    m_pTranslatedCategoryMenu =
	pAddressBookCategoryDB->GetCategoryMenu(false);

    // Select the current category
    if (m_nRow >= 0) {
	int nCategory =
	    pAddressBookCategoryDB->FindRow(CATID,
					    AddressBookDB::
					    GetAddressBookDB()->
					    GetCategory(m_nRow));

	if (nCategory < 0) {
	    // Fix if invalid category
	    nCategory = 0;
	}
	m_pCategory->value(nCategory);
    }
    // Set the menu
    m_pCategory->menu(m_pTranslatedCategoryMenu);
}


//--------------------------------------------------------------//
// Create the first tab page                                    //
//--------------------------------------------------------------//
void
AddressBookDlg::CreatePage1()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int i;
    int nWidth;
    int nX;
    int nY;

    // Create this page
    m_pPage[0] = new Fl_Group(DLG_BORDER,
			      DLG_BORDER + DLG_TAB_HEIGHT,
			      m_pTabs->w() - 2 * DLG_BORDER,
			      m_pTabs->h() - 2 * DLG_BORDER, _("Name"));
    nWidth = m_pPage[0]->w();
    nX = m_pPage[0]->x();
    nY = m_pPage[0]->y();

    // Create the input fields
    m_pLastName = new Fl_Input(nX + DLG_INDENT,
			       nY + DLG_BORDER,
			       m_pPage[0]->w() - DLG_INDENT,
			       DLG_INPUT_HEIGHT, _("Last Name:"));
    m_pLastName->maximum_size(pAddressBookDB->GetColumnSize(AB_LASTNAME));
    m_pFirstName = new Fl_Input(nX + DLG_INDENT,
				nY + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
				m_pPage[0]->w() - DLG_INDENT,
				DLG_INPUT_HEIGHT, _("First Name:"));
    m_pFirstName->maximum_size(pAddressBookDB->GetColumnSize(AB_FIRSTNAME));
    m_pTitle = new Fl_Input(nX + DLG_INDENT,
			    nY + 3 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
			    m_pPage[0]->w() - DLG_INDENT,
			    DLG_INPUT_HEIGHT, _("Title:"));
    m_pTitle->maximum_size(pAddressBookDB->GetColumnSize(AB_TITLE));
    m_pCompany = new Fl_Input(nX + DLG_INDENT,
			      nY + 4 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT,
			      m_pPage[0]->w() - DLG_INDENT,
			      DLG_INPUT_HEIGHT, _("Company:"));
    m_pCompany->maximum_size(pAddressBookDB->GetColumnSize(AB_COMPANY));
    m_pMessage = new Fl_Box(nX + 0,
			    nY + 5 * DLG_BORDER + 4 * DLG_INPUT_HEIGHT,
			    DLG_INDENT, DLG_INPUT_HEIGHT, _("Show in list:"));
    m_pMessage->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

    // Set up the Info fields
    for (i = 0; i < 7; ++i) {
	m_pRadioButton[i] =
	    new Fl_Check_Button(nX + DLG_INDENT - DLG_RADIO_SIZE,
				nY + 6 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT +
				i * (DLG_BORDER + DLG_INPUT_HEIGHT),
				DLG_RADIO_SIZE, DLG_INPUT_HEIGHT);
	m_pRadioButton[i]->type(FL_RADIO_BUTTON);
	m_pRadioButton[i]->down_box(FL_ROUND_DOWN_BOX);
	m_pInfoChoice[i] = new Fl_Choice(nX + DLG_INDENT,
					 nY + 6 * DLG_BORDER +
					 5 * DLG_INPUT_HEIGHT +
					 i * (DLG_BORDER + DLG_INPUT_HEIGHT),
					 (nWidth - DLG_INDENT -
					  DLG_BORDER) / 3, DLG_INPUT_HEIGHT);
	FillInfoChoice(m_pInfoChoice[i], i);
	m_pInfo[i] =
	    new Fl_Input(nX + DLG_INDENT +
			 (nWidth - DLG_INDENT - DLG_BORDER) / 3 + DLG_BORDER,
			 nY + 6 * DLG_BORDER + 5 * DLG_INPUT_HEIGHT +
			 i * (DLG_BORDER + DLG_INPUT_HEIGHT),
			 nWidth - DLG_INDENT - DLG_BORDER - (nWidth -
							     DLG_INDENT -
							     DLG_BORDER) / 3,
			 DLG_INPUT_HEIGHT);
	m_pInfo[i]->maximum_size(pAddressBookDB->GetColumnSize(AB_DEP1 + i));
    }

    // Set the initial data into the dialog if this is a changed row
    if (m_nRow >= 0) {
	int nSetting;

	// Set the text fields
	m_pLastName->value(pAddressBookDB->GetLastName(m_nRow).c_str());
	m_pFirstName->value(pAddressBookDB->GetFirstName(m_nRow).c_str());
	m_pTitle->value(pAddressBookDB->GetTitle(m_nRow).c_str());
	m_pCompany->value(pAddressBookDB->GetCompany(m_nRow).c_str());

	// Determine which show radio button to be used
	nSetting = pAddressBookDB->GetShowFlag(m_nRow);
	if (nSetting < 0 || nSetting >= 6) {
	    nSetting = 0;
	}
	m_pRadioButton[nSetting]->value(1);

	// Set the Info data
	for (i = 0; i < 7; ++i) {
	    m_pInfoChoice[i]->value(pAddressBookDB->GetInfoID(m_nRow, i));
	    m_pInfo[i]->value(pAddressBookDB->GetInfo(m_nRow, i).c_str());
	}
    }

    m_pPage[0]->end();
}


//--------------------------------------------------------------//
// Create the second tab page                                   //
//--------------------------------------------------------------//
void
AddressBookDlg::CreatePage2()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int i;
    int nX;
    int nY;

    // Create this page
    m_pPage[1] = new Fl_Group(DLG_BORDER,
			      DLG_BORDER + DLG_TAB_HEIGHT,
			      m_pTabs->w() - 2 * DLG_BORDER,
			      m_pTabs->h() - 2 * DLG_BORDER, _("Address"));
    nX = m_pPage[0]->x();
    nY = m_pPage[0]->y();

    // Create the input fields
    m_pAddress = new Fl_Input(nX + DLG_INDENT,
			      nY + DLG_BORDER,
			      m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
			      DLG_INPUT_HEIGHT, _("Address:"));
    m_pAddress->maximum_size(pAddressBookDB->GetColumnSize(AB_ADDRESS));
    m_pCity = new Fl_Input(nX + DLG_INDENT,
			   nY + 2 * DLG_BORDER + DLG_INPUT_HEIGHT,
			   m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
			   DLG_INPUT_HEIGHT, _("City:"));
    m_pCity->maximum_size(pAddressBookDB->GetColumnSize(AB_CITY));
    m_pRegion = new Fl_Input(nX + DLG_INDENT,
			     nY + 3 * DLG_BORDER + 2 * DLG_INPUT_HEIGHT,
			     m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
			     DLG_INPUT_HEIGHT, _("State:"));
    m_pRegion->maximum_size(pAddressBookDB->GetColumnSize(AB_REGION));
    m_pPostalCode = new Fl_Input(nX + DLG_INDENT,
				 nY + 4 * DLG_BORDER + 3 * DLG_INPUT_HEIGHT,
				 m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
				 DLG_INPUT_HEIGHT, _("Zip:"));
    m_pPostalCode->maximum_size(pAddressBookDB->GetColumnSize(AB_POSTALCODE));
    m_pCountry = new Fl_Input(nX + DLG_INDENT,
			      nY + 5 * DLG_BORDER + 4 * DLG_INPUT_HEIGHT,
			      m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
			      DLG_INPUT_HEIGHT, _("Country:"));
    m_pCountry->maximum_size(pAddressBookDB->GetColumnSize(AB_COUNTRY));

    // Create the anniversary and birthday input fields
    m_pBirthday = new Fl_Input(nX + DLG_INDENT,
			       nY + 7 * DLG_BORDER + 6 * DLG_INPUT_HEIGHT,
			       m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
			       DLG_INPUT_HEIGHT, _("Birthday:"));
    m_pBirthday->maximum_size(pAddressBookDB->GetColumnSize(AB_BDAY));
    m_pAnniversary = new Fl_Input(nX + DLG_INDENT,
				  nY + 8 * DLG_BORDER + 7 * DLG_INPUT_HEIGHT,
				  m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
				  DLG_INPUT_HEIGHT, _("Anniversary:"));
    m_pAnniversary->maximum_size(pAddressBookDB->GetColumnSize(AB_ANNIV));

    for (i = 0; i < 4; ++i) {
	m_strCustomField[i] = pAddressBookDB->GetCustomFieldName(i) + ":";
	m_pCustom[i] = new Fl_Input(nX + DLG_INDENT,
				    nY + 10 * DLG_BORDER +
				    9 * DLG_INPUT_HEIGHT + i * (DLG_BORDER +
								DLG_INPUT_HEIGHT),
				    m_pPage[1]->w() - DLG_INDENT - DLG_BORDER,
				    DLG_INPUT_HEIGHT,
				    m_strCustomField[i].c_str());
	m_pCustom[i]->maximum_size(pAddressBookDB->
				   GetColumnSize(AB_CUST1 + i));
    }

    // Set the initial data into the dialog if this is a changed row
    if (m_nRow >= 0) {
	// Set the text fields
	m_pAddress->value(pAddressBookDB->GetAddress(m_nRow).c_str());
	m_pCity->value(pAddressBookDB->GetCity(m_nRow).c_str());
	m_pRegion->value(pAddressBookDB->GetRegion(m_nRow).c_str());
	m_pPostalCode->value(pAddressBookDB->GetPostalCode(m_nRow).c_str());
	m_pCountry->value(pAddressBookDB->GetCountry(m_nRow).c_str());
	m_pBirthday->value(pAddressBookDB->GetBirthday(m_nRow).c_str());
	m_pAnniversary->value(pAddressBookDB->GetAnniversary(m_nRow).c_str());

	// Set the Custom Field data
	for (i = 0; i < 4; ++i) {
	    m_pCustom[i]->value(pAddressBookDB->GetCustomField(m_nRow, i).
				c_str());
	}
    }

    m_pPage[1]->end();
}


//--------------------------------------------------------------//
// Create the third tab page                                    //
//--------------------------------------------------------------//
void
AddressBookDlg::CreatePage3()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int nX;
    int nY;
    Note *pNote;

    // Create this page
    m_pPage[2] = new Fl_Group(DLG_BORDER,
			      DLG_BORDER + DLG_TAB_HEIGHT,
			      m_pTabs->w() - 2 * DLG_BORDER,
			      m_pTabs->h() - 2 * DLG_BORDER, _("Note"));
    nX = m_pPage[0]->x();
    nY = m_pPage[0]->y();

    // Create the note editor
    if (m_nRow == -1) {
	// New item
	pNote = pAddressBookDB->GetNewNote();
    } else {
	pNote = pAddressBookDB->GetNote(m_nRow);
    }
    m_pNoteEditor = new NoteEditor(nX + DLG_BORDER,
				   nY + DLG_BORDER,
				   m_pPage[2]->w() - 2 * DLG_BORDER,
				   m_pPage[2]->h() - 2 * DLG_BORDER,
				   true, pNote);
    m_pNoteEditor->SetDestroyNote(true);

    m_pPage[2]->end();
}


//--------------------------------------------------------------//
// Run the modal dialog                                         //
//--------------------------------------------------------------//
int
AddressBookDlg::DoModal()
{
    AddressBookDB *pAddressBookDB = AddressBookDB::GetAddressBookDB();
    int i;
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// OK button was pressed, update the Address Book record

	// Create a new Address Book record if needed
	if (m_nRow < 0) {
	    m_nRow = pAddressBookDB->Insert();
	}
	// Change all fields
	pAddressBookDB->SetAddress(m_nRow, m_pAddress->value());
	pAddressBookDB->SetAnniversary(m_nRow, m_pAnniversary->value());
	pAddressBookDB->SetBirthday(m_nRow, m_pBirthday->value());
	pAddressBookDB->SetCategory(m_nRow,
				    AddressBookCategoryDB::
				    GetAddressBookCategoryDB()->
				    GetCategoryID(m_pCategory->value()));
	pAddressBookDB->SetCity(m_nRow, m_pCity->value());
	pAddressBookDB->SetCompany(m_nRow, m_pCompany->value());
	pAddressBookDB->SetCountry(m_nRow, m_pCountry->value());
	pAddressBookDB->SetFirstName(m_nRow, m_pFirstName->value());
	pAddressBookDB->SetLastName(m_nRow, m_pLastName->value());
	pAddressBookDB->SetPostalCode(m_nRow, m_pPostalCode->value());
	pAddressBookDB->SetRegion(m_nRow, m_pRegion->value());
	pAddressBookDB->SetTitle(m_nRow, m_pTitle->value());

	// Determine the show flag
	for (i = 0; i < 7; ++i) {
	    if (m_pRadioButton[i]->value() == 1) {
		pAddressBookDB->SetShowFlag(m_nRow, i);
	    }
	}

	// Change the info fields
	for (i = 0; i < 7; ++i) {
	    pAddressBookDB->SetInfoID(m_nRow, i, m_pInfoChoice[i]->value());
	    pAddressBookDB->SetInfo(m_nRow, i, m_pInfo[i]->value());
	}

	// Change the custom fields
	for (i = 0; i < 4; ++i) {
	    pAddressBookDB->SetCustom(m_nRow, i, m_pCustom[i]->value());
	}

	// Save changes to the note, (AddressBookDB will save the note to disk)
	pAddressBookDB->SetNote(m_nRow, m_pNoteEditor->GetNote());

	// Save these changes
	pAddressBookDB->Save();

	// Save the record number for later
	m_nRecno = pAddressBookDB->GetRecno(m_nRow);
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Fill an Info choice menu with selections                     //
//--------------------------------------------------------------//
void
AddressBookDlg::FillInfoChoice(Fl_Choice * pChoice, int nIndex)
{
    int i;
    int nMax = AddressBookDB::GetInfoNameCount() + 1;

    // Create the menu
    m_pTranslatedMenu[nIndex] = new Fl_Menu_Item[nMax];
    memset(m_pTranslatedMenu[nIndex], 0, nMax * sizeof(Fl_Menu_Item));
    for (i = 0; i < nMax - 1; ++i) {
	// Use strdup so that FreeTranslatedMenu will work correctly
	m_pTranslatedMenu[nIndex][i].text =
	    strdup(AddressBookDB::GetInfoName(i).c_str());
    }

    // Set the menu
    pChoice->menu(m_pTranslatedMenu[nIndex]);
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
AddressBookDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_ADDRESS_BOOK_DLG);
}
