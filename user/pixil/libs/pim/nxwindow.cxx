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


#include <nxwindow.h>
#include <iostream>
#include <nxapp.h>
#include <nxbox.h>

///////////////////////
// NxPimWindow Methods

//
// Constructors
//

extern void fl_internal_boxtype(Fl_Boxtype t, Fl_Box_Draw_F * f);

void
nx_rectbound(int x, int y, int w, int h, Fl_Color bgcolor)
{
    fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
    fl_rect(x, y, w, h);
    fl_color(bgcolor);
    fl_rectf(x + 1, y + 1, w - 2, h - 2);
}

void
tab_draw(int x, int y, int w, int h, Fl_Color c)
{
    fl_color(c);
    fl_draw_box(FL_ROUNDED_BOX, x, y, w + 5, h, c);
    fl_rectf(x, h - 5, w + 5, 10);
    fl_rectf(w, h + 3, 152, 2);
}

void
nx_bottom_box(int x, int y, int w, int h, Fl_Color c)
{
    y = y + h - 3;
    fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
    fl_rect(x, y, w, 1);
    fl_color(c);
    fl_rectf(x + 1, y + 1, w - 2, h - 2);
}

// Default
NxPimWindow::NxPimWindow(char *_title, Fl_Menu_Item * menu_item, NxDb * catDb,
			 string _catDbName, string _noteDbName,
			 void (*update_items) (const char *category))
{

    //  fl_internal_boxtype(FL_BORDER_BOX, nx_rectbound);
    //  fl_internal_boxtype(_FL_ROUNDED_BOX, nx_rectbound);
    //  fl_internal_boxtype(FL_UP_BOX, nx_rectbound);
#ifdef NANOX
    fl_internal_boxtype(FL_BOTTOM_BOX, nx_bottom_box);
#else
    fl_internal_boxtype(FL_DOWN_BOX, nx_bottom_box);
#endif

    menu = 0;
    grp = 0;

    catDb_ = catDb;
    //fprintf(stderr, "NxPimwWindow:catDb_[%p]\n", catDb_);

    pim_window = new NxDoubleWindow(W_X, W_Y, W_W, W_H);
    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);

    menu_bar = new menu_cb_struct;

    grp = new Fl_Group(MENU_X, MENU_Y, MENU_W, MENU_H);

    // Menu Button

#ifndef TABS
    menu_button = new NxButton(MB_X, MB_Y, MB_W, MB_H, _title);
    menu_button->movable(false);	// don't allow this button to do any resize.
#else
    Fl::set_boxtype(FL_PDA_NO_BOX, tab_draw, 1, 1, 2, 2);
    menu_button = new Fl_Button(2, 1, MB_W, MB_H, _title);
    menu_button->box(FL_PDA_NO_BOX);
    menu_button->color(NxApp::Instance()->getGlobalColor(APP_BG));
    menu_button->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));
#endif

    menu_button->callback((Fl_Callback *) hideButtons_cb, (void *) menu_bar);

    if (catDb_) {

	// Category List
#ifndef TABS
	category_list =
	    new NxCategoryList(CL_X, CL_Y, CL_W, CL_H, catDb, _catDbName);
#else
	category_list =
	    new NxCategoryList(CL_X + 5, 2, CL_W, CL_H - 1, catDb,
			       _catDbName);
#endif
    } else {
	category_list = 0;
    }

    grp->end();

    // Menu
    menu = new NxMenuBar(MENU_X, MENU_Y, MENU_W, MENU_H, grp);
    menu->menu(menu_item);
    menu->callback((Fl_Callback *) hideMenuBar_cb, (void *) menu_bar);
    menu->hide();
    menu_bar->grp = grp;
    menu_bar->menu = menu;

    pim_window->end();

    if (catDb_)
	MakeEditCategoryWindow(catDb, _catDbName, _noteDbName,
			       (void (*)(const char *)) update_items);

}

