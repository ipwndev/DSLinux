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

// Warning: this menu code is quite a mess!

// This file contains code for implementing NxMenuItem, and for
// methods for bringing up popup menu hierarchies without using the
// NxMenu_ widget.

#include <FL/Fl.H>
#include <nxmenuwindow.h>
#include <nxmenu_.h>
#include <stdio.h>
#include <FL/fl_draw.H>

#include <nxapp.h>

#define SCROLL_UP 0x7FFF	//set to really big number

int
NxMenuItem::size() const
{
    const NxMenuItem *m = this;
    int nest = 0;
    for (;;) {
	if (!m->text) {
	    if (!nest)
		return (m - this + 1);
	    nest--;
	} else if (m->flags & FL_SUBMENU) {
	    nest++;
	}
	m++;
    }
}

const NxMenuItem *
NxMenuItem::next(int n) const
{
    if (n < 0)
	return 0;		// this is so selected==-1 returns NULL
    const NxMenuItem *m = this;
    int nest = 0;
    while (n > 0) {
	if (!m->text) {
	    if (!nest)
		return m;
	    nest--;
	} else if (m->flags & FL_SUBMENU) {
	    nest++;
	}
	m++;
	if (!nest && m->visible())
	    n--;
    }
    return m;
}

// appearance of current menus are pulled from this parent widget:
static const NxMenu_ *button;

////////////////////////////////////////////////////////////////

// tiny window for title of menu:
class menutitle:public NxMenuWindow
{
    void draw();
  public:
    const NxMenuItem *menu;
      menutitle(int X, int Y, int W, int H, const NxMenuItem *);
};

// each vertical menu has one of these:
class nxmenuwindow:public NxMenuWindow
{
    void draw();
    void drawentry(const NxMenuItem *, int i, int erase);
  public:
      menutitle * title;
    int handle(int);
    int itemheight;		// zero == menubar
    int numitems;
    int selected;
    int scrollnumber;		// the number of item that is shown first
    int scrollsize;		// number of items to show in drop down menu
    int drawn_selected;		// last redraw has this selected
    const NxMenuItem *menu;
    const NxMenuItem *init_menu;
      nxmenuwindow(const NxMenuItem * m, int X, int Y, int W, int H,
		   int scroll_size, const NxMenuItem * picked,
		   const NxMenuItem * title, int menubar =
		   0, int menubar_title = 0);
     ~nxmenuwindow();
    void set_selected(int);
    int find_selected(int mx, int my);
    int titlex(int);
    void autoscroll(int);
    void position(int x, int y);
    void scroll(int direction);
    void reposition();
    bool scrolls;
};

#define UP 			1
#define DOWN 		2
#define SCROLL  4

#define LEADING 4		// extra vertical leading

// values for menustate.state:
#define INITIAL_STATE 0		// no mouse up or down since popup() called
#define PUSH_STATE 1		// mouse has been pushed on a normal item
#define DONE_STATE 2		// exit the popup, the current item was picked
#define MENU_PUSH_STATE 3	// mouse has been pushed on a menu title
#define MENU_SCROLL_STATE 4	// scroll on the menu has been pushed

struct menustate
{
    const NxMenuItem *current_item;	// what mouse is pointing at
    int menu_number;		// which menu it is in
    int item_number;		// which item in that menu, -1 if none
    nxmenuwindow *p[20];	// pointers to menus
    int nummenus;
    int menubar;		// if true p[0] is a menubar
    int state;
    int scroll_count;
};
static menustate *p;

extern char fl_draw_shortcut;

// width of label, including effect of & characters:
int
NxMenuItem::measure(int *hp, const NxMenu_ * m) const
{
    Fl_Label l;
    l.value = text;
    l.type = labeltype_;

    /* JHC -  A change here - make sure that we use the default
       fonts and not the standard fonts */

    l.font =
	labelsize_ ? labelfont_ : uchar(m ? m->
					textfont() : DEFAULT_TEXT_FONT);
    l.size = labelsize_ ? labelsize_ : m ? m->textsize() : DEFAULT_TEXT_SIZE;
    l.color = labelcolor_;
    fl_draw_shortcut = 1;
    int w = 0;
    int h = 0;
    l.measure(w, hp ? *hp : h);
    fl_draw_shortcut = 0;
    if (flags & (FL_MENU_TOGGLE | FL_MENU_RADIO))
	w += 14;
    return w;
}

