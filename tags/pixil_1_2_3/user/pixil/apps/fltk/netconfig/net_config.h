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



#ifndef NET_CONFIG_H
#define NET_CONFIG_H

#include <nxapp.h>

#include "ip_input.h"
#include "ppp_modem.h"
#include "ip_setup.h"
#include "dns_setup.h"

//////////// Constants //////////////////////////
#define GATEWAY_FILE "/tmp/gateway"

#define WV_LAN		"wvlan0"
#define DEV_ETH0 	"eth0"
#define DEV_LO 		"lo"
#define DEV_LO_LONG 	2130706432L

#define BOOTPROTO "BOOTPROTO"
#define DOMAIN 	"DOMAIN"
#define SEARCH 	"SEARCH"
#define DNS_1 	"DNS_1"
#define DNS_2 	"DNS_2"
#define DNS_3 	"DNS_3"
//#define NETMASK "NETMASK"
#define DHCP 	"DHCP"
//#define DEVICE        "DEVICE"
//#define IPADDR        "IPADDR"
#define ONBOOT 	"ONBOOT"
#define BOOTP 	"BOOTP"
#define PUMP 	"PUMP"
#define SUBNET 	"SUBNET"
//#define BROADCAST "BROADCAST"

#define MAX_TRIES 	10
#define MAX_LEN 	100

#define APP "Network"

class NetConfig:public NxApp
{

    static NetConfig *_inst;
    static pid_t pid;

    NxWindow *cWin;

    PPP_Modem *this_modem;
    IP_Setup *this_ip_setup;
    DNS_Setup *this_dns_setup;


    ////////////////////////////////////////////////////////////////////////////////
    // FLNX-Colosseum IPC

    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);

  protected:

    void MakeIPWin()
    {
	this_ip_setup = new IP_Setup();
    }
    void MakePPPWin()
    {
	this_modem = new PPP_Modem(cWin);
    }
    void MakeDNSWin()
    {
	this_dns_setup = new DNS_Setup();
    }

  public:
    NetConfig(int argc, char *argv[]);

    // Access pt.
    static NetConfig *I()
    {
	assert(_inst);
	return _inst;
    }
    void write_net_values();

    static void show_modem(Fl_Widget * fl, void *o)
    {
	Fl_Window *tWin = I()->this_modem->get_parent_win();
	NxApp::Instance()->show_window(tWin);
    }

    static void show_ip(Fl_Widget * fl, void *o)
    {
	Fl_Window *tWin = I()->this_ip_setup->get_parent_win();
	NxApp::Instance()->show_window(tWin);
    }

    static void show_dns(Fl_Widget * fl, void *o)
    {
	Fl_Window *tWin = I()->this_dns_setup->get_parent_win();
	NxApp::Instance()->show_window(tWin);
    }

};

#endif
