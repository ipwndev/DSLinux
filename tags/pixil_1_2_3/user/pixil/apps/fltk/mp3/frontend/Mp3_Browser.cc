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

#include "Mp3_Browser.h"

#include <FL/fl_draw.H>
#include <nxapp.h>


Mp3_Browser::Mp3_Browser(int x, int y, int w, int h):
Fl_Toggle_Tree(x, y, w, h)
{
}

void
Mp3_Browser::draw_node(int depth, int cy, Fl_Toggle_Node * node)
{
    Mp3_Node *tnode = (Mp3_Node *) node;

    if (damage() == FL_DAMAGE_CHILD && !tnode->changed_ && damaged_ == 0)
	return;

    tnode->changed_ = 0;
    if (tnode->selected_) {
	fl_color(selection_color());
	fl_rectf(x() + depth * 16 + pixmap_offset_, cy,
		 w() - depth * 16 - pixmap_offset_, height_(tnode) - 1);
	fl_xyline(x(), cy - 1, x() + w(), cy - 1);
	fl_xyline(x(), cy + height_(tnode) - 1, x() + w(),
		  cy + height_(tnode) - 1);
	fl_xyline(x() + depth * 16 + pixmap_offset_, cy - 1,
		  x() + depth * 16 + pixmap_offset_, cy + height_(tnode) - 1);
    } else {
	fl_color(color());
	fl_rectf(x() + depth * 16 + pixmap_offset_, cy,
		 w() - depth * 16 - pixmap_offset_, height_(tnode) - 1);
	fl_color(selection_color());
	fl_xyline(x(), cy - 1, x() + w(), cy - 1);
	fl_xyline(x(), cy + height_(tnode) - 1, x() + w(),
		  cy + height_(tnode) - 1);
	fl_xyline(x() + depth * 16 + pixmap_offset_, cy - 1,
		  x() + depth * 16 + pixmap_offset_, cy + height_(tnode) - 1);
    }
    fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
    if (tnode->selected_)
	textcolor(selection_label_color());
    //      else
    //  textcolor(((Fl_Toggle_Node *)node)->color());

    if (tnode->label_) {
	int D = depth * 16 + label_offset_;
	draw_label((Mp3_Node *) tnode, D, x() + depth * 16 + pixmap_offset_,
		   cy, w() - depth * 16 - pixmap_offset_, 16);
    }
    if (tnode->pixmap_) {
	tnode->pixmap_->draw(x() + 2, cy + 2);
    }
}

void
Mp3_Browser::draw_label(Mp3_Node * node, int indent, int x, int y, int w,
			int h)
{
    Fl_Font font = textfont();
    Fl_Color lcol = textcolor();
    Fl_Align align = (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CLIP);
    int size = textsize();
    char str[1024];
    char time_str[255];

    memset(str, 0, sizeof(str));
    memset(time_str, 0, sizeof(time_str));
    fl_font(font, size);

    int time_width = (int) fl_width("MM:MM");
    int label_width = w - time_width - 17;
    int time_x = x + w - time_width - 12;
    int dot_width = (int) fl_width("...");
    int len = 0;

    if (node->label_) {
	int lw = (int) fl_width(node->label_);
	if (lw > label_width) {
	    char temp_str[255];
	    len = strlen(node->label_);

	    strcpy(temp_str, node->label_);
	    while (lw + dot_width > label_width) {
		memset(temp_str, 0, sizeof(temp_str));
		strncpy(temp_str, node->label_, --len);
		lw = (int) fl_width(temp_str);
	    }
	    strcat(temp_str, "...");
	    strcpy(node->label_, temp_str);
	}
    }
    if (!active_r())
	lcol = inactive(lcol);
    fl_color(lcol);

    fl_draw(node->label_, x + 5, y, w - indent, h + 1, align);
    align = (Fl_Align) (FL_ALIGN_RIGHT | FL_ALIGN_CLIP);
    fl_draw(node->time_label_, time_x, y, time_width, h + 1, align);
}

Mp3_Browser::~Mp3_Browser()
{
}
