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

#include <pixil_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pixil_config.h>

#include "nxaddress.h"

#include <nxbutton.h>
#include <nxmultilineoutput.h>

#include <icons/address.xpm>

#define DEBUG

#ifdef DEBUG
#define DPRINT(str, args...) printf("NXADDRESS DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

#define COLS 2
#define CONTACTS "add"
#define INFO "add_info"
#define CATEGORY "add_category"
#define CUSTFIELDS "add_custfields"

#define NX_INIFILE "add_"

#define APP "Address Book"

#define	NXINPUT_H		21

extern int exit_flag;

about about_address = {
    "About Address Book",
    "(c) 2001, Century Software.",
    "jeffm@censoft.com",
    "08/24/01",
    "1.0"
};

////////////////////////////////////////////////////////////////////////////////
// Database Table Definitions

// Field References
#define RECNO 0
#define CAT 1
#define SHOW 2
#define LASTNAME 3
#define FIRSTNAME 4
#define COMPANY 5
#define TITLE 6
#define DEP1ID 7
#define DEP2ID 8
#define DEP3ID 9
#define DEP4ID 10
#define DEP5ID 11
#define DEP6ID 12
#define DEP7ID 13
#define DEP1 14
#define DEP2 15
#define DEP3 16
#define DEP4 17
#define DEP5 18
#define DEP6 19
#define DEP7 20
#define ADDRESS 21
#define CITY 22
#define REGION 23		// State, Province, Town, etc.
#define POSTALCODE 24		// ZIP, Postal Code, etc.
#define COUNTRY 25
#define BDAY 26
#define ANNIV 27
#define CUST1 28
#define CUST2 29
#define CUST3 30
#define CUST4 31
#define NOTE 32

// Contacts Fields
field cFields[] = {
    {'i', 1, 0},		// Field 0:RECNO
    {'i', 1, 0},		//       1:CAT
    {'i', 1, 0},		//       3:SHOW
    {'c', TEXT, 0},		//       4:LASTNAME
    {'c', TEXT, 0},		//       5:FIRSTNAME
    {'c', TEXT, 0},		//       6:COMPANY
    {'c', TEXT, 0},		//       7:TITLE
    {'i', 1, 0},		//       8:DEP1ID
    {'i', 1, 0},		//       9:DEP2ID
    {'i', 1, 0},		//      10:DEP3ID
    {'i', 1, 0},		//      11:DEP4ID
    {'i', 1, 0},		//      12:DEP5ID
    {'i', 1, 0},		//      13:DEP6ID
    {'i', 1, 0},		//      14:DEP7ID
    {'c', TEXT, 0},		//      15:DEP1
    {'c', TEXT, 0},		//      16:DEP2
    {'c', TEXT, 0},		//      17:DEP3
    {'c', TEXT, 0},		//      18:DEP4
    {'c', TEXT, 0},		//      19:DEP5
    {'c', TEXT, 0},		//      20:DEP6
    {'c', TEXT, 0},		//      21:DEP7
    {'c', DBL_TEXT, 0},		//      22:ADDRESS
    {'c', TEXT, 0},		//      23:CITY
    {'c', TEXT, 0},		//      24:REGION
    {'c', TEXT, 0},		//      25:POSTALCODE
    {'c', TEXT, 0},		//      26:COUNTRY
    {'c', DATE, 0},		//      27:BDAY
    {'c', DATE, 0},		//      28:ANNIV
    {'c', TEXT, 0},		//      29:CUST1
    {'c', TEXT, 0},		//      30:CUST2
    {'c', TEXT, 0},		//      31:CUST3
    {'c', TEXT, 0},		//      32:CUST4
    {'c', NOTEDB, 0},		//      33:NOTE
    {0}
};

// Database
fildes cFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			// extension
    33,				// nfields
    &cFields[0]			// fieldlist
};

#define INFO_TYPE 10

// Info
field iFields[] = {
    {'i', 1, 0},		// Field 0:infoid
    {'c', INFO_TYPE, 0},	//       1:info_type
    {0}
};

// Database
fildes iFile = {
    0, 0, 0,
    "dbf",
    2,
    &iFields[0]
};

#define CAT_NAME 10

// Category List
field catFields[] = {
    {'i', 1, 0},		// Field 0:catid
    {'c', CAT_NAME, 0},		//       1:cat_name
    {0}
};

// Database
fildes catFile = {
    0, 0, 0,
    "dbf",
    2,
    &catFields[0]
};

#define CUSTOM_NAME 10

// Custom Fields
field custFields[] = {
    {'i', 1, 0},		// Field 0: custid
    {'c', CUSTOM_NAME, 0},	//       1: custom name
    {0}
};

// Database
fildes custFile = {
    0, 0, 0,
    "dbf",
    2,
    &custFields[0]
};

////////////////////////////////////////////////////////////////////////////////
// Menus Definitions

Fl_Menu_Item addrMenuItems[] = {

    {"Record", 0, 0, 0, FL_SUBMENU},
    {"Duplicate Contact", 0, NxAddress::dup_callback, 0, FL_MENU_DIVIDER},
    {"Exit Address Book", 0, NxAddress::exit_callback},
    {0},

    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback},
    //{ "Keyboard",0, NxApp::keyboard_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //  { "Font"},
    //  { "Preferences"},
    {"Search", 0, NxAddress::lookup_callback},
    {"About Address Book", 0, NxApp::show_about},
    {0},

    {0}
};

