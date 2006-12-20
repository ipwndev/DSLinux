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
// Class for the main window's toolbar for the Pixil Desktop.   //
//--------------------------------------------------------------//
#ifndef TOOLBAR_H_

#define TOOLBAR_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
#include <FL/Fl_Group.H>
#include "ToolbarButton.h"
using namespace std;
typedef ToolbarButton *PTOOLBARBUTTON;

//--------------------------------------------------------------//
// External structure for item definitions.                     //
//--------------------------------------------------------------//
typedef struct tagToolbarItem
{
    int m_nType;		// The type of entry
    const char *m_pszTooltip;	// Tooltip text
    Fl_Callback *m_pfnCallback;	// The callback if this is selected
    int m_nImage;		// The image for this button
    int m_nDisabledImage;	// The image when disabled
}
ToolbarItem;

//--------------------------------------------------------------//
// Values for the button type.                                  //
//--------------------------------------------------------------//
#define TOOLBAR_END    0
#define TOOLBAR_BUTTON 1
#define TOOLBAR_GAP    2

//--------------------------------------------------------------//
// The class.                                                   //
//--------------------------------------------------------------//
class Toolbar:public Fl_Group
{
  public:Toolbar(int nX,	// Constructor
	    int nY, int nWidth, int nHeight,
	    const ToolbarItem * pToolbarItem);
     ~Toolbar();		// Destructor
    void Enable(int nEnabledImage,	// Enable or disable a button
		bool bEnable = true);
  private:int m_nCount;	// Number of ToolbarButtons
    ToolbarButton **m_ppToolbarButton;	// The internal copy of the toolbar items
    static void Callback(Fl_Widget * pWidget,	// Translate widget involved in the callback
			 void *pUserData);
};


#endif /*  */
