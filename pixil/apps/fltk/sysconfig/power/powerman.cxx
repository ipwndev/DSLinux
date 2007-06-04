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
#include <pixlib/pixlib.h>
#include "nxpwr.h"

static char *tmout_lbls[2][7] = {
    {
     "30 Seconds",
     "1 Minute",
     "2 Minutes",
     "3 Minutes",
     "4 Minutes",
     "5 Minutes",
     NULL},
    {"1 Minute",
     "2 Minutes",
     "5 Minutes",
     "10 Minutes",
     "15 Minutes",
     "30 Minutes",
     NULL}
};
static signed short tmout_vals[2][7] = {
    {30, 60, 120, 180, 240, 300, -1},
    {60, 120, 300, 600, 900, 1800, -1}
};
static float f_sltimer = PWR_BATTERY_TMR;	// Default BATTERY seconds

//-------------------------------------------------------------------------------
//
//      NxPowerman constructor(s) and destructor
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	~NxPowerman()
**	Desc:		Class NxPowerman destructor, responsible for free()ing dynamic
**				memory
**	Accepts:	N/A
**	Returns:	N/A
**
\*******************************************************************************/
NxPowerman::~NxPowerman()
{
    delete _mbtns[0];
    delete _mbtns[1];
    delete _cbtns[0];
    delete _cbtns[1];
    delete _batsl;
    delete _batstate;
    delete _mainw;

}				// end of NxPowerman::~NxPowerman()

/*******************************************************************************\
**
**	Function:	NxPowerman()
**	Desc:		Class NxPowerman constructor, handles parsing of commandline
**				arguments
**	Accepts:	int argc = Number of arguments on instantiation
**				char **argv = Argument vector
**				char *appname = Applications par name
**	Returns:	N/A
**
\*******************************************************************************/
NxPowerman::NxPowerman(int X, int Y, int W, int H, char *appname)
{
    memset(&_pwr_settings, 0, sizeof(_pwr_settings));

    _mainw = 0;

    _winX = X;
    _winY = Y;

    // Get the Application preferences from PAR
    GetAppPrefs();

    // Build the window and widgets
    MakeWindow(X, Y, W, H);

    // Set the initial values....
    SetValues();
}

void
NxPowerman::ShowWindow(void)
{
    Fl::add_timeout(f_sltimer, sl_upd_tmr, (void *) this);
    _mainw->show();
}

void
NxPowerman::HideWindow(void)
{
    Fl::remove_timeout(sl_upd_tmr, (void *) this);
    _mainw->hide();
}

void
NxPowerman::GetAppPrefs(void)
{
    char *pardb;		// name of the default database
    db_handle *hdb;		// Database handle

    // Setup the database
    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No default database present!");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RDONLY)) == NULL) {
	printf("Error opening %s, error=%d", pardb, pardb_errno);
	return;
    }				// end of if

    // Get the stored values for the ac power
    if (par_getGlobalPref(hdb, "power", "ac_off", PAR_BOOL,
			  &_pwr_settings[AC_IDX].pwr_off, sizeof(bool)) == -1)
	printf("No ac_off value!");
    if (par_getGlobalPref(hdb, "power", "ac_offval", PAR_INT,
			  &_pwr_settings[AC_IDX].timeval, sizeof(int)) == -1)
	printf("No ac_offval value!");

    // Get the stored values for the battery power
    if (par_getGlobalPref(hdb, "power", "bat_off", PAR_BOOL,
			  &_pwr_settings[BAT_IDX].pwr_off,
			  sizeof(bool)) == -1)
	printf("No bat_off value!");
    if (par_getGlobalPref(hdb, "power", "bat_offval", PAR_INT,
			  &_pwr_settings[BAT_IDX].timeval, sizeof(int)) == -1)
	printf("No bat_offval value!");

    db_closeDB(hdb);
    return;
}

