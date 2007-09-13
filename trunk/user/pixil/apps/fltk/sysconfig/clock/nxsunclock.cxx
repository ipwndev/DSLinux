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

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <FL/Enumerations.H>
#include <FL/x.H>
#ifdef NOTUSED
#include "hashdb_api.h"
#endif

#include "nxclock.h"

#ifdef NOTUSED
#include "scfxn.h"
#endif

#include <sysconf_plugin.h>

// Typedef, macros, enum/struct/union definitions

#define		SUNCLOCK_TIMEOUT		30.0
#define		DEG_ADJ_VAL			10

#ifdef NOTUSED
// File scope variables
static char tz_label[2][64];	// Static labels for the tz buttons
static bool bQuery = false;	// Query is not to be displayed
#endif

nxSunclock::nxSunclock(int X, int Y, int W, int H, char *appname)
{
    // Query PAR for any known parameters:
    // This will need to query the default image file name, current tz mode,
    // and other application preferences

#ifdef NOTUSED
    _grImage = NULL;
    _zndb = NULL;
    _ePtMode = tzCTYQUERY;
    _eCurTzMode = tzHOME;
#endif

    memset(&_app_settings, 0, sizeof(_app_settings));

#ifdef NOTUSED
    GetAppPrefs();
#endif

#ifdef NOTUSED
    // Initialize data members to known state
    _sun_array = _map_array = NULL;
#endif

    _appmode = eNORMAL;
#ifdef NOTUSED
    _sunwin = NULL;
#endif
    _timewin = NULL;
#ifdef NOTUSED
    _sunmap = NULL;
#endif
    _winX = X;
    _winY = Y;

#ifdef NOTUSED
    _tzbtns[(int) tzHOME] = _tzbtns[(int) tzVISIT] = NULL;
#endif

    _clckedit = 0;

#ifdef NOTUSED
    if (!_zndb) {
	char buf[255];
	getcwd(buf, sizeof(buf));
	sprintf(buf + strlen(buf), "/%s", DEFAULT_ZONEDB);
	_zndb = new char[strlen(buf) + 1];
	strcpy(_zndb, buf);
    }
#endif

    // Setup FLTK
    MakeWindows(X, Y, W, H);

    // Set the initial time
    SetTimes();

#ifdef NOTUSED
    // Set the initial sunclock phase...
    SetSunclock();
#endif

    // Register a timer to update (redraw) the system clock every minute or so...

#ifdef NOTUSED
    Fl::add_timeout(SUNCLOCK_TIMEOUT, Sunclock_Timeout, (void *) this);
#endif

    Fl::add_timeout(0.5, Timer_Timeout, (void *) this);

}

/*******************************************************************************\
**
**	Function:	~nxSunclock()
**	Desc:		Destructor for the nxSunclock class
**	Accepts:	N/A
**	Returns:	N/A
**
\*******************************************************************************/
nxSunclock::~nxSunclock()
{
#ifdef NOTUSED
    delete[]_grImage;
    delete[]_zndb;
    delete[]_sun_array;
    delete[]_map_array;
    delete[]_sunmap->array;
    delete _sunmap;
    delete _tzbtns[(int) tzHOME];
    delete _tzbtns[(int) tzVISIT];
    delete _timeui;
    delete _selcity;
#endif
    delete _timewin;
#ifdef NOTUSED
    delete _sunwin;
#endif
}

// end of nxSunclock::~nxSunclock()

void
nxSunclock::ShowWindow(void)
{
    _timewin->show();
}

void
nxSunclock::HideWindow(void)
{
    _timewin->hide();
}

void
nxSunclock::GetAppPrefs(void)
{
#ifdef NOTUSED
    char par_data[512] = { '\0' },	// Node to get
#endif
    char *pardb;		// Name of the default database
    db_handle *parH;		// Handle to the par database

    // Generic Database handling junk....
    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No default database present!");
	return;
    }				// end of if

    if ((parH = db_openDB(pardb, PAR_DB_MODE_RDONLY)) == NULL) {
	printf("Error opening %s, error=%d", pardb, pardb_errno);
	return;
    }
    // Low-level application info   

#ifdef NOTUSED
    // Query PAR to get the location of the default graphic image
    par_getAppPref(parH, UTILITY_PAR_NAME, "graphic", "map", par_data,
		   sizeof(par_data));
    if (par_data[0]) {
	_grImage = new char[strlen(par_data) + 1];
	strcpy(_grImage, par_data);
    }				// end of if 
#endif

#ifdef NOTUSED
    // Query PAR to get the location of the default city/zone database file
    memset(par_data, 0, sizeof(par_data));
    par_getAppPref(parH, UTILITY_PAR_NAME, "zoneinfo", "db", par_data,
		   sizeof(par_data));
    if (par_data[0]) {
	_zndb = new char[strlen(par_data) + 1];
	strcpy(_zndb, par_data);
    }				// end of if 
#endif

    // Global Timezone application info

#ifdef NOTUSED
    // Home timezone location  (city,reg,tz)
    if (par_getGlobalPref(parH, "timezone", "home_tz", PAR_TEXT,
			  par_data, sizeof(par_data)) == -1) {
	// No such record in the PAR
	printf("Home Timezone not in par!");
	printf("Using default.");

	// Load up default values.
	strcpy(_app_settings.cities[(int) tzHOME].name, "London");
	strcpy(_app_settings.cities[(int) tzHOME].reg, "EN");
	strcpy(_app_settings.cities[(int) tzHOME].tz, "Europe/London");
    }				// end of if
    else {
	parsecity(par_data, &_app_settings.cities[(int) tzHOME]);
	memset(par_data, 0, sizeof(par_data));
    }				// end of else
#endif

#ifdef NOTUSED
    // Visiting timezone location
    if (par_getGlobalPref(parH, "timezone", "visit_tz", PAR_TEXT,
			  par_data, sizeof(par_data)) == -1) {
	// No visit record in the PAR
	printf("No visiting tz found!");
    }				// end of if
    else {
	parsecity(par_data, &_app_settings.cities[(int) tzVISIT]);
	memset(par_data, 0, sizeof(par_data));
    }				// end of else

    // Is home location active location?
    if ((par_getGlobalPref(parH, "timezone", "home_active", PAR_BOOL,
			   (char *) &_app_settings.home_current,
			   sizeof(_app_settings.home_current))) == -1) {
	// Home is not set
	printf("Forced home tz active");
#endif
	_app_settings.home_current = true;
#ifdef NOTUSED
    }				// end of if
