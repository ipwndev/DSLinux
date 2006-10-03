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
// Main application for PixilDT prototype.                      //
//--------------------------------------------------------------//
#ifndef PIXILDT_H_

#define PIXILDT_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <FL/Fl_Window.H>
#include "PixilMainWnd.h"
class PixilDT
{
  public:enum DateFormat	// Date formating types
    { DATE_DMY = 0, DATE_DYM, DATE_MDY, DATE_MYD, DATE_YDM, DATE_YMD,
    };
      PixilDT(int argc, char **argv);	// Constructor
     ~PixilDT();		// Destructor
    inline static void GetAMPM(string & strAM,	// Get the strings used for AM and PM
			       string & strPM)
    {
	strAM = m_pThis->m_strAM;
	strPM = m_pThis->m_strPM;
    }
    inline static PixilDT *GetApp()	// Get a pointer to the one and only app class
    {
	return (m_pThis);
    }
    inline PixilMainWnd *GetMainWindow() const	// Get the main application window
    {
	return (m_pMainWindow);
    }
    inline static DateFormat GetDateFormat()	// Get the locale's date formatting
    {
	return (m_pThis->m_nDateFormat);
    }
    inline static char GetDateSeparator()	// Get the date separator character
    {
	return (m_pThis->m_cDateSeparator);
    }
    inline static char GetTimeSeparator()	// Get the time separator character
    {
	return (m_pThis->m_cTimeSeparator);
    }
    static void SetPixilIcon(Fl_Window * pWindow);	// Set the Pixil Icon for a window
    void ShowHelp(int nHelpID);	// Show help for a given help ID
  private:char m_cDateSeparator;
    // The date separator character
    char m_cTimeSeparator;	// The time separator character
    DateFormat m_nDateFormat;	// The current locale's date formatting
    PixilMainWnd *m_pMainWindow;	// Container window
    static PixilDT *m_pThis;	// Singleton pointer
    string m_strAM;		// Code for AM times
    string m_strPM;		// Code for PM times
    void DetermineDateFormat();	// Determine which date format is in use for the locale
};


#endif /*  */
