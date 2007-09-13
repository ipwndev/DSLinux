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

#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#ifdef CONFIG_NANOX
#include <wm/scrtoplib.h>
#endif

#include "alarm_d.h"
#include "rtc.h"

#define RECNO 			0
#define NAME				1
#define NAME_RECNO	2
#define DESCRIPTION 3
#define START_TIME	4
#define END_TIME		5
#define ALARM_TIME	6
#define STATUS			7

#define NUM_FIELDS  8


#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: Alarmd: " str, ## args)
#else
#define DPRINT(str, args...)
#endif

///////////
// Database

// Alarm
field cFields[] = {
    {'i', 1, 0}
    ,				// Field 0:RECNO
    {'c', APP_NAME, 0}
    ,				//                   1:NAME
    {'i', 1, 0}
    ,				//                           2:NAME_RECNO
    {'c', DESC, 0}
    ,				//                           3:DESCRIPTION
    {'l', 1, 0}
    ,				//                           4:START_TIME
    {'l', 1, 0}
    ,				//                           5:END_TIME
    {'l', 1, 0}
    ,				//                           6:ALARM_TIME
    {'i', 1, 0}
    ,				//                           7:STATUS
    {0}
};

// Database
fildes cFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			//     extension
    NUM_FIELDS,			// nFields
    &cFields[0]			// fieldlist
};


char *path = "/var/run/alarmd.pid";

Alarm_d *
    Alarm_d::instance_ =
    0;

Alarm_d::Alarm_d(int argc, char **argv)
{

    pid_t pid = 0;
    int pathfd_;
    char buf[512];
    int ret;

    memset(buf, 0, sizeof(buf));
    pid = getpid();
    pathfd_ = open(path, O_RDWR | O_TRUNC | O_CREAT);
    if (-1 == pathfd_) {
	perror("open(): /var/run/alarmd.pid");
	exit(errno);
    }
    sprintf(buf, "%d", pid);
    ret = write(pathfd_, buf, strlen(buf));
    if (-1 == ret) {
	perror("write(): pid");
	exit(errno);
    }
    close(pathfd_);

    alarm_db_ = new NxDb(argc, argv);
    optind = 1;
    inidir = alarm_db_->GetPath();
    OpenAlarmDatabase();
    desc_.fd = -1;
    desc_.alarm_recno = -1;
    desc_.recno = -1;
    memset(desc_.appname, 0, sizeof(desc_.appname));
    memset(desc_.desc, 0, sizeof(desc_.desc));
    flags_ = 0;
    cur_time_ = -1;
    alarm_time_ = -1;
    time_interval_ = -1;
    fd_ = -1;
    rtcfd_ = -1;
    error_ = 0;
    SetId();
    instance_ = this;
    status_ = STATUS_CLEAR;
}

Alarm_d *
Alarm_d::Instance()
{
    if (0 == instance_) {
	DPRINT("instance_ is messed up BAIL!!!\n");
	exit(-1);
    }
    return instance_;
}

void
Alarm_d::HandleSignal(int signal)
{

    DPRINT("Recieved signal [%d]\n", signal);

    if (SIGUSR1 == signal) {	// going to suspend need to set /dev/rtc
	Alarm_d::Instance()->SetAlarm(Alarm_d::Instance()->alarm_time_);
	Alarm_d::Instance()->SetStatus(STATUS_RTC_SET);
    } else if (SIGUSR2 == signal) {	// coming back from suspend close /dev/rtc
	Alarm_d::Instance()->ClearRtc();
	Alarm_d::Instance()->SetStatus(STATUS_RTC_CLEAR);
    } else if (SIGPIPE == signal) {
	switch (Alarm_d::Instance()->GetStatus()) {
	case STATUS_READ:
	    Alarm_d::Instance()->WaitCol(WAIT_RECONNECT);
	    break;
	case STATUS_WRITE_MSG:
	    DPRINT("Error: Not able to send message!\n");
	    Alarm_d::Instance()->MarkEntry(ERROR_SEND);
	    break;
	case STATUS_WRITE_ERR:
	    DPRINT("Error: Message being Dropped!\n");
	    break;
	default:
	    DPRINT("Error: Unknown Status!\n");
	    Alarm_d::Instance()->SetStatus(STATUS_UNKNOWN);
	    break;
	}
    } else {
	Alarm_d::Instance()->~Alarm_d();
	exit(0);
    }
}