#endif

#ifdef NOTUSED
    // Set the global state based on this value
    if (_app_settings.home_current == true)
	_eCurTzMode = tzHOME;
    else
	_eCurTzMode = tzVISIT;
#endif

#ifdef PROMPT_FOR_DST
    // Get the home dst usage
    if ((par_getGlobalPref(parH, "timezone", "h_use_dst", PAR_BOOL,
			   (char *) &_app_settings.use_dst[(int) tzHOME],
			   sizeof(_app_settings.use_dst[(int) tzHOME]))) ==
	-1) {
	printf("Forcing home dst usage!");
	_app_settings.use_dst[(int) tzHOME] = true;
    }				// end of if

    // Get the visit dst usage value
    if ((par_getGlobalPref(parH, "timezone", "v_use_dst", PAR_BOOL,
			   (char *) &_app_settings.use_dst[(int) tzVISIT],
			   sizeof(_app_settings.use_dst[(int) tzVISIT]))) ==
	-1) {
	printf("Forcing visit dst usage!");
	_app_settings.use_dst[(int) tzVISIT] = true;
    }				// end of if
#endif // PROMPT_FOR_DST

    db_closeDB(parH);
    return;
}

void
nxSunclock::MakeWindows(int X, int Y, int W, int H)
{
#ifdef NOTUSED
    _mainw = new Fl_Group(X, Y, W, H);
    MakeSunwin(W, H);
#endif

    MakeTimewin(X, Y, W, H);

#ifdef NOTUSED
    _mainw->end();
    _timewin->hide();
#endif
}

void *
nxSunclock::GetWindow(void)
{
    return (void *) _timewin;
}

#ifdef NOTUSED

