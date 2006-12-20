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
// Class for a box with an image and which tracks mouse         //
// movements and allows left mouse clicks on the image.         //
//--------------------------------------------------------------//
#ifndef IMAGEBOX_H_

#define IMAGEBOX_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pixmap.H>
#include "Messages.h"
using namespace std;
class ImageBox:public Fl_Group
{
  public:ImageBox(int nX,	// Constructor
	     int nY, int nWidth, int nHeight, Fl_Pixmap * pPixmap,
	     PixilDTMessage nMessage, const char *pszPrompt);
     ~ImageBox();		// Destructor
    int Message(PixilDTMessage nMessage,	// Processing from the parent's callback function
		int nInfo);
  protected:void draw();	// Draw this widget
    int handle(int nEvent);	// Handle events in this widget
  private:  bool m_bMouseDown;
    // Is the mouse down for this widget
    bool m_bMouseIn;		// Is the mouse in this widget
    Fl_Color m_nDownColor;	// The "down" background color
    Fl_Color m_nNormalColor;	// The original background color
    Fl_Pixmap *m_pPixmap;	// Pointer to the image
    enum PixilDTMessage m_nMessage;	// Message to issue when selected
      std::string m_strPrompt;	// Prompt for the image
    void PopDown();		// Pop the image down
    void PopUp();		// Pop the image up
};


#endif /*  */
