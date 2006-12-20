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


#include <cstdio>
#include <iostream>

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <sync/coder.h>
#include <sync/msg_defs.h>
#include <sync/syncerr.h>

const string tok = "^";

//see PIXIL Synchronization Messaging Schema Specification
//for message layout

void
MsgCoder::DecodeMsg(const string msg_packet)
{

    int bpos = 0, epos = msg_packet.find(set_token);

    if (epos == -1)
	return;

    string this_msg = msg_packet.substr(bpos, epos);
    vmessages.push_back(this_msg);
    string next_msg = msg_packet.substr(epos + 1, msg_packet.length());
    DecodeMsg(next_msg);

}

void
MsgCoder::DecodeMsg(const string msg_packet, short int &msg_id, int &msg_len)
{

    int bpos = 0, epos = msg_packet.find(set_token);

    if (epos == -1)
	return;

    string str_msg_id = msg_packet.substr(bpos, epos);
    msg_id = (short int) atoi(str_msg_id.c_str());
    string next_msg1 = msg_packet.substr(epos + 1, msg_packet.length());

    epos = next_msg1.find(set_token);

    string str_msg_len = next_msg1.substr(bpos, epos);
    msg_len = atoi(str_msg_len.c_str());

    string next_msg2 = next_msg1.substr(epos + 1, next_msg1.length());

    DecodeMsg(next_msg2);

}

string
    MsgCoder::EncodeMsg(const int msgid, const vector < string > *vstr,
			const string data)
{

    string msg;
    int len = 0;
    char strid[16];
    char str_len[16];
    unsigned int size = 0;
    unsigned int idx = 0;
    string tmp_msg = "";
    string tmp_str;

    sprintf(strid, "%d", msgid);

    if (data != "-1")
	len = data.length();

    sprintf(str_len, "%d", len);

    switch (msgid) {
    case ERR:
    case INFO:
	msg = strid + tok;
	size = vstr->size();
	for (idx = 0; idx < size; idx++)
	    msg = msg + *(vstr->begin() + idx) + tok;
	break;
    case BP:
	msg = strid + tok + *(vstr->begin()) + tok;
	break;
    case RD:
	// vstr is 0: Row Status
	// 1:Primary Key
	// Column Number
	// Column Data
	size = vstr->size();
	//tmp_msg = app_id + tok;
	for (idx = 0; idx < size; idx++) {
	    tmp_str = *(vstr->begin() + idx);
	    tmp_msg.append(tmp_str);
	    tmp_msg.append(tok);
	}
	msg = strid + tok + tmp_msg;
	break;
    case TS:
	// the number of columns is in the 1 position of the vector,
	// followed by the column types, followed by the column sizes.S
	size = atoi((vstr->begin() + 1)->c_str());
	idx = 0;
	tmp_msg = *(vstr->begin()) + tok + *(vstr->begin() + 1) + tok;
	for (idx = 2; idx <= size; idx++) {
	    tmp_str = *(vstr->begin() + idx);
	    tmp_msg.append(tmp_str);
	    tmp_msg.append(tok);
	}
	for (idx = size + 1; idx < vstr->size(); idx++) {
	    tmp_str = *(vstr->begin() + idx);
	    tmp_msg.append(tmp_str);
	    tmp_msg.append(tok);
	}
	msg = strid + tok + tmp_msg;
	break;
    case BS:
    case ES:
	msg = strid + tok + *(vstr->begin()) + tok;
	break;
    default:			// OK, EP, ET, EOT, FLIP, COMMIT
	msg = strid + tok;
    }

    return msg;
}

string MsgCoder::BeginSync(const string & app_id)
{

    vector < string > vstr;

    vstr.push_back(app_id);

    return EncodeMsg(BS, &vstr);
}

string MsgCoder::EndSync(const string & app_id)
{

    vector < string > vstr;

    vstr.push_back(app_id);

    return EncodeMsg(ES, &vstr);
}

string MsgCoder::Ok()
{
    return EncodeMsg(OK);
}

string MsgCoder::EndPimSync()
{
    return EncodeMsg(EP);
}

string MsgCoder::EndTable()
{
    return EncodeMsg(ET);
}

string MsgCoder::EndOfTables()
{
    return EncodeMsg(EOT);
}

string MsgCoder::Flip()
{
    return EncodeMsg(FLIP);
}

string MsgCoder::Commit()
{
    return EncodeMsg(COMMIT);
}

string MsgCoder::Err(const int err_code, const string & msg)
{

    vector < string > vstr;
    char
	err_str[16];

    sprintf(err_str, "%d", err_code);
    vstr.push_back(err_str);

    vstr.push_back(get_error_msg(err_code));

    if (msg.length() > 0)
	vstr.push_back(msg);

    return EncodeMsg(ERR, &vstr);
}

string MsgCoder::Info(const int info_code, const string & msg)
{

    vector < string > vstr;
    char
	info_str[16];

    sprintf(info_str, "%d", info_code);
    vstr.push_back(info_str);

    if (msg.length() > 0)
	vstr.push_back(msg);

    return EncodeMsg(INFO, &vstr);
}

string MsgCoder::BeginPimSync(const string & app_id)
{

    vector < string > vstr;

    vstr.push_back(app_id);

    return EncodeMsg(BP, &vstr);
}

string
    MsgCoder::RowData(const int status, const int key,
		      vector < string > &data)
{

    //data is in the form of col_num, data repeated as often as needed
    vector < string > vstr;
    char int_str[16];

    sprintf(int_str, "%d", status);
    vstr.push_back(int_str);
    sprintf(int_str, "%d", key);
    vstr.push_back(int_str);

    for (unsigned int idx = 0; idx < data.size(); idx++)
	vstr.push_back(data[idx]);

    return EncodeMsg(RD, &vstr);
}

string
    MsgCoder::TableSchema(int table_num, const vector < char >&col_type,
			  const vector < int >&col_size)
{

    // the table num goes into to 0 vector pos
    // this will use a new vector to push the number of collumns into the
    // 1 vector position, then will use that number to determine the
    // offset into the vector for the index to the column sizes

    vector < string > vstr;
    int size = col_type.size();
    char str[16];
    int idx = 0;

    sprintf(str, "%d", table_num);
    vstr.push_back(str);

    sprintf(str, "%d", size);
    vstr.push_back(str);

    for (idx = 0; idx < size; idx++) {
	sprintf(str, "%c", col_type[idx]);
	vstr.push_back(str);
    }

    size = col_size.size();
    for (idx = 0; idx < size; idx++) {
	sprintf(str, "%d", col_size[idx]);
	vstr.push_back(str);
    }
    return EncodeMsg(TS, &vstr);
}

string MsgCoder::Abort()
{
    return EncodeMsg(ABORT);
}

string MsgCoder::Status()
{
    return EncodeMsg(STATUS);
}
