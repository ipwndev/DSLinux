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


#ifndef PPP_MODEM_H
#define PPP_MODEM_H

/////// Includes ////////
#include <FL/Fl_Group.H>
#include <FL/forms.H>
#include <FL/Fl_Tabs.H>

#include <nxapp.h>
#include <nxwindow.h>
#include <nxholdbrowser.h>
#include <nxbutton.h>
#include <nxmenubutton.h>
#include <nxradioroundbutton.h>
#include <nxcheckbutton.h>
#include <nxmultilineinput.h>
#include <nxinput.h>

#include "ip_input.h"

extern "C"
{
#include <pixlib/pixlib.h>
#include <pixlib/pix_comm_proto.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
}

//////// Defines ////////
#define SCRIPT_PREFIX  	"/tmp/ppp/chat-"
#define ACCOUNT_FILE		"/tmp/ppp/accounts"
#define DEVICE_FILE 		"/tmp/ppp/devices"
#define DIALER_FILE 		"/etc/wvdial.conf"
#define CONFIG_FILE 		"/tmp/ppp/config"
#define PPP_TMP_DIR 	"/tmp/ppp"
#define PPP_PEER_DIR "/etc/ppp/peers"
#define MODE_RDWRX	S_IRWXU|S_IRWXG|S_IRWXO
#define MODE_RDX     S_IROTH|S_IXOTH|S_IRGRP|S_IXGRP|S_IRUSR|S_IXUSR|S_IWUSR
#define MODE_RDWR S_IROTH|S_IWOTH|S_IWGRP|S_IRGRP|S_IRUSR|S_IWUSR

//////// static const ////////
static const int MAX_PPP_DEVS = 10;
static const int MAX_ACCOUNTS = 512;
static const int MAX_LINE_LEN = 255;
static const int MAX_PATH_LEN = 30;
static const int MAX_STR_LEN = 40;
static const int MAX_ARGS = 100;
static const int MAX_SCRIPT_SIZE = 4096;

//////// Classes ////////
class PPP_Add_Account;
class PPP_Account;
class PPP_Modem;
class PPP_Add_Config;

///////////////////////////////////////////////////////////
//
//      class:                  PPP_Add_Account
//      description:    This class defines the ui for adding an
//                                                      account.
//
///////////////////////////////////////////////////////////
class PPP_Add_Account
{
    NxPimWindow *add_win;
    NxInput *add_general_account;
    NxInput *add_general_phone;
    NxInput *add_general_username;
    NxInput *add_general_password;

  public:

      NxButton * ok_button;
    NxButton *cancel_button;

    Fl_Group *add_general_group;
    FL_EXPORT void set_max_values();
    FL_EXPORT void set_account_value(const char *);
    FL_EXPORT void set_phone_value(const char *);
    FL_EXPORT void set_username_value(const char *);
    FL_EXPORT void set_password_value(const char *);
    FL_EXPORT const char *get_account_value();
    FL_EXPORT const char *get_phone_value();
    FL_EXPORT const char *get_username_value();
    FL_EXPORT const char *get_password_value();
    Fl_Window *get_parent_win()
    {
	return add_win->GetWindowPtr();
    }
    FL_EXPORT void set_secret();
    FL_EXPORT PPP_Add_Account();
    FL_EXPORT ~ PPP_Add_Account();
};

///////////////////////////////////////////////////////////
//
//      Class:                  PPP_Account
//      Description:    This class defines the ui for viewong and 
//                                                      and manipulating accounts
//
///////////////////////////////////////////////////////////
class PPP_Account
{

    NxPimWindow *acct_win;

    PPP_Modem *account_modem;
    Fl_Group *ppp_account_group;
    NxButton *ppp_add_account_button;
    NxButton *ppp_edit_account_button;
    NxButton *ppp_delete_account_button;
    NxHoldBrowser *ppp_account_browser;
    NxMenuButton *ppp_account_choice;
    FL_EXPORT void edit_account_value(PPP_Add_Account *);
    FL_EXPORT void change_account_value(PPP_Add_Account *);
    FL_EXPORT const char *get_account_value(PPP_Add_Account *);
    FL_EXPORT void write_account(PPP_Add_Account *);
    FL_EXPORT void add_account(PPP_Add_Account *);
    FL_EXPORT void edit_account(PPP_Add_Account *);
    FL_EXPORT int get_account_names();
    FL_EXPORT void fill_account_browser();
    static FL_EXPORT void ppp_add_account_cb(Fl_Widget *, void *parent);
    static FL_EXPORT void ppp_delete_account_cb(Fl_Widget *, void *parent);
    FL_EXPORT int get_num_accounts();
    FL_EXPORT char **get_account_names_array();
  public:
      FL_EXPORT void hide();
    FL_EXPORT void show();
    Fl_Window *get_parent_win()
    {
	return acct_win->GetWindowPtr();
    }
    PPP_Modem *get_ppp_modem()
    {
	return account_modem;
    }
    FL_EXPORT PPP_Account(PPP_Modem * this_modem);
    FL_EXPORT ~ PPP_Account();

};

