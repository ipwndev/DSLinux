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
// Global application options.                                  //
//                                                              //
// Options maintained here are:                                 //
//                                                              //
// Application:                                                 //
//  ConfirmDelete - Yes or No                                   //
//  MainPage - 0 through 3 for Scheduler, Address Book, To Do   //
//       List or Notes to be displayed first.                   //
//  SchedulerPage - 0 through 3 for daily, weekly, monthly, or  //
//       yearly page to be displayed first.                     //
//                                                              //
// Database:                                                    //
//  Path - path to the database                                 //
//                                                              //
// Scheduler                                                    //
//  DayBegins - 24 hour based hour that the workday begins      //
//  WeekBegins - 0 for Sunday or 1 for Monday                   //
//                                                              //
// Search:                                                      //
//  String - last search string used                            //
//                                                              //
// ToDoList:                                                    //
//  ShowCategory - Yes or No                                    //
//  ShowCompleted - Yes or No                                   //
//  ShowDueDate - Yes or No                                     //
//  ShowOnlyDue - Yes or No                                     //
//  ShowPriority - Yes or No                                    //
//  Sort - 0 through 3 for sort by priority/due date, due       //
//       date/priority, category/priority, or category/due date //
//--------------------------------------------------------------//
#ifndef OPTIONS_H_

#define OPTIONS_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
#include <FL/Fl_Widget.H>
using namespace std;

// Turn Windows Profile API on or off
#ifdef WIN32
#define WIN32_PROFILE_API	// Undefine this to use the Linux version of the code
#else /*  */
#undef WIN32_PROFILE_API
#endif /*  */
class Options
{
  public:Options();		// Constructor
    ~Options();			// Destructor
    static void Destroy();	// Destroy the singleton Options object
    inline static bool GetConfirmDelete()	// Get the ToDo List's Show Categories setting
    {
	return (GetBooleanValue("Application", "ConfirmDelete", true));
    }
    inline static string GetDatabasePath()	// Get the path to the database
    {
	return (GetStringValue("Database", "Path", "."));
    }
    inline static int GetDayBegins()	// Get the beginning hour of the day for scheduler displays
    {
	return (GetIntValue("Scheduler", "DayBegins"));
    }
    inline static string GetHelpPath()	// Get the help file directory
    {
	return (GetStringValue("Help", "Path", "."));
    }
    inline static int GetMainPage()	// Get the main page to be displayed - 0 through 3
    {
	return (GetIntValue("Application", "MainPage"));
    }
    inline static int GetSchedulerPage()	// Get the scheduler page to be displayed - 0 through 3
    {
	return (GetIntValue("Application", "SchedulerPage"));
    }
    inline static string GetSearchString()	// Get the last search string used
    {
	return (GetStringValue("Search", "String", ""));
    }
    static string GetStringValue(const char *pszSection,
				 const char *pszSetting,
				 const char *pszDefault);
    inline static bool GetToDoShowCategory()	// Get the ToDo List's Show Categories setting
    {
	return (GetBooleanValue("ToDoList", "ShowCategory", false));
    }
    inline static bool GetToDoShowCompleted()	// Get the ToDo List's Show Completed items setting
    {
	return (GetBooleanValue("ToDoList", "ShowCompleted", false));
    }
    inline static bool GetToDoShowDueDate()	// Get the ToDo List's Show Due Dates setting
    {
	return (GetBooleanValue("ToDoList", "ShowDueDate", false));
    }
    inline static bool GetToDoShowOnlyDue()	// Get the ToDo List's Show Only Due setting
    {
	return (GetBooleanValue("ToDoList", "ShowOnlyDue", false));
    }
    inline static bool GetToDoShowPriority()	// Get the ToDo List's Show Priority setting
    {
	return (GetBooleanValue("ToDoList", "ShowPriority", false));
    }
    inline static int GetToDoSort()	// Get the ToDo List's Show Priority setting
    {
	return (GetIntValue("ToDoList", "Sort"));
    }
    inline static int GetWeekBegins()	// Get the beginning day of the week for scheduler displays
    {
	return (GetIntValue("Scheduler", "WeekBegins"));
    }
    inline static void SetConfirmDelete(bool bConfirmDelete)	// Set the confirm delete flag
    {
	SetBooleanValue("Application", "ConfirmDelete", bConfirmDelete);
    }
    inline static void SetDatabasePath(const char *pszString)	// Set the database path
    {
	SetStringValue("DataBase", "Path", pszString);
    }
    inline static void SetDayBegins(int nHour)	// Set the main page to be displayed
    {
	SetIntValue("Scheduler", "DayBegins", nHour);
    }
    inline static void SetMainPage(int nPage)	// Set the main page to be displayed
    {
	SetIntValue("Application", "MainPage", nPage);
    }
    inline static void SetSchedulerPage(int nPage)	// Set the scheduler page to be displayed
    {
	SetIntValue("Application", "SchedulerPage", nPage);
    }
    inline static void SetSearchString(const char *pszString)	// Set the last search string used
    {
	SetStringValue("Search", "String", pszString);
    }
    static void SetStringValue(const char *pszSection,	// Set a string value into the INI file
			       const char *pszSetting, const char *pszValue);
    inline static void SetToDoShowCategory(bool bShowCategory)	// Set the ToDo List's Show Categories setting
    {
	SetBooleanValue("ToDoList", "ShowCategory", bShowCategory);
    }
    inline static void SetToDoShowCompleted(bool bShowCompleted)	// Set the ToDo List's Show Completed items setting
    {
	SetBooleanValue("ToDoList", "ShowCompleted", bShowCompleted);
    }
    inline static void SetToDoShowDueDate(bool bShowDueDate)	// Set the ToDo List's Show Due Dates setting
    {
	SetBooleanValue("ToDoList", "ShowDueDate", bShowDueDate);
    }
    inline static void SetToDoShowOnlyDue(bool bShowOnlyDue)	// Set the ToDo List's Show Only Due setting
    {
	SetBooleanValue("ToDoList", "ShowOnlyDue", bShowOnlyDue);
    }
    inline static void SetToDoShowPriority(bool bShowPriority)	// Set the ToDo List's Show Priority setting
    {
	SetBooleanValue("ToDoList", "ShowPriority", bShowPriority);
    }
    inline static void SetToDoSort(int nSort)	// Set the ToDo List's Sort order setting
    {
	SetIntValue("ToDoList", "Sort", nSort);
    }
    inline static void SetWeekBegins(int nHour)	// Set the main page to be displayed
    {
	SetIntValue("Scheduler", "WeekBegins", nHour);
    }
  private:static Options *m_pThis;
    // Singleton pointer
    string m_strIniFile;	// Name of the INI file
    int m_UserIniFile;

#ifndef WIN32_PROFILE_API
    void *m_pINIFileReader;	// Pointer to the INI File Reader object
#endif /*  */
    static bool GetBooleanValue(const char *pszSection,	// Get a boolean value from the INI file
				const char *pszSetting, bool bDefault);
    static int GetIntValue(const char *pszSection,	// Get an integer value from the INI file
			   const char *pszSetting);
    static void SetBooleanValue(const char *pszSection,	// Set a boolean value into the INI file
				const char *pszSetting, bool bValue);
    static void SetIntValue(const char *pszSection,	// Set an integer value into the INI file
			    const char *pszSetting, int nValue);
    static void SetIniFileName();	// Get the INI file name
};


#endif /*  */