void
NxMenuItem::draw(int x, int y, int w, int h, const NxMenu_ * m, int selected) const
{

    Fl_Label l;
    l.value = text;
    l.type = labeltype_;

    l.font =
	labelsize_ ? labelfont_ : uchar(m ? m->
					textfont() : DEFAULT_TEXT_FONT);
    l.size = labelsize_ ? labelsize_ : m ? m->textsize() : DEFAULT_TEXT_SIZE;
    l.color = labelcolor_ ? labelcolor_ : m ? m->textcolor() : int (FL_BLACK);
    if (!active())
	l.color = inactive((Fl_Color) l.color);
    Fl_Color color = m ? m->color() : FL_GRAY;

    if (selected) {

	Fl_Color r = m ? m->selection_color() : FL_SELECTION_COLOR;
	Fl_Boxtype b = m && m->down_box()? m->down_box() : FL_FLAT_BOX;

	if (contrast(r, color) != r) {	// back compatability boxtypes
	    if (selected == 2) {	// menu title
		r = color;
		b = m ? m->box() : FL_UP_BOX;
	    } else {
		r = (Fl_Color) (FL_COLOR_CUBE - 1);	// white
		l.color = contrast((Fl_Color) labelcolor_, r);
	    }
	} else {
	    l.color = contrast((Fl_Color) labelcolor_, r);
	}

	if (selected == 2) {	// menu title
	    fl_draw_box(b, x, y, w, h, r);
	    x += 3;
	    w -= 8;
	} else {
	    //fl_draw_box(b, x+1, y-1, w-2, h+2, r);
	    fl_draw_box(b, x + 1, y - 1, w - 1, h + 3, r);
	}
    }


    if (flags & (FL_MENU_TOGGLE | FL_MENU_RADIO)) {

	int y1 = y + (h - 14) / 2;
	fl_color(FL_DARK3);
	if (flags & FL_MENU_RADIO) {
	    fl_line(x + 2, y1 + 7, x + 8, y1 + 1, x + 14, y1 + 7);
	    if (selected) {
		fl_color(color);
		fl_polygon(x + 3, y1 + 7, x + 8, y1 + 2, x + 13, y1 + 7,
			   x + 8, y1 + 12);
	    }
	    fl_color(FL_LIGHT3);
	    fl_line(x + 14, y1 + 7, x + 8, y1 + 13, x + 2, y1 + 7);
	    if (value()) {
		fl_color(FL_BLACK);
		fl_polygon(x + 4, y1 + 7, x + 8, y1 + 3, x + 12, y1 + 7,
			   x + 8, y1 + 11);
	    }
	} else {

	    fl_yxline(x + 3, y1 + 11, y1 + 2, x + 12);

	    if (selected) {
		fl_color(color);
		fl_rectf(x + 4, y1 + 3, 9, 9);
	    }

	    fl_color(FL_LIGHT3);
	    fl_xyline(x + 4, y1 + 12, x + 13, y1 + 3);

	    if (value()) {
		fl_color(FL_BLACK);
		fl_rectf(x + 5, y1 + 4, 7, 7);
	    }

	}
	x += 14;
	w -= 14;
    }

    if (!fl_draw_shortcut)
	fl_draw_shortcut = 1;

    l.draw(x + 3, y, w, h, FL_ALIGN_LEFT);
    fl_draw_shortcut = 0;

}

menutitle::menutitle(int X, int Y, int W, int H, const NxMenuItem * L):
NxMenuWindow(X, Y, W, H, 0)
{
    end();
    set_modal();
    clear_border();
    menu = L;
    if (L->labelcolor_)
	clear_overlay();
}

nxmenuwindow::nxmenuwindow(const NxMenuItem * m, int X, int Y, int Wp, int Hp,
			   int scroll_size, const NxMenuItem * picked,
			   const NxMenuItem * t, int menubar,
			   int menubar_title)
    :
