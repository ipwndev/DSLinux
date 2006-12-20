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



#include "dns_setup.h"
#include "net_config.h"

extern "C"
{
#include <pixlib/pixlib.h>
#include <pixlib/pix_comm_proto.h>
#include <stdio.h>
}

Fl_Menu_Item dnsMenuItems[] = {

    {"TCP/IP Setup", 0, NetConfig::show_ip},
    {"DNS Setup", 0, NetConfig::show_dns},
    {"Modem Setup", 0, NetConfig::show_modem},
    {0},

    {0}
};

///////////////////////////////////////////////////////////
//
//      Function:               DNS_Setup::set_button_cb()
//      Description:    This function write out net values and restart
//                                                      the network 
//      Parametets:     Fl_Widget *widget -  widget calling the function
//                                              void *parent -  Parent object of the widget
//      Returns:                none    
//
//////////////////////////////////////////////////////////
void
DNS_Setup::set_button_cb(Fl_Widget * widget, void *parent)
{
    //DNS_Setup *dns_parent = (DNS_Setup *)parent;
    int err = 0;

    NetConfig::I()->write_net_values();
    err = IP_Setup::IP()->net_down();
    if (0 == err) {
	IP_Setup::IP()->net_up();
    }
}

///////////////////////////////////////////////////////////
//
//      Function:               DNS_Setup::defaults_button_cb()
//      Description:    This function restores the inputs to the last saved 
//                                                      settings.
//      Parametets:     Fl_Widget *widget -  widget calling the function
//                                              void *parent -  Parent object of the widget
//      Returns:                none    
//
//////////////////////////////////////////////////////////
void
DNS_Setup::defaults_button_cb(Fl_Widget * widget, void *parent)
{
    DNS_Setup *dns_parent = (DNS_Setup *) parent;
    dns_parent->get_values_from_file();
}

///////////////////////////////////////////////////////////
//
//      Function:               DNS_Setup::get_values_from_file()
//      Description:    This function fills in the input boxes with
//                                                      values loaded from the appropriate files 
//      Parametets:     none    
//      Returns:                none    
//
//////////////////////////////////////////////////////////
void
DNS_Setup::get_values_from_file()
{
    char domain[61];
    char search[61];
    char dns_1[17];
    char dns_2[17];
    char dns_3[17];

    domain[0] = '\0';
    search[0] = '\0';
    dns_1[0] = '\0';
    dns_2[0] = '\0';
    dns_3[0] = '\0';
    pix_sys_get_net_value(DOMAIN, domain);
    pix_sys_get_net_value(SEARCH, search);
    pix_sys_get_net_value(DNS_1, dns_1);
    pix_sys_get_net_value(DNS_2, dns_2);
    pix_sys_get_net_value(DNS_3, dns_3);
    domain_->value(domain);
    search_domain_->value(search);
    dns1_ip_->set_ip_inputs(dns_1, false);
    dns2_ip_->set_ip_inputs(dns_2, false);
    dns3_ip_->set_ip_inputs(dns_3, false);
}

///////////////////////////////////////////////////////////
//
//      Function:               DNS_Setup::DNS_Setup()
//      Description:    This function is the constructor for the DNS_Setup      
//                                                      object wich describes the UI.
//      Parametets:     Fl_Tabs *tabs - pointer to the group of tabs
//                                                      that the object belongs to.
//                                              Fl_Window *win - pointer to the parent window
//      Returns:                none    
//
//////////////////////////////////////////////////////////
DNS_Setup::DNS_Setup()
{

    dns_win_ = new NxPimWindow("DNS Setup", dnsMenuItems, MENU);
    NxApp::Instance()->add_window((Fl_Window *) dns_win_->GetWindowPtr());

    dns_setup_group_ = new Fl_Group(W_X, W_Y, W_W, W_H);

    int oset = 20;

    // Domain
    domain_ =
	new NxInput(BUTTON_X + 90, MB_Y + (1 * MB_H + oset), 140, 20,
		    "Default Domain:");
    domain_->maximum_size(60);

    // DNS
    dns1_ip_ =
	new IP_Input(BUTTON_X + 90, MB_Y + (3 * MB_H + oset), 140, 20,
		     "Nameserver 1:");
    dns2_ip_ =
	new IP_Input(BUTTON_X + 90, MB_Y + (5 * MB_H + oset), 140, 20,
		     "Nameserver 2:");
    dns3_ip_ =
	new IP_Input(BUTTON_X + 90, MB_Y + (7 * MB_H + oset), 140, 20,
		     "Nameserver 3:");

    // Search Domain
    search_domain_ =
	new NxInput(BUTTON_X + 90, MB_Y + (9 * MB_H + oset), 140, 20,
		    "Search domain:");
    search_domain_->maximum_size(60);

    // Buttons
    defaults_button_ =
	new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
		     "Reset");
    defaults_button_->callback(defaults_button_cb, this);

    set_button_ =
	new NxButton(BUTTON_X + BUTTON_WIDTH + 3, BUTTON_Y, BUTTON_WIDTH,
		     BUTTON_HEIGHT, "Set");
    set_button_->callback(set_button_cb, this);

    dns_setup_group_->end();

    dns_win_->add((Fl_Widget *) dns_setup_group_);

    dns1_ip_->set_ip_inputs("", false);
    dns2_ip_->set_ip_inputs("", false);
    dns3_ip_->set_ip_inputs("", false);
    get_values_from_file();
}

///////////////////////////////////////////////////////////
//
//      Function:               DNS_Setup::~DNS_Setup()
//      Description:    This function is the deconstructor for the DNS_Setup    
//                                                      object
//      Parametets:     none
//      Returns:                none    
//
//////////////////////////////////////////////////////////
DNS_Setup::~DNS_Setup()
{
}