void
NxPowerman::MakeWindow(int X, int Y, int W, int H)
{
    int curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4;			// Left margin
    NxApp *instance = sysconf_get_instance();

    // Make the window.... (and all necessary widgets to it)
    _mainw = new Fl_Group(X, Y, W, H);
    _mainw->color(instance->getGlobalColor(APP_BG));
    // Set the widgets
    curx = X + BUTTON_X;
    cury = Y + BUTTON_Y - _winY;

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
    }
    cury = Y + (2 * mar);
    curx = X + mar;
    {
	// Add the Label for the mode selection
	NxOutput *o = new NxOutput(curx, cury, (_mainw->w() - curx) - MB_W,
				   BUTTON_HEIGHT);

	instance->def_font(o);
	o->selection_color(o->color());
	o->align(FL_ALIGN_LEFT);
	o->textfont(FL_BOLD);
	o->value("Power state output");
	_batstate = o;
	cury += BUTTON_HEIGHT;
    }				// end of ML text widget
    {
	NxSlider *o;
	NxOutput *lbl;

	// Label
	lbl = new NxOutput(_mainw->w() - 45, cury, 40, BUTTON_HEIGHT);
	instance->def_font(lbl);
	lbl->selection_color(lbl->color());
	lbl->align(FL_ALIGN_LEFT);
	lbl->textfont(FL_BOLD);
	lbl->value("100%");
	lbl->deactivate();

	// Slider value
	o = new NxSlider(curx + (6 * mar), cury,
			 _mainw->w() - ((8 * mar) + lbl->w()), BUTTON_HEIGHT);

	instance->def_font(o);
	o->box(FL_BORDER_BOX);
	o->maximum(100.0);
	o->minimum(0.0);
	o->type(FL_HOR_FILL_SLIDER);
	o->step(1.0);
	o->align(FL_ALIGN_LEFT);
	o->label("0%");
	o->labelfont(FL_BOLD);
	o->deactivate();
	_batsl = o;

	cury += (4 * BUTTON_HEIGHT);
    }				// end of slider widget
    {
	NxOutput *o = new NxOutput(curx, cury, _mainw->w(), BUTTON_HEIGHT);

	instance->def_font(o);
	o->box(FL_FLAT_BOX);
	o->selection_color(o->color());
	o->align(FL_ALIGN_LEFT);
	o->textfont(FL_BOLD);
	o->value("In Battery Mode:");
	cury += BUTTON_HEIGHT;
    }				// end of Battery label widget
    {
	NxCheckButton *o = new NxCheckButton(curx, cury);
	instance->def_font(o);
	o->when(FL_WHEN_CHANGED);
	o->callback(cb_cb, (void *) this);
	o->box(FL_FLAT_BOX);
	o->label("Turn off device if\nidle for: ");
	_cbtns[0] = o;

	curx = _mainw->w() - ((2 * mar) + MB_W);
    }				// end of battery power down check button
    {
	NxMenuButton *o = new NxMenuButton(curx, cury, MB_W + mar, MB_H);

	instance->def_font(o);
	for (int i = 0; tmout_lbls[BAT_IDX][i]; i++)
	    o->add(tmout_lbls[BAT_IDX][i]);
	o->box(FL_BORDER_BOX);
	o->when(FL_WHEN_CHANGED);
	o->callback(mb_cb, (void *) this);
	o->label("Unset");
	_mbtns[BAT_IDX] = o;
	curx = mar;
	cury += 3 * BUTTON_HEIGHT;
    }				// end of Battery timevals menu button
    {
	NxOutput *o = new NxOutput(curx, cury, _mainw->w(), BUTTON_HEIGHT);

	instance->def_font(o);
	o->box(FL_FLAT_BOX);
	o->selection_color(o->color());
	o->align(FL_ALIGN_LEFT);
	o->textfont(FL_BOLD);
	o->value("In External Power Mode:");
	cury += BUTTON_HEIGHT;
    }				// end of ACPower label widget
    {
	NxCheckButton *o = new NxCheckButton(curx, cury);

	instance->def_font(o);
	o->when(FL_WHEN_CHANGED);
	o->callback(cb_cb, (void *) this);
	o->box(FL_FLAT_BOX);
	o->label("Turn off device if\nidle for: ");
	_cbtns[1] = o;

	curx = _mainw->w() - ((2 * mar) + MB_W);
    }				// end of battery power down check button
    {
	NxMenuButton *o = new NxMenuButton(curx, cury, MB_W + mar, MB_H);

	instance->def_font(o);
	for (int i = 0; tmout_lbls[AC_IDX][i]; i++)
	    o->add(tmout_lbls[AC_IDX][i]);
	o->box(FL_BORDER_BOX);
	o->when(FL_WHEN_CHANGED);
	o->callback(mb_cb, (void *) this);
	o->label("Unset");
	_mbtns[AC_IDX] = o;
	curx = mar;
	cury += BUTTON_HEIGHT;
    }				// end of Battery timevals menu button
    _mainw->end();
    _mainw->hide();

    return;
}				// end of NxPowerman::MakeWindow(void)