Fl_Menu_Item addrViewMenuItems[] = {

    {"Record", 0, 0, 0, FL_SUBMENU},
    {"Delete Contact", 0, NxAddress::delView_callback},
    {"Duplicate Contact", 0, NxAddress::dup_callback, 0, FL_MENU_DIVIDER},
    {"Exit Address Book", 0, NxAddress::exit_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //  { "Font"},
    //  { "Rename Custom Fields"},
    {"About Address Book", 0, NxApp::show_about},
    {0},

    {0},

};

Fl_Menu_Item addrEditMenuItems[] = {

    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER},
    //  { "Select All"},
    //{ "Keyboard", 0, NxApp::keyboard_callback},
    {"Exit Address Book", 0, NxAddress::exit_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //  { "Font"},
    {"Contact Details", 0, NxAddress::details_callback},
    {"Rename Custom Fields", 0, NxAddress::custom_callback},
    {"About Address Book", 0, NxApp::show_about},
    {0},

    {0},

};

////////////////////////////////////////////////////////////////////////////////
// Static Data Members

NxDb *
    NxAddress::db;
int
    NxAddress::id;
int
    NxAddress::g_EditFlag;
int
    NxAddress::g_SearchFlag;
char *
    NxAddress::nx_inidir;

NxWindow *
    NxAddress::main_window;

NxPimWindow *
    NxAddress::addr_list_window;
NxPimWindow *
    NxAddress::addr_view_window;
NxPimWindow *
    NxAddress::addr_edit_window;

NxPimPopWindow *
    NxAddress::addr_details_window;
NxPimPopWindow *
    NxAddress::addr_note_window;
NxPimPopWindow *
    NxAddress::addr_dellist_window;
NxPimPopWindow *
    NxAddress::addr_deledit_window;
NxPimPopWindow *
    NxAddress::addr_delview_window;
NxPimPopWindow *
    NxAddress::addr_lookup_window;
NxPimPopWindow *
    NxAddress::addr_results_window;
NxPimPopWindow *
    NxAddress::addr_custom_window;

NxCategoryList *
    NxAddress::note_category;
NxCategoryList *
    NxAddress::edit_category_list;
NxCategoryList *
    NxAddress::view_category_list;

Fl_Editor *
    NxAddress::g_editor;
NxInput *
    NxAddress::lookup_input;
Flv_Table_Child *
    NxAddress::results_table;
NxOutput *
    NxAddress::results_message;
Flv_Table_Child *
    NxAddress::table;
NxCategoryList *
    NxAddress::cat_list[CAT_NUM];

bool NxAddress::AllFlag;

// edit window
NxInput *
    NxAddress::edit_lastname;
NxInput *
    NxAddress::edit_firstname;
NxInput *
    NxAddress::edit_company;
NxInput *
    NxAddress::edit_title;
NxMiscList *
    NxAddress::edit_misc_list1;
NxMiscList *
    NxAddress::edit_misc_list2;
NxMiscList *
    NxAddress::edit_misc_list3;
NxMiscList *
    NxAddress::edit_misc_list4;
NxMiscList *
    NxAddress::edit_misc_list5;
NxMiscList *
    NxAddress::edit_misc_list6;
NxMiscList *
    NxAddress::edit_misc_list7;
NxInput *
    NxAddress::edit_misc1;
NxInput *
    NxAddress::edit_misc2;
NxInput *
    NxAddress::edit_misc3;
NxInput *
    NxAddress::edit_misc4;
NxInput *
    NxAddress::edit_misc5;
NxInput *
    NxAddress::edit_misc6;
NxInput *
    NxAddress::edit_misc7;
NxInput *
    NxAddress::editAddress;
NxInput *
    NxAddress::editCity;
NxInput *
    NxAddress::editRegion;
NxInput *
    NxAddress::editPostalCode;
NxInput *
    NxAddress::editCountry;
NxInput *
    NxAddress::edit_bday;
NxInput *
    NxAddress::edit_anniv;
NxInput *
    NxAddress::edit_custom1;
NxInput *
    NxAddress::edit_custom2;
NxInput *
    NxAddress::edit_custom3;
NxInput *
    NxAddress::edit_custom4;
char *
    NxAddress::szNoteFile;
NxMiscList *
    NxAddress::details_show_list;

// view window
NxOutput *
    NxAddress::viewName;
NxOutput *
    NxAddress::viewBusTitle;
NxOutput *
    NxAddress::viewCompany;
NxOutput *
    NxAddress::viewAddress;
NxOutput *
    NxAddress::viewCityRegionPC;
NxOutput *
    NxAddress::viewCountry;
NxOutput *
    NxAddress::viewWorkPhone;
NxOutput *
    NxAddress::viewEMail;

// custom window
NxInput *
    NxAddress::custom1Input;
NxInput *
    NxAddress::custom2Input;
NxInput *
    NxAddress::custom3Input;
NxInput *
    NxAddress::custom4Input;

//NxMiscList * NxAddress::details_misc_list;
Fl_Pixmap *
    NxAddress::address_pixmap;

////////////////////////////////////////////////////////////////////////////////
// Constructor

NxAddress::NxAddress(int argc, char *argv[])
    :
NxApp(APP)
{

    db = new NxDb(argc, argv);
    optind = 1;
    NxApp::Instance()->set_keyboard(argc, argv);
    nx_inidir = db->GetPath();
    szNoteFile = new char[NOTEDB];
    strcpy(szNoteFile, "^");
    NxApp::Instance()->set_about(about_address);
    g_EditFlag = 0;

    // Open or Create contacts database
    if (!db->Open((string) CONTACTS, &cFile, cFields, 0)) {
	if (db->Create((string) CONTACTS, &cFile, cFields, 0)) {
	    if (!db->Open((string) CONTACTS, &cFile, cFields, 0)) {
		exit(-1);
	    }

	    /* Install some sample records if we have selected as such */

#ifdef CONFIG_SAMPLES

	    for (int i = 0; i < 1; i++) {

		char *record = 0;
		record = Record(i,	// Id
				0,	// CategoryId
				0,	// ShowDisplayId
				"Baggins",	// Last Name
				"Frodo ",	// First Name
				"Middle Earth, Inc.",	// Company Name
				"CEO",	// Title
				0, 1, 2, 3, 4, 5, 6,	// Dep Indexes
				"555.555.0001",	// Home Dep
				"555.555.0002 x123",	// Work Dep
				"555.555-0003",	// Fax Dep
				"555.555.0004",	// Mobile Dep
				"555.555.0005",	// Pager Dep
				"frodo@middleearth.net",	// E-Mail Dep
				"http://onering.net",	// Web Page
				"Bagend #1",	// Address
				"Hobbitten",	// City
				"Shire",	// Region
				"00001",	// Postal Code
				"Middele Earth",	// Country
				"04/04/1940",	// Birthday
				"06/06/1960",	// Anniversary
				"Custom Value",	// Custom Value 1
				"Custom Value",	// Custom Value 2
				"Custom Value",	// Custom Value 3
				"Custom Value",	// Custom Value 4
				"^");	// Note File Name (^ = empty)

		db->Insert(CONTACTS, record);
		delete[]record;

	    }
#endif
	} else {

	    exit(-1);

	}

    }
    // Database opened or created successfully, get number of records.
    //int recno = db->NumRecs(CONTACTS);
    //id = recno;
    //if ( recno > 1) {

    //  char buf[4];
    //  db->Extract(CONTACTS, recno, 0, buf);
    //  id = atoi(buf);

    //}

    id = GetKey(db, CONTACTS, RECNO);

    // Register DB for Synchronization
    //
    int file_fields[1];

    file_fields[0] = 32;
    SyncRegisterDB(db, cFields, CONTACTS, 1, file_fields, 1);

    // Open or Create info database
    if (!db->Open((string) INFO, &iFile, iFields, 0)) {

	if (db->Create((string) INFO, &iFile, iFields, 0)) {

	    if (!db->Open((string) INFO, &iFile, iFields, 0)) {
		exit(-1);
	    }

	    char *record1 = 0;
	    record1 = Record(0, "Home", 0);
	    db->Insert(INFO, record1);
	    delete[]record1;

	    char *record2 = 0;
	    record2 = Record(1, "Work", 0);
	    db->Insert(INFO, record2);
	    delete[]record2;

	    char *record3 = 0;
	    record3 = Record(2, "Fax", 0);
	    db->Insert(INFO, record3);
	    delete[]record3;

	    char *record4 = 0;
	    record4 = Record(3, "Mobile", 0);
	    db->Insert(INFO, record4);
	    delete[]record4;

	    char *record5 = 0;
	    record5 = Record(4, "Pager", 0);
	    db->Insert(INFO, record5);
	    delete[]record5;

	    char *record6 = 0;
	    record6 = Record(5, "E-Mail", 0);
	    db->Insert(INFO, record6);
	    delete[]record6;

	    char *record7 = 0;
	    record7 = Record(6, "Web", 0);
	    db->Insert(INFO, record7);
	    delete[]record7;

	} else {

	    exit(-1);

	}

    }
    // Register DB for Synchronization

    /* FIXME:  Temporarly commented out since crappy sync
       can't handle it (yet) */

    //SyncRegisterDB(db, iFields, INFO, 3, NULL, 0);

    // Open or Create category database
    if (!db->Open((string) CATEGORY, &catFile, catFields, 0)) {

	if (db->Create((string) CATEGORY, &catFile, catFields, 0)) {

	    if (!db->Open((string) CATEGORY, &catFile, catFields, 0)) {
		exit(-1);
	    }

	    char *record1 = 0;
	    record1 = Record(0, "Unfiled");
	    db->Insert(CATEGORY, record1);
	    delete[]record1;

	    char *record2 = 0;
	    record2 = Record(1, "Business");
	    db->Insert(CATEGORY, record2);
	    delete[]record2;

	    char *record3 = 0;
	    record3 = Record(2, "Personal");
	    db->Insert(CATEGORY, record3);
	    delete[]record3;

	} else {

	    exit(-1);

	}

    }
    // Register DB for Synchronization
    /* FIXME: Same thing - sync can't handle this DB yet */
    //SyncRegisterDB(db, catFields, CATEGORY, 0);

// Op en or Create custom fields database
    if (!db->Open((string) CUSTFIELDS, &custFile, custFields, 0)) {

	if (db->Create((string) CUSTFIELDS, &custFile, custFields, 0)) {

	    if (!db->Open((string) CUSTFIELDS, &custFile, custFields, 0)) {
		exit(-1);
	    }

	    char *record1 = 0;
	    record1 = CustRecord(0, "Custom1");
	    db->Insert(CUSTFIELDS, record1);
	    delete[]record1;

	    char *record2 = 0;
	    record2 = CustRecord(1, "Custom2");
	    db->Insert(CUSTFIELDS, record2);
	    delete[]record2;

	    char *record3 = 0;
	    record3 = CustRecord(2, "Custom3");
	    db->Insert(CUSTFIELDS, record3);
	    delete[]record3;

	    char *record4 = 0;
	    record4 = CustRecord(3, "Custom4");
	    db->Insert(CUSTFIELDS, record4);
	    delete[]record4;

	} else {

	    exit(-1);

	}

    }
    // Register DB for Synchronization

    /* FIXME:  Ditto, ditto */
    //SyncRegisterDB(db, custFields, CUSTFIELDS, 2);

    g_PasteTarget = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // Instantiate ALL Windows

    main_window = new NxWindow(W_W, W_H, APP);
    make_list_window();
    make_edit_window();
    make_view_window();
    make_details_window();
    make_note_window();
    make_dellist_window();
    make_deledit_window();
    make_delview_window();
    make_lookup_window();
    make_results_window();
    make_custom_window();
    main_window->end();

    note_category = addr_list_window->category_list;
    note_category->select(category_callback);
    edit_category_list = addr_edit_window->category_list;
    edit_category_list->select(list_callback);
    view_category_list = addr_view_window->category_list;
    view_category_list->select(list_callback);

    address_pixmap = new Fl_Pixmap(address_xpm);
    table->set_image(address_pixmap, -1, 0, 1);

    cat_list[0] = note_category;
    cat_list[1] = edit_category_list;
    cat_list[2] = view_category_list;
    fill_categories();

    add_items(table, "All");
    set_catlist_window((Fl_Window *) addr_list_window->
		       GetEditCategoryWindowPtr());

    ////////////////////////////////////////////////////////////////////////////////
    // FLNX-Colosseum IPC

    set_shown_window(addr_list_window->GetWindowPtr());

#ifdef CONFIG_COLOSSEUM
    Add_Fd("nxaddress", _ClientIPCHandler);
#else
    ExecuteShow();
#endif
}

NxAddress::~NxAddress()
{
    db->Close(CONTACTS);
    db->Close(INFO);
    db->Close(CATEGORY);
    delete db;
    db = 0;
}

void
NxAddress::Refresh()
{
    set_category(view_category_list->label());
}

char *
NxAddress::strup(const char *str1, int size)
{

    char *str2 = new char[size + 1];
    memset(str2, 0, size);

    char c;

    for (int i = 0; i < size + 1; i++) {
	c = str1[i];
	str2[i] = (char) toupper(c);
    }

    return str2;

}

int
NxAddress::compar(NxTodo ** rec1, NxTodo ** rec2)
{

    NxTodo *record1 = *rec1;
    NxTodo *record2 = *rec2;

    char *cmpStr1 = strup(record1->szLastName, TEXT);
    char *cmpStr2 = strup(record2->szLastName, TEXT);

    ////////////////////////////////////////////////////////////////////////////////
    // If both lastnames exist and both lastnames are equal
    // compare off firstname.

    if ((!(cmpStr1[0] == 0) && !(cmpStr2[0] == 0))) {
	if (strcmp(cmpStr1, cmpStr2) == 0) {
	    cmpStr1 = strup(record1->szFirstName, TEXT);
	    cmpStr2 = strup(record2->szFirstName, TEXT);
	}
    }

    if (cmpStr1[0] == 0)
	cmpStr1 = strup(record1->szFirstName, TEXT);
    if (cmpStr1[0] == 0)
	cmpStr1 = strup(record1->szCompany, TEXT);

    if (cmpStr2[0] == 0)
	cmpStr2 = strup(record2->szFirstName, TEXT);
    if (cmpStr2[0] == 0)
	cmpStr2 = strup(record2->szCompany, TEXT);

    return (strcmp(cmpStr1, cmpStr2));

}

////////////////////////////////////////////////////////////////////////////////
// FLNX-Colosseum IPC Methods

#ifdef CONFIG_COLOSSEUM

void
NxAddress::ClientIPCHandler(int fd, void *o, int ipc_id)
{


    DPRINT("ClientIPCHandler(): ipc_id = %d, msg %s.\n", ipc_id, (char *) o);

    char *tokenMsg = new char[MAX_LENGTH];
    memset(tokenMsg, 0, MAX_LENGTH);
    char *passMsg = new char[MAX_LENGTH];
    memset(passMsg, 0, MAX_LENGTH);

    if (o == NULL) {

	int length = MAX_LENGTH - 1;

	printf("ClientIPCHandler(): Read_Fd() called.\n");
	DPRINT("ClientIPCHandler(): passMsg(%p), length(%p)\n", passMsg,
	       &length);
	DPRINT("ClientIPCHandler(): passMsg(%s), length(%i)\n", passMsg,
	       length);
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);
	printf
	    ("ClientIPCHandler(): And this is the message... %s, %d bytes.\n",
	     passMsg, length);

	if ((passMsg == NULL) || (passMsg[0] == 0))
	    return;
	else
	    strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	DPRINT("ClientIPCHandler(): Find Address Book ipcId.\n");
	ipc_id = NxApp::Instance()->Find_Fd("nxaddress");
	DPRINT
	    ("ClientIPCHandler(): ipc_id = %d, And this is the message... %s.\n",
	     ipc_id, passMsg);

    } else {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);

    }

    // Explode Message
    char *service = new char[MAX_LENGTH];
    char *msg_cmd = new char[MAX_LENGTH];
    char *data_item = new char[MAX_LENGTH];

    // SERVICE
    char *tmp = strtok(tokenMsg, TOKEN);
    if (NULL != tmp)
	strcpy(service, tmp);

    // MSG_CMD
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(data_item, tmp);

    DPRINT("ClientIPCHandler(): Expoding Message... %s, %s, %s.\n", service,
	   msg_cmd, data_item);

    if (strcmp(msg_cmd, "EXECUTE") == 0) {

	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	if (strcmp(data_item, "datesearch") == 0) {
	    char *msg = new char[MAX_LENGTH];
	    strcpy(msg, "nxaddress^ACK^DATA^search^");
	    DPRINT("*** Write_Fd\n");
	    NxApp::Instance()->Write_Fd(ipc_id, msg, strlen(msg));
	    delete[]msg;
	    return;
	}

	if (strcmp(data_item, "search") == 0) {

	    DPRINT("Searching %s, %s, %s\n", service, msg_cmd, data_item);

	    char *searchStr = new char[MAX_LENGTH];
	    char *width = new char[4];

	    char *data = strtok(NULL, TOKEN);
	    strcpy(searchStr, data);
	    data = strtok(NULL, TOKEN);
	    strcpy(width, data);

	    ExecuteSearch(ipc_id, searchStr, atoi(width));

	    delete[]searchStr;
	    delete[]width;
	    searchStr = width = NULL;

	}

	if (strcmp(data_item, "showrecord") == 0) {

	    int recno = 0;
	    char *data = strtok(NULL, TOKEN);

	    DPRINT("showrecord: data[%s]\n", data);
	    recno = atoi(data);
	    DPRINT("^^^^ start viewRecord\n");
	    viewRecord(recno);
	    DPRINT("^^^^ pass viewRecord\n");
	}

    }
    // Memory Mangement
    delete[]service;
    delete[]msg_cmd;
    delete[]data_item;
    delete[]tokenMsg;
    service = msg_cmd = data_item = tokenMsg = NULL;

    NxApp::Instance()->ServerIPCHandler(fd, ipc_id, (char *) passMsg);

    delete[]passMsg;
    passMsg = NULL;

}
#endif /* CONFIG_COLOSSEUM */

/* IPC is required to do a search */

#ifdef CONFIG_COLOSSEUM

void
NxAddress::ExecuteSearch(int ipc_id, char *searchStr, int width)
{

    if (searchStr == NULL)
	return;

    DPRINT("ExecuteSearch has now been started: \"%s\"\n", searchStr);

    char *searchString = new char[MAX_LENGTH];
    strcpy(searchString, searchStr);

    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = search(searchString)) != NULL) {

	char *result = formatString(note, width);

	// Send DATA message to client
	// (e.g. "nxaddress^DATA^1^Century *Software* is neato^")

	DPRINT("ExecuteSearch has found a result.\n");

	char szId[4];
	strcpy(msg, "nxaddress^DATA^search^");
	sprintf(szId, "%d", note->nId);
	strcat(msg, szId);
	strcat(msg, "^");
	strcat(msg, result);
	strcat(msg, "^");

	DPRINT
	    ("Write_Fd ipc_id: %dnxaddress^DATA^search^ ... ExecuteSearch... %s\n",
	     ipc_id, msg);
	NxApp::Instance()->Write_Fd(ipc_id, msg, strlen(msg));

    }

    strcpy(msg, "nxaddress^ACK^DATA^search^");
    DPRINT("Write_Fd nxaddress^ACK^DATA^search^\n");
    NxApp::Instance()->Write_Fd(ipc_id, msg, strlen(msg));

    delete[]msg;
    delete[]searchString;
    msg = searchString = NULL;

}

