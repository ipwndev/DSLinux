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



#ifndef _DNS_SETUP_H_
#define _DNS_SETUP_H_

/////// Includes ////////
#include <FL/Fl_Group.H>
#include <FL/forms.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Input.H>

#include "ip_input.h"

#include <nxapp.h>
#include <nxwindow.h>
#include <nxbutton.h>
#include <nxinput.h>

/////// Classes ///////
///////////////////////////////////////////////
//
// class:                       DNS_Setup
// description: The DNS_Setup class defines the
//                                      user interface for setting up
//                                      the DNS
///////////////////////////////////////////////                                         
class DNS_Setup
{
    Fl_Group *dns_setup_group_;
    Fl_Input *domain_;
    IP_Input *dns1_ip_;
    IP_Input *dns2_ip_;
    IP_Input *dns3_ip_;
    Fl_Input *search_domain_;
    NxButton *defaults_button_;
    NxButton *set_button_;

    NxPimWindow *dns_win_;

    FL_EXPORT void get_values_from_file();
    static FL_EXPORT void set_button_cb(Fl_Widget *, void *parent);
    static FL_EXPORT void defaults_button_cb(Fl_Widget *, void *parent);

  public:
      Fl_Window * get_parent_win()
    {
	return dns_win_->GetWindowPtr();
    }
    const char *get_domain_value()
    {
	return domain_->value();
    }
    const char *get_search_value()
    {
	return search_domain_->value();
    }
    char *get_dns1_value()
    {
	return dns1_ip_->get_ip_inputs(true);
    }
    unsigned long get_dns1_lvalue()
    {
	return dns1_ip_->iptol();
    }
    char *get_dns2_value()
    {
	return dns2_ip_->get_ip_inputs(true);
    }
    unsigned long get_dns2_lvalue()
    {
	return dns2_ip_->iptol();
    }
    char *get_dns3_value()
    {
	return dns3_ip_->get_ip_inputs(true);
    }
    unsigned long get_dns3_lvalue()
    {
	return dns3_ip_->iptol();
    }
    FL_EXPORT DNS_Setup();
    FL_EXPORT ~ DNS_Setup();
};

#endif
