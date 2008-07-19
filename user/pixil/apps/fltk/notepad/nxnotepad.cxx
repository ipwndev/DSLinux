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


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pixil_config.h>
#include "nxnotepad.h"

#include <icons/folder_small.xpm>

#ifdef DEBUG
#include <assert.h>
#define DPRINT(str, args...) printf("NXNOTEPAD DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

#define MAX_RECS				255

#define NX_INIFILE "not"

extern int exit_flag;

about about_notes = {
    "About Notes",
    "Century Software, 2001",
    "trumanl@censoft.com",
    "08/24/01",
    "1.0"
};

// Note database
field noteFields[] = {
    {'i', 1, 0},
    {'i', 1, 0},		/* note category */
    {'c', FILE_LEN, 0},		/* key note name */
    {'c', TITLE_LEN, 0},	/* note title */
    {'i', 1, 0},		/* archive 1=yes 0=no */
    {0}
};

fildes noteFile = {		/* system file */
    0, 0, 0,			/* database file */
    "dbf",			/* extension */
    NOTE_NUM_FIELDS,		/* nfields */
    &noteFields[0]		/* fieldlist */
};

// Cat database
field catFields[] = {
    {'i', 1, 0},
    {'c', CAT_LEN, 0},
    {0}
};

fildes catFile = {		/* system file */
    0, 0, 0,			/* database file */
    "dbf",			/* extension */
    CAT_NUM_FIELDS,		/* nfields */
    &catFields[0]		/* field list */
};

//Menus
Fl_Menu_Item list_window_menu[] = {
    {"Record", 0, 0, 0, FL_SUBMENU},
    {"New Note", 0, NxNotepad::new_callback},
    {"Delete Note", 0, NxNotepad::delete_note_callback, 0, FL_MENU_DIVIDER},
    {"Exit Notes", 0, NxNotepad::exit_callback},
    {0},
    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback},
    //{"Select All"},
//              {"Keyboard", 0, NxApp::keyboard_callback},
    {0},
    {"Options", 0, 0, 0, FL_SUBMENU},
    //{"Font"},
    {"Search", 0, NxNotepad::lookup_callback},
    {"About Notes", 0, NxApp::show_about},
    {0},
    {0},
    {0}
};

Fl_Menu_Item new_note_menu[] = {
    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER},
    //{"Select All"},
//              {"Keyboard", 0, NxApp::keyboard_callback, 0, FL_MENU_DIVIDER},
    {"Exit Notes", 0, NxNotepad::exit_callback},
    {0},
    {"Options", 0, 0, 0, FL_SUBMENU},
    //{"Font"},
    {"About Notes", 0, NxApp::show_about},
    {0},
    {0},
    {0}
};

Fl_Menu_Item note_edit_menu[] = {
    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER},
    //{"Select All"},
//              {"Keyboard", 0, NxApp::keyboard_callback},
    {"Exit Notes", 0, NxNotepad::exit_callback},
    {0},
    {"Options", 0, 0, 0, FL_SUBMENU},
    //{"Font"},
    {"About Notes", 0, NxApp::show_about},
    {0},
    {0},
    {0}
};

NxWindow *
    NxNotepad::main_window;
NxDb *
    NxNotepad::note_db;
char *
    NxNotepad::nx_inidir;

NxPimWindow *
    NxNotepad::note_list_window;
NxPimWindow *
    NxNotepad::note_edit_window;

NxPimPopWindow *
    NxNotepad::note_delete_window;
NxPimPopWindow *
    NxNotepad::note_lookup_window;
NxPimPopWindow *
    NxNotepad::note_results_window;

NxCategoryList *
    NxNotepad::note_category;
NxCategoryList *
    NxNotepad::edit_category;
NxCategoryList *
    NxNotepad::cat_list[CAT_NUM];

Fl_Editor *
    NxNotepad::g_editor;
Fl_Toggle_Tree *
    NxNotepad::tree;
NxScroll *
    NxNotepad::note_list;
Fl_Pixmap *
    NxNotepad::folderSmall;

bool NxNotepad::AllFlag;
bool NxNotepad::new_note_;
bool NxNotepad::g_SearchFlag;
bool NxNotepad::save_archive_;

int
    NxNotepad::note_key;

NxInput *
    NxNotepad::lookup_input;
Flv_Table_Child *
    NxNotepad::results_table;

NxOutput *
    NxNotepad::results_message;

NxNotepad::NxNotepad(int argc, char *argv[])
    :