NxMenuWindow(X, Y, Wp, Hp, 0)
{
    end();
    set_modal();
    clear_border();
    menu = m;
    drawn_selected = -1;
    scrollnumber = 0;
    scrollsize = scroll_size;
    box(button && button->box()? button->box() : FL_UP_BOX);
    color(button ? button->color() : FL_GRAY);

    NxApp::Instance()->def_font(this);

    selected = -1;
    init_menu = 0;
    scrolls = true;
    {
	int i = 0;
	if (m)
	    for (const NxMenuItem * m1 = m;; m1 = m1->next(), i++) {
		if (picked) {
		    if (m1 == picked) {
			selected = i;
			picked = 0;
			init_menu = m1;
		    } else if (m1 > picked) {
			selected = i - 1;
			picked = 0;
			Wp = Hp = 0;
		    }
		}
		if (!m1->text)
		    break;
	    }
	numitems = i;
    }

    if (numitems <= scrollsize) {
	scrolls = false;
    } else
	numitems += 2;

    if (menubar) {
	itemheight = 0;
	title = 0;
	return;
    }

    itemheight = 1;

    int hotKeysw = 0;
    int Wtitle = 0;
    int Htitle = 0;
    if (t)
	Wtitle = t->measure(&Htitle, button) + 12;
    int W = 0;
    if (m)
	for (; m->text; m = m->next()) {
	    int h;
	    int w1 = m->measure(&h, button);
	    if (h + LEADING > itemheight)
		itemheight = h + LEADING;
	    if (m->flags & (FL_SUBMENU | FL_SUBMENU_POINTER))
		w1 += 14;
	    if (w1 > W)
		W = w1;
	    if (m->shortcut_) {
		w1 = int (fl_width(fl_shortcut_label(m->shortcut_))) + 8;
		if (w1 > hotKeysw)
		    hotKeysw = w1;
	    }
	    if (m->labelcolor_)
		clear_overlay();
	}
    if (selected >= 0 && !Wp)
	X -= W / 2;
    int BW = Fl::box_dx(box());
    W += hotKeysw + 2 * BW + 7;
    if (Wp > W)
	W = Wp;
    if (Wtitle > W)
	W = Wtitle;

    if (!Wp) {
	if (X < 0)
	    X = 0;
	if (X > Fl::w() - W)
	    X = Fl::w() - W;
    }
    x(X);
    w(W);

    //h((numitems ? itemheight*numitems-LEADING : 0)+2*BW+5); // add
    if (!scrolls)
	h((numitems ? itemheight * numitems - LEADING : 0) + 2 * BW + 5);
    else
	h((numitems ? itemheight * (scrollsize + 2) - LEADING : 0) + 2 * BW +
	  5);

    if (selected >= 0)
	Y = Y + (Hp - itemheight) / 2 - selected * itemheight - BW + 1;
    else
	Y = Y + Hp;
    if (m)
	y(Y - 1);
    else {
	y(Y - 3);
	w(1);
	h(1);
    }

    if (t) {
	int ht = menubar_title ? button->h() - 6 : Htitle + 2 * BW + 3;
	title = new menutitle(X, Y - ht - 3, Wtitle, ht, t);
    } else
	title = 0;
    reposition();
}

nxmenuwindow::~nxmenuwindow()
{
    delete title;
}

void
nxmenuwindow::position(int X, int Y)
{
    if (title) {
	title->position(X, title->y() + Y - y());
    }
    NxMenuWindow::position(X, Y);
    // x(X); y(Y); // don't wait for response from X
}

// scroll so item i is visible on screen
void
nxmenuwindow::autoscroll(int i)
{
    if (i == SCROLL_UP)
	return;
    int Y = y() + Fl::box_dx(box()) + 2 + i * itemheight;
    if (Y <= Fl::y())
	Y = Fl::y() - Y + 10;
    else {
	Y = Y + itemheight - Fl::h() - Fl::y();
	if (Y < 0)
	    return;
	Y = -Y - 10;
    }
    NxMenuWindow::position(x(), y() + Y);
    // y(y()+Y); // don't wait for response from X
}

void
nxmenuwindow::scroll(int direction)
{
    int set_draw = 0;

    if (DOWN & direction) {
	int hidden = (numitems - 2) - (scrollnumber + 1);
	hidden = numitems - 2 - hidden;
	if (hidden + scrollsize <= numitems - 2) {
	    set_draw = 1;
	    if (direction & SCROLL) {
		scrollnumber++;
	    } else {
		scrollnumber++;
		selected--;
	    }
	}
    } else if (UP & direction) {
	if (scrollnumber) {
	    set_draw = 1;
	    if (direction & SCROLL) {
		scrollnumber--;
	    } else {
		scrollnumber--;
		selected++;
	    }
	}
    }
    if (1 == set_draw) {
	damage(FL_DAMAGE_ALL);
	redraw();
    }
}

