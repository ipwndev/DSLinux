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

#ifndef		NXBOX_INCLUDED
#define		NXBOX_INCLUDED	1

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <stdio.h>

class NxBox:public Fl_Box
{
    int save_h;
    bool move;
    bool resize_;
    bool default_box_;
    Fl_Color box_color_;
  public:
      NxBox(int x, int y, int w, int h, char *l = 0);
      virtual ~ NxBox()
    {
    }

    virtual void draw();
    void movable(bool flag)
    {
	move = flag;
    }
    void default_box(bool flag)
    {
	default_box_ = flag;
    }
    void resize(bool flag)
    {
	resize_ = flag;
    }
    virtual void resize(int x, int y, int w, int h)
    {
	if (resize_) {
	    int new_h;
	    if (h == save_h)
		new_h = save_h;
	    else
		new_h = h - (this->y() - y) - 6;

	    if (move)
		Fl_Widget::resize(x, y, w, h /*new_h */ );
	    else
		Fl_Widget::resize(this->x(), this->y(), this->w(), new_h);
	}

    }
    void box_color(Fl_Color col)
    {
	box_color_ = col;
    }
};

#endif //      NXBOXINCLUDED