NxApp()
{

    NxApp::Instance()->set_about(about_notes);
    note_db = new NxDb(argc, argv);
    optind = 1;
    NxApp::Instance()->set_keyboard(argc, argv);
    nx_inidir = note_db->GetPath();
    open_note_database();
    open_cat_database();

    main_window = new NxWindow(W_W, W_H, APP_NAME);
    make_list_window();
    make_edit_window();
    make_delete_window();
    make_lookup_window();
    make_results_window();
    main_window->end();

    set_shown_window(note_list_window->GetWindowPtr());

    note_category = note_list_window->category_list;
    edit_category = note_edit_window->category_list;

    cat_list[0] = note_category;
    cat_list[1] = edit_category;
    fill_categories();

    folderSmall = new Fl_Pixmap(folder_small);
    add_items(tree, note_category->label());
    note_category->select(category_callback);
    edit_category->select(list_callback);
    save_archive_ = false;
    new_note_ = false;

    note_key = GetKey(note_db, NOTE_DATABASE, NOTE_INDEX);
    DPRINT("Note key is [%d]\n", note_key);

    set_catlist_window((Fl_Window *) note_list_window->
		       GetEditCategoryWindowPtr());

    // Register databases for PIM Synchronization
    //////////////////////////////////////////////
    int file_fields[1];
    file_fields[0] = 2;		// Column of file name.

    SyncRegisterDB(note_db, noteFields, NOTE_DATABASE, 1, file_fields, 1);
    SyncRegisterDB(note_db, catFields, CAT_DATABASE, 0, NULL, 0);

#ifdef CONFIG_COLOSSEUM
    Add_Fd("nxnotepad", _ClientIPCHandler);
#else
    ExecuteShow();
#endif

}

NxNotepad::~NxNotepad()
{

    note_db->Close(NOTE_DATABASE);
    note_db->Close(CAT_DATABASE);
    delete note_db;
    note_db = 0;

}

void
NxNotepad::Refresh()
{
    set_category(note_category->label());
}

//
// FLNX-Colosseum IPC Methods
//

#ifdef CONFIG_COLOSSEUM

void
NxNotepad::ClientIPCHandler(int fd, void *o, int ipc_id)
{

    //  DPRINT("\n");
    // DPRINT("ClientIPCHandler has now been started.\n");

    char *tokenMsg = new char[MAX_LENGTH];
    memset(tokenMsg, 0, MAX_LENGTH);
    char *passMsg = new char[MAX_LENGTH];
    memset(passMsg, 0, MAX_LENGTH);

    //  int ipc_id = -1;

    //  DPRINT("And this is the message... %s.\n", msg);

    if (o == NULL) {

	int length = MAX_LENGTH - 1;
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);

	if ((passMsg == NULL) || (passMsg[0] == 0))
	    return;
	else
	    strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxnotepad");

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
    if (tmp != NULL)
	strcpy(service, tmp);

    // MSG_CMD
    tmp = strtok(NULL, TOKEN);
    if (tmp != NULL)
	strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    if (tmp != NULL)
	strcpy(data_item, tmp);

    //DPRINT("Expoding Message... %s, %s, %s.\n", service, msg_cmd, data_item);

    if (strcmp(msg_cmd, "EXECUTE") == 0) {

	//    DPRINT("EXECUTE message command recv.\n");

	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	if (strcmp(data_item, "search") == 0) {

	    DPRINT("%s, %s, %s\n", service, msg_cmd, data_item);

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
	    char *data = new char[255];

	    tmp = strtok(NULL, TOKEN);
	    strcpy(data, tmp);
	    viewRecord(data);
	    delete[]data;
	    data = NULL;
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

void
NxNotepad::ExecuteSearch(int ipc_id, char *searchStr, int width)
{

    if (searchStr == NULL)
	return;

    char *msg = new char[MAX_LENGTH];
    NxNote *note;

    while ((note = search(searchStr)) != NULL) {

	char *result = formatString(note, width);

	// Send DATA message to client
	// (e.g. "nxnotepad^DATA^1^Century *Software* is neato^")
	strcpy(msg, "nxnotepad^DATA^search^");
	strcat(msg, "-2^");	// means show on filename
	strcat(msg, note->szFile);
	strcat(msg, "^");
	strcat(msg, result);
	strcat(msg, "^");

	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    }

    strcpy(msg, "nxnotepad^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;

}

#endif

//
// Window Methods
//

void
NxNotepad::exit_callback(Fl_Widget * fl, void *l)
{
    exit_flag = 1;
    main_window->hide();
}

Fl_Window *
NxNotepad::get_main_window()
{

    if (main_window)
	return main_window;
    else
	return 0;

}

void
NxNotepad::show_default_window()
{
    show_window(note_list_window->GetWindowPtr());
}

void
NxNotepad::make_list_window()
{
    note_list_window =
	new NxPimWindow(APP_NAME, list_window_menu, note_db, CAT_DATABASE,
			NOTE_DATABASE, (void (*)(const char *)) set_category);
    add_window((Fl_Window *) note_list_window->GetWindowPtr());

    {
	NxScroll *o = note_list =
	    new NxScroll(-1, 31, W_W + 2, BUTTON_Y - 33);
	o->movable(false);
	{
	    tree = new Fl_Toggle_Tree(0, 31, W_W, 10);
	    tree->callback(note_tree_callback);
	    NxApp::Instance()->def_font(tree);
	}
	o->end();
	note_list_window->add((Fl_Widget *) o);
    }
    {
	NxBox *o = new NxBox(5, 5, 85, 25, "");
	note_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT,
			 "Add");
	o->callback(new_callback);
	note_list_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(BUTTON_X + 58, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Edit");
	o->callback(edit_callback);
	note_list_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(BUTTON_X + 116, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Delete");
	o->callback(delete_note_callback);
	note_list_window->add((Fl_Widget *) o);
    }
}
void
NxNotepad::make_edit_window()
{
    note_edit_window =
	new NxPimWindow("Notes", note_edit_menu, note_db, CAT_DATABASE,
			NOTE_DATABASE, (void (*)(const char *)) set_category);
    add_window((Fl_Window *) note_edit_window->GetWindowPtr());

    g_editor = new Fl_Editor(5, 30, W_W - 11, BUTTON_Y - 41);
    g_editor->movable(false);
    g_editor->box(FL_BORDER_BOX);
    g_editor->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
    g_editor->callback(NxApp::Instance()->pasteTarget_callback, (void *) 1);
    g_editor->textsize(0x0C);

    note_edit_window->add((Fl_Widget *) g_editor);
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Done");
	o->callback(done_edit_callback);
	note_edit_window->add((Fl_Widget *) o);
    }
}

void
NxNotepad::make_delete_window()
{
    note_delete_window = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) note_delete_window->GetWindowPtr());
    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current note ?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	note_delete_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(yes_delete_callback);
	note_delete_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(no_delete_callback);
	note_delete_window->add((Fl_Widget *) o);
    }
}

void
NxNotepad::make_lookup_window()
{
    note_lookup_window = new NxPimPopWindow("Search");
    add_window((Fl_Window *) note_lookup_window->GetWindowPtr());
    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "What are you looking for?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	note_lookup_window->add((Fl_Widget *) o);
    }
    {
	NxInput *o = lookup_input =
	    new NxInput(BUTTON_X, 60, W_W - BUTTON_WIDTH, 20);
	lookup_input->maximum_size(99);
	note_lookup_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Search");
	o->callback(searchLookup_callback);
	note_lookup_window->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelLookup_callback);
	note_lookup_window->add((Fl_Widget *) o);
    }
}

