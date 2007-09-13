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
#include "nxui.h"
#include <pixlib/pixlib.h>

#include <sysconf_plugin.h>

// Typedef, macro, enum/struct/union definitions
#define		VALIDATE_CHAR(c, f)		((((f) & UI_DFLAGS_ALPHA) && isalpha((c))) || \
									 (((f) & UI_DFLAGS_NUMERIC) && isdigit((c))) || \
									 (((f) & UI_DFLAGS_PUNC) && ispunct((c))) || \
									 (((f) & UI_DFLAGS_WHITESP) && isspace((c))) )

static const char *ParIds[UI_FLD_MAXFLDS + 1] = { " ",
    "Name",
    "Company",
    "Address",
    "Phone",
    "Email",
    "Notes"
};

/*******************************************************************************\
**
**	Function:	~NxUserInfo()
**	Desc:		Class NxUserInfo destructor, responsible for free()ing dynamic
**				memory
**	Accepts:	N/A
**	Returns:	N/A
**
\*******************************************************************************/
NxUserInfo::~NxUserInfo()
{
    // Delete the widgets 
    delete _resetb;
    delete _saveb;

    // Delete all of the input widgets
    for (int i = 0; i < UI_FLD_MAXFLDS; i++)
	delete _Inputs[i].input;

    delete _userInfog;
    delete _notesg;
    delete _mainTab;
    delete _mainw;
}				// end of NxUserInfo::~NxUserInfo()

/*******************************************************************************\
**
**	Function:	NxUserInfo()
**	Desc:		Class NxUserInfo constructor, handles parsing of commandline
**				arguments
**	Accepts:	int argc = Number of arguments on instantiation
**				char **argv = Argument vector
**	Returns:	N/A
**
\*******************************************************************************/
NxUserInfo::NxUserInfo(int X, int Y, int W, int H, char *appname)
{
    _winX = X;
    _winY = Y;

    // Build the window and widgets
    MakeWindow(X, Y, W, H);

    // Get the Application preferences from PAR
    GetAppPrefs();

}

