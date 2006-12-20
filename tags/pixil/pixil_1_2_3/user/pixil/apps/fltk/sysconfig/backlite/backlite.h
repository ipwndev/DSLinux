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


#ifndef		NXBACKLITE_INCLUDED
#define		NXBACKLITE_INCLUDED	1

// System header files


// Local header files
#include <nxcheckbutton.h>
#include <nxmenubutton.h>
#include <nxmloutput.h>
#include <nxoutput.h>
#include <nxvalueslider.h>
#include "nxbl.h"

// Typedefs, macros, enum/struct/union definitions

#define		UTILITY_PAR_NAME "backlight"
#define		UTILITY_NAME	"BackLight Config"

#define DPRINTF(str, args...) fprintf(stderr, str, ## args)

typedef enum
{
    blUNKNOWN = -1,		// Unknown mode (need to query system)
    blBATTERY,			// Battery mode
    blACPOWER			// External power source
}
bl_mode_t;

typedef struct
{
    signed int tmout_val,	// Time out value (in seconds)
      brite_val;		// Brightness percentage (0 - 100)
    bool wake_up;		// Wake up on press/tap
    char dirty_flgs;		// Flags to determine dirty values
}
bl_settings_t;

// Values for bl_settings_t.dirty_flgs
#define			BLVAL_DFLGS_BRITE		0x0001	// The Brightness level has changed
#define			BLVAL_DFLGS_TMOUT		0x0002	// The Timeout value has changed
#define			BLVAL_DFLGS_WAKEUP		0x0004	// The Wake up on tap/press value has changed


class NxBacklite
{
  public:
    NxBacklite(int, int, int, int, char *appname);
     ~NxBacklite();

    void ShowWindow(void);
    void HideWindow(void);

  private:
      bl_mode_t _blMode;	// Back lite mode (battery/power)
    bl_settings_t _bl_settings[2];	// COntains the settings
    int _winX, _winY;

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    NxMenuButton *_mbmode,	// Mode selection menu button
     *_mbtimeunit[2];		// Time unit selection menu button
    NxCheckButton *_pwrdn_ck,	// Time check
     *_wake_ck;			// Wake-up timer
    NxValueSlider *_brightness;	// Level widget for the brightness
    NxMlOutput *_bat_warning;	// Battery warning ml output widget
    NxOutput *_pwr_lnk;		// Power app link

    void GetAppPrefs(void);	// Get the application prefrences
    void MakeWindow(int, int, int, int);	// Creates the window/widgets
    void SetAppPrefs(void);	// Stores the application preferences
    void SetBl(void);		// Sets the backlite mode
    void SetValues(void);	// Set the values for the widgets based on

    static void brite_sel_cb(Fl_Widget * w, void *d);	// Call back for brightness control
    static void mode_sel_cb(Fl_Widget * w, void *d);	// Call back for mode selection
    static void pwrlnk_cb(Fl_Widget * w, void *d);
    static void pwrdn_ck_cb(Fl_Widget * w, void *d);	// Call back for the power_down check box
    static void save_exit_cb(Fl_Widget * w, void *d);	// Call back for ok/cancel buttons
    static void tm_sel_cb(Fl_Widget * w, void *d);	// Call back for time seletion
    static void wake_toggle_cb(Fl_Widget * w, void *d);	// Call back for the wake-up check button
};


#endif
