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


/////////////////// Includes ////////////////////
#include <FL/Fl.H>

#include <FL/fl_draw.H>
#include <FL/forms.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Menu_Item.H>

extern "C"
{
#include <wm/ini.h>
}

#include "net_config.h"
#include "ppp_modem.h"

Fl_Menu_Item modemMenuItems[] = {

    {"TCP/IP Setup", 0, NetConfig::show_ip},
    {"DNS Setup", 0, NetConfig::show_dns},
    {"Modem Setup", 0, NetConfig::show_modem},
    {0},

    {0}
};

Fl_Menu_Item setupMenuItems[] = {

    {"Modem Setup", 0, NetConfig::show_modem},
    {0},

    {0}
};

/********************************ppp_add_account************************************/
////////////////////////////////////////////////////////////
//
// Functioin:           PPP_Add_Account::set_max_value()
// Description: This function sets the max input lengths for
//                                              inputs of PPP_Add_Account object
// Parameters:          none
// Returns:                     none
//
/////////////////////////////////////////////////////////////
void
PPP_Add_Account::set_max_values()
{
    add_general_account->maximum_size(19);
    add_general_phone->maximum_size(11);
    add_general_username->maximum_size(19);
    add_general_password->maximum_size(19);
}

/////////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Account::set_account_value
// Description: Sets the add_general_account input value
// Parameters:          const chat *value
// Returns:                     none
//
/////////////////////////////////////////////////////////////
void
PPP_Add_Account::set_account_value(const char *value)
{
    add_general_account->value(value);
}

