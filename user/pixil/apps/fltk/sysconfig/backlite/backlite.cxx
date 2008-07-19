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


// System header files
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>


// Local header files
#include <FL/Enumerations.H>
#include "nxbl.h"
#include <nxbutton.h>
#include <wm/scrtoplib.h>
#include <pixlib/pixlib.h>

#include <sysconf_plugin.h>

static char *def_bat_warn = "Warning: Using backlight with battery\n"
    "will reduce battery life.", *tmout_lbls[2][11] = {
    {"10 Seconds",
     "30 Seconds",
     "1 Minute",
     "2 Minutes",
     "3 Minutes",
     "4 Minutes",
     "5 Minutes",
     NULL},
    {"1 Minute",
     "2 Minutes",
     "3 Minutes",
     "4 Minutes",
     "5 Minutes",
     "6 Minutes",
     "7 Minutes",
     "8 Minutes",
     "9 Minutes",
     "10 Minutes",
     NULL}
}, *mode_lbls[] =
{
"Battery", "External"};
signed short tmout_vals[2][11] = {
    {10, 30, 60, 120, 180, 240, 300, -1},
    {60, 120, 180, 240, 300, 360, 420,
     480, 540, 600, -1}
};

/*******************************************************************************\
**
**	Function:	~NxBacklite()
**	Desc:		Class NxBacklite destructor, responsible for free()ing dynamic
**				memory
**	Accepts:	N/A
**	Returns:	N/A
**
\*******************************************************************************/
NxBacklite::~NxBacklite()
{
    // Reset the backlite to the last stored value for the current mode....

    if (pix_pwr_onBattery())
	_blMode = blBATTERY;
    else
	_blMode = blACPOWER;

    GetAppPrefs();
    SetValues();
    SetBl();

    delete _mbmode;
    delete _mbtimeunit[0];
    delete _mbtimeunit[1];
    delete _pwrdn_ck;
    delete _wake_ck;
    delete _brightness;
    delete _bat_warning;
    delete _mainw;

}

/*******************************************************************************\
**
**	Function:	NxBacklite()
**	Desc:		Class NxBacklite constructor, handles parsing of commandline
**				arguments
**	Accepts:	int argc = Number of arguments on instantiation
**				char **argv = Argument vector
**				char *appname = Applications par name
**	Returns:	N/A
**
\*******************************************************************************/
NxBacklite::NxBacklite(int X, int Y, int W, int H, char *appname)
{
    _mainw = 0;

    // Get the current power mode....
    if (pix_pwr_onBattery())
	_blMode = blBATTERY;
    else
	_blMode = blACPOWER;

    memset(&_bl_settings, 0, sizeof(_bl_settings));

    _winX = X;
    _winY = Y;

    // Get the Application preferences from PAR
    GetAppPrefs();

    // Build the window and widgets
    MakeWindow(X, Y, W, H);

    // Set the initial values....
    SetValues();
    SetBl();
}

void
NxBacklite::GetAppPrefs(void)
{
    char *pardb;		// name of the default database
    db_handle *hdb;		// Database handle
    int idx;			// Index value

    // Setup the database
    if ((pardb = db_getDefaultDB()) == NULL) {
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RDONLY)) == NULL) {
	return;
    }				// end of if

    // Get the stored values for the ac power
    idx = (int) blACPOWER;
    if (par_getGlobalPref(hdb, "backlight", "ac_timeout", PAR_INT,
			  &_bl_settings[idx].tmout_val, sizeof(int)) == -1)
	printf("No ac_timeout value!");
    if (par_getGlobalPref(hdb, "backlight", "ac_wakeup", PAR_BOOL,
			  &_bl_settings[idx].wake_up, sizeof(bool)) == -1)
	printf("No ac_wakeup value!");
    if (par_getGlobalPref(hdb, "backlight", "ac_level", PAR_INT,
			  &_bl_settings[idx].brite_val, sizeof(int)) == -1)
	printf("No ac_level value!");

    // Get the stored values for the batter power
    idx = (int) blBATTERY;
    if (par_getGlobalPref(hdb, "backlight", "bat_timeout", PAR_INT,
			  &_bl_settings[idx].tmout_val, sizeof(int)) == -1)
	printf("No bat_timeout value!");
    if (par_getGlobalPref(hdb, "backlight", "bat_wakeup", PAR_BOOL,
			  &_bl_settings[idx].wake_up, sizeof(bool)) == -1)
	printf("No bat_wakeup value!");
    if (par_getGlobalPref(hdb, "backlight", "bat_level", PAR_INT,
			  &_bl_settings[idx].brite_val, sizeof(int)) == -1)
	printf("No bat_level value!");

    // Close the database and return
    db_closeDB(hdb);
    return;
}				// end of NxBacklite::GetAppPrefs(void)