/*******************************************************************************\
**
**	Function:	void MakeSunwin()
**	Desc:		Makes the sunclock window and adds all of the necessary widgets
**				to it.
**	Accpets:	int w = width of the window 
**				int h = height of the window
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::MakeSunwin(int w, int h)
{
    int b_x = _winX,		// Bottom row x value
      b_y,			// Botton row y value
      t_x = _winX,		// Top row x value
      t_y;			// Top row y value

    _sunwin = new Fl_Group(_winX, _winY, w, h, UTILITY_NAME);

    //_sunwin->color(NxApp::Instance()->getGlobalColor(APP_BG));

    // Add the widgets

    t_y = (h - ((6 * BUTTON_HEIGHT) + gimp_image.height)) / 2;
    {
	// TL Button
	NxButton *o =
	    new NxButton(t_x, t_y, BUTTON_WIDTH, (3 * BUTTON_HEIGHT),
			 "Set Timezone");
	o->when(FL_WHEN_RELEASE_ALWAYS);
	o->callback(set_tz_cb, (void *) this);
	o->box(FL_BORDER_BOX);
	t_x += BUTTON_WIDTH;
    }				// end of Home Time Zone button widget
    {
	// T text
	NxButton *o = _tzbtns[(int) tzHOME] =
	    new NxButton(t_x, t_y, (w - (2 * BUTTON_WIDTH)),
			 (3 * BUTTON_HEIGHT));
	NxBox *p =
	    new NxBox(t_x, t_y - BUTTON_HEIGHT, (w - (2 * BUTTON_WIDTH)),
		      BUTTON_HEIGHT);

	o->when(FL_WHEN_RELEASE_ALWAYS);
	o->callback(Toggle_tz_cb, (void *) this);
	p->when(FL_WHEN_NEVER);
	o->box(FL_BORDER_BOX);
	p->box(FL_FLAT_BOX);

	//if (_eCurTzMode != tzHOME)
	// o->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

	//p->color(NxApp::getGlobalColor(APP_BG));
	//p->labelcolor(NxApp::getGlobalColor(APP_FG));

	p->label("Home Location");
	t_x += (w - (2 * BUTTON_WIDTH));
    }				// end of Home Time Zone information output widget
    {
	// TR Button
	NxButton *o =
	    new NxButton(t_x, t_y, BUTTON_WIDTH, (3 * BUTTON_HEIGHT));
	o->when(FL_WHEN_RELEASE_ALWAYS);
	o->box(FL_BORDER_BOX);
	o->label("Set Date/Time");
	o->callback(set_dt_tm_cb, (void *) this);
    }				// end of Set Date/Time button widget
    t_y += (3 * BUTTON_HEIGHT);
    {
	// Main button (NxImage())
	int dynsz;

	Fl_Button *o = _sunbutton = new Fl_Button(0, t_y,
						  gimp_image.width,
						  gimp_image.height);
	o->box(FL_FLAT_BOX);
	o->down_box(o->box());
	o->align(FL_ALIGN_INSIDE);
	o->when(FL_WHEN_RELEASE);
	o->callback(map_click_cb, (void *) this);

	dynsz =
	    gimp_image.height * gimp_image.width * gimp_image.bytes_per_pixel;
	_sun_array = new unsigned char[dynsz];

	memcpy(_sun_array, gimp_image.pixel_data, dynsz);
	_sunmap = new Fl_Image(_sun_array, gimp_image.width, gimp_image.height, 4);	//gimp_image.bytes_per_pixel);
//              _sunmap->setvdata(1);
	_sunmap->label(o);
    }
    {
	// Citylist popup menu
	NxMenuButton *o = _selcity = new NxMenuButton(0, t_y, 0, 0);

	o->when(FL_WHEN_RELEASE);
	o->callback(sel_city_cb, (void *) this);
	o->textsize(DEFAULT_TEXT_SIZE);
	o->textfont(DEFAULT_TEXT_FONT);
	o->type(Fl_Menu_Button::POPUP3);
//              o->hide();
    }				// end of the city select browser
    b_y = t_y + gimp_image.height;
    {
	// BL Button
	NxButton *o =
	    new NxButton(b_x, b_y, BUTTON_WIDTH, (3 * BUTTON_HEIGHT),
			 "Save");
	o->when(FL_WHEN_RELEASE_ALWAYS);
	o->callback(save_exit_cb, (void *) this);
	o->box(FL_BORDER_BOX);
	b_x += BUTTON_WIDTH;
    }				// end of Set Visiting TimeZone button widget
    {
	// B Text

	NxButton *o = _tzbtns[(int) tzVISIT] =
	    new NxButton(b_x, b_y, (w - (2 * BUTTON_WIDTH)),
			 (3 * BUTTON_HEIGHT));
	NxBox *p = new NxBox(b_x, b_y + o->h(),
			     (w - (2 * BUTTON_WIDTH)), BUTTON_HEIGHT);

	o->box(FL_BORDER_BOX);
	p->box(FL_FLAT_BOX);
	o->when(FL_WHEN_RELEASE_ALWAYS);
	p->when(FL_WHEN_NEVER);
	o->callback(Toggle_tz_cb);
	//if (_eCurTzMode != tzVISIT)
	//o->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

	//p->color(NxApp::getGlobalColor(APP_BG));
	//p->labelcolor(NxApp::getGlobalColor(APP_FG));

	p->label("Visiting Location");
	b_x += (w - (2 * BUTTON_WIDTH));
    }				// end of Visiting Time Zone information output widget
    {
	// BR Button
	NxButton *o =
	    new NxButton(b_x, b_y, BUTTON_WIDTH, (3 * BUTTON_HEIGHT),
			 "Reset");

	o->when(FL_WHEN_RELEASE_ALWAYS);
	o->callback(save_exit_cb);
	o->box(FL_BORDER_BOX);
    }				// end of Set timeout
    _sunwin->end();

    return;
}				// end of nxSunclock::MakeSunwin(int, int)

#endif

/*******************************************************************************\
**
**	Function:	void SetAppPrefs()
**	Desc:		Stores the changed (i.e. dirty) prefences into the PAR
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::SetAppPrefs(void)
{
#ifdef NOTUSED
    char par_data[512];		// string data
#endif

    char *pardb;		// Ptr to the default db name
    db_handle *parH;		// Handle to the db

    // Get the default name
    if ((pardb = db_getDefaultDB()) == NULL) {
	// Error accessing db
	printf("Unable to find PAR!");
	printf("Ignoring %s changes!", UTILITY_NAME);
	return;
    }				// end of if

    // Open the database
    if ((parH = db_openDB(pardb, PAR_DB_MODE_RW)) == NULL) {
	// Error opening db
	printf("Unable to open %s.", pardb);
	printf("Ignoring %s changes!", UTILITY_NAME);
	return;
    }				// end of if

#ifdef NOTUSED

    // Begin updating database
    if (_app_settings.dirty_flgs & APP_DFLGS_HOME) {
	sprintf(par_data, "%s,%s,%s", _app_settings.cities[(int) tzHOME].name,
		_app_settings.cities[(int) tzHOME].reg,
		_app_settings.cities[(int) tzHOME].tz);
	if (par_setGlobalPref(parH, "timezone", "home_tz", PAR_TEXT,
			      par_data, strlen(par_data)) == -1) {
	    // Unable to update the *changed* home tz
	    printf("Unable to update timezone.home_tz");
	    printf("HTZ=%s will be lost!", par_data);
	}			// end of if
    }				// end of if

    if (_app_settings.dirty_flgs & APP_DFLGS_VISIT) {
	sprintf(par_data, "%s,%s,%s",
		_app_settings.cities[(int) tzVISIT].name,
		_app_settings.cities[(int) tzVISIT].reg,
		_app_settings.cities[(int) tzVISIT].tz);
	if (par_setGlobalPref
	    (parH, "timezone", "visit_tz", PAR_TEXT, par_data,
	     sizeof(par_data)) == -1) {
	    // Unable to update the *changed* visiting tz
	    printf("Unable to update timezone.visit_tz");
	    printf("VTZ=%s will be lost!", par_data);
	}			// end of if
    }				// end of if
    if ((_app_settings.dirty_flgs & APP_DFLGS_CUR) &&
	par_setGlobalPref(parH, "timezone", "home_active", PAR_BOOL,
			  (char *) &_app_settings.home_current,
			  sizeof(_app_settings.home_current)) == -1) {
	// Unable to update the *changed* active flag
	printf("Unable to update timezone.home_active");
	printf("home_active=%s, not saved!",
	       _app_settings.home_current == true ? "true" : "false");
    }				// end of if
#endif

#ifdef PROMPT_FOR_DST
    if (_app_settings.dirty_flgs & APP_DFLGS_DST) {
	if (par_setGlobalPref(parH, "timezone", "h_use_dst", PAR_BOOL,
			      (char *) &_app_settings.use_dst[(int) tzHOME],
			      sizeof(_app_settings.use_dst[(int) tzHOME])) ==
	    -1) {
	    // Unable to update the *changed* h_use_dst flag
	    printf("Unable to update timezone.h_use_dst");
	    printf("h_use_dst=%s, not saved!",
		   _app_settings.use_dst[(int) tzHOME] ==
		   true ? "true" : "false");
	}			// end of if
	if (par_setGlobalPref(parH, "timezone", "v_use_dst", PAR_BOOL,
			      (char *) &_app_settings.use_dst[(int) tzVISIT],
			      sizeof(_app_settings.use_dst[(int) tzVISIT])) ==
	    -1) {
	    // Unable to update the *changed* v_use_dst flag
	    printf("Unable to update timezone.v_use_dst");
	    printf("v_use_dst=%s, not saved!",
		   _app_settings.use_dst[(int) tzVISIT] ==
		   true ? "true" : "false");
	}			// end of if
    }				// end of if
#endif //      PROMPT_FOR_DST

    db_closeDB(parH);

    // Send a colosseum broadcast message to everyone about the timezone change...
    char col_msg[CL_MAX_MSG_LEN] = { '\0' };
    int col_len = sprintf(col_msg, "sc_clock^SYSCON_TZ_CHANGE");
    sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);

    return;
}				// end of nxSunclock::SetAppPrefs(void)

/*******************************************************************************\
**
**	Function:	void MakeTimewin()
**	Desc:		Makes the time-setting window and adds the appropriate widgets
**	Accepts:	int w = width of the window
**				int h = height of the window
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::MakeTimewin(int X, int Y, int w, int h)
{
    int c_x,			// Clocks x value
      c_y,			// Clocks y value
      d_x,			// Dates x value
      d_y;			// Dates y value

    NxApp *instance = sysconf_get_instance();

    // Make the encompassing window
    _timewin = new Fl_Group(_winX, _winY, w, h);
    _timewin->color(instance->getGlobalColor(APP_BG));

    // Add all of the widgets

    d_y = Y + BUTTON_Y - _winY;
    d_x = X + BUTTON_X;

    {
	// Add the "OK" button
	NxButton *o;		// Button widget

	d_x = X + BUTTON_X;
	d_y = Y + BUTTON_Y - _winY;
	o = new NxButton(d_x, d_y, BUTTON_WIDTH, BUTTON_HEIGHT, "Save");

	o->when(FL_WHEN_RELEASE);
	o->callback(show_sc_win_cb, (void *) this);
	d_x += 63;
    }				// end of OK Button
    {
	// Add the "Cancel Button"
	NxButton *o;

	o = new NxButton(d_x, d_y, BUTTON_WIDTH, BUTTON_HEIGHT, "Reset");

	o->when(FL_WHEN_RELEASE);
	o->callback(show_sc_win_cb);

	c_y += BUTTON_HEIGHT + 2;
    }				// end of cancel button
    c_x = d_x = X + 2;
    d_y = Y + h / 3 + 10;
    c_y = Y;
    {
	NxButton *o;		// Label button

	o = new NxButton(c_x, c_y, _timewin->w(), BUTTON_HEIGHT);
	o->box(FL_BORDER_BOX);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	o->label("Current Time");
	c_y = Y + o->h() + 2;
    }				// end of if
    {
	// Fl Clock widget....
	Fl_Clock_Output *o;	// Clock widget

	_tclock = o = new Fl_Clock_Output(c_x, c_y, w / 3,
					  (h - (3 * BUTTON_HEIGHT)) / 3);
	o->box(FL_FLAT_BOX);
	o->color(instance->getGlobalColor(APP_BG));
	o->type(FL_SQUARE_CLOCK);
	c_x += (w / 3) + 5;
	c_y = Y + o->h() / 3;
	d_y = c_y + o->h();
    }				// end of Fl_Clock widget
    {
	NxTmInput *o;		// Time input widget

	_timeui = o =
	    new NxTmInput(c_x, c_y, (int) (2 * BUTTON_WIDTH), BUTTON_HEIGHT,
			  0, 3);
	o->color(instance->getGlobalColor(APP_BG));

	for (int i = 0; i < o->GetUnits(); i++) {
	    o->Units[i]->box(FL_FLAT_BOX);
	    o->Units[i]->when(FL_WHEN_CHANGED);
	    o->Units[i]->callback(upd_tm_cb, (void *) this);
	    o->Units[i]->color(instance->getGlobalColor(APP_BG));
	    o->Units[i]->selection_color(instance->getGlobalColor(APP_SEL));

	    instance->def_font(o);
	}

	c_x += o->w();
    }				// end of Time input widget
    {
	// Time Inc/Dec buttons
	NxButton *dn,		// Down button
	 *up;			// Up button
	dn = new NxButton(c_x, c_y + (BUTTON_HEIGHT / 2), BUTTON_WIDTH / 3,
			  ((BUTTON_HEIGHT / 3) * 2), "-");
	up = new NxButton(c_x, c_y - (BUTTON_HEIGHT / 2), BUTTON_WIDTH / 3,
			  ((BUTTON_HEIGHT / 3) * 2), "+");
//              dn = new NxButton(c_x, c_y + (BUTTON_HEIGHT / 2), BUTTON_WIDTH / 4,
//                                                      BUTTON_HEIGHT / 2, "-");
//              up = new NxButton(c_x, c_y, BUTTON_WIDTH / 4, BUTTON_HEIGHT / 2, "+");

//              dn->color(DEF_BG);
//              up->color(DEF_BG);
//              dn->box(FL_BORDER_BOX);
//              up->box(FL_BORDER_BOX);
	// Set the call backs
	dn->when(FL_WHEN_RELEASE);
	up->when(FL_WHEN_RELEASE);
	dn->callback(adj_tm_cb, (void *) this);
	up->callback(adj_tm_cb, (void *) this);

	c_y += BUTTON_HEIGHT;
    }				// end of Time inc/dec buttons
#ifdef PROMPT_FOR_DST
    {
	Fl_Check_Button *o;	// Check Button

	c_x = w / 3 + 5;
	_dstbtn = o =
	    new Fl_Check_Button(c_x, c_y, BUTTON_WIDTH, 2 * BUTTON_HEIGHT,
				"Use Daylight Savings");
	o->value(0);
	if ((_eCurTzMode == tzHOME
	     && _app_settings.use_dst[(int) tzHOME] == true)
	    || (_eCurTzMode == tzVISIT
		&& _app_settings.use_dst[(int) tzVISIT] == true))
	    o->value(1);
    }				// end of DST Check box
#endif // PROMPT_FOR_DST
    {
	NxButton *o;		// Calendar section label

	o = new NxButton(0, d_y, _timewin->w(), BUTTON_HEIGHT,
			 "Current Date");

	o->box(FL_BORDER_BOX);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	d_y += BUTTON_HEIGHT + 5;
    }				// end of Calendar label
    {
	// Fl_Calendar widget
	Fl_Calendar *o;		// Calendar widget

	d_x = X + 5;

	_cal = o = new Fl_Calendar(d_x, d_y, (w - (2 * d_x)), (h - (d_y)));

	//                                 (h - (d_y + (2 * BUTTON_HEIGHT))));
    }				// end of Calendar    

    _timewin->end();
    _timewin->hide();

    return;
}				// end of nxSunclock::MakeTimewin(int, int)

#ifdef NOTUSED

/*******************************************************************************\
**
**	Function:	void SetSunclock()
**	Desc:		Calculates the position of the sun, and darkens the current pixel
**				values where the sun isnt -- thus creating the sun clock....
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::SetSunclock(void)
{
    static int cnt = 0;
#ifdef NANOX
    int isize =
	gimp_image.height * gimp_image.width * gimp_image.bytes_per_pixel;
    printf("We are allocating %d bytes for the image\n", isize);

    // Put the original data back into the array, before twiddling with it
    memcpy(_sun_array, gimp_image.pixel_data, isize);

    nxsunimage((unsigned char *) gimp_image.pixel_data, _sun_array,
	       gimp_image.height, gimp_image.width,
	       gimp_image.bytes_per_pixel);

    cnt++;
#endif

    _sunbutton->damage(FL_DAMAGE_ALL);
    _sunbutton->redraw();

    return;
}				// end of nxSunclock::SetSunclock(void)

#endif

/*******************************************************************************\
**
**	Function:	void SetTimes()
**	Desc:		Calculates the current time for both timezone (home/visit) and
**				properly formats the time on the button label
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::SetTimes(void)
{
#ifdef NOTUSED
    char timefmt[64];		// Time format
    time_t nowtime = time(NULL);	// Current gmt (utc) time
    struct tm act_time,		// Active time struct
      inact_time;		// Inactive time struct
#endif

#ifdef NOTUSED
    tz_mode_enum acttz,		// Active time zone index
      inacttz;			// Inactive time zone index
#endif

#ifdef NOTUSED
    // Determine active/inactive widgets
    if (_eCurTzMode == tzHOME) {
	acttz = tzHOME;
	inacttz = tzVISIT;
    }				// end of if
    else {
	acttz = tzVISIT;
	inacttz = tzHOME;
    }				// end of else
#endif

    // Set the user-editable clock
    if (_clckedit == 0) {
	time_t timeval;
	struct tm *tmptm;

	time(&timeval);
	tmptm = localtime(&timeval);
#ifdef PROMPT_FOR_DST
	if (!_dstbtn->value() && tmptm->tm_isdst) {
	    timeval -= (60 * 60);
	    tmptm = localtime(&timeval);
	}			// end of if
#endif // PROMPT_FOR_DST
	_tclock->value(tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec);
	_timeui->SetTime(tmptm);
    }				// end of if

#ifdef NOTUSED
    // switch timezones to the inactive to get it's time...
    if (bQuery == true) {
	if (query_rec.name[0] > ' ') {
	    setenv("TZ", query_rec.tz, 1);
	    tzset();
	    memcpy(&inact_time, localtime(&nowtime), sizeof(inact_time));
	    sprintf(timefmt, "%s, %s\n%%a, %%b %%d, %%Y\n%%H:%%M:%%S",
		    query_rec.name, query_rec.reg);
	    strftime(tz_label[(int) inacttz], sizeof(tz_label[(int) inacttz]),
		     timefmt, &inact_time);
	}			// end of if
	else {
	    sprintf(tz_label[(int) inacttz], "No selection");
	}			// end of else
    }				// end of if
    else {
	if (_app_settings.cities[(int) inacttz].name[0] > ' ') {

	    setenv("TZ", _app_settings.cities[(int) inacttz].tz, 1);
	    tzset();
	    memcpy(&inact_time, localtime(&nowtime), sizeof(inact_time));

	    sprintf(timefmt, "%s, %s\n%%a, %%b %%d, %%Y\n%%H:%%M:%%S",
		    _app_settings.cities[(int) inacttz].name,
		    _app_settings.cities[(int) inacttz].reg);
	    strftime(tz_label[(int) inacttz], sizeof(tz_label[(int) inacttz]),
		     timefmt, &inact_time);
	}			// end of if
	else {
	    sprintf(tz_label[(int) inacttz], "Not Set");
	}			// end of else
    }				// end of else
    _tzbtns[(int) inacttz]->label(tz_label[(int) inacttz]);
    _tzbtns[(int) inacttz]->redraw();

    // Switch timezones back to the current value
    memcpy(&act_time, localtime(&nowtime), sizeof(act_time));

    if (_app_settings.cities[(int) acttz].name[0] > ' ') {
	setenv("TZ", _app_settings.cities[(int) acttz].tz, 1);
	tzset();
	memcpy(&act_time, localtime(&nowtime), sizeof(act_time));

	sprintf(timefmt, "%s, %s\n%%a, %%b %%d, %%Y\n%%H:%%M:%%S",
		_app_settings.cities[(int) acttz].name,
		_app_settings.cities[(int) acttz].reg);
	strftime(tz_label[(int) acttz], sizeof(tz_label[(int) acttz]),
		 timefmt, &act_time);
    }				// end of if
    else {
	sprintf(tz_label[(int) acttz], "Not set");
    }				// end of else
    _tzbtns[(int) acttz]->label(tz_label[(int) acttz]);
    _tzbtns[(int) acttz]->redraw();
#endif

    return;
}				// end of nxSunclock::SetTimes(void)

void
nxSunclock::save_exit_cb(Fl_Widget * w, void *d)
{
    nxSunclock *me = (nxSunclock *) d;
    NxButton *nxb = (NxButton *) w;

    if (!strcmp(nxb->label(), "Save")) {
	DPRINTF("Saving any changed data!\n");
#ifdef NOTUSED
	me->SetAppPrefs();
#endif
    }				// end of if

    // Update as needed (incase of a reset)
#ifdef NOTUSED
    me->GetAppPrefs();
#endif
    me->SetTimes();

    return;
}				// end of NxPowerman::save_exit_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void adj_tm_cb()
**	Desc:		Increments/Decrements the time unit value and sets the clock 
**				accordingly
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for branch
**				void *d = Ptr to any andcillary data
**	Returns:	Nothing (void);
**
\*******************************************************************************/
void
nxSunclock::adj_tm_cb(Fl_Widget * w, void *d)
{
    int adj_val,		// Value to adjust by
      indx = -1;		// Index into Unit array
    nxSunclock *me = (nxSunclock *) d;
    NxButton *nxb = (NxButton *) w;
    NxTmUnit *tu = NULL;	// Time unit
    NxTmUnit *f = (NxTmUnit *) Fl::focus();

    // Set the adjustment value
    if (!(memcmp(nxb->label(), "+", 1)))
	adj_val = 1;
    else
	adj_val = -1;

    // Get the selection widget and adjust it
    for (int idx = 0; idx < me->_timeui->GetUnits(); idx++) {
	if (me->_timeui->Units[idx] == f) {
	    tu = (NxTmUnit *) f;
	    indx = idx;
	    break;
	}			// end of if
    }				// end of for

    // Adjust the value
    if (tu) {
	int val;		// Clock value
	char ubuf[3];		// unit buffer

	strcpy(ubuf, tu->value());
	val = atoi(ubuf);
	val += adj_val;
	if (val > max_val[indx])
	    val = 0;
	if (val < 0)
	    val = max_val[indx];
	sprintf(ubuf, "%2.2d", val);
	tu->value(ubuf);
	tu->do_callback();
    }				// end of if

    return;
}				// end of void nxSunclock::adj_time_cb(Fl_Widget *, void *)