////////////////////////////////////////////////////////////////

void
nxmenuwindow::drawentry(const NxMenuItem * m, int i, int erase)
{
    if (!m)
	return;			// this happens if -1 is selected item and redrawn

    int BW = Fl::box_dx(box());
    int x = BW;
    int W = this->w();
    int w = W - 2 * BW - 1;
    int y = 0;
    if (scrolls)
	y = BW + 2 + (i + 1) * itemheight;
    else
	y = BW + 2 + i * itemheight;

    int h = itemheight - LEADING;

    if (erase && i != selected) {
	fl_color(button ? button->color() : FL_GRAY);
	//    fl_rectf(x+1, y-1, w-2, h+2);
	fl_rectf(x + 1, y - 1, w - 1, h + 3);
    }

    m->draw(x, y, w, h, button, i == selected);

    // the shortcuts and arrows assumme fl_color() was left set by draw():
    if (m->submenu()) {
	int y1 = y + (h - 14) / 2;
	fl_polygon(x + w - 13, y1 + 2, x + w - 13, y1 + 2 + 10, x + w - 3,
		   y1 + 2 + 5);
    } else if (m->shortcut_) {
	Fl_Font f = button ? button->textfont() : (Fl_Font) DEFAULT_TEXT_FONT;
	fl_font(f, button ? button->textsize() : DEFAULT_TEXT_SIZE);
	fl_draw(fl_shortcut_label(m->shortcut_), x, y, w - 3, h,
		FL_ALIGN_RIGHT);
    }

    if (m->flags & FL_MENU_DIVIDER) {
	fl_color(FL_DARK3);
	fl_xyline(BW - 1, y + h + 1, W - 2 * BW + 2);
	fl_color(FL_LIGHT3);
	fl_xyline(BW - 1, y + h + 2, W - 2 * BW + 2);
    }

}

void
menutitle::draw()
{
    menu->draw(0, 0, w(), h(), button, 2);
}

void
nxmenuwindow::reposition()
{
    int Y = y() + Fl::box_dx(box()) + LEADING + (scrollsize + 3) * itemheight;

    Y = Y + itemheight - Fl::h() - Fl::y();
    if (Y < 0) {
	return;
    }
    Y = y() + LEADING - (scrollsize + 3) * itemheight;
    NxMenuWindow::position(x(), Y);

}

void
nxmenuwindow::draw()
{

    reposition();
    menustate & p = *(::p);
    if (scrolls) {
	if (damage() != FL_DAMAGE_CHILD) {	// complete redraw
	    const NxMenuItem *m;
	    int picked_menu = -1;
	    if (init_menu && menu && p.state == INITIAL_STATE) {
		for (m = menu, picked_menu = 0; m->next();
		     m = m->next(), picked_menu++) {
		    if (m == init_menu)
			break;
		}
	    }

	    fl_draw_box(box(), 0, 0, w(), h(), color());
	    if (menu) {
		int i;
		if (picked_menu >= 0) {	// move picked item to top if not last entries
		    int rem = numitems - picked_menu - 3;
		    if (rem < scrollsize - 1) {	// on the last entries
			selected = scrollsize - rem - 1;
			scrollnumber = numitems - scrollsize - 2;
			for (m = menu, i = 0; m->text; i++, m = m->next())
			    if (i >= numitems - scrollsize - 2)
				drawentry(m, i - scrollnumber, 0);
		    } else {
			scrollnumber = picked_menu;
			selected = 0;
			for (m = menu, i = 0; m->text; i++, m = m->next()) {
			    if (i >= scrollnumber
				&& i <= (scrollsize + scrollnumber - 1))
				drawentry(m, i - scrollnumber, 0);
			}
		    }
		} else {
		    for (m = menu, i = 0; m->text; i++, m = m->next())
			if (i >= scrollnumber
			    && i <= (scrollsize + scrollnumber - 1))
			    drawentry(m, i - scrollnumber, 0);
		}
	    }
	} else {
	    if (selected < scrollsize + scrollnumber) {
		if (damage() & FL_DAMAGE_CHILD && selected != drawn_selected) {	// change selection
		    if (drawn_selected >= 0 && drawn_selected < scrollsize)
			drawentry(menu->next(drawn_selected + scrollnumber),
				  drawn_selected, 1);
		    drawentry(menu->next(selected + scrollnumber), selected,
			      1);
		}
	    }
	}
	drawn_selected = selected;

	//draw the up arrow
	fl_color(FL_BLACK);
	fl_begin_polygon();
	fl_transformed_vertex(itemheight / 2, 5);
	fl_transformed_vertex(itemheight / 2 - 5, itemheight - 6);
	fl_transformed_vertex(itemheight / 2 + 5, itemheight - 6);
	fl_end_polygon();

	//draw the down arrow
	fl_begin_polygon();
	fl_transformed_vertex(itemheight / 2, h() - 5);
	fl_transformed_vertex(itemheight / 2 - 5, h() - itemheight + 7);
	fl_transformed_vertex(itemheight / 2 + 5, h() - itemheight + 7);
	fl_end_polygon();
    } else {
	if (damage() != FL_DAMAGE_CHILD) {	// complete redraw
	    fl_draw_box(box(), 0, 0, w(), h(), color());
	    if (menu) {
		const NxMenuItem *m;
		int i;
		for (m = menu, i = 0; m->text; i++, m = m->next())
		    drawentry(m, i, 0);
	    }
	} else {
	    if (damage() & FL_DAMAGE_CHILD && selected != drawn_selected) {	// change selection
		drawentry(menu->next(drawn_selected), drawn_selected, 1);
		drawentry(menu->next(selected), selected, 1);
	    }
	}
	drawn_selected = selected;
    }
}