void
NxNotepad::make_results_window()
{

    note_results_window =
	new NxPimPopWindow("Searh Results",
			   NxApp::Instance()->getGlobalColor(APP_FG), 5,
			   (W_W / 3), W_W - 10, (W_H - (W_W / 2)));
    add_window((Fl_Window *) note_results_window->GetWindowPtr());

    {
	results_message = new NxOutput(4, 19, W_W - 19, 25);
	results_message->value("Nothing Found.");
	results_message->hide();
	note_results_window->add((Fl_Widget *) results_message);
    }
    {
	results_table =
	    new Flv_Table_Child(4, 19, (W_W - 19),
				(W_H - (W_W / 2) - 3 * (BUTTON_HEIGHT)), 0,
				(W_W - 25));
	results_table->callback(view_callback);
	results_table->selection_color(NxApp::Instance()->
				       getGlobalColor(HILIGHT));
	results_table->SetCols(1);
	note_results_window->add((Fl_Widget *) results_table);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneLookup_callback);
	note_results_window->add((Fl_Widget *) o);
    }
}

void
NxNotepad::add_items(Fl_Toggle_Tree * t, const char *szCategory)
{
    int rec_array[MAX_RECS];
    int cat_array[1];
    char value[16];
    char cat[16];
    Fl_Toggle_Node *n;
    NxNote *note;
    int idx = 0;

    for (idx = 0; idx < MAX_RECS; idx++) {
	rec_array[idx] = -1;
    }
    cat_array[0] = -1;

    strcpy(value, "");

    note_db->Select(CAT_DATABASE, const_cast < char *>(szCategory), CAT_DESC,
		    cat_array, 1);

    if (0 <= cat_array[0]) {
	note_db->Extract(CAT_DATABASE, cat_array[0], CAT_INDEX, value);
    }
    if (0 == strcmp("All", szCategory)) {
	note_db->Select(NOTE_DATABASE, rec_array, MAX_RECS);
    } else {
	note_db->Select(NOTE_DATABASE, value, NOTE_CAT, rec_array, 255);
    }

    idx = 0;
    while (-1 != rec_array[idx]) {

	int j = -1;
	int jdx = rec_array[idx];
	idx++;
	if ((0 <= cat_array[0]) || (0 == strcmp("All", szCategory))) {
	    if (0 <= cat_array[0]) {
		note_db->Extract(NOTE_DATABASE, jdx, NOTE_CAT, cat);
		j = strcmp(value, cat);
	    }
	    if ((j == 0) || (0 == strcmp("All", szCategory))) {

		note = new NxNote;
		note->bDeleteMe = 0;

		int catid_array[1];
		char catid[8];

		catid_array[0] = -1;

		memset(note->szCategory, 0, CAT_LEN);
		note_db->Extract(NOTE_DATABASE, jdx, NOTE_CAT, catid);
		note_db->Select(CAT_DATABASE, catid, CAT_INDEX, catid_array,
				1);
		if (-1 != catid_array[0])
		    note_db->Extract(CAT_DATABASE, catid_array[0], CAT_DESC,
				     note->szCategory);
		else
		    strcpy(note->szCategory, "Unfiled");

		char str_key[16];

		note_db->Extract(NOTE_DATABASE, jdx, NOTE_INDEX, str_key);
		note->key = atoi(str_key);

		note_db->Extract(NOTE_DATABASE, jdx, NOTE_FILE, note->szFile);

		note_db->Extract(NOTE_DATABASE, jdx, NOTE_DESC,
				 note->szTitle);

		n = t->add_next(note->szTitle, 0, folderSmall);

		n->user_data(note);
		t->traverse_up();
	    }
	}
    }
    note_list->position(0, 0);
}

