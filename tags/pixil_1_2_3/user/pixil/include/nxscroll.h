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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef		NXSCROLL_INCLUDED
#define		NXSCROLL_INCLUDED	1

#include <FL/Fl_Group.H>
#include "nxscrollbar.h"
#include <FL/Fl_Scrollbar.H>

class NxScroll:public Fl_Group
{
    int save_h;
    bool move;
    bool resize_;
    int xposition_, yposition_;
    int width_, height_;
    int oldx, oldy;
    static FL_EXPORT void hscrollbar_cb(Fl_Widget *, void *);
    static FL_EXPORT void scrollbar_cb(Fl_Widget *, void *);
    FL_EXPORT void fix_scrollbar_order();
    static FL_EXPORT void draw_clip(void *, int, int, int, int);
    FL_EXPORT void bbox(int &, int &, int &, int &);

  protected:
      FL_EXPORT void draw();

  public:
      NxScrollbar scrollbar;
    NxScrollbar hscrollbar;

    void movable(bool flag)
    {
	move = flag;
    }
    void resize(bool flag)
    {
	resize_ = flag;
    }
    FL_EXPORT void resize(int, int, int, int);
    FL_EXPORT int handle(int);

    enum
    {				// values for type()
	HORIZONTAL = 1,
	VERTICAL = 2,
	BOTH = 3,
	ALWAYS_ON = 4,
	HORIZONTAL_ALWAYS = 5,
	VERTICAL_ALWAYS = 6,
	BOTH_ALWAYS = 7
    };

    int xposition() const
    {
	return xposition_;
    }
    int yposition() const
    {
	return yposition_;
    }
    FL_EXPORT void position(int, int);

    NxScroll(int x, int y, int w, int h, const char *l = 0);
};				// end of NxSlider::NxScroll()

#endif //      NXSCROLL_INCLUDED
