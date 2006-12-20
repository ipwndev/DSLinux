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
#include <string.h>
#include "misclist.h"

static void
select_callback(Fl_Widget * fl, long l)
{
    NxMiscList *list = (NxMiscList *) fl;
    strcpy(list->m_CatBuf, list->text());
    list->label(list->m_CatBuf);
    list->hide();
    list->show();

    if (list->m_Cb)
	(*list->m_Cb) (list, (void *) list->m_CatBuf);
}

NxMiscList::NxMiscList(int x, int y, int w, int h):
Fl_Menu_Button(x, y, w, h, "")
{

    add("Work|Home|Fax|Other|E-Mail|Main|Pager|Mobile");

    callback(select_callback);
    select(0);
}

int
NxMiscList::value()
{
    return Fl_Menu_Button::value();
}

void
NxMiscList::value(int n)
{
    Fl_Menu_Button::value(n);

    int l = strlen(text(n));
    strncpy(m_CatBuf, text(n), l);

    label(m_CatBuf);
    hide();
    show();

}