void
Alarm_d::CloseDB()
{
    alarm_db_->Close(ALARM_DATABASE);
}

int
Alarm_d::Monitor()
{
    int err = 0;
    struct timeval tv;
    int retval = 0;
    int maxfd;

    WaitCol(WAIT_CONNECT);
    if (0 > fd_) {
	DPRINT("Unable to register %s\n", ALARM_D);
	exit(-1);
    }

    while (1) {

	if (status_ == STATUS_READ || status_ == STATUS_WRITE_ERR)
	    WaitCol(WAIT_RECONNECT);

	FD_ZERO(&fds_);
	FD_SET(fd_, &fds_);
	maxfd = fd_;
	alarm_time_ = GetNextAlarm();

	DPRINT("in monitor loop: alarm_time = %ld\n", alarm_time_);
	if (0 == alarm_time_) {
	    retval = select(maxfd + 1, &fds_, NULL, NULL, NULL);
	    if (EINTR == errno && -1 == retval) {
		DPRINT("select: NULL\n");
		continue;
	    }
	} else if (alarm_time_ > 0) {
	    cur_time_ = time(NULL);
	    time_interval_ = alarm_time_ - cur_time_;
	    if (0 >= time_interval_) {
		err = SendMsg();
		continue;
	    }
	    if (0 < time_interval_) {
		tv.tv_sec = time_interval_;
		tv.tv_usec = 0;
		DPRINT("time_interval_: [%ld]\n", time_interval_);
		retval = select(maxfd + 1, &fds_, NULL, NULL, &tv);
		if (EINTR == errno && -1 == retval) {
		    DPRINT("select: time_interval_\n");
		    continue;
		}
	    }
	}
	if (FD_ISSET(fd_, &fds_)) {
	    err = ReadMsg();
	    if (0 > err) {
		WaitCol(WAIT_RECONNECT);
		continue;
	    }
	} else if (!FD_ISSET(fd_, &fds_)) {
	    err = SendMsg();
	    if (0 > err)
		continue;
	} else
	    continue;
    }

}

void
Alarm_d::WaitCol(int flag)
{
    int temp_fd;

    flags_ = 0;
    temp_fd = fd_;
    close(fd_);

    if (WAIT_RECONNECT == flag) {
	fd_ = ClReconnect((unsigned char *) ALARM_D);
	while (0 > fd_) {
	    sleep(1);
	    DPRINT("Waiting for colosseum reconnect\n");
	    fd_ = ClReconnect((unsigned char *) ALARM_D);
	    if (CL_CLIENT_CONNECTED == fd_) {
		fd_ = temp_fd;
		break;
	    }
	}
    }
    if (WAIT_CONNECT == flag) {
	fd_ = ClRegister((unsigned char *) ALARM_D, &flags_);
	while (0 >= fd_) {
	    sleep(1);
	    DPRINT("Waiting for colosseum\n");
	    fd_ = ClRegister((unsigned char *) ALARM_D, &flags_);
	    if (CL_CLIENT_CONNECTED == fd_) {
		fd_ = temp_fd;
		break;
	    }
	}
    }
}

void
Alarm_d::ClearRtc()
{
    int ret = 0;

    DPRINT("Clearing rtc\n");

    if (0 > rtcfd_) {
	DPRINT("rtcfd_ [%d]\n", rtcfd_);
	return;
    }

    ret = ioctl(rtcfd_, RTC_AIE_OFF, 0);
    if (-1 == ret) {
	perror("ioctl: RTC_AIE_OFF");
	//exit(errno);
    }

    close(rtcfd_);
    DPRINT("closed rtcfd_\n");
    rtcfd_ = -1;
}