/*******************************************************************************\
**
**	Function:	void GetAppPrefs()
**	Desc:		Retrieves the current preferences for this application from the
**				PAR database, storing the results in the _bl_settings member
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxUserInfo::GetAppPrefs(void)
{
    char *pardb,		// name of the default database
      par_data[512] = { '\0' };	// Values from the database
    int idx,			// Index value
      rc;
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

    // Get the User Information from par
    for (idx = 0; idx < UI_FLD_MAXFLDS; idx++) {
	memset(par_data, 0, sizeof(par_data));
	if ((rc =
	     par_getGlobalPref(hdb, "UserId",
			       const_cast <
			       char *>(ParIds[_Inputs[idx].fld_name]),
			       PAR_TEXT, par_data, sizeof(par_data) - 1)) > 0
	    && par_data[0] >= ' ')
	{
	    // Handle the different widget cases
	    if (_Inputs[idx].dta_flags & UI_DFLAGS_SL) {
		NxInput *nxip;	// Input widget ptr

		// Note: This uses the "Truman Hack" to hide/show changed input
		// box widgets to display properly...
		nxip = (NxInput *) _Inputs[idx].input;
		nxip->value(par_data, strlen(par_data));
		nxip->hide();
		nxip->damage(FL_DAMAGE_ALL);
		nxip->redraw();
		nxip->position(strlen(par_data));
		nxip->mark(strlen(par_data));
		nxip->show();
	    }			// end of if
	    else {
		Fl_Editor *nxip;	// Editor widget ptr

		nxip = (Fl_Editor *) _Inputs[idx].input;
		nxip->Clear();
		nxip->LoadFrom(par_data);
	    }			// end of else
	}			// end of if
	else {
	    if (_Inputs[idx].dta_flags & UI_DFLAGS_SL) {
		((NxInput *) _Inputs[idx].input)->value(NULL);
		((NxInput *) _Inputs[idx].input)->damage(FL_DAMAGE_ALL);
		((NxInput *) _Inputs[idx].input)->redraw();
	    }			// end of if
	    else {
		((Fl_Editor *) _Inputs[idx].input)->Clear();
	    }			// end of else
	}			// end of else

	_Inputs[idx].dta_flags &= ~UI_DFLAGS_DIRTY;
    }				// end of for

    // Close the database and return
    db_closeDB(hdb);
    return;
}				// end of NxUserInfo::GetAppPrefs(void)

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
NxUserInfo::MakeWindow(int X, int Y, int W, int H)
{
    int curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4,			// Left margin
      tab_bmar;			// Tab bottom margin
    NxApp *instance = sysconf_get_instance();

    _mainw = new Fl_Group(X, Y, W, H);

    _mainw->color(instance->getGlobalColor(APP_BG));
    tab_bmar = _mainw->h() - ((2 * mar) + BUTTON_HEIGHT);

    curx = X + BUTTON_X;
    cury = Y + BUTTON_Y - _winY;
    {
	NxButton *o;

	o = new NxButton(curx, cury, BUTTON_WIDTH, BUTTON_HEIGHT);
	o->label("Save");
	o->when(FL_WHEN_RELEASE);
	o->callback(save_reset_cb, (void *) this);
	o->show();
	_saveb = o;
	curx += 63;
    }
    {
	NxButton *o;

	o = new NxButton(curx, cury, BUTTON_WIDTH, BUTTON_HEIGHT);
	o->label("Reset");
	o->when(FL_WHEN_RELEASE);
	o->callback(save_reset_cb, (void *) this);
	_resetb = o;
    }
    {
	float max_lbl_wid = 0;	// Label width
	int input_width;
	Fl_Tabs *o;		// Tab widget

	curx = X + mar;
	cury = Y + mar;

	fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
	for (int i = 1; i <= UI_FLD_MAXFLDS; i++) {
	    float tmp_wid;

	    if ((tmp_wid = fl_width(ParIds[i])) > max_lbl_wid)
		max_lbl_wid = tmp_wid;
	}			// end of for
	max_lbl_wid += fl_width(": ") + mar;

	o = new Fl_Tabs(curx, cury, _mainw->w() - (2 * mar),
			tab_bmar - (mar + 10));
	o->box(FL_FLAT_BOX);
	{
	    int grpx, grpy = cury + mar;
	    Fl_Group *g1;	// ID Tab Group

	    g1 = new Fl_Group(curx, cury + mar, o->w(), o->h() - 20);
	    instance->def_font(g1);
	    g1->box(FL_FLAT_BOX);
	    g1->label("User Id");
	    g1->color(_mainw->color());

	    grpx = g1->x() + (int) max_lbl_wid;
	    input_width = g1->w() - ((int) max_lbl_wid + mar);
	    {
		// Name input box
		NxInput *ip;	// Input widget

		ip = new NxInput(grpx, grpy, input_width, 20, "Name: ");
		ip->when(FL_WHEN_CHANGED);
		ip->callback(input_cb, (void *) this);
		ip->maximum_size(_Inputs[0].dta_sz = 40);
		_Inputs[0].dta_flags = UI_DFLAGS_SL | UI_APW;
		_Inputs[0].fld_name = UI_FLD_NAME;
		_Inputs[0].input = ip;
		grpy += 20 + 5;
	    }			// end of Name field
	    {
		NxInput *ip;	// Input widget

		ip = new NxInput(grpx, grpy, input_width, 20, "Company: ");
		ip->when(FL_WHEN_CHANGED);
		ip->callback(input_cb, (void *) this);
		ip->maximum_size(_Inputs[1].dta_sz = 40);
		_Inputs[1].fld_name = UI_FLD_COMPANY;
		_Inputs[1].dta_flags = UI_DFLAGS_SL | UI_GENERIC;
		_Inputs[1].input = ip;
		grpy += 20 + 10;
	    }			// end of Company Field
	    {
		Fl_Editor *mip;
		long ed_flags;

		mip =
		    new Fl_Editor(grpx, grpy, input_width, 20 * 3 + 1,
				  "Address: \n\n\n\n\n");
		instance->def_font(mip);
		mip->box(FL_BORDER_BOX);
		mip->align(FL_ALIGN_LEFT);
		ed_flags = mip->Engine()->GetFlags();
		ed_flags |= EF_BYTELIMITS;
		mip->Engine()->SetFlags(ed_flags);
		mip->when(FL_WHEN_CHANGED);
		mip->callback(input_cb, (void *) this);
		mip->Engine()->SetMaxChar((_Inputs[2].dta_sz = 132));
		_Inputs[2].fld_name = UI_FLD_ADDRESS;
		_Inputs[2].dta_flags = UI_DFLAGS_ML | UI_GENERIC;
		_Inputs[2].input = mip;
		grpy += (3 * 20) + 5;
	    }			// end of Address field
	    {
		NxInput *ip;

		ip = new NxInput(grpx, grpy, input_width, 20, "Phone: ");
		ip->when(FL_WHEN_CHANGED);
		ip->callback(input_cb, (void *) this);
		ip->maximum_size(_Inputs[3].dta_sz = 24);
		_Inputs[3].fld_name = UI_FLD_PHONE;
		_Inputs[3].dta_flags = UI_DFLAGS_SL | UI_GENERIC;
//                              Changed the validation of the field to allow anything, as suggested by Jeff Clausen
//                              _Inputs[3].dta_flags = UI_DFLAGS_SL | UI_DFLAGS_NUMERIC | UI_DFLAGS_WHITESP | UI_DFLAGS_ALPHA;
		_Inputs[3].input = ip;
		grpy += (20 + 5);
	    }			// end of Phone field
	    {
		NxInput *ip;

		ip = new NxInput(grpx, grpy, input_width, 20, "Email: ");
		ip->when(FL_WHEN_CHANGED);
		ip->callback(input_cb, (void *) this);
		ip->maximum_size(_Inputs[4].dta_sz = 40);
		_Inputs[4].fld_name = UI_FLD_EMAIL;
		_Inputs[4].dta_flags = UI_DFLAGS_SL | UI_GENERIC;
		_Inputs[4].input = ip;
	    }			// end of input widget definitions...
	    g1->end();
	    _userInfog = g1;
	}			// end of ID tab group
	{
	    Fl_Group *g2;	// Notes Tab Group

	    int grpx, grpy = cury + (3 * mar);

	    g2 = new Fl_Group(curx, cury, o->w(), o->h() - 20);
	    g2->box(FL_FLAT_BOX);
	    g2->label("Notes");

	    instance->def_font(g2);
	    g2->color(_mainw->color());

	    grpx = g2->x() + (int) max_lbl_wid;
	    {
		Fl_Editor *mip;	// Editor box
		long ed_flags;

		mip =
		    new Fl_Editor(grpx, grpy, input_width, 7 * 20,
				  "Notes: \n\n\n\n\n\n\n\n\n\n\n");
		mip->align(FL_ALIGN_LEFT);
		ed_flags = mip->Engine()->GetFlags();
		ed_flags |= EF_BYTELIMITS;
		mip->Engine()->SetFlags(ed_flags);
		mip->Engine()->SetMaxChar((_Inputs[5].dta_sz = 220));
		_Inputs[5].fld_name = UI_FLD_NOTES;
		_Inputs[5].dta_flags = UI_DFLAGS_ML | UI_GENERIC;
		_Inputs[5].input = mip;
	    }			// end of input widget definitions
	    g2->end();
	    _notesg = g2;
	}			// end of Notes tab group
	o->end();
	_mainTab = o;
    }				// end of Tab widget
    _mainw->end();
    _mainw->hide();

    return;
}				// end of NxUserInfo::MakeWindow(void)

void
NxUserInfo::ShowWindow(void)
{
    _mainw->show();
}

void
NxUserInfo::HideWindow(void)
{
    _mainw->hide();
}

/*******************************************************************************\
**
**	Function:	void SetAppPrefs()
**	Desc:		Stores any changed values into the PAR database
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxUserInfo::SetAppPrefs(void)
{
    char *pardb,		// Database name
      par_data[512];		// Par data
    int idx,			// Quick index variable
      rc;			// Result code
    db_handle *hdb;		// Database handle

    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No Default database! Changes lost");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RW)) == NULL) {
	printf("Unable to open %s, error=%d", pardb, pardb_errno);
	return;
    }				// end of if

    // Write out any changes
    for (idx = 0; idx < UI_FLD_MAXFLDS; idx++) {
	memset(par_data, 0, sizeof(par_data));
	if (_Inputs[idx].dta_flags & UI_DFLAGS_DIRTY) {
	    if (_Inputs[idx].dta_flags & UI_DFLAGS_SL) {
		NxInput *nxip = (NxInput *) _Inputs[idx].input;
		strcpy(par_data, nxip->value());
	    }			// end of if 
	    else {
		Fl_Editor *nxip = (Fl_Editor *) _Inputs[idx].input;
		nxip->SaveTo(par_data);
	    }			// end of else
	    rc = par_setGlobalPref(hdb, "UserId",
				   const_cast <
				   char *>(ParIds[_Inputs[idx].fld_name]),
				   PAR_TEXT, par_data, strlen(par_data));
	    _Inputs[idx].dta_flags &= ~UI_DFLAGS_DIRTY;
	}			// end of if 
    }				// end of for

    char col_msg[CL_MAX_MSG_LEN] = { '\0' };
    int col_len = sprintf(col_msg, "sc_userinfo^SYSCON_UI_CHANGE");

    sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);

    return;
}				// end of NxUserInfo::SetAppPrefs(void)

/*******************************************************************************\
**
**	Function:	void input_cp()
**	Desc:		Callback for any input in an input widget
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxUserInfo::input_cb(Fl_Widget * w, void *d)
{
    char *cp;
    int i, pos, rc, sz;
    NxUserInfo *me = (NxUserInfo *) d;
    NxInput *nxip = NULL;

    // Find the correct widget, and automatically mark him as being dirty
    for (i = 0; i < UI_FLD_MAXFLDS; i++) {
	if (w == me->_Inputs[i].input) {
	    if (me->_Inputs[i].dta_flags & UI_DFLAGS_SL) {
		nxip = (NxInput *) w;
		me->_Inputs[i].dta_flags |= UI_DFLAGS_DIRTY;
		break;
	    }			// end of else
	}			// end of if
    }				// end of for

    if (nxip == NULL)
	return;

    // Determine if a character was added...
    if ((rc = Fl::event_key()) >= FL_Button || !rc)
	return;

    sz = nxip->size() + 1;
    pos = nxip->position();
    cp = (char *) calloc(sz + 1, sizeof(char));
    if (cp == NULL)
	return;
    strcpy(cp, nxip->value());

    // This is a generic field validator, specific fields will have to 
    // be identified and handled separately...
    if (!VALIDATE_CHAR(cp[pos - 1], me->_Inputs[i].dta_flags)) {
	printf("Invalid character for this field!\n");
	memmove(&cp[pos - 1], &cp[pos], sz - pos);
	nxip->value(cp);
	nxip->position(pos - 1);
	nxip->redraw();
    }				// end of if
    free(cp);

    return;
}				// end of NxUserInfo::input_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void save_reset_cb()
**	Desc:		Callback for saving/reseting data
**	Accepts:	Fl_Widget *w = Ptr to widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxUserInfo::save_reset_cb(Fl_Widget * w, void *d)
{
    NxUserInfo *me = (NxUserInfo *) d;
    NxButton *btn = (NxButton *) w;

    if (me->_saveb == btn) {
	// Save things....
	for (int i = 0; i < UI_FLD_MAXFLDS; i++) {
	    if (me->_Inputs[i].dta_flags & UI_DFLAGS_ML) {
		Fl_Editor *nxip = (Fl_Editor *) me->_Inputs[i].input;
		if (nxip->changed() == true)
		    me->_Inputs[i].dta_flags |= UI_DFLAGS_DIRTY;
	    }			// end of if
	}			// end of for
	me->SetAppPrefs();
    }				// end of if 
    else {
	// Reset the values 
	me->GetAppPrefs();
    }

    return;
}