void
NxNotepad::select_note(NxNote * note)
{
    Fl_Toggle_Node *node = tree->traverse_start();

    while (NULL != node) {
	if (0 == strcmp(note->szFile, ((NxNote *) node->user_data())->szFile)) {
	    tree->select_range(node, node, 0);
	    break;
	}
	node = tree->traverse_forward();
    }
}

void
NxNotepad::viewRecord(char *fileName)
{
    NxApp::Instance()->set_catlist_window((Fl_Window *) note_edit_window->
					  GetEditCategoryWindowPtr());

    NxNote *note = new NxNote;

    int rec_array[1];

    rec_array[0] = -1;

    note_db->Select(NOTE_DATABASE, fileName, NOTE_FILE, rec_array, 1);

    int catid_array[1];
    char catid[8];

    catid_array[0] = -1;
    note_db->Extract(NOTE_DATABASE, rec_array[0], NOTE_CAT, catid);
    note_db->Select(CAT_DATABASE, catid, 0, catid_array, 1);
    if (-1 != catid_array[0])
	note_db->Extract(CAT_DATABASE, catid_array[0], 1, note->szCategory);
    else
	strcpy(note->szCategory, "Unfiled");

    strcpy(note->szFile, fileName);
    g_editor->Clear();
    FILE *fd = fopen(note->szFile, "r");
    g_editor->LoadFrom(fd);
    g_editor->MoveTo(0, 0);
    fclose(fd);

    g_SearchFlag = false;
    new_note_ = false;
    select_note(note);
    edit_category->label(note->szCategory);

    NxApp::Instance()->show_window(note_edit_window->GetWindowPtr());

    delete note;

}

void
NxNotepad::view_callback(Fl_Widget * fl, void *o)
{
    NxNote *n = 0;
    if (Fl::event_clicks()) {
	NxApp::Instance()->set_catlist_window((Fl_Window *) note_edit_window->
					      GetEditCategoryWindowPtr());
	n = (NxNote *) results_table->selected();

	if (n) {
	    char tempCat[CAT_LEN];
	    memset(tempCat, 0, CAT_LEN);

	    strcpy(tempCat, n->szCategory);
	    if (!tempCat[0]) {
		strcpy(tempCat, "Unfiled");
	    }
	    g_editor->Clear();
	    if (n->szFile[0] != '^') {
		FILE *fd = fopen(n->szFile, "r");
		g_editor->LoadFrom(fd);
		g_editor->MoveTo(0, 0);
		fclose(fd);
	    }
	    edit_category->label(tempCat);
	}

	g_SearchFlag = false;
	new_note_ = false;
	select_note(n);
	NxApp::Instance()->show_window(note_edit_window->GetWindowPtr());

    }

    Fl::event_clicks(0);
}

void
NxNotepad::note_tree_callback(Fl_Widget * fl, long l)
{
    if (tree->selection_count() > 1)
	tree->unselect();

    if (Fl::event_clicks()) {

	edit_callback(fl, l);
    }

    Fl::event_clicks(0);
}