/*******************************************************************************\
**
**	Function:	void SetAppPrefs()
**	Desc:		Stores any changed values into the PAR database
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxPowerman::SetAppPrefs(void)
{
    char *pardb;		// Database name
    db_handle *hdb;		// Database handle

    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No Default database! Changes lost");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RW)) == NULL) {
	printf("Unable to open %s, error=%d", pardb, pardb_errno);
	return;
    }				// end of if

    // Write out any ACPOWER changes
    if ((_pwr_settings[AC_IDX].dirty_flg & PWR_DFLGS_PWRCB) &&
	par_setGlobalPref(hdb, "power", "ac_off", PAR_BOOL,
			  (char *) &_pwr_settings[AC_IDX].pwr_off,
			  sizeof(bool)) == -1)
	printf("PAR error writing power.ac_off(%d) error=%d",
	       _pwr_settings[AC_IDX].pwr_off, pardb_errno);
    if ((_pwr_settings[AC_IDX].dirty_flg & PWR_DFLGS_TMVAL)
	&& par_setGlobalPref(hdb, "power", "ac_offval", PAR_INT,
			     (char *) &_pwr_settings[AC_IDX].timeval,
			     sizeof(int)) == -1)
	printf("PAR error writing power.ac_offval(%d) error=%d",
	       _pwr_settings[AC_IDX].timeval, pardb_errno);

    // Write out any BATTERY changes
    if ((_pwr_settings[BAT_IDX].dirty_flg & PWR_DFLGS_PWRCB) &&
	par_setGlobalPref(hdb, "power", "bat_off", PAR_BOOL,
			  (char *) &_pwr_settings[BAT_IDX].pwr_off,
			  sizeof(bool)) == -1)
	printf("PAR error writing power.bat_off(%d) error=%d",
	       _pwr_settings[BAT_IDX].pwr_off, pardb_errno);
    if ((_pwr_settings[BAT_IDX].dirty_flg & PWR_DFLGS_TMVAL)
	&& par_setGlobalPref(hdb, "power", "bat_offval", PAR_INT,
			     (char *) &_pwr_settings[BAT_IDX].timeval,
			     sizeof(int)) == -1)
	printf("PAR error writing power.bat_offval, error=%d", pardb_errno);

    // Need to send broadcast message about power changes
    char col_msg[CL_MAX_MSG_LEN] = { '\0' };
    int col_len = sprintf(col_msg, "sc_power^SYSCON_PWR_CHANGE");
    sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);

    return;
}				// end of NxPowerman::SetAppPrefs(void)

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
NxPowerman::SetValues(void)
{
    int i,			// Loop iterator
      j;			// Loop iterator

    // Set check buttons and menu buttons....
    for (i = 0; i < AC_IDX + 1; i++) {
	// Get the current time out value and activate the menu button
	for (j = 0; tmout_vals[i][j] != -1; j++) {
	    if (tmout_vals[i][j] == _pwr_settings[i].timeval) {
		_mbtns[i]->label(tmout_lbls[i][j]);
		break;
	    }			// end of if
	}			// end of for
	if (tmout_vals[i][j] == -1)
	    _mbtns[i]->label("Unset");

	if (_pwr_settings[i].pwr_off == true) {
	    _cbtns[i]->value(1);
	    _mbtns[i]->activate();
	}			// end of if
	else {
	    _cbtns[i]->value(0);
	    _mbtns[i]->deactivate();
	}			// end of else
	_mbtns[i]->redraw();
    }				// end of for 

    // Set the slider value

    // Determine if we are on battery power or line power
    if (pix_pwr_onBattery()) {
	// We are on battery power */
	_batsl->value(pix_pwr_getbat(PWR_BAT_PERCENT));
	_batstate->value("Power state: Battery");
    }				// end of if
    else {
	// We are on External power
	if (pix_pwr_isCharging()) {
	    float curval;	// Current value

	    curval = _batsl->value();
	    curval += 10.0;
	    if (curval > _batsl->maximum())
		curval = _batsl->minimum();
	    _batsl->value(curval);
	    _batstate->value("Power State: Charging");
	}			// end of if
	else {
	    _batsl->value(_batsl->maximum());
	    _batstate->value("Power State: External");
	}			// end of else
    }				// end of else
    _batsl->redraw();

    return;
}				// end of NxBPowerman::SetValues(void)

