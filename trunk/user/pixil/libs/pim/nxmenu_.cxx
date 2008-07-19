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

#include <FL/Fl.H>
#include <nxmenu_.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int
NxMenu_::value(const NxMenuItem * m)
{
    clear_changed();
    if (value_ != m) {
	value_ = m;
	return 1;
    }
    return 0;
}

// When user picks a menu item, call this.  It will do the callback.
// Unfortunatly this also casts away const for the checkboxes, but this
// was necessary so non-checkbox menus can really be declared const...
const NxMenuItem *
NxMenu_::picked(const NxMenuItem * v)
{
    if (v) {
	if (v->radio()) {
	    if (!v->value()) {	// they are turning on a radio item
		set_changed();
		((NxMenuItem *) v)->setonly();
	    }
	    redraw();
	} else if (v->flags & FL_MENU_TOGGLE) {
	    set_changed();
	    ((NxMenuItem *) v)->flags ^= FL_MENU_VALUE;
	    redraw();
	} else if (v != value_) {	// normal item
	    set_changed();
	}
	value_ = v;
	if (when() & (FL_WHEN_CHANGED | FL_WHEN_RELEASE)) {
	    if (changed() || when() & FL_WHEN_NOT_CHANGED) {
		clear_changed();
		if (value_ && value_->callback_)
		    value_->do_callback((Fl_Widget *) this);
		else
		    do_callback();
	    }
	}
    }
    return v;
}

// turn on one of a set of radio buttons
void
NxMenuItem::setonly()
{
    flags |= FL_MENU_RADIO | FL_MENU_VALUE;
    NxMenuItem *j;
    for (j = this;;) {		// go down
	if (j->flags & FL_MENU_DIVIDER)
	    break;		// stop on divider lines
	j++;
	if (!j->text || !j->radio())
	    break;		// stop after group
	j->clear();
    }
    for (j = this - 1;; j--) {	// go up
	if (!j->text || (j->flags & FL_MENU_DIVIDER) || !j->radio())
	    break;
	j->clear();
    }
}

NxMenu_::NxMenu_(int X, int Y, int W, int H, const char *l)
    :
Fl_Widget(X, Y, W, H, l)
{
    set_flag(SHORTCUT_LABEL);
    box(FL_UP_BOX);
    when(FL_WHEN_RELEASE_ALWAYS);
    value_ = menu_ = 0;
    alloc = 0;
    selection_color(FL_SELECTION_COLOR);
    textfont(FL_HELVETICA);
    textsize(FL_NORMAL_SIZE);
    textcolor(FL_BLACK);
    down_box(FL_NO_BOX);
}

int
NxMenu_::size() const
{
    if (!menu_)
	return 0;
    return menu_->size();
}

void
NxMenu_::menu(const NxMenuItem * m)
{
    clear();
    value_ = menu_ = (NxMenuItem *) m;
}

void
NxMenu_::menu(const Fl_Menu_Item * m)
{
    clear();
    value_ = menu_ = (NxMenuItem *) m;
}

#if 1
// this version is ok with new NxMenu_add code with fl_menu_array_owner:

void
NxMenu_::copy(const NxMenuItem * m, void *user_data)
{
    int n = m->size();
    NxMenuItem *newMenu = new NxMenuItem[n];
    memcpy(newMenu, m, n * sizeof(NxMenuItem));
    menu(newMenu);
    alloc = 1;			// make destructor free array, but not strings
    // for convienence, provide way to change all the user data pointers:
    if (user_data)
	for (; n--;) {
	    if (newMenu->callback_)
		newMenu->user_data_ = user_data;
	    newMenu++;
	}
}

#else
// This is Guillaume Nodet's fixed version for the older NxMenu_add
// that enlarged the array at powers of 2:

void
NxMenu_::copy(const NxMenuItem * m, void *user_data)
{
    int i, s = m->size(), n = s;
    for (i = 0; n; n >>= 1, i++);
    n = 1 << i;
    NxMenuItem *newMenu = new NxMenuItem[n];
    memcpy(newMenu, m, s * sizeof(NxMenuItem));
    memset(newMenu + s, 0, (n - s) * sizeof(NxMenuItem));
    menu(newMenu);
    alloc = 1;			// make destructor free it
    // for convienence, provide way to change all the user data pointers:
    if (user_data)
	for (; s--;) {
	    if (newMenu->callback_)
		newMenu->user_data_ = user_data;
	    newMenu++;
	}
}
#endif

NxMenu_::~NxMenu_()
{
    clear();
}

// Fl_Menu::add() uses this to indicate the owner of the dynamically-
// expanding array.  We must not free this array:
NxMenu_ *nx_menu_array_owner = 0;

void
NxMenu_::clear()
{
    if (alloc) {
	if (alloc > 1)
	    for (int i = size(); i--;)
		if (menu_[i].text)
		    free((void *) menu_[i].text);
	if (this == nx_menu_array_owner)
	    nx_menu_array_owner = 0;
	else
	    delete[]menu_;
	menu_ = 0;
	value_ = 0;
	alloc = 0;
    }
}
