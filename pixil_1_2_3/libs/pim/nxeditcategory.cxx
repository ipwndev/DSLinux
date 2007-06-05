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
#include <nxapp.h>
#include <icons/folder_small.xpm>
#include <stdio.h>
#include <string.h>

#define CAT_INDEX 0
#define CAT_FIELD 1

void (*update_tree_items_) (const char *);

void
NxEditCategory::list_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;

    if (edit_cat->get_category_tree()->selection_count() > 1)
	edit_cat->get_category_tree()->unselect();

    if (Fl::event_clicks())
	rename_cb(fl, o);
}

void
NxEditCategory::done_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    edit_cat->get_category_tree()->unselect();
    (*update_tree_items_) (edit_cat->get_menu_list()->label());
    NxApp::Instance()->show_window(edit_cat->get_show_window()->
				   GetWindowPtr());
}

void
NxEditCategory::new_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    edit_cat->get_new_cat_input()->value("");
    NxApp::Instance()->show_window(edit_cat->get_new_cat_window()->
				   GetWindowPtr(), DEACTIVATE,
				   edit_cat->GetWindowPtr());
}

void
NxEditCategory::rename_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    Fl_Toggle_Node *node = edit_cat->get_category_tree()->selected();


    if (node) {
	edit_cat->get_new_cat_input()->value("");
	NxApp::Instance()->show_window(edit_cat->get_rename_cat_window()->
				       GetWindowPtr(), DEACTIVATE,
				       edit_cat->GetWindowPtr());
    }
}

void
NxEditCategory::done_new_cat_cb(Fl_Widget * fl, void *o)
{
    char value[255];
    bool match = false;
    int n_count = 0;
    int len = 0;

    NxEditCategory *edit_cat = (NxEditCategory *) o;

    if (0 == strcasecmp("Edit", edit_cat->get_new_cat_input()->value()))
	return;

    Fl_Toggle_Node *n = edit_cat->get_category_tree()->traverse_start();
    while (n) {
	n = edit_cat->get_category_tree()->traverse_forward();
	n_count++;
    }
    if (0 == strcmp("", edit_cat->get_new_cat_input()->value()))
	return;
	/****
		check input for / or \ and change it to -
		Why? Fltk uses a / as a sub-menu and skips \\
	*****/
    len = strlen(edit_cat->get_new_cat_input()->value());
    char *new_value = (char *) (edit_cat->get_new_cat_input()->value());

    for (int jdx = 0; jdx <= len; jdx++) {
	int c = edit_cat->get_new_cat_input()->value()[jdx];
	if (c == '/' || c == '\\') {
	    new_value[jdx] = '-';
	}
    }
    edit_cat->get_new_cat_input()->value(new_value);

    while (!match) {		// find new key
	for (int idx = 1;
	     idx <=
	     edit_cat->get_cat_db()->NumRecs(edit_cat->get_catdb_name());
	     idx++) {
	    edit_cat->get_cat_db()->Extract(edit_cat->get_catdb_name(), idx,
					    1, value);
	    if (0 == strcmp(value, edit_cat->get_new_cat_input()->value())) {
		match = true;
		edit_cat->get_new_cat_input()->value("");
		return;
	    }
	}
	break;
    }

    // match is false
    edit_cat->get_category_tree()->add_next((char *) edit_cat->
					    get_new_cat_input()->value(), 0,
					    edit_cat->get_folder_pix());
    edit_cat->get_category_tree()->traverse_up();
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
    n = edit_cat->get_category_tree()->traverse_start();
    while (n) {
	if (0 == strcmp(edit_cat->get_new_cat_input()->value(), n->label()))
	    edit_cat->get_category_tree()->select_range(n, n, 0);
	n = edit_cat->get_category_tree()->traverse_forward();
    }
    edit_cat->new_cat_update((char *) edit_cat->get_new_cat_input()->value());
}

void
NxEditCategory::cancel_new_cat_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
}