////////////////////////////////////////////////////////////
// 
// Function:            PPP_Add_Account::set_pjone_value
// Description: Sets the input value for add_general_phone
// Parameters:          const chat *value
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Account::set_phone_value(const char *value)
{
    add_general_phone->value(value);
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Account::set_username_value
// Description: Sets the usernam input value
// Parameters:          const chat *value
// Returns:                     none
//
// ////////////////////////////////////////////////////////
void
PPP_Add_Account::set_username_value(const char *value)
{
    add_general_username->value(value);
}

///////////////////////////////////////////////////////////
//
// Functino:            PPP_Add_Accpunt::set_password_value
// Description: sets the password input value
// Paremeters:          const char *value
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Account::set_password_value(const char *value)
{
    add_general_password->value(value);
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Account::get_account_value()
// Description: get the account input value
// Paremeters:          none
// Returns:                     const char *
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Account::get_account_value()
{
    return add_general_account->value();
}

///////////////////////////////////////////////////////////
// 
// Funtion:                     PPP_Add_Account::get_phone_value
// description: get the phone input value
// Parameters:          none
// Returns:                     const char *
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Account::get_phone_value()
{
    return add_general_phone->value();
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Account;;get_username_value
// Description: get the username input value
// Parameters:          none
// returns:                     const char *
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Account::get_username_value()
{
    return add_general_username->value();
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Accoujnt::get_password_value
// Description: get the passworkd input value
// Parameters:          none
// Returns:                     const chat *
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Account::get_password_value()
{
    return add_general_password->value();
}

///////////////////////////////////////////////////////////
//
//      Funtion:                        PPP_Add_Account::set_secret()
//      Description:    set the password input to type secret
//      Parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Account::set_secret()
{
    add_general_password->type(FL_SECRET_INPUT);
}

///////////////////////////////////////////////////////////
//
// Funtions             PPP_Add_Account::PPP_Add_Account
// Description: The PPP_Add_Account class instantiation
// Parameters:          Fl_Tabs *add_account_tabs - pointer to parent tab
// returns:                     none
//
///////////////////////////////////////////////////////////
PPP_Add_Account::PPP_Add_Account()
{
    add_win = new NxPimWindow("Add Account", setupMenuItems, MENU);
    NetConfig::I()->add_window((Fl_Window *) add_win->GetWindowPtr());

    ok_button =
	new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
    cancel_button =
	new NxButton(BUTTON_X + BUTTON_WIDTH + 3, BUTTON_Y, BUTTON_WIDTH,
		     BUTTON_HEIGHT, "Cancel");

    add_general_group = new Fl_Group(W_X, W_Y, W_W, W_H);

    add_general_group->add(ok_button);
    add_general_group->add(cancel_button);

    add_general_account = new NxInput(70, 65, 130, 20, "Account:");
    add_general_phone = new NxInput(70, 90, 130, 20, "Number:");
    add_general_username = new NxInput(70, 115, 130, 20, "Username:");
    add_general_password = new NxInput(70, 140, 130, 20, "Password:");

    add_general_group->end();

    add_win->add((Fl_Widget *) add_general_group);

    NxMenuButton *butt = (NxMenuButton *) (add_win->_menu);
    butt->movable(true);
    butt->resize(butt->x(), butt->y(), MB_W + 20, butt->h());
    butt->movable(false);

}

/****************************ppp_account*******************************/
///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::edit_account_value
// description: Get input values from files based on which
//                                              account is selected in the account browser
// Parameters:          PPP_Add_Account *account - pointer to object
// Returns:                     none
//
/////////////////////////////////////////////////////////// 
void
PPP_Account::edit_account_value(PPP_Add_Account * account)
{
    char account_value[20];
    char phone[12] = "";
    char password[20] = "";
    char username[20] = "";
    int val = 0;


    val = ppp_account_browser->value();
    if (0 >= val) {
	return;
    }
    if (ppp_account_browser->text(val)) {
	strcpy(account_value, ppp_account_browser->text(val));
    } else {
	strcpy(account_value, "");
    }
    IniGetString(account_value, "number", NULL, phone, sizeof(phone) - 1,
		 ACCOUNT_FILE);
    IniGetString(account_value, "password", NULL, password,
		 sizeof(password) - 1, ACCOUNT_FILE);
    IniGetString(account_value, "username", NULL, username,
		 sizeof(username) - 1, ACCOUNT_FILE);
    account->set_account_value(account_value);
    account->set_phone_value(phone);
    account->set_password_value(password);
    account->set_username_value(username);
}

///////////////////////////////////////////////////////////
//
// Functions:           PPP_Account::change_account_value
// Descrition:          Change the account value input to a value that
//                                              has not been used yet
// Parameters:          PPP_Add_Account *account - pointer to object
// Returns:                     none
//
// ////////////////////////////////////////////////////////
void
PPP_Account::change_account_value(PPP_Add_Account * account)
{
    char buf[20];
    bool match = true;
    int accounts = ppp_account_browser->size();
    int nums = 1;

    sprintf(buf, "NewAccount%d", nums);
    while (match) {
	for (int idx = 1; idx <= accounts; idx++) {
	    if (0 == strcmp(buf, ppp_account_browser->text(idx))) {
		match = true;
		idx = 0;
		sprintf(buf, "NewAccount%d", ++nums);
	    }
	}
	match = false;
    }
    account->set_account_value(buf);
}

///////////////////////////////////////////////////////////
//
// Funtions:            PPP_Account::get_account_value()
// descrition:          get the account input value
// Parameters:          PPP_Add_account *account - pointer to object
// Returns:                     const chat *
//
///////////////////////////////////////////////////////////
const char *
PPP_Account::get_account_value(PPP_Add_Account * account)
{
    return account->get_account_value();
}

///////////////////////////////////////////////////////////
//
//      Function:               PPP_Account::write_account()
//      Description:    Write out the account values based on the value in
//                                                      the account input
//      Parameters;             PPP_Add_Account *account - pointer to an object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Account::write_account(PPP_Add_Account * account)
{
    char acnt[50];
    char phone[20];

    strcpy(acnt, account->get_account_value());
    IniWriteString(acnt, "number", (char *) account->get_phone_value(),
		   ACCOUNT_FILE);
    IniWriteString(acnt, "username", (char *) account->get_username_value(),
		   ACCOUNT_FILE);
    IniWriteString(acnt, "password", (char *) account->get_password_value(),
		   ACCOUNT_FILE);

    sprintf(acnt, "Dialer %s", account->get_account_value());
    strcpy(phone, account->get_phone_value());
    IniWriteString(acnt, "Phone", phone, DIALER_FILE);
    IniWriteString(acnt, "Username", (char *) account->get_username_value(),
		   DIALER_FILE);
    IniWriteString(acnt, "Password", (char *) account->get_password_value(),
		   DIALER_FILE);

    chmod(ACCOUNT_FILE, MODE_RDWR);
}

///////////////////////////////////////////////////////////
//
// Functoin:            PPP_Account::edit_account
// Description: change the account value if it is "" and write out
//                                              the account, and set parent inputs.
// Parameters:          PPP_Add_Account *account - pointer to an object
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Account::edit_account(PPP_Add_Account * account)
{
    char acnt[20];

    strcpy(acnt, account->get_account_value());
    if (0 == strcmp(acnt, "")) {
	change_account_value(account);
	strcpy(acnt, account->get_account_value());
    }
    for (int idx = 1; idx <= ppp_account_browser->size(); idx++) {
	if (0 == strcmp(acnt, ppp_account_browser->text(idx))) {
	    write_account(account);
	    return;
	}
    }
    fl_addto_browser(ppp_account_browser, acnt);
    account_modem->add_account(acnt);
    write_account(account);
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::add_account()
// Description:   add account to account browser and write it out
// Parameters:          PPP_Add_Accouht *account - pointer to an object
// retuns:                      none
//
///////////////////////////////////////////////////////////
void
PPP_Account::add_account(PPP_Add_Account * account)
{
    char acnt[20];

    memset(acnt, 0, sizeof(acnt));
    strncpy(acnt, account->get_account_value(), 19);
    if (0 == strcmp(acnt, "")) {
	change_account_value(account);
	strcpy(acnt, account->get_account_value());
    }
    fl_addto_browser(ppp_account_browser, get_account_value(account));
    account_modem->add_account(acnt);
    write_account(account);
}

///////////////////////////////////////////////////////////
//
// Functon:                     PPP_Accoujt::ppp_delete_account_cb()
// Descriptioni:  Delete an account
// Parameters:          Fl_Widget *widget - widget calling function
//                                      void *parent - pointer to parent object
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Account::ppp_delete_account_cb(Fl_Widget * widget, void *parent)
{
    char dialer_value[40];
    char account_value[20];
    int val = 0;
    PPP_Account *account = (PPP_Account *) (parent);

    val = account->ppp_account_browser->value();
    if (0 >= val) {
	return;
    }
    strcpy(account_value, account->ppp_account_browser->text(val));
    IniDelSection(account_value, ACCOUNT_FILE);
    fl_delete_browser_line(account->ppp_account_browser, val);
    fl_select_browser_line(account->ppp_account_browser, 1);
    account->account_modem->delete_account(val);
    sprintf(dialer_value, "Dialer %s", account_value);
    IniDelSection(dialer_value, DIALER_FILE);
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::ppp_add_account_cb()
// Description: Show the edit/add account window 
// Paremeters:          Fl_Widget *widget - pointer to the calling widget
//                                      void *parent - pointer to the parent object
// Returns:                     none
//              
///////////////////////////////////////////////////////////
void
PPP_Account::ppp_add_account_cb(Fl_Widget * widget, void *parent)
{
    NxButton *button = (NxButton *) widget;
    PPP_Account *account = (PPP_Account *) (parent);
    PPP_Modem *this_modem = account->get_ppp_modem();
    PPP_Add_Account *acnt = this_modem->_add_account;

    PPP_Add_Account *add_account = this_modem->_add_account;

    acnt->set_max_values();
    acnt->set_secret();

    if (button == account->ppp_add_account_button) {
	account->change_account_value(acnt);
    }
    if (button == account->ppp_edit_account_button) {
	account->edit_account_value(acnt);
    }

    add_account->get_parent_win()->show();

    while (add_account->get_parent_win()->shown()) {
	Fl::wait();
	for (;;) {
	    Fl_Widget *o = Fl::readqueue();
	    if (!o) {
		break;
	    }
	    if (o == add_account->ok_button) {
		if (button == account->ppp_add_account_button) {
		    account->add_account(acnt);
		}
		if (button == account->ppp_edit_account_button) {
		    account->edit_account(acnt);
		}
		add_account->get_parent_win()->hide();
		account->show();
		//      parent_win->show();
		//      parent_win->remove(&ppp_account_win);
		return;
	    }
	    if ( /*o == &ppp_account_win || */ o ==
		add_account->cancel_button) {
		add_account->get_parent_win()->hide();
		account->show();
		//      parent_win->show();
		//      parent_win->remove(&ppp_account_win);
		return;
	    }
	}
    }

}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::fill_acocunt_browser()
// Description: populate the account browser object with account
// Parameters:          none
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Account::fill_account_browser()
{
    int size = account_modem->get_num_accounts();

    for (int idx = 0; idx < size; idx++) {
	fl_addto_browser(ppp_account_browser,
			 account_modem->get_account_name(idx));
    }
    fl_set_browser_topline(ppp_account_browser, 1);
    fl_select_browser_line(ppp_account_browser, 1);
}

///////////////////////////////////////////////////////////
//
// Function             PPP_Account::hide()
// Description: hide the PPP_Account object
// Parameters:          none
// retuns:                      none
//
///////////////////////////////////////////////////////////
void
PPP_Account::hide()
{
    get_parent_win()->hide();
    /*
       //ppp_account_browser->hide();
       ppp_add_account_button->hide();
       ppp_edit_account_button->hide();
       ppp_delete_account_button->hide();
     */
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::show()
// descritpion: show the PPP_Account object
// ParemetersL          none
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Account::show()
{
    get_parent_win()->show();
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Account::PPP_Account()
// Description: PPP_Account class instatiation
// Parameters:          Fl_Tabs *ppp_setup_tabs - pointer to parent tabs
//                                      PPP_Modem *this_modem - pointer to parent modem
// Returns:                     none
//                              
///////////////////////////////////////////////////////////
PPP_Account::PPP_Account(PPP_Modem * this_modem)
{
    account_modem = this_modem;

    acct_win = new NxPimWindow("Accounts", setupMenuItems, MENU);
    NetConfig::I()->add_window((Fl_Window *) acct_win->GetWindowPtr());

    ppp_account_group = new Fl_Group(W_X, W_Y, W_W, W_H);

    ppp_account_browser = new NxHoldBrowser((240 - (MENU_W) + 20) / 2,
					    2 * MENU_Y + MENU_H + 5,
					    MENU_W - 20,
					    BUTTON_Y - 5 - (BUTTON_HEIGHT +
							    MENU_Y + MENU_H),
					    "Accounts");
    ppp_account_browser->align(FL_ALIGN_TOP);

    ppp_add_account_button =
	new NxButton(3, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT, "Add");
    ppp_edit_account_button =
	new NxButton(62, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT, "Edit");
    ppp_delete_account_button =
	new NxButton(121, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
		     "Delete");

    ppp_add_account_button->callback(ppp_add_account_cb, this);
    ppp_edit_account_button->callback(ppp_add_account_cb, this);
    ppp_delete_account_button->callback(ppp_delete_account_cb, this);

    {
	NxButton *o =
	    new NxButton(181, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Done");
	o->callback(NetConfig::show_modem);
    }

    ppp_account_group->end();

    acct_win->add((Fl_Widget *) ppp_account_group);

    fill_account_browser();
}

/***************************ppp_add_config***********************************/
///////////////////////////////////////////////////////////  
//  Menu items array
// ////////////////////////////////////////////////////////
static Fl_Menu_Item ppp_speed_menu[] = {
    {"115200"},
    {"57600"},
    {"38400"},
    {"19200"},
    //{"USB"},
    {0}
};

///////////////////////////////////////////////////////////
//
// Funtions:            PPP_Add_config:set_max_value()
// description: set maximum inputs lengths
// ParametersL          none
// Returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_max_values()
{
    ppp_add_config_script->maximum_size(4095);
    ppp_add_config_input->maximum_size(60);
    //add_dns_domain->maximum_size(29);
}

///////////////////////////////////////////////////////////
//
// Function:            PPP_Add_Config::set_config_value()
// Description: set the config input
// Parameters:          chat *value - value string
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_config_value(char *value)
{
    ppp_add_config_input->value(value);
}

///////////////////////////////////////////////////////////
//
// functon:                     PPP_Add_Config::set_device_value
// description: set the device choice
// parameters:          chat *value - string
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_device_value(char *value)
{
    int size = ppp_add_config_dev_choice->size();

    for (int idx = 0; idx < size; idx++) {
	if (0 == strcmp(value, ppp_add_config_dev_choice->text(idx))) {
	    ppp_add_config_dev_choice->value(idx);
	    break;
	}
    }
}

///////////////////////////////////////////////////////////
//
// funtions:            PPP_Add_Config::set_speed_value()
// description: set the speed choice
// parameters:          chat *value - string value
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_speed_value(char *value)
{
    int size = ppp_add_config_speed_choice->size();

    for (int idx = 0; idx < size; idx++) {
	if (0 == strcmp(value, ppp_add_config_speed_choice->text(idx))) {
	    ppp_add_config_speed_choice->value(idx);
	    break;
	}
    }
}

///////////////////////////////////////////////////////////
//
// function:            PPP_Add_Config::set_dest_value()
// description: set the dest inputs
// parameters:          char *value - string value
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_dest_value(char *value)
{
    ppp_add_config_ip_dest->set_ip_inputs(value, false);
}

////////////////////////////////////////////////////////////
//
// functon:                     PPP_Add_Config::set_source_value()
// description: set the source ip value
// parameters:          char *value - string value
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_source_value(char *value)
{
    ppp_add_config_ip_source->set_ip_inputs(value, false);
}

///////////////////////////////////////////////////////////
//
// function:            PPP_Add_Config::set_broadcast_value()
// description: set the broadcast ip value
// paremeters;          char *value - string value
// returns:             none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_broadcast_value(char *value)
{
    ppp_add_config_broadcast->set_ip_inputs(value, false);
}

///////////////////////////////////////////////////////////
//
// function:            PPP_Add_config::set_gateway_value
// description: set the gateway ip value
// parameters:          char *value - string value
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_gateway_value(char *value)
{
    ppp_add_config_gateway->set_ip_inputs(value, false);
}

///////////////////////////////////////////////////////////
//
// function:            PPP_Add_config::set_dhcp_value
// description: set the dhcp button value
// parameters:          char *value - string value 
// returns:                     none
//
//\////////////////////////////////////////////////////////
void
PPP_Add_Config::set_dhcp_value(char *value)
{
    if (0 == strcmp("on", value)) {
	dhcp_button->value(1);
    }
    if (0 == strcmp("off", value)) {
	dhcp_button->value(0);
    }
    dhcp_button->do_callback();
}

///////////////////////////////////////////////////////////
//
//      functoin:               PPP_Add_Config::set_auto_value()
//      description:    set the auto button value
//      parameters:             char *value - string value
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_auto_value(char *value)
{
    if (0 == strcmp("on", value)) {
	auto_button->value(1);
    }
    if (0 == strcmp("off", value)) {
	auto_button->value(0);
    }
    auto_button->do_callback();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::set_script_value()
//      description:    set the script multiline input value
//      parameters:             char *value - string value
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_script_value(char *value)
{
    ppp_add_config_script->value(value);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::set_dns_)domain_value()
//      description:    set the dns input value
//      parameters:             const chat *value - string value
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_dns_domain_value(const char *value)
{
    add_dns_domain->value(value);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::set_dns_ip_value()
//      descripton:             set the dns ip input value
//      parameters:             char *value - string value
//      returnsL                        none
//      
///////////////////////////////////////////////////////////
void
PPP_Add_Config::set_dns_ip_value(char *value)
{
    add_dns_ip->set_ip_inputs(value, false);
}

///////////////////////////////////////////////////////////
// 
// function:            PPP_Add_Config::get_config_value
// descriptin:          get the config input value
// paremeters:          none
// returns:                     const chat * - input value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_config_value()
{
    return ppp_add_config_input->value();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_device_value()
//      description:    get the device choice value
//      parameters:             none
//      returns:                        const char * - choice value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_device_value()
{
    int val = ppp_add_config_dev_choice->value();

    return ppp_add_config_dev_choice->text(val);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_speed_value()
//      description:    get the speed choice value
//      parameters:             none
//      returns:                        const char * - choice value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_speed_value()
{
    int val = ppp_add_config_speed_choice->value();

    return ppp_add_config_speed_choice->text(val);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_dest_value()
//      description:    get the dest ip value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_dest_value()
{
    return ppp_add_config_ip_dest->get_ip_inputs();
}

///////////////////////////////////////////////////////////
//
//      functon:                        PPP_Add_Config::get_source_value()
//      description:    get the source ip value
//      parameters:             none
 //     returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_source_value()
{
    return ppp_add_config_ip_source->get_ip_inputs();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_broadcast_value()
//      description:    get the ip broadcaset value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_broadcast_value()
{
    return ppp_add_config_broadcast->get_ip_inputs();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_gateway_value()
//      description:    get the ip gateway value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_gateway_value()
{
    return ppp_add_config_gateway->get_ip_inputs();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_dhcp_value()
//      descri[rion:    get the dhcp button value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_dhcp_value()
{
    if (0 == dhcp_button->value()) {
	return "off";
    }

    if (1 == dhcp_button->value()) {
	return "on";
    }
    return "off";
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_auto_value()
//      description:    get the auto button value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_auto_value()
{
    if (0 == auto_button->value()) {
	return "off";
    }
    if (1 == auto_button->value()) {
	return "on";
    }
    return "off";
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_script_value()
//      desciption:             get the script input value
//      parameters:             none
//      returns:                        const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_script_value()
{
    return ppp_add_config_script->value();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_dns_domain_value
//      description:    get the dns domain input value
//      parameters:             none
//      returns                         const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_dns_domain_value()
{
    return add_dns_domain->value();
}

///////////////////////////////////////////////////////////
//
// function:            PPP_Add_Config::get_dns_ip_value
// description: get the dns ip input value
// parameters:          none
// returns:                     const char * - value
//
///////////////////////////////////////////////////////////
const char *
PPP_Add_Config::get_dns_ip_value()
{
    return add_dns_ip->get_ip_inputs();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::get_ttys()
//      description:    fill in the dev choice menu with ttys
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////     
void
PPP_Add_Config::get_ttys()
{
    char file[255];
    int num = 0;
    struct stat stat_buf;

    sprintf(file, "/dev/ttySA%d", num);
    while (0 == stat(file, &stat_buf)) {
	sprintf(file, "/dev/ttySA%d", num);
	if (0 == stat(file, &stat_buf)) {
	    sprintf(file, "\\/dev\\/ttySA%d", num);
	    ppp_add_config_dev_choice->add(file);
	}
	sprintf(file, "/dev/ttySA%d", num);
	num++;
    }

    sprintf(file, "/dev/modem");
    if (0 == stat(file, &stat_buf)) {
	sprintf(file, "\\/dev\\/modem");
	ppp_add_config_dev_choice->add(file);
    }
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::ppp_add_config_dhcp_cb()
//      description:    callback when dhcp button is pressed
//      parameters:             Fl_Widget *widget - pointer to the calling widget
//                                              void *parent - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::ppp_add_config_dhcp_cb(Fl_Widget * widget, void *parent)
{
    NxRadioRoundButton *button = (NxRadioRoundButton *) widget;
    PPP_Add_Config *config = (PPP_Add_Config *) parent;

    if (button->value()) {
	config->ppp_add_config_ip_dest->deactivate();
	config->ppp_add_config_ip_source->deactivate();
    } else {
	config->ppp_add_config_ip_dest->activate();
	config->ppp_add_config_ip_source->activate();
    }
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::ppp_add_config_auto_cb()
//      description:    callback when auto button is pressed
//      parameters:             Fl_Widget *widget - pointer to calling widget
//                                              void *parent - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Add_Config::ppp_add_config_auto_cb(Fl_Widget * widget, void *parent)
{
    NxRadioRoundButton *button = (NxRadioRoundButton *) widget;
    //PPP_Add_Config *config = (PPP_Add_Config *)parent;

    if (0 == button->value()) {
	//config->add_dns_domain->activate();
	//config->add_dns_ip->activate();
    }
    if (1 == button->value()) {
	//config->add_dns_domain->deactivate();
	//config->add_dns_ip->deactivate();
    }
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::set_ip_info()
//      description:    set the ip inputs from saved values
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////     
void
PPP_Add_Config::set_ip_info()
{
    char local_ip[20] = "";
    char dest_ip[20] = "";
    pix_comm_ppp_options_t ppp_options;
    int status;
    char config_value[60];

    status = pix_comm_if_active("ppp0");

    if (status == PIX_COMM_ACTIVE) {
	pix_comm_ppp_get_ip_info("ppp0", &ppp_options);
	sprintf(local_ip, "%d.%d.%d.%d",
		(unsigned char) (ppp_options.local_ipaddr >> 24) & 0xFF,
		(unsigned char) (ppp_options.local_ipaddr >> 16) & 0xFF,
		(unsigned char) (ppp_options.local_ipaddr >> 8) & 0xFF,
		(unsigned char) ppp_options.local_ipaddr & 0xFF);

	sprintf(dest_ip, "%d.%d.%d.%d",
		(unsigned char) (ppp_options.remote_ipaddr >> 24) & 0xFF,
		(unsigned char) (ppp_options.remote_ipaddr >> 16) & 0xFF,
		(unsigned char) (ppp_options.remote_ipaddr >> 8) & 0xFF,
		(unsigned char) ppp_options.remote_ipaddr & 0xFF);
    } else {
	strcpy(config_value, get_config_value());
	IniGetString(config_value, "dest", NULL, dest_ip, sizeof(dest_ip) - 1,
		     CONFIG_FILE);
	IniGetString(config_value, "source", NULL, local_ip,
		     sizeof(local_ip) - 1, CONFIG_FILE);
    }
    ppp_add_config_ip_dest->set_ip_inputs(dest_ip, false);
    ppp_add_config_ip_source->set_ip_inputs(local_ip, false);
}


///////////////////////////////////////////////////////////
//
//      function:               PPP_Add_Config::PPP_Add_Config
//      description:    PPP_Add_Config class instatiation
//      parameters:             Fl_tabs *add_config_tabw - pointer to parent tabs
//      returns:                        none
//
///////////////////////////////////////////////////////////
PPP_Add_Config::PPP_Add_Config()
{

    config_win = new NxPimWindow("Add Config", setupMenuItems, MENU);
//  config_win->GetWindowPtr()->resizable(NULL);
    NetConfig::I()->add_window((Fl_Window *) config_win->GetWindowPtr());

    {
	ppp_add_config_group = new Fl_Group(W_X, W_Y, W_W, W_H);

	// Ok and Cancel buttons
	ok_button =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Ok");
	cancel_button =
	    new NxButton(BUTTON_X + BUTTON_WIDTH + 3, BUTTON_Y, BUTTON_WIDTH,
			 BUTTON_HEIGHT, "Cancel");

	ppp_add_config_group->add(ok_button);
	ppp_add_config_group->add(cancel_button);

	int i_oset = 15;
	int c_oset = 15;
	ppp_add_config_input =
	    new NxInput(MB_X + 78, MB_Y + (1 * MB_H + i_oset), W_W - 160, 20,
			"Config Name");

	// Device
	ppp_add_config_dev_choice =
	    new NxMenuButton(MB_X + 3, MB_Y + (3 * MB_H + i_oset), MB_W + 10,
			     MB_H, "Device");
	ppp_add_config_dev_choice->callback(device_cb);
	ppp_add_config_dev_choice->movable(true);
	get_ttys();

	// Speed
	ppp_add_config_speed_choice =
	    new NxMenuButton(MB_X + 3, MB_Y + (5 * MB_H + i_oset), MB_W + 10,
			     MB_H, "Speed");
	ppp_add_config_speed_choice->menu(ppp_speed_menu);
	ppp_add_config_speed_choice->callback(speed_cb);
	ppp_add_config_speed_choice->movable(true);

	// Auto or DHCP
	dhcp_button =
	    new NxCheckButton(MB_X + 39, MB_Y + (7 * MB_H + c_oset), 20, 20,
			      "DHCP");
	dhcp_button->align(FL_ALIGN_LEFT);
	dhcp_button->callback(ppp_add_config_dhcp_cb, this);
	dhcp_button->movable(true);

	auto_button =
	    new NxCheckButton(MB_X + 86, MB_Y + (7 * MB_H + c_oset), 20, 20,
			      "Auto");
	auto_button->align(FL_ALIGN_LEFT);
	auto_button->callback(ppp_add_config_auto_cb, this);
	auto_button->movable(true);

	ppp_add_config_ip_dest =
	    new IP_Input(MB_X + 63, MB_Y + (9 * MB_H + c_oset), 140, 20,
			 "Dest IP    ");
	ppp_add_config_ip_source =
	    new IP_Input(MB_X + 63, MB_Y + (11 * MB_H + i_oset), 140, 20,
			 "Source IP");

	//ppp_add_config_gateway = new IP_Input(85, 105, 130, 20, "Gateway:");
	//ppp_add_config_broadcast = new IP_Input(85, 130, 130, 20, "Broadcast:");


	//add_dns_domain = new NxInput(85, 180, 130, 20, "Domainname:");
	//add_dns_domain->hide();
	//add_dns_ip = new IP_Input(85, 205, 130, 20, "DNS-Server IP:");
	//add_dns_ip->hide();

	ppp_add_config_script =
	    new NxMultilineInput(BUTTON_X, BUTTON_Y - BUTTON_HEIGHT - 20,
				 W_W - 15, 30, "Chat Script");
	ppp_add_config_script->align(FL_ALIGN_TOP);


	ppp_add_config_group->end();
	config_win->add((Fl_Widget *) ppp_add_config_group);
    }

}

/***************************ppp_modem_setup**********************************/

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_setup::edit_config_value
//      description:    get the config values from config file
//      parameters:             PPP_Add_config *config - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::edit_config_value(PPP_Add_Config * config)
{
    char config_value[60];
    char dev_choice[20];
    char speed_choice[15];
    char ip_dest[20] = "";
    char ip_source[20] = "";
    //char ip_broadcast[20] = "";
    //char ip_gateway[20] = "";
    char dhcp[5];
    char autos[5] = "";
    //char dnsip[16] = "";
    char domainname[30] = "";
    char file[70];
    int val = 0;

    val = ppp_modem_setup_browser->value();
    if (0 >= val) {
	return;
    }

    strcpy(config_value, ppp_modem_setup_browser->text(val));
    IniGetString(config_value, "device", NULL, dev_choice,
		 sizeof(dev_choice) - 1, CONFIG_FILE);
    IniGetString(config_value, "speed", NULL, speed_choice,
		 sizeof(speed_choice) - 1, CONFIG_FILE);
    IniGetString(config_value, "dest", NULL, ip_dest, sizeof(ip_dest) - 1,
		 CONFIG_FILE);
    IniGetString(config_value, "source", NULL, ip_source,
		 sizeof(ip_source) - 1, CONFIG_FILE);
    //IniGetString(config_value, "broadcast", NULL, ip_broadcast, sizeof(ip_broadcast) - 1, CONFIG_FILE);
    //IniGetString(config_value, "gateway", NULL, ip_gateway, sizeof(ip_gateway) - 2, CONFIG_FILE);
    IniGetString(config_value, "dhcp", NULL, dhcp, sizeof(dhcp) - 1,
		 CONFIG_FILE);
    IniGetString(config_value, "auto", NULL, autos, sizeof(autos) - 1,
		 CONFIG_FILE);
    //IniGetString(config_value, "dnsip", NULL, dnsip, sizeof(dnsip) - 1, CONFIG_FILE);
    IniGetString(config_value, "domainname", NULL, domainname,
		 sizeof(domainname) - 1, CONFIG_FILE);
    config->set_config_value(config_value);
    config->set_device_value(dev_choice);
    config->set_speed_value(speed_choice);
    config->set_dest_value(ip_dest);
    config->set_source_value(ip_source);
    //config->set_broadcast_value(ip_broadcast);
    //config->set_gateway_value(ip_gateway);
    config->set_dhcp_value(dhcp);
    config->set_auto_value(autos);
    //config->set_dns_domain_value(domainname);
    //config->set_dns_ip_value(dnsip);    
    sprintf(file, "%s%s", SCRIPT_PREFIX, config_value);
    get_script_file(file, config);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::change_config_value()
//      description:    change the config value to one not in use
//      parameters:             PPP_Add_Config *config - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::change_config_value(PPP_Add_Config * config)
{
    char buf[20];
    bool match = true;
    int configs = ppp_modem_setup_browser->size();
    int nums = 1;

    sprintf(buf, "NewConfig%d", nums);
    while (match) {
	for (int idx = 1; idx <= configs; idx++) {
	    if (0 == strcmp(buf, ppp_modem_setup_browser->text(idx))) {
		match = true;
		idx = 0;
		sprintf(buf, "NewConfig%d", ++nums);
	    }
	}
	match = false;
    }
    config->set_config_value(buf);
    config->set_device_value(0);
    config->set_speed_value(0);
    config->set_dhcp_value("on");
    config->set_auto_value("on");
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::write_config()
//      description:    write the config file
//      parameters:             PPP_Add_Config *config - pointer to parnet object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::write_config(PPP_Add_Config * config)
{
    char cfg[60];
    char dialer_cfg[90];
    char script_file[70];

    strcpy(cfg, config->get_config_value());
    IniWriteString(cfg, "device", (char *) config->get_device_value(),
		   CONFIG_FILE);
    IniWriteString(cfg, "speed", (char *) config->get_speed_value(),
		   CONFIG_FILE);
    IniWriteString(cfg, "dest", (char *) config->get_dest_value(),
		   CONFIG_FILE);
    IniWriteString(cfg, "source", (char *) config->get_source_value(),
		   CONFIG_FILE);
    //IniWriteString(cfg, "broadcast",  (char *)config->get_broadcast_value(), CONFIG_FILE);
    //IniWriteString(cfg, "gateway", (char *)config->get_gateway_value(), CONFIG_FILE);
    IniWriteString(cfg, "dhcp", (char *) config->get_dhcp_value(),
		   CONFIG_FILE);
    IniWriteString(cfg, "auto", (char *) config->get_auto_value(),
		   CONFIG_FILE);
    //IniWriteString(cfg, "domainname", (char *)config->get_dns_domain_value(), CONFIG_FILE);
    //IniWriteString(cfg, "dnsip", (char *)config->get_dns_ip_value(), CONFIG_FILE);      
    sprintf(script_file, "%s%s", SCRIPT_PREFIX, cfg);
    write_script_file(script_file, config);
    sprintf(dialer_cfg, "Dialer %s", cfg);
    IniWriteString(dialer_cfg, "Modem", (char *) config->get_device_value(),
		   DIALER_FILE);
    IniWriteString(dialer_cfg, "Baud", (char *) config->get_speed_value(),
		   DIALER_FILE);

    chmod(CONFIG_FILE, MODE_RDWR);
    chmod(DIALER_FILE, MODE_RDWR);
    chmod(script_file, MODE_RDWR);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::write_script_file()
//      description:    write out the script file for this config
//      parameters:             char *file - file to write to
//                                              PPP_Add_Config *config - pointer to parent object
//
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::write_script_file(char *file, PPP_Add_Config * config)
{
    FILE *fp;

    if (0 == strcmp(config->get_script_value(), "")) {
	remove(file);
	return;
    }
    if ((fp = fopen(file, "w+")) == NULL) {
	return;
    }
    fputs(config->get_script_value(), fp);
    fclose(fp);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_setup::get_script_file()
//      description:    get the script input from a file
//      parameters:             char *file - file to get script form
//                                              PPP_Add_Config *config - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::get_script_file(char *file, PPP_Add_Config * config)
{
    FILE *fp;
    char script[MAX_SCRIPT_SIZE] = "";
    char buf[4096];
    off_t pos = 0;

    if ((fp = fopen(file, "r")) == NULL) {
	return;
    }
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	pos += strlen(buf);
	if (MAX_SCRIPT_SIZE < pos) {
	    off_t diff = pos - MAX_SCRIPT_SIZE;
	    strncat(script, buf, strlen(buf) - diff - 1);
	    break;
	} else {
	    strncat(script, buf, strlen(buf));
	}
    }
    config->set_script_value(script);
    fclose(fp);
}


///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::edit_config()
//      description:    edit the config value
//      parameters:             PPP_Add_config * config - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::edit_config(PPP_Add_Config * config)
{
    char cfg[60];

    strcpy(cfg, config->get_config_value());
    if (0 == strcmp(cfg, "")) {
	change_config_value(config);
	strcpy(cfg, config->get_config_value());
    }
    for (int idx = 1; idx <= ppp_modem_setup_browser->size(); idx++) {
	if (0 == strcmp(cfg, ppp_modem_setup_browser->text(idx))) {
	    write_config(config);
	    return;
	}
    }
    fl_addto_browser(ppp_modem_setup_browser, cfg);
    account_modem->add_config(cfg);
    write_config(config);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::add_config
//      description:    add config value to config browser and file
//      parameters:             PPP_Add_config *config - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::add_config(PPP_Add_Config * config)
{
    char cfg[60];

    strncpy(cfg, config->get_config_value(), 59);
    if (0 == strcmp(cfg, "")) {
	change_config_value(config);
	strcpy(cfg, config->get_config_value());
    }
    fl_addto_browser(ppp_modem_setup_browser, get_config_value(config));
    account_modem->add_config(cfg);
    write_config(config);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::ppp_delete_config_cb()
//      descriptioin:   delete config file from config browser and file
//      parameters:             Fl_Widget *widget - pointer to calling widget
//                                              void *parent - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::ppp_delete_config_cb(Fl_Widget * widget, void *parent)
{
    char cfg[60];
    int val = 0;
    char file[70];
    PPP_Modem_Setup *setup = (PPP_Modem_Setup *) parent;

    val = setup->ppp_modem_setup_browser->value();
    if (0 >= val) {
	return;
    }
    strcpy(cfg, setup->ppp_modem_setup_browser->text(val));
    IniDelSection(cfg, CONFIG_FILE);
    fl_delete_browser_line(setup->ppp_modem_setup_browser, val);
    fl_select_browser_line(setup->ppp_modem_setup_browser, 1);
    setup->account_modem->delete_config(val);
    sprintf(file, "%s%s", SCRIPT_PREFIX, cfg);
    unlink(file);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::ppp_add_config_cb()
//      description:    callback for add/edit config 
//      parameters:             Fl_Widget *widget - pointer to calling widget
//                                              void *parent - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::ppp_add_config_cb(Fl_Widget * widget, void *parent)
{
    NxButton *button = (NxButton *) widget;
    PPP_Modem_Setup *setup = (PPP_Modem_Setup *) (parent);
    PPP_Modem *this_modem = setup->get_ppp_modem();
    PPP_Add_Config *config = this_modem->_add_config;

    if (button == setup->ppp_modem_setup_add_button) {
	setup->change_config_value(config);
    }

    if (button == setup->ppp_modem_setup_edit_button) {
	if (setup->ppp_modem_setup_browser->size() == 0)
	    return;
	setup->edit_config_value(config);
    }

    config->set_ip_info();

    this_modem->_add_config->show();


    while (this_modem->_add_config->get_parent_window()->shown()) {
	Fl::wait();
	for (;;) {
	    Fl_Widget *o = Fl::readqueue();
	    if (!o) {
		break;
	    }
	    if (o == config->ok_button) {
		if (button == setup->ppp_modem_setup_add_button) {
		    setup->add_config(config);
		}
		if (button == setup->ppp_modem_setup_edit_button) {
		    setup->edit_config(config);
		}
		this_modem->_add_config->hide();
		setup->show();

		//      parent_win->show();
		//      parent_win->remove(&ppp_config_win);
		return;
	    }
	    if ( /*o == &ppp_config_win || */ o == config->cancel_button) {
		//      ppp_config_win.hide();
		this_modem->_add_config->hide();
		setup->show();
		//      parent_win->show();
		//      parent_win->remove(&ppp_config_win);
		return;
	    }
	}
    }

}


///////////////////////////////////////////////////////////
//
//      functoin:               PPP_Modem_Setup::fill_config_browser()
//      description:    fill config browser from config file
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::fill_config_browser()
{
    int size = account_modem->get_num_configs();

    for (int idx = 0; idx < size; idx++) {
	fl_addto_browser(ppp_modem_setup_browser,
			 account_modem->get_config_name(idx));
    }
    fl_set_browser_topline(ppp_modem_setup_browser, 1);
    fl_select_browser_line(ppp_modem_setup_browser, 1);
}

///////////////////////////////////////////////////////////
//
//      fucntion:               PPP_Modem_Setup::hide()
//      description:    hide the object
//      parameters:             none
//      returns:                        none
//
////////////////////////////////////////////////////////////
void
PPP_Modem_Setup::hide()
{

    get_parent_win()->hide();
    /*
       ppp_modem_setup_add_button->show();
       ppp_modem_setup_edit_button->show();
       ppp_modem_setup_delete_button->show();
     */
}

void
PPP_Add_Config::show()
{
    get_parent_window()->show();
}

void
PPP_Add_Config::hide()
{
    get_parent_window()->hide();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::show()
//      description:    show the object
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem_Setup::show()
{
    get_parent_win()->show();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem_Setup::PPP_Modem_Setup
//      description:    PPP_Modem_Setup class instatiation function
//      parameters:             Fl_Tabs *ppp_setup_tabs - pointer to parent tabs
//                                              PPP_Modem *this_modem - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////
PPP_Modem_Setup::PPP_Modem_Setup(PPP_Modem * this_modem)
{
    account_modem = this_modem;

    modem_setup_win = new NxPimWindow("Setup", setupMenuItems, MENU);
    NetConfig::I()->add_window((Fl_Window *) modem_setup_win->GetWindowPtr());

    ppp_setup_group = new Fl_Group(W_X, W_Y, W_W, W_H);

    ppp_modem_setup_browser = new NxHoldBrowser((240 - (MENU_W) + 20) / 2,
						2 * MENU_Y + MENU_H + 5,
						MENU_W - 20,
						BUTTON_Y - 5 -
						(BUTTON_HEIGHT + MENU_Y +
						 MENU_H), "Configurations");
    ppp_modem_setup_browser->align(FL_ALIGN_TOP);

    ppp_modem_setup_add_button = new NxButton(3, BUTTON_Y, BUTTON_WIDTH - 4,
					      BUTTON_HEIGHT, "Add");
    ppp_modem_setup_edit_button = new NxButton(62, BUTTON_Y, BUTTON_WIDTH - 4,
					       BUTTON_HEIGHT, "Edit");
    ppp_modem_setup_delete_button =
	new NxButton(121, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
		     "Delete");

    ppp_modem_setup_add_button->callback(ppp_add_config_cb, this);
    ppp_modem_setup_edit_button->callback(ppp_add_config_cb, this);
    ppp_modem_setup_delete_button->callback(ppp_delete_config_cb, this);

    {
	NxButton *o =
	    new NxButton(181, BUTTON_Y, BUTTON_WIDTH - 4, BUTTON_HEIGHT,
			 "Done");
	o->callback(NetConfig::show_modem);
    }

    ppp_setup_group->end();

    modem_setup_win->add((Fl_Widget *) ppp_setup_group);

    fill_config_browser();
}

/********************ppp_modem**************************************/

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::set_ppp_options()
//      description:    set get options for config and account
//      parameters:             none
//      returns:                        none
//              
///////////////////////////////////////////////////////////             
void
PPP_Modem::set_ppp_options()
{
    char account_value[25];
    char config_value[25];
    char phone[20] = "";
    char password[25] = "";
    char username[25] = "";
    char device[128] = "";
    char speed[10] = "";
    char dest_ip[16] = "";
    char source_ip[16] = "";
    //char broadcast_ip[16] = "";
    //char gateway_ip[16] = "";

    if (ppp_config_choice->text()) {
	strcpy(config_value, ppp_config_choice->text());
    } else {
	strcpy(config_value, "");
    }
    if (ppp_account_choice->text()) {
	strcpy(account_value, ppp_account_choice->text());
    } else {
	strcpy(account_value, "");
    }
    IniGetString(account_value, "number", NULL, phone, sizeof(phone) - 1,
		 ACCOUNT_FILE);
    IniGetString(account_value, "password", NULL, password,
		 sizeof(password) - 1, ACCOUNT_FILE);
    IniGetString(account_value, "username", NULL, username,
		 sizeof(username) - 1, ACCOUNT_FILE);
    IniGetString(config_value, "device", NULL, device, sizeof(device) - 1,
		 CONFIG_FILE);
    IniGetString(config_value, "speed", NULL, speed, sizeof(speed) - 1,
		 CONFIG_FILE);
    IniGetString(config_value, "dest", NULL, dest_ip, sizeof(dest_ip) - 1,
		 CONFIG_FILE);
    IniGetString(config_value, "source", NULL, source_ip,
		 sizeof(source_ip) - 1, CONFIG_FILE);

    unsigned long val = strtol(speed, NULL, 10);
    ppp_options.speed = val;
    strcpy(ppp_options.device, device);
    strcpy(ppp_options.telephone, phone);
    strcpy(ppp_options.account, username);
    strcpy(ppp_options.password, password);
    ppp_options.local_ipaddr = inet_network(source_ip);
    ppp_options.remote_ipaddr = inet_network(dest_ip);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::get_script()
//      description:    get script value from file
//      parameters:             char *script - pointer to script value
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::get_script(char *script)
{
    char file[255] = "";
    FILE *fp;
    char buf[4096] = "";
    off_t pos = 0;

    sprintf(file, "%s%s", SCRIPT_PREFIX, ppp_config_choice->text());
    if ((fp = fopen(file, "r")) == NULL) {
	return;
    }
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	pos += strlen(buf);
	if (MAX_SCRIPT_SIZE < pos) {
	    off_t diff = pos - MAX_SCRIPT_SIZE;
	    strncat(script, buf, strlen(buf) - diff - 1);
	    break;
	} else {
	    strncat(script, buf, strlen(buf));
	}
    }
    fclose(fp);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::set_pppd_command()
//      descriptons:    fille out the array of command strings from the script
//      parameters;             char *script - script string
//                                              int *count - count of how many string in array
//      returns:                        char ** - pointer to array of string
//
///////////////////////////////////////////////////////////
char **
PPP_Modem::set_pppd_command(char *script, int *count)
{
    int byte_count = 0;
    unsigned int idx;
    int c;
    int new_line = 0;
    char **pppd_command;
    char *buf;
    char *tmp_ptr;

    if (0 == strcmp("", script)) {
	return (NULL);
    }
    buf = (char *) calloc(MAX_SCRIPT_SIZE + 1, sizeof(char));
    if (NULL == buf) {
	perror("PPP_Modem (CALLOC)");
	return (NULL);
    }
    tmp_ptr = buf;

    strcpy(buf, script);
    for (idx = 0; idx < strlen(script); idx++) {
	c = script[idx];
	if (c == '\n' || c == '\0') {
	    new_line++;
	}
    }
    *count = new_line;
    if ((pppd_command =
	 (char **) calloc(new_line + 1, sizeof(char *))) == NULL) {
	perror("PPP_Modem (CALLOC)");
	return (NULL);
    }
    new_line = 0;
    for (idx = 0; idx < strlen(script); idx++) {
	byte_count++;
	c = script[idx];
	if (c == '\n' || c == '\0') {
	    pppd_command[new_line] =
		(char *) calloc(byte_count + 1, sizeof(char));
	    if (pppd_command[new_line] == NULL) {
		perror("PPP_Modem (CALLOC)");
		if (NULL != buf) {
		    buf = tmp_ptr;
		    free(buf);
		}
		free(pppd_command);
		return (NULL);
	    }
	    strncpy(pppd_command[new_line], buf, byte_count - 1);
	    buf += byte_count;
	    byte_count = 0;
	    new_line++;
	}
    }
    if (NULL != buf) {
	buf = tmp_ptr;
	free(buf);
    }
    return pppd_command;
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::ppp_connect_button_cb
//      descriptioin:   callback function for connect button
//      parameters:             Fl_widget *widget - pointer to calling widget
//                                              void *parent - pointer to parent object
// returns:                     none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::ppp_connect_button_cb(Fl_Widget * widget, void *parent)
{
    PPP_Modem *modem = (PPP_Modem *) parent;
    pix_comm_ppp_options_t ppp_options;
    char dhcp[5] = "";
    char config_value[25];
    int dhcp_i;
    char **pppd_command;
    char script[MAX_SCRIPT_SIZE] = "";
    int err;
    int count;

    modem->set_ppp_options();
    modem->set_status(pix_comm_if_active("ppp0"));

    if (PIX_COMM_ACTIVE == modem->get_status()) {
	pix_comm_ppp_disconnect("ppp0");
	sleep(7);
    } else {
	if (modem->ppp_config_choice->text()) {
	    strcpy(config_value, modem->ppp_config_choice->text());
	} else {
	    strcpy(config_value, "");
	}
	IniGetString(config_value, "dhcp", NULL, dhcp, sizeof(dhcp) - 1,
		     CONFIG_FILE);
	if (!strcasecmp("on", dhcp)) {
	    dhcp_i = 1;
	} else {
	    dhcp_i = 0;
	}
	ppp_options = modem->get_ppp_options();
	modem->get_script(script);
	if (0 != strcmp("", script)) {
	    pppd_command = modem->set_pppd_command(script, &count);
	    modem->set_status_box("    Attempting to start interface");
	    err =
		pix_comm_ppp_connect(PIX_COMM_PPP_MODEM, &ppp_options,
				     pppd_command, count, dhcp_i,
				     config_value);
	    if (NULL != pppd_command) {
		free(pppd_command);
	    }
	} else {
	    modem->set_status_box("    Attempting to start interface");
	    err = pix_comm_ppp_connect(PIX_COMM_PPP_MODEM, &ppp_options,
				       NULL, 0, dhcp_i, config_value);
	}
	if (err) {
	    modem->set_status_box("       Error starting  interface ppp0");
	    return;
	}
    }
    modem->set_status_box();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::ppp_device_setup_cb()
//      description:    callback function for setup of device/configuration
//      parameters:             Fl_Widget *widget - pointer to calling widget
//                                              void *parent - pointer to parent object
//      returns:                        none
//
///////////////////////////////////////////////////////////     
void
PPP_Modem::ppp_device_setup_cb(Fl_Widget * widget, void *parent)
{
    ((PPP_Modem *) parent)->ppp_device_setup->show();
}

///////////////////////////////////////////////////////////
//
//      fucntion:               PPP_Modem::ppp_account_setup_cb
//      description:    callback function for edit acounts
//      parameters:             Fl_Widget *widget - pointer to calling widget
//                                              vois *parent - pointer to parent object
//      retruns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::ppp_account_setup_cb(Fl_Widget * widget, void *parent)
{
    PPP_Modem *this_modem = (PPP_Modem *) parent;
    this_modem->_acct->show();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::get_config()
//      description:    get config values from file
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::get_configs()
{
    FILE *fp;
    char buf[MAX_LINE_LEN];
    char *p;
    char *configs[MAX_ACCOUNTS];

    if ((fp = fopen(CONFIG_FILE, "r")) == NULL) {
	return;
    }
    int idx = 0;
    for (idx = 0; idx < MAX_ACCOUNTS; idx++) {
	configs[idx] = NULL;
    }
    idx = 0;
    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
	if (MAX_ACCOUNTS < idx) {
	    break;
	}
	if (buf[0] == '[') {
	    p = buf;
	    p++;
	    configs[idx] = (char *) calloc(strlen(p) - 1, sizeof(char));
	    strncpy(configs[idx], p, strlen(p) - 2);
	    idx++;
	}
    }
    fclose(fp);

    for (idx = 0; NULL != configs[idx]; idx++) {
	ppp_config_choice->add(configs[idx]);
    }
    ppp_config_choice->value(0);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::get_devices()
//      description:    get devices from system
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::get_devices()
{
    ppp_count = MAX_PPP_DEVS;
    pix_comm_get_if_list(PIX_COMM_TYPE_PPP, ppp_list, &ppp_count);
}

/////////////////////////////////////////////////////////////
//
//      functoin:               PPP_Modem::get_accounts()
//      description:    fill the accounts chi\oice menu from account file
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::get_accounts()
{
    FILE *fp;
    char buf[MAX_LINE_LEN];
    char *p;
    char *accounts[MAX_ACCOUNTS];

    if ((fp = fopen(ACCOUNT_FILE, "r")) == NULL) {
	return;
    }
    int idx = 0;
    for (idx = 0; idx < MAX_ACCOUNTS; idx++) {
	accounts[idx] = NULL;
    }
    idx = 0;
    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
	if (MAX_ACCOUNTS < idx) {
	    break;
	}
	if (buf[0] == '[') {
	    p = buf;
	    p++;
	    accounts[idx] = (char *) calloc(strlen(p) - 1, sizeof(char));
	    strncpy(accounts[idx], p, strlen(p) - 2);
	    idx++;
	}
    }
    fclose(fp);

    for (idx = 0; NULL != accounts[idx]; idx++) {
	ppp_account_choice->add(accounts[idx]);
    }
    ppp_account_choice->value(0);
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::set_status_box()
//      description:    set the status box value based on the the current
//                                              state of the connection
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::set_status_box()
{
    _status = pix_comm_if_active("ppp0");
    if (PIX_COMM_ACTIVE == _status) {
	_active = true;
	ppp_status_box->value("         Interface ppp0 is active");
	ppp_connect_button->label("Disconnect");
	ppp_connect_button->redraw();
    } else {
	_active = false;
	ppp_status_box->value("      Interface ppp0 is not active");
	ppp_connect_button->label("Connect");
	ppp_connect_button->redraw();
    }
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Momde::hide()
//      description:    hide the PPP_Momdem object
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::hide()
{
    ppp_config_choice->hide();
    ppp_device_setup_button->hide();
    ppp_account_choice->hide();
    ppp_status_box->hide();
    ppp_connect_button->hide();
    ppp_account_setup_button->hide();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::show()
//      description:    show the PPP_Modem object
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::show()
{
    modem_win->GetWindowPtr()->show();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::check_ppp_dirs()
//      description:    check to see if directories exist and create them
//                                              if they don't exist
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
void
PPP_Modem::check_ppp_dirs()
{
    struct stat stat_buf;
    int err;

    err = stat(PPP_TMP_DIR, &stat_buf);
    if (0 > err) {
	err = mkdir(PPP_TMP_DIR, MODE_RDWRX);
	if (0 > err) {
	    perror("PPP_Modem (MKDIR)");
	}
	err = chmod(PPP_TMP_DIR, MODE_RDWRX);
	if (0 > err) {
	    perror("PPP_Modem (CHMOD)");
	}
    }
    err = stat(PPP_PEER_DIR, &stat_buf);
    if (0 > err) {
	err = mkdir(PPP_PEER_DIR, MODE_RDX);
	if (0 > err) {
	    perror("PPP_Modem (MKDIR)");
	}
    }
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Modem::PPP_Modem
//      description:    PPP_Mocdm class instatiation
//      parameters:             Fl_Tabs *tabs - pointer to parent tabs
//                                              Fl_Window *win - pointer to parent window
//      returns:                        none
//
///////////////////////////////////////////////////////////
PPP_Modem::PPP_Modem(Fl_Window * win)
{

    modem_win = new NxPimWindow("Modem Setup", modemMenuItems, MENU);
    NetConfig::I()->add_window((Fl_Window *) modem_win->GetWindowPtr());

    ppp_modem_group = new Fl_Group(W_X, W_Y, W_W, W_H);

    int oset = 15;

    // Configuration
    ppp_device_setup_button = new NxButton(BUTTON_X, MB_Y + (1 * MB_H + oset),
					   BUTTON_WIDTH, BUTTON_HEIGHT,
					   "Setup");
    ppp_device_setup_button->callback(ppp_device_setup_cb, this);
    ppp_device_setup_button->movable(false);
    ppp_config_choice =
	new NxMenuButton(BUTTON_X + (MB_W + 3), MB_Y + (1 * MB_H + oset),
			 MB_W + 20, MB_H, "Configuration");
    ppp_config_choice->callback(config_cb);

    // Connect to
    ppp_account_setup_button =
	new NxButton(BUTTON_X, MB_Y + (3 * MB_H + oset), BUTTON_WIDTH,
		     BUTTON_HEIGHT, "Setup");
    ppp_account_setup_button->callback(ppp_account_setup_cb, this);
    ppp_account_setup_button->movable(false);
    ppp_account_choice =
	new NxMenuButton(BUTTON_X + (MB_W + 3), MB_Y + (3 * MB_H + oset),
			 MB_W + 20, MB_H, "Connect to");
    ppp_account_choice->callback(account_cb);


    ppp_status_box =
	new NxOutput(0, MB_Y + (7 * MB_H + oset), W_W, 50, "Status");
    ppp_status_box->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    ppp_status_box->movable(false);
    NxApp::Instance()->big_font(ppp_status_box);

    ppp_connect_button = new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH,
				      BUTTON_HEIGHT, "Connect");
    ppp_connect_button->callback(ppp_connect_button_cb, this);

    ppp_modem_group->end();
    modem_win->add((Fl_Widget *) ppp_modem_group);

    get_accounts();
    get_configs();
    set_status_box();
    check_ppp_dirs();

    ppp_device_setup = new PPP_Modem_Setup(this);
    _add_config = new PPP_Add_Config();
    _acct = new PPP_Account(this);
    _add_account = new PPP_Add_Account();
}

///////////////////////////////////////////////////////////
//
//      function:               PPP_Momde::~PPP_Modem()
//      description:    PPP_Modem class destroyer
//      parameters:             none
//      returns:                        none
//
///////////////////////////////////////////////////////////
PPP_Modem::~PPP_Modem()
{
}
