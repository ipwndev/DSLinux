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
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include "Dialog.h"
#include "FLTKUtil.h"
#include "HelpID.h"
#include "Messages.h"
#include "Options.h"
#include "PixilDT.h"
#include "ToDoListShowDlg.h"

#include "VCMemoryLeak.h"


#define DLG_CHOICE_WIDTH 190
#define DLG_HEIGHT       (12*DLG_BORDER+7*DLG_BUTTON_HEIGHT)
#define DLG_WIDTH        400


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ToDoListShowDlg::ToDoListShowDlg(Fl_Widget * pParent)
:  Fl_Window(pParent->x() + ((pParent->w() - DLG_WIDTH) >> 1),
	  pParent->y() + ((pParent->h() - DLG_HEIGHT) >> 1),
	  DLG_WIDTH, DLG_HEIGHT, _("Show Options"))
{
    Fl_Box *pBox;

    // Create the dialog widgets
    // Create the sort order widget
    pBox = new Fl_Box(DLG_BORDER,
		      DLG_BORDER,
		      w() - 4 * DLG_BORDER - DLG_CHOICE_WIDTH -
		      DLG_BUTTON_WIDTH, DLG_BUTTON_HEIGHT, _("Sort by:"));
    pBox->
	align(FL_ALIGN_LEFT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
	      FL_ALIGN_WRAP);
    CreateOrderWidget(w() - 2 * DLG_BORDER - DLG_BUTTON_WIDTH -
		      DLG_CHOICE_WIDTH, DLG_BORDER, DLG_CHOICE_WIDTH,
		      DLG_BUTTON_HEIGHT);

    // Create the buttons used for selections
    pBox = new Fl_Box(DLG_BORDER,
		      2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
		      w() - 3 * DLG_BORDER - DLG_BUTTON_WIDTH,
		      h() - 3 * DLG_BORDER - DLG_BUTTON_HEIGHT);
    pBox->box(FL_EMBOSSED_FRAME);
    m_pCompletedButton = new Fl_Check_Button(3 * DLG_BORDER,
					     4 * DLG_BORDER +
					     DLG_BUTTON_HEIGHT,
					     w() - 5 * DLG_BORDER -
					     DLG_BUTTON_WIDTH,
					     DLG_BUTTON_HEIGHT,
					     _("Show &Completed Items"));
    m_pCompletedButton->down_box(FL_ROUND_DOWN_BOX);
    m_pCompletedButton->value(Options::GetToDoShowCompleted() ==
			      true ? 1 : 0);
    m_pDueButton =
	new Fl_Check_Button(3 * DLG_BORDER,
			    5 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT,
			    w() - 5 * DLG_BORDER - DLG_BUTTON_WIDTH,
			    DLG_BUTTON_HEIGHT, _("Show Only Due &Items"));
    m_pDueButton->down_box(FL_ROUND_DOWN_BOX);
    m_pDueButton->value(Options::GetToDoShowOnlyDue() == true ? 1 : 0);
    m_pDueDateButton = new Fl_Check_Button(3 * DLG_BORDER,
					   7 * DLG_BORDER +
					   4 * DLG_BUTTON_HEIGHT,
					   w() - 5 * DLG_BORDER -
					   DLG_BUTTON_WIDTH,
					   DLG_BUTTON_HEIGHT,
					   _("Show &Due Dates"));
    m_pDueDateButton->down_box(FL_ROUND_DOWN_BOX);
    m_pDueDateButton->value(Options::GetToDoShowDueDate() == true ? 1 : 0);
    m_pPriorityButton = new Fl_Check_Button(3 * DLG_BORDER,
					    8 * DLG_BORDER +
					    5 * DLG_BUTTON_HEIGHT,
					    w() - 5 * DLG_BORDER -
					    DLG_BUTTON_WIDTH,
					    DLG_BUTTON_HEIGHT,
					    _("Show &Priorities"));
    m_pPriorityButton->down_box(FL_ROUND_DOWN_BOX);
    m_pPriorityButton->value(Options::GetToDoShowPriority() == true ? 1 : 0);
    m_pCategoryButton = new Fl_Check_Button(3 * DLG_BORDER,
					    9 * DLG_BORDER +
					    6 * DLG_BUTTON_HEIGHT,
					    w() - 5 * DLG_BORDER -
					    DLG_BUTTON_WIDTH,
					    DLG_BUTTON_HEIGHT,
					    _("Show &Categories"));
    m_pCategoryButton->down_box(FL_ROUND_DOWN_BOX);
    m_pCategoryButton->value(Options::GetToDoShowCategory() == true ? 1 : 0);

    // Create the buttons
    m_pOKButton = new Fl_Return_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				       DLG_BORDER,
				       DLG_BUTTON_WIDTH,
				       DLG_BUTTON_HEIGHT, fl_ok);
    m_pCancelButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				    2 * DLG_BORDER + DLG_BUTTON_HEIGHT,
				    DLG_BUTTON_WIDTH,
				    DLG_BUTTON_HEIGHT, fl_cancel);
    m_pCancelButton->shortcut("^[");
    m_pHelpButton = new Fl_Button(w() - DLG_BUTTON_WIDTH - DLG_BORDER,
				  3 * DLG_BORDER + 2 * DLG_BUTTON_HEIGHT,
				  DLG_BUTTON_WIDTH,
				  DLG_BUTTON_HEIGHT, _("&Help"));
    m_pHelpButton->callback(OnHelpButton);

    end();

    // The DoModal method will call show on this window
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
ToDoListShowDlg::~ToDoListShowDlg()
{
    FreeTranslatedMenu(m_pSortMenu);
}


