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

#ifndef ALARM_H_
#define ALARM_H_

#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <nxdb.h>

extern "C"
{
#include <ipc/colosseum.h>
}

#define TOKEN						"^"
#define ALARM_D					"alarmd"
#define ALARM_DATABASE 	"alarm"
#define ALARM_INDEX			0

#define APP_NAME 				25
#define DESC						100
#define ID							4

#define MAX_LEN 				CL_MAX_MSG_LEN

#define ERROR_START			0x0001
#define ERROR_SEND			0x0002
#define ERROR_MAX_DB		0x0004

#define WAIT_CONNECT    0x0001
#define WAIT_RECONNECT  0x0002

#define STATUS_CLEAR			0x0000
#define STATUS_READ				0x0001
#define STATUS_WRITE_MSG	0x0002
#define STATUS_WRITE_ERR	0x0004
#define STATUS_UNKNOWN		0x0008
#define STATUS_RTC_SET		0x0010
#define STATUS_RTC_CLEAR  0x0020

#define MAX_RECS				1024

typedef struct desc_
{
    int fd;
    int alarm_recno;
    int recno;
    char appname[APP_NAME];
    char desc[DESC];
    long start_time;
    long end_time;
    char msg[255];
}
alarm_desc;

class Alarm_d
{
  private:
    NxDb * alarm_db_;
    alarm_desc desc_;
    fd_set fds_;
    int rtcfd_;
    int fd_;
    int flags_;
    long cur_time_;
    long alarm_time_;
    long time_interval_;
    char *inidir;
    int id_;
    int error_;
    static Alarm_d *instance_;
    int status_;
    void SetAlarm(time_t alarm_time);
    void ClearRtc();
  public:
      Alarm_d(int argc, char **argv);
     ~Alarm_d();
    int Monitor();
    long GetNextAlarm();
    void MarkEntry(int flag);
    void RemoveEntry();
    void ParseMsg(char *msg);
    void SetId();
    int ReadMsg();
    int SendMsg();
    void SendErrorMsg(char *service);
    void WaitCol(int flag);
    static void HandleSignal(int signal);
    void OpenAlarmDatabase();
    static Alarm_d *Instance();
    int GetFd()
    {
	return fd_;
    }
    int GetStatus()
    {
	return status_;
    }
    void SetStatus(int status)
    {
	status_ = status;
    }
    void CloseDB();
    void CheckAlarm(char *service, int recno);
};

#endif