///////////////////////////////////////////////////////////
//
//      Class                           PPP_Add_Config
//      Descriptions    This class defines the ui for adding a config
//
///////////////////////////////////////////////////////////
class PPP_Add_Config
{

    NxPimWindow *config_win;

    NxMenuButton *ppp_add_config_dev_choice;
    NxMenuButton *ppp_add_config_speed_choice;
    NxCheckButton *dhcp_button;
    NxCheckButton *auto_button;
    IP_Input *ppp_add_config_ip_dest;
    IP_Input *ppp_add_config_ip_source;
    IP_Input *ppp_add_config_gateway;
    IP_Input *ppp_add_config_broadcast;
    NxInput *ppp_add_config_input;
    NxInput *add_dns_domain;
    IP_Input *add_dns_ip;
    NxMultilineInput *ppp_add_config_script;
    static void device_cb(Fl_Widget * fl, void *o = 0)
    {
	fl->label(((NxMenuButton *) fl)->text());
	fl->redraw();
    }
    static void speed_cb(Fl_Widget * fl, void *o = 0)
    {
	fl->label(((NxMenuButton *) fl)->text());
	fl->redraw();
    }
    static void ppp_add_config_dhcp_cb(Fl_Widget *, void *parent);
    static void ppp_add_config_auto_cb(Fl_Widget *, void *parent);
  public:
    NxButton * ok_button;
    NxButton *cancel_button;

    Fl_Group *ppp_add_config_group;
    FL_EXPORT void set_max_values();
    FL_EXPORT void set_config_value(char *);
    FL_EXPORT void set_device_value(char *);
    FL_EXPORT void set_device_value(int val)
    {
	ppp_add_config_dev_choice->value(val);
    }
    FL_EXPORT void set_speed_value(char *);
    FL_EXPORT void set_speed_value(int val)
    {
	ppp_add_config_speed_choice->value(val);
    }
    FL_EXPORT void set_dest_value(char *);
    FL_EXPORT void set_source_value(char *);
    FL_EXPORT void set_broadcast_value(char *);
    FL_EXPORT void set_gateway_value(char *);
    FL_EXPORT void set_dhcp_value(char *);
    FL_EXPORT void set_auto_value(char *);
    FL_EXPORT void set_script_value(char *);
    const char *get_auto_value();
    const char *get_dhcp_value();
    const char *get_config_value();
    const char *get_device_value();
    const char *get_speed_value();
    const char *get_dest_value();
    const char *get_source_value();
    const char *get_broadcast_value();
    const char *get_gateway_value();
    const char *get_script_value();
    FL_EXPORT void get_ttys();
    FL_EXPORT void fill_dev_choice();
    FL_EXPORT void fill_speed_choice();
    FL_EXPORT void set_dns_domain_value(const char *);
    FL_EXPORT void set_dns_ip_value(char *);
    FL_EXPORT const char *get_dns_domain_value();
    FL_EXPORT const char *get_dns_ip_value();
    void set_ip_info();
    Fl_Window *get_parent_window()
    {
	return config_win->GetWindowPtr();
    }
    FL_EXPORT void hide();
    FL_EXPORT void show();
    FL_EXPORT PPP_Add_Config();
    FL_EXPORT ~ PPP_Add_Config();
};

///////////////////////////////////////////////////////////
//
//      Class                           PPP_Modem_Setup
//      Description             This class defines the ui for manipulating
//                                                      the ui for configurations
//
///////////////////////////////////////////////////////////
class PPP_Modem_Setup
{

    NxPimWindow *modem_setup_win;

