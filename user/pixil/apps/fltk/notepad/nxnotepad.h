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


#ifndef __NXNOTE_H
#define __NXNOTE_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <nxbox.h>
#include <Flek/Fl_Toggle_Tree.H>
#include <FL/Flv_Table_Child.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Editor.H>

#include <nxapp.h>
#include <nxdb.h>
#include <nxwindow.h>
#include <nxmenubutton.h>
#include <nxscroll.h>
#include <nxinput.h>
#include <nxoutput.h>
#include <nxbutton.h>

#include <catlist.h>

#define CAT_NUM 2

#define APP_NAME "Notes"

#define NOTE_DATABASE 	"not"
#define KEY_LEN 			  10
#define CAT_LEN					13
#define TITLE_LEN				50
#define FILE_LEN			  100
#define NOTE_NUM_FIELDS 5
#define NOTE_INDEX 			0
#define NOTE_CAT 				1
#define NOTE_FILE				2
#define NOTE_DESC 			3
#define NOTE_ARCH				4
#define CAT_DATABASE  	"notcat"
#define CAT_NUM_FIELDS 	2
#define CAT_INDEX 			0
#define CAT_DESC				1

struct NxNote
{
    int key;
    char szFile[FILE_LEN];
    char szTitle[TITLE_LEN];
    char szCategory[CAT_LEN];
    int bDeleteMe;
};

class NxNotepad:public NxApp
{

  private:
    static bool AllFlag;
    static int note_key;

#ifdef CONFIG_COLOSSEUM
    // FLNX-Colosseum IPC
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);
    static void ExecuteSearch(int ipc_handler, char *searchStr, int width);
#endif

    static NxScroll *note_list;
    about about_app;

    static bool g_SearchFlag;

    static NxDb *note_db;
    static char *nx_inidir;

    static NxWindow *main_window;

    // Standard PIM Windows
    static NxPimWindow *note_list_window;
    static NxPimWindow *note_edit_window;

    // Pim Popup Windows
    static NxPimPopWindow *note_delete_window;
    static NxPimPopWindow *note_lookup_window;
    static NxPimPopWindow *note_results_window;

    static Fl_Editor *g_editor;

    // Lookup widgets
    static NxInput *lookup_input;
    static Flv_Table_Child *results_table;
    static NxOutput *results_message;

    static NxCategoryList *note_category;
    static NxCategoryList *edit_category;

    static Fl_Toggle_Tree *tree;
    static Fl_Pixmap *folderSmall;

    static void note_tree_callback(Fl_Widget * fl, long l);
    static void edit_callback(Fl_Widget * fl, long l);
    static void delete_callback(Fl_Widget * fl, long l);
    static void category_callback(Fl_Widget * fl, void *l);
    static void list_callback(Fl_Widget * fl, void *l);
    static void done_edit_callback(Fl_Widget * fl, void *l);
    static void yes_delete_callback(Fl_Widget * fl, void *l);
    static void no_delete_callback(Fl_Widget * fl, void *l);
    static void save_archive_callback(Fl_Widget * fl, void *l);
    static void cancelLookup_callback(Fl_Widget * fl, void *l);
    static void doneLookup_callback(Fl_Widget * fl, void *l);
    static void searchLookup_callback(Fl_Widget * fl, void *l);
    static void view_callback(Fl_Widget * fl, void *l);
    static void viewRecord(char *fileName);
    static void select_note(NxNote * note);

    static void set_category(const char *szCat);
    static void reset_category(char *szCat);
    static void clear_tree();
    static void add_items(Fl_Toggle_Tree * t, const char *szCategory);
    static void write_note(NxNote * note);

    static NxNote *search(const char *);
    static char *formatString(const NxNote *, int);

    static bool save_archive_;
    static NxCategoryList *cat_list[CAT_NUM];
    static bool new_note_;
  private:
    void make_list_window();
    void make_edit_window();
    void make_delete_window();
    void make_lookup_window();
    void make_results_window();
    static void write_archive_copy();
    static void open_note_database();
    static void open_cat_database();
    static void insert_default_categories();
    static void insert_default_note();
    static void fill_categories();

  public:
      NxNotepad(int argc, char *argv[]);
      virtual ~ NxNotepad();
    Fl_Window *get_main_window();
    void show_default_window();

  protected:
      virtual void Refresh();

  public:
    // Menu Item callbacks
    static void new_callback(Fl_Widget *, void *);
    static void delete_note_callback(Fl_Widget *, void *);
    static void lookup_callback(Fl_Widget * fl, void *l);
    static void exit_callback(Fl_Widget * fl, void *l);
};

extern char NxEditCategoryBuf[30];

#endif
