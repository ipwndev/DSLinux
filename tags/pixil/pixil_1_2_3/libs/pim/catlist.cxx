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


#include <stdio.h>
#include <string.h>
#include "catlist.h"
#include "nxapp.h"

static void
select_callback(Fl_Widget * fl, long l)
{
    //cerr << "select_callback()\n";
    NxCategoryList *list = (NxCategoryList *) fl;
    memset(list->m_CatBuf, 0, sizeof(list->m_CatBuf));
    strcpy(list->m_CatBuf, list->text());
    if (0 == strcasecmp("Edit", list->m_CatBuf)) {
	NxApp::Instance()->show_window(NxApp::Instance()->
				       get_catlist_window());
	list->label(list->get_category_buf());
    } else {
	strcpy(list->get_category_buf(), list->m_CatBuf);
	list->label(list->m_CatBuf);
	list->hide();
	list->show();
	if (list->m_Cb)
	    (*list->m_Cb) (list, (void *) list->m_CatBuf);

    }

    list->set_value();
}

char *
NxCategoryList::label()
{
    return const_cast < char *>(Fl_Widget::label());
}

void
NxCategoryList::set_value()
{
    for (int idx = 0; idx < size() - 1; idx++) {
	if (0 == strcmp(label(), text(idx))) {
	    value(idx);
	}
    }
}

void
NxCategoryList::label(char *szLabel)
{
    strcpy(m_CatBuf, szLabel);
    Fl_Widget::label(m_CatBuf);

    if (0 != strcasecmp("Edit", m_CatBuf))
	set_value();

}

int
NxCategoryList::add(const char *value)
{
    int ret = 0;

    remove(size() - 2);
    remove(size() - 2);
    ret = NxMenuButton::add(value);

    if (0 > ret)
	return ret;
    ret = NxMenuButton::add("Unfiled");
    if (0 > ret)
	return ret;
    ret = NxMenuButton::add("Edit");
    return ret;
}

void
NxCategoryList::set_category_buf(char *cat_buf)
{
    strcpy(NxCategoryBuf, cat_buf);
}


NxCategoryList::NxCategoryList(int x, int y, int w, int h, NxDb * db,
			       string _dbName):
NxMenuButton(x, y, w, h, "")
{

    add("All");
    box(FL_SHADOW_BOX);

    // Extract records from category database.
    int nLastItem = db->NumRecs(_dbName);
    int key = 0;
    int fieldNo = 1;
    char *new_category = new char[255];

    strcpy(NxCategoryBuf, "All");

    for (int i = 0; i < nLastItem; i++) {

	char find[4];
	sprintf(find, "%d", i);
	int get_recno[1];
	get_recno[0] = -1;

	db->Select(_dbName, find, key, get_recno, 1);
	int recno = get_recno[0];

	if (recno == -1) {
	    break;
	}

	db->Extract(_dbName, recno, fieldNo, new_category);
	add(new_category);

    }

    delete[]new_category;
    new_category = 0;

    callback(select_callback);
    add("Unfiled");
    add("Edit");
    select(0);

}