    PPP_Modem *account_modem;
    Fl_Group *ppp_setup_group;
    NxButton *save_button;
    NxHoldBrowser *ppp_modem_setup_browser;
    NxButton *ppp_modem_setup_add_button;
    NxButton *ppp_modem_setup_edit_button;
    NxButton *ppp_modem_setup_delete_button;
    static void ppp_add_config_cb(Fl_Widget *, void *parent);
    static void ppp_delete_config_cb(Fl_Widget *, void *parent);
  public:
      FL_EXPORT void get_devices();
    FL_EXPORT void set_speed(int);
    const char *get_config_value(PPP_Add_Config * config)
    {
	return config->get_config_value();
    }
    FL_EXPORT void edit_config_value(PPP_Add_Config *);
    FL_EXPORT void edit_config(PPP_Add_Config *);
    FL_EXPORT void add_config(PPP_Add_Config *);
    FL_EXPORT void write_config(PPP_Add_Config *);
    FL_EXPORT void change_config_value(PPP_Add_Config *);
    FL_EXPORT void fill_config_browser();
    FL_EXPORT void get_script_file(char *, PPP_Add_Config *);
    FL_EXPORT void write_script_file(char *, PPP_Add_Config *);
    PPP_Modem *get_ppp_modem()
    {
	return account_modem;
    }
    FL_EXPORT void hide();
    FL_EXPORT void show();
    Fl_Window *get_parent_win()
    {
	return modem_setup_win->GetWindowPtr();
    }
    FL_EXPORT PPP_Modem_Setup(PPP_Modem * this_modem);
    FL_EXPORT ~ PPP_Modem_Setup();
};

///////////////////////////////////////////////////////////
//
//      Class                           PPP_Modem
//      descriptions    This class defines the ui for manipulating
//                                                      accounts and configs for a modem.
//
///////////////////////////////////////////////////////////
class PPP_Modem
{
    bool _active;
    int _status;
    pix_comm_ppp_options_t ppp_options;
    pix_comm_ppp_info_t ppp_info;
    pix_comm_interface_t ppp_list[MAX_PPP_DEVS];
    int ppp_count;

    NxPimWindow *modem_win;

    Fl_Group *ppp_modem_group;
    //      NxMenuButton *ppp_config_choice;
    NxMenuButton *ppp_config_choice;
    NxMenuButton *ppp_modem_speed;
    NxMenuButton *ppp_account_choice;
    NxButton *ppp_connect_button;
    NxButton *ppp_account_setup_button;
    NxButton *ppp_device_setup_button;
    NxOutput *ppp_status_box;
    PPP_Modem_Setup *ppp_device_setup;
    PPP_Account *modem_account;
    PPP_Add_Account *modem_add_account;
    static void config_cb(Fl_Widget * fl, void *o = 0)
    {
	fl->label(((NxMenuButton *) fl)->text());
	fl->redraw();
    }
    static void account_cb(Fl_Widget * fl, void *o = 0)
    {
	fl->label(((NxMenuButton *) fl)->text());
	fl->redraw();
    }
    static void ppp_connect_button_cb(Fl_Widget *, void *parent);
    static void ppp_account_setup_cb(Fl_Widget *, void *parent);
    static void ppp_device_setup_cb(Fl_Widget *, void *parent);
  public:
    NxButton * cancel_button;
    NxButton *ok_button;
    PPP_Account *_acct;
    PPP_Add_Config *_add_config;
    PPP_Add_Account *_add_account;
    FL_EXPORT void get_accounts();
    FL_EXPORT void get_configs();
    FL_EXPORT void get_devices();
    FL_EXPORT void add_account(char *account)
    {
	ppp_account_choice->add(account);
    }
    FL_EXPORT void add_config(char *config)
    {
	ppp_config_choice->add(config);
    }
    FL_EXPORT void hide();
    FL_EXPORT void show();
    FL_EXPORT void set_ppp_options();
    pix_comm_ppp_options_t get_ppp_options()
    {
	return ppp_options;
    }
    bool get_active_status()
    {
	return _active;
    }
    void set_active_status(bool status)
    {
	_active = status;
    }
    void set_status_box(char *status)
    {
	ppp_status_box->value(status);
    }
    void set_status_box();
    int get_status()
    {
	return _status;
    }
    void set_status(int status)
    {
	_status = status;
    }
    int get_num_accounts() const
    {
	return (ppp_account_choice->size() - 1);
    }
    int get_num_configs() const
    {
	return (ppp_config_choice->size() - 1);
    }
    const char *get_account_name(int i)
    {
	return ppp_account_choice->text(i);
    }
    const char *get_config_name(int i)
    {
	return ppp_config_choice->text(i);
    }
    FL_EXPORT void delete_account(int i)
    {
	fl_delete_menu_item(ppp_account_choice, i);
    }
    FL_EXPORT void delete_config(int i)
    {
	fl_delete_menu_item(ppp_config_choice, i);
    }
    void check_ppp_dirs();
    void get_script(char *);
    char **set_pppd_command(char *, int *);
    Fl_Window *get_parent_win()
    {
	return modem_win->GetWindowPtr();
    }
    FL_EXPORT PPP_Modem(Fl_Window * win);
    FL_EXPORT ~ PPP_Modem();
};

#endif
