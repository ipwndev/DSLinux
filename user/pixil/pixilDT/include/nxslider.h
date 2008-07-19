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

#ifndef		NXSLIDER_INCLUDED
#define		NXSLIDER_INCLUDED	1

#include <FL/Fl.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Valuator.H>

class NxSlider:public Fl_Valuator
{

    float slider_size_;
    float slider_size_min_;
    uchar slider_;
    FL_EXPORT void _NxSlider();
    FL_EXPORT void draw_bg(int, int, int, int);
    bool movable_;


  protected:
    int handle(int);

  public:
    void slider_hor_lines(int x, int y, int w, int h, int W, Fl_Color c);
    void slider_ver_lines(int x, int y, int w, int h, int W, Fl_Color c);
    int scrollvalue(int windowtop, int windowsize, int first, int totalsize);


      NxSlider(int x, int y, int w, int h, const char *l = 0);
    void draw(int x, int y, int w, int h);
    void draw();

    FL_EXPORT void bounds(float a, float b);
    float slider_size() const
    {
	return slider_size_;
    }
    FL_EXPORT void slider_size(float v);
    Fl_Boxtype slider() const
    {
	return (Fl_Boxtype) slider_;
    }
    void slider(Fl_Boxtype c)
    {
	slider_ = c;
    }
    FL_EXPORT int handle(int, int, int, int, int);

    void movable(bool flag)
    {
	movable_ = flag;
    }

    void resize(int x, int y, int w, int h)
    {
	if (movable_) {
	    Fl_Valuator::resize(x, y, w, h);
	}
    }

};				// end of NxSlider::NxSlider()

#endif //      NXSLIDER_INCLUDED
