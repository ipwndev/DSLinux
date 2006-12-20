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


#ifndef nxnotepad_h
#define nxnotepad_h

#include <FL/Fl.H>
#include <nxbox.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Flv_Table_Child.H>
#include <FL/Fl_Editor.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Editor.H>

#include <nxapp.h>
#include <nxwindow.h>
#include <nxscroll.h>
#include <nxinput.h>
#include <nxoutput.h>
#include <nxdb.h>
#include <catlist.h>

#include "misclist.h"

#define APP_NAME "Contacts"
#define CAT_NUM  3
#define SECTION   100
#define CATEGORYS 50

//#define ID  4
#define TEXT 50
#define DBL_TEXT 100
#define DATE 20
#define NOTEDB 100

#define HOME 0
#define WORK 1
#define FAX 2
#define MOBILE 3
#define PAGER 4
#define EMAIL 5
#define WEBPAGE 6

struct NxTodo
{

    // Indexes
    //char szId[ID];
    //char szCategory[CATEGORYS];
    int nId;			// replacing szId[ID]
    int nCat;			// replacing szCategory[ID]
    char szCategory[CATEGORYS];	// Category label
    int nShowDisplay;

    // Data
    char szLastName[TEXT];
    char szFirstName[TEXT];
    char szCompany[TEXT];
    char szJobTitle[TEXT];
    int nDep1;
    int nDep2;
    int nDep3;
    int nDep4;
    int nDep5;
    int nDep6;
    int nDep7;
    char szDep1[TEXT];
    char szDep2[TEXT];
    char szDep3[TEXT];
    char szDep4[TEXT];
    char szDep5[TEXT];
    char szDep6[TEXT];
    char szDep7[TEXT];
    char szAddress[DBL_TEXT];
    char szCity[TEXT];
    char szRegion[TEXT];	// State, Province, Town, etc.
    char szPostalCode[TEXT];	// ZIP, Postal Code, etc.
    char szCountry[TEXT];
    char szBDay[DATE];
    char szAnniv[DATE];
    char szCustom1[TEXT];
    char szCustom2[TEXT];
    char szCustom3[TEXT];
    char szCustom4[TEXT];
    char szNoteFile[NOTEDB];
};

class NxAddress:public NxApp
{

    ////////////////////////////////////////////////////////////////////////////////
    // Data Members

  private:
    NxScroll * note_list;

    static bool AllFlag;

    static NxDb *db;
    static int id;
    static int g_EditFlag;
    static int g_SearchFlag;
    static char *nx_inidir;

    // Container Window
    static NxWindow *main_window;

    // Standard PIM Windows
    static NxPimWindow *addr_list_window;
    static NxPimWindow *addr_edit_window;
    static NxPimWindow *addr_view_window;

    // PIM Popup Windows
    static NxPimPopWindow *addr_details_window;
    static NxPimPopWindow *addr_note_window;
    static NxPimPopWindow *addr_dellist_window;
    static NxPimPopWindow *addr_deledit_window;
    static NxPimPopWindow *addr_delview_window;
    static NxPimPopWindow *addr_lookup_window;
    static NxPimPopWindow *addr_results_window;
    static NxPimPopWindow *addr_custom_window;

    static Fl_Editor *g_editor;

    // Phone Lookup Methods
    static NxInput *lookup_input;
    static Flv_Table_Child *results_table;
    static NxOutput *results_message;

    static NxCategoryList *note_category;
    static NxCategoryList *edit_category_list;
    static NxCategoryList *view_category_list;
    //  static NxCategoryList * details_category_list;
    static NxCategoryList *cat_list[CAT_NUM];

    static Flv_Table_Child *table;

    // edit window
    static NxInput *edit_lastname;
    static NxInput *edit_firstname;
    static NxInput *edit_company;
    static NxInput *edit_title;
    static NxMiscList *edit_misc_list1;
    static NxMiscList *edit_misc_list2;
    static NxMiscList *edit_misc_list3;
    static NxMiscList *edit_misc_list4;
    static NxMiscList *edit_misc_list5;
    static NxMiscList *edit_misc_list6;
    static NxMiscList *edit_misc_list7;
    static NxInput *edit_misc1;
    static NxInput *edit_misc2;
    static NxInput *edit_misc3;
    static NxInput *edit_misc4;
    static NxInput *edit_misc5;
    static NxInput *edit_misc6;
    static NxInput *edit_misc7;
    static NxInput *editAddress;
    static NxInput *editCity;
    static NxInput *editRegion;
    static NxInput *editPostalCode;
    static NxInput *editCountry;
    static NxInput *edit_bday;
    static NxInput *edit_anniv;
    static NxInput *edit_custom1;
    static NxInput *edit_custom2;
    static NxInput *edit_custom3;
    static NxInput *edit_custom4;

    // view window
    static NxOutput *viewName;
    static NxOutput *viewBusTitle;
    static NxOutput *viewCompany;
    static NxOutput *viewAddress;
    static NxOutput *viewCityRegionPC;
    static NxOutput *viewCountry;
    static NxOutput *viewWorkPhone;
    static NxOutput *viewEMail;

    // custom fields window
    static NxInput *custom1Input;
    static NxInput *custom2Input;
    static NxInput *custom3Input;
    static NxInput *custom4Input;

    // details window
    static NxMiscList *details_show_list;
    static Fl_Pixmap *address_pixmap;

