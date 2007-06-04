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


#ifndef		NXUSERINFO_INCLUDED
#define		NXUSERINFO_INCLUDED	1

// System header files


// Local header files
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Editor.H>
#include <nxbox.h>
#include <nxbutton.h>
#include <nxinput.h>
#include "nxui.h"

#define		UTILITY_PAR_NAME "userinfo"
#define		UTILITY_NAME	"User Info"

typedef struct
{
    unsigned char fld_name;	// Name of the input field
    unsigned short dta_flags,	// Data flags (defined below)
      dta_sz;			// Length of input data
    Fl_Widget *input;		// Input widget (could be ml or sl)
}
UserInput_t;

// Defines for the field names
#define			UI_FLD_NAME					1	// Name field
#define			UI_FLD_COMPANY				2	// Company field
#define			UI_FLD_ADDRESS				3	// Address field
#define			UI_FLD_PHONE				4	// Phone field
#define			UI_FLD_EMAIL				5	// Email field
#define			UI_FLD_NOTES				6	// Notes field
#define			UI_FLD_MAXFLDS				6	// Number of fields

// Defines for dta_flags member of UserInput_t
#define			UI_DFLAGS_SL				0x0001	// Single line input
#define			UI_DFLAGS_ML				0x0002	// Multi line input
#define			UI_DFLAGS_ALPHA				0x0004	// Allows alpha text
#define			UI_DLFAGS_EMAIL				0x0008	// Email formatting (and allowable characters)
#define			UI_DFLAGS_NUMERIC			0x0010	// Allows numeric tabs
#define			UI_DFLAGS_PUNC				0x0020	// Allows punctuation tabs
#define			UI_DFLAGS_WHITESP			0x0040	// Allows white space
#define			UI_DFLAGS_DIRTY				0x8000	// The value has changed

// More useful groupings of DFLAGS...
#define			UI_GENERIC			(UI_DFLAGS_ALPHA | \
									 UI_DFLAGS_NUMERIC | \
									 UI_DFLAGS_PUNC | \
									 UI_DFLAGS_WHITESP)
#define			UI_APW				(UI_DFLAGS_ALPHA | \
									 UI_DFLAGS_PUNC | \
									 UI_DFLAGS_WHITESP)

class NxUserInfo
{
  public:
    NxUserInfo(int X, int Y, int W, int H, char *appname);
     ~NxUserInfo();

    void ShowWindow(void);
    void HideWindow(void);

  private:
    //AppMode_t _appmode;         // Mode of application
    int _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate
    UserInput_t _Inputs[UI_FLD_MAXFLDS];	// Array of all inputs (for both tabs)

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    Fl_Tabs *_mainTab;		// Tabbed view
    Fl_Group *_userInfog,	// User info tab group
     *_notesg;			// Notes group
    NxButton *_resetb,		// Discard changes
     *_saveb;			// Save changes

    void GetAppPrefs(void);	// Get the application prefrences
    void MakeWindow(int X, int Y, int W, int H);	// Creates the window/widgets
    void SetAppPrefs(void);	// Stores the application preferences
    // the current state
    // Private static widget callbacks
    static void input_cb(Fl_Widget * w, void *d);	// Call back for any input (char by char)
    static void save_reset_cb(Fl_Widget * w, void *d);	// Call back to save/reset values

};				// end of class NxUserInfo

#endif //      NXUSERINFO_INCLUDED
