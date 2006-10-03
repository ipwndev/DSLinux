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



// Methods to alter the menu in an NxMenu_ widget.

// These are for Forms emulation and for dynamically changing the
// menus.  They are in this source file so they are not linked in if
// not used, which is what will happen if the the program only uses
// constant menu tables.

// Not at all guaranteed to be Forms compatable, especially with any
// string with a % sign in it!

#include <nxmenu_.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// If the array is this, we will double-reallocate as necessary:
static NxMenuItem *local_array = 0;
static int local_array_alloc = 0;	// number allocated
static int local_array_size = 0;	// == size(local_array)
extern NxMenu_ *nx_menu_array_owner;	// in NxMenu_.cxx

// For historical reasons there are matching methods that work on a
// user-allocated array of NxMenuItem.  These methods are quite
// depreciated and should not be used.  These old methods use the
// above pointers to detect if the array belongs to an NxMenu_
// widget, and if so it reallocates as necessary.

// Insert a single NxMenuItem into an array of size at offset n,
// if this is local_array it will be reallocated if needed.
static NxMenuItem *
insert(NxMenuItem * array, int size, int n, const char *text, int flags)
{
    if (array == local_array && size >= local_array_alloc) {
	local_array_alloc = 2 * size;
	NxMenuItem *newarray = new NxMenuItem[local_array_alloc];
	memmove(newarray, array, size * sizeof(NxMenuItem));
	delete[]local_array;
	local_array = array = newarray;
    }
    // move all the later items:
    memmove(array + n + 1, array + n, sizeof(NxMenuItem) * (size - n));
    // create the new item:
    NxMenuItem *m = array + n;
    m->text = text ? strdup(text) : 0;
    m->shortcut_ = 0;
    m->callback_ = 0;
    m->user_data_ = 0;
    m->flags = flags;
    m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;
    return array;
}

// Add an item.  The text is split at '|' characters to automatically
// produce submenus (actually a totally unnecessary feature as you can
// now add submenu titles directly by setting SUBMENU in the flags):
int
NxMenuItem::add(const char *text,
		int shortcut, Fl_Callback * cb, void *data, int flags)
{
    NxMenuItem *array = this;
    NxMenuItem *m = this;
    const char *p;
    char *q;
    char buf[1024];

    int size = array == local_array ? local_array_size : array->size();
    int flags1 = 0;
    char *item;
    for (;;) {			/* do all the supermenus: */

	/* fill in the buf with name, changing \x to x: */
	q = buf;
	for (p = text; *p && *p != '/'; *q++ = *p++)
	    if (*p == '\\')
		p++;
	*q = 0;

	item = buf;
	if (*item == '_') {
	    item++;
	    flags1 = FL_MENU_DIVIDER;
	}
	if (*p != '/')
	    break;		/* not a menu title */
	text = p + 1;		/* point at item title */

	/* find a matching menu title: */
	for (; m->text; m = m->next())
	    if (m->flags & FL_SUBMENU && !strcmp(item, m->text))
		break;

	if (!m->text) {		/* create a new menu */
	    int n = m - array;
	    array = insert(array, size, n, item, FL_SUBMENU | flags1);
	    size++;
	    array = insert(array, size, n + 1, 0, 0);
	    size++;
	    m = array + n;
	}
	m++;			/* go into the submenu */
	flags1 = 0;
    }

    /* find a matching menu item: */
    for (; m->text; m = m->next())
	if (!strcmp(m->text, item))
	    break;

    if (!m->text) {		/* add a new menu item */
	int n = m - array;
	array = insert(array, size, n, item, flags | flags1);
	size++;
	if (flags & FL_SUBMENU) {	// add submenu delimiter
	    array = insert(array, size, n + 1, 0, 0);
	    size++;
	}
	m = array + n;
    }

    /* fill it in */
    m->shortcut_ = shortcut;
    m->callback_ = cb;
    m->user_data_ = data;
    m->flags = flags | flags1;

    if (array == local_array)
	local_array_size = size;
    return m - array;
}

int
NxMenu_::add(const char *t, int s, Fl_Callback * c, void *v, int f)
{
    // make this widget own the local array:
    if (this != nx_menu_array_owner) {
	if (nx_menu_array_owner) {
	    NxMenu_ *o = nx_menu_array_owner;
	    // the previous owner get's its own correctly-sized array:
	    int value_offset = o->value_ - local_array;
	    int n = local_array_size;
	    NxMenuItem *newMenu = o->menu_ = new NxMenuItem[n];
	    memcpy(newMenu, local_array, n * sizeof(NxMenuItem));
	    if (o->value_)
		o->value_ = newMenu + value_offset;
	}
	if (menu_) {
	    // this already has a menu array, use it as the local one:
	    delete[]local_array;
	    if (!alloc)
		copy(menu_);	// duplicate a user-provided static array
	    // add to the menu's current array:
	    local_array_alloc = local_array_size = size();
	    local_array = menu_;
	} else {
	    // start with a blank array:
	    alloc = 2;		// indicates that the strings can be freed
	    if (local_array) {
		menu_ = local_array;
	    } else {
		local_array_alloc = 15;
		local_array = menu_ = new NxMenuItem[local_array_alloc];
	    }
	    menu_[0].text = 0;
	    local_array_size = 1;
	}
	nx_menu_array_owner = this;
    }

    int r = menu_->add(t, s, c, v, f);
    // if it rellocated array we must fix the pointer:
    int value_offset = value_ - menu_;
    menu_ = local_array;	// in case it reallocated it
    if (value_)
	value_ = menu_ + value_offset;
    return r;
}

// This is a Forms (and SGI GL library) compatable add function, it
// adds many menu items, with '|' seperating the menu items, and tab
// seperating the menu item names from an optional shortcut string.
int
NxMenu_::add(const char *str)
{
    char buf[128];
    int r = 0;
    while (*str) {
	int shortcut = 0;
	char *c;
	for (c = buf; *str && *str != '|'; str++) {
	    if (*str == '\t') {
		*c++ = 0;
		shortcut = fl_old_shortcut(str);
	    } else
		*c++ = *str;
	}
	*c = 0;
	r = add(buf, shortcut, 0, 0, 0);
	if (*str)
	    str++;
    }
    return r;
}

void
NxMenu_::replace(int i, const char *str)
{
    if (i < 0 || i >= size())
	return;
    if (!alloc)
	copy(menu_);
    if (alloc > 1) {
	free((void *) menu_[i].text);
	str = strdup(str);
    }
    menu_[i].text = str;
}

void
NxMenu_::remove(int i)
{
    int n = size();
    if (i < 0 || i >= n)
	return;
    if (!alloc)
	copy(menu_);
    if (alloc > 1)
	free((void *) menu_[i].text);
    memmove(&menu_[i], &menu_[i + 1], (n - i) * sizeof(NxMenuItem));
}
