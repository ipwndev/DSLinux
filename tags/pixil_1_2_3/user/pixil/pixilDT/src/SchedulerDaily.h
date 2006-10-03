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
// Scheduler Daily tab page                                     //
//--------------------------------------------------------------//
#ifndef SCHEDULERDAILY_H_

#define SCHEDULERDAILY_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include "AddressBookList.h"
#include "DatePicker.h"
#include "ScheduleContainer.h"
#include "ToDoListList.h"
class SchedulerDaily:public Fl_Group
{
  public:SchedulerDaily(int nX,
		   // Constructor
		   int nY, int nWidth, int nHeight);
     ~SchedulerDaily();		// Destructor
    void DisplayDay(time_t nDate);	// Display a particular day's appointments
    int Message(PixilDTMessage nMessage,	// Message from the parent widget
		int nInfo);
    void Print();		// Print this data
  private:  AddressBookList * m_pAddressBookList;
    // The address book list
    char m_szDate[64];		// The label for the date banner
    DatePicker *m_pDatePicker;	// The date picker widget
    Fl_Box *m_pDate;		// Date banner at top of widget
    Fl_Button *m_pAddressButton;	// The Address Book button
    Fl_Button *m_pEditButton;	// The Edit button
    Fl_Button *m_pNewButton;	// The New button
    Fl_Button *m_pTodayButton;	// The Today button
    Fl_Button *m_pToDoButton;	// The ToDo List button
    Fl_Choice *m_pAddressBookCategory;	// The Category choice widget
    Fl_Choice *m_pToDoListCategory;	// The Category choice widget
    Fl_Menu_Item *m_pAddressBookCategoryMenu;	// Category choice menu
    Fl_Menu_Item *m_pToDoListCategoryMenu;	// Category choice menu
    ScheduleContainer *m_pScheduleContainer;	// Container for 1 day's appointments
    ToDoListList *m_pToDoListList;	// To Do List
    time_t m_nDate;		// The current date for refreshes
    static void AddressBookCategorySelected(Fl_Widget * pWidget,	// Callback for the Address Book category choice
					    void *pUserData);
    void DisplayAddressBook();	// Display the address book
    void DisplayToDoList();	// Display the ToDo List
    static void NewCallback(Fl_Widget * pWidget,	// Callback for the New button
			    void *pUserData);
    static void OnAddressButton(Fl_Widget * pWidget,	// Callback for the Address button
				void *pUserData);
    static void OnAddressGoto(int nRow,	// Process an Address Book double click
			      int nCol);
    static void OnDatePick(Fl_Widget * pWidget,	// Process a date pick
			   time_t nDate);
    static void OnEditButton(Fl_Widget * pWidget,	// Callback for the Edit button
			     void *pUserData);
    static void OnToDoButton(Fl_Widget * pWidget,	// Callback for the ToDo button
			     void *pUserData);
    static void OnToDoGoto(int nRow,	// Process a ToDo item double click
			   int nCol);
    static void TodayCallback(Fl_Widget * pWidget,	// Callback for the Today button
			      void *pUserData);
    static void ToDoListCategorySelected(Fl_Widget * pWidget,	// Callback for the ToDo List category choice
					 void *pUserData);
  public:void Refresh()
    {
	m_pToDoListList->Refresh();
	m_pAddressBookList->Refresh();
	m_pScheduleContainer->Refresh(m_nDate);
    }
};


#endif /*  */
