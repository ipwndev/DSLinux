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


/* System header files */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Local header files */
#include "nxtminput.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>

//-------------------------------------------------------------------------------
//
//      Class NxTmUnit definition (implementation)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	int NxTmUnit::handle()
**	Desc:		Event handler for this class (Overrides the Fl_Input::handle()
**				function.
**	Accepts:	int event = Event to handle
**	Returns:	int; non-zero if event was handled, 0 otherwise.
**
\*******************************************************************************/
int
NxTmUnit::handle(int event)
{
    int c,			// The current character
      mod = 0,			// If the widget was modified
      unit_idx;			// Unit index

    switch (event) {
    case FL_KEYBOARD:
	switch (c = Fl::event_key()) {
	case FL_BackSpace:
	    // handle the backspace
	    for (unit_idx = 0; unit_idx < parent->GetUnits(); unit_idx++) {
		if (this == parent->Units[unit_idx])
		    break;
	    }			// end of for
	    if (unit_idx)
		unit_idx--;
	    if (position() == 0) {
		parent->Units[unit_idx]->take_focus();
		int len = strlen(parent->Units[unit_idx]->value());
		parent->Units[unit_idx]->position(len);
		mod = 1;
	    }			// end of if
	    else
		return (Fl_Input::handle(event));
	    break;
	default:
	    // Get the correct widget
	    for (unit_idx = 0; unit_idx < parent->GetUnits(); unit_idx++)
		if (this == parent->Units[unit_idx])
		    break;

	    if (c >= 0x30 && c <= 0x39) {
		char tmpbuf[3] = { '\0' };
		char buf[3] = { '\0' };

		sprintf(tmpbuf, "%d", max_val[unit_idx]);
		strcpy(buf, value());
		// Determine if we are in selection mode, and if so, overwrite the buffer
//                                              if (mark() != position())
//                                              {
//                                                      // Remove the selection
//                                                      position(mark());
//                                                      buf[0] = buf[1] = '0';  
//                                              } // end of if
//                                              if (position() == 0)
//                                              {
//                                                      // New text.....
//                                                      buf[1] = (char) c;
//                                              } // end of if 
//                                              else if (position() == 2)
//                                              {
		if (buf[1])
		    buf[0] = buf[1];
		else
		    buf[0] = '0';
		buf[1] = (char) c;

		// This can never be a valid 10's digit
		if ((buf[0] > tmpbuf[0]) || atoi(buf) > atoi(tmpbuf))
		    buf[0] = '0';
//                                              } // end of else if 
		value(buf);
		if (when() & FL_WHEN_CHANGED)
		    do_callback();
		else
		    set_changed();
	    }			// end of if
	    else if (c >= FL_Button) {
		return (Fl_Input::handle(event));
	    }			// end of else-if
	    else {
		// Ignore the non-numeric key...
		return (0);
	    }			// end of else
	    break;
	}			// end of switch
	break;
    default:
	// Let the Fl_Input::handle() handler have a go...
	return (Fl_Input::handle(event));
    }				// end of switch

    // Return whether we have taken the character or not....
    return (1);
}				// end of NxTmUnit::handle(int)

//-------------------------------------------------------------------------------
//
//      Class NxTmInput definition (implementation)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void NxTmInput::draw()
**	Desc:		Handles the drawing for the over all input box
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxTmInput::draw(void)
{
    int iwid, owid, x1 = x(), y1 = y(), w1 = w(), h1 = h(), xpos = 0;

    owid = (w1 / MAX_UNITS) / 3;
    iwid = owid * 2;

    if (damage() & (FL_DAMAGE_EXPOSE | FL_DAMAGE_ALL)) {
	Fl_Color c;

	c = NxApp::Instance()->getGlobalColor(APP_BG);
	draw_box(FL_FLAT_BOX, x(), y(), w(), h(), c);
	align(FL_ALIGN_RIGHT);
	for (int idx = 0; idx < _nunits; idx++) {
//                      Units[idx]->resize(x1+xpos+2, y1+2, w1/3-4, h1-4);
	    Units[idx]->box(FL_FLAT_BOX);
	    Units[idx]->align(FL_ALIGN_CENTER);
	    Units[idx]->draw();
	    xpos += w1 / 3;
	}			// end of for
	xpos = x1;


	// Draw the colon
	xpos = x1;
	for (int jdx = 0; jdx < _nunits; jdx++) {
	    if (jdx < 2) {
		xpos = Units[jdx]->x() + Units[jdx]->w();
#ifdef NANOX
		draw_box(FL_BLACK_BOX, xpos, y1 + h1 - 7, 4, 4, FL_BLACK);
		draw_box(FL_BLACK_BOX, xpos, y1 + h1 - 13, 4, 4, FL_BLACK);
#else
		draw_box(FL_FLAT_BOX, xpos, y1 + h1 - 7, 4, 4, FL_BLACK);
		draw_box(FL_FLAT_BOX, xpos, y1 + h1 - 13, 4, 4, FL_BLACK);
#endif
		xpos += owid;
	    }			// end of if
	}			// end of for 
    }				//end of if

    return;
}				// end of NxTmInput::draw(void)

/*******************************************************************************\
**
**	Function:	char *GetTime(void)
**	Desc:		Gets the time as entered in this widget in the format of
**				HH:MM:SS:[AP]M (assuming that all four input widgets are in use)
**	Accepts:	Nothing (void)
**	Returns:	char * (what is contained in _tmstring)
**
\*******************************************************************************/
char *
NxTmInput::GetTime(void)
{
    if (_nunits == 3) {
	// Only hour/minute/second
	sprintf(_tmstring, "%2.2s:%2.2s:%2.2s", Units[0]->value(),
		Units[1]->value(), Units[2]->value());
    }				// end of if
    else {
	// Hour/Minute/second/am or pm
	sprintf(_tmstring, "%2.2s:%2.2s:%2.2s %2.2s", Units[0]->value(),
		Units[1]->value(), Units[2]->value(), Units[3]->value());
    }				// end of else

    return (_tmstring);
}				// end of NxTmInput::GetTime(void)

/*******************************************************************************\
**
**	Function:	void GetTime(struct tm *)
**	Desc:		Gets the time as stored in the widget set, and properly converts it
**				into a struct tm.
**	Accepts:	struct tm *tptr = Ptr to the tm struct
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::GetTime(struct tm *tptr)
{
    int idx = 0;		// Index

    if (tptr == NULL)
	return;

    // Get the time
    tptr->tm_hour = atoi(Units[idx++]->value());
    tptr->tm_min = atoi(Units[idx++]->value());
    tptr->tm_sec = atoi(Units[idx++]->value());

    if (_nunits == 4) {
	if (!memcmp(Units[idx]->value(), "PM", 2)) {
	    if (tptr->tm_hour < 12)
		tptr->tm_hour += 12;
	}			// end of if
    }				// end of if

    return;
}				// end of NxTmInput::GetTime(struct tm *)

/*******************************************************************************\
**
**	Function:	void SetUnits()
**	Desc:		Sets the value of _nunits to what is desired (if units are
**				3 then a 24 hour clock is assumed
**	Accepts:	int num = number of units to use
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::SetUnits(int num)
{
    // Validate the incoming argument
    if (num <= MAX_UNITS && num > 0)
	_nunits = num;
    else
	_nunits = 4;

    // Activate the used ones and deactivate the unused....
    for (int i = 0; i < MAX_UNITS; i++) {
	if (i < _nunits)
	    Units[i]->activate();
	else
	    Units[i]->deactivate();
    }				// end of for


    return;
}				// end of NxTmInput::SetUnits(int)

/*******************************************************************************\
**
**	Function:	void SetTime(char *)
**	Desc:		Sets the time according to a time string (in format HH:MM:SS AM/PM)
**	Accepts:	char *tmstr = Time string
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::SetTime(char *tmstr)
{
    char *cp,			// Character pointer
     *cpdup;
    int nelem;			// Number of time elements

    if (tmstr == NULL || *tmstr == 0)
	return;

    cpdup = strdup(tmstr);

    // Parse the string into a vector-like entity
    for (nelem = 1, cp = cpdup; *cp; cp++) {
	if (*cp == ':' || *cp == ' ') {
	    *cp = '\0';
	    nelem++;
	}			// end of if 
    }				// end of for

    cp = cpdup;
    for (int i = 0; i < nelem; cp += strlen(cp) + 1, i++)
	Units[i]->value(cp);

    free(cpdup);
    // Redraw this widget
    redraw();
    return;
}				// end of NxTmInput::SetTime(char *)

/*******************************************************************************\
**
**	Function:	void SetTime(struct tm *)
**	Desc:		Sets the time (in the widget) according to the tm struct values
**	Accepts:	struct tm *tptr = Ptr to a time value
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::SetTime(struct tm *tptr)
{
    char tmbuf[2 + 1] = { '\0' };	// Time buffer
    int hour;			// Potentially modified hour value

    if (tptr == NULL)
	return;

    // Convert the hour
    hour = tptr->tm_hour;
    if (_nunits == 4) {
	// There is an am/pm designation
	if (hour > 12)
	    hour -= 12;
	if (hour == 0)
	    hour = 12;
    }				// end of if
    sprintf(tmbuf, "%2.2d", hour);
    Units[0]->value(tmbuf);
    sprintf(tmbuf, "%2.2d", tptr->tm_min);
    Units[1]->value(tmbuf);
    sprintf(tmbuf, "%2.2d", tptr->tm_sec);
    Units[2]->value(tmbuf);

#if 0
    if (_nunits == 4) {
	Units[3]->value(dayzn[tptr->tm_hour > 12]);
    }				// end of if
#endif

    return;
}				// end of NxTmInput::SetTime(struct tm *)

/*******************************************************************************\
**
**	Function:	void hide()
**	Desc:		Hides this widget (and all children widgets)
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::hide(void)
{
    for (int idx = 0; idx < _nunits; idx++) {
	Units[idx]->hide();
	Units[idx]->redraw();
    }				// end of for

    return;
}				// end of NxTmInput::hide(void)

/*******************************************************************************\
**
**	Function:	void show()
**	Desc:		Shows this widget (and all children widgets)
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxTmInput::show(void)
{
    for (int idx = 0; idx < _nunits; idx++) {
	Units[idx]->show();
	Units[idx]->redraw();
    }				// end of for

    return;
}				// end of NxTmInput::show(void)

/*******************************************************************************\
**
**	Function:	NxTmInput()
**	Desc:		Default constructor for the NxTmInput() class
**	Accepts:	int x = X component (passed along)
**				int y = Y component (passed along)
**				int w = W component (passed along)
**				int h = H component (passed along)
**				int l = Label (passed along)
**	Returns:	N/A
**
\*******************************************************************************/
NxTmInput::NxTmInput(int x, int y, int w, int h, char *l, int nfld):
Fl_Widget(x, y, w, h, l)
{
    int iwidth, owidth;

    _nunits = nfld;

    owidth = (int) fl_width(":");
    iwidth = (int) fl_width("444") + 5;

    for (int idx = 0; idx < MAX_UNITS; idx++) {
	Units[idx] =
	    new NxTmUnit(x + (idx * (owidth + iwidth)), y, iwidth, h, "");
	Units[idx]->NxTmUnit::SetParent(this);
	Units[idx]->align(FL_ALIGN_RIGHT);
	Units[idx]->maximum_size(2);
	if (idx >= _nunits) {
	    Units[idx]->deactivate();
	    Units[idx]->hide();
	}			// end of if

    }				// end of for

    memset(_tmstring, 0, sizeof(_tmstring));
}				// end of NxTmInput::NxTmInput(int, int, int, int, char *)
