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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */



#ifndef Nx_Menu_Item_H
#define Nx_Menu_Item_H

#ifndef Fl_Widget_H
// used to get the Fl_Callback typedefs:
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Widget.H>
#endif

class NxMenu_;

struct NxMenuItem
{
    const char *text;		// label()
    int shortcut_;
    Fl_Callback *callback_;
    void *user_data_;
    int flags;
    uchar labeltype_;
    uchar labelfont_;
    uchar labelsize_;
    uchar labelcolor_;

    // advance N items, skipping submenus:
    FL_EXPORT const NxMenuItem *next(int = 1) const;
    NxMenuItem *next(int i = 1)
    {
	return (NxMenuItem *) (((const NxMenuItem *) this)->next(i));
    }

    // methods on menu items:
    const char *label() const
    {
	return text;
    }
    void label(const char *a)
    {
	text = a;
    }
    void label(Fl_Labeltype a, const char *b)
    {
	labeltype_ = a;
	text = b;
    }
    Fl_Labeltype labeltype() const
    {
	return (Fl_Labeltype) labeltype_;
    }
    void labeltype(Fl_Labeltype a)
    {
	labeltype_ = a;
    }
    Fl_Color labelcolor() const
    {
	return (Fl_Color) labelcolor_;
    }
    void labelcolor(uchar a)
    {
	labelcolor_ = a;
    }
    Fl_Font labelfont() const
    {
	return (Fl_Font) labelfont_;
    }
    void labelfont(uchar a)
    {
	labelfont_ = a;
    }
    uchar labelsize() const
    {
	return labelsize_;
    }
    void labelsize(uchar a)
    {
	labelsize_ = a;
    }
    Fl_Callback_p callback() const
    {
	return callback_;
    }
    void callback(Fl_Callback * c, void *p)
    {
	callback_ = c;
	user_data_ = p;
    }
    void callback(Fl_Callback * c)
    {
	callback_ = c;
    }
    void callback(Fl_Callback0 * c)
    {
	callback_ = (Fl_Callback *) c;
    }
    void callback(Fl_Callback1 * c, long p = 0)
    {
	callback_ = (Fl_Callback *) c;
	user_data_ = (void *) p;
    }
    void *user_data() const
    {
	return user_data_;
    }
    void user_data(void *v)
    {
	user_data_ = v;
    }
    long argument() const
    {
	return (long) user_data_;
    }
    void argument(long v)
    {
	user_data_ = (void *) v;
    }
    int shortcut() const
    {
	return shortcut_;
    }
    void shortcut(int s)
    {
	shortcut_ = s;
    }
    int submenu() const
    {
	return flags & (FL_SUBMENU | FL_SUBMENU_POINTER);
    }
    int checkbox() const
    {
	return flags & FL_MENU_TOGGLE;
    }
    int radio() const
    {
	return flags & FL_MENU_RADIO;
    }
    int value() const
    {
	return flags & FL_MENU_VALUE;
    }
    void set()
    {
	flags |= FL_MENU_VALUE;
    }
    void clear()
    {
	flags &= ~FL_MENU_VALUE;
    }
    FL_EXPORT void setonly();
    int visible() const
    {
	return !(flags & FL_MENU_INVISIBLE);
    }
    void show()
    {
	flags &= ~FL_MENU_INVISIBLE;
    }
    void hide()
    {
	flags |= FL_MENU_INVISIBLE;
    }
    int active() const
    {
	return !(flags & FL_MENU_INACTIVE);
    }
    void activate()
    {
	flags &= ~FL_MENU_INACTIVE;
    }
    void deactivate()
    {
	flags |= FL_MENU_INACTIVE;
    }
    int activevisible() const
    {
	return !(flags & 0x11);
    }

    // used by menubar:
    FL_EXPORT int measure(int *h, const NxMenu_ *) const;
    FL_EXPORT void draw(int x, int y, int w, int h, const NxMenu_ *, int t =
			0) const;

    // popup menus without using an NxMenu_ widget:
    FL_EXPORT const NxMenuItem *popup(int X, int Y,
				      int scroll_size,
				      const char *title = 0,
				      const NxMenuItem * picked = 0,
				      const NxMenu_ * = 0) const;
    FL_EXPORT const NxMenuItem *pulldown(int X, int Y, int W, int H,
					 int scroll_size,
					 const NxMenuItem * picked = 0,
					 const NxMenu_ * = 0,
					 const NxMenuItem * title = 0,
					 int menubar = 0) const;
    FL_EXPORT const NxMenuItem *test_shortcut() const;
    FL_EXPORT const NxMenuItem *find_shortcut(int *ip = 0) const;

    void do_callback(Fl_Widget * o) const
    {
	callback_(o, user_data_);
    }
    void do_callback(Fl_Widget * o, void *arg) const
    {
	callback_(o, arg);
    }
    void do_callback(Fl_Widget * o, long arg) const
    {
	callback_(o, (void *) arg);
    }

    // back-compatability, do not use:
    int checked() const
    {
	return flags & FL_MENU_VALUE;
    }
    void check()
    {
	flags |= FL_MENU_VALUE;
    }
    void uncheck()
    {
	flags &= ~FL_MENU_VALUE;
    }
    FL_EXPORT int add(const char *, int shortcut, Fl_Callback *, void * =
		      0, int = 0);
    int add(const char *a, const char *b, Fl_Callback * c, void *d =
	    0, int e = 0)
    {
	return add(a, fl_old_shortcut(b), c, d, e);
    }
    FL_EXPORT int size() const;
};

typedef NxMenuItem Nx_Menu;	// back compatability

#endif