#ifdef NOTUSED

/*******************************************************************************\
**
**	Function:	void map_click_cb()
**	Desc:		Callback function for a click on the map image button.  Functionality
**				is based on the current mode (i.e. City time query or timezone
**				selection), and current home/visiting selection.
**	Accepts:	Fl_Widget *w = widget responsible for branching to this callback
**				void *d = Any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::map_click_cb(Fl_Widget * W, void *d)
{
    float lat,			// Latitude
      lon;			// Longitude
    int ccnt,			// City count
      h,			// Height
      mw,			// Max width 
      w,			// Width
      x = Fl::event_x(), y = Fl::event_y(), xpos,	// mouse X position
      ypos;			// mouse Y position
    NxButton *nxb = (NxButton *) W;	// Button widget
    nxSunclock *me = (nxSunclock *) d;

    // Adjust the X and Y mouse clicks to be relative to the widget...
    xpos = x - nxb->x();
    ypos = y - nxb->y();
    w = nxb->w();
    h = nxb->h();

    // Correction for the map?
    xpos += 6;

    // Convert the x/y coordinate's into relative latitude and longitude values
    lat = (((-180.0 * ypos) + (180.0 * h)) / h) - 90.0;
    lon = (((360.0 * (float) xpos) / (float) w) - 180.0);
    if (lon > 180.0)
	lon -= 360.0;

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    mw = 0;

    me->_selcity->clear();
    if ((ccnt = GetCityList(me->_zndb, (int) lat, (int) lon)) > 0) {
	// Load up a Fl_SelectBrowser widget
	for (int i = 0; i < ccnt; i++) {
	    float tmp;
	    char buf[25];

	    sprintf(buf, "%s, %s", city_list[i].city_name, city_list[i].reg);
	    if ((tmp = fl_width(buf) + 5) > mw)
		mw = (int) tmp;
	    me->_selcity->add(buf);
	}			// end of for
	if ((x + mw) > nxb->w()) {
	    x = nxb->w() - (mw + 2);
	}			// end of if 
	me->_selcity->popup(x, y);
    }				// end of if
    else {
#ifdef NOTUSED
	me->SetAppPrefs();
#endif
    }				// end of if

    // Reset the data
#ifdef NOTUSED
    me->GetAppPrefs();
#endif
    me->SetTimes();

    return;
}				// end of nxSunclock::save_exit_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void sel_city_cb()
**	Desc:		The user has selected an item from the list, determine which
**				one, and how to process it.
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::sel_city_cb(Fl_Widget * w, void *d)
{
    nxSunclock *me = (nxSunclock *) d;
    NxMenuButton *nxmb = (NxMenuButton *) w;
    int idx;

    idx = nxmb->value();
    if (idx <= -1)
	return;

    if (me->_ePtMode == tzCTYSET) {
	// Set the value for the selected timezone
	memcpy(me->_app_settings.cities[(int) me->_eCurTzMode].name,
	       city_list[idx].city_name,
	       sizeof(me->_app_settings.cities[(int) me->_eCurTzMode].name) -
	       1);
	memcpy(me->_app_settings.cities[(int) me->_eCurTzMode].reg,
	       city_list[idx].reg,
	       sizeof(me->_app_settings.cities[(int) me->_eCurTzMode].reg) -
	       1);
	ExpandTimeZone(me->_app_settings.cities[(int) me->_eCurTzMode].tz,
		       city_list[idx].zinfo,
		       sizeof(me->_app_settings.cities[(int) me->_eCurTzMode].
			      tz) - 1);
	if (me->_eCurTzMode == tzHOME)
	    me->_app_settings.dirty_flgs |= APP_DFLGS_HOME;
	else
	    me->_app_settings.dirty_flgs |= APP_DFLGS_VISIT;
    } /* end of if */
    else {
	// Set the value for a city query
	memset(&query_rec, 0, sizeof(query_rec));
	memcpy(query_rec.name, city_list[idx].city_name,
	       sizeof(query_rec.name) - 1);
	memcpy(query_rec.reg, city_list[idx].reg, sizeof(query_rec.reg) - 1);
	ExpandTimeZone(query_rec.tz, city_list[idx].zinfo,
		       sizeof(query_rec.tz) - 1);
	bQuery = true;
    }				// end of else

    return;
}				// end of nxSunclock::sel_city_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void set_dt_tm_cb()
**	Desc:		Causes the window that allows the user to set the current date 
**				and time to be shown
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::set_dt_tm_cb(Fl_Widget * w, void *d)
{
    nxSunclock *me = (nxSunclock *) d;

    me->_sunwin->hide();
    me->_timewin->show();
    Fl::focus(me->_timeui->Units[0]);
    return;
}				// end of nxSunclock::set_dt_tm_cb(Fl_Widget *, void *)
#endif

