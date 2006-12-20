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


#ifndef		NXABOUT_INCLUDED
#define		NXABOUT_INCLUDED	1

// System header files


// Local header files
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Editor.H>
#include <FL/Fl_Window.H>
#include <nxbox.h>
//#include <nxmultilineinput.h>
#include "nxabout.h"
#include "sysconf.h"
#include <sysconfig.h>


// Typedefs, macros, enum/struct/union definitions

// Array indices for the widget fields
#define		ABOUT_OSINFO		0	// Operating system
#define		ABOUT_OEINFO		1	// Operating environment
#define		ABOUT_CPUINFO		2	// CPU info
#define		ABOUT_MEMINFO		3	// Memory installed
#define		ABOUT_PCMCIA		4	// PCMCIA widget
#define		ABOUT_USERINFO		5	// User information
#define		ABOUT_DUMMY			6	// Just a placeholder to get the size of array

#define		UTILITY_PAR_NAME "about"
#define		UTILITY_NAME	"About"

// Global variables


// Class definition
class NxAbout
{
  public:
    NxAbout(int X, int Y, int W, int H, char *);
     ~NxAbout();		// Class destructor

    void ShowWindow(void);
    void HideWindow(void);

  private:
    // Private data
      AppMode_t _appmode;	// Mode of application
    int _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    NxBox *_info_wid[ABOUT_DUMMY];	// Info widgets

    void GetPluginData(void);	// Get the application data
    void FormatString(char *string, int w, int idx);

    void MakeWindow(int, int, int, int);
    void SetWidths(void);	// Sets the widths for labels/data
    void SetValues(void);
};				// end of class NxAbout

#endif //      NXABOUT_INCLUDED