#endif /* CONFIG_COLOSSEUM */


void
NxAddress::exit_callback(Fl_Widget * fl, void *l)
{
    exit_flag = 1;
    NxApp::Instance()->hide_all_windows();
    main_window->hide();
}

////////////////////////////////////////////////////////////////////////////////
// Database Methods

char *
NxAddress::Record(int id, int cat_id, int show_id,
		  string last_name, string first_name, string company,
		  string title, int dep1id, int dep2id, int dep3id,
		  int dep4id, int dep5id, int dep6id, int dep7id, string dep1,
		  string dep2, string dep3, string dep4, string dep5,
		  string dep6, string dep7, string address, string city,
		  string region, string postalCode, string country,
		  string bday, string anniv, string custom1, string custom2,
		  string custom3, string custom4, string note)
{

    char *rec = new char[MAXRECSIZ];
    memset(rec, 0, MAXRECSIZ);

    //char szId[ID];
    //char szCategory[ID];
    //sprintf(szId, "%d", id);
    //sprintf(szCategory, "%d", cat_id);
    //strcpy (&rec[cFields[RECNO].offset], szId);
    //strcpy (&rec[cFields[CAT].offset], szCategory);

    put16(&rec[cFields[RECNO].offset], id);
    put16(&rec[cFields[CAT].offset], cat_id);
    put16(&rec[cFields[SHOW].offset], show_id);
    strcpy(&rec[cFields[LASTNAME].offset], last_name.c_str());
    strcpy(&rec[cFields[FIRSTNAME].offset], first_name.c_str());
    strcpy(&rec[cFields[COMPANY].offset], company.c_str());
    strcpy(&rec[cFields[TITLE].offset], title.c_str());
    put16(&rec[cFields[DEP1ID].offset], dep1id);
    put16(&rec[cFields[DEP2ID].offset], dep2id);
    put16(&rec[cFields[DEP3ID].offset], dep3id);
    put16(&rec[cFields[DEP4ID].offset], dep4id);
    put16(&rec[cFields[DEP5ID].offset], dep5id);
    put16(&rec[cFields[DEP6ID].offset], dep6id);
    put16(&rec[cFields[DEP7ID].offset], dep7id);
    strcpy(&rec[cFields[DEP1].offset], dep1.c_str());
    strcpy(&rec[cFields[DEP2].offset], dep2.c_str());
    strcpy(&rec[cFields[DEP3].offset], dep3.c_str());
    strcpy(&rec[cFields[DEP4].offset], dep4.c_str());
    strcpy(&rec[cFields[DEP5].offset], dep5.c_str());
    strcpy(&rec[cFields[DEP6].offset], dep6.c_str());
    strcpy(&rec[cFields[DEP7].offset], dep7.c_str());
    strcpy(&rec[cFields[ADDRESS].offset], address.c_str());
    strcpy(&rec[cFields[CITY].offset], city.c_str());
    strcpy(&rec[cFields[REGION].offset], region.c_str());
    strcpy(&rec[cFields[POSTALCODE].offset], postalCode.c_str());
    strcpy(&rec[cFields[COUNTRY].offset], country.c_str());
    strcpy(&rec[cFields[BDAY].offset], bday.c_str());
    strcpy(&rec[cFields[ANNIV].offset], anniv.c_str());
    strcpy(&rec[cFields[CUST1].offset], custom1.c_str());
    strcpy(&rec[cFields[CUST2].offset], custom2.c_str());
    strcpy(&rec[cFields[CUST3].offset], custom3.c_str());
    strcpy(&rec[cFields[CUST4].offset], custom4.c_str());

    //cout << "Record(): note = " << note << endl;

    strcpy(&rec[cFields[NOTE].offset], note.c_str());

    return rec;

}

char *
NxAddress::Record(int infoid, string info_type, int dummy)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);
    put16(&rec[iFields[0].offset], infoid);
    strcpy(&rec[iFields[1].offset], info_type.c_str());

    return rec;

}

char *
NxAddress::Record(int catid, string cat_name)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);
    put16(&rec[catFields[0].offset], catid);
    strcpy(&rec[catFields[1].offset], cat_name.c_str());

    return rec;

}

char *
NxAddress::CustRecord(int custid, string cust_name)
{
    char *rec = new char[MAXRECSIZ];
    memset(rec, 0, MAXRECSIZ);
    put16(&rec[custFields[0].offset], custid);
    strcpy(&rec[custFields[1].offset], cust_name.c_str());

    return rec;
}

////////////////////////////////////////////////////////////////////////////////
// Window Methods

Fl_Window *
NxAddress::get_main_window()
{
    if (main_window)
	return main_window;
    else
	return 0;
}

void
NxAddress::show_default_window()
{
    show_window(addr_list_window->GetWindowPtr());
}

void
NxAddress::make_list_window()
{

    addr_list_window = new NxPimWindow(APP_NAME, addrMenuItems, db, CATEGORY,
				       CONTACTS,
				       (void (*)(const char *)) set_category);

    add_window((Fl_Window *) addr_list_window->GetWindowPtr());

    {

	NxBox *o = new NxBox(-1, 30, W_W + 2, BUTTON_Y - 32);
	o->movable(false);
	o->box(FL_BORDER_BOX);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	addr_list_window->add((Fl_Widget *) o);

	table = new Flv_Table_Child(0, 31, W_W, BUTTON_Y - 40, 0, 50, 51);
	table->movable(false);
	table->callback(view_callback);
	table->SetCols(COLS);
	addr_list_window->add((Fl_Widget *) table);
    }

    {
	NxBox *o = new NxBox(5, 5, 85, 25, "");
	addr_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Add");
	o->callback(new_callback);
	addr_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Edit");


	o->callback(edit_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	addr_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 126, BUTTON_Y, BUTTON_WIDTH,
				   BUTTON_HEIGHT, "Delete");


	o->callback(delList_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	addr_list_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_view_window()
{

    addr_view_window =
	new NxPimWindow(APP_NAME, addrViewMenuItems, db, CATEGORY, CONTACTS,
			(void (*)(const char *)) set_category);

    add_window((Fl_Window *) addr_view_window->GetWindowPtr());

    int x = 0;
    int y = MENU_H + 17;

    {
	// First and Last Name
	viewName = new NxOutput(x, y, W_W - 5, 25, "");
	viewName->box(FL_FLAT_BOX);
	NxApp::Instance()->big_font(viewName);
	addr_view_window->add((Fl_Widget *) viewName);
    }

    {
	// Title
	viewBusTitle = new NxOutput(x, y += 25, W_W - 5, 15, "");
	viewBusTitle->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewBusTitle);
    }

    {
	// Company
	viewCompany = new NxOutput(x, y += 20, W_W - 5, 15, "");
	viewCompany->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewCompany);
    }

    {
	// Street Address
	viewAddress = new NxOutput(x, y += 20, W_W - 5, 15, "");
	viewAddress->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewAddress);
    }

    // City, Region Postal Code
    {
	viewCityRegionPC = new NxOutput(x, y += 20, W_W - 5, 15, "");
	viewCityRegionPC->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewCityRegionPC);
    }

    // Country
    {
	viewCountry = new NxOutput(x, y += 20, W_W - 5, 15, "");
	viewCountry->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewCountry);
    }

    {
	// Work Phone Number
	viewWorkPhone = new NxOutput(x, y += 30, W_W - 93, 15, "");
	viewWorkPhone->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewWorkPhone);
    }

    {
	// E-Mail Address
	viewEMail = new NxOutput(x, y += 20, W_W - 5, 15, "");
	viewEMail->box(FL_FLAT_BOX);
	addr_view_window->add((Fl_Widget *) viewEMail);
    }

    {
	// Done Button
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Done");


	o->callback(doneView_callback);

	addr_view_window->add((Fl_Widget *) o);
    }

    {
	// Edit Button
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Edit");


	o->callback(edit_callback);

	addr_view_window->add((Fl_Widget *) o);
    }

    {
	// Delete Button
	NxButton *o = new NxButton(BUTTON_X + 126, BUTTON_Y, BUTTON_WIDTH,
				   BUTTON_HEIGHT, "Delete");


	o->callback(delView_callback);

	addr_view_window->add((Fl_Widget *) o);
    }

}


