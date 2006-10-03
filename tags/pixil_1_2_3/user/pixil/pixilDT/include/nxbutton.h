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

#ifndef		NXBUTTON_INCLUDED
#define		NXBUTTON_INCLUDED	1

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <stdio.h>

class NxButton:public Fl_Button
{

  public:
    NxButton(int x, int y, int w, int h, char *l = 0);
    void activate(void);	// Overridden activate function
    void deactivate(void);	// Overridden deactivate function
    int getactive()
    {
	return active;
    }				// Returns the active status

    void movable(bool flag)
    {
	move = flag;
    }

    virtual void resize(int x, int y, int w, int h)
    {

	if (move) {

	    int new_y;
	    if (y == save_y)
		new_y = save_y;
	    else
		new_y = y - int ((this->h() / 2) - 0.5);

	    Fl_Widget::resize(x, new_y, this->w(), this->h());

	}

    }

  private:
    int active;			// Flag to determine a "fake" limited active status
    bool move;
    int save_y;

  protected:
    void drawButtonCode(int x, int y, int w, int h, Fl_Color c);
    void draw(void);		// Overridden draw function
    int handle(int event);	// Overridden handle function
};

#endif //      NXBUTTON_INCLUDED
