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



#include "ip_setup.h"
#include "net_config.h"

extern "C"
{
#include <pixlib/pixlib.h>
#include <pixlib/pix_comm_proto.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
}

#include <nxoutput.h>

Fl_Menu_Item ipMenuItems[] = {

    {"TCP/IP Setup", 0, NetConfig::show_ip},
    {"DNS Setup", 0, NetConfig::show_dns},
    {"Modem Setup", 0, NetConfig::show_modem},
    {0},

    {0}
};

IP_Setup *
    IP_Setup::_inst;
char *
    IP_Setup::netconf;

char *
IP_Setup::get_ip_value()
{
    ip_address_->get_ip_inputs(false);
    return ip_address_->get_ip_inputs(false);
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_wireless_value()
//      Description: This function set the value of the
//                   wireles button depending if the interface
//                   exists or not
//      Parametets:  none
//      Returns:     none
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_have_wireless_value()
{

    if (PIX_COMM_OK == pix_comm_wireless_get_name(DEV_ETH0, wireless_name)) {
	if (0 == strcmp(wireless_name, WNAME)) {
	    have_wireless_lan_ = true;
	}
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::get_dhcp_value()
//      Description: This function returns the value of the
//                   dhcp button
//      Parametets:  none
//      Returns:     bool - true if button is pressed
//                   false if button is not pressed 
//
//////////////////////////////////////////////////////////
bool IP_Setup::get_dhcp_value()
{
    if (0 == dhcp_button_->value()) {
	return false;
    } else {
	return true;
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::dhcp_button_cb()
//      Description: This function is the callback when the 
//                   dhcp button is pressed. It causes the input boxes
//                   to either be activated or deactivated depending
//                   on the state of the dhcp button.
//      Parametets:  Fl_Widget *widget - widget doing the calling
//                   void *parent -  the widgets parent
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::dhcp_button_cb(Fl_Widget * widget, void *parent)
{
    IP_Setup *ip_setup = (IP_Setup *) parent;
    NxCheckButton *button = (NxCheckButton *) widget;

    if (0 == button->value()) {
	ip_setup->ip_address_->activate();
	ip_setup->netmask_->activate();
	ip_setup->broadcast_->activate();
	ip_setup->gateway_->activate();
    }
    if (1 == button->value()) {
	ip_setup->ip_address_->deactivate();
	ip_setup->netmask_->deactivate();
	ip_setup->broadcast_->deactivate();
	ip_setup->gateway_->deactivate();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::start_stop_button_cb()
//      Description: This function is the callback when the 
//                   the start_stop_button is pressed. It either stops
//                   or starts the network depending on the status
//                   of the network.
//      Parametets:  Fl_Widget *widget - widget doing the calling
//                   void *parent -  the widgets parent
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::start_stop_button_cb(Fl_Widget * widget, void *parent)
{
    IP_Setup *ip_parent = (IP_Setup *) parent;

    NetConfig::I()->write_net_values();

    if (PIX_COMM_ACTIVE == ip_parent->get_eth0_active_status() ||
	PIX_COMM_ACTIVE == ip_parent->get_wvlan_active_status()) {
	IP_Setup::IP()->net_down();
	ip_parent->start_stop_button_->label("Start");
	ip_parent->start_stop_button_->redraw();
    } else {
	IP_Setup::IP()->net_up();
	ip_parent->start_stop_button_->label("Stop");
	ip_parent->start_stop_button_->redraw();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_button_label
//      Description: This function sets the label on the start_stop_button
//                   determined by if the network is active or not 
//      Parametets:  none       
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_button_label()
{
    if (PIX_COMM_ACTIVE == eth0_active_ || PIX_COMM_ACTIVE == wvlan_active_) {
	start_stop_button_->label("Stop");
    } else {
	start_stop_button_->label("Start");
    }
    start_stop_button_->redraw();
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_button_cb
//      Description: This function is the call made when the 
//                   set button is pressed. It saves the input box
//                   values, stops the network and brings the
//                   network back up
//      Parametets:  Fl_Widget *widget - widget that is doing the call
//                   void *parent - parent object of the widget 
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_button_cb(Fl_Widget * widget, void *parent)
{

    IP_Setup *ip_parent = (IP_Setup *) parent;
    int err = 0;

    char *keys[MAX_KEYS];
    char *vals[MAX_KEYS];

    for (int i = 0; i < MAX_KEYS; i++) {
	keys[i] = new char[MAX_VALS];
	vals[i] = new char[MAX_VALS];
	keys[i][0] = '\0';
	vals[i][0] = '\0';
    }

    strcpy(keys[DEVICE], KEY_DEVICE);
    strcpy(keys[PROTO], KEY_PROTO);
    strcpy(keys[IPADDR], KEY_IPADDR);
    strcpy(keys[NETMASK], KEY_NETMASK);
    strcpy(keys[BROADCAST], KEY_BROADCAST);
    strcpy(keys[GATEWAY], KEY_GATEWAY);
    strcpy(keys[ESSID], KEY_ESSID);
    strcpy(keys[WEPID], KEY_WEPID);

    // Set device
    if (ip_parent->have_dev_eth0_)
	strcpy(vals[DEVICE], DEV_ETH0);

    // Get string values for ip, netmask, broadcast, essid, and wepid
    strcpy(vals[IPADDR], ip_parent->get_ip_value());
    strcpy(vals[NETMASK], ip_parent->get_netmask_value());
    strcpy(vals[BROADCAST], ip_parent->get_broadcast_value());
    strcpy(vals[GATEWAY], ip_parent->get_gateway_value());

    if (ip_parent->wireless_check->value()) {
	char essid_tmp[MAX_VALS];
	essid_tmp[0] = '\0';
	strcpy(essid_tmp, "\"");
	strcat(essid_tmp, ip_parent->essid_input->value());
	strcat(essid_tmp, "\"");
	strcpy(vals[ESSID], essid_tmp);
	if (ip_parent->wep_check->value())
	    strcpy(vals[WEPID], ip_parent->wep_input->value());
    }
    // Take the network down
    err = IP_Setup::IP()->net_down();
    NetConfig::I()->write_net_values();

    if (0 == err) {

	// Write new netscript values
	if (ip_parent->have_wireless_lan_) {
	    pix_comm_set_netscript_values(netconf, (const char **) keys,
					  (const char **) vals, MAX_KEYS);
	} else {
	    pix_comm_set_netscript_values(netconf, (const char **) keys, (const char **) vals, MAX_KEYS - 2);	// no essid and no wepid
	}
	err = IP_Setup::IP()->net_up();

	ip_parent->get_ip_info();

	// Memory management
	for (int i = 0; i < MAX_KEYS; i++) {
	    delete[]keys[i];
	    delete[]vals[i];
	    keys[i] = NULL;
	    vals[i] = NULL;
	}

    }

    ip_parent->set_button_label();

}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::reset_button_cb
//      Description: This function is the call made when the 
//                   reset button is pressed. It changes the values
//                   of the inputs boxes back to the last saved state
//      Parametets:  Fl_Widget *widget - widget that is doing the call
//                   void *parent - parent object of the widget 
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::reset_button_cb(Fl_Widget * widget, void *parent)
{
    IP_Setup *ip_parent = (IP_Setup *) parent;
    ip_parent->get_ip_info();
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_have_if_flags
//      Description: This function set the have_dev_eth0 and have_wvlan
//                   flags 
//      Parametets:  none
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_have_if_flags()
{
    int err = 0;
    int count = MAX_ETH_DEVS;
    int wcount = MAX_ETH_DEVS;
    pix_comm_interface_t eth_list[MAX_ETH_DEVS];
    pix_comm_interface_t wireless_list[MAX_ETH_DEVS];

    err = pix_comm_wireless_get_if_list(wireless_list, &wcount);
    if (0 <= err) {
	err = -1;
	for (int idx = 0; idx <= count; idx++) {
	    if (0 == strcmp(wireless_list[idx].name, WV_LAN)) {
		have_dev_wvlan_ = true;
		err = 0;
		break;
	    }
	}
    }
    err = pix_comm_get_if_list(PIX_COMM_TYPE_ETHERNET, eth_list, &count);
    if (0 <= err) {
	err = -1;
	for (int idx = 0; idx <= count; idx++) {
	    if (0 == strcmp(eth_list[idx].name, DEV_ETH0)) {
		have_dev_eth0_ = true;
		err = 0;
		break;
	    }
	}
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::get_ip_info
//      Description: This function loads the ipnut boxes of the 
//                   user interface,  it gets the values from
//                   reading values from current network settings
//                   or from a specified file, and then writes
//                   out the settings by calling write_net_values.
//      Parametets:  none
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::get_ip_info()
{
    char dhcp;
    pix_comm_ipaddr_t ip_info;
    char essid_tmp[MAX_VALS];

    char *keys[MAX_KEYS];
    char *vals[MAX_KEYS];

    for (int i = 0; i < MAX_KEYS; i++) {
	keys[i] = new char[MAX_VALS];
	vals[i] = new char[MAX_VALS];
	keys[i][0] = '\0';
	vals[i][0] = '\0';
    }

    strcpy(keys[DEVICE], KEY_DEVICE);
    strcpy(keys[PROTO], KEY_PROTO);
    strcpy(keys[IPADDR], KEY_IPADDR);
    strcpy(keys[NETMASK], KEY_NETMASK);
    strcpy(keys[BROADCAST], KEY_BROADCAST);
    strcpy(keys[GATEWAY], KEY_GATEWAY);
    strcpy(keys[ESSID], KEY_ESSID);
    strcpy(keys[WEPID], KEY_WEPID);

    // currently can only get config for one dev
    // get wvlan first for wirelless because
    // eth0 can also be wirelless
    if (true == have_dev_wvlan_) {
	pix_comm_get_ip_address(WV_LAN, &ip_info);
	wvlan_active_ = pix_comm_if_active(WV_LAN);
    }

    if (true == have_dev_eth0_) {
	pix_comm_get_ip_address(DEV_ETH0, &ip_info);
	eth0_active_ = pix_comm_if_active(DEV_ETH0);
	strcpy(vals[DEVICE], DEV_ETH0);
    }

    if (true == have_wireless_lan_) {
	wireless_check->value(1);
	wireless_check->do_callback();
	essid_tmp[0] = '\0';
	pix_comm_wireless_get_essid(DEV_ETH0, essid_tmp);
	strcpy(vals[ESSID], "\"");
	strcat(vals[ESSID], essid_tmp);
	strcat(vals[ESSID], "\"");
	pix_comm_wireless_get_encode(DEV_ETH0, vals[WEPID]);

	if (vals[ESSID][0] != 0) {
	    essid_input->value(essid_tmp);
	    if (vals[WEPID][0] != 0) {
		wep_check->value(1);
		wep_check->do_callback();
		wep_input->value(vals[WEPID]);
	    }
	}

    } else {
	wireless_check->value(0);
	essid_input->value("");
	wep_check->value(0);
	wep_input->value("");
	wireless_check->do_callback();
	wep_check->do_callback();
    }

    set_button_label();
    set_status_box(TYPE_ETH);
    set_status_box(TYPE_WVLAN);

    // Set str values for ip, netmask, and broadcast
    sprintf(vals[IPADDR], "%d.%d.%d.%d",
	    (unsigned char) (ip_info.addr >> 24) & 0xFF,
	    (unsigned char) (ip_info.addr >> 16) & 0xFF,
	    (unsigned char) (ip_info.addr >> 8) & 0xFF,
	    (unsigned char) ip_info.addr & 0xFF);
    sprintf(vals[NETMASK], "%d.%d.%d.%d",
	    (unsigned char) (ip_info.netmask >> 24) & 0xFF,
	    (unsigned char) (ip_info.netmask >> 16) & 0xFF,
	    (unsigned char) (ip_info.netmask >> 8) & 0xFF,
	    (unsigned char) ip_info.netmask & 0xFF);
    sprintf(vals[BROADCAST], "%d.%d.%d.%d",
	    (unsigned char) (ip_info.broadcast >> 24) & 0xFF,
	    (unsigned char) (ip_info.broadcast >> 16) & 0xFF,
	    (unsigned char) (ip_info.broadcast >> 8) & 0xFF,
	    (unsigned char) ip_info.broadcast & 0xFF);

    ip_address_->set_ip_inputs(vals[IPADDR], true);
    netmask_->set_ip_inputs(vals[NETMASK], true);
    broadcast_->set_ip_inputs(vals[BROADCAST], true);

    // Set gateway
    char buf[255];
    if (PIX_COMM_ACTIVE == eth0_active_ || PIX_COMM_ACTIVE == wvlan_active_) {
	unsigned long gway;
	pix_comm_get_default_gateway(&gway);
	sprintf(vals[GATEWAY], "%d.%d.%d.%d",
		(unsigned char) (gway >> 24) & 0xFF,
		(unsigned char) (gway >> 16) & 0xFF,
		(unsigned char) (gway >> 8) & 0xFF,
		(unsigned char) gway & 0xFF);
    } else {
	FILE *fp = fopen(GATEWAY_FILE, "r");
	if (NULL != fp) {
	    if (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
		fclose(fp);
		strncpy(vals[GATEWAY], buf, sizeof(vals[GATEWAY]));
	    }
	}
    }
    gateway_->set_ip_inputs(vals[GATEWAY], true);

    pix_sys_get_net_value(DHCP, &dhcp);
    if ('Y' == dhcp || 'y' == dhcp) {
	dhcp_button_->value(1);
	strcpy(vals[PROTO], "dynamic");
    } else {
	dhcp_button_->value(0);
	strcpy(vals[PROTO], "static");
    }
    dhcp_button_->do_callback();
    if (have_wireless_lan_)
	pix_comm_set_netscript_values(netconf, (const char **) keys,
				      (const char **) vals, MAX_KEYS);
    else
	pix_comm_set_netscript_values(netconf, (const char **) keys, (const char **) vals, MAX_KEYS - 2);	// don't include ESSID/WEPID keys

    NetConfig::I()->write_net_values();
    set_button_label();

    for (int i = 0; i < MAX_KEYS; i++) {
	delete[]keys[i];
	delete[]vals[i];
	keys[i] = NULL;
	vals[i] = NULL;
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_status_box
//      Description: This function sets the text in the status_box_ 
//      Parametets:  char *status - string containing status    
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_status_box(int type, char *status)
{
    if (TYPE_ETH == type) {
	eth0_status_box_->value(status);
	eth0_status_box_->redraw();
    }
    if (TYPE_WVLAN == type) {
	wvlan_status_box_->value(status);
	wvlan_status_box_->redraw();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::set_status_box
//      Description: This function sets the text in the status_box_ 
//      Parametets:  none       
//      Returns:     none       
//
//////////////////////////////////////////////////////////
void
IP_Setup::set_status_box(int type)
{
    char *status;

    if (TYPE_ETH == type) {
	if (PIX_COMM_ACTIVE == eth0_active_) {
	    status = "       Interface eth0 is active";
	} else {
	    status = "    Interface eth0 is not active";
	}
	eth0_status_box_->value(status);
	eth0_status_box_->redraw();
    }
    if (TYPE_WVLAN == type) {
	if (PIX_COMM_ACTIVE == wvlan_active_) {
	    status = "     Interface wvlan0 is active";
	} else {
	    status = "   Interface wvlan0 is not active";
	}
	wvlan_status_box_->value(status);
	wvlan_status_box_->redraw();
    }

}




///////////////////////////////////////////////////////////
//
//      Method       net_down()
//      Description: This function brings the ethernet down
//      Parametets:  none
//      Returns:     int err 
//
//////////////////////////////////////////////////////////
int
IP_Setup::net_down()
{

    pid_t childpid;
    int child_status;
    int idx;

    int err = 0;

    //  bool have_dev_wvlan0 = get_have_wvlan0_value();
    bool dhcp_state = get_dhcp_value();

    //bool have_dev_eth0 = get_have_dev_eth0_value();

    if ((childpid = vfork()) == -1) {
	perror("net_down (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0) {
	for (idx = 3; idx < 20; idx++)
	    close(idx);

	err = execl("/bin/sh", "sh", ifdown, netconf, NULL);
	exit(0);

    }				// in parent
    else {
	waitpid(childpid, &child_status, 0);
	if (0 == child_status) {
	    set_eth0_active_status(PIX_COMM_INACTIVE);
	    set_dhcp_value(dhcp_state);
	} else {
	    perror("net_down (EXECL)");
	    set_status_box(TYPE_ETH, "   Error shutting down interface eth0");
	}

	return child_status;
    }

}

//////////////////////////////////////////////////////////
//
// Function:    net_up
// Description: this function brings the ethernet up
// Parameters:  none
// Return:      int err
//
//////////////////////////////////////////////////////////
int
IP_Setup::net_up()
{
    int err = 0;
    bool dhcp_state = get_dhcp_value();
    //bool have_dev_wvlan0 = get_have_wvlan0_value();
    //bool have_dev_eth0 = get_have_dev_eth0_value();

    pid_t childpid;
    int child_status;
    int idx;


    if ((childpid = vfork()) == -1) {
	perror("net_down (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0) {
	for (idx = 3; idx < 20; idx++)
	    close(idx);

	err = execl("/bin/sh", "sh", ifup, netconf, NULL);
	exit(0);

    }				// in parent
    else {
	waitpid(childpid, &child_status, 0);
	if (0 == child_status) {
	    set_eth0_active_status(PIX_COMM_ACTIVE);
	    set_dhcp_value(dhcp_state);
	} else if (err < 0) {
	    perror("net_up (EXECL)");
	    set_status_box(TYPE_ETH, "   Error starting interface eth0");
	}

	return child_status;
    }
}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::IP_Setup
//      Description: This function is the constructor for the IP_Setup
//                   object.  It determines the look of the UI. 
//      Parametets:  Fl_Tabs *tabs - pointer to the tabs that this
//                   object belongs to
//                   Fl_Window *win - pointer to the parent window
//                   of the object
//      Returns:     none       
//
//////////////////////////////////////////////////////////

#include <iostream>
using namespace std;

IP_Setup::IP_Setup()
{

    _inst = this;

    ifup = new char[255];
    ifdown = new char[255];
    netconf = new char[255];

    getGblParStr("scripts", "ifup", ifup, 255);
    getGblParStr("scripts", "ifdown", ifdown, 255);
    getGblParStr("scripts", "netconf", netconf, 255);

    cout << "ifup = " << ifup << endl;
    cout << "ifdown = " << ifdown << endl;
    cout << "netconf = " << netconf << endl;

    wireless_name = new char[255];
    ip_setup_win_ = new NxPimWindow("TCP/IP Setup", ipMenuItems, MENU);
    NxApp::Instance()->add_window((Fl_Window *) ip_setup_win_->
				  GetWindowPtr());

    ip_setup_group_ = new Fl_Group(W_X, W_Y, W_W, W_H);

    int oset = 12;

    // DHCP
    dhcp_button_ = new NxCheckButton(BUTTON_X, MB_Y + (2 * oset), "DHCP");
    dhcp_button_->callback(dhcp_button_cb, this);

    // IP
    int i_oset = 65;
    ip_address_ =
	new IP_Input(BUTTON_X + i_oset, MB_Y + (4 * oset), 140, 20,
		     "IP Address:");
    netmask_ =
	new IP_Input(BUTTON_X + i_oset, MB_Y + (6 * oset), 140, 20,
		     "Netmask:");
    broadcast_ =
	new IP_Input(BUTTON_X + i_oset, MB_Y + (8 * oset), 140, 20,
		     "Broadcast:");
    gateway_ =
	new IP_Input(BUTTON_X + i_oset, MB_Y + (10 * oset), 140, 20,
		     "Gateway:");

    ///////////
    // Wireless 
    ///////////

    // Wireless NxCheckButton & callback (If checked, activate ESSID & WEP NxCheckButton & deactivate WEP NxInput)
    wireless_check =
	new NxCheckButton(BUTTON_X, MB_Y + (12 * oset), "Wireless");
    wireless_check->callback(wireless_check_cb, &inputs);

    // ESSID
    ////////

    // ESSID NxInput box
    essid_input =
	new NxInput(BUTTON_X + i_oset - 5, MB_Y + (14 * oset), 130, 20,
		    "ESS ID:");
    essid_input->deactivate();
    inputs.essid_input = essid_input;

    // WEP
    //////

    // WEP NxCheckButton & callback (If checked, activate WEP NxInput)
    wep_check =
	new NxCheckButton(BUTTON_X + oset + 6, MB_Y + (16 * oset), "WEP");
    wep_check->deactivate();
    inputs.wep_check = wep_check;
    wep_check->callback(wep_check_cb, &inputs);

    // WEP NxInput box
    wep_input =
	new NxInput(BUTTON_X + i_oset + 25, MB_Y + (18 * oset), 100, 20,
		    "WEP ID::");
    wep_input->deactivate();
    inputs.wep_input = wep_input;

    // Status Box
    eth0_status_box_ = new NxOutput(0, MB_Y + (17 * oset), W_W, 50, "");
    NxApp::Instance()->big_font(eth0_status_box_);
    eth0_status_box_->hide();

    // Status Box (We don't need this any more)
    wvlan_status_box_ = new NxOutput(30, 175, 180, 20, "");
    wvlan_status_box_->hide();

    // Buttons
    set_button_ =
	new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Set");
    set_button_->callback(set_button_cb, this);
    start_stop_button_ =
	new NxButton(BUTTON_X + (1 * BUTTON_WIDTH) + 3, BUTTON_Y,
		     BUTTON_WIDTH, BUTTON_HEIGHT, "Stop");
    start_stop_button_->callback(start_stop_button_cb, this);
    reset_button_ =
	new NxButton(BUTTON_X + (2 * BUTTON_WIDTH) + 6, BUTTON_Y,
		     BUTTON_WIDTH, BUTTON_HEIGHT, "Reset");
    reset_button_->callback(reset_button_cb, this);

    ip_setup_group_->end();

    ip_setup_win_->add((Fl_Widget *) ip_setup_group_);

    set_status_box(TYPE_ETH);
    set_status_box(TYPE_WVLAN);

    set_have_if_flags();	// Sets have lo, eth0

    set_have_wireless_value();	// Sets have wireless


    //get_ip_info();

}

///////////////////////////////////////////////////////////
//
//      Function:    IP_Setup::~IP_Setup
//      Description: This function is the deconstructor for the IP_Setup
//                   object.   
//      Parametets:  none       
//      Returns:     none       
//
//////////////////////////////////////////////////////////
IP_Setup::~IP_Setup()
{
    delete[]ifup;
    delete[]ifdown;
    delete[]netconf;
    delete[]wireless_name;
    wireless_name = ifup = ifdown = netconf = NULL;
}