void
NxAddress::make_edit_window()
{

    addr_edit_window =
	new NxPimWindow(APP_NAME, addrEditMenuItems, db, CATEGORY, CONTACTS,
			(void (*)(const char *)) set_category);

    add_window((Fl_Window *) addr_edit_window->GetWindowPtr());

    {
	int x = 77;
	int y = 31;
	NxScroll *o = new NxScroll(-1, y, W_W + 2, BUTTON_Y - 33);
	o->movable(false);

	edit_lastname = new NxInput(x, y, W_W - 93, NXINPUT_H, "Last Name");
	edit_firstname = new NxInput(x, y +=
				     20, W_W - 93, NXINPUT_H, "First Name");
	edit_company = new NxInput(x, y +=
				   20, W_W - 93, NXINPUT_H, "Company");
	edit_title = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "Title");

	// info1
	{
	    NxMiscList *o = edit_misc_list1 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc1 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info2
	{
	    NxMiscList *o = edit_misc_list2 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc2 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info3
	{
	    NxMiscList *o = edit_misc_list3 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc3 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info4
	{
	    NxMiscList *o = edit_misc_list4 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc4 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info5
	{
	    NxMiscList *o = edit_misc_list5 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc5 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info6
	{
	    NxMiscList *o = edit_misc_list6 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc6 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	// info7
	{
	    NxMiscList *o = edit_misc_list7 =
		new NxMiscList(2, y + 22, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	    o->value(0);
	    edit_misc7 = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "");
	}

	editAddress = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "Address");
	editCity = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "City");
	editRegion = new NxInput(x, y +=
				 20, W_W - 93, NXINPUT_H, "State/Prov");
	editPostalCode = new NxInput(x, y +=
				     20, W_W - 93, NXINPUT_H, "Zip/Postal");
	editCountry = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "Country");
	edit_bday = new NxInput(x, y += 20, W_W - 93, NXINPUT_H, "Birthday");
	edit_anniv = new NxInput(x, y +=
				 20, W_W - 93, NXINPUT_H, "Anniversary");
	edit_custom1 = new NxInput(x, y +=
				   20, W_W - 93, NXINPUT_H, "Custom1");
	edit_custom2 = new NxInput(x, y +=
				   20, W_W - 93, NXINPUT_H, "Custom2");
	edit_custom3 = new NxInput(x, y +=
				   20, W_W - 93, NXINPUT_H, "Custom3");
	edit_custom4 = new NxInput(x, y +=
				   20, W_W - 93, NXINPUT_H, "Custom4");

	o->end();
	addr_edit_window->add((Fl_Widget *) o);

    }

    {
	NxButton *o =
	    new NxButton(1 + 2, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Done");

	o->callback(doneEdit_callback);

	addr_edit_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(60 + 2, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Delete");
	o->callback(delEdit_callback);
	addr_edit_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(119 + 2, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelEdit_callback);
	addr_edit_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(178 + 2, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Notes");
	o->callback(notes_callback);
	addr_edit_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_details_window()
{

    addr_details_window = new NxPimPopWindow("Contact Details");
    add_window((Fl_Window *) addr_details_window->GetWindowPtr());

    {
	NxOutput *o = new NxOutput(BUTTON_X, 38, 125, 20);
#ifdef NANOX
	o->box(FL_PDA_NO_BOX);
#else
	o->box(FL_NO_BOX);
#endif
	o->value("Show In List");
	addr_details_window->add((Fl_Widget *) o);
    }

    {
	NxMiscList *o = details_show_list =
	    new NxMiscList(100, 40, BUTTON_WIDTH + 6, BUTTON_HEIGHT);
	o->value(0);
	addr_details_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(doneDetails_callback);
	addr_details_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelDetails_callback);
	addr_details_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_note_window()
{

    int w_h = 175;
    addr_note_window = new NxPimPopWindow(5, (W_H / 4), W_W - 10, w_h);
    add_window((Fl_Window *) addr_note_window->GetWindowPtr());

    g_editor = new Fl_Editor(3, 3, W_W - 17, 146);
    g_editor->box(FL_BORDER_BOX);
    addr_note_window->add((Fl_Widget *) g_editor);

    {
	NxButton *o =
	    new NxButton(BUTTON_X, w_h - 23, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Done");
	o->callback(doneNote_callback);
	addr_note_window->add((Fl_Widget *) o);
    }
}


void
NxAddress::make_dellist_window()
{

    addr_dellist_window = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) addr_dellist_window->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current contact ?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	addr_dellist_window->add((Fl_Widget *) o);
    }


    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(yesDelList_callback);
	addr_dellist_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(noDelList_callback);
	addr_dellist_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_deledit_window()
{

    addr_deledit_window = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) addr_deledit_window->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current contact ?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	addr_deledit_window->add((Fl_Widget *) o);
    }


    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(yesDelEdit_callback);
	addr_deledit_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(noDelEdit_callback);
	addr_deledit_window->add((Fl_Widget *) o);
    }

}


void
NxAddress::make_delview_window()
{

    addr_delview_window = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) addr_delview_window->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current contact ?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	addr_delview_window->add((Fl_Widget *) o);
    }


    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(yesDelView_callback);
	addr_delview_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(noDelView_callback);
	addr_delview_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_lookup_window()
{

    addr_lookup_window = new NxPimPopWindow("Search");
    add_window((Fl_Window *) addr_lookup_window->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "What are you looking for?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	addr_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxInput *o = lookup_input =
	    new NxInput(BUTTON_X, 60, W_W - BUTTON_WIDTH, 20);
	addr_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Search");
	o->callback(searchLookup_callback);
	addr_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelLookup_callback);
	addr_lookup_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_results_window()
{


    addr_results_window = new NxPimPopWindow("Search Results",
					     NxApp::Instance()->
					     getGlobalColor(APP_FG), 5,
					     (W_W / 3), W_W - 10,
					     (W_H - (W_W / 2)));

    add_window((Fl_Window *) addr_results_window->GetWindowPtr());

    {
	results_message = new NxOutput(4, 19, W_W - 19, 25);
	results_message->value("Nothing Found.");
	results_message->hide();
	addr_results_window->add((Fl_Widget *) results_message);
    }

    {
	results_table =
	    new Flv_Table_Child(4, 19, (W_W - 19),
				(W_H - (W_W / 2) - 3 * (BUTTON_HEIGHT)), 0,
				(W_W - 25));
	results_table->callback(view_callback);
	results_table->SetCols(1);
	addr_results_window->add((Fl_Widget *) results_table);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneLookup_callback);
	addr_results_window->add((Fl_Widget *) o);
    }

}

void
NxAddress::make_custom_window()
{

    addr_custom_window = new NxPimPopWindow("Custom Fields",
					    NxApp::Instance()->
					    getGlobalColor(APP_FG), 5,
					    (W_W / 3), W_W - 10,
					    (W_H - (W_W / 2)));

    add_window((Fl_Window *) addr_custom_window->GetWindowPtr());

    // Title
    {
	NxMultilineOutput *o =
	    new NxMultilineOutput(BUTTON_X, 20, W_W - 25, 40, "");
	o->value("Rename the custom fields\nby editing the text below:");
	addr_custom_window->add((Fl_Widget *) o);
    }

    // Custom 1
    custom1Input = new NxInput(BUTTON_X + 3, 50, W_W - 93, 20, "");
    custom1Input->when(FL_WHEN_RELEASE_ALWAYS);
    custom1Input->callback(NxApp::Instance()->pasteTarget_callback);
    addr_custom_window->add((Fl_Widget *) custom1Input);

    // Custom 2
    custom2Input = new NxInput(BUTTON_X + 3, 70, W_W - 93, 20, "");
    custom2Input->when(FL_WHEN_RELEASE_ALWAYS);
    custom2Input->callback(NxApp::Instance()->pasteTarget_callback);
    addr_custom_window->add((Fl_Widget *) custom2Input);

    // Custom 3
    custom3Input = new NxInput(BUTTON_X + 3, 90, W_W - 93, 20, "");
    custom3Input->when(FL_WHEN_RELEASE_ALWAYS);
    custom3Input->callback(NxApp::Instance()->pasteTarget_callback);
    addr_custom_window->add((Fl_Widget *) custom3Input);

    // Custom 4
    custom4Input = new NxInput(BUTTON_X + 3, 110, W_W - 93, 20, "");
    custom4Input->when(FL_WHEN_RELEASE_ALWAYS);
    custom4Input->callback(NxApp::Instance()->pasteTarget_callback);
    addr_custom_window->add((Fl_Widget *) custom4Input);

    {
	NxButton *o =
	    new NxButton(BUTTON_X, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneCustom_callback);
	addr_custom_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	o->callback(cancelCustom_callback);
	addr_custom_window->add((Fl_Widget *) o);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Callbacks

void
NxAddress::new_callback(Fl_Widget * fl, long l)
{

    NxTodo *n = new NxTodo;

    // Indexes
    //memset(n->szId, 0, ID);
    n->nId = 0;
    n->nCat = 0;
    memset(n->szCategory, 0, CATEGORYS);
    n->nShowDisplay = 0;

    // Data
    memset(n->szLastName, 0, TEXT);
    memset(n->szFirstName, 0, TEXT);
    memset(n->szCompany, 0, TEXT);
    memset(n->szJobTitle, 0, TEXT);
    n->nDep1 = 0;
    n->nDep2 = 1;
    n->nDep3 = 2;
    n->nDep4 = 3;
    n->nDep5 = 4;
    n->nDep6 = 5;
    n->nDep7 = 6;
    memset(n->szDep1, 0, TEXT);
    memset(n->szDep2, 0, TEXT);
    memset(n->szDep3, 0, TEXT);
    memset(n->szDep4, 0, TEXT);
    memset(n->szDep5, 0, TEXT);
    memset(n->szDep6, 0, TEXT);
    memset(n->szDep7, 0, TEXT);
    memset(n->szAddress, 0, DBL_TEXT);
    memset(n->szCity, 0, TEXT);
    memset(n->szRegion, 0, TEXT);
    memset(n->szPostalCode, 0, TEXT);
    memset(n->szCountry, 0, TEXT);
    memset(n->szBDay, 0, DATE);
    memset(n->szAnniv, 0, DATE);
    memset(n->szCustom1, 0, TEXT);
    memset(n->szCustom2, 0, TEXT);
    memset(n->szCustom3, 0, TEXT);
    memset(n->szCustom4, 0, TEXT);
    memset(n->szNoteFile, 0, NOTEDB);

    NxApp::Instance()->set_catlist_window((Fl_Window *) addr_edit_window->
					  GetEditCategoryWindowPtr());

    set_szNoteFile("^");
    strcpy(n->szNoteFile, "^");

    strcpy(n->szCategory, note_category->label());

    edit_category_list->label(n->szCategory);

    _fill_form(n);

    g_EditFlag = 0;

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    //cout << "new_callback(): n->szNoteFile = " << n->szNoteFile << endl;

    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr());

}

void
NxAddress::select_note(NxTodo * note)
{
    int rows = table->rows();
    DPRINT("A\n");
    for (int idx = 0; idx <= rows; idx++) {
	DPRINT("B table[%p]\n", table);
	void *data = table->data(idx);
	DPRINT("B1 [%p]\n", data);
	NxTodo *n = (NxTodo *) data;
	DPRINT("C\n");
	//if(0 == strcmp(note->szId, n->szId)) {
	if (note->nId == n->nId) {
	    DPRINT("D\n");
	    table->row(idx);	// this is a select?
	    DPRINT("E\n");
	    break;
	}
    }
}

void
NxAddress::viewRecord(int recno)
{

    NxApp::Instance()->set_catlist_window((Fl_Window *) addr_view_window->
					  GetEditCategoryWindowPtr());

    NxTodo *note = new NxTodo;
    DPRINT("1\n");

    int rec_array[1];

    rec_array[0] = -1;
    char c_recno[8];
    sprintf(c_recno, "%d", recno);
    db->Select(CONTACTS, c_recno, RECNO, rec_array, 1);
    recno = rec_array[0];

    int catid_array[1];
    //char catid[8];
    char catid[4];
    char buf1[4];

    catid_array[0] = -1;

    db->Extract(CONTACTS, recno, CAT, catid);
    note->nCat = atoi(catid);

    db->Select(CATEGORY, catid, RECNO, catid_array, 1);

    if (-1 != catid_array[0])
	db->Extract(CATEGORY, catid_array[0], CAT, note->szCategory);
    else
	strcpy(note->szCategory, "Unfiled");

    DPRINT("2\n");

    //db->Extract(CONTACTS, recno, RECNO, note->szId);
    db->Extract(CONTACTS, recno, RECNO, buf1);
    note->nId = atoi(buf1);
    db->Extract(CONTACTS, recno, SHOW, buf1);
    note->nShowDisplay = atoi(buf1);
    db->Extract(CONTACTS, recno, LASTNAME, note->szLastName);
    db->Extract(CONTACTS, recno, FIRSTNAME, note->szFirstName);
    db->Extract(CONTACTS, recno, COMPANY, note->szCompany);
    db->Extract(CONTACTS, recno, TITLE, note->szJobTitle);
    db->Extract(CONTACTS, recno, DEP1ID, buf1);
    note->nDep1 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP2ID, buf1);
    note->nDep2 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP3ID, buf1);
    note->nDep3 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP4ID, buf1);
    note->nDep4 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP5ID, buf1);
    note->nDep5 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP6ID, buf1);
    note->nDep6 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP7ID, buf1);
    note->nDep7 = atoi(buf1);
    db->Extract(CONTACTS, recno, DEP1, note->szDep1);
    db->Extract(CONTACTS, recno, DEP2, note->szDep2);
    db->Extract(CONTACTS, recno, DEP3, note->szDep3);
    db->Extract(CONTACTS, recno, DEP4, note->szDep4);
    db->Extract(CONTACTS, recno, DEP5, note->szDep5);
    db->Extract(CONTACTS, recno, DEP6, note->szDep6);
    db->Extract(CONTACTS, recno, DEP7, note->szDep7);
    db->Extract(CONTACTS, recno, ADDRESS, note->szAddress);
    db->Extract(CONTACTS, recno, CITY, note->szCity);
    db->Extract(CONTACTS, recno, REGION, note->szRegion);
    db->Extract(CONTACTS, recno, POSTALCODE, note->szPostalCode);
    db->Extract(CONTACTS, recno, COUNTRY, note->szCountry);
    db->Extract(CONTACTS, recno, BDAY, note->szBDay);
    db->Extract(CONTACTS, recno, ANNIV, note->szAnniv);
    db->Extract(CONTACTS, recno, CUST1, note->szCustom1);
    db->Extract(CONTACTS, recno, CUST2, note->szCustom2);
    db->Extract(CONTACTS, recno, CUST3, note->szCustom3);
    db->Extract(CONTACTS, recno, CUST4, note->szCustom4);

    db->Extract(CONTACTS, recno, NOTE, note->szNoteFile);

    g_EditFlag = 1;
    g_SearchFlag = 0;
    DPRINT("2a\n");
    select_note(note);
    DPRINT("3\n");
    _fill_view_form(note);
    DPRINT("4\n");

    view_category_list->label(note->szCategory);
    DPRINT("5\n");
    NxApp::Instance()->show_window(addr_view_window->GetWindowPtr());
    DPRINT("6\n");
    delete note;
}

void
NxAddress::view_callback(Fl_Widget * fl, void *o)
{

    if (Fl::event_clicks()) {

	g_EditFlag = 1;

	NxApp::Instance()->set_catlist_window((Fl_Window *) addr_view_window->
					      GetEditCategoryWindowPtr());

	NxTodo *n = 0;

	if (g_SearchFlag) {

	    n = (NxTodo *) results_table->selected();

	} else {

	    n = (NxTodo *) table->selected();

	}

	view_category_list->label(n->szCategory);

	if (n) {
	    select_note(n);
	    _fill_view_form(n);
	}

	if (0 == strcmp("All", note_category->label()))
	    AllFlag = true;

	NxApp::Instance()->show_window(addr_view_window->GetWindowPtr());

    }

    Fl::event_clicks(0);	// Reset clicks to 0 so it does not hold one of the clicks.

}

