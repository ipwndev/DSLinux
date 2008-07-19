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
// A class for what looks like a spin control.                  //
//--------------------------------------------------------------//
#ifndef SPININPUT_H_

#define SPININPUT_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <cstdlib>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Pixmap.H>
class SpinInput:public Fl_Group
{
  public:SpinInput(int nX,	// Default constructor
	      int nY, int nWidth, int nHeight, const char *pszLabel,
	      int nMaxSize, int nMinimum, int nMaximum);
     ~SpinInput();		// Destructor
    inline int value()		// Get the value of this widget
    {
	return (atoi(m_pInput->value()));
    }
    void value(int nValue);	// Set the value of this widget
  private:Fl_Int_Input * m_pInput;
    // The input area
    Fl_Pixmap *m_pDownPixmap;	// The Down Button pixmap
    Fl_Pixmap *m_pUpPixmap;	// The Up Button pixmap
    int m_nMax;			// The maximum value
    int m_nMin;			// The minimum value
    static void OnDownButton(Fl_Widget * pWidget,	// Down button was clicked
			     void *pUserData);
    static void OnUpButton(Fl_Widget * pWidget,	// Up button was clicked
			   void *pUserData);
};


#endif /*  */
