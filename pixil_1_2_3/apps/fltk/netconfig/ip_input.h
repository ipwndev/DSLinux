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


#ifndef IP_INPUT_H
#define IP_INPUT_H

////// Includes ////////
#include <FL/Fl_Input.H>

///// Defines //////////
#define MAX_IP_LEN	16

///// Const ////////////
const static int NUM_INPUTS = 4;

///// Classes /////////
class Single_IP_Input;

///////////////////////////////////////////////
//
// class:                       IP_Input
// description: The IP_Input class defines the
//                                      user interface for setting up
//                                      the ip input boxes
///////////////////////////////////////////////                                         
class IP_Input:public Fl_Widget
{
    char ip_addr[MAX_IP_LEN];
    FL_EXPORT void draw();
    FL_EXPORT void move_focus();
  public:
      FL_EXPORT unsigned long iptol();
    FL_EXPORT char *get_ip_inputs();
    FL_EXPORT char *get_ip_inputs(bool);
    FL_EXPORT void set_ip_inputs(char *, bool);
    Single_IP_Input *input[NUM_INPUTS];
    FL_EXPORT void activate();
    FL_EXPORT void deactivate();
    FL_EXPORT void hide();
    FL_EXPORT void show();
    FL_EXPORT IP_Input(int, int, int, int, const char * = 0);
    virtual void resize(int x, int y, int w, int h)
    {
	Fl_Widget::resize(x, y, this->w(), this->h());
    }
};

///////////////////////////////////////////////
//
// class:                       Single_IP_Input
// description: The Single_IP_Input class defines the
//                                      user interface for setting up
//                                      the single ip input boxes
///////////////////////////////////////////////                                         
class Single_IP_Input:public Fl_Input
{
    FL_EXPORT int handle_key();
    IP_Input *parent;
  public:
      FL_EXPORT void set_parent(IP_Input *);
    FL_EXPORT int handle(int);
    FL_EXPORT Single_IP_Input(int X, int Y, int W, int H,
			      const char *L):Fl_Input(X, Y, W, H, L)
    {
    }
};


#endif