//--------------------------------------------------------------//
// Create the sort order widget                                 //
//--------------------------------------------------------------//
void
ToDoListShowDlg::CreateOrderWidget(int nX, int nY, int nWidth, int nHeight)
{
    static const char *pszMenuText[4] = {
	N_("Priority, Due Date"),
	N_("Due Date, Priority"),
	N_("Category, Due Date"),
	N_("Category, Priority"),
    };
    int i;
    int nMax = sizeof(pszMenuText) / sizeof(char *);

    // Create the widget
    m_pSortChoice = new Fl_Choice(nX, nY, nWidth, nHeight);

    // Create the menu
    m_pSortMenu = new Fl_Menu_Item[nMax + 1];
    memset(m_pSortMenu, 0, (nMax + 1) * sizeof(Fl_Menu_Item));

    // Add the categories to the menu
    for (i = 0; i < nMax; ++i) {
	// Use strdup so that FreeTranslatedMenu will work correctly
	m_pSortMenu[i].text = strdup(_(pszMenuText[i]));
    }

    // Set the menu into the choice
    m_pSortChoice->menu(m_pSortMenu);

    // Set the initial value
    m_pSortChoice->value(Options::GetToDoSort());
}


//--------------------------------------------------------------//
// Run the modal dialog                                         //
//--------------------------------------------------------------//
int
ToDoListShowDlg::DoModal()
{
    int nReturn =::DoModal(this, m_pOKButton, m_pCancelButton);

    if (nReturn == 1) {
	// OK button was pressed, update the show options
	Options::SetToDoShowCompleted(m_pCompletedButton->value() ==
				      1 ? true : false);
	Options::SetToDoShowOnlyDue(m_pDueButton->value() ==
				    1 ? true : false);
	Options::SetToDoShowDueDate(m_pDueDateButton->value() ==
				    1 ? true : false);
	Options::SetToDoShowPriority(m_pPriorityButton->value() ==
				     1 ? true : false);
	Options::SetToDoShowCategory(m_pCategoryButton->value() ==
				     1 ? true : false);
	Options::SetToDoSort(m_pSortChoice->value());
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Help button was clicked (static callback).                   //
//--------------------------------------------------------------//
void
ToDoListShowDlg::OnHelpButton(Fl_Widget * pWidget, void *pUserData)
{
    PixilDT::GetApp()->ShowHelp(HELP_TODO_SHOW_DLG);
}