//  Size All
NxPimWindow::NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb,
			 string _catDbName, string _noteDbName,
			 void (*update_items) (const char *category), int x,
			 int y, int w, int h, int menu_x, int menu_y,
			 int menu_w, int menu_h, int cl_x, int cl_y, int cl_w,
			 int cl_h, char *nx_inipath)
{

    menu = 0;
    grp = 0;

    catDb_ = catDb;
    pim_window = new NxDoubleWindow(x, y, w, h);
    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);

    // Menu
    menu = new NxMenuBar(menu_x, menu_y, menu_w, menu_h);
    menu->menu(menu_item);
    menu->hide();

    if (catDb_) {
	// Category List
	category_list =
	    new NxCategoryList(cl_x, cl_y, cl_w, cl_h, catDb, _catDbName);
    } else {
	category_list = 0;
    }

    // Menu Button
    menu_button = new NxButton(MB_X, MB_Y, MB_W, MB_H, "Address");

    pim_window->end();

    if (catDb_)
	MakeEditCategoryWindow(catDb, _catDbName, _noteDbName,
			       (void (*)(const char *)) update_items);

}

// Size Window Only or Menu Only
NxPimWindow::NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb,
			 string _catDbName, string _noteDbName,
			 void (*update_items) (const char *category), int x,
			 int y, int w, int h, int type)
{

    menu = 0;
    grp = 0;

    catDb_ = catDb;
    // Window
    if (type == WINDOW)
	pim_window = new NxDoubleWindow(x, y, w, h);
    else
	pim_window = new NxDoubleWindow(W_X, W_Y, W_W, W_H);

    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);

    // Menu
    if (type == MENU)
	menu = new NxMenuBar(x, y, w, h);
    else
	menu = new NxMenuBar(MENU_X, MENU_Y, MENU_W, MENU_H);

    menu->hide();

    if (catDb_) {
	// Category List
	category_list =
	    new NxCategoryList(CL_X, CL_Y, CL_W, CL_H, catDb, _catDbName);
    } else {
	category_list = 0;
    }

    // Menu Button
    menu_button = new NxButton(MB_X, MB_Y, MB_W, MB_H, "Address");

    pim_window->end();

    if (catDb_)
	MakeEditCategoryWindow(catDb, _catDbName, _noteDbName,
			       (void (*)(const char *)) update_items);

}

// Just a menubar or just a menubutton, but not both.
NxPimWindow::NxPimWindow(char *_title, Fl_Menu_Item * menu_item, int type)
{

#ifdef NANOX
    fl_internal_boxtype(FL_BOTTOM_BOX, nx_bottom_box);
#else
    fl_internal_boxtype(FL_DOWN_BOX, nx_bottom_box);
#endif

    menu = 0;
    grp = 0;

    pim_window = new NxDoubleWindow(W_X, W_Y, W_W, W_H);
    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);

    if (type == MENU) {

	// Menu
	_menu = new NxMenuButton(MB_X, MB_Y, MB_W, MB_H, _title);
	_menu->menu(menu_item);

    } else {

	menu_bar = new menu_cb_struct;
	grp = new Fl_Group(MENU_X, MENU_Y, MENU_W, MENU_H);

	// Menu Button
	menu_button = new NxButton(MB_X, MB_Y, MB_W, MB_H, _title);
	menu_button->callback((Fl_Callback *) hideButtons_cb,
			      (void *) menu_bar);

	grp->end();

	// Menu
	menu = new NxMenuBar(MENU_X, MENU_Y, MENU_W, MENU_H, grp);
	menu->menu(menu_item);
	menu->callback((Fl_Callback *) hideMenuBar_cb, (void *) menu_bar);
	menu->hide();
	menu_bar->grp = grp;
	menu_bar->menu = menu;
    }

    pim_window->end();

}

