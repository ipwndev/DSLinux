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
// Find Dialog.                                                 //
//--------------------------------------------------------------//
#ifndef FINDDLG_H_

#define FINDDLG_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <vector>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>
#include "FindList.h"
#include "Messages.h"
#include "NxDbAccess.h"
using namespace std;

//--------------------------------------------------------------//
// Information about rows found so far                          //
//--------------------------------------------------------------//
class FindRow
{
  public:inline FindRow()	// Default constructor
    {
    }
    inline FindRow(const FindRow & Other)	// Copy constructor
    {
	*this = Other;
    }
    inline ~ FindRow()		// Destructor
    {
    }
    inline FindRow & operator=(const FindRow & Other)	// Assignment operator
    {
	m_nPhysicalRow = Other.m_nPhysicalRow;
	m_nType = Other.m_nType;
	m_strKeyValue = Other.m_strKeyValue;
	return (*this);
    }

    // All data is public just like when this used to be a structure
    int m_nPhysicalRow;		// Physical row of of the found item
    PixilDTMessage m_nType;	// Type of row found
    string m_strKeyValue;	// Key value of the record - name, description, etc...
};


//--------------------------------------------------------------//
// The Find dialog class.                                       //
//--------------------------------------------------------------//
class FindDlg:public Fl_Window
{
  public:enum FindAction
    { ActionCancel = 0, ActionGoTo,
    };
      FindDlg(Fl_Widget * pParent);	// Constructor
     ~FindDlg();		// Destructor
    int DoModal();		// Run the modal dialog
    inline int GetAction() const	// Get the action selected by the dialog
    {
	return (m_nAction);
    }
    inline const string & GetFindKey(int nRow)	// Get the string key of a found item
    {

#ifdef DEBUG
	assert(nRow >= 0 && nRow < (int) m_vFindRow.size());

#endif				/*  */
	return (m_vFindRow[nRow].m_strKeyValue);
    }
    inline int GetFindRows()	// Get the number of rows found
    {
	return (m_vFindRow.size());
    }
    inline int GetFindType(int nRow)	// Get the type of a found item
    {

#ifdef DEBUG
	assert(nRow >= 0 && nRow < (int) m_vFindRow.size());

#endif /*  */
	return (m_vFindRow[nRow].m_nType);
    }
    inline int GetRecno() const	// Get the physical record number of the record selected for GoTo
    {
	return (m_nRecno);
    }
    inline PixilDTMessage GetType() const	// Get the type of information being selected
    {
	return (m_nType);
    }
    void Notify(PixilDTMessage nMessage,	// Notification from a child widget
		int nInfo);
  private:FindList * m_pResults;
    // Where the search results are stored
    Fl_Button *m_pCancelButton;	// Cancel button
    Fl_Button *m_pGoToButton;	// GoTo button
    Fl_Button *m_pHelpButton;	// Help button
    Fl_Button *m_pRadioButton[2];	// The radio buttons
    Fl_Button *m_pSearchButton;	// Search button
    Fl_Input *m_pInput;		// The search string
    int m_nAction;		// The final dialog action, Cancel or GoTo
    int m_nRecno;		// Physical record number selected for GoTo
    PixilDTMessage m_nType;	// The message for the type of information selected
    vector < FindRow > m_vFindRow;	// Rows found so far
    static string GetAddressKey(NxDbAccess * pDB,	// Get address book key
				int nRow);
    static string GetNoteKey(NxDbAccess * pDB,	// Get notes key
			     int nRow);
    static string GetSchedulerKey(NxDbAccess * pDB,	// Get Scheduler key
				  int nRow);
    static string GetToDoKey(NxDbAccess * pDB,	// Get ToDo List key
			     int nRow);
    static void OnHelpButton(Fl_Widget * pWidget,	// Process click on the Help button
			     void *pUserData);
    static void OnSearchButton(Fl_Widget * pWidget,	// Process click on the search button
			       void *pUserData);
    void Search(NxDbAccess * pDB,	// Search a data base
		PixilDTMessage nType,
		string(*pfnGetKeyValue) (NxDbAccess *, int));
};


#endif /*  */
