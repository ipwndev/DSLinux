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

#ifndef NXWINDOW_H
#define NXWINDOW_H

#include <string.h>

#include <FL/nxdb.h>
#include <FL/nxmenubar.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Box.H>
#include "FL/Fl_ToggleTree.H"

#include "catlist.h"

#include <FL/nxapp.h>
#include <FL/nxbutton.h>
#include <FL/nxoutput.h>
#include <FL/nxinput.h>
#include <FL/nxoutput.h>
#include <FL/nxscroll.h>

// Default Category List Ini File
#define NX_INIPATH "./nxad"

struct menu_cb_struct
{
    Fl_Group *grp;
    NxMenuBar *menu;
};

enum
{ WINDOW, MENU };

// Normal PIM Window with Menu and Category List

class NxEditCategory;

class NxWindow:public Fl_Window
{
  public:
    NxWindow(int x, int y, int w, int h, char *l = 0):Fl_Window(x, y, w, h, l)
    {
	this->resizable(this);
    }
    NxWindow(int w, int h, char *l = 0):Fl_Window(w, h, l)
    {
	this->resizable(this);
    }

    virtual void resize_notify(int X, int Y, int W, int H)
    {
	//printf("resize_notify to H = %d\n", H);
	NxApp::Instance()->resize_notify(0, 0, W, H);

    }

};

#define PIM_WINDOW 0
#define PIMP_WINDOW 1

class NxDoubleWindow:public Fl_Double_Window
{

    int type;
    int save_h;

  public:
      NxDoubleWindow(int x, int y, int w, int h, char *l =
		     0):Fl_Double_Window(x, y, w, h, l)
    {
	this->resizable(this);
	type = -1;
	save_h = h;
    }
    void WinType(int _type)
    {
	type = _type;
    }
    int WinType()
    {
	return type;
    }

    virtual void resize_notify(int X, int Y, int W, int H)
    {

	int dy;

	//printf("NxDoubleWindow::resize_notify\t%d, %d, %d, %d\n",X, Y, W, H);

	this->damage(FL_DAMAGE_ALL);

	switch (type) {

	case PIM_WINDOW:

	    if ((X == x()) && (Y == y()) && (W == w()) && (H == h()))
		return;

	    if (parent() == NULL)	// Always resize top-level windows
		size(this->w(), H);
	    else if ((H + this->y()) <= parent()->h()) {	// Resize if fits within parent.
		size(this->w(), H);
	    } else if ((H + this->y()) == parent()->h())
		size(this->w(), H);
	    else if ((h() + y()) > parent()->h())	// If > parent, fit within parent
		size(this->w(), parent()->h() - y());

	    redraw();

	    break;

	case PIMP_WINDOW:

	    // Always center PIM_POP_WINDOWS to parent windows
	    dy = (parent()->h() - H) / 2;

	    if (dy == Y)
		return;

	    this->resize(this->x(), dy, this->w(), save_h);
	    break;

	default:
	    size(W, H);
	    break;

	}			// switch

    }				// resize_notify()

};

class NxPimWindow
{

  private:

    NxDoubleWindow * pim_window;
    Fl_Group *grp;
    menu_cb_struct *menu_bar;
    char *title;
    NxDb *catDb_;

    NxEditCategory *edit_category_window;
    static void hideButtons_cb(Fl_Widget * fl, void *o);
    static void hideMenuBar_cb(Fl_Widget * fl, void *o);

    void MakeEditCategoryWindow(NxDb *, string, string,
				void (*)(const char *));
  public:

    ////////////////////////////////////////////////////////////////////////////////
    // Constructors

    // Default NxPimWindow Constructor: menu bar, category screens, and database.
      NxPimWindow(char *_title, Fl_Menu_Item * menu_item, NxDb * catDb,
		  string _catDbName, string _noteDbName,
		  void (*update_items) (const char *category));

    // Size All
      NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb, string _catDbName,
		  string _noteDbName,
		  void (*update_items) (const char *category), int x, int y,
		  int w, int h, int menu_x, int menu_y, int menu_w,
		  int menu_h, int cl_x, int cl_y, int cl_w, int cl_h,
		  char *nx_inipath);

    // Menu bar only
      NxPimWindow(char *_title, Fl_Menu_Item * menu_item, int type);

    // Size Window Only or Menu Only
    // Where type == WIND or TYPE = MENU
      NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb, string _catDbName,
		  string _noteDbName,
		  void (*update_items) (const char *category), int x, int y,
		  int w, int h, int type);

    // Size Category List Only
      NxPimWindow(Fl_Menu_Item * menu_item, NxDb * catDb, string _catDbName,
		  string _noteDbName,
		  void (*update_items) (const char *category), int cl_x,
		  int cl_y, int cl_w, int cl_h, char *nx_inipath);

    // Window only
      NxPimWindow(int x, int y, int w, int h);


      virtual ~ NxPimWindow()
    {
	delete menu_bar;
    }

    // Methods
    NxDoubleWindow *GetWindowPtr();
    void add(Fl_Widget * w);
    void show();
    NxButton *menu_button;
    NxMenuButton *_menu;
    NxMenuBar *menu;
    NxCategoryList *category_list;
    NxDoubleWindow *GetEditCategoryWindowPtr();
    NxEditCategory *GetEditCategoryPtr()
    {
	return edit_category_window;
    }
    NxDoubleWindow *get_pim_window()
    {
	return pim_window;
    }
    void set_pim_window(NxDoubleWindow * w)
    {
	pim_window = w;
    }

};

