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
// FLTK utilities                                               //
//--------------------------------------------------------------//
#ifndef FLTKUTIL_H_

#define FLTKUTIL_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
#include <utility>
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
using namespace std;
void BreakLines(const char *pszString,	// Break a string into lines delimited by new line characters
		vector < string > &vLine);
Fl_Menu_Item *CreateChoice(int nCount,	// Create a translated menu for an Fl_Choice
			   const char *const *ppszText, bool bTranslateMenu);
int DoModal(Fl_Window * pWindow,	// Run a modal dialog
	    Fl_Button * pOKButton, Fl_Button * pCancelButton);
int DoPopupMenu(const Fl_Menu_Item * pMenuItem,	// Show a popup menu
		int nX, int nY);
void FreeTranslatedMenu(Fl_Menu_Item * pMenuItem);	// Free a menu created by TranslateMenuItems
Fl_Color GetFLTKColor(int nRGB);	// Get an FLTK color
Fl_Color GetFLTKColor(int nRed, int nGreen, int nBlue);	// Get an FLTK color
void SetBackgroundColor();	// Set the window backgound color (FL_GRAY)
Fl_Menu_Item *TranslateMenuItems(const Fl_Menu_Item * pMenu_Item);	// Translate text in a set of const menu items
Fl_Menu_Item *TranslateMenuItems(Fl_Menu_Item * pMenu_Item);	// Translate text in a set of non-const menu items
string WrapText(const char *pszText,	// Get the first part of a line of text
		int nMaxWidth, Fl_Widget * pWidget);

// WIN32 debugging routines
#ifdef WIN32
#ifdef _DEBUG
extern "C"
{
    __declspec(dllimport) void __stdcall OutputDebugStringA(const char
							    *pOutputString);
}

#define OutputDebugString  OutputDebugStringA

#endif				// _DEBUG
#endif				// WIN32

#endif				/*  */
