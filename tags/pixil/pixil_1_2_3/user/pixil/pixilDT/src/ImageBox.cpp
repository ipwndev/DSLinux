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
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "FLTKUtil.h"
#include "ImageBox.h"
#include "LeftGroup.h"

#include <cstdio>

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ImageBox::ImageBox(int nX,
		   int nY,
		   int nWidth,
		   int nHeight,
		   Fl_Pixmap * pPixmap,
		   PixilDTMessage nMessage, const char *pszPrompt)
    :
Fl_Group(nX, nY, nWidth, nHeight, "")
{
    int i;
    unsigned int nColor;
    unsigned int nComponent;
    unsigned int nNewColor;

    // Save the prompt string
    m_strPrompt = pszPrompt;

    // Set the color for this group
    m_nNormalColor = parent()->color();
    color(m_nNormalColor);
    box(FL_UP_BOX);

    // Generate the down color
    nColor = Fl::get_color(m_nNormalColor);
    for (nNewColor = 0, i = 0; i < 24; i += 8) {
	nComponent = ((((nColor >> (24 - i)) & 0xff) + 0x100) >> 1);
	nNewColor += (nComponent << i);
    }
    m_nDownColor = GetFLTKColor(nNewColor);

    // No child widgets
    end();

    // Create the image
    m_pPixmap = pPixmap;
    m_pPixmap->label(this);
    align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);

    // Save the notification message number
    m_nMessage = nMessage;

    // Display this widget
    show();

    // Initialize mouse tracking
    m_bMouseDown = m_bMouseIn = false;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ImageBox::~ImageBox()
{
    delete m_pPixmap;
}


//--------------------------------------------------------------//
// Draw this widget.                                            //
//--------------------------------------------------------------//
void
ImageBox::draw()
{
    int nHeight;
    int nWidth;

    // Draw the box
    draw_box();

    // Draw the image
    fl_measure_pixmap(m_pPixmap->data, nWidth, nHeight);
    m_pPixmap->draw(x() + ((w() - nWidth) >> 1), y() + 6);

    // Draw the text
    fl_font(labelfont(), (3 * labelsize()) >> 2);
    fl_color(FL_BLACK);
    fl_draw(m_strPrompt.c_str(), x(), y() + h() - 14, w(), 14, FL_ALIGN_CLIP);
}


//--------------------------------------------------------------//
// Event handler                                                //
//--------------------------------------------------------------//
int
ImageBox::handle(int nEvent)
{
    int nReturn = Fl_Group::handle(nEvent);

    switch (nEvent) {
    case FL_ENTER:
	nReturn = 1;
	m_bMouseIn = true;
	if (m_bMouseDown == true) {
	    PopDown();
	}
	break;

    case FL_LEAVE:
	nReturn = 1;
	m_bMouseIn = false;
	if (m_bMouseDown == true) {
	    PopUp();
	    m_bMouseDown = false;
	}
	break;

    case FL_PUSH:
	PopDown();
	nReturn = 1;
	m_bMouseDown = true;
	dynamic_cast < LeftGroup * >(parent())->Notify(m_nMessage, 0);
	break;

    case FL_RELEASE:
	nReturn = 1;
	m_bMouseDown = false;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Process notifications from the parent widget.  This allows   //
// this widget to act like a radio button, if the passed        //
// message is not the message number for this widget then it    //
// will be popped up.                                           //
//--------------------------------------------------------------//
int
ImageBox::Message(PixilDTMessage nMessage, int nInfo)
{
    int nReturn = 0;		// Default return value

    // If the message number does not match this widget's message then pop the button up
    if (nMessage != m_nMessage) {
	PopUp();
    } else {
	PopDown();
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Pop this widget down.                                        //
//--------------------------------------------------------------//
void
ImageBox::PopDown()
{
    box(FL_DOWN_BOX);
    color(m_nDownColor);
    redraw();
}


//--------------------------------------------------------------//
// Pop this widget up.                                          //
//--------------------------------------------------------------//
void
ImageBox::PopUp()
{
    box(FL_UP_BOX);
    color(m_nNormalColor);
    redraw();
}