// PIM Popup Window
class NxPimPopWindow
{

  private:
    NxDoubleWindow * pimp_window;

  public:
    // Constructors
    NxPimPopWindow(char *title,
		   Fl_Color title_color =
		   NxApp::Instance()->getGlobalColor(APP_FG), int x =
		   10, int y = (W_H / 3), int w = W_W - 20, int h = 114);

      NxPimPopWindow(int x = (W_W / 3), int y = (W_H / 3), int w =
		     (W_W / 2), int h = 230);

    // Methods
    NxDoubleWindow *GetWindowPtr();
    void add(Fl_Widget * w);

};


#define MAX_NAME_SIZE 10

class NxPimWindow;
class NxPimPopWindow;

class NxEditCategory:public NxPimWindow
{
    NxDoubleWindow *pim_window;
    NxCategoryList *menu_list_;
    NxDb *cat_db_;
    field *cat_field_;
    ndxfile *cat_ndx_;
    fildes *cat_des_;
    field *note_field_;
    ndxfile *note_ndx_;
    fildes *note_des_;
    NxScroll *category_list_;
    Fl_ToggleTree *category_tree_;
    NxPimWindow *show_win_;
    NxPimPopWindow *new_cat_win_;
    NxPimPopWindow *rename_cat_win_;
    NxPimPopWindow *delete_cat_win_;
    NxInput *new_cat_input_;
    NxInput *rename_cat_input_;
    NxButton *done_button_;
    NxButton *new_button_;
    NxButton *rename_button_;
    NxButton *delete_button_;
    static FL_EXPORT void list_cb(Fl_Widget *, void *);
    static FL_EXPORT void done_cb(Fl_Widget *, void *);
    static FL_EXPORT void new_cb(Fl_Widget *, void *);
    static FL_EXPORT void rename_cb(Fl_Widget *, void *);
    static FL_EXPORT void delete_cb(Fl_Widget *, void *);
    static FL_EXPORT void delete_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void done_new_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void cancel_new_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void done_rename_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void cancel_rename_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void delete_yes_cat_cb(Fl_Widget *, void *);
    static FL_EXPORT void delete_no_cat_cb(Fl_Widget *, void *);
    Fl_Pixmap *folderSmall_;
    string catdb_name_;
    string notedb_name_;
    void rename_cat_update(char *, char *);
    void new_cat_update(char *new_value);
    NxEditCategory *g_NxEditCatWin;
  public:
      NxPimWindow * get_show_window()
    {
	return show_win_;
    }
    NxInput *get_new_cat_input()
    {
	return new_cat_input_;
    }
    NxInput *get_rename_cat_input()
    {
	return rename_cat_input_;
    }
    NxPimPopWindow *get_new_cat_window()
    {
	return new_cat_win_;
    }
    NxPimPopWindow *get_rename_cat_window()
    {
	return rename_cat_win_;
    }
    NxPimPopWindow *get_delete_cat_window()
    {
	return delete_cat_win_;
    }
    Fl_ToggleTree *get_category_tree()
    {
	return category_tree_;
    }
    NxDb *get_cat_db()
    {
	return cat_db_;
    }
    field *get_cat_field()
    {
	return cat_field_;
    }
    ndxfile *get_cat_ndx()
    {
	return cat_ndx_;
    }
    fildes *get_cat_des()
    {
	return cat_des_;
    }
    field *get_note_field()
    {
	return note_field_;
    }
    ndxfile *get_note_ndx()
    {
	return note_ndx_;
    }
    fildes *get_note_des()
    {
	return note_des_;
    }
    NxScroll *get_category_list()
    {
	return category_list_;
    }
    void set_show_window(NxPimWindow * pim_win)
    {
	show_win_ = pim_win;
    }
    void set_menu_list(NxCategoryList * menu_list)
    {
	menu_list_ = menu_list;
    }
    NxCategoryList *get_menu_list()
    {
	return menu_list_;
    }
    string get_catdb_name()
    {
	return catdb_name_;
    }
    string get_notedb_name()
    {
	return notedb_name_;
    }
    Fl_Pixmap *get_folder_pix()
    {
	return folderSmall_;
    }
    NxEditCategory(NxDb * catDb, string, string, void (*)(const char *) = 0);
    void add_items(Fl_ToggleTree * t);
    void clear_tree();
};

#endif
