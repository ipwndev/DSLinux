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


/* derived from Flv_List.cxx from the FLEK tookit */
/* Original copyright:  Copyright (C) 1999 Laurence Charlton */
/* Licenced under the LGPL */

#include <FL/Flv_Table_Child.H>

#include "nxapp.h"
#include <iostream>

Flv_Table_Child::Flv_Table_Child(int X, int Y, int W, int H, const char *l,
				 int _col0, int _col1, int _col2, int _col3):
Flv_Table(X, Y, W, H, l)
{
    move = true;
    save_h = H;
    numRows = 0;
    numCols = 0;
    lastNumRecs = 0;
    val = 0;
    col0 = _col0;
    col1 = _col1;
    col2 = _col2;
    col3 = _col3;
    has_scrollbar(FLVS_VERTICAL);
    feature(FLVF_ROW_SELECT);
    global_style.align(FL_ALIGN_LEFT);	//      Left alignment
    global_style.height(15);

    // Global App Colors
    global_style.foreground(NxApp::Instance()->getGlobalColor(APP_FG));
    global_style.background(NxApp::Instance()->getGlobalColor(HILIGHT_LITE));
    hilight_dark = NxApp::Instance()->getGlobalColor(HILIGHT_DARK);
    scrollbar.color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
    scrollbar.selection_color(NxApp::Instance()->getGlobalColor(SCROLL_FACE));
    hscrollbar.color(NxApp::Instance()->getGlobalColor(SCROLL_TRAY));
    hscrollbar.selection_color(NxApp::Instance()->
			       getGlobalColor(SCROLL_FACE));
    images = 0;
}

Flv_Table_Child::~Flv_Table_Child()
{
    if (node) {
	delete[]node;
	node = 0;
    }

    if (val) {
	delete[]val;
	val = 0;
    }

    node_image *next_image = images->next;
    node_image *last_image = 0;

    while (next_image) {

	last_image = next_image;
	next_image = next_image->next;
	delete last_image;

    }

    delete images;

}

//
// Image Management
//

void
Flv_Table_Child::_set_image(Fl_Pixmap * image, int R, int C, int id)
{

    if (!images) {

	images = new node_image;
	images->id = id;
	images->R = R;
	images->C = C;
	images->image_data = image;
	images->next = 0;

    } else {

	node_image *next_image = images;

	while (next_image) {
	    next_image = next_image->next;
	}

	next_image = new node_image;
	next_image->id = id;
	next_image->image_data = image;
	next_image->next = 0;

    }

}

void
Flv_Table_Child::set_image(Fl_Pixmap * image, int R, int C, int id)
{
    _set_image(image, R, C, id);
}

Fl_Pixmap *
Flv_Table_Child::get_image(int id)
{

    node_image *next_image = images;

    while (next_image) {

	if (next_image->id == id)
	    return next_image->image_data;

	next_image = next_image->next;

    }

    return 0;

}

int
Flv_Table_Child::find_image(int R, int C)
{

    node_image *next_image = images;

    while (next_image) {

	// Row and Col
	if ((next_image->R == R) && (next_image->C == C))
	    return next_image->id;
	// Col
	if (next_image->C == C)
	    return next_image->id;
	// Row
	if (next_image->R == R)
	    return next_image->id;

	next_image = next_image->next;

    }

    return 0;

}

char *
Flv_Table_Child::get_value(int R, int C, int &image)
{

    image = find_image(R, C);
    return val[R].col[C]._data;
}

void
Flv_Table_Child::set_value(int R, int C, char *data)
{

    strcpy(val[R].col[C]._data, data);

}

void
Flv_Table_Child::Init(int numRecs)
{

    //
    // node_data
    //

    delete[]node;
    node = 0;

    node = new node_data[numRecs];

    //
    // table_row
    //

    if (val) {

	for (int r = 0; r < lastNumRecs; r++)

	    for (int c = 0; c < numCols; c++) {

		delete[]val[r].col[c]._data;
		val[r].col[c]._data = 0;

	    }


	delete[]val;
	val = 0;

    }

    val = new table_row[numRecs];
    lastNumRecs = numRecs;

    for (int r = 0; r < numRecs; r++) {

	val[r].col = new col_data[numCols];

	for (int c = 0; c < numCols; c++) {

	    val[r].col[c]._data = new char[255];

	}

    }

    numRows = numRecs;

}

void
Flv_Table_Child::Add(int row, void *data)
{

    node[row].data = data;

}

void *
Flv_Table_Child::data(int row)
{
    return node[row].data;
}

void *
Flv_Table_Child::selected()
{

    int curRow = row();

    if (rows())
	return node[curRow].data;
    else
	return 0;

}

//      Note: This is so flexible, you don't *have* to use style
//      if you'd rather program the conditions...
void
Flv_Table_Child::get_style(Flv_Style & s, int R, int C)
{
    const char *st;
    int pixmap = 0;

    Flv_Table::get_style(s, R, C);	//       Get standard style

    if (R < 0)			//   Heading/Footing is bold
	s.font((Fl_Font) (s.font() + FL_BOLD));

    if (R == -2)		// Row footer exception
    {
	s.background(FL_BLACK);	//      Black background
	s.foreground(FL_WHITE);	//      White text
	s.frame(FL_FLAT_BOX);	//      No box
	s.align(FL_ALIGN_RIGHT);	//      Right aligned
    }

    if (R > -1 && C > -1 && (R % 2) == 0 && C < 4)	//      Highlight every other row
	s.background(hilight_dark);

    st = get_value(R, C, pixmap);

    if (st)
	if (strstr(st, "-$"))	//     Negative $ are RED
	{
	    s.border(FLVB_OUTER_ALL);	// Nice dark box
	    s.foreground(FL_RED);	// Text in red
	    s.background(Fl_Color(215));	// Pale yellow background
	}

}

#define SZ 5

void
Flv_Table_Child::draw_cell(int Offset, int &X, int &Y, int &W, int &H, int R,
			   int C)
{
    Flv_Style s;

    get_style(s, R, C);
    Flv_Table::draw_cell(Offset, X, Y, W, H, R, C);

    int pixmap = 0;
    char *value = get_value(R, C, pixmap);

    int sX;
    if (pixmap) {
	(get_image(pixmap))->draw(X, Y - 1);
	sX = X + Offset + 25;
    } else {
	sX = X - Offset;
    }

    fl_draw(value, sX, Y, W, H, s.align());

}

int
Flv_Table_Child::text_width()
{
    int scrollbar_width = (scrollbar.visible()? scrollbar.w() : 0);
    int W = w() - scrollbar_width - 1;

    return W;
}

static int cw[10];		//      Column width
//      Another way would be to override handle for FL_SIZE.  We could also
//      spend a lot of time setting styles for the columns and returning
//      that, but since we're calculating it anyway, why bother.
int
Flv_Table_Child::col_width(int C)
{

    static int LW = -1;
    int scrollbar_width = (scrollbar.visible()? scrollbar.w() : 0);
    int W = w() - scrollbar_width - 1;
    int ww, t;

    //    Either always calculate or be sure to check that width
    //    hasn't changed.
    if (W != LW)		//      Width change, recalculate
    {
	cw[0] = (W * col0) / 100;	//      30              Name
	cw[1] = (W * col1) / 100;	//      10              Gender
	cw[2] = (W * col2) / 100;
	cw[3] = (W * col3) / 100;

	for (ww = 0, t = 0; t < 4; t++) {
	    ww += cw[t];
	}
	cw[4] = W - ww - 1;	//        ~30% +/- previous rounding errors
	LW = W;
    }

    return cw[C];

}
