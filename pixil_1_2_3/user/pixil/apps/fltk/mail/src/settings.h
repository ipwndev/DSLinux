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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <string.h>

struct setAccount
{
    char name[25];

    char values[6][31];
    char tempvalues[6][31];
    int saved[6];

    struct setAccount *next;
};

class mailSettings
{
  private:

    char keywords[6][31];

    struct setAccount *_accounts;
    int _acount;

    int _count;

    void add_new_account(char *name);
    struct setAccount *get_account(char *name);
    int set_value(char *account, char *keyword, const char *value);

  public:
      mailSettings(void);

    int load_settings();
    int save_settings();

    void set_temp_value(char *account, char *keyword, const char *value);

    int confirm_account_changes(char *account);
    void reject_account_changes(char *account);

    int confirm_all_changes();
    void reject_all_changes();

    char *get_value(char *account, char *keyword);

    int get_account_count(void)
    {
	return (_acount);
    }
    char *get_account_name(int num);
};

#endif