// Size Category List Only
NxPimWindow::NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb,
			 string _catDbName, string _noteDbName,
			 void (*update_items) (const char *category),
			 int cl_x, int cl_y, int cl_w, int cl_h,
			 char *nx_inipath)
{

    menu = 0;
    grp = 0;

    catDb_ = catDb;
    pim_window = new NxDoubleWindow(W_X, W_Y, W_W, W_H);
    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);

    // Menu
    menu = new NxMenuBar(MENU_X, MENU_Y, MENU_W, MENU_H);
    menu->menu(menu_item);
    menu->box(FL_BORDER_BOX);
    menu->hide();

    if (catDb_) {
	// Category List
	category_list =
	    new NxCategoryList(cl_x, cl_y, cl_w, cl_h, catDb, _catDbName);
    } else {
	category_list = 0;
    }

    // Menu Button
    menu_button = new NxButton(MB_X, MB_Y, MB_W, MB_H, "Address");

    pim_window->end();
    if (catDb_)
	MakeEditCategoryWindow(catDb, _catDbName, _noteDbName,
			       (void (*)(const char *)) update_items);

}

NxPimWindow::NxPimWindow(int x, int y, int w, int h)
{

    menu = 0;
    grp = 0;

    pim_window = new NxDoubleWindow(x, y, w, h);
    pim_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pim_window->WinType(PIM_WINDOW);
    pim_window->end();

}

// Callbacks
void
NxPimWindow::hideMenuBar_cb(Fl_Widget * fl, void *o)
{

    menu_cb_struct *s = (menu_cb_struct *) o;
    Fl_Group *grp = s->grp;
    NxMenuBar *menu = s->menu;

    menu->hide();
    grp->show();

}

void
NxPimWindow::hideButtons_cb(Fl_Widget * fl, void *o)
{

    menu_cb_struct *s = (menu_cb_struct *) o;
    Fl_Group *grp = s->grp;
    NxMenuBar *menu = s->menu;

    grp->hide();
    menu->show();

}

// Methods

NxDoubleWindow *
NxPimWindow::GetWindowPtr()
{
    return pim_window;
}

NxDoubleWindow *
NxPimWindow::GetEditCategoryWindowPtr()
{
    return edit_category_window->GetWindowPtr();
}

void
NxPimWindow::add(Fl_Widget * w)
{
    pim_window->add(w);
}

void
NxPimWindow::MakeEditCategoryWindow(NxDb * catDb, string catdb_name,
				    string notedb_name,
				    void (*update_items) (const char
							  *category))
{
    edit_category_window = new NxEditCategory(catDb, catdb_name, notedb_name,
					      (void (*)(const char *))
					      update_items);
    NxApp::Instance()->add_window(edit_category_window->GetWindowPtr());
    edit_category_window->set_show_window(this);
    edit_category_window->set_menu_list(this->category_list);
}

//////////////////////////
// NxPimPopWindow Methods

//
// Constructors
//
#include <nxwindow.h>

NxPimPopWindow::NxPimPopWindow(char *title,
			       Fl_Color title_color,
			       int x, int y, int w, int h)
{


    pimp_window = new NxDoubleWindow(x, y, w, h);
    pimp_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pimp_window->WinType(PIMP_WINDOW);

    {
	NxBox *o = new NxBox(0, 0, w - 1, h);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_BORDER_BOX);
    }

    {
	NxOutput *o = new NxOutput(3, 3, (w - 7), 15, "");
	o->box(FL_BORDER_BOX);
	o->textcolor(NxApp::Instance()->getGlobalColor(TITLE_FG));
	o->color(NxApp::Instance()->getGlobalColor(TITLE_BG));
	o->value(title);
    }

    pimp_window->end();
    pimp_window->resizable(pimp_window);

}

NxPimPopWindow::NxPimPopWindow(int x, int y, int w, int h)
{

    pimp_window = new NxDoubleWindow(x, y, w, h);
    pimp_window->color(NxApp::Instance()->getGlobalColor(APP_BG));
    pimp_window->WinType(PIMP_WINDOW);

    {
	Fl_Box *o = new Fl_Box(0, 0, w, h);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_BORDER_BOX);
    }

    pimp_window->end();
    pimp_window->resizable(pimp_window);

}

// Methods

NxDoubleWindow *
NxPimPopWindow::GetWindowPtr()
{
    return pimp_window;
}

void
NxPimPopWindow::add(Fl_Widget * w)
{
    pimp_window->add(w);
}