void
NxNotepad::new_callback(Fl_Widget * fl, void *l)
{
    char tempCat[CAT_LEN];

    tree->unselect();
    new_note_ = true;

    NxApp::Instance()->set_catlist_window((Fl_Window *) note_edit_window->
					  GetEditCategoryWindowPtr());

    NxNote *n = new NxNote;

    memset(tempCat, 0, CAT_LEN);
    memset(n->szCategory, 0, CAT_LEN);
    memset(n->szTitle, 0, TITLE_LEN);

    strcpy(n->szFile, "^");
    strcpy(n->szTitle, "Untitled");


    if (0 == strcmp("All", note_category->label())) {
	AllFlag = true;
	strcpy(tempCat, "Unfiled");
    } else {
	strcpy(tempCat, note_category->label());
    }

    g_editor->Clear();

    fill_categories();

    edit_category->label(tempCat);
    edit_category->hide();
    edit_category->show();
    note_category->label(tempCat);

    delete(n);
    n = 0;

    NxApp::Instance()->show_window(note_edit_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxNotepad::edit_callback(Fl_Widget * fl, long l)
{
    Fl_Toggle_Node *node = tree->selected();

    fill_categories();
    NxApp::Instance()->set_catlist_window((Fl_Window *) note_edit_window->
					  GetEditCategoryWindowPtr());
    if (node) {
	char tempCat[CAT_LEN];
	memset(tempCat, 0, CAT_LEN);

	strcpy(tempCat, ((NxNote *) node->user_data())->szCategory);
	if (!tempCat[0]) {
	    strcpy(tempCat, "Unfiled");
	}
	g_editor->Clear();
	DPRINT("Filename [%s]\n", ((NxNote *) node->user_data())->szFile);
#ifdef DEBUG
	assert(((NxNote *) node->user_data())->szFile);
#endif
	if (((NxNote *) node->user_data())->szFile[0] != '^') {
	    FILE *fd = fopen(((NxNote *) node->user_data())->szFile, "r");
	    if (NULL == fd) {
		int tfd = 0;
		char tplate[128];
		snprintf(tplate, sizeof(tplate), "%s/%sXXXXXX", nx_inidir,
			 NX_INIFILE);
		tfd = mkstemp(tplate);
		if (tfd > 0) {
		    // should always have a note!
		    DPRINT("NO NOTE!\n");
		    strcpy(((NxNote *) node->user_data())->szFile, tplate);
		    DPRINT("Opening [%s]\n",
			   ((NxNote *) node->user_data())->szFile);
		    fd = fdopen(tfd, "w+");
		    DPRINT("fd [%p]\n", fd);
		}
	    }
	    if (NULL != fd) {
		new_note_ = false;
		DPRINT("fd not null\n");
		g_editor->LoadFrom(fd);
		g_editor->MoveTo(0, 0);
		fclose(fd);
	    } else {
		DPRINT("fd is null\n");
		((NxNote *) node->user_data())->szFile[0] = '^';
	    }
	}
	edit_category->label(tempCat);

	if (0 == strcmp("All", note_category->label()))
	    AllFlag = true;

	NxApp::Instance()->show_window(note_edit_window->GetWindowPtr());
    }
}

void
NxNotepad::delete_note_callback(Fl_Widget * fl, void *l)
{
    Fl_Toggle_Node *node = tree->selected();

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    if (node) {
	NxApp::Instance()->show_window(note_delete_window->GetWindowPtr(),
				       DEACTIVATE,
				       note_list_window->GetWindowPtr());
    }
}

void
NxNotepad::delete_callback(Fl_Widget * fl, long l)
{
    Fl_Toggle_Node *node = tree->selected();
    NxNote *n;
    int rec_array[1];

    if (node) {
	n = (NxNote *) node->user_data();
	char key_buf[255];
	memset(key_buf, 0, sizeof(key_buf));
	sprintf(key_buf, "%d", n->key);

	note_db->Select(NOTE_DATABASE, key_buf, NOTE_INDEX, rec_array, 1);
	note_db->DeleteRec(NOTE_DATABASE, rec_array[0]);
	unlink(n->szFile);
	if (AllFlag) {
	    set_category("All");
	    AllFlag = false;
	} else
	    set_category(n->szCategory);
    }
}

void
NxNotepad::category_callback(Fl_Widget * fl, void *l)
{
    set_category((char *) l);
}

void
NxNotepad::list_callback(Fl_Widget * fl, void *l)
{
    reset_category((char *) l);
}

void
NxNotepad::done_edit_callback(Fl_Widget * fl, void *l)
{
    Fl_Toggle_Node *node = tree->selected();

    NxNote *n = 0;
    bool delete_me = false;

    NxApp::Instance()->set_catlist_window((Fl_Window *) note_list_window->
					  GetEditCategoryWindowPtr());

    if (!g_SearchFlag) {
	if (node && !new_note_) {
	    DPRINT("not a new note\n");
	    n = ((NxNote *) node->user_data());
	} else {
	    DPRINT("have a new note\n");
	    n = new NxNote;
	    delete_me = true;
	    strcpy(n->szFile, "^");
	    new_note_ = false;
	}

	if (n) {
	    char *f = n->szFile;
	    FILE *fd = 0;

	    if (f[0] == '^') {
		char tplate[128];
		int tfd = 0;

		snprintf(tplate, sizeof(tplate), "%s/%sXXXXXX", nx_inidir,
			 NX_INIFILE);
		tfd = mkstemp(tplate);

		n->key = -1;

		if (tfd > 0) {
		    fd = fdopen(tfd, "w+");
		    strcpy(n->szFile, tplate);
		}
	    } else
		fd = fopen(f, "w+");

	    if (fd) {
		g_editor->SaveTo(fd);
		fclose(fd);
		write_note(n);
	    }
	    else 
	      fprintf(stderr, "NOTEPAD - Unable to write the file\n");

	    if (delete_me)
		delete n;
	}
    } else {
	g_SearchFlag = false;
    }

    if (AllFlag) {
	set_category("All");
	AllFlag = false;
    } else
	set_category(edit_category->label());

    NxApp::Instance()->show_window(note_list_window->GetWindowPtr());
}

void
NxNotepad::yes_delete_callback(Fl_Widget * fl, void *l)
{
    Fl_Toggle_Node *node = tree->selected();
    NxNote *n;
    int rec_array[1];

    if (node) {
	n = (NxNote *) node->user_data();
	char key_buf[255];
	memset(key_buf, 0, sizeof(key_buf));
	sprintf(key_buf, "%d", n->key);

	note_db->Select(NOTE_DATABASE, key_buf, NOTE_INDEX, rec_array, 1);
	note_db->DeleteRec(NOTE_DATABASE, rec_array[0]);
	unlink(n->szFile);
	if (AllFlag) {
	    set_category("All");
	    AllFlag = false;
	} else
	    set_category(note_category->label());
    }
    NxApp::Instance()->show_window(note_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxNotepad::no_delete_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(note_list_window->GetWindowPtr(),
				   ACTIVATE);
    save_archive_ = false;
}

void
NxNotepad::save_archive_callback(Fl_Widget * fl, void *l)
{
    save_archive_ = true;
}

void
NxNotepad::lookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(note_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   note_list_window->GetWindowPtr());
    lookup_input->value("");
    Fl::focus((Fl_Widget *) lookup_input);
}

void
NxNotepad::cancelLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(note_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxNotepad::doneLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(note_list_window->GetWindowPtr(),
				   ACTIVATE);
}

char *
NxNotepad::formatString(const NxNote * note, int pixels)
{
    int width = 0;
    int dot_width = 0;
    int idx = 0;
    char *new_string = new char[TITLE_LEN + 3];

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    width = (int) fl_width(note->szTitle);
    dot_width = (int) fl_width("...");

    if (width <= pixels) {
	return (char *) note->szTitle;
    } else {
	while (width > pixels) {
	    idx++;
	    memset(new_string, 0, TITLE_LEN + 3);
	    strncpy(new_string, note->szTitle, strlen(note->szTitle) - idx);
	    width = (int) fl_width(new_string) + dot_width;
	}
	sprintf(new_string, "%s...", new_string);
	return new_string;
    }
    return (char *) note->szTitle;
}

NxNote *
NxNotepad::search(const char *searchVal)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;
    char *needle = strup(searchVal, strlen(searchVal));

    if (0 == cur_record) {
	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}
	// Select all records
	note_db->Select(NOTE_DATABASE, rec_array, 255);
    }

    if (255 == cur_record) {
	cur_record = 0;
	delete[]needle;
	needle = 0;
	return NULL;
    }

    bool found = false;
    NxNote *note = new NxNote;

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

	note_db->Extract(NOTE_DATABASE, jdx, NOTE_CAT, catid);
	note_db->Select(CAT_DATABASE, catid, CAT_INDEX, catid_array, 1);
	if (-1 != catid_array[0])
	    note_db->Extract(CAT_DATABASE, catid_array[0], CAT_DESC,
			     note->szCategory);
	else
	    strcpy(note->szCategory, "Unfiled");

	note_db->Extract(NOTE_DATABASE, jdx, NOTE_FILE, note->szFile);
	char *haystack = strup(note->szFile, FILE_LEN);
	//if (strstr(haystack, needle))
	//      found = true;
	//delete[] haystack;
	//haystack = 0;

	//also need to look through the file
	found |= searchFile(searchVal, note->szFile);

	note_db->Extract(NOTE_DATABASE, jdx, NOTE_DESC, note->szTitle);
	haystack = strup(note->szTitle, TITLE_LEN);

	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	if (found) {
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
NxNotepad::searchLookup_callback(Fl_Widget * fl, void *l)
{
    int total_found = 0;
    char *searchVal = (char *) lookup_input->value();
    char *needle = strup(searchVal, strlen(searchVal));
    NxNote *note;

    results_table->Init(255);
    while ((note = search(searchVal)) != NULL) {
	results_table->Add(total_found, note);
	char *label = formatString(note, results_table->text_width());
	results_table->set_value(total_found, 0, label);
	total_found++;
    }
    delete[]needle;
    needle = 0;

    results_table->rows(total_found);
    if (total_found == 0) {
	results_message->show();
	results_table->hide();
    } else {
	results_message->hide();
	results_table->show();
    }

    NxApp::Instance()->show_window(note_list_window->GetWindowPtr());
    NxApp::Instance()->show_window(note_results_window->GetWindowPtr(),
				   DEACTIVATE,
				   note_list_window->GetWindowPtr());

}


void
NxNotepad::write_archive_copy()
{
    fprintf(stderr, "Would be doing write_archive_copy\n");
}

void
NxNotepad::write_note(NxNote * note)
{
    char *buf = new char[MAXRECSIZ];
    int rec_array[1];
    int cat_array[1];
    char value[50];
    char title[TITLE_LEN];
    FILE *fp;
    int c;
    int count = 0;
    bool all_space = true;
    unsigned int idx;

    rec_array[0] = -1;
    if (0 == strcmp("All", edit_category->label())) {
	strcpy(note->szCategory, "Unfiled");
    } else {
	strcpy(note->szCategory, edit_category->label());
    }

    char key_buf[255];
    memset(key_buf, 0, sizeof(key_buf));
    sprintf(key_buf, "%d", note->key);

    note_db->Select(NOTE_DATABASE, key_buf, NOTE_INDEX, rec_array, 1);
    DPRINT("key is [%d]\n", note->key);

    cat_array[0] = -1;
    note_db->Select(CAT_DATABASE, note->szCategory, CAT_DESC, cat_array, 1);
    note_db->Extract(CAT_DATABASE, cat_array[0], CAT_INDEX, value);

    fp = fopen(note->szFile, "r");
    memset(title, 0, sizeof(title));
    if (NULL != fp) {
	while ((c = fgetc(fp)) != EOF) {
	    title[count] = c;
	    if (c == '\n' || count >= TITLE_LEN - 1) {
		title[count] = '\0';
		break;
	    }
	    count++;
	}

	if (title[0]) {
	    strcpy(note->szTitle, title);
	} else {
	    strcpy(note->szTitle, "Untitled");
	}
	fclose(fp);
    } else {
	strcpy(note->szTitle, "Untitled");
    }

    for (idx = 0; idx < strlen(title); idx++) {
	if (!isspace(title[idx])) {
	    all_space = false;
	    break;
	}
    }

    if (all_space)
	strcpy(note->szTitle, "Untitled");

    if (-1 == rec_array[0]) {
	memset(buf, 0, MAXRECSIZ);
	note_key =
	    NxApp::Instance()->GetKey(note_db, NOTE_DATABASE, NOTE_INDEX) + 1;
	put16(&buf[noteFields[NOTE_INDEX].offset], note_key);
	strcpy(&buf[noteFields[NOTE_FILE].offset], note->szFile);
	put16(&buf[noteFields[NOTE_CAT].offset], atoi(value));
	strcpy(&buf[noteFields[NOTE_DESC].offset], note->szTitle);
	put16(&buf[noteFields[NOTE_ARCH].offset], 0);

	note_db->Insert(NOTE_DATABASE, buf);
	//fprintf(stderr, "Inserting: name[%s] cat[%s] title[%s] archive[%d]\n",
	//      note->szFile, note->szCategory, note->szTitle, 0);

    } else {
	note_db->Extract(NOTE_DATABASE, rec_array[0], buf);
	put16(&buf[noteFields[NOTE_CAT].offset], atoi(value));
	strcpy(&buf[noteFields[NOTE_DESC].offset], note->szTitle);
	note_db->Edit(NOTE_DATABASE, rec_array[0], buf);
	//fprintf(stderr, "Editing: name[%s] cat[%s] title[%s]\n",
	//              note->szFile, note->szCategory, note->szTitle);
    }
    delete[]buf;
}

void
NxNotepad::clear_tree()
{
    Fl_Toggle_Node *n = tree->traverse_start();
    while (n) {
	delete(char *) n->user_data();
	tree->remove(n);
	n = tree->traverse_start();
    }
}

void
NxNotepad::set_category(const char *szCat)
{
    char szRealCategory[CAT_LEN];

    memset(szRealCategory, 0, sizeof(szRealCategory));
    if (!szCat[0]) {
	strcpy(szRealCategory, "All");
    } else {
	strcpy(szRealCategory, szCat);
    }

    clear_tree();
    add_items(tree, szRealCategory);
    fill_categories();

    edit_category->label(szRealCategory);
    edit_category->hide();
    edit_category->show();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();
}

void
NxNotepad::reset_category(char *szCat)
{
    char szRealCategory[CAT_LEN];

    memset(szRealCategory, 0, sizeof(szRealCategory));
    if (!szCat[0]) {
	strcpy(szRealCategory, "All");
    } else {
	strcpy(szRealCategory, szCat);
    }

    fill_categories();

    edit_category->label(szRealCategory);
    edit_category->hide();
    edit_category->show();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();
}

void
NxNotepad::insert_default_categories()
{
    char buf[MAXRECSIZ];
    int idx = 0;

    memset(buf, 0, sizeof(buf));
    put16(&buf[catFields[CAT_INDEX].offset], idx++);
    strcpy(&buf[catFields[CAT_DESC].offset], "Unfiled");
    note_db->Insert(CAT_DATABASE, buf);
    memset(buf, 0, sizeof(buf));
    put16(&buf[catFields[CAT_INDEX].offset], idx++);
    strcpy(&buf[catFields[CAT_DESC].offset], "Business");
    note_db->Insert(CAT_DATABASE, buf);
    memset(buf, 0, sizeof(buf));
    put16(&buf[catFields[CAT_INDEX].offset], idx++);
    strcpy(&buf[catFields[CAT_DESC].offset], "Personal");
    note_db->Insert(CAT_DATABASE, buf);
}

void
NxNotepad::insert_default_note()
{
    char buf[MAXRECSIZ];
    int cat_array[1];
    FILE *fp;
    char file_name[FILE_LEN];

    cat_array[0] = 0;
    memset(buf, 0, sizeof(buf));
    put16(&buf[noteFields[NOTE_INDEX].offset], 0);
    sprintf(file_name, "%scen.txt", nx_inidir);
    strcpy(&buf[noteFields[NOTE_FILE].offset], file_name);
    put16(&buf[noteFields[NOTE_CAT].offset], 1);
    strcpy(&buf[noteFields[NOTE_DESC].offset], "How to get to Mount Doom");
    put16(&buf[noteFields[NOTE_ARCH].offset], 0);
    note_db->Insert(NOTE_DATABASE, buf);

    fp = fopen(file_name, "w+");
    if (fp) {
	fputs
	    ("How to get to Mount Doom\nGo east through Gondor until you see Mordor. Get past the gate and throw the ring into the lava.",
	     fp);
	fclose(fp);
    }
}

void
NxNotepad::open_cat_database()
{
    if (!note_db->Open(CAT_DATABASE, &catFile, catFields, CAT_INDEX)) {
	if (note_db->Create(CAT_DATABASE, &catFile, catFields, CAT_INDEX)) {
	    if (!note_db->Open(CAT_DATABASE, &catFile, catFields, CAT_INDEX)) {
		exit(-1);
	    }
	    insert_default_categories();
	} else {
	    exit(-1);
	}
    }
}

void
NxNotepad::open_note_database()
{
    if (!note_db->Open(NOTE_DATABASE, &noteFile, noteFields, NOTE_INDEX)) {
	if (note_db->Create(NOTE_DATABASE, &noteFile, noteFields, NOTE_INDEX)) {
	    if (!note_db->
		Open(NOTE_DATABASE, &noteFile, noteFields, NOTE_INDEX)) {
		exit(-1);
	    }
#ifdef CONFIG_SAMPLES
	    insert_default_note();
#endif
	} else {
	    exit(-1);
	}
    }
}

void
NxNotepad::fill_categories()
{

    char ret_buf[CAT_LEN];
    int rec_count = note_db->NumRecs(CAT_DATABASE);

    for (int idx = 0; idx < CAT_NUM; idx++) {
	cat_list[idx]->clear();
	cat_list[idx]->add("All");
	for (int jdx = 1; jdx <= rec_count; jdx++) {
	    memset(ret_buf, 0, sizeof(ret_buf));
	    if (note_db->Extract(CAT_DATABASE, jdx, CAT_DESC, ret_buf)) {
		cat_list[idx]->add(ret_buf);
	    }

	}
	cat_list[idx]->label(const_cast < char *>(cat_list[idx]->text(0)));
    }
    note_list_window->GetEditCategoryPtr()->clear_tree();
    note_list_window->GetEditCategoryPtr()->add_items(note_list_window->
						      GetEditCategoryPtr()->
						      get_category_tree());
    note_edit_window->GetEditCategoryPtr()->clear_tree();
    note_edit_window->GetEditCategoryPtr()->add_items(note_edit_window->
						      GetEditCategoryPtr()->
						      get_category_tree());
}