void
NxEditCategory::done_rename_cat_cb(Fl_Widget * fl, void *o)
{
    char old_name[MAX_NAME_SIZE];
    char new_name[MAX_NAME_SIZE];
    bool match = false;
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    Fl_Toggle_Node *node = edit_cat->get_category_tree()->selected();
    int len = 0;

    if (0 == strcmp("", edit_cat->get_rename_cat_input()->value())) {
	return;
    }

    if (0 == strcasecmp("Edit", edit_cat->get_rename_cat_input()->value()))
	return;

	/***
		check input for / or \ and change it to -
		Why? Fltk uses a / as a sub-menu and skips \
	****/
    len = strlen(edit_cat->get_rename_cat_input()->value());
    char *new_value = (char *) (edit_cat->get_rename_cat_input()->value());

    for (int idx = 0; idx <= len; idx++) {
	int c = edit_cat->get_rename_cat_input()->value()[idx];
	if (c == '/' || c == '\\')
	    new_value[idx] = '-';
    }

    if (node) {
	memset(old_name, 0, sizeof(old_name));
	strcpy(old_name, node->label());
	Fl_Toggle_Node *n = edit_cat->get_category_tree()->traverse_start();
	while (n) {
	    if (0 == strcmp(n->label(), new_value)) {
		match = true;
		edit_cat->get_rename_cat_input()->value("");
		return;
	    }
	    n = edit_cat->get_category_tree()->traverse_forward();
	}
	if (false == match)
	    node->label(new_value);
	edit_cat->get_category_tree()->hide();
	edit_cat->get_category_tree()->show();
	memset(new_name, 0, sizeof(new_name));
	strcpy(new_name, new_value);
	edit_cat->rename_cat_update(old_name, new_name);
    }
    node = edit_cat->get_category_tree()->traverse_start();
    while (node) {
	if (0 == strcmp(new_name, node->label()))
	    edit_cat->get_category_tree()->select_range(node, node, 0);
	node = edit_cat->get_category_tree()->traverse_forward();
    }
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
}

void
NxEditCategory::cancel_rename_cat_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
}

void
NxEditCategory::delete_cb(Fl_Widget * fl, void *o)
{
    //Delete category
}

void
NxEditCategory::delete_cat_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    Fl_Toggle_Node *node = edit_cat->get_category_tree()->selected();

    if (node) {
	NxApp::Instance()->show_window(edit_cat->get_delete_cat_window()->
				       GetWindowPtr(), DEACTIVATE,
				       edit_cat->GetWindowPtr());
    }
}

void
NxEditCategory::delete_yes_cat_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    Fl_Toggle_Node *node = edit_cat->get_category_tree()->selected();
    int rec_array[255];
    int rec[1];
    int cat_array[1];
    int size;
    int idx = 0;
    char ret_buf[255];
    char field_buf[8];
    char rec_buf[MAXRECSIZ];
    int cat_index = 0;

    for (idx = 0; idx < 255; idx++) {
	rec_array[idx] = -1;
    }
    cat_array[0] = -1;

    size = edit_cat->get_menu_list()->size();
    //fprintf(stderr, "delete_yes_cb: size[%d]\n", size);
    for (int jdx = 0; jdx < size - 2; jdx++) {
	//fprintf(stderr, "delete_yes_cb: doing strcmp jdx[%d]\n", jdx);
	if (0 == strcmp(node->label(), edit_cat->get_menu_list()->text(jdx))) {
	    edit_cat->get_menu_list()->remove(jdx);
	}
    }
    // get cat record that is to be removed
    edit_cat->get_cat_db()->Select(edit_cat->get_catdb_name(), node->label(),
				   1, rec_array, 1);
    cat_index = rec_array[0];
    // get the index value
    edit_cat->get_cat_db()->Extract(edit_cat->get_catdb_name(), rec_array[0],
				    0, ret_buf);
    int remove_num = atoi(ret_buf);

    // get the index num for "Unfiled" from the cataegory database
    edit_cat->get_cat_db()->Select(edit_cat->get_catdb_name(), "Unfiled", 1,
				   cat_array, 1);
    edit_cat->get_cat_db()->Extract(edit_cat->get_catdb_name(), cat_array[0],
				    0, field_buf);
    int num = atoi(field_buf);

    for (idx = 0; idx < 255; idx++) {
	rec_array[idx] = -1;
    }

    // get records that have the cat index that is to be removed
    edit_cat->get_cat_db()->Select(edit_cat->get_notedb_name(), ret_buf,
				   CAT_FIELD, rec_array, 255);
    int rec_count = 0;
    for (idx = 0; idx < 255; idx++) {
	if (-1 != rec_array[idx])
	    rec_count++;
    }

    // go through the records editing the ones with cat index
    for (idx = 0; idx < rec_count; idx++) {
	rec[0] = -1;
	NxDb *db = edit_cat->get_cat_db();
	string notedb_name = edit_cat->get_notedb_name();

	db->Select(notedb_name, ret_buf, CAT_FIELD, rec, 1);

	// get the field that has the cat id
	//fprintf(stderr, "delete_yes_cb: recno[%d]\n", rec[0]);
	if (-1 != rec[0]) {
	    edit_cat->get_cat_db()->Extract(edit_cat->get_notedb_name(),
					    rec[0], CAT_FIELD, rec_buf);
	    int cat_field = atoi(rec_buf);
	    //fprintf(stderr, "delete_yes_cb: cat_field[%d]\n", cat_field);
	    //fprintf(stderr, "delete_yes_cb: num[%d]\n", num);
	    //fprintf(stderr, "delete_yes_cb: remove_num[%d]\n", remove_num);
	    // compare it to the field to remove
	    if (remove_num == cat_field) {
		// edit the record with cat id to to the new cat id
		edit_cat->get_cat_db()->Extract(edit_cat->get_notedb_name(),
						rec[0], rec_buf);
		put16(&rec_buf[edit_cat->get_note_field()[CAT_FIELD].offset],
		      num);
		edit_cat->get_cat_db()->Edit(edit_cat->get_notedb_name(),
					     rec[0], rec_buf);
	    }
	}
    }
    // delete the category from the database
    edit_cat->get_cat_db()->DeleteRec(edit_cat->get_catdb_name(), cat_index);
    edit_cat->get_category_tree()->remove(node);

    edit_cat->get_menu_list()->label("Unfiled");
    (*update_tree_items_) ("Unfiled");
    edit_cat->get_category_tree()->unselect();
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
}

