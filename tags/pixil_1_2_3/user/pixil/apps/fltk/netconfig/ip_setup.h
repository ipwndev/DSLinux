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



#ifndef _IP_SETUP_H_
#define _IP_SETUP_H_

/////// Includes ////////
#include <assert.h>

#include <FL/forms.H>
#include <FL/Fl_Tabs.H>

#include "ip_input.h"

#include <nxapp.h>
#include <nxwindow.h>
#include <nxbutton.h>
#include <nxcheckbutton.h>
//////// Defines ////////
#define MAX_ETH_DEVS  3

#define TYPE_ETH 0
#define TYPE_WVLAN 1

#define WNAME "IEEE 802.11-DS"

#define MAX_VALS  34		// 32 + 2(FOR "")

#define DEVICE 0
#define PROTO  1
#define IPADDR 2
#define NETMASK 3
#define BROADCAST 4
#define GATEWAY 5
#define ESSID 6
#define WEPID 7

#define MAX_KEYS 8
#define KEY_DEVICE "DEVICE"
#define KEY_PROTO  "PROTO"
#define KEY_IPADDR "IPADDR"
#define KEY_NETMASK   "NETMASK"
#define KEY_BROADCAST "BROADCAST"
#define KEY_GATEWAY   "GATEWAY"
#define KEY_ESSID     "ESSID"
#define KEY_WEPID     "WEPID"

//////// Structures /////
struct tmp_inputs
{
    NxInput *essid_input;
    NxCheckButton *wep_check;
    NxInput *wep_input;
};

//////// Classes ////////
///////////////////////////////////////////////
//
// class:                       IP_Setup
// description: The IP_Setup class defines the
//                                      user interface for setting up
//                                      the TCP/IP  network
///////////////////////////////////////////////                                         
class IP_Setup
{
    char *ifup, *ifdown;
    static char *netconf;
    static IP_Setup *_inst;
    int eth0_active_;
    int wvlan_active_;
    bool have_dev_eth0_;
    bool have_dev_wvlan_;
    bool have_wireless_lan_;

    NxPimWindow *ip_setup_win_;

    Fl_Group *ip_setup_group_;
    NxButton *start_stop_button_;
    IP_Input *ip_address_;
    IP_Input *netmask_;
    IP_Input *broadcast_;
    IP_Input *gateway_;
    NxCheckButton *dhcp_button_;
    NxButton *reset_button_;
    NxButton *set_button_;
    NxOutput *eth0_status_box_;
    NxOutput *wvlan_status_box_;

    ///////////
    // Wireless
    ///////////

    char *wireless_name;

    // Wireless NxCheckButton
    NxCheckButton *wireless_check;
    tmp_inputs inputs;

    static void wireless_check_cb(Fl_Widget * fl, void *o)
    {
	tmp_inputs *inputs = (tmp_inputs *) o;
	NxCheckButton *wireless_check = (NxCheckButton *) fl;
	NxInput *essid_input = inputs->essid_input;
	NxCheckButton *wep_check = inputs->wep_check;
	NxInput *wep_input = inputs->wep_input;

	if (wireless_check->value())
	{
	    essid_input->activate();
	    wep_check->activate();
	    wep_input->deactivate();
	} else
	{
	    essid_input->deactivate();
	    wep_check->value(0);
	    wep_check->deactivate();
	    wep_input->deactivate();
	}
    }

    // ESSID
    ////////
    NxInput *essid_input;

    // WEP
    //////
    NxCheckButton *wep_check;
    NxInput *wep_input;

    static void wep_check_cb(Fl_Widget * fl, void *o)
    {
	tmp_inputs *inputs = (tmp_inputs *) o;
	NxCheckButton *wep_check = (NxCheckButton *) fl;
	NxInput *wep_input = inputs->wep_input;

	if (wep_check->value())
	    wep_input->activate();
	else
	    wep_input->deactivate();
    }

    static FL_EXPORT void reset_button_cb(Fl_Widget *, void *parent);
    static FL_EXPORT void start_stop_button_cb(Fl_Widget *, void *parent);
    static FL_EXPORT void dhcp_button_cb(Fl_Widget *, void *parent);
    static FL_EXPORT void set_button_cb(Fl_Widget *, void *parent);
    void set_have_if_flags();
  public:
    Fl_Window * get_parent_win() {
	return (ip_setup_win_->GetWindowPtr());
    }
    void set_ip_value(char *addr)
    {
	ip_address_->set_ip_inputs(addr, false);
    }
    char *get_ip_value();	// {return ip_address_->get_ip_inputs(false);}
    unsigned long get_ip_lvalue()
    {
	return ip_address_->iptol();
    }
    void set_netmask_value(char *addr)
    {
	netmask_->set_ip_inputs(addr, false);
    }
    char *get_netmask_value()
    {
	return netmask_->get_ip_inputs(false);
    }
    unsigned long get_netmask_lvalue()
    {
	return netmask_->iptol();
    }
    void set_broadcast_value(char *addr)
    {
	broadcast_->set_ip_inputs(addr, false);
    }
    char *get_broadcast_value()
    {
	return broadcast_->get_ip_inputs(false);
    }
    unsigned long get_broadcast_lvalue()
    {
	return broadcast_->iptol();
    }
    char *get_gateway_value()
    {
	return gateway_->get_ip_inputs(false);
    }
    unsigned long get_gateway_lvalue()
    {
	return gateway_->iptol();
    }
    bool get_dhcp_value();
    void set_dhcp_value(bool value)
    {
	dhcp_button_->value(value);
    }
    bool get_have_dev_eth0_value()
    {
	return (have_dev_eth0_ ? true : false);
    }
    void set_have_dev_eth0_value(bool value)
    {
	have_dev_eth0_ = value;
    }
    bool get_have_wvlan0_value()
    {
	return (have_dev_wvlan_ ? true : false);
    }
    void set_have_wvlan0_value(bool value)
    {
	have_dev_wvlan_ = value;
    }
    bool get_have_wireless_value()
    {
	return (have_wireless_lan_ ? true : false);
    }
    void set_have_wireless_value();
    void set_button_label();
    void set_eth0_active_status(int status)
    {
	eth0_active_ = status;
    }
    int get_eth0_active_status()
    {
	return eth0_active_;
    }
    void set_wvlan_active_status(int status)
    {
	wvlan_active_ = status;
    }
    int get_wvlan_active_status()
    {
	return wvlan_active_;
    }
    void set_status_box(int);
    void set_status_box(int, char *);
    void get_ip_info();
    void set_ip_info();
    void set_gateway_value(char *value)
    {
	gateway_->set_ip_inputs(value, false);
    }

  public:
    int net_up();
    int net_down();
    static IP_Setup *IP()
    {
	assert(_inst);
	return _inst;
    }
    FL_EXPORT IP_Setup();
    FL_EXPORT ~ IP_Setup();
};

#endif