void
NxAddress::edit_callback(Fl_Widget * fl, long l)
{

    NxApp::Instance()->set_catlist_window((Fl_Window *) addr_edit_window->
					  GetEditCategoryWindowPtr());

    NxTodo *n = 0;

    if (g_SearchFlag) {

	n = (NxTodo *) results_table->selected();

    } else {

	n = (NxTodo *) table->selected();

    }

    if (n == NULL)
	return;

    edit_category_list->label(n->szCategory);

    if (n)
	_fill_form(n);

    g_EditFlag = 1;

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    //cout << "edit_callback(): n->szNoteFile = " << n->szNoteFile << endl;

    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr());

}

void
NxAddress::delList_callback(Fl_Widget * fl, void *l)
{


    if (table->selected() == NULL)
	return;

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    NxApp::Instance()->show_window(addr_dellist_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_list_window->GetWindowPtr());
}

void
NxAddress::delEdit_callback(Fl_Widget * fl, void *l)
{
    g_SearchFlag = 0;

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    NxApp::Instance()->show_window(addr_deledit_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_edit_window->GetWindowPtr());
}

void
NxAddress::delView_callback(Fl_Widget * fl, void *l)
{
    g_SearchFlag = 0;

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    NxApp::Instance()->show_window(addr_delview_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_view_window->GetWindowPtr());
}

void
NxAddress::doneEdit_callback(Fl_Widget * fl, long l)
{

    int bDeleteMe = 0;

    NxTodo *n = 0;

    if (g_SearchFlag) {
	n = (NxTodo *) results_table->selected();
	g_SearchFlag = 0;
    } else if (g_EditFlag) {
	n = (NxTodo *) table->selected();
    }

    if (!n) {
	//cout << "doneEdit_callback(): !table->selected()\n";
	n = new NxTodo;
	strcpy(n->szNoteFile, get_szNoteFile());
	bDeleteMe = 1;
    } else
	//cout << "doneEdit_callback(): table->selected()\n";

	//cout << "doneEdit_callback(): n->szNoteFile = " << n->szNoteFile << endl;

	NxApp::Instance()->set_catlist_window((Fl_Window *) addr_list_window->
					      GetEditCategoryWindowPtr());

    strcpy(n->szCategory, edit_category_list->label());
    n->nCat = GetCatId(n->szCategory);

    if ((n->nCat == 0) || (strcmp(n->szCategory, "All") == 0))
	strcpy(n->szCategory, "Unfiled");

    // fill in note structure here
    strcpy(n->szFirstName, edit_firstname->value());
    strcpy(n->szLastName, edit_lastname->value());
    strcpy(n->szCompany, edit_company->value());
    strcpy(n->szJobTitle, edit_title->value());
    n->nDep1 = edit_misc_list1->value();
    n->nDep2 = edit_misc_list2->value();
    n->nDep3 = edit_misc_list3->value();
    n->nDep4 = edit_misc_list4->value();
    n->nDep5 = edit_misc_list5->value();
    n->nDep6 = edit_misc_list6->value();
    n->nDep7 = edit_misc_list7->value();
    strcpy(n->szDep1, edit_misc1->value());
    strcpy(n->szDep2, edit_misc2->value());
    strcpy(n->szDep3, edit_misc3->value());
    strcpy(n->szDep4, edit_misc4->value());
    strcpy(n->szDep5, edit_misc5->value());
    strcpy(n->szDep6, edit_misc6->value());
    strcpy(n->szDep7, edit_misc7->value());
    strcpy(n->szAddress, editAddress->value());
    strcpy(n->szCity, editCity->value());
    strcpy(n->szRegion, editRegion->value());
    strcpy(n->szPostalCode, editPostalCode->value());
    strcpy(n->szCountry, editCountry->value());
    strcpy(n->szBDay, edit_bday->value());
    strcpy(n->szAnniv, edit_anniv->value());
    strcpy(n->szCustom1, edit_custom1->value());
    strcpy(n->szCustom2, edit_custom2->value());
    strcpy(n->szCustom3, edit_custom3->value());
    strcpy(n->szCustom4, edit_custom4->value());

    if (g_EditFlag) {

	int recno[1];
	recno[0] = -1;
	char szId[4];
	sprintf(szId, "%d", n->nId);
	db->Select(CONTACTS, szId, RECNO, recno, 1);

	if (recno[0] != -1)
	    edit_note(n, recno[0]);

	g_EditFlag = 0;

    } else {

	//cout << "doneEdit_callback(): write_note(n)\n";
	write_note(n);

    }

    if (AllFlag) {
	set_category("All");
	AllFlag = false;
    } else
	set_category(n->szCategory);

    if (bDeleteMe)
	delete n;

    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr());

}

void
NxAddress::doneView_callback(Fl_Widget * fl, long l)
{

    NxTodo *n = 0;
    if (g_SearchFlag) {

	n = (NxTodo *) results_table->selected();
	set_category("All");
	g_SearchFlag = 0;

    } else {

	n = (NxTodo *) table->selected();
	if (AllFlag) {
	    set_category("All");
	    AllFlag = false;
	} else
	    set_category(view_category_list->label());
    }

    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr());
}

void
NxAddress::doneNote_callback(Fl_Widget * fl, long l)
{

    char tplate[128];
    int tfd = 0;
    FILE *fd = 0;

    NxTodo *n = (NxTodo *) table->selected();
    char *f = 0;

    snprintf(tplate, sizeof(tplate), "%s/%sXXXXXX", nx_inidir, NX_INIFILE);

    if (n && g_EditFlag) {

	f = n->szNoteFile;

	if (f[0] == '^') {
	    tfd = mkstemp(tplate);

	    if (tfd > 0) {
		strcpy(n->szNoteFile, tplate);
		fd = fdopen(tfd, "w+");
	    }
	} else
	    fd = fopen(f, "w+");
    } else {
	tfd = mkstemp(tplate);

	if (tfd > 0) {
	    set_szNoteFile(tplate);
	    fd = fdopen(tfd, "w+");
	}
    }

    if (fd) {
	g_editor->SaveTo(fd);
	fclose(fd);
    }

    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::category_callback(Fl_Widget * fl, void *l)
{
    set_category((char *) l);
}

void
NxAddress::list_callback(Fl_Widget * fl, void *l)
{
    reset_category((char *) l);
}

void
NxAddress::details_callback(Fl_Widget * fl, void *l)
{

    NxApp::Instance()->show_window(addr_details_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_edit_window->GetWindowPtr());
}

void
NxAddress::doneDetails_callback(Fl_Widget * fl, void *o)
{

    NxTodo *n = (NxTodo *) table->selected();
    n->nShowDisplay = details_show_list->value();

    int recno[1];
    recno[0] = -1;
    char szId[4];
    sprintf(szId, "%d", n->nId);
    db->Select(CONTACTS, szId, RECNO, recno, 1);

    if (recno[0] != -1)
	edit_note(n, recno[0]);

    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::cancelDetails_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);
}

/*
void NxAddress::delDetails_callback(Fl_Widget * fl, void *l) {
  // Delete current record
  NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
		       ACTIVATE);
}
*/

void
NxAddress::cancelEdit_callback(Fl_Widget * fl, void *l)
{
    g_SearchFlag = 0;
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr());
}

void
NxAddress::notes_callback(Fl_Widget * fl, void *l)
{

    g_editor->Clear();

    NxTodo *n = (NxTodo *) table->selected();
    char *f = 0;

    //cout << "notes_callback(): g_EditFlag = " << g_EditFlag << endl;
    if (n && g_EditFlag) {

	//cout << "notes_callback(): table->selected()\n";
	f = n->szNoteFile;

    } else {

	//cout << "notes_callback(): !table->selected()\n";
	f = get_szNoteFile();
    }

    //cout << "notes_callback(): f = " << f << endl;

    if (f[0] != '^') {

	FILE *fd = fopen(f, "r");

	if (fd) {
	    g_editor->LoadFrom(fd);
	    g_editor->MoveTo(0, 0);
	    fclose(fd);
	}

    }

    NxApp::Instance()->show_window(addr_note_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_edit_window->GetWindowPtr());
}

void
NxAddress::custom_callback(Fl_Widget * fl, void *o)
{

    int rec_array[4];
    char *custlabel1 = new char[CUSTOM_NAME];
    char *custlabel2 = new char[CUSTOM_NAME];
    char *custlabel3 = new char[CUSTOM_NAME];
    char *custlabel4 = new char[CUSTOM_NAME];

    for (int idx = 0; idx < 4; idx++) {
	rec_array[idx] = -1;
    }

    db->Select(CUSTFIELDS, rec_array, 4);

    if ((rec_array[0] >= 0)) {
	db->Extract(CUSTFIELDS, rec_array[0], 1, custlabel1);
	db->Extract(CUSTFIELDS, rec_array[1], 1, custlabel2);
	db->Extract(CUSTFIELDS, rec_array[2], 1, custlabel3);
	db->Extract(CUSTFIELDS, rec_array[3], 1, custlabel4);

	custom1Input->value(custlabel1);
	custom2Input->value(custlabel2);
	custom3Input->value(custlabel3);
	custom4Input->value(custlabel4);
    }

    NxApp::Instance()->show_window(addr_custom_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_edit_window->GetWindowPtr());
}


void
NxAddress::doneCustom_callback(Fl_Widget * fl, void *o)
{

    int recno[4];

    for (int i = 0; i < 4; i++)
	recno[i] = -1;

    db->Select(CUSTFIELDS, recno, 4);

    char buffer[4];
    int id;

    // Custom1
    db->Extract(CUSTFIELDS, recno[0], 0, buffer);
    id = atoi(buffer);
    char *record1 = CustRecord(id, custom1Input->value());
    db->Edit(CUSTFIELDS, recno[0], record1);
    edit_custom1->label(custom1Input->value());
    delete record1;

    // Custom2
    db->Extract(CUSTFIELDS, recno[1], 0, buffer);
    id = atoi(buffer);
    char *record2 = CustRecord(id, custom2Input->value());
    db->Edit(CUSTFIELDS, recno[1], record2);
    edit_custom2->label(custom2Input->value());
    delete record2;

    // Custom3
    db->Extract(CUSTFIELDS, recno[2], 0, buffer);
    id = atoi(buffer);
    char *record3 = CustRecord(id, custom3Input->value());
    db->Edit(CUSTFIELDS, recno[2], record3);
    edit_custom3->label(custom3Input->value());
    delete record3;

    // Custom4
    db->Extract(CUSTFIELDS, recno[3], 0, buffer);
    id = atoi(buffer);
    char *record4 = CustRecord(id, custom4Input->value());
    db->Edit(CUSTFIELDS, recno[3], record4);
    edit_custom4->label(custom4Input->value());
    delete record4;

    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);

}

void
NxAddress::cancelCustom_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);
}


