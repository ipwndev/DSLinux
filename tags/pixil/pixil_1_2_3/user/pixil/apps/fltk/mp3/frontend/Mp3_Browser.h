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



#ifndef Mp3_Browser_H
#define Mp3_Broswer_H

#include "Mp3_Node.h"
#include <FL/Fl.H>
#include <Flek/Fl_Toggle_Tree.H>
#include <FL/Fl_Pixmap.H>
#include <stdio.h>

class Mp3_Browser:public Fl_Toggle_Tree
{

  public:

    Mp3_Browser(int x, int y, int w, int h);
      virtual ~ Mp3_Browser();

    Mp3_Node *add_next(char *label = 0, char *time_label = 0, int can_open =
		       1, Fl_Pixmap * pixmap = 0, void *d = 0)
    {
	Mp3_Node *node;
	  node = new Mp3_Node(label, time_label, can_open, pixmap, d);
	  Fl_Toggle_Tree::add_next(node);
	  return node;
    }
  protected:
    void draw_node(int depth, int cy, Fl_Toggle_Node * node);
    void draw_label(Mp3_Node * node, int indent, int x, int y, int w, int h);

};

#endif
