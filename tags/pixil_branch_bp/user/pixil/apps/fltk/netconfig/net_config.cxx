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



#include "net_config.h"


#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/forms.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Radio_Button.H>

#include <nxapp.h>
#include <nxwindow.h>
#include <nxbrowser.h>
#include <nxradioroundbutton.h>

#include <assert.h>

extern "C"
{
#include <pixlib/pixlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
}

about about_net_conf = {
    "About Network Config",
    "(c) 2001, Century Software.",
    "jeffm@censoft.com",
    "09/04/01",
    "1.0"
};

NetConfig *
    NetConfig::_inst;
pid_t NetConfig::pid;

void
NetConfig::ClientIPCHandler(int fd, void *o, int ipc_id)
{

    char *tokenMsg = new char[MAX_LENGTH];
    char *passMsg = new char[MAX_LENGTH];
    memset(tokenMsg, 0, MAX_LENGTH);
    memset(passMsg, 0, MAX_LENGTH);

    if (o == NULL) {

	int length = MAX_LENGTH - 1;
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);
	if ((passMsg == NULL) || (passMsg[0] == 0))
	    return;
	else
	    strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxnet");

    } else {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);

    }


    // Explode Message
    char *service = new char[MAX_LENGTH];
    char *msg_cmd = new char[MAX_LENGTH];
    char *data_item = new char[MAX_LENGTH];

    // SERVICE
    char *tmp = strtok(tokenMsg, TOKEN);
    strcpy(service, tmp);

    // MSG_CMD
    tmp = strtok(NULL, TOKEN);
    strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    strcpy(data_item, tmp);

    // Memory Mangement
    delete[]service;
    delete[]msg_cmd;
    delete[]data_item;
    delete[]tokenMsg;
    service = msg_cmd = data_item = tokenMsg = NULL;

    NxApp::Instance()->ServerIPCHandler(fd, ipc_id, (char *) passMsg);

    delete[]passMsg;
    passMsg = NULL;


}

//////////////////////////////////////////////////////////
//
//
// function:            write_net_values        
// description: this function writes out ethernet values
// parameters:          none    
// return:                      none    
//
//////////////////////////////////////////////////////////
void
NetConfig::write_net_values()
{
    pix_sys_ipaddr_str_t ip_v;
    pix_sys_dns_t dom_v;

    ip_v.addr = IP_Setup::IP()->get_ip_value();
    ip_v.netmask = IP_Setup::IP()->get_netmask_value();
    ip_v.broadcast = IP_Setup::IP()->get_broadcast_value();
    ip_v.dhcp = (int) IP_Setup::IP()->get_dhcp_value();
    ip_v.subnet = "";
    ip_v.gateway = IP_Setup::IP()->get_gateway_value();
    dom_v.domain = this_dns_setup->get_domain_value();
    dom_v.search = this_dns_setup->get_search_value();
    dom_v.long_dns_1 = this_dns_setup->get_dns1_lvalue();
    dom_v.long_dns_2 = this_dns_setup->get_dns2_lvalue();
    dom_v.long_dns_3 = this_dns_setup->get_dns3_lvalue();
    dom_v.str_dns_1 = this_dns_setup->get_dns1_value();
    dom_v.str_dns_2 = this_dns_setup->get_dns2_value();
    dom_v.str_dns_3 = this_dns_setup->get_dns3_value();

    pix_sys_write_net_values(ip_v, dom_v);
}

//////////////////////////////////////////////////////////
//
// Constructor  
// description: this function starts the net_config
//              user interface 
// parameters:  none    
// return:      none    
//
//////////////////////////////////////////////////////////
NetConfig::NetConfig(int argc, char *argv[])
    :
NxApp(APP)
{

    //////////////////////////////////////
    // Access pt. initialization

    _inst = this;

    NxApp::Instance()->set_keyboard(argc, argv);
    NxApp::Instance()->set_about(about_net_conf);

    this_modem = NULL;
    this_ip_setup = NULL;
    this_dns_setup = NULL;

    cWin = new NxWindow(W_X, W_Y, W_W, W_H, "Network Configuration");
    MakePPPWin();
    MakeDNSWin();
    MakeIPWin();

    cWin->end();

    this_ip_setup->get_ip_info();

    ////////////////////////////////////////
    // FLNX-Colosseum IPC

    set_shown_window(this_ip_setup->get_parent_win());
    Add_Fd("nxnet", _ClientIPCHandler);

}

//////////////////////////////////////////////////////////
//
// function:    main    
// description: main function   
// parameters:  int argc - argument count
//              char **argv - argument array    
// return:      int - return value      
//
//////////////////////////////////////////////////////////
int
main(int argc, char **argv)
{

    NetConfig net_config(argc, argv);

    return (Fl::run());
}