char *
NxAddress::formatString(const NxTodo * note, int pixels)
{
    int width = 0;
    int dot_width = 0;
    int idx = 0;
    int jdx = 0;
    int num_len = 0;
    char *new_string = new char[DBL_TEXT + DBL_TEXT + TEXT + 3];
    char new_temp_string[DBL_TEXT + DBL_TEXT + TEXT + 3];
    char name_string[TEXT + TEXT + 3];
    char temp_name_string[TEXT + TEXT + 3];
    char num_string[TEXT + 3];
    char temp_string[TEXT + 3];

    dot_width = (int) fl_width("...");
    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    memset(name_string, 0, TEXT + TEXT + 3);
    sprintf(name_string, "%s,%s", note->szLastName, note->szFirstName);

    memset(new_string, 0, TEXT + TEXT + TEXT + 3);

    strcpy(temp_name_string, name_string);
    width = (int) fl_width(name_string);
    if (width >= (pixels / 2)) {
	while (width > (pixels / 2)) {
	    idx++;
	    memset(name_string, 0, TEXT + TEXT + 3);
	    strncpy(name_string, temp_name_string,
		    strlen(temp_name_string) - idx);
	    width = (int) fl_width(name_string) + dot_width;
	}
	sprintf(name_string, "%s...", name_string);
    } else {
	while (width < (pixels / 2)) {
	    if (strlen(name_string) >= TEXT + TEXT + 3)
		break;
	    sprintf(name_string, "%s ", name_string);
	    width = (int) fl_width(name_string);
	}
    }

    num_len = strlen(note->szDep1);
    memset(temp_string, 0, sizeof(temp_string));
    memset(num_string, 0, sizeof(num_string));

    width = (int) fl_width(note->szDep1);
    if (width >= (pixels / 2)) {
	sprintf(temp_string, "%*.*s", TEXT, num_len, note->szDep1);
	strcpy(num_string, temp_string);
    } else
	strcpy(num_string, note->szDep1);

    idx = 0;
    jdx = 0;
    width = (int) fl_width(num_string);
    while (width >= (pixels / 2)) {
	if (isspace(num_string[0])) {
	    idx++;
	    memmove(num_string, num_string + 1, TEXT + 3 - idx);
	    width = (int) fl_width(num_string);
	} else {
	    jdx++;
	    memset(num_string, 0, sizeof(num_string));
	    strncpy(num_string, note->szDep1, strlen(note->szDep1) - jdx);
	    width = (int) fl_width(num_string) + dot_width;
	}
    }

    if (jdx != 0)
	sprintf(num_string, "%s...", num_string);

    sprintf(new_string, "%s%s", name_string, num_string);

    char *pStr = strstr(new_string, num_string);
    int len = strlen(new_string) - strlen(pStr);

    memset(new_temp_string, 0, sizeof(new_temp_string));
    strncpy(new_temp_string, new_string, len);
    width = (int) fl_width(new_temp_string);
    while (width <= (pixels / 2)) {
	sprintf(new_temp_string, "%s ", new_temp_string);
	width = (int) fl_width(new_temp_string);
    }

    sprintf(new_string, "%s%s", new_temp_string, num_string);

    return (char *) new_string;
}

