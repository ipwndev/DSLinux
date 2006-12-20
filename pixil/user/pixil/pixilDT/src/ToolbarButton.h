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
// Class for a button on a toolbar object.                      //
//--------------------------------------------------------------//
#ifndef TOOLBARBUTTON_H_

#define TOOLBARBUTTON_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <cstdio>
#include <string>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>
using namespace std;

#define TOOLBAR_BUTTON_WIDTH 22
class ToolbarButton:public Fl_Button
{
  public:ToolbarButton(int nX,
		  // Constructor
		  int nY, int nImage, int nDisabledImage,
		  const char *pszTooltip);
     ~ToolbarButton();		// Destructor
    inline void DoCallback()	// Do the real callback with the parent as the sending widget
    {
	(*m_pfnCallback) (parent(), NULL);
    }
    void Enable(bool bEnable = true);	// Enable or disable this button
    inline int GetNormalImage() const	// Get the normal image index
    {
	return (m_nNormalImage);
    }
    inline static int GetWidth()	// Get the default button width
    {
	return (TOOLBAR_BUTTON_WIDTH);
    }
    inline void SetCallback(Fl_Callback * pfnCallback)	// Set the real callback
    {
	m_pfnCallback = pfnCallback;
    }
  private:bool m_bEnabled;	// Is this button enabled or not
    Fl_Callback *m_pfnCallback;	// The real callback
    Fl_Pixmap *m_pPixmap;	// The image for this button
    int m_nNormalImage;		// The normal image
    int m_nDisabledImage;	// The disabled image
    std::string m_strTooltip;	// The tooltip text
};


#endif /*  */
