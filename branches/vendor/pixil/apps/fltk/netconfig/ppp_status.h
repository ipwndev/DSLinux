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


#ifndef PPP_STATUS_H
#define PPP_STATUS_H

#include <FL/Fl_Widget.H>
#ifndef PPP_MODEM_H
#include "ppp_modem.h"
#endif

extern "C"
{
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
}

class PPP_Status:public Fl_Widget
{
    friend class PPP_Modem;
    Fl_Callback *connect_cb_;
    Fl_Callback *disconnect_cb_;
    Fl_Callback *setup_account_cb_;
    Fl_Callback *setup_device_cb_;
    Fl_Callback *cancel_cb_;
    PPP_Modem *modem_;
    bool dialing_, connected_, disconnect_wish_;
    time_t connect_time_;
    int hour_, minute_, second_;
    int fd_;
    char *speed_;
    ulong value_;
    ulong start_;
    FL_EXPORT int handle(int);
  protected:
      FL_EXPORT void draw(int, int, int, int)
    {
	return;
    }
    FL_EXPORT void draw()
    {
	return;
    }
  public:
    FL_EXPORT Fl_Callback_p get_connect_cb()const
    {
	return connect_cb_;
    }
    FL_EXPORT Fl_Callback_p get_disconnect_cb() const
    {
	return disconnect_cb_;
    }
    FL_EXPORT Fl_Callback_p get_setup_account_cb() const
    {
	return setup_account_cb_;
    }
    FL_EXPORT Fl_Callback_p get_setup_device_cb() const
    {
	return setup_device_cb_;
    }
    FL_EXPORT Fl_Callback_p get_cancel_cb() const
    {
	return cancel_cb_;
    }
    FL_EXPORT void set_connect_cb(Fl_Callback * c)
    {
	connect_cb_ = c;
    }
    FL_EXPORT void set_cancel_cb(Fl_Callback * c)
    {
	cancel_cb_ = c;
    }
    FL_EXPORT void set_disconnect_cb(Fl_Callback * c)
    {
	disconnect_cb_ = c;
    }
    FL_EXPORT void set_account_setup_cb(Fl_Callback * c)
    {
	setup_account_cb_ = c;
    }
    FL_EXPORT void set_device_setup_cb(Fl_Callback * c)
    {
	setup_device_cb_ = c;
    }
    FL_EXPORT void set_fd(int);
    FL_EXPORT void set_modem(PPP_Modem *);
    time_t get_connect_time() const
    {
	return connect_time_;
    }
    int get_fd() const
    {
	return fd_;
    }
    bool get_disconnect_wish() const
    {
	return disconnect_wish_;
    }
    bool get_dialing() const
    {
	return dialing_;
    }
    bool get_connected() const
    {
	return connected_;
    }
    FL_EXPORT void set_disconnect_wish(bool);
    FL_EXPORT void set_dialing(bool);
    FL_EXPORT void set_connected(bool);
    FL_EXPORT void value(ulong v);	// set to Unix time
    FL_EXPORT void value(int, int, int);	// set hour, minute, second
    ulong value() const
    {
	return value_;
    }
    int hour() const
    {
	return hour_;
    }
    int minute() const
    {
	return minute_;
    }
    int second() const
    {
	return second_;
    }
    FL_EXPORT void update_status();
    PPP_Modem *get_modem()
    {
	return modem_;
    }
    FL_EXPORT PPP_Status(int, int, int, int, const char *);
};

#endif