NxTodo *
NxAddress::search(const char *searchVal)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;
    char *needle = strup(searchVal, strlen(searchVal));

    if (0 == cur_record) {

	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}

	db->Select(CONTACTS, rec_array, 255);

    }

    if (255 == cur_record) {
	cur_record = 0;
	delete[]needle;
	needle = 0;
	return NULL;
    }

    bool found = false;
    NxTodo *note = new NxTodo;

    while (cur_record < 255) {
	int catid_array[1];
	char catid[8];
	found = false;

	catid_array[0] = -1;

	jdx = rec_array[cur_record];
	if (-1 == jdx) {
	    cur_record++;
	    continue;
	}

	db->Extract(CONTACTS, jdx, CAT, catid);
	db->Select(CATEGORY, catid, RECNO, catid_array, 1);
	if (-1 != catid_array[0])
	    db->Extract(CATEGORY, catid_array[0], CAT, note->szCategory);
	else
	    strcpy(note->szCategory, "Unfiled");

	// last_name
	db->Extract(CONTACTS, jdx, LASTNAME, note->szLastName);
	char *haystack = strup(note->szLastName, TEXT);

	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// id
	char szId[4];
	db->Extract(CONTACTS, jdx, RECNO, szId);
	note->nId = atoi(szId);

	// first_name
	db->Extract(CONTACTS, jdx, FIRSTNAME, note->szFirstName);
	haystack = strup(note->szFirstName, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// title
	db->Extract(CONTACTS, jdx, TITLE, note->szJobTitle);
	haystack = strup(note->szJobTitle, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// company
	db->Extract(CONTACTS, jdx, COMPANY, note->szCompany);
	haystack = strup(note->szCompany, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// Address
	db->Extract(CONTACTS, jdx, ADDRESS, note->szAddress);
	haystack = strup(note->szAddress, DBL_TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// City
	db->Extract(CONTACTS, jdx, CITY, note->szCity);
	haystack = strup(note->szCity, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// Region
	db->Extract(CONTACTS, jdx, REGION, note->szRegion);
	haystack = strup(note->szRegion, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// Postal Code
	db->Extract(CONTACTS, jdx, POSTALCODE, note->szPostalCode);
	haystack = strup(note->szPostalCode, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// Country
	db->Extract(CONTACTS, jdx, COUNTRY, note->szCountry);
	haystack = strup(note->szCountry, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info1id
	char buf1[4];
	db->Extract(CONTACTS, jdx, DEP1ID, buf1);
	note->nDep1 = atoi(buf1);

	// info2id
	db->Extract(CONTACTS, jdx, DEP2ID, buf1);
	note->nDep2 = atoi(buf1);

	// info3id
	db->Extract(CONTACTS, jdx, DEP3ID, buf1);
	note->nDep3 = atoi(buf1);

	// info4id
	db->Extract(CONTACTS, jdx, DEP4ID, buf1);
	note->nDep4 = atoi(buf1);

	// info5id
	db->Extract(CONTACTS, jdx, DEP5ID, buf1);
	note->nDep5 = atoi(buf1);

	// info6id
	db->Extract(CONTACTS, jdx, DEP6ID, buf1);
	note->nDep6 = atoi(buf1);

	// info7id
	db->Extract(CONTACTS, jdx, DEP7ID, buf1);
	note->nDep7 = atoi(buf1);

	// info1
	db->Extract(CONTACTS, jdx, DEP1, note->szDep1);
	haystack = strup(note->szDep1, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info2
	db->Extract(CONTACTS, jdx, DEP2, note->szDep2);
	haystack = strup(note->szDep2, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info3
	db->Extract(CONTACTS, jdx, DEP3, note->szDep3);
	haystack = strup(note->szDep3, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info4
	db->Extract(CONTACTS, jdx, DEP4, note->szDep4);
	haystack = strup(note->szDep4, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info5
	db->Extract(CONTACTS, jdx, DEP5, note->szDep5);
	haystack = strup(note->szDep5, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info6
	db->Extract(CONTACTS, jdx, DEP6, note->szDep6);
	haystack = strup(note->szDep6, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// info7
	db->Extract(CONTACTS, jdx, DEP7, note->szDep7);
	haystack = strup(note->szDep7, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// bday
	db->Extract(CONTACTS, jdx, BDAY, note->szBDay);
	haystack = strup(note->szBDay, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// anniv
	db->Extract(CONTACTS, jdx, ANNIV, note->szAnniv);
	haystack = strup(note->szAnniv, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// cust1
	db->Extract(CONTACTS, jdx, CUST1, note->szCustom1);
	haystack = strup(note->szCustom1, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// cust2
	db->Extract(CONTACTS, jdx, CUST2, note->szCustom2);
	haystack = strup(note->szCustom2, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// cust3
	db->Extract(CONTACTS, jdx, CUST3, note->szCustom3);
	haystack = strup(note->szCustom3, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// cust4
	db->Extract(CONTACTS, jdx, CUST4, note->szCustom4);
	haystack = strup(note->szCustom4, TEXT);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	// Note File Name
	db->Extract(CONTACTS, jdx, 16, note->szNoteFile);
	haystack = strup(note->szNoteFile, NOTEDB);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	if (true == found) {
	    cur_record++;
	    delete[]needle;
	    needle = 0;
	    return note;
	} else {
	    cur_record++;
	    continue;
	}

    }
    cur_record = 0;
    delete[]note;
    note = 0;
    delete[]needle;
    needle = 0;
    return NULL;
}

void
NxAddress::searchLookup_callback(Fl_Widget * fl, void *o)
{

    g_SearchFlag = 1;
    int total_found = 0;
    NxTodo *note;
    char *searchVal = (char *) lookup_input->value();
    char *needle = strup(searchVal, strlen(searchVal));

    results_table->Init(255);
    while ((note = search(searchVal)) != NULL) {
	results_table->Add(total_found, note);
	char *label = formatString(note, results_table->text_width());
	results_table->set_value(total_found, 0, label);
	total_found++;
    }

    delete[]needle;
    needle = 0;

    if (total_found == 0) {
	results_message->show();
	results_table->hide();
    } else {
	results_message->hide();
	results_table->show();
    }

    results_table->rows(total_found);

    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr());
    add_items(table, "All");
    NxApp::Instance()->show_window(addr_results_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_list_window->GetWindowPtr());

}

void
NxAddress::cancelLookup_callback(Fl_Widget * fl, void *o)
{
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::doneLookup_callback(Fl_Widget * fl, void *o)
{
    g_SearchFlag = 0;
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::yesDelList_callback(Fl_Widget * fl, void *l)
{

    // Delete Note
    NxTodo *n = (NxTodo *) table->selected();

    if (n) {

	// Unlink note file
	////////////////////
	unlink(n->szNoteFile);

	int recno[1];
	recno[0] = -1;

	char szId[4];
	sprintf(szId, "%d", n->nId);
	db->Select((string) CONTACTS, szId, 0, recno, 1);

	if (recno[0] == -1) {
	    return;
	}

	int fp1 = 0, fp2 = 0;
	db->GetFlags(CONTACTS, recno[0], fp1);
	//cout << "yesDelList_callback(): fp = " << fp1 << endl;
	db->DeleteRec(CONTACTS, recno[0]);
	db->GetFlags(CONTACTS, recno[0], fp2);
	//cout << "yesDelList_callback(): fp = " << fp2 << "err = " << err << endl;

    } else {

	return;

    }

    if (AllFlag) {
	set_category("All");
	AllFlag = false;
    } else
	set_category(n->szCategory);

    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::noDelList_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::yesDelEdit_callback(Fl_Widget * fl, void *l)
{


    // Delete Note
    NxTodo *n = (NxTodo *) table->selected();

    if (n) {

	// Unlink note file
	////////////////////
	unlink(n->szNoteFile);

	int recno[1];
	recno[0] = -1;

	NxTodo *n = (NxTodo *) table->selected();

	char szId[4];
	sprintf(szId, "%d", n->nId);
	db->Select((string) CONTACTS, szId, 0, recno, 1);

	if (recno[0] == -1) {
	    return;
	}
	db->DeleteRec(CONTACTS, recno[0]);
    } else {

	return;

    }

    if ("AllFlag") {
	set_category("All");
	AllFlag = false;
    } else
	set_category(n->szCategory);


    // Delete Note
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::noDelEdit_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(addr_edit_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::yesDelView_callback(Fl_Widget * fl, void *l)
{

    // Delete Note
    NxTodo *n = (NxTodo *) table->selected();

    if (n) {

	// Unlink note file
	////////////////////
	unlink(n->szNoteFile);

	int recno[1];
	recno[0] = -1;

	NxTodo *n = (NxTodo *) table->selected();

	char szId[4];
	sprintf(szId, "%d", n->nId);
	db->Select((string) CONTACTS, szId, 0, recno, 1);

	if (recno[0] == -1) {
	    return;
	}
	db->DeleteRec(CONTACTS, recno[0]);

    } else {

	return;

    }


    if (AllFlag) {
	set_category("All");
	AllFlag = false;
    } else
	set_category(n->szCategory);
    // Delete Note
    NxApp::Instance()->show_window(addr_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::noDelView_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(addr_view_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxAddress::dup_callback(Fl_Widget * fl, void *o)
{

    NxTodo *dupped = (NxTodo *) table->selected();

    if (dupped) {

	NxTodo *n = new NxTodo;

	// Index
	//strcpy(n->szId, dupped->szId);
	n->nId = dupped->nId;
	n->nCat = dupped->nCat;
	strcpy(n->szCategory, dupped->szCategory);
	n->nShowDisplay = dupped->nShowDisplay;

	// Data
	strcpy(n->szFirstName, dupped->szFirstName);
	strcpy(n->szLastName, dupped->szLastName);
	strcpy(n->szCompany, dupped->szCompany);
	strcpy(n->szJobTitle, dupped->szJobTitle);
	n->nDep1 = dupped->nDep1;
	n->nDep2 = dupped->nDep2;
	n->nDep3 = dupped->nDep3;
	n->nDep4 = dupped->nDep4;
	n->nDep5 = dupped->nDep5;
	n->nDep6 = dupped->nDep6;
	n->nDep7 = dupped->nDep7;
	strcpy(n->szDep1, dupped->szDep1);
	strcpy(n->szDep2, dupped->szDep2);
	strcpy(n->szDep3, dupped->szDep3);
	strcpy(n->szDep4, dupped->szDep4);
	strcpy(n->szDep5, dupped->szDep5);
	strcpy(n->szDep6, dupped->szDep6);
	strcpy(n->szDep7, dupped->szDep7);
	strcpy(n->szAddress, dupped->szAddress);
	strcpy(n->szCity, dupped->szCity);
	strcpy(n->szRegion, dupped->szRegion);
	strcpy(n->szPostalCode, dupped->szPostalCode);
	strcpy(n->szCountry, dupped->szCountry);
	strcpy(n->szBDay, dupped->szBDay);
	strcpy(n->szAnniv, dupped->szAnniv);
	strcpy(n->szCustom1, dupped->szCustom1);
	strcpy(n->szCustom2, dupped->szCustom2);
	strcpy(n->szCustom3, dupped->szCustom3);
	strcpy(n->szCustom4, dupped->szCustom4);
	strcpy(n->szNoteFile, dupped->szNoteFile);

	char *f = n->szNoteFile;

	if (f[0] != '^') {
	    char tplate[128];
	    int tfd = 0;

	    snprintf(tplate, sizeof(tplate), "%s/%sXXXXXX", nx_inidir,
		     NX_INIFILE);

	    tfd = mkstemp(tplate);

	    if (tfd > 0) {
		strcpy(n->szNoteFile, tplate);
		//strcpy(f, tempnam(nx_inidir, NX_INIFILE));
		//strcpy(n->szNoteFile, f);

		FILE *fd = fdopen(tfd, "w+");
		if (fd) {
		    g_editor->SaveTo(fd);
		    fclose(fd);
		}
	    }
	}

	write_note(n);

	if (0 != strcmp("All", note_category->label())) {
	    set_category(n->szCategory);
	    add_items(table, n->szCategory);
	} else {
	    add_items(table, "All");
	}
	delete n;

    } else {

	return;

    }

}


void
NxAddress::lookup_callback(Fl_Widget * fl, void *o)
{

    NxApp::Instance()->show_window(addr_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   addr_list_window->GetWindowPtr());
    Fl::focus((Fl_Widget *) lookup_input);
    lookup_input->value("");
}


void
NxAddress::_fill_view_form(NxTodo * n)
{

    // First and Last Name
    char name[TEXT + TEXT + 2];
    strcpy(name, n->szFirstName);
    strcat(name, " ");
    strcat(name, n->szLastName);
    viewName->value(name);

    // Business Title
    viewBusTitle->value(n->szJobTitle);

    // Company
    viewCompany->value(n->szCompany);

    // Address
    viewAddress->value(n->szAddress);

    // City
    int saveX = viewCityRegionPC->x();
    int saveY = viewCityRegionPC->y();
    int saveH = viewCityRegionPC->h();

    char tmpStr[MAX_LENGTH];
    int l1 = strlen(n->szCity);
    int l2 = strlen(n->szRegion);
    int l3 = strlen(n->szPostalCode);

    if ((l1 + l2 + l3) < MAX_LENGTH - 3) {

	strcpy(tmpStr, n->szCity);

	if ((n->szCity[0] != 0) && (n->szRegion[0] != 0))
	    strcat(tmpStr, ", ");

	strcat(tmpStr, n->szRegion);

	if ((n->szCity[0] != 0) || (n->szRegion[0] != 0))
	    strcat(tmpStr, " ");

	strcat(tmpStr, n->szPostalCode);
    } else if ((l1 + l2) < MAX_LENGTH - 2) {
	strcpy(tmpStr, n->szCity);

	if ((n->szRegion[0] != 0) && (n->szCity[0] != 0))
	    strcat(tmpStr, ", ");

	strcat(tmpStr, n->szRegion);

    } else if (l1 <= MAX_LENGTH) {
	strcpy(tmpStr, n->szCity);
    } else {
	memset(tmpStr, 0, MAX_LENGTH);
    }


    int nSpace = (int) fl_width("N");
    int pixels = (int) fl_width(tmpStr);

    viewCityRegionPC->resize(saveX, saveY, pixels + nSpace + nSpace, saveH);
    viewCityRegionPC->value(tmpStr);

    // Country
    viewCountry->value(n->szCountry);

    for (int i = 0; i < 7; i++) {

	// Work Phone
	if (n->nDep1 == WORK)
	    viewWorkPhone->value(n->szDep1);
	else if (n->nDep2 == WORK)
	    viewWorkPhone->value(n->szDep2);
	else if (n->nDep3 == WORK)
	    viewWorkPhone->value(n->szDep3);
	else if (n->nDep4 == WORK)
	    viewWorkPhone->value(n->szDep4);
	else if (n->nDep5 == WORK)
	    viewWorkPhone->value(n->szDep5);
	else if (n->nDep6 == WORK)
	    viewWorkPhone->value(n->szDep6);
	else if (n->nDep7 == WORK)
	    viewWorkPhone->value(n->szDep7);

	// E-Mail
	if (n->nDep1 == EMAIL)
	    viewEMail->value(n->szDep1);
	else if (n->nDep2 == EMAIL)
	    viewEMail->value(n->szDep2);
	else if (n->nDep3 == EMAIL)
	    viewEMail->value(n->szDep3);
	else if (n->nDep4 == EMAIL)
	    viewEMail->value(n->szDep4);
	else if (n->nDep5 == EMAIL)
	    viewEMail->value(n->szDep5);
	else if (n->nDep6 == EMAIL)
	    viewEMail->value(n->szDep6);
	else if (n->nDep7 == EMAIL)
	    viewEMail->value(n->szDep7);

    }

}

void
NxAddress::_fill_form(NxTodo * n)
{

    edit_firstname->value(n->szFirstName);
    edit_lastname->value(n->szLastName);
    edit_company->value(n->szCompany);
    edit_title->value(n->szJobTitle);
    edit_misc_list1->value(n->nDep1);
    edit_misc_list2->value(n->nDep2);
    edit_misc_list3->value(n->nDep3);
    edit_misc_list4->value(n->nDep4);
    edit_misc_list5->value(n->nDep5);
    edit_misc_list6->value(n->nDep6);
    edit_misc_list7->value(n->nDep7);
    edit_misc1->value(n->szDep1);
    edit_misc2->value(n->szDep2);
    edit_misc3->value(n->szDep3);
    edit_misc4->value(n->szDep4);
    edit_misc5->value(n->szDep5);
    edit_misc6->value(n->szDep6);
    edit_misc7->value(n->szDep7);
    editAddress->value(n->szAddress);
    editCity->value(n->szCity);
    editRegion->value(n->szRegion);
    editPostalCode->value(n->szPostalCode);
    editCountry->value(n->szCountry);
    edit_bday->value(n->szBDay);
    edit_anniv->value(n->szAnniv);

    int rec_array[4];
    char *custlabel1 = new char[CUSTOM_NAME];
    char *custlabel2 = new char[CUSTOM_NAME];
    char *custlabel3 = new char[CUSTOM_NAME];
    char *custlabel4 = new char[CUSTOM_NAME];

    for (int idx = 0; idx < 4; idx++) {
	rec_array[idx] = -1;
    }

    db->Select(CUSTFIELDS, rec_array, 4);

    if ((rec_array[0] >= 0)) {
	db->Extract(CUSTFIELDS, rec_array[0], 1, custlabel1);
	db->Extract(CUSTFIELDS, rec_array[1], 1, custlabel2);
	db->Extract(CUSTFIELDS, rec_array[2], 1, custlabel3);
	db->Extract(CUSTFIELDS, rec_array[3], 1, custlabel4);

	edit_custom1->label(custlabel1);
	edit_custom2->label(custlabel2);
	edit_custom3->label(custlabel3);
	edit_custom4->label(custlabel4);
    }

    edit_custom1->value(n->szCustom1);
    edit_custom2->value(n->szCustom2);
    edit_custom3->value(n->szCustom3);
    edit_custom4->value(n->szCustom4);

}

void
NxAddress::edit_note(NxTodo * note, int recno)
{

//  int id = atoi(note->szId);
//  int catid = GetCatId(note->szCategory);


    char *record = Record(note->nId, note->nCat, note->nShowDisplay,
			  (string) note->szLastName,
			  (string) note->szFirstName,
			  (string) note->szCompany, (string) note->szJobTitle,
			  note->nDep1, note->nDep2, note->nDep3, note->nDep4,
			  note->nDep5, note->nDep6, note->nDep7,
			  (string) note->szDep1, (string) note->szDep2,
			  (string) note->szDep3,
			  (string) note->szDep4, (string) note->szDep5,
			  (string) note->szDep6,
			  (string) note->szDep7,
			  (string) note->szAddress, (string) note->szCity,
			  (string) note->szRegion,
			  (string) note->szPostalCode,
			  (string) note->szCountry,
			  (string) note->szBDay, (string) note->szAnniv,
			  (string) note->szCustom1, (string) note->szCustom2,
			  (string) note->szCustom3, (string) note->szCustom4,
			  (string) note->szNoteFile);

    db->Edit(CONTACTS, recno, record);

    delete[]record;
    record = 0;

}

void
NxAddress::write_note(NxTodo * note)
{

    id++;

    int catid = GetCatId(note->szCategory);

    char *record = Record(id, catid, note->nShowDisplay,
			  (string) note->szLastName,
			  (string) note->szFirstName,
			  (string) note->szCompany, (string) note->szJobTitle,
			  note->nDep1, note->nDep2, note->nDep3, note->nDep4,
			  note->nDep5, note->nDep6, note->nDep7,
			  (string) note->szDep1, (string) note->szDep2,
			  (string) note->szDep3,
			  (string) note->szDep4, (string) note->szDep5,
			  (string) note->szDep6,
			  (string) note->szDep7,
			  (string) note->szAddress, (string) note->szCity,
			  (string) note->szRegion,
			  (string) note->szPostalCode,
			  (string) note->szCountry,
			  (string) note->szBDay, (string) note->szAnniv,
			  (string) note->szCustom1, (string) note->szCustom2,
			  (string) note->szCustom3, (string) note->szCustom4,
			  (string) note->szNoteFile);

    //cout << "write_note(): note->szNoteFile = " << note->szNoteFile << endl;
    db->Insert(CONTACTS, record);

    delete[]record;
    record = 0;

}

// Get Category ID
int
NxAddress::GetCatId(char *szCategory)
{

    if ((strcmp(szCategory, "All") == 0))
	strcpy(szCategory, "Unfiled");


    int recno[1];
    recno[0] = -1;

    char catid[255];

    db->Select((string) CATEGORY, szCategory, 1, recno, 1);

    if (recno[0] == -1)
	return (0);

    db->Extract((string) CATEGORY, recno[0], 0, catid);

    int cid = atoi(catid);

    return cid;

}

// add items to the tree view
void
NxAddress::add_items(Flv_Table_Child * t, const char *szCategory)
{

    // Fix me!
    t->row(0);

    int rec_array[255];
    int cat_array[1];
    char value[16];
    char cat[16];
    int idx = 0;

    cat_array[0] = -1;

    for (idx = 0; idx < 255; idx++) {
	rec_array[idx] = -1;
    }

    sprintf(value, " ");

    db->Select(CATEGORY, (char *) szCategory, 1, cat_array, 1);

    if ((cat_array[0] >= 0))
	db->Extract(CATEGORY, cat_array[0], 0, value);

    if (0 == strcmp("All", szCategory))
	db->Select(CONTACTS, rec_array, 255);
    else
	db->Select(CONTACTS, value, 1, rec_array, 255);

    int cnt = 0;
    while (rec_array[cnt] != -1) {
	cnt++;
    }

    // Allocate Memory
    // for Array of record struct.
    NxTodo *note[cnt];
    for (int i = 0; i < cnt; i++)
	note[i] = new NxTodo;

    t->Init(cnt);

    int row;
    for (row = 0; row < cnt; row++) {

	int j = -1;
	int jdx = rec_array[row];

	if ((cat_array[0] >= 0) || (0 == strcmp("All", szCategory))) {

	    if (cat_array[0] >= 0) {
		db->Extract(CONTACTS, jdx, 1, cat);
		j = strcmp(value, cat);
	    }

	    if ((j == 0) || 0 == strcmp("All", szCategory)) {

		// id
		char szId[4];
		db->Extract(CONTACTS, jdx, 0, szId);
		note[row]->nId = atoi(szId);

		// categoryid
		int catid_array[1];
		char catid[8];

		catid_array[0] = -1;
		memset(note[row]->szCategory, 0, CATEGORYS);

		db->Extract(CONTACTS, jdx, 1, catid);
		note[row]->nCat = atoi(catid);
		db->Select(CATEGORY, catid, 0, catid_array, 1);

		if (-1 != catid_array[0])
		    db->Extract(CATEGORY, catid_array[0], 1,
				note[row]->szCategory);
		else
		    strcpy(note[row]->szCategory, "Unfiled");

		// show
		char szShowDisplay[4];
		db->Extract(CONTACTS, jdx, SHOW, szShowDisplay);
		note[row]->nShowDisplay = atoi(szShowDisplay);

		// last_name
		db->Extract(CONTACTS, jdx, LASTNAME, note[row]->szLastName);

		// first_name
		db->Extract(CONTACTS, jdx, FIRSTNAME, note[row]->szFirstName);

		// company
		db->Extract(CONTACTS, jdx, COMPANY, note[row]->szCompany);

		// title
		db->Extract(CONTACTS, jdx, TITLE, note[row]->szJobTitle);

		// info1id
		char buf1[4];
		db->Extract(CONTACTS, jdx, DEP1ID, buf1);
		note[row]->nDep1 = atoi(buf1);

		// info2id
		db->Extract(CONTACTS, jdx, DEP2ID, buf1);
		note[row]->nDep2 = atoi(buf1);

		// info3id
		db->Extract(CONTACTS, jdx, DEP3ID, buf1);
		note[row]->nDep3 = atoi(buf1);

		// info4id
		db->Extract(CONTACTS, jdx, DEP4ID, buf1);
		note[row]->nDep4 = atoi(buf1);

		// info5id
		db->Extract(CONTACTS, jdx, DEP5ID, buf1);
		note[row]->nDep5 = atoi(buf1);

		// info6id
		db->Extract(CONTACTS, jdx, DEP6ID, buf1);
		note[row]->nDep6 = atoi(buf1);

		// info7id
		db->Extract(CONTACTS, jdx, DEP7ID, buf1);
		note[row]->nDep7 = atoi(buf1);

		// info1
		db->Extract(CONTACTS, jdx, DEP1, note[row]->szDep1);

		// info2
		db->Extract(CONTACTS, jdx, DEP2, note[row]->szDep2);

		// info3
		db->Extract(CONTACTS, jdx, DEP3, note[row]->szDep3);

		// info4
		db->Extract(CONTACTS, jdx, DEP4, note[row]->szDep4);

		// info5
		db->Extract(CONTACTS, jdx, DEP5, note[row]->szDep5);

		// info6
		db->Extract(CONTACTS, jdx, DEP6, note[row]->szDep6);

		// info7
		db->Extract(CONTACTS, jdx, DEP7, note[row]->szDep7);

		// Address
		db->Extract(CONTACTS, jdx, ADDRESS, note[row]->szAddress);

		// City
		db->Extract(CONTACTS, jdx, CITY, note[row]->szCity);

		// Region
		db->Extract(CONTACTS, jdx, REGION, note[row]->szRegion);

		// Postal Code
		db->Extract(CONTACTS, jdx, POSTALCODE,
			    note[row]->szPostalCode);

		// Country
		db->Extract(CONTACTS, jdx, COUNTRY, note[row]->szCountry);

		// bday
		db->Extract(CONTACTS, jdx, BDAY, note[row]->szBDay);

		// anniv
		db->Extract(CONTACTS, jdx, ANNIV, note[row]->szAnniv);

		// custom1
		db->Extract(CONTACTS, jdx, CUST1, note[row]->szCustom1);

		// custom2
		db->Extract(CONTACTS, jdx, CUST2, note[row]->szCustom2);

		// custom3
		db->Extract(CONTACTS, jdx, CUST3, note[row]->szCustom3);

		// custom4
		db->Extract(CONTACTS, jdx, CUST4, note[row]->szCustom4);

		// Note File Name
		db->Extract(CONTACTS, jdx, NOTE, note[row]->szNoteFile);

		/*
		   if ( note->nDep1 == details_misc_list->value() )
		   sprintf(inbuf,"%s, %s\t%s",note->szLastName,note->szFirstName,note->szDep1);
		   else if ( note->nDep2 == details_misc_list->value() )
		   sprintf(inbuf,"%s, %s\t%s",note->szLastName,note->szFirstName,note->szDep2);
		   else if ( note->nDep3 == details_misc_list->value() )
		   sprintf(inbuf,"%s, %s\t%s",note->szLastName,note->szFirstName,note->szDep3);
		   else if ( note->nDep4 == details_misc_list->value() )
		   sprintf(inbuf,"%s, %s\t%s",note->szLastName,note->szFirstName,note->szDep4);
		   else // default case, use Work
		 */

	    }
	}
    }

    qsort(note, cnt, sizeof(NxTodo *),
	  (int (*)(const void *, const void *)) compar);

    for (row = 0; row < cnt; row++) {

	t->Add(row, note[row]);

	char label[50];

	// 1st Column
	if ((note[row]->szLastName[0] == 0)
	    && (note[row]->szFirstName[0] != 0))
	    sprintf(label, "%s", note[row]->szFirstName);
	else if ((note[row]->szLastName[0] == 0)
		 && (note[row]->szFirstName[0] == 0))
	    sprintf(label, "%s", note[row]->szCompany);
	else {
	    if (note[row]->szFirstName[0] == 0)
		sprintf(label, "%s", note[row]->szLastName);
	    else
		sprintf(label, "%s, %s", note[row]->szLastName,
			note[row]->szFirstName);
	}


	t->set_value(row, 0, label);

	// 2nd Column
	int foundFlag = 1;
	switch (note[row]->nShowDisplay) {

	case HOME:
	    if (note[row]->nDep1 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep1, "H");

	    } else if (note[row]->nDep2 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep2, "H");

	    } else if (note[row]->nDep3 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep3, "H");

	    } else if (note[row]->nDep4 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep4, "H");

	    } else if (note[row]->nDep5 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep5, "H");

	    } else if (note[row]->nDep6 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep6, "H");

	    } else if (note[row]->nDep7 == HOME) {

		sprintf(label, "%s%s", note[row]->szDep7, "H");

	    } else
		foundFlag = 0;

	    break;

	case WORK:
	    if (note[row]->nDep1 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep1, "W");

	    } else if (note[row]->nDep2 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep2, "W");

	    } else if (note[row]->nDep3 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep3, "W");

	    } else if (note[row]->nDep4 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep4, "W");

	    } else if (note[row]->nDep5 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep5, "W");

	    } else if (note[row]->nDep6 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep6, "W");

	    } else if (note[row]->nDep7 == WORK) {

		sprintf(label, "%s%s", note[row]->szDep7, "W");

	    } else
		foundFlag = 0;

	    break;

	case FAX:
	    if (note[row]->nDep1 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep1, "F");

	    } else if (note[row]->nDep2 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep2, "F");

	    } else if (note[row]->nDep3 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep3, "F");

	    } else if (note[row]->nDep4 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep4, "F");

	    } else if (note[row]->nDep5 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep5, "F");

	    } else if (note[row]->nDep6 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep6, "F");

	    } else if (note[row]->nDep7 == FAX) {

		sprintf(label, "%s%s", note[row]->szDep7, "F");

	    } else
		foundFlag = 0;

	    break;

	case MOBILE:
	    if (note[row]->nDep1 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep1, "M");

	    } else if (note[row]->nDep2 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep2, "M");

	    } else if (note[row]->nDep3 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep3, "M");

	    } else if (note[row]->nDep4 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep4, "M");

	    } else if (note[row]->nDep5 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep5, "M");

	    } else if (note[row]->nDep6 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep6, "M");

	    } else if (note[row]->nDep7 == MOBILE) {

		sprintf(label, "%s%s", note[row]->szDep7, "M");

	    } else
		foundFlag = 0;

	    break;

	case PAGER:
	    if (note[row]->nDep1 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep1, "P");

	    } else if (note[row]->nDep2 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep2, "P");

	    } else if (note[row]->nDep3 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep3, "P");

	    } else if (note[row]->nDep4 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep4, "P");

	    } else if (note[row]->nDep5 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep5, "P");

	    } else if (note[row]->nDep6 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep6, "P");

	    } else if (note[row]->nDep7 == PAGER) {

		sprintf(label, "%s%s", note[row]->szDep7, "P");

	    } else
		foundFlag = 0;

	    break;

	case EMAIL:
	    if (note[row]->nDep1 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep1);

	    } else if (note[row]->nDep2 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep2);

	    } else if (note[row]->nDep3 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep3);

	    } else if (note[row]->nDep4 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep4);

	    } else if (note[row]->nDep5 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep5);

	    } else if (note[row]->nDep6 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep6);

	    } else if (note[row]->nDep7 == EMAIL) {

		sprintf(label, "%s", note[row]->szDep7);

	    } else
		foundFlag = 0;

	    break;

	case WEBPAGE:
	    if (note[row]->nDep1 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep1);

	    } else if (note[row]->nDep2 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep2);

	    } else if (note[row]->nDep3 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep3);

	    } else if (note[row]->nDep4 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep4);

	    } else if (note[row]->nDep5 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep5);

	    } else if (note[row]->nDep6 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep6);

	    } else if (note[row]->nDep7 == WEBPAGE) {

		sprintf(label, "%s", note[row]->szDep7);


	    } else
		foundFlag = 0;

	    break;

	}


	if (!foundFlag) {

	    switch (note[row]->nDep1) {
	    case HOME:
		sprintf(label, "%s%s", note[row]->szDep1, "H");
		break;
	    case WORK:
		sprintf(label, "%s%s", note[row]->szDep1, "W");
		break;
	    case FAX:
		sprintf(label, "%s%s", note[row]->szDep1, "F");
		break;
	    case MOBILE:
		sprintf(label, "%s%s", note[row]->szDep1, "M");
		break;
	    case PAGER:
		sprintf(label, "%s%s", note[row]->szDep1, "P");
		break;
	    case EMAIL:
		sprintf(label, "%s", note[row]->szDep1);
		break;
	    case WEBPAGE:
		sprintf(label, "%s", note[row]->szDep1);
		break;
	    }

	}

	t->set_value(row, 1, label);

    }

    t->rows(cnt);
}