void
NxBacklite::ShowWindow(void)
{
    _mainw->show();
}

void
NxBacklite::HideWindow(void)
{
    _mainw->hide();
}

/*******************************************************************************\
**
**	Function:	void MakeWindow()
**	Desc:		Creates the main fltk window and adds the appropriate widgets to
**				it
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::MakeWindow(int X, int Y, int W, int H)
{
    int curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4;			// Left margin

    NxApp *instance = sysconf_get_instance();

    if (_mainw)
	return;

    _mainw = new Fl_Group(X, Y, W, H);
    _mainw->color(instance->getGlobalColor(APP_BG));

    // Set the widgets
    cury = Y + BUTTON_Y - _winY;
    curx = X + BUTTON_X;
    {
	// Add the bottom "OK" button
	NxButton *o;		// Ok button

	o = new NxButton(curx, cury, BUTTON_WIDTH, BUTTON_HEIGHT, "Save");
	o->when(FL_WHEN_RELEASE);
	o->callback(save_exit_cb, (void *) this);
	curx += 63;
    }				// end of OK button
    {
	// Add the bottom "Cancel" Button
	NxButton *o = new NxButton(curx, cury, BUTTON_WIDTH,
				   BUTTON_HEIGHT, "Reset");

	o->when(FL_WHEN_RELEASE);
	o->callback(save_exit_cb, (void *) this);
    }				// end of Cancel button
    cury = Y + 2 * mar;
    curx = X + mar;
    {
	// Add the Label for the mode selection
	NxOutput *o = new NxOutput(curx, cury, (_mainw->w() - curx) - MB_W,
				   BUTTON_HEIGHT);

	o->selection_color(o->color());
	o->align(FL_ALIGN_LEFT);
	o->value("Power mode: ");
	curx = _mainw->w() - (curx + MB_W);
    }				// end of ML text widget
    {
	NxMenuButton *o = _mbmode = new NxMenuButton(curx, cury, MB_W, MB_H);

	instance->def_font(o);
	for (int i = 0; i < ((int) blACPOWER + 1); i++)
	    o->add(mode_lbls[i]);
	o->box(FL_BORDER_BOX);
	o->when(FL_WHEN_CHANGED);
	o->callback(mode_sel_cb, (void *) this);
	o->label("Unknown");
	cury += (int) (BUTTON_HEIGHT * 2);
	curx = X + mar;
	DPRINTF("Menubutton: x=%d y=%d w=%d h=%d.\n", o->x(), o->y(), o->w(),
		o->h());
    }				// end of Mode menu button
    {
	// Add the Multiline warning...
	NxMlOutput *o = _bat_warning = new NxMlOutput(curx, cury,
						      _mainw->w() - curx,
						      (2 * BUTTON_HEIGHT));

	o->color(_mainw->color());
	o->selection_color(o->color());
	o->value(def_bat_warn);
	instance->def_font(o);

	cury += o->h() + BUTTON_HEIGHT;
	o->hide();
    }				// end of battery warning widget
    {
	NxCheckButton *o = new NxCheckButton(curx, cury);
	instance->def_font(o);
	o->when(FL_WHEN_CHANGED);
	o->callback(pwrdn_ck_cb, (void *) this);
	o->box(FL_FLAT_BOX);
	o->label("Turn off backlight if\nidle for: ");
	_pwrdn_ck = o;

	curx = _mainw->w() - ((2 * mar) + MB_W);
    }				// end of Power down check button
    {
	NxMenuButton *o = new NxMenuButton(curx, cury, MB_W + mar, MB_H);

	instance->def_font(o);
	for (int i = 0; tmout_lbls[(int) blACPOWER][i]; i++)
	    o->add(tmout_lbls[(int) blACPOWER][i]);
	o->box(FL_BORDER_BOX);
	o->when(FL_WHEN_CHANGED);
	o->callback(tm_sel_cb, (void *) this);
	o->label("Unset");
	_mbtimeunit[(int) blACPOWER] = o;
	o->hide();
    }				// end of ACPower timevals menu button
    {
	NxMenuButton *o = new NxMenuButton(curx, cury, MB_W + mar, MB_H);

	instance->def_font(o);
	for (int i = 0; tmout_lbls[(int) blBATTERY][i]; i++)
	    o->add(tmout_lbls[(int) blBATTERY][i]);
	o->box(FL_BORDER_BOX);
	o->when(FL_WHEN_CHANGED);
	o->callback(tm_sel_cb);
	o->label("Unset");
	_mbtimeunit[(int) blBATTERY] = o;
	curx = X + mar;
	cury += (3 * BUTTON_HEIGHT);
    }				// end of BATTERY timevals menu button
    {
	NxCheckButton *o = new NxCheckButton(curx, cury);
	instance->def_font(o);
	o->box(FL_FLAT_BOX);
	o->label
	    ("Turn on backlight when a button is\npressed or the screen is tapped");
	o->when(FL_WHEN_CHANGED);
	o->callback(wake_toggle_cb, (void *) this);
	_wake_ck = o;
	curx = X + mar;
	cury = cury + o->h() + (2 * BUTTON_HEIGHT);
    }				// end of wake check_box widget
    {
	NxValueSlider *o =
	    new NxValueSlider(curx, cury, _mainw->w() - (5 * curx),
			      BUTTON_HEIGHT);

	instance->def_font(o);
	o->box(FL_BORDER_BOX);
	o->maximum(100.0);
	o->minimum(0.0);
	o->type(FL_HORIZONTAL);
	o->step(1.0);
	o->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	o->label("Brightness Level:");
	o->when(FL_WHEN_CHANGED);
	o->callback(brite_sel_cb, (void *) this);

	_brightness = o;
    }				// end of slider
    _mainw->end();
    _mainw->hide();

    return;
}				// end of NxBacklite::MakeWindow(void)

/*******************************************************************************\
**
**	Function:	void SetAppPrefs()
**	Desc:		Stores any changed values into the PAR database
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxBacklite::SetAppPrefs(void)
{
    char *pardb;		// Database name
    int idx;			// Quick index variable
    db_handle *hdb;		// Database handle

    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("Unable to get the current database\n");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RW)) == NULL) {
	printf("Unable to open the database %s\n", pardb);
	return;
    }				// end of if

    // Write out any ACPOWER changes
    idx = (int) blACPOWER;
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_BRITE) &&
	par_setGlobalPref(hdb, "backlight", "ac_level", PAR_INT,
			  (char *) &_bl_settings[idx].brite_val,
			  sizeof(int)) == -1)
	printf("PAR error writing backlight.ac_level(%d) error=%d",
	       _bl_settings[idx].brite_val, pardb_errno);
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_TMOUT)
	&& par_setGlobalPref(hdb, "backlight", "ac_timeout", PAR_INT,
			     (char *) &_bl_settings[idx].tmout_val,
			     sizeof(int)) == -1)
	printf("PAR error writing backlight.ac_timeout(%d) error=%d",
	       _bl_settings[idx].tmout_val, pardb_errno);
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_WAKEUP)
	&& par_setGlobalPref(hdb, "backlight", "ac_wakeup", PAR_BOOL,
			     (char *) &_bl_settings[idx].wake_up,
			     sizeof(bool)) == -1)
	printf("PAR error writing backlight.ac_wakeup, error=%d",
	       pardb_errno);

    // Write out any BATTERY changes
    idx = (int) blBATTERY;
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_BRITE) &&
	par_setGlobalPref(hdb, "backlight", "bat_level", PAR_INT,
			  (char *) &_bl_settings[idx].brite_val,
			  sizeof(int)) == -1)
	printf("PAR error writing backlight.bat_level(%d) error=%d",
	       _bl_settings[idx].brite_val, pardb_errno);
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_TMOUT)
	&& par_setGlobalPref(hdb, "backlight", "bat_timeout", PAR_INT,
			     (char *) &_bl_settings[idx].tmout_val,
			     sizeof(int)) == -1)
	printf("PAR error writing backlight.bat_timeout(%d) error=%d",
	       _bl_settings[idx].tmout_val, pardb_errno);
    if ((_bl_settings[idx].dirty_flgs & BLVAL_DFLGS_WAKEUP)
	&& par_setGlobalPref(hdb, "backlight", "bat_wakeup", PAR_BOOL,
			     (char *) &_bl_settings[idx].wake_up,
			     sizeof(bool)) == -1)
	printf("PAR error writing backlight.bat_wakeup, error=%d",
	       pardb_errno);

    // Send broadcast message to anyone who cares...
    char col_msg[CL_MAX_MSG_LEN] = { '\0' };
    int col_len = sprintf(col_msg, "sc_backlite^SYSCON_BL_CHANGE");

    /* Call back to the main sysconfig app */
    sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);
    return;
}				// end of NxBacklite::SetAppPrefs(void)

