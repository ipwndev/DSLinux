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


#ifndef Nx_Menu__H
#define Nx_Menu__H

#ifndef Fl_Widget_H
#include <FL/Fl_Widget.H>
#endif
#include <FL/Fl_Menu_Item.H>
#include "nxmenuitem.h"

class NxMenu_:public Fl_Widget
{

    NxMenuItem *menu_;
    const NxMenuItem *value_;

  protected:

      uchar down_box_;
    uchar textfont_;
    uchar textsize_;
    uchar textcolor_;
    uchar alloc;

  public:
      FL_EXPORT const NxMenuItem *picked(const NxMenuItem *);

    FL_EXPORT NxMenu_(int, int, int, int, const char * = 0);
      FL_EXPORT ~ NxMenu_();

    const NxMenuItem *test_shortcut()
    {
	return picked(menu()->test_shortcut());
    }
    FL_EXPORT void global ();

    const NxMenuItem *menu() const
    {
	return menu_;
    }
    FL_EXPORT void menu(const NxMenuItem * m);
    FL_EXPORT void menu(const Fl_Menu_Item * m);
    FL_EXPORT void copy(const NxMenuItem * m, void *user_data = 0);
    FL_EXPORT int add(const char *, int shortcut, Fl_Callback *, void * =
		      0, int = 0);
    int add(const char *a, const char *b, Fl_Callback * c, void *d =
	    0, int e = 0)
    {
	return add(a, fl_old_shortcut(b), c, d, e);
    }
    FL_EXPORT int size() const;
    FL_EXPORT void clear();
    FL_EXPORT int add(const char *);
    FL_EXPORT void replace(int, const char *);
    FL_EXPORT void remove(int);
    void shortcut(int i, int s)
    {
	menu_[i].shortcut(s);
    }
    void mode(int i, int x)
    {
	menu_[i].flags = x;
    }
    int mode(int i) const
    {
	return menu_[i].flags;
    }

    const NxMenuItem *mvalue() const
    {
	return value_;
    }
    int value() const
    {
	return value_ - menu_;
    }
    FL_EXPORT int value(const NxMenuItem *);
    int value(int i)
    {
	return value(menu_ + i);
    }
    const char *text() const
    {
	return value_ ? value_->text : 0;
    }
    const char *text(int i) const
    {
	return menu_[i].text;
    }

    Fl_Font textfont() const
    {
	return (Fl_Font) textfont_;
    }
    void textfont(uchar c)
    {
	textfont_ = c;
    }
    uchar textsize() const
    {
	return textsize_;
    }
    void textsize(uchar c)
    {
	textsize_ = c;
    }
    Fl_Color textcolor() const
    {
	return (Fl_Color) textcolor_;
    }
    void textcolor(uchar c)
    {
	textcolor_ = c;
    }

    Fl_Boxtype down_box() const
    {
	return (Fl_Boxtype) down_box_;
    }
    void down_box(Fl_Boxtype b)
    {
	down_box_ = b;
    }

    // back compatability:
    Fl_Color down_color() const
    {
	return selection_color();
    }
    void down_color(uchar c)
    {
	selection_color(c);
    }
};

#endif