void
nxmenuwindow::set_selected(int i)
{
    if (scrolls) {
	if (scrollnumber)
	    selected = i - scrollnumber;
	else
	    selected = i;
	damage(FL_DAMAGE_CHILD);
    } else {
	if (i != selected) {
	    selected = i;
	    damage(FL_DAMAGE_CHILD);
	}
    }
}

////////////////////////////////////////////////////////////////

int
nxmenuwindow::find_selected(int mx, int my)
{
    if (!menu || !menu->text)
	return -1;
    mx -= x();
    my -= y();
    if (my < 0 || my >= h())
	return -1;
    if (!itemheight) {		// menubar
	int x = 3;
	int i = 0;
	const NxMenuItem *m = menu;
	for (;; m = m->next(), i++) {
	    if (!m->text)
		return -1;
	    x += m->measure(0, button) + 16;
	    if (x > mx)
		break;
	}
	return i;
    }
    if (mx < Fl::box_dx(box()) || mx >= w())
	return -1;
    int i = (my - Fl::box_dx(box()) - 1) / itemheight;
    if (scrolls) {
	if (0 == i)
	    return SCROLL_UP;
	i--;			// add here
	i += scrollnumber;
    }
    if (i < 0 || i >= numitems + 1)
	return -1;
    return i;
}

// return horizontal position for item i in a menubar:
int
nxmenuwindow::titlex(int i)
{
    const NxMenuItem *m;
    int x = 3;
    for (m = menu; i--; m = m->next())
	x += m->measure(0, button) + 16;
    return x;
}

// match shortcuts & label shortcuts, don't search submenus:
// returns menu item and index
const NxMenuItem *
NxMenuItem::find_shortcut(int *ip) const
{
    const NxMenuItem *m1 = this;
    for (int ii = 0; m1 && m1->text; m1 = m1->next(1), ii++) {
	if (m1->activevisible() && (Fl::test_shortcut(m1->shortcut_)
				    || Fl_Widget::test_shortcut(m1->text))) {
	    if (ip)
		*ip = ii;
	    return m1;
	}
    }
    return 0;
}

////////////////////////////////////////////////////////////////
// NxMenuItem::popup(...)

// Because Fl::grab() is done, all events go to one of the menu windows.
// But the handle method needs to look at all of them to find out
// what item the user is pointing at.  And it needs a whole lot
// of other state variables to determine what is going on with
// the currently displayed menus.
// So the main loop (handlemenu()) puts all the state in a structure
// and puts a pointer to it in a static location, so the handle()
// on menus can refer to it and alter it.  The handle() method
// changes variables in this state to indicate what item is
// picked, but does not actually alter the display, instead the
// main loop does that.  This is because the X mapping and unmapping
// of windows is slow, and we don't want to fall behind the events.

static inline void
setitem(const NxMenuItem * i, int m, int n)
{
    p->current_item = i;
    p->menu_number = m;
    p->item_number = n;
}