#ifdef NOTUSED
/*******************************************************************************\
**
**	Function:	void set_tz_cb()
**	Desc:		Allows the user to select/set the timezone for the active mode
**				(i.e. home/visit) on the next map click.
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::set_tz_cb(Fl_Widget * w, void *d)
{
    static char *button_label[2] = { {"Set Timezone"},
    {"View Time"}
    };
    NxButton *nxb = (NxButton *) w;
    nxSunclock *me = (nxSunclock *) d;

    if (me->_ePtMode == tzCTYSET) {
	me->_ePtMode = tzCTYQUERY;
    }				// end of if
    else {
	me->_ePtMode = tzCTYSET;
	bQuery = false;
    }				// end of else

    nxb->label(button_label[me->_ePtMode]);

    return;
}				// end of nxSunclock::set_tz_cb(Fl_Widget *, void *)
#endif

/*******************************************************************************\
**
**	Function:	void show_sc_win_cb()
**	Desc:		Hides the time window and shows the sunclock window
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::show_sc_win_cb(Fl_Widget * w, void *d)
{
    time_t nowt,		// Now ctime value
      newt;			// New ctime value
    struct tm curtm;		// Current date/time value
    struct timeval tv;		// Timeval structure
    nxSunclock *me = (nxSunclock *) d;
    NxButton *nxb = (NxButton *) w;

#ifdef NOTUSED
    // Determine if anything needs to be set permanently
#ifdef PROMPT_FOR_DST
    bool *dstval;		// Dst value
    if (me->_eCurTzMode == tzHOME)
	dstval = &me->_app_settings.use_dst[(int) tzHOME];
    else
	dstval = &me->_app_settings.use_dst[(int) tzVISIT];
#endif // PROMPT_FOR_DST
#endif

    if (!(memcmp(nxb->label(), "Save", 4))) {
#ifdef PROMPT_FOR_DST
	// This is the ok button -- need to update everything:
	//      1)      Check the value of the dst flag
	if (me->_dstbtn->value() && *dstval == false) {
	    *dstval = true;
	    me->_app_settings.dirty_flgs |= APP_DFLGS_DST;
	}			// end of if
	else if (!(me->_dstbtn->value()) && *dstval == true) {
	    *dstval = false;
	    me->_app_settings.dirty_flgs |= APP_DFLGS_DST;
	}			// end of else-if
#endif // PROMPT_FOR_DST
	//      2)      Get the date and time values from the clock/calendar widgets

	time(&nowt);
	me->_timeui->GetTime(&curtm);
	curtm.tm_mon = me->_cal->month() - 1;
	curtm.tm_mday = me->_cal->day();
	curtm.tm_year = me->_cal->year() - 1900;
	newt = mktime(&curtm);

#ifdef PROMPT_FOR_DST
	// Check if we need to adjust the time for dst purposes
	if (curtm.tm_isdst && *dstval == false) {
	    newt -= (60 * 60);
	}			// end of if
#endif

	tv.tv_sec = newt;
	//      3)      Set the system date/time
	if (settimeofday(&tv, 0) == -1) {
	}			// end of if

	// Inform all apps via colosseum broadcast message that the system time changed
	char col_msg[CL_MAX_MSG_LEN] = { '\0' };
	int col_len = sprintf(col_msg, "sc_clock^SYSCON_TM_CHANGE");
	sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);
    }				// end of if

    // reset the edit flag
    me->_clckedit = 0;

#ifdef NOTUSED
    // Finally, hide this window and show the other.
    me->_timewin->hide();
    me->_sunwin->show();
#endif

    return;
}				// end of nxSunclock::show_sc_win_cb(Fl_Widget *, void *)

#ifdef NOTUSED
/*******************************************************************************\
**
**	Function:	void Sunclock_Timeout()
**	Desc:		Response to a timer "timeout" to recalculat the position of the
**				sun and redraw the image on the screen.
**	Accepts:	void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::Sunclock_Timeout(void *d)
{
    nxSunclock *me = (nxSunclock *) d;

    // Draw the current sun/shadow state on the map only if the browser isn't being shown
//      if (!me->_selcity->Fl_Widget::visible())
//      {
    me->SetSunclock();
//      } // end of if 

    Fl::add_timeout(SUNCLOCK_TIMEOUT, Sunclock_Timeout, (void *) me);
    return;
}				// end of nxSunclock::Sunclock_Timeout(void *)

#endif

/*******************************************************************************\
**
**	Function:	void Timer_Timeout()
**	Desc:		Response to a timer "timeout" to allow the current time to be
**				updated on the screen properly
**	Accepts:	void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::Timer_Timeout(void *d)
{
    // Needs to set the current time for both timezones
    nxSunclock *me = (nxSunclock *) d;

    me->SetTimes();

    Fl::add_timeout(0.5, Timer_Timeout, (void *) me);

    return;
}				// end of nxSunclock::Timer_Timeout(void *)

#ifdef NOTUSED

/*******************************************************************************\
**
**	Function:	void Toggle_tz_cb()
**	Desc:		Toggles the current timezone between the Home/Visiting timezones
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::Toggle_tz_cb(Fl_Widget * w, void *d)
{
    NxButton *nxb = (NxButton *) w;	// So I don't have to cast...
    nxSunclock *me;		// The object...

    me = (nxSunclock *) d;
    if (nxb == me->_tzbtns[(int) tzHOME] && me->_eCurTzMode == tzVISIT) {
	// Force me to be active...
	//me->_tzbtns[(int) tzHOME]->labelcolor(NxApp::Instance()->
	//                                      getGlobalColor(BUTTON_TEXT));
	//me->_tzbtns[(int) tzVISIT]->labelcolor(NxApp::Instance()->
	//                                 getGlobalColor(APP_FG));
	me->_eCurTzMode = tzHOME;
	me->_app_settings.home_current = true;
	me->_app_settings.dirty_flgs |= APP_DFLGS_CUR;
    }				// end of if 
    else if (nxb == me->_tzbtns[(int) tzVISIT] && me->_eCurTzMode == tzHOME) {
	//me->_tzbtns[(int) tzVISIT]->labelcolor(NxApp::Instance()->
	//getGlobalColor(BUTTON_TEXT));
	//me->_tzbtns[(int) tzHOME]->labelcolor(NxApp::Instance()->
	//getGlobalColor(APP_FG));
	me->_eCurTzMode = tzVISIT;
	me->_app_settings.home_current = false;
	me->_app_settings.dirty_flgs |= APP_DFLGS_CUR;
    }				// end of else

    // Modify the timezone info ONLY if there is a zoneinfo
    // timezone available...
    if (me->_app_settings.cities[(int) me->_eCurTzMode].tz[0] > ' ') {
	// And, of course, we have access to it....
	if (!access("/etc", W_OK | X_OK)) {
	    char sysbuf[128];	// Buffer for a system() call
	    int rc;		// Result code

	    unlink("/etc/localtime");
	    sprintf(sysbuf, "ln -sf /usr/share/zoneinfo/%s /etc/localtime",
		    me->_app_settings.cities[(int) me->_eCurTzMode].tz);
	    if ((rc = system(sysbuf)) != -1 && rc != 0x7f) {
		// The system call succeeded, send a colosseum message...
	    }			// end of if
	}			// end of if
    }				// end of if

    return;
}				// end of nxSunclock::Toggle_tz_cb(Fl_Widget *, void *)
#endif

/*******************************************************************************\
**
**	Function:	void upd_tm_cb()
**	Desc:		Updates the clock with the current values of _timeui
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
nxSunclock::upd_tm_cb(Fl_Widget * w, void *d)
{
    struct tm tmv;
    nxSunclock *me = (nxSunclock *) d;
//      NxTmUnit                        *tu = (NxTmUnit *)w;

    // Stop update of the clock
    me->_clckedit = 1;

    me->_timeui->GetTime(&tmv);
    me->_tclock->value(tmv.tm_hour, tmv.tm_min, tmv.tm_sec);

    return;
}				// end of 

#ifdef NOTUSED

//-------------------------------------------------------------------------------
//
//      Non-Class functions
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void ExpandTimeZone()
**	Desc:		Expands the timezone from the hashdb values to the zoneinfo
**				values.
**	Accepts:	char *dst = Storage for the new timezone
**				char *src = Encoded timezone value (2 char dir, 5 char file)
**				int dzise = Size of the destination
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
ExpandTimeZone(char *dst, char *src, int dsize)
{
    char dirbuf[255];		// Directory buffer
    int len = 0;		// Length
    DIR *tzdir;			// Directory handle
    struct dirent *dir;		// Directory entry

    // Open the main directory, and find the matching one
    strcpy(dirbuf, "/usr/share/zoneinfo");
    tzdir = opendir(dirbuf);
    while (tzdir && (dir = readdir(tzdir)) != NULL) {
	if (!memcmp(dir->d_name, src, 2)) {
	    strcat(dirbuf, "/");
	    strcat(dirbuf, dir->d_name);
	    len = snprintf(dst + len, dsize - len, "%s", dir->d_name);
	    closedir(tzdir);
	    tzdir = opendir(dirbuf);
	    while (tzdir && (dir = readdir(tzdir)) != NULL) {
		if (!memcmp(dir->d_name, src + 2, 5)) {
		    snprintf(dst + len, dsize - len, "/%s", dir->d_name);
		    break;
		}		// end of if 
	    }			// end of while */
	    break;
	}			// end of if 
    }				// end of while

    closedir(tzdir);

    return;
}				// end of ExpandTimeZone(char *, char *)