void
NxEditCategory::delete_no_cat_cb(Fl_Widget * fl, void *o)
{
    NxEditCategory *edit_cat = (NxEditCategory *) o;
    NxApp::Instance()->show_window(edit_cat->GetWindowPtr(), ACTIVATE);
}

void
NxEditCategory::clear_tree()
{
    Fl_Toggle_Node *n = category_tree_->traverse_start();
    while (n) {
	delete(char *) n->user_data();
	category_tree_->remove(n);
	n = category_tree_->traverse_start();
    }
}

void
NxEditCategory::add_items(Fl_Toggle_Tree * t)
{
    char inbuf[255];
    int idx;
    int rec_count = cat_db_->NumRecs(catdb_name_);
    folderSmall_ = new Fl_Pixmap(folder_small);

    for (idx = 1; idx <= rec_count; idx++) {
	memset(inbuf, 0, sizeof(inbuf));
	if (cat_db_->Extract(catdb_name_, idx, 1, inbuf)) {
	    if (0 == strcmp("Unfiled", inbuf) || 0 == strcmp("All", inbuf))
		continue;
	    t->add_next(inbuf, 0, folderSmall_);
	    t->traverse_up();
	}
    }
}

void
NxEditCategory::new_cat_update(char *new_value)
{
    int size;
    //int index = cat_db_->NumRecs(catdb_name_) + 1;
    int index =
	(NxApp::Instance()->
	 GetKey(cat_db_, (char *) catdb_name_.c_str(), 0)) + 1;
    char rec[MAXRECSIZ];

    size = menu_list_->size();
    menu_list_->add(new_value);

    memset(rec, 0, sizeof(rec));
    put16(&rec[cat_field_[0].offset], index);
    strcpy(&rec[cat_field_[1].offset], new_value);
    cat_db_->Insert(catdb_name_, rec);
    new_cat_input_->value("");
}

void
NxEditCategory::rename_cat_update(char *old_name, char *new_name)
{
    int size;
    int rec_no[1];
    char rec[MAXRECSIZ];

    rec_no[0] = -1;

    size = menu_list_->size();
    for (int jdx = 0; jdx < size - 1; jdx++) {
	if (0 == strcmp(old_name, menu_list_->text(jdx))) {
	    menu_list_->replace(jdx, new_name);
	}
    }
    if (0 == strcmp(old_name, menu_list_->label())) {
	menu_list_->label(new_name);
    }

    rename_cat_input_->value("");
    cat_db_->Select(catdb_name_, old_name, 1, rec_no, 1);
    if (-1 != rec_no[0]) {
	cat_db_->Extract(catdb_name_, rec_no[0], rec);
	strcpy(&rec[cat_field_[CAT_FIELD].offset], new_name);
	cat_db_->Edit(catdb_name_, rec_no[0], rec);
	(*update_tree_items_) (new_name);
    }
}

NxEditCategory::NxEditCategory(NxDb * catDb, string catdb_name,
			       string notedb_name,
			       void (*update_items) (const char *category))
    :