static void
setitem(int m, int n)
{
    menustate & p = *(::p);
    p.current_item = (n >= 0) ? p.p[m]->menu->next(n) : 0;
    p.menu_number = m;
    p.item_number = n;
}

static int
forward(int menu)
{				// go to next item in menu menu if possible
    menustate & p = *(::p);
    nxmenuwindow & m = *(p.p[menu]);
    int item = (menu == p.menu_number) ? p.item_number : m.selected;
    while (++item < m.numitems) {
	const NxMenuItem *m1 = m.menu->next(item);
	if (m1->activevisible()) {
	    setitem(m1, menu, item);
	    return 1;
	}
    }
    return 0;
}

static int
backward(int menu)
{				// previous item in menu menu if possible
    menustate & p = *(::p);
    nxmenuwindow & m = *(p.p[menu]);
    int item = (menu == p.menu_number) ? p.item_number : m.selected;
    if (item < 0)
	item = m.numitems;
    while (--item >= 0) {
	const NxMenuItem *m1 = m.menu->next(item);
	if (m1->activevisible()) {
	    setitem(m1, menu, item);
	    return 1;
	}
    }
    return 0;
}

int
nxmenuwindow::handle(int e)
{
    menustate & p = *(::p);
    switch (e) {
    case FL_KEYBOARD:
	switch (Fl::event_key()) {
	case FL_Tab:
	    if (Fl::event_shift() & FL_SHIFT)
		goto BACKTAB;
	case ' ':
	    if (!forward(p.menu_number)) {
		p.item_number = -1;
		forward(p.menu_number);
	    }
	    return 1;
	case FL_BackSpace:
	case 0xFE20:		// backtab
	  BACKTAB:
	    if (!backward(p.menu_number)) {
		p.item_number = -1;
		backward(p.menu_number);
	    }
	    return 1;
	case FL_Up:
	    if (p.menubar && p.menu_number == 0);
	    else if (backward(p.menu_number));
	    else if (p.menubar && p.menu_number == 1)
		setitem(0, p.p[0]->selected);
	    return 1;
	case FL_Down:
	    if (p.menu_number || !p.menubar)
		forward(p.menu_number);
	    else if (p.menu_number < p.nummenus - 1)
		forward(p.menu_number + 1);
	    return 1;
	case FL_Right:
	    if (p.menubar
		&& (p.menu_number <= 0 || p.menu_number == 1
		    && p.nummenus == 2))
		forward(0);
	    else if (p.menu_number < p.nummenus - 1)
		forward(p.menu_number + 1);
	    return 1;
	case FL_Left:
	    if (p.menubar && p.menu_number <= 1)
		backward(0);
	    else if (p.menu_number > 0)
		setitem(p.menu_number - 1, p.p[p.menu_number - 1]->selected);
	    return 1;
	case FL_Enter:
	    p.state = DONE_STATE;
	    return 1;
	case FL_Escape:
	    setitem(0, -1, 0);
	    p.state = DONE_STATE;
	    return 1;
	}
	break;
    case FL_SHORTCUT:{
	    for (int menu = p.nummenus; menu--;) {
		nxmenuwindow & mw = *(p.p[menu]);
		int item;
		const NxMenuItem *m = mw.menu->find_shortcut(&item);
		if (m) {
		    setitem(m, menu, item);
		    if (!m->submenu())
			p.state = DONE_STATE;
		    return 1;
		}
	    }
	}
	break;
    case FL_PUSH:
	//case FL_MOVE:
    case FL_DRAG:{
	    int mx = Fl::event_x() + x();
	    int my = Fl::event_y() + y();
	    int item = 0;
	    int menu;
	    for (menu = p.nummenus - 1;; menu--) {
		item = p.p[menu]->find_selected(mx, my);
		int scroll_state = 0;
		if (scrolls) {
		    if (SCROLL_UP == item) {
			scroll_state = UP;
			if (e == FL_DRAG)
			    scroll_state |= SCROLL;
			p.p[menu]->scroll(scroll_state);
			p.state = MENU_SCROLL_STATE;
			return 1;
		    }
		    if (p.p[menu]->numitems - 2 == item ||
			item >
			(p.p[menu]->scrollsize + p.p[menu]->scrollnumber -
			 1)) {
			scroll_state = DOWN;
			if (e == FL_DRAG)
			    scroll_state |= SCROLL;
			p.p[menu]->scroll(scroll_state);
			p.state = MENU_SCROLL_STATE;
			return 1;
		    }
		}
		if (FL_PUSH == e) {
		    if (p.current_item && p.current_item->submenu()	// this is a menu title
			&& item != p.p[menu]->selected	// and it is not already on
			&& !p.current_item->callback_)	// and it does not have a callback
			p.state = MENU_PUSH_STATE;
		    else
			p.state = PUSH_STATE;

		}
		if (item >= 0)
		    break;
		if (menu <= 0)
		    break;
	    }
	    if (item < p.p[menu]->scrollsize + p.p[menu]->scrollnumber) {
		setitem(menu, item);
	    }
	}
	return 1;
    case FL_RELEASE:
	// do nothing if they try to pick inactive items
	if (p.current_item && !p.current_item->activevisible()) {
	    return 1;
	}
	// Mouse must either be held down/dragged some, or this must be
	// the second click (not the one that popped up the menu):
	if (!Fl::event_is_click() || p.state == PUSH_STATE || p.menubar && p.current_item && !p.current_item->submenu()	// button
	    ) {
#if 0				// makes the check/radio items leave the menu up
	    const NxMenuItem *m = p.current_item;
	    if (m && button && (m->flags & (FL_MENU_TOGGLE | FL_MENU_RADIO))) {
		((NxMenu_ *) button)->picked(m);
		p.p[p.menu_number]->redraw();
	    } else
#endif
		p.state = DONE_STATE;
	}
	return 1;
    }
    return Fl_Window::handle(e);
}