/*******************************************************************************\
**
**	Function:	void SetBl()
**	Desc:		Performs the actual setting (i.e. making the pixlib calls) of the
**				backlight.
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxBacklite::SetBl(void)
{
    char col_msg[CL_MAX_MSG_LEN];
    int bl_max,			// Maximum value of the backlight
      bl_val,			// Value to set
      col_len, idx,		// Array index
      scrtp_id;
    static int bl_oldval = 0;	// Statically saved value

    DPRINTF("In SetBl() -- top\n");
    idx = (int) _blMode;
    bl_max = pix_bl_getmxval();
    bl_val =
	(int) (((float) ((bl_max * _bl_settings[idx].brite_val) / 100.0)));

    // Get the screen top id from colosseum
    if (bl_val == 0 || bl_oldval == 0) {
	DPRINTF("Getting scrtop id, values=(N)%d (O)%d\n", bl_val, bl_oldval);
	scrtp_id = sysconf_ipc_find("nxscrtop");
	if (scrtp_id > -1) {
	    scrtop_message msg;	// Message to send to the screentop

	    col_len = sizeof(msg.action);
	    memset(&msg, 0, col_len);
	    // Send a message to the screentop to switch to bl off image
	    DPRINTF("Preparing \"%s\" message\n",
		    bl_val == 0 ? "BL_OFF" : "BL_ON");
	    if (bl_val == 0)
		msg.type = CATEGORY_ITEM(CATEGORY_NA_ACTION, NA_ACTION_BLOFF);
	    else
		msg.type = CATEGORY_ITEM(CATEGORY_NA_ACTION, NA_ACTION_BLON);

	    memcpy(col_msg, &msg, col_len);

	    sysconf_ipc_write(scrtp_id, col_msg, col_len);
	} else {
	    DPRINTF("Find_Fd() failed on \"nxscrtop\"\n");
	}
    }
    // Make the call after communicating with the screen top, to ensure the levels
    // are correctly set.
    DPRINTF("Before pix_bl_ctrl()\n");
    pix_bl_ctrl(bl_val ? 1 : 0, bl_val);

    DPRINTF("Saving %d into bl_oldval\n", bl_val);
    bl_oldval = bl_val;

    return;
}				// end of NxBacklite::SetBl(void)

/*******************************************************************************\
**
**	Function:	void SetValues()
**	Desc:		Sets the widgets values based upon the current state of the 
**				application (_mode)
**	Accpets:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::SetValues(void)
{
    int idx;			// Shorthand notation

    printf("MBMODE is still %p\n", _mbmode);

    switch (_blMode) {
    case blUNKNOWN:
	_mbmode->label("Unknown");
	_bat_warning->hide();
	_pwrdn_ck->hide();
	_wake_ck->hide();
	_brightness->hide();
	_mbtimeunit[(int) blBATTERY]->hide();
	_mbtimeunit[(int) blACPOWER]->hide();
	break;
    case blBATTERY:
	_mbmode->label(mode_lbls[(int) blBATTERY]);
	_mbtimeunit[(int) blBATTERY]->show();
	_mbtimeunit[(int) blACPOWER]->hide();
	_bat_warning->show();
	_pwrdn_ck->show();
	_wake_ck->show();
	_brightness->show();
	break;
    case blACPOWER:
	_mbmode->label(mode_lbls[(int) blACPOWER]);
	_mbtimeunit[(int) blBATTERY]->hide();
	_mbtimeunit[(int) blACPOWER]->show();
	_bat_warning->hide();
	_pwrdn_ck->show();
	_wake_ck->show();
	_brightness->show();
	break;
    }				// end of switch

    // Set the values....
    if (_blMode != blUNKNOWN) {
	idx = (int) _blMode;

	// Set the timeout stuff
	if (_bl_settings[idx].tmout_val > 0) {
	    int i;		// Loop iterator

	    _pwrdn_ck->value(1);
	    _mbtimeunit[idx]->activate();
	    for (i = 0; tmout_vals[idx][i] != -1; i++) {
		if (_bl_settings[idx].tmout_val == tmout_vals[idx][i]) {
		    _mbtimeunit[idx]->label(tmout_lbls[idx][i]);
		    break;
		}		// end of if
	    }			// end of for
	    if (tmout_vals[idx][i] == -1)
		_mbtimeunit[idx]->label("Not Set");
	}			// end of if
	else {
	    _pwrdn_ck->value(0);
	    _mbtimeunit[idx]->deactivate();
	}			// end of else

	// Set the wakeup stuff
	if (_bl_settings[idx].wake_up == true)
	    _wake_ck->value(1);
	else
	    _wake_ck->value(0);

	// Set the backlite brightness value
	if (_bl_settings[idx].brite_val >= 0) {
	    _bl_settings[idx].brite_val =
		(int) _brightness->clamp(_bl_settings[idx].brite_val);
	    _brightness->value(_bl_settings[idx].brite_val);
	}			// end of if
    }				// end of if
    return;
}				// end of NxBacklite::SetValues(void)

//-------------------------------------------------------------------------------
//
//      Private static callback methods
//              void brite_sel_cb(Fl_Widget *, void *)
//              void mode_sel_cb(Fl_Widget *, void *)
//              void pwrdn_ck_cb(Fl_Widget *, void *)
//              void save_exit_cb(Fl_Widget *, void *)
//              void tm_sel_cb(Fl_Widget *, void *)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void brite_sel_cb()
**	Desc:		Callback for the brightness level widget, will adjust the level using
**				pixlib calls
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::brite_sel_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxValueSlider *nxvs = (NxValueSlider *) w;
    int idx;

    idx = (int) me->_blMode;
    me->_bl_settings[idx].brite_val = (int) nxvs->value();
    me->_bl_settings[idx].dirty_flgs |= BLVAL_DFLGS_BRITE;

    // Actuall change the backlight value....
    me->SetBl();

    return;
}				// end of NxBacklite::brite_sel_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void mode_sel_cb()
**	Desc:		Callback for mode selection (toggles between battery/external power)
**	Accepts:	Fl_Widget *w = Ptr to widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::mode_sel_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxMenuButton *nxmb = (NxMenuButton *) w;

    me->_blMode = (bl_mode_t) nxmb->value();

    me->SetValues();
    me->SetBl();

    return;
}				// end of NxBacklite::mode_sel_Cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void pwrlnk_cb()
**	Desc:		Callback to start the power-management app/utility
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::pwrlnk_cb(Fl_Widget * w, void *d)
{
    DPRINTF("In pwrlnk_cb()\n");
    return;
}				// end of NxBacklite::pwrlnk_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void pwrdn_ck_cb()
**	Desc:		Callback to for the powerdown checkbox widget
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::pwrdn_ck_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxCheckButton *nxcb = (NxCheckButton *) w;

    if (nxcb->value())
	me->_mbtimeunit[(int) me->_blMode]->activate();
    else {
	me->_mbtimeunit[(int) me->_blMode]->deactivate();
	me->_bl_settings[(int) me->_blMode].tmout_val = 0;
	me->_bl_settings[(int) me->_blMode].dirty_flgs |= BLVAL_DFLGS_TMOUT;
    }				// end of else

    return;
}				// end of NxBacklite::pwrdn_ck_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void save_exit_cb()
**	Desc:		Handles the save/exit of this application/utility
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::save_exit_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxButton *nxb = (NxButton *) w;

    if (!strcmp(nxb->label(), "Save")) {
	DPRINTF("Saving any changed data!\n");
	me->SetAppPrefs();
	me->_mbmode->value(-1);
	me->_mbtimeunit[0]->value(-1);
	me->_mbtimeunit[1]->value(-1);
    }				// end of if

    // Need to set the backlight back to what it was last stored as (in case we've just
    // canceled/reset
    if (pix_pwr_onBattery())
	me->_blMode = blBATTERY;
    else
	me->_blMode = blACPOWER;

    me->GetAppPrefs();
    me->SetValues();
    me->SetBl();

    return;
}				// end of NxBacklite::save_exit_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void tm_sel_cb()
**	Desc:		Callback for the timeunit menu button widgets
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::tm_sel_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxMenuButton *nxmb = (NxMenuButton *) w;
    int idx;			// Index

    DPRINTF("In tm_sel_cb()\n");
    idx = (int) me->_blMode;

    // Store the value....
    me->_bl_settings[idx].tmout_val = tmout_vals[idx][nxmb->value()];
    me->_bl_settings[idx].dirty_flgs |= BLVAL_DFLGS_TMOUT;

    nxmb->label(tmout_lbls[idx][nxmb->value()]);
    nxmb->redraw();

    return;
}				// end of NxBacklite::tm_sel_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void wake_toggle_cb()
**	Desc:		Sets the value of the wake up on touch variable depending on current
**				state of the check button
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxBacklite::wake_toggle_cb(Fl_Widget * w, void *d)
{
    NxBacklite *me = (NxBacklite *) d;
    NxCheckButton *nxcb = (NxCheckButton *) w;
    int idx;			// index

    idx = (int) me->_blMode;
    if (nxcb->value())
	me->_bl_settings[idx].wake_up = true;
    else
	me->_bl_settings[idx].wake_up = false;
    me->_bl_settings[idx].dirty_flgs |= BLVAL_DFLGS_WAKEUP;

    return;
}				// end of NxBacklite::wake_toggle_cb(Fl_Widget *, void *)
