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
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Local header files
#include <FL/Enumerations.H>
#include <nxmultilineoutput.h>
#include "nxcal.h"
#include "cal_ui.h"

#include <sysconf_plugin.h>

static const char *wid_message =
    "Use this utility to re-align the touch\nscreen if the "
    "device is not responding\naccurately to screen taps.";

/*******************************************************************************\
**
**	Function:	~NxCal()
**	Desc:		Class NxCal destructor, responsible for free()ing dynamic
**				memory
**	Accepts:	N/A
**	Returns:	N/A
**
\*******************************************************************************/
NxCal::~NxCal()
{
    delete[]_txtfile;
    if (_calmode == clINTER) {
	delete _proceed;
	delete _mainw;
    }				// end of if
}				// end of NxCal::~NxCal()

/*******************************************************************************\
**
**	Function:	NxCal()
**	Desc:		Class NxCal constructor, handles parsing of commandline
**				arguments
**	Accepts:	int argc = Number of arguments on instantiation
**				char **argv = Argument vector
**				char *appname = Applications par name
**	Returns:	N/A
**
\*******************************************************************************/
NxCal::NxCal(int X, int Y, int W, int H, char *appname)
{
    _calmode = clINTER;		// Default to Non-interactive mode
    _txtfile = NULL;
    _winX = X;
    _winY = Y;

    // Get the default filename from PAR
    GetAppData();

    if (_calmode == clINTER) {
	MakeWindow(X, Y, W, H);
    }
}

void
NxCal::ShowWindow(void)
{
    if (_mainw)
	_mainw->show();
}

void
NxCal::HideWindow(void)
{
    if (_mainw)
	_mainw->hide();
}

/*******************************************************************************\
**
**	Function:	int StartnxCal()
**	Desc:		Starts the raw Microwindows screen calibrator application via
**				fork()/exec().
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
int
NxCal::StartnxCal(void)
{
    int nxcal_flgs = 0;		/* Flags */

    if (_calmode == clNONINTER)
	nxcal_flgs |= NXCAL_NONINT_MODE;

    return (nxcal(nxcal_flgs, _txtfile));
}				// end of NxCal::StartnxCal(void)

/*******************************************************************************\
**
**	Function:	void GetAppData()
**	Desc:		Retrieves the current data for this application from the
**				PAR database.
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxCal::GetAppData(void)
{
    char *pardb,		// name of the default database
     *txt_ev;			// Text environment variable
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

    // See if we need to pull the _txtfile data from either an Environment Variable (preferred)
    // or use the default
    if (_txtfile == NULL) {
	if ((txt_ev = getenv("NXCAL_DATA")) == NULL) {
	    // Need to get the default from PAR
	    char par_data[PATH_MAX] = { '\0' };

	    par_getAppPref(hdb, UTILITY_NAME, "data", "calfile", par_data,
			   sizeof(par_data));
	    if (par_data[0]) {
		_txtfile = new char[strlen(par_data) + 1];
		strcpy(_txtfile, par_data);
	    }			// end of if
	}			// end of if
	else {
	    _txtfile = new char[strlen(txt_ev) + 1];
	    strcpy(_txtfile, txt_ev);
	}			// end of else
    }				// end of if

    // Close the database and return
    db_closeDB(hdb);
    return;
}				// end of NxCal::GetAppData(void)

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
NxCal::MakeWindow(int X, int Y, int W, int H)
{
    int curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4;			// Left margin
    NxApp *instance = sysconf_get_instance();

    // Make the window.... (and all necessary widgets to it)
    _mainw = new Fl_Group(X, Y, W, H);

    _mainw->color(instance->getGlobalColor(APP_BG));

    // Set the widgets
    curx = _winX + mar;
    cury = _winY += mar;
    {
	// Add the multiline message
	NxMultilineOutput *o =
	    new NxMultilineOutput(curx, cury, _mainw->w() - (2 * mar),
				  3 * BUTTON_HEIGHT);

	o->value(wid_message);
	cury += (2 + 3) * BUTTON_HEIGHT;
    }
    {
	// Add the "proceed" button
	int width;
	NxButton *o;

	curx += width = (_mainw->w() - ((2 * mar) + BUTTON_WIDTH)) / 2;
	o = new NxButton(width, cury, BUTTON_WIDTH, BUTTON_HEIGHT);

	o->label("Proceed");
	o->when(FL_WHEN_RELEASE);
	o->callback(cal_cb, (void *) this);
	_proceed = o;
    }
    _mainw->end();
    _mainw->hide();

    return;
}				// end of NxCal::MakeWindow(void)

//-------------------------------------------------------------------------------
//
//      Private static callback methods
//              void cal_cb(Fl_Widget *w, void *d)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void cal_cb()
**	Desc:		Callback to start the calibration routines (calls the raw nanox
**				calibration routines)
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxCal::cal_cb(Fl_Widget * w, void *d)
{
    NxCal *me = (NxCal *) d;

    me->StartnxCal();

    // TODO:  Needs to issue a colosseum message back to the dispatching agent
    // to show/call the previous app, because this one is going to exit!!!
    // and possibly send a message to the window manager to re-draw itself and/or
    // the current app (since this will be screwing up the background)

//      exit(EXIT_SUCCESS);
}				// end of NxCal::cal_cb(Fl_Widget *, void *)