void
Alarm_d::SetAlarm(time_t alarm_time)
{
    time_t time_now;
    tm *tt;
    rtc_time rtc_tm;
    int ret = 0;

    if (0 >= alarm_time)
	return;

    rtcfd_ = open("/dev/rtc", O_RDWR);
    DPRINT("rtcfd_ [%d]\n", rtcfd_);

    if (-1 == rtcfd_) {
	perror("Unable to open /dev/rtc");
	if (errno)
	    return;
    }

    time_now = time(NULL);
    tt = gmtime(&time_now);

    rtc_tm.tm_sec = tt->tm_sec;
    rtc_tm.tm_min = tt->tm_min;
    rtc_tm.tm_hour = tt->tm_hour;
    rtc_tm.tm_mday = tt->tm_mday;
    rtc_tm.tm_mon = tt->tm_mon;
    rtc_tm.tm_year = tt->tm_year;

    ret = ioctl(rtcfd_, RTC_SET_TIME, &rtc_tm);
    if (-1 == ret) {
	perror("ioctl: RTC_SET_TIME");
	close(rtcfd_);
	return;
	//exit(errno);
    }
    DPRINT("set rtc to %d-%d-%d, %02d:%02d:%02d \n",
	   rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
	   rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    if (alarm_time > 2) {	// give few second headstart on wake-up
	alarm_time -= 2;
	tt = gmtime(&alarm_time);

	rtc_tm.tm_sec = tt->tm_sec;
	rtc_tm.tm_min = tt->tm_min;
	rtc_tm.tm_hour = tt->tm_hour;
	rtc_tm.tm_mday = tt->tm_mday;
	rtc_tm.tm_mon = tt->tm_mon;
	rtc_tm.tm_year = tt->tm_year;

	ret = ioctl(rtcfd_, RTC_ALM_SET, &rtc_tm);
	if (-1 == ret) {
	    perror("ioctl: RTC_ALM_SET");
	    close(rtcfd_);
	    return;
	    //exit(errno);
	}
	DPRINT("set alarm date/time %d-%d-%d, %02d:%02d:%02d\n",
	       rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
	       rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
    }

    ret = ioctl(rtcfd_, RTC_ALM_READ, &rtc_tm);
    if (-1 == ret) {
	perror("ioctl: RTC_ALM_READ");
	close(rtcfd_);
	return;
	//exit(errno);
    }

    DPRINT("rtc Alarm set to %02d:%02d:%02d\n",
	   rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    ret = ioctl(rtcfd_, RTC_AIE_ON, 0);
    if (-1 == ret) {
	perror("ioctl: RTC_AIE_ON");
	close(rtcfd_);
	return;
	//exit(errno);
    }

    DPRINT("Set interrupts for RTC\n");

}

long
Alarm_d::GetNextAlarm()
{
    int rec_array[MAX_RECS];
    int idx = 0;
    int small_rec = -1;
    char c_value[16];
    long l_value = LONG_MAX;
    long temp_value = 0;

    for (idx = 0; idx < MAX_RECS; idx++)
	rec_array[idx] = -1;

    alarm_db_->Select(ALARM_DATABASE, rec_array, MAX_RECS);

    for (idx = 0; idx < MAX_RECS; idx++) {
	if (-1 == rec_array[idx])
	    continue;
	alarm_db_->Extract(ALARM_DATABASE, rec_array[idx], STATUS, c_value);
	if (0 != atoi(c_value)) {
	    DPRINT("status [%d]\n", atoi(c_value));
	    continue;
	}
	DPRINT("idx [%d]\n", idx);
	alarm_db_->Extract(ALARM_DATABASE, rec_array[idx], ALARM_TIME,
			   c_value);
	temp_value = strtol(c_value, NULL, 10);
	DPRINT("temp_value [%ld]\n", temp_value);
	if (temp_value < l_value) {
	    DPRINT("temp_value smaller than l_value\n");
	    l_value = temp_value;
	    DPRINT("set l_value [%ld]\n", l_value);
	    small_rec = rec_array[idx];
	}
    }
    if (-1 == small_rec)
	return 0;
    else {
	alarm_db_->Extract(ALARM_DATABASE, small_rec, RECNO, c_value);
	desc_.alarm_recno = atoi(c_value);
	alarm_db_->Extract(ALARM_DATABASE, small_rec, NAME, desc_.appname);
	alarm_db_->Extract(ALARM_DATABASE, small_rec, NAME_RECNO, c_value);
	desc_.recno = atoi(c_value);
	alarm_db_->Extract(ALARM_DATABASE, small_rec, DESCRIPTION,
			   desc_.desc);
	alarm_db_->Extract(ALARM_DATABASE, small_rec, START_TIME, c_value);
	desc_.start_time = strtol(c_value, NULL, 10);
	alarm_db_->Extract(ALARM_DATABASE, small_rec, END_TIME, c_value);
	desc_.end_time = strtol(c_value, NULL, 10);

	return l_value;
    }
}

void
Alarm_d::SetId()
{

    int temp_rec = 0;
    char buf[16];
    int num_recs = alarm_db_->NumRecs(ALARM_DATABASE);
    int idx = 0;

    id_ = 0;
    if (num_recs == MAX_RECS) {
	id_ = -1;
    } else {
	for (idx = 0; idx < num_recs; idx++) {
	    alarm_db_->Extract(ALARM_DATABASE, idx, RECNO, buf);
	    temp_rec = atoi(buf);
	    if (temp_rec > id_)
		id_ = temp_rec;
	}
    }
    if (INT_MAX == id_) {
	int rec_array[MAX_RECS];

	for (idx = 0; idx < MAX_RECS; idx++)
	    rec_array[idx] = -1;

	alarm_db_->Select(ALARM_DATABASE, rec_array, MAX_RECS);

	id_ = 0;
	for (idx = 0; idx < MAX_RECS; idx++) {
	    if (-1 == rec_array[idx])
		continue;

	    char record[MAXRECSIZ];

	    // try to reset the id of the recno starting at zero    
	    alarm_db_->Extract(ALARM_DATABASE, rec_array[idx], record);
	    put16(&record[cFields[RECNO].offset], id_);
	    alarm_db_->Edit(ALARM_DATABASE, rec_array[idx], record);
	    id_++;
	}
	if (INT_MAX == id_)
	    id_ = -1;
    }
    if (-1 == id_)
	error_ = ERROR_MAX_DB;

}

void
Alarm_d::MarkEntry(int flag)
{

    int rec_array[1];
    int status;
    char c_value[8];
    char buf[MAXRECSIZ];
    int flag_;

    sprintf(buf, "%d", desc_.alarm_recno);
    alarm_db_->Select(ALARM_DATABASE, buf, RECNO, rec_array, 1);
    alarm_db_->Extract(ALARM_DATABASE, rec_array[0], STATUS, c_value);
    status = atoi(c_value);

    flag_ = status | flag;

    alarm_db_->Extract(ALARM_DATABASE, rec_array[0], buf);
    put16(&buf[cFields[STATUS].offset], flag_);
    alarm_db_->Edit(ALARM_DATABASE, rec_array[0], buf);

}

// Remove alarm if it is already set in the database
void
Alarm_d::CheckAlarm(char *service, int recno)
{
    int rec_array[MAX_RECS];
    int idx = 0;
    char c_value[16];

    for (idx = 0; idx < MAX_RECS; idx++)
	rec_array[idx] = -1;

    alarm_db_->Select(ALARM_DATABASE, service, NAME, rec_array, MAX_RECS);
    for (idx = 0; idx < MAX_RECS; idx++) {
	if (-1 == rec_array[idx])
	    continue;
	alarm_db_->Extract(ALARM_DATABASE, rec_array[idx], NAME_RECNO,
			   c_value);
	if (recno == atoi(c_value))
	    alarm_db_->DeleteRec(ALARM_DATABASE, rec_array[idx]);
    }

}

void
Alarm_d::RemoveEntry()
{
    int rec_array[1];
    char buf[16];

    sprintf(buf, "%d", desc_.alarm_recno);
    alarm_db_->Select(ALARM_DATABASE, buf, RECNO, rec_array, 1);
    alarm_db_->DeleteRec(ALARM_DATABASE, rec_array[0]);
    if (ERROR_MAX_DB & error_)
	error_ = error_ ^ ERROR_MAX_DB;
}

void
Alarm_d::ParseMsg(char *msg)
{

    DPRINT("Parsing msg: [%s]\n", msg);
    char *service = new char[MAX_LEN];
    char *msg_cmd = new char[MAX_LEN];
    char *app_name = new char[MAX_LEN];
    char buf[MAXRECSIZ];
    int err = 0;

    char *tmp = strtok(msg, TOKEN);
    strcpy(service, tmp);

    tmp = strtok(NULL, TOKEN);
    if (NULL == tmp)
	return;
    strcpy(msg_cmd, tmp);

    if (strcmp(msg_cmd, "SET") == 0) {
	char *c_recno = new char[16];
	char *desc = new char[DESC];
	char *c_start_time = new char[16];
	char *c_end_time = new char[16];
	char *c_alarm_time = new char[16];
	int recno;
	long start_time = 0;
	long end_time = 0;
	long alarm_time = 0;

	tmp = strtok(NULL, TOKEN);
	strcpy(c_recno, tmp);
	recno = strtol(c_recno, (char **) NULL, 10);
	if (ERANGE == errno)
	    err = -1;

	tmp = strtok(NULL, TOKEN);
	if (NULL == tmp)
	    err = -1;
	else
	    strcpy(desc, tmp);

	tmp = strtok(NULL, TOKEN);
	if (NULL == tmp)
	    err = -1;
	else {
	    strcpy(c_start_time, tmp);
	    start_time = strtol(c_start_time, NULL, 10);
	    if (ERANGE == errno)
		err = -1;
	}

	tmp = strtok(NULL, TOKEN);
	if (NULL == tmp)
	    err = -1;
	else {
	    strcpy(c_end_time, tmp);
	    end_time = strtol(c_end_time, NULL, 10);
	    if (ERANGE == errno)
		err = -1;
	}

	tmp = strtok(NULL, TOKEN);
	if (NULL == tmp)
	    err = -1;
	else {
	    strcpy(c_alarm_time, tmp);
	    alarm_time = strtol(c_alarm_time, NULL, 10);
	    if (ERANGE == errno)
		err = -1;
	}

	if (-1 == id_) {	// no room left in db can register alarm
	    err = -1;
	    SendErrorMsg(service);
	}
	if (0 == err) {
	    id_++;
	    memset(buf, 0, MAXRECSIZ);
	    put16(&buf[cFields[RECNO].offset], id_);
	    strcpy(&buf[cFields[NAME].offset], service);
	    put16(&buf[cFields[NAME_RECNO].offset], recno);
	    strcpy(&buf[cFields[DESCRIPTION].offset], desc);
	    put32(&buf[cFields[START_TIME].offset], start_time);
	    put32(&buf[cFields[END_TIME].offset], end_time);
	    put32(&buf[cFields[ALARM_TIME].offset], alarm_time);
	    put16(&buf[cFields[STATUS].offset], 0);

	    CheckAlarm(service, recno);
	    alarm_db_->Insert(ALARM_DATABASE, buf);
	}

	delete[]c_recno;
	delete[]desc;
	delete[]c_start_time;
	delete[]c_end_time;
	delete[]c_alarm_time;
	c_recno = desc = c_start_time = c_end_time = c_alarm_time = 0;

    }

    if (strcmp(msg_cmd, "DELETE") == 0) {
	DPRINT("recievced DELETE\n");
	char *c_recno = new char[16];
	int recno = 0;

	tmp = strtok(NULL, TOKEN);
	if (NULL == tmp)
	    err = -1;
	else {
	    strcpy(c_recno, tmp);
	    recno = strtol(c_recno, (char **) NULL, 10);
	    if (ERANGE == errno)
		err = -1;
	}

	DPRINT("err [%d]\n", err);
	if (0 == err) {
	    int rec_array[MAX_RECS];
	    int idx = 0;
	    int app_recno;
	    char c_app_recno[16];
	    char name[APP_NAME];

	    for (idx = 0; idx < MAX_RECS; idx++)
		rec_array[idx] = -1;

	    alarm_db_->Select(ALARM_DATABASE, rec_array, MAX_RECS);

	    for (idx = 0; idx < MAX_RECS; idx++) {
		if (-1 == rec_array[idx])
		    continue;

		alarm_db_->Extract(ALARM_DATABASE, rec_array[idx], NAME,
				   name);
		DPRINT("service [%s] name [%s]\n", service, name);
		if (0 == strcmp(name, service)) {
		    alarm_db_->Extract(ALARM_DATABASE, rec_array[idx],
				       NAME_RECNO, c_app_recno);
		    app_recno = strtol(c_app_recno, (char **) NULL, 10);
		    DPRINT("app_recno [%d]\n", app_recno);
		    if (ERANGE == errno)
			err = -1;
		    if ((0 == err) && (app_recno == recno)) {
			DPRINT("DeleteRec\n");
			alarm_db_->DeleteRec(ALARM_DATABASE, rec_array[idx]);
		    }
		}
	    }
	}
	delete[]c_recno;
	c_recno = 0;
    }
    delete[]service;
    delete[]msg_cmd;
    delete[]app_name;
    service = msg_cmd = app_name = 0;
}

int
Alarm_d::ReadMsg()
{
    unsigned short ret;
    int err;
    char msg[MAX_LEN];
    int len = MAX_LEN;

    status_ = STATUS_READ;
    err = ClGetMessage(msg, &len, &ret);
    status_ = STATUS_CLEAR;
    if (0 > err)
	return -1;

    ParseMsg(msg);
    return 0;
}

void
Alarm_d::SendErrorMsg(char *service)
{

    int fd = ClFindApp((unsigned char *) (service));
    char msg[255];
    int len = sizeof(msg);

    sprintf(msg, "%s^INITIATE^0", ALARM_D);
    status_ = STATUS_WRITE_ERR;
    ClSendMessage(fd, msg, len);
    status_ = STATUS_CLEAR;
    sprintf(msg, "%s^ERROR^%d", ALARM_D, error_);
    status_ = STATUS_WRITE_ERR;
    ClSendMessage(fd, msg, len);
    status_ = STATUS_CLEAR;

}

int
Alarm_d::SendMsg()
{
    int err = 0;
    int flags;
    int len = sizeof(desc_.msg);
    char msg[255];

    desc_.fd = ClFindApp((unsigned char *) (desc_.appname));

    DPRINT("desc_.fd [%d]\n", desc_.fd);
    if (0 >= desc_.fd) {

	flags = 0;
	DPRINT("Trying to start [%s]\n", desc_.appname);
	desc_.fd = ClStartApp((unsigned char *) (desc_.appname),
			      (unsigned char *) NULL, flags, 5);
	if (0 >= desc_.fd) {
	    DPRINT("Unable to start [%s]\n", desc_.appname);
	    MarkEntry(ERROR_START);
	    return -1;
	}
    }
#ifdef CONFIG_NANOX
    // force the app to raise
    DPRINT("Show [%s]\n", desc_.appname);
    scrtopShowApp(desc_.appname);
#endif

    sprintf(msg, "%s^INITIATE^0", ALARM_D);

    status_ = STATUS_WRITE_MSG;
    ClSendMessage(desc_.fd, msg, sizeof(msg));
    status_ = STATUS_CLEAR;

    sprintf(desc_.msg, "%s^ALARM^%d^%s^%ld^%ld",
	    ALARM_D, desc_.recno, desc_.desc, desc_.start_time,
	    desc_.end_time);
    DPRINT("sending msg [%s]\n", desc_.msg);
    status_ = STATUS_WRITE_MSG;
    err = ClSendMessage(desc_.fd, desc_.msg, len);
    status_ = STATUS_CLEAR;
    if (0 > err) {
	DPRINT("Unable to send message\n");
	MarkEntry(ERROR_SEND);
	return -1;
    }
    RemoveEntry();
    return 0;

}

void
Alarm_d::OpenAlarmDatabase()
{
    if (!alarm_db_->Open(ALARM_DATABASE, &cFile, cFields, ALARM_INDEX)) {
	if (alarm_db_->Create(ALARM_DATABASE, &cFile, cFields, ALARM_INDEX)) {
	    if (!alarm_db_->
		Open(ALARM_DATABASE, &cFile, cFields, ALARM_INDEX)) {
		exit(-1);
	    }
	} else
	    exit(-1);
    }
}

Alarm_d::~Alarm_d()
{
    close(rtcfd_);
    close(fd_);
    CloseDB();
    delete alarm_db_;
    alarm_db_ = 0;
    unlink(path);
}
