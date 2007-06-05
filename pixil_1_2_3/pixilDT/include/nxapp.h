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

#ifndef NXAPP_H
#define NXAPP_H

#define MAX_WIN 255

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_ToggleTree.H>

#include <FL/nxdb.h>		// databases

#ifdef NANOX
#include <nano-X.h>
#endif

#include <string>
#include <vector>

extern "C"
{

#include <par.h>

}

#include <stdio.h>

//
// CONSTANTS
//

static db_handle *
openPar()
{
    db_handle *db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
    return db;
}

static db_handle *par_db = openPar();

static int
getGblParInt(char *cat, char *pref)
{
    int s;
    par_getGlobalPref(par_db, cat, pref, PAR_INT, &s, sizeof(int));
    return s;
}

static void
getGblParStr(char *cat, char *pref, char *text, int len)
{
    par_getGlobalPref(par_db, cat, pref, PAR_TEXT, text, len);
}

// Default Colors
//#define DEF_FG  FL_BLACK
//#define DEF_BG  FL_WHITE
//#define DEF_SEL FL_BLACK

// Default Window Button Size
#define W_X getGblParInt("appsizes", "w_x")
#define W_Y getGblParInt("appsizes", "w_y")
#define W_W getGblParInt("appsizes", "w_w")
#define W_H getGblParInt("appsizes", "w_h")

     // Default Menu Button Size
#define MB_X getGblParInt("appsizes", "mb_x")
#define MB_Y getGblParInt("appsizes", "mb_y")
#define MB_W getGblParInt("appsizes", "mb_w")
#define MB_H getGblParInt("appsizes", "mb_h")

     // Default Menu Size
#define MENU_X getGblParInt("appsizes", "menu_x")
#define MENU_Y getGblParInt("appsizes", "menu_y")
#define MENU_W getGblParInt("appsizes", "menu_w")
#define MENU_H getGblParInt("appsizes", "menu_h")

     // Default Category List Size
#define CL_X getGblParInt("appsizes", "cl_x")
#define CL_Y getGblParInt("appsizes", "cl_y")
#define CL_W getGblParInt("appsizes", "cl_w")
#define CL_H getGblParInt("appsizes", "cl_h")

     // BUTTONS
#define BUTTON_X getGblParInt("appsizes", "button_x")
#define BUTTON_Y getGblParInt("appsizes", "button_y")
#define BUTTON_WIDTH  getGblParInt("appsizes", "button_w")
#define BUTTON_HEIGHT getGblParInt("appsizes", "button_h")

     // FONTS
#define DEFAULT_LABEL_FONT getGblParInt("appfonts", "lbl_font")
#define DEFAULT_LABEL_SIZE getGblParInt("appfonts", "lbl_size")
#define DEFAULT_TEXT_FONT getGblParInt("appfonts", "txt_font")
#define DEFAULT_TEXT_SIZE getGblParInt("appfonts", "txt_size")
#define DEFAULT_BIG_FONT getGblParInt("appfonts", "big_font")
#define DEFAULT_BIG_SIZE getGblParInt("appfonts", "big_size")
#define DEFAULT_SMALL_SIZE getGblParInt("appfonts", "sm_size")
#define DEFAULT_SMALL_FONT getGblParInt("appfonts", "sm_font")

#define SMALL_BLK 512
#define BIG_BLK   1024

// FLNX-Colosseum IPC
#define MAX_CLIENTS 255
#define MAX_LENGTH 4096
#define TOKEN "^"

// year formats
#define SHORT_YEAR 0x0001
#define LONG_YEAR  0x0002

// Windows
enum
{ NONE, DEACTIVATE, ACTIVATE };

typedef struct about_
{
    char title[50];
    char copyright[50];
    char author[50];
    char date[20];
    char version[10];
}
about;

// NxApp Global Colors
enum NxApp_Color
{
    APP_BG = 0,
    APP_FG,
    APP_SEL,
    BUTTON_FACE,
    BUTTON_TEXT,
    BUTTON_PUSH,
    BUTTON_3D_LITE,
    BUTTON_3D_DARK,
    HILIGHT,
    HILIGHT_TEXT,
    HILIGHT_LITE,
    HILIGHT_DARK,
    TITLE_FG,
    TITLE_BG,
    SCROLL_FACE,
    SCROLL_LITE,
    SCROLL_DARK,
    SCROLL_TRAY,
    RADIO_FILL,
    EDITOR_BG,
    EDITOR_FG,
    EDITOR_SEL
};

#define MAX_CLR_ATTRIB 22

struct sync_db_struct
{
    NxDb *db;
    field *pField;
    string str_dbName;
    int table_num;
    int size;
    int *file_fields;
};

// Implement NxApp as Singleton pattern.
class NxApp
{

  private:
    int saveX, saveY, saveW, saveH;
    int nextSyncState;
    int appSyncState;
    int exitApp;

  public:
    int noguisearch_flag;
    static NxApp *Instance();	/* Global Access Point */

  protected:
      NxApp(char *app = 0);
      virtual ~ NxApp();
    void SyncDb(char *new_msg);

