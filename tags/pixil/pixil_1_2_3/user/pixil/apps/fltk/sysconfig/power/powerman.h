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


#ifndef		POWERMAN_INCLUDED
#define		POWERMAN_INCLUDED	1

// System header files


// Local header files
#include "nxpwr.h"
#include <FL/Fl_Window.H>
#include <nxbutton.h>
#include <nxcheckbutton.h>
#include <nxmenubutton.h>
#include <nxoutput.h>
#include <nxslider.h>
#include <sysconf.h>

#include <sysconf_plugin.h>

// Typedefs, macros, enum/struct/union definitions

#define		UTILITY_PAR_NAME "power"
#define 	UTILITY_NAME	"Power Management"

#define		BAT_IDX			0
#define		AC_IDX			1

typedef struct
{
    bool pwr_off;		// Flag that user wishes power control
    int timeval;		// Time value to turn device off
    char dirty_flg;		// Flag to indicate a change in db
}
pwr_settings_t;
#define					PWR_DFLGS_PWRCB			0x01	// Power check button value has changed
#define					PWR_DFLGS_TMVAL			0x02	// Power timevalue has changed

// Class definition
class NxPowerman
{
  public:
    //Constructor and destructor
    NxPowerman(int X, int Y, int W, int H, char *appname);
     ~NxPowerman();

    void ShowWindow(void);
    void HideWindow(void);

  private:
    // Private data
      AppMode_t _appmode;	// Application mode
    pwr_settings_t _pwr_settings[2];	// Pwr settings
    int _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    NxMenuButton *_mbtns[2];	// Menu buttons (for timeout values)
    NxCheckButton *_cbtns[2];	// Check Buttons (control menu buttons)
    NxSlider *_batsl;		// Battery slider
    NxOutput *_batstate;	// Battery state message

    // Private functions
    void GetAppPrefs(void);
    void MakeWindow(int X, int Y, int W, int H);
    void SetAppPrefs(void);
    void SetValues(void);

    // Private static widget callback functions
    static void cb_cb(Fl_Widget * w, void *d);	// Call back for the check buttons
    static void mb_cb(Fl_Widget * w, void *d);	// Call back for the menu buttons
    static void save_exit_cb(Fl_Widget * w, void *d);	// Call back to save/exit the app/utility
    static void sl_upd_tmr(void *d);	// Call back to update the Slider values

};

#endif
