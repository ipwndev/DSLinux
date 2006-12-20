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

#include <fspl_panel.h>
#include <stdio.h>

mp3_button::mp3_button(int x, int y, int w, int h, int type):
NxButton(x, y, w, h)
{
    type_ = type;
}

void
mp3_button::draw(void)
{
    NxButton::draw();

    int center_x = x() + w() / 2;
    int center_y = y() + h() / 2;


    if (value()) {
	fl_color(contrast(color(), labelcolor()));

	if (type_ == MP3_FORWARD) {

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x, center_y - 3);
	    fl_transformed_vertex(center_x + 5, center_y + 2);
	    fl_transformed_vertex(center_x, center_y + 7);
	    fl_end_polygon();

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x - 5, center_y - 3);
	    fl_transformed_vertex(center_x, center_y + 2);
	    fl_transformed_vertex(center_x - 5, center_y + 7);
	    fl_end_polygon();
	} else if (type_ == MP3_BACK) {

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x, center_y - 3);
	    fl_transformed_vertex(center_x - 5, center_y + 2);
	    fl_transformed_vertex(center_x, center_y + 7);
	    fl_end_polygon();

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x + 5, center_y - 3);
	    fl_transformed_vertex(center_x, center_y + 2);
	    fl_transformed_vertex(center_x + 5, center_y + 7);
	    fl_end_polygon();
	} else if (type_ == MP3_PLAY) {

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x - 3, center_y - 3);
	    fl_transformed_vertex(center_x + 2, center_y + 2);
	    fl_transformed_vertex(center_x - 3, center_y + 7);
	    fl_end_polygon();
	} else if (type_ == MP3_STOP) {
	    //fl_rectf(center_x - 14, center_y - 1, 3, 8);
	    //fl_rectf(center_x - 8, center_y - 1, 3, 8);
	    //fl_line(center_x - 1, center_y + 5, center_x + 1, center_y - 1);
	    fl_rectf(center_x - 4, center_y - 1, 8, 8);
	} else if (type_ == MP3_PAUSE) {
	    fl_rectf(center_x - 4, center_y - 1, 3, 8);
	    fl_rectf(center_x + 1, center_y - 1, 3, 8);
	} else {
	    printf("Unknown MP3_Button type!\n");
	}

    } else {
	fl_color(labelcolor());
	if (type_ == MP3_FORWARD) {

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x, center_y - 5);
	    fl_transformed_vertex(center_x + 5, center_y);
	    fl_transformed_vertex(center_x, center_y + 5);
	    fl_end_polygon();

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x - 5, center_y - 5);
	    fl_transformed_vertex(center_x, center_y);
	    fl_transformed_vertex(center_x - 5, center_y + 5);
	    fl_end_polygon();
	} else if (type_ == MP3_BACK) {
	    fl_begin_polygon();
	    fl_transformed_vertex(center_x, center_y - 5);
	    fl_transformed_vertex(center_x - 5, center_y);
	    fl_transformed_vertex(center_x, center_y + 5);
	    fl_end_polygon();

	    fl_begin_polygon();
	    fl_transformed_vertex(center_x + 5, center_y - 5);
	    fl_transformed_vertex(center_x, center_y);
	    fl_transformed_vertex(center_x + 5, center_y + 5);
	    fl_end_polygon();
	} else if (type_ == MP3_PLAY) {
	    fl_begin_polygon();
	    fl_transformed_vertex(center_x - 3, center_y - 5);
	    fl_transformed_vertex(center_x + 2, center_y);
	    fl_transformed_vertex(center_x - 3, center_y + 5);
	    fl_end_polygon();
	} else if (type_ == MP3_STOP) {
	    //fl_rectf(center_x - 14, center_y - 3, 3, 8);
	    //fl_rectf(center_x - 8, center_y - 3, 3, 8);
	    //fl_line(center_x - 1, center_y + 3, center_x + 1, center_y - 3);
	    fl_rectf(center_x - 4, center_y - 3, 8, 8);
	} else if (type_ == MP3_PAUSE) {
	    fl_rectf(center_x - 4, center_y - 3, 3, 8);
	    fl_rectf(center_x + 1, center_y - 3, 3, 8);
	} else {
	    printf("Unkown MP3_Button type!\n");
	}
    }
}

mp3_button::~mp3_button()
{
}
