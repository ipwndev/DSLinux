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


#ifndef _DECODE_MSG_
#define _DECODE_MSG_

#include <vector>
#include <string>

using namespace std;

class MsgCoder
{

  public:
    string set_token;
    vector < string > vmessages;
    MsgCoder():set_token("^")
    {
    }
    MsgCoder(string token):set_token(token)
    {
    }
    ~MsgCoder() {
    }
    void DecodeMsg(const string msg_packet);
    void DecodeMsg(const string msg_packet, short int &msg_id, int &msg_len);
    string EncodeMsg(const int msgid, const vector < string > *vstr = 0,
		     const string data = "-1");
    string Ok();
    string EndPimSync();
    string EndTable();
    string EndOfTables();
    string Flip();
    string Commit();
    string Err(const int err_code, const string & msg = "");
    string Info(const int info_code, const string & msg = "");
    string BeginPimSync(const string & app_id);
    string RowData(const int status, const int key, vector < string > &data);
    string TableSchema(int table_num, const vector < char >&col_type,
		       const vector < int >&col_size);
    string Abort();
    string Status();
    string BeginSync(const string & app_id);
    string EndSync(const string & app_id);

};

#endif