void
NxAddress::clear_table()
{

    table->Init(0);
    table->rows(0);

}

void
NxAddress::set_category(char *szCat)
{

    char szRealCategory[CATEGORYS];

    //if (!szCat[0])
    //    strcpy(szRealCategory, "All");
    //else

    strcpy(szRealCategory, szCat);

    //  clear_table();

    add_items(table, szRealCategory);

    fill_categories();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();
    edit_category_list->label(szRealCategory);
    edit_category_list->hide();
    edit_category_list->show();
    view_category_list->label(szRealCategory);
    view_category_list->hide();
    view_category_list->show();
}

void
NxAddress::reset_category(char *szCat)
{
    char szRealCategory[CATEGORYS];

    //if (!szCat[0])
    //strcpy(szRealCategory, "All");
    //else
    strcpy(szRealCategory, szCat);

    fill_categories();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();
    edit_category_list->label(szRealCategory);
    edit_category_list->hide();
    edit_category_list->show();
    view_category_list->label(szRealCategory);
    view_category_list->hide();
    view_category_list->show();
}

void
NxAddress::fill_categories()
{

    char ret_buf[CAT_NAME];
    int rec_count = db->NumRecs(CATEGORY);

    for (int idx = 0; idx < CAT_NUM; idx++) {
	cat_list[idx]->clear();
	cat_list[idx]->add("All");
	for (int jdx = 1; jdx <= rec_count; jdx++) {
	    db->Extract(CATEGORY, jdx, 1, ret_buf);
	    cat_list[idx]->add(ret_buf);
	}
	cat_list[idx]->label(const_cast < char *>(cat_list[idx]->text(0)));
    }
    addr_list_window->GetEditCategoryPtr()->clear_tree();
    addr_list_window->GetEditCategoryPtr()->add_items(addr_list_window->
						      GetEditCategoryPtr()->
						      get_category_tree());
    addr_edit_window->GetEditCategoryPtr()->clear_tree();
    addr_edit_window->GetEditCategoryPtr()->add_items(addr_edit_window->
						      GetEditCategoryPtr()->
						      get_category_tree());

}