/*******************************************************************************\
**
**	Function:	void parsecity
**	Desc:		Parses the city timezone info as stored in par for the home
**				and visiting timezone records.
**	Accepts:	char *data = data to parse
**				city_rec_t *crec = Ptr to a city record structure
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
parsecity(char *data, city_rec_t * crec)
{
    char *cp,			// Generic pointer
     *vect[3];			// Vector
    int cnt,			// Number of fields
      i;			// Loop iterate

    if (!data || !crec)
	return;

    vect[0] = crec->name;
    vect[1] = crec->reg;
    vect[2] = crec->tz;

    for (cnt = 1, cp = data; *cp; cp++) {
	if (*cp == ',') {
	    *cp = '\0';
	    cnt++;
	}			// end of if
    }				// end of for

    for (i = 0, cp = data; i < cnt; i++, cp += strlen(cp) + 1) {
	strcpy(vect[i], cp);
    }				// end of for

    return;
}				// end of void

/*******************************************************************************\
**
**	Function:	GetCityList()
**	Desc:		Gets a list of cities that are within +- 5 degrees lat,long of 
**				the current point.
**	Accepts:	char *zndb = Path to the database
**				int latitude = latitude of current point
**				int longitude = longitude of current point
**	Returns:	int; number of cities found.
**
\*******************************************************************************/
static int
GetCityList(char *zndb, int latitude, int longitude)
{
    int cnt = 0,		// Number of records found
      i,			// Loop iterator
      lon_val,			// Longitude value
      mode;			// Find mode 
    hashdb_t *hdbd;		// Handle to the hashdb
    h_data_rec record;		// Record struct


    // Clean out the static list
    if (city_size)
	memset(city_list, 0, (city_size * sizeof(h_data_rec)));

    // Open the database
    if ((hdbd = h_opendb(zndb, O_RDONLY)) == NULL)
	return (-1);

    for (i = latitude - DEG_ADJ_VAL; i < latitude + DEG_ADJ_VAL; i++) {
	mode = HDB_KEY_HASH;
	while (h_getrecord(mode, hdbd,
			   (mode == HDB_KEY_HASH ? i + 90 : record.next),
			   &record) != -1) {
	    mode = HDB_KEY_DATA;
	    lon_val = record.lon / 60;
	    if (lon_val >= longitude - DEG_ADJ_VAL
		&& lon_val <= longitude + DEG_ADJ_VAL) {
		/* Add city to list */
		if (cnt + 1 >= city_size) {
		    if (city_size == 0) {
			city_list =
			    (h_data_rec *) calloc(5, sizeof(h_data_rec));
			if (city_list == NULL)
			    return (-1);
		    }		// end of if
		    else {
			h_data_rec *tmp;

			tmp =
			    (h_data_rec *) realloc(city_list,
						   (city_size +
						    5) * sizeof(record));
			if (tmp == NULL)
			    return (-1);
			city_list = tmp;
		    }		// end of else
		    city_size += 5;
		}		// end of if 
		memcpy(&city_list[cnt], &record, sizeof(h_data_rec));
		cnt++;
	    }			/* end of if */
	}			/* end of while */
    }				/* end of for */
    h_closedb(hdbd);

    // Possibly sort the list???
    return (cnt);
}				// end of void GetCityList(char *, int, int)

#endif
