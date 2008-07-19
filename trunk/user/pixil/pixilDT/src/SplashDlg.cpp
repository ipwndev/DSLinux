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
// Splash Screen (dialog).                                      //
//--------------------------------------------------------------//
#include <cstdio>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/x.H>
#include "PixilDT.h"
#include "SplashDlg.h"

#include "VCMemoryLeak.h"


// These should be close to the image's true size
#define DLG_HEIGHT 340
#define DLG_WIDTH  480


//--------------------------------------------------------------//
// The splash image                                             //
//--------------------------------------------------------------//
#include "images/pix_splash.xpm"


//--------------------------------------------------------------//
// Singleton pointer                                            //
//--------------------------------------------------------------//
SplashDlg *
    SplashDlg::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
SplashDlg::SplashDlg(float fSeconds, bool & bTimedOut)
    :
Fl_Window((Fl::w() - DLG_WIDTH) / 2,
	  (Fl::h() - DLG_HEIGHT) / 2, DLG_WIDTH, DLG_HEIGHT)
{
    Fl_Box *pBox;

#ifdef WIN32
    // Set the window icon
    // This has to be done here as this window is shown prior to the
    // main window, but will cause the class "FLTK" to be registered
    // for the WIN32 version of the code.
    PixilDT::GetApp()->SetPixilIcon(this);
#endif

    m_pThis = this;
    pBox = new Fl_Box(Fl::box_dw(FL_DOWN_BOX),
		      Fl::box_dh(FL_DOWN_BOX),
		      w() - 2 * Fl::box_dw(FL_DOWN_BOX),
		      h() - 2 * Fl::box_dh(FL_DOWN_BOX));
    m_pPixmap = new Fl_Pixmap(pix_splash);
    m_pPixmap->label(pBox);
    end();

    // Show the window
    bTimedOut = false;
    m_pbTimedOut = &bTimedOut;
    clear_border();
    set_modal();
    show();

    // Wait for the selected amount of time
    Fl::add_timeout(fSeconds, Timeout, NULL);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
SplashDlg::~SplashDlg()
{
    delete m_pPixmap;
}


//--------------------------------------------------------------//
// Close this window after the timeout                          //
//--------------------------------------------------------------//
void
SplashDlg::Timeout(void *pUserData)
{
    *(m_pThis->m_pbTimedOut) = true;
    m_pThis->do_callback();
}
