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


#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include <nxapp.h>

extern "C"
{
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
}

#include "ip_input.h"
///////////////////////////////////////////////////////////
//
//      Function:               Single_IP_Input::set_paretn()
//      Description:    This function sets a Single_IP_INput parent 
//      Parametets:     IP_Input *par - pointer to an IP_Input object
//      Returns:                none    
//
//////////////////////////////////////////////////////////
void
Single_IP_Input::set_parent(IP_Input * par)
{
    parent = par;
}

///////////////////////////////////////////////////////////
//
//      Function:               Single_IP_Input::handle()
//      Description:    This function handles the input into a Single_IP_Input object
//                                                      and limits the input to numbers between 0 and 255 
//      Parametets:     int event - interger numver of the event being passed in
//      Returns:                int 
//
//////////////////////////////////////////////////////////
int
Single_IP_Input::handle(int event)
{
    int idx;

    switch (event) {
    case FL_KEYBOARD:
	switch (Fl::event_key()) {
	case FL_BackSpace:
	    for (idx = 0; idx < NUM_INPUTS; idx++) {
		if (this == parent->input[idx]) {
		    break;
		}
	    }
	    if (idx) {
		idx--;
	    }
	    if (0 == position()) {
		parent->input[idx]->take_focus();
		int len = strlen(parent->input[idx]->value());
		parent->input[idx]->position(len);
	    }
	default:
	    int c = Fl::event_key();
	    if ((c > 47 && c < 58) || c > FL_Button) {
		if ((2 == strlen(value())) && (c > 47 && c < 58)) {
		    char strbuf[NUM_INPUTS];
		    char tmpbuf[NUM_INPUTS];
		    int pos = position();

		    sprintf(tmpbuf, "%c", c);
		    sprintf(strbuf, "%s", value());
		    if (0 == pos) {
			strbuf[2] = strbuf[1];
			strbuf[1] = strbuf[0];
			strbuf[0] = tmpbuf[0];
		    }
		    if (1 == pos) {
			strbuf[2] = strbuf[1];
			strbuf[1] = tmpbuf[0];
		    }
		    if (2 == pos) {
			strbuf[2] = tmpbuf[0];
		    }
		    strbuf[3] = '\0';
		    int move_pos = 1;
		    //FIXME
		    // for some dumb reason strtol seg faults
		    // in fact a lot seg faults so if you can fix it
		    // please do this is why this is kludged togethor
		    if (strbuf[0] > '2') {
			strbuf[0] = '2';
			strbuf[1] = '5';
			strbuf[2] = '5';
			move_pos = 0;
		    }
		    if (strbuf[0] == '2') {
			if (strbuf[1] > '5') {
			    strbuf[1] = '5';
			    strbuf[2] = '5';
			    move_pos = 0;
			}
		    }
		    if (strbuf[0] == '2' && strbuf[1] == '5') {
			if (strbuf[2] > '5') {
			    strbuf[2] = '5';
			    move_pos = 0;
			}
		    }
		    value(strbuf);
		    return 0;
		}
		if ((3 == strlen(value())) && c < FL_Button) {
		    for (idx = 0; idx < NUM_INPUTS; idx++) {
			if (this == parent->input[idx]) {
			    break;
			}
		    }
		    if (idx != NUM_INPUTS - 1) {
			idx++;
		    }
		    parent->input[idx]->take_focus();
		    return 0;
		}
		return Fl_Input::handle(event);
	    } else {
		return 0;
	    }
	}
    default:
	return Fl_Input::handle(event);
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::draw()
//      Description:    This function draws an IP_Input object
//      Parametets:     none    
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::draw()
{
    int x1 = x();
    int y1 = y();
    int w1 = w();
    int h1 = h();

    int xpos = 0;

    if (damage() & (FL_DAMAGE_EXPOSE | FL_DAMAGE_ALL)) {
	draw_box(FL_BOTTOM_BOX, x(), y(), w(), h(), FL_WHITE);
	align(FL_ALIGN_LEFT);
	for (int idx = 0; idx < NUM_INPUTS; idx++) {
	    input[idx]->resize(x1 + xpos + 2, y1 + 2, w1 / 4 - 5, h1 - 5);
	    input[idx]->box(FL_FLAT_BOX);
	    input[idx]->draw();
	    xpos += w1 / 4;
	}
	xpos = x1;
	for (int jdx = 0; jdx < NUM_INPUTS; jdx++) {
	    if (jdx != 3) {
		xpos += w1 / 4;
		draw_box(FL_BLACK_BOX, xpos - 4, y1 + h1 - 7, 4, 4, FL_BLACK);
	    }
	}
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::iptol()
//      Description:    This function converts the value of an IP_Input 
//                                                      object from dot notation into an unsigned long
//      Parametets:     none    
//      Returns:                unsigned long
//
//////////////////////////////////////////////////////////
unsigned long
IP_Input::iptol()
{
    unsigned long val = 0L;
    unsigned long tmp = 0L;

    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	tmp = strtol(input[idx]->value(), NULL, 10);
	if (0 == idx) {
	    tmp = (tmp & 0xFF) << 24;
	    val = val | tmp;
	} else if (1 == idx) {
	    tmp = (tmp & 0xFF) << 16;
	    val = val | tmp;
	} else if (2 == idx) {
	    tmp = (tmp & 0xFF) << 8;
	    val = val | tmp;
	} else if (3 == idx) {
	    tmp = (tmp & 0xFF);
	    val = val | tmp;
	} else {
	    fprintf(stderr, "Error! out of range\n");
	    val = 0L;
	    break;
	}
    }
    return val;
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::get_ip_inputs()
//      Description:    This function returns the value of the IP_Input
//                                                      object
//      Parametets:     bool blanks - Determines if it returns 0.0.0.0 or ""    
//      Returns:                char * - value of IP_Input object
//
//////////////////////////////////////////////////////////
char *
IP_Input::get_ip_inputs(bool blanks)
{
    ip_addr[0] = 0;

    if (false == blanks) {
	for (int idx = 0; idx < NUM_INPUTS; idx++) {
	    if (0 == strcmp(input[idx]->value(), "")) {
		input[idx]->value("");
	    }
	}
	sprintf(ip_addr, "%s.%s.%s.%s", input[0]->value(), input[1]->value(),
		input[2]->value(), input[3]->value());
    } else {
	bool first = false;
	for (int idx = 0; idx < NUM_INPUTS; idx++) {
	    if (0 != strcmp(input[idx]->value(), "")) {
		if (first == false) {
		    first = true;
		    sprintf(ip_addr, "%s", input[idx]->value());
		} else {
		    sprintf(ip_addr, "%s.%s", ip_addr, input[idx]->value());
		}
	    }
	}
    }
    return ip_addr;
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::get_ip_inputs()
//      Description:    This function returns the value of the IP_Input
//                                                      object
//      Parametets:     none    
//      Returns:                char * - value of IP_Input object
//
//////////////////////////////////////////////////////////
char *
IP_Input::get_ip_inputs()
{
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	if (0 == strcmp(input[idx]->value(), "")) {
	    input[idx]->value("0");
	}
    }
    sprintf(ip_addr, "%s.%s.%s.%s", input[0]->value(), input[1]->value(),
	    input[2]->value(), input[3]->value());
    return ip_addr;
}


///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::set_ip_inputs()
//      Description:    This function sets the value of the IP_Input
//                                                      object
//      Parametets:     chat *ip_addr - set object to this value
//                                              bool blanks - determine if blanks meaning "" are permitted
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::set_ip_inputs(char *ip_addr, bool blanks)
{
    int len = 0;
    char buf[NUM_INPUTS];
    char *strptr;
    char *tmpptr;

    if (0 == strcmp("", ip_addr) || (NULL == ip_addr)) {
	for (int idx = 0; idx < NUM_INPUTS; idx++) {
	    input[idx]->value("");
	}
	return;
    }
    if (false == blanks) {
	if (4 > strlen(ip_addr)) {
	    for (int idx = 0; idx < NUM_INPUTS; idx++) {
		input[idx]->value("0");
	    }
	} else {
	    tmpptr = ip_addr;
	    for (int idx = 0; idx < 3; idx++) {
		strptr = strstr(ip_addr, ".");
		if (NULL != strptr) {
		    len = strptr - ip_addr;
		    strncpy(buf, ip_addr, len);
		    buf[len] = '\0';
		    input[idx]->value(buf);
		    ip_addr += (len + 1);
		}
	    }
	    len = strlen(ip_addr);
	    strncpy(buf, ip_addr, len);
	    buf[len] = '\0';
	    input[3]->value(buf);
	}
    } else {
	int per_count = 0;
	for (int idx = 0; idx < NUM_INPUTS; idx++) {
	    input[idx]->value("");
	}
	for (unsigned int idx = 0; idx <= strlen(ip_addr); idx++) {
	    if ('.' == ip_addr[idx]) {
		per_count++;
	    }
	}
	for (int idx = 0; idx < per_count; idx++) {
	    strptr = strstr(ip_addr, ".");
	    if (NULL != strptr) {
		len = strptr - ip_addr;
		strncpy(buf, ip_addr, len);
		buf[len] = '\0';
		input[idx]->value(buf);
		ip_addr += (len + 1);
	    }
	}
	len = strlen(ip_addr);
	strncpy(buf, ip_addr, len);
	buf[len] = '\0';
	input[per_count]->value(buf);
	if (per_count < 3) {
	    for (int idx = per_count + 1; idx < 3; idx++) {
		input[idx]->value("");
	    }
	}
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::activate()
//      Description:    This function allows the inputs area of the object to
//                                                      be changed
//      Parametets:     none
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::activate()
{
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	input[idx]->activate();
    }
    labelcolor(FL_BLACK);
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::deactivate()
//      Description:    This function disallows the inputs area of the object to
//                                                      be changed
//      Parametets:     none
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::deactivate()
{
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	input[idx]->deactivate();
    }
    labelcolor(inactive(labelcolor()));
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::hide()
//      Description:    This function hides the input boxes
//      Parametets:     none
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::hide()
{
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	input[idx]->hide();
	input[idx]->redraw();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::show()
//      Description:    This function shows the input boxes
//      Parametets:     none
//      Returns:                none
//
//////////////////////////////////////////////////////////
void
IP_Input::show()
{
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	input[idx]->show();
	input[idx]->redraw();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               IP_Input::IP_Input()
//      Description:    This function is the constructor of the IP_Input object
//      Parametets:     int X - x pos
//                                              int Y - y pos
//                                              int W - widdth
//                                              int H - height
//                                              const char *L - label
//                                              Fl_Widget(X<Y<W<H<L) -  class inheretance       
//      Returns:                none
//
//////////////////////////////////////////////////////////
IP_Input::IP_Input(int X, int Y, int W, int H, const char *L)
    :
Fl_Widget(X, Y, W, H, L)
{

    box(FL_BOTTOM_BOX);
    for (int idx = 0; idx < NUM_INPUTS; idx++) {
	input[idx] = new Single_IP_Input(0, 0, 0, 0, "");
	input[idx]->Single_IP_Input::set_parent(this);
	input[idx]->maximum_size(3);
	input[idx]->value("0");
	input[idx]->labelfont(DEFAULT_LABEL_FONT);
	input[idx]->labelsize(DEFAULT_LABEL_SIZE);

	input[idx]->textfont(DEFAULT_LABEL_FONT);
	input[idx]->textsize(DEFAULT_LABEL_SIZE);
    }

    /* Set the default fonts */

    labelfont(DEFAULT_LABEL_FONT);
    labelsize(DEFAULT_LABEL_SIZE);
}
