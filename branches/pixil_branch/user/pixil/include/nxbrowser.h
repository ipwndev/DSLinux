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


#ifndef		NXBROWSER_INCLUDED
#define		NXBROWSER_INCLUDED	1

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <stdio.h>

class NxBrowser:public Fl_Browser
{

    int save_h;
    bool move;

  public:
      NxBrowser(int x, int y, int w, int h, const char * = 0);

    void movable(bool flag)
    {
	move = flag;
    }

    virtual void resize(int x, int y, int w, int h)
    {
	int new_h;

	if (h == save_h)
	    new_h = save_h;
	else {
	    new_h = h - (this->y() - y) - 5;
	}

	printf("h%d, new_h%d, save_h%d\n", h, new_h, save_h);

	if (move)
	    Fl_Browser::resize(x, y, w, new_h);
	else
	    Fl_Browser::resize(this->x(), this->y(), w, new_h);
    }

};

#endif //      NXBROWSER_INCLUDED