//-------------------------------------------------------------------------------
//
//      Private static callback methods
//              static void cb_cb(Fl_Widget *, void *)
//              static void mb_cb(Fl_Widget *, void *)
//              static void save_exit_cb(Fl_Widget *, void *)
//              static void sl_upd_tmr(void *d)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void cb_cb()
**	Desc:		Callback for setting the activeness of the menu buttons
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxPowerman::cb_cb(Fl_Widget * w, void *d)
{
    NxPowerman *me = (NxPowerman *) d;
    NxCheckButton *nxcb = (NxCheckButton *) w;
    int idx;

    if (me->_cbtns[0] == nxcb)
	idx = BAT_IDX;
    else
	idx = AC_IDX;

    // If we are in here, the value had to have changed!
    me->_pwr_settings[idx].dirty_flg |= PWR_DFLGS_PWRCB;
    if (nxcb->value())
	me->_pwr_settings[idx].pwr_off = true;
    else
	me->_pwr_settings[idx].pwr_off = false;

    me->SetValues();

    return;
}				// end of NxPowerman::cb_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void mb_cb()
**	Desc:		Callback for time value selections
**	Accepts:	Fl_Widget *w = Ptr to widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxPowerman::mb_cb(Fl_Widget * w, void *d)
{
    NxPowerman *me = (NxPowerman *) d;
    NxMenuButton *nxmb = (NxMenuButton *) w;
    int idx;

    if (nxmb == me->_mbtns[0])
	idx = BAT_IDX;
    else
	idx = AC_IDX;

    if (nxmb->value() > -1) {
	me->_pwr_settings[idx].timeval = tmout_vals[idx][nxmb->value()];
	me->_pwr_settings[idx].dirty_flg |= PWR_DFLGS_TMVAL;
    }				// end of if

    me->SetValues();

    return;
}				// end of NxPowerman::mb_cb(Fl_Widget *, void *)

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
NxPowerman::save_exit_cb(Fl_Widget * w, void *d)
{
    NxPowerman *me = (NxPowerman *) d;
    NxButton *nxb = (NxButton *) w;

    if (!strcmp(nxb->label(), "Save")) {
	me->SetAppPrefs();
    }				// end of if

    // Update as needed (incase of a reset)
    me->GetAppPrefs();
    me->SetValues();

    return;
}				// end of NxPowerman::save_exit_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void sl_upd_tmr()
**	Desc:		Timer callback to get the current battery level, and update the slider
**	Accepts:	void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxPowerman::sl_upd_tmr(void *d)
{
    NxPowerman *me = (NxPowerman *) d;

    if (pix_pwr_isCharging())
	f_sltimer = PWR_CHARGE_TMR;
    else
	f_sltimer = PWR_BATTERY_TMR;

    me->SetValues();

    Fl::add_timeout(f_sltimer, me->sl_upd_tmr, d);
    return;
}				// end of NxPowerman::sl_upd_tmr(void *)