const NxMenuItem *
NxMenuItem::pulldown(int X, int Y, int W, int H, int scroll_size,
		     const NxMenuItem * initial_item,
		     const NxMenu_ * pbutton,
		     const NxMenuItem * t, int menubar) const
{
    Fl_Group::current(0);	// fix possible user error...

    printf("MENU:  Starting at %d, %d\n", X, Y);

    button = pbutton;
    if (pbutton) {
	for (Fl_Window * w = pbutton->window(); w; w = w->window()) {
	    X += w->x();
	    Y += w->y();
	}
    } else {
	X += Fl::event_x_root() - Fl::event_x();
	Y += Fl::event_y_root() - Fl::event_y();
    }

    nxmenuwindow mw(this, X, Y, W, H, scroll_size, initial_item, t, menubar);
    Fl::grab(mw);
    menustate p;
    ::p = &p;
    p.p[0] = &mw;
    p.nummenus = 1;
    p.menubar = menubar;
    p.state = INITIAL_STATE;

    nxmenuwindow *fakemenu = 0;	// kludge for buttons in menubar

    // preselected item, pop up submenus if necessary:
    if (initial_item && mw.selected >= 0) {
	setitem(0, mw.selected);
	goto STARTUP;
    }

    p.current_item = 0;
    p.menu_number = 0;
    p.item_number = -1;
    if (menubar)
	mw.handle(FL_DRAG);	// find the initial menu
    initial_item = p.current_item;
    if (initial_item)
	goto STARTUP;

    // the main loop, runs until p.state goes to DONE_STATE:
    for (;;) {

	// make sure all the menus are shown:
	{
	    for (int k = menubar; k < p.nummenus; k++)
		if (!p.p[k]->shown()) {
		    if (p.p[k]->title)
			p.p[k]->title->show();
		    p.p[k]->show();
		}
	}

	// get events:
	{
	    const NxMenuItem *oldi = p.current_item;
	    Fl::wait();
	    if (p.state == DONE_STATE)
		break;		// done.
	    if (p.current_item == oldi)
		continue;
	}
	// only do rest if item changes:

	delete fakemenu;
	fakemenu = 0;		// turn off "menubar button"

	if (!p.current_item) {	// pointing at nothing
	    // turn off selection in deepest menu, but don't erase other menus:
	    p.p[p.nummenus - 1]->set_selected(-1);
	    continue;
	}

	delete fakemenu;
	fakemenu = 0;
	initial_item = 0;	// stop the startup code
	//p.p[p.menu_number]->autoscroll(p.item_number);

      STARTUP:
	nxmenuwindow & cw = *p.p[p.menu_number];
	const NxMenuItem *m = p.current_item;
	if (!m->activevisible()) {	// pointing at inactive item
	    cw.set_selected(-1);
	    initial_item = 0;	// turn off startup code
	    continue;
	}
	cw.set_selected(p.item_number);

	if (m == initial_item)
	    initial_item = 0;	// stop the startup code if item found
	if (m->submenu()) {
	    const NxMenuItem *title = m;
	    const NxMenuItem *menutable;
	    if (m->flags & FL_SUBMENU)
		menutable = m + 1;
	    else
		menutable = (NxMenuItem *) (m)->user_data_;
	    // figure out where new menu goes:
	    int nX, nY;
	    if (!p.menu_number && p.menubar) {	// menu off a menubar:
		nX = cw.x() + cw.titlex(p.item_number);
		nY = cw.y() + cw.h();
		initial_item = 0;
	    } else {
		nX = cw.x() + cw.w();
		nY = cw.y() + 1 + p.item_number * cw.itemheight;
		title = 0;
	    }
	    if (initial_item) {	// bring up submenu containing initial item:
		nxmenuwindow *n =
		    new nxmenuwindow(menutable, X, Y, W, H, scroll_size,
				     initial_item, title);
		p.p[p.nummenus++] = n;
		// move all earlier menus to line up with this new one:
		if (n->selected >= 0) {
		    int dy = n->y() - nY;
		    int dx = n->x() - nX;
		    for (int menu = 0; menu <= p.menu_number; menu++) {
			nxmenuwindow *t = p.p[menu];
			int nx = t->x() + dx;
			if (nx < 0) {
			    nx = 0;
			    dx = -t->x();
			}
			int ny = t->y() + dy + 1;
			if (ny < 0) {
			    ny = 0;
			    dy = -t->y() - 1;
			}
			t->position(nx, ny);
		    }
		    setitem(p.nummenus - 1, n->selected);
		    goto STARTUP;
		}
	    } else if (p.nummenus > p.menu_number + 1 &&
		       p.p[p.menu_number + 1]->menu == menutable) {
		// the menu is already up:
		while (p.nummenus > p.menu_number + 2)
		    delete p.p[--p.nummenus];
		p.p[p.nummenus - 1]->set_selected(-1);
	    } else {
		// delete all the old menus and create new one:
		while (p.nummenus > p.menu_number + 1)
		    delete p.p[--p.nummenus];
		p.p[p.nummenus++] = new nxmenuwindow(menutable, nX, nY,
						     title ? 1 : 0, 0,
						     scroll_size, 0, title, 0,
						     menubar);
	    }
	} else {		// !m->submenu():
	    while (p.nummenus > p.menu_number + 1)
		delete p.p[--p.nummenus];
	    if (!p.menu_number && p.menubar) {
		// kludge so "menubar buttons" turn "on" by using menu title:
		fakemenu = new nxmenuwindow(0,
					    cw.x() + cw.titlex(p.item_number),
					    cw.y() + cw.h(), 0, scroll_size,
					    0, 0, m, 0, 1);
		fakemenu->title->show();
	    }
	}
    }
    const NxMenuItem *m = p.current_item;
    delete fakemenu;
    while (p.nummenus > 1)
	delete p.p[--p.nummenus];
    mw.hide();
    Fl::release();
    return m;
}

const NxMenuItem *
NxMenuItem::popup(int X, int Y,
		  int scroll_size,
		  const char *title,
		  const NxMenuItem * picked, const NxMenu_ * button) const
{
    static NxMenuItem dummy;	// static so it is all zeros
    dummy.text = title;
    return pulldown(X, Y, 0, 0, scroll_size, picked, button,
		    title ? &dummy : 0);
}

const NxMenuItem *
NxMenuItem::test_shortcut() const
{
    const NxMenuItem *m = this;
    const NxMenuItem *ret = 0;
    if (m)
	for (; m->text; m = m->next()) {
	    if (m->activevisible()) {
		// return immediately any match of an item in top level menu:
		if (Fl::test_shortcut(m->shortcut_))
		    return m;
		// if (Fl_Widget::test_shortcut(m->text)) return m;
		// only return matches from lower menu if nothing found in top menu:
		if (!ret && m->submenu()) {
		    const NxMenuItem *s =
			(m->flags & FL_SUBMENU) ? m +
			1 : (const NxMenuItem *) m->user_data_;
		    ret = s->test_shortcut();
		}
	    }
	}
    return ret;
}