NxPimWindow(W_X, W_Y, W_W, W_H)
{

    int ww[3] = { 150, 200, 0 };

    g_NxEditCatWin = this;
    catdb_name_ = catdb_name;
    notedb_name_ = notedb_name;
    cat_db_ = catDb;
    //fprintf(stderr, "cat_db_ [%p]\n", cat_db_);
    cat_field_ = cat_db_->GetField(catdb_name_);
    cat_des_ = cat_db_->GetFilDes(catdb_name_);
    note_field_ = cat_db_->GetField(notedb_name_);
    note_des_ = cat_db_->GetFilDes(notedb_name_);
    update_tree_items_ = update_items;

    pim_window = get_pim_window();
    {
	NxButton *o = new NxButton(MB_X, MB_Y, MB_W, MB_H, "Edit Category");
	o->movable(false);
	add(o);
    }
    {
	NxScroll *o = category_list_ =
	    new NxScroll(-1, 31, W_W + 2, BUTTON_Y - 38);
	o->align(FL_ALIGN_TOP_LEFT);
	o->movable(false);
	{
	    //category_list_->scrollbar.size(12, category_list_->scrollbar.h());
	    category_tree_ = new Fl_Toggle_Tree(0, 31, W_W, 10);
	    category_tree_->callback(list_cb, this);
	}
	o->end();
    }
    add(category_list_);

    done_button_ =
	new NxButton(1, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT, "Done");
    done_button_->callback(done_cb, this);
    add(done_button_);

    new_button_ =
	new NxButton(59, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT, "New");
    new_button_->callback(new_cb, this);
    add(new_button_);

    rename_button_ =
	new NxButton(118, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
		     "Rename");
    rename_button_->callback(rename_cb, this);
    add(rename_button_);

    delete_button_ =
	new NxButton(177, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
		     "Delete");
    delete_button_->callback(delete_cat_cb, this);
    add(delete_button_);

    category_tree_->column_widths(ww);
    add_items(category_tree_);

    new_cat_win_ =
	new NxPimPopWindow("New Category",
			   NxApp::Instance()->getGlobalColor(APP_FG));
    //fprintf(stderr, "NxEditCategory: new_cat_win_->GetWindowPtr[%p]\n",new_cat_win_->GetWindowPtr());
    NxApp::Instance()->add_window((Fl_Window *) new_cat_win_->GetWindowPtr());

    {
	new_cat_input_ =
	    new NxInput(BUTTON_X, 55, 150, 20, "Enter a new category name:");
	new_cat_input_->align(FL_ALIGN_TOP_LEFT);
	new_cat_input_->maximum_size(MAX_NAME_SIZE);
	new_cat_win_->add((Fl_Widget *) new_cat_input_);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(done_new_cat_cb, this);
	new_cat_win_->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancel_new_cat_cb, this);
	new_cat_win_->add((Fl_Widget *) o);
    }

    rename_cat_win_ =
	new NxPimPopWindow("Rename Category",
			   NxApp::Instance()->getGlobalColor(APP_FG));
    NxApp::Instance()->add_window((Fl_Window *) rename_cat_win_->
				  GetWindowPtr());
    {
	rename_cat_input_ =
	    new NxInput(BUTTON_X, 55, 150, 20, "Enter a new category name:");
	rename_cat_input_->align(FL_ALIGN_TOP_LEFT);
	rename_cat_input_->maximum_size(MAX_NAME_SIZE);
	rename_cat_win_->add((Fl_Widget *) rename_cat_input_);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(done_rename_cat_cb, this);
	rename_cat_win_->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancel_rename_cat_cb, this);
	rename_cat_win_->add((Fl_Widget *) o);
    }

    delete_cat_win_ =
	new NxPimPopWindow("Delete Category",
			   NxApp::Instance()->getGlobalColor(APP_FG));
    NxApp::Instance()->add_window((Fl_Window *) delete_cat_win_->
				  GetWindowPtr());
    {
	Fl_Box *o = new Fl_Box(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			       "Are you sure you want to delete this category?");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	delete_cat_win_->add((Fl_Widget *) o);
    }
    {
	Fl_Box *o = new Fl_Box(BUTTON_X, 75, W_W - BUTTON_X - 15, 0,
			       "All notes whithin this category become \"Unfiled\".");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	delete_cat_win_->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(delete_yes_cat_cb, this);
	delete_cat_win_->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(delete_no_cat_cb, this);
	delete_cat_win_->add((Fl_Widget *) o);
    }
    pim_window->end();
    set_pim_window(pim_window);
}