    ////////////////////////////////////////////////////////////////////////////////
    // FLNX-Colosseum IPC

#ifdef CONFIG_COLOSSEUM
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);
    static void ExecuteSearch(int ipc_id, char *searchStr, int width);
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // Private Callbacks

    // addr_list_window callbacks
    static void new_callback(Fl_Widget * fl, long l);	// New Button
    static void view_callback(Fl_Widget * fl, void *o = 0);	// Double Click
    static void edit_callback(Fl_Widget * fl, long l);	// Edit Button
    static void delList_callback(Fl_Widget * fl, void *l);	// Delete Button
    static void yesDelList_callback(Fl_Widget * fl, void *l);	// Yes Delete Button
    static void noDelList_callback(Fl_Widget * fl, void *l);	// No Delete Button

    // addr_view_window callbacks
    static void doneView_callback(Fl_Widget * fl, long l);	// Done Button
    //  static void edit_callback(Fl_Widget * fl, long l); // Edit Button
    //  static void delView_callback(Fl_Widget * fl, void * l); // Delete Button
    static void yesDelView_callback(Fl_Widget * fl, void *l);	// Yes Button
    static void noDelView_callback(Fl_Widget * fl, void *l);	// No Button

    // addr_edit_window callbacks
    static void doneEdit_callback(Fl_Widget * fl, long l);	// Done Button
    static void delEdit_callback(Fl_Widget * fl, void *l);	// Delete Button
    static void yesDelEdit_callback(Fl_Widget * fl, void *l);	// Yes Delete Button
    static void noDelEdit_callback(Fl_Widget * fl, void *l);	// No Delete Button
    static void cancelEdit_callback(Fl_Widget * fl, void *l);	// Cancel Button
    static void notes_callback(Fl_Widget * fl, void *l);	// Notes Button

    // addr_lookup_window callbacks
    static void searchLookup_callback(Fl_Widget * fl, void *o);
    static void cancelLookup_callback(Fl_Widget * fl, void *o);
    static void doneLookup_callback(Fl_Widget * fl, void *o);

    // addr_details_window callbacks
    static void doneDetails_callback(Fl_Widget * fl, void *l);	// Done Button
    static void cancelDetails_callback(Fl_Widget * fl, void *l);	// Cancel Button
    //static void delDetails_callback(Fl_Widget* fl, void *l); // Delete Button

    // addr_note_window callbacks
    static void doneNote_callback(Fl_Widget * fl, long l);	// Done Button

    // addr_custom_window callbacks
    static void doneCustom_callback(Fl_Widget * fl, void *o);
    static void cancelCustom_callback(Fl_Widget * fl, void *o);

    ////////////////////////////////////////////////////////////////////////////////
    // Database functions

    static char *Record(int id, int cat_id, int show_id,
			string last_name, string first_name, string company,
			string title, int info1id, int info2id, int info3id,
			int info4id, int info5id, int info6id, int info7id,
			string info1, string info2, string info3,
			string info4, string info5, string info6,
			string info7, string address, string city,
			string region, string postalCode, string country,
			string bday, string anniv, string custom1,
			string custom2, string custom3, string custom4,
			string note);
    static char *Record(int infoid, string info_type, int dummy);
    static char *Record(int catid, string cat_name);
    static char *CustRecord(int custid, string cust_name);
    static int GetCatId(char *szCategory);

    // Database Callbacks
    static void priority_callback(Fl_Widget * fl, void *l);	// Database
    static void category_callback(Fl_Widget * fl, void *l);	// Database

    static void fill_categories();
    static void list_callback(Fl_Widget * fl, void *l);

    static void set_category(char *szCat);
    static void reset_category(char *szCat);
    static void clear_table();
    static void add_items(Flv_Table_Child * t, const char *szCategory);

    static void _fill_view_form(NxTodo * n);
    static void _fill_form(NxTodo * n);
    static void edit_note(NxTodo * note, int recno);
    static void write_note(NxTodo * note);

    ////////////////////////////////////////////////////////////////////////////////
    // Searching

    static NxTodo *search(const char *searchVal);
    static char *formatString(const NxTodo * note, int pixels);

    static void viewRecord(int recno);
    static void select_note(NxTodo * note);

  private:
    static char *szNoteFile;

  public:
    static char *get_szNoteFile()
    {
	return szNoteFile;
    }
    static void set_szNoteFile(char *_szNoteFile)
    {
	memset(szNoteFile, 0, NOTEDB);
	strcpy(szNoteFile, _szNoteFile);
	//cout << "set_szNoteFile(): szNoteFile = " << szNoteFile << endl;
    }

  private:
    void make_list_window();
    void make_view_window();
    void make_edit_window();
    void make_details_window();
    void make_note_window();
    void make_dellist_window();
    void make_deledit_window();
    void make_delview_window();
    void make_lookup_window();
    void make_results_window();
    void make_custom_window();

    static char *strup(const char *str1, int size);
    static int compar(NxTodo ** rec1, NxTodo ** rec2);

  protected:
    virtual void Refresh();

  public:
    NxAddress(int argc, char *argv[]);
    ~NxAddress();
    Fl_Window *get_main_window();
    void show_default_window();
    char **GetSearchResults();


    ////////////////////////////////////////////////////////////////////////////////
    // Public menu callbacks

  public:
    static void dup_callback(Fl_Widget * fl, void *o);
    static void delView_callback(Fl_Widget * fl, void *l);
    static void lookup_callback(Fl_Widget * fl, void *o);
    static void exit_callback(Fl_Widget * fl, void *l);
    static void details_callback(Fl_Widget * fl, void *o);
    static void custom_callback(Fl_Widget * fl, void *o);

};

#endif
