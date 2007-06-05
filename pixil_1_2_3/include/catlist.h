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


#ifndef __CAT_H
#define __CAT_H

#include "nxdb.h"

#include <FL/Fl.H>
#include "nxmenubutton.h"


class NxCategoryList:public NxMenuButton
{
  private:
    char NxCategoryBuf[200];
  public:
      Fl_Callback * m_Cb;
    char m_CatBuf[200];

      NxCategoryList(int x, int y, int w, int h, NxDb * db, string _dbName);
    void select(Fl_Callback * c)
    {
	m_Cb = c;
    }
    FL_EXPORT int add(const char *);
    void label(char *szLabel);
    char *label();
    char *get_category_buf()
    {
	return NxCategoryBuf;
    }
    void set_category_buf(char *cat_buf);
    void set_value();
};

#endif
