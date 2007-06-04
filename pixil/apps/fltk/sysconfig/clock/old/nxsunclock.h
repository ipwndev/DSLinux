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



#ifndef		NXSUNCLOCK_INCLUDED
#define		NXSUNCLOCK_INCLUDED		1

// System header files


// Local header files
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Calendar.H>
#include <FL/Fl_Image.H>
#include <nximage.h>
#include <nxapp.h>
#include <nxbutton.h>
#include <nxmenubutton.h>
#include <nxoutput.h>
#include <nxtminput.h>
#include "sysconf.h"
#include <sysconfig.h>

// Typedefs, macro, enum/struct/union definitions
typedef struct
{
    unsigned char red,		// Red pixel value
      green,			// Green pixel value
      blue,			// Blue pixel value
      alpha;			// Alpha blend value
}
RGBpixel;

typedef enum
{
    tzCTYQUERY = 0,		// Query the time at the specified city
    tzCTYSET			// Sets the current timezone (either home/visiting)
}
tz_ptmode_enum;

typedef enum
{
    tzHOME = 0,			// Affects the home timezone only
    tzVISIT			// Affects the Visiting timezone only
}
tz_mode_enum;

typedef struct
{
    char name[25 + 1],		// City name
      reg[2 + 1],		// State/Country 2 char value
      tz[64];			// Timezone stuff
}
city_rec_t;

typedef struct
{
    city_rec_t cities[2];	// Home/Visiting city records
    bool home_current;		// Flag detailing which is current
    unsigned char dirty_flgs;	// Flags indicating what has changed and needs
    // to be written back out to PAR
}
app_value_t;

// Values for app_value_t.dirty_flgs
#define			APP_DFLGS_HOME		0x01	// Home city info has changed
#define			APP_DFLGS_VISIT		0x02	// Visit city info has changed
#define			APP_DFLGS_CUR		0x04	// Current active city has changed

#define			UTILITY_PAR_NAME	"worldclock"
//#define                       UTILITY_NAME    "WorldClock"

// Class definition
class nxSunclock
{
  public:
    // Constructor // destructor
    ~nxSunclock();		// Class destructor
    nxSunclock(int X, int Y, int W, int H, char *appname);	// Class constructor
    void *GetWindow(void);

  private:
    // Private data
    unsigned char *_sun_array,	// Buffer of modified day/night map data
     *_map_array;		// Buffer of original map data
    char *_grImage,		// Ptr to the image path
     *_zndb;			// Ptr to the zone database
    tz_ptmode_enum _ePtMode;	// Current point mode (query/set)
    tz_mode_enum _eCurTzMode;	// Current mode (home/visiting)
    AppMode_t _appmode;		// Current app mode
    int _clckedit,		// clock edit mode
      _winX,			// Windows TL X value
      _winY;			// Windows TL Y value
    app_value_t _app_settings;	// Application settings


    // Fltk widgets/windows
    Fl_Group *_timewin;		// Window for setting date/time
    Fl_Group *_sunwin;		// Main window
    Fl_Group *_mainw;
    Fl_Calendar *_cal;		// Calendar widget
//              Fl_Check_Button         *_dstbtn;                                                       // Dst button
    Fl_Clock_Output *_tclock;	// Time clock
//              NxImage                         *_sunmap;                                                       // Sun map image
    Fl_Image *_sunmap;		// Sun map image
    Fl_Button *_sunbutton;
    NxButton *_tzbtns[2];	// Home/Visiting timezone buttons
    NxTmInput *_timeui;		// Widget to set the time
    NxMenuButton *_selcity;	// "Popup" menu button

    // Private methods
    void ColosseumLogger(int level, char *str, ...);	// Logging function
    void GetAppPrefs(void);	// Get the application preferences
    void MakeWindows(int, int, int, int);	// Creates the fltk windows
    void MakeSunwin(int w, int h);	// Creates the sunclock window
    void MakeTimewin(int x, int y, int w, int h);	// Creates the time window
    void SetAppPrefs(void);	// Saves the App prefences
    void SetSunclock();		// Creates the sunclock image
    void SetTimes();		// Sets the time

  public:

    // Static Methods 
    static void adj_tm_cb(Fl_Widget * w, void *d);	// Inc/Dec the time unit
    static void map_click_cb(Fl_Widget * w, void *d);	// Callback for a click on the map
    static void save_exit_cb(Fl_Widget * w, void *d);	// Save/Exit callback
    static void sel_city_cb(Fl_Widget * w, void *d);	// Call back for the city selection
    static void set_dt_tm_cb(Fl_Widget * w, void *d);	// Callback to set the date/time
    static void set_tz_cb(Fl_Widget * w, void *d);	// Change tz for active zone
    static void show_sc_win_cb(Fl_Widget * w, void *d);	// Call back to return to sunclock
    static void Sunclock_Timeout(void *d);	// Callback to update (refresh) the map
    static void Timer_Timeout(void *d);	// Callback to update the current time
    static void Toggle_tz_cb(Fl_Widget * w, void *d);	// Callback to make a timezone active
    static void upd_tm_cb(Fl_Widget * w, void *d);	// Update the clock's time

};				// end of nxSunclock definition


#endif //       NXSUNCLOCK_INCLUDED