    // FLNX-Colosseum IPC
  public:
    void IPCError(int err, char *errMsg);	/* Error codes from Colosseum */
    int StartApp(char *_appName, unsigned char *args = 0, int flags =
		 0, int timeout = 0);
    int ExecuteShow();
    int ExecuteNoGui();
    int Add_Fd(char *_appName, void (*cb) (int, void *), void *o = 0);
    void Remove_Fd(int fd);
    int Find_Fd(char *_appName);
    int Read_Fd(char *readMsg, int *length);
    int Write_Fd(int fd, char *writeMsg, int length);
    int VerifyClient(char *client);
    static void _ClientIPCHandler(int fd, void *o);
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1)
    {
    }
    void ServerIPCHandler(int fd, int ipc_id, void *o);
    int GetDateString(char *buf, const struct tm *tm, int size, int format);
    bool ValidMsgId(short int msg_id);

    // PIM database synchronization
    /////////////////////////////////////////////////////////////////////////////////////

  private:
    vector < sync_db_struct > v_SyncDB;
    int cur_db;
    int cur_row;
    int cur_table;
    int total_rows;
    int rows[1024];
    int cur_table_size;
    sync_db_struct *c_db_struct;
    void GetRowData(int &flags, vector < string > &data, string & key);
    void GetTableSchema(vector < char >&col_type, vector < int >&col_size);
    bool SaveRowData(const vector < string > &vmessages);
    bool CheckTableSchema(const vector < string > &vmessages);
    void UpdateFlags();
    void DoError(int err, int ipc);
    int agent_ipc;
    int app_ipc;
  protected:
    virtual void Refresh()
    {
    }

  public:
    // This methods puts the names of each database to synchronize
    // into the vstr_SyncDB vector<string>.
    int GetKey(NxDb * db, char *db_name, int key_field);
    void SyncRegisterDB(NxDb * db, field * pField, string str_dbName,
			int table_num, int *file_fields = NULL, int size = 0)
    {
	sync_db_struct s;


	if (NULL != file_fields) {
	    s.size = size;
	    s.file_fields = new int[size];
	    memcpy(s.file_fields, file_fields, size * sizeof(int));
	} else {
	    s.size = 0;
	    s.file_fields = NULL;
	}
	s.db = db;
	s.pField = pField;
	s.str_dbName = str_dbName;
	s.table_num = table_num;
	v_SyncDB.push_back(s);
    }


  private:
    int fd;
    char *winTitle;
    char *appName;
    int nextClient;
    char *clientList[MAX_CLIENTS];

  private:
    static NxApp *_instance;
#ifdef NANOX
    GR_WINDOW_ID check_for_keyboard(GR_WINDOW_ID wid);
#endif
    int launch_keyboard(int, int, int, int);

  private:
    string keyboard_path_;
    string keyboard_maps_;
    string keyboard_size_;
    Fl_Window *window[MAX_WIN];
    Fl_Window *catlist_window;
    int cur_size;
    void *cat_ptr;
    bool title_running;
    bool keyboard_running;
    about about_app;
    Fl_Window *shown_window;
    Fl_Window *about_window;
    static bool paste_ok_;
    int exitOnSearch;

    static void hide_about_cb(Fl_Widget * fl, void *o);

    // Global Colors
    Fl_Color global_colors[MAX_CLR_ATTRIB];
    Fl_Color globalColor(unsigned long c);

  protected:
    int NxApp::colorsFromPAR();

  public:
    Fl_Color getGlobalColor(NxApp_Color nc);

    static char *strup(const char *str1, int size);
    static char *strdown(const char *str1, int size);
    static bool searchFile(const char *str, const char *file);
    static Fl_Widget *g_PasteTarget;
    static Fl_Widget *undoTarget;
    static int fl_editor_type;

    void hide_all_windows();
    virtual void show_window(Fl_Window * w = 0, int type =
			     NONE, Fl_Window * w_target = 0);
    void add_window(Fl_Window * w);
    void resize_notify(int X, int Y, int W, int H);

    // Callbacks  
    static void undo_callback(Fl_Widget * fl, void *o);
    static void copy_callback(Fl_Widget * fl, void *o);
    static void pasteTarget_callback(Fl_Widget * fl, void *o = 0);
    static void paste_callback(Fl_Widget * fl, void *o);
    static void cut_callback(Fl_Widget * fl, void *o);
    static void keyboard_callback(Fl_Widget * fl, void *o);

    void set_catlist_window(Fl_Window * w);
    void set_keyboard(int argc, char *argv[]);
    Fl_Window *get_catlist_window();

    Fl_Window **get_window_list()
    {
	return window;
    }
    Fl_Window *get_shown_window()
    {
	return shown_window;
    }
    void set_shown_window(Fl_Window * win)
    {
	shown_window = win;
    }
    Fl_Window *get_about_window()
    {
	return about_window;
    }
    void set_about_window(Fl_Window * win)
    {
	about_window = win;
    }

    void set_about(about);
    about get_about()
    {
	return about_app;
    }
    static void show_about(Fl_Widget *, void *);

    int get_cur_size()
    {
	return cur_size;
    }

    // Default fonts
    void def_font(Fl_Widget * widget);
    void def_font(Fl_Input * input);
    void def_font(Fl_Output * output);
    void big_font(Fl_Output * output);
    void def_font(Fl_Menu_Button * mb);
    void def_font(Fl_Menu_Bar * mb);
    void def_font(Fl_Hold_Browser * hb);
    void def_font(Fl_ToggleTree * tree);
    void def_font();
    void def_small_font();
};

#endif
