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


#include <stdio.h>
#include <string.h>
#include <time.h>

#include "nxmail.h"
#include "callbacks.h"
#include "ui.h"
#include "mailengine.h"
#include "settings.h"

#include "actions.h"

static void
handle_mail_error(MAILERROR error)
{

    NxMail *inst = NxMail::Inst();

    char errorMsg[100];


    switch (error) {
    case MAIL_OPEN_ERROR:
	sprintf(errorMsg, "Couldn't talk to server %s!",
		inst->EngineGetSettings("server"));
	break;
    case MAIL_AUTH_ERROR:
	sprintf(errorMsg, "Couldn't authorize %s!",
		inst->EngineGetSettings("username"));
	break;

    case MAIL_SEND_ERROR:
	sprintf(errorMsg, "Couldn't send a message to %s",
		inst->EngineGetSettings("smtpserver"));
	break;
    default:
	return;
    }

    inst->MainSetStatus(errorMsg);

}

static int
get_message_number(Fl_Browser * mlist)
{
    int list = mlist->value();
    return ((int) mlist->data(list));
}

/* Various callbacks for the different windows */

void
new_message_callback()
{
    /* Just fire the editor */
    NxMail *inst = NxMail::Inst();
    inst->EditorClearFields();
    inst->EditorShowWindow();
}

void
reply_message_callback(ReplyType type)
{
    reply_to_message(type);
}

void
delete_message_callback()
{
    NxMail *inst = NxMail::Inst();

    char outmsg[100];

    MAILERROR r;

    sprintf(outmsg, "Deleting message from %s...",
	    inst->EngineGetSettings("server"));
    inst->MainSetStatus(outmsg);

    r = inst->EngineOpenSession();

    if (r != MAIL_SUCCESS) {
	handle_mail_error(r);
	return;
    }

    int msg = get_message_number(inst->MainGetMList());
    delete_message(msg);

    inst->MainSetStatus("");
    inst->EngineCloseSession();

    inst->MainShowWindow();
}

void
check_mail_callback()
{

    NxMail *inst = NxMail::Inst();

    char outmsg[100];

    MAILERROR r;

    sprintf(outmsg, "Getting mail from %s...",
	    inst->EngineGetSettings("server"));

    inst->MainSetStatus(outmsg);

    r = inst->EngineOpenSession();

    if (r != MAIL_SUCCESS) {
	handle_mail_error(r);
	return;
    }

    get_message_list();
    inst->EngineCloseSession();
}

void
do_settings_callback()
{

    NxMail *inst = NxMail::Inst();

    inst->SettingsUpdateFields();
    inst->SettingsShowWindow();

}

static void
build_addrlist(char *value, nxmail_addr_t * alist)
{
    int count = nxmail_count_addrstr(value);

    if (!count)
	return;

    count = nxmail_alloc_addrlist(alist, count);
    nxmail_build_addrlist(value, alist, count);
}

void
send_message_callback()
{

    NxMail *inst = NxMail::Inst();

    MAILERROR r;

    char outmsg[100];
    char addrstr[100];

    nxmail_header_t header;

    bzero(&header, sizeof(header));

    /* Get the to and cc lists */
    build_addrlist((char *) inst->EditorGetTo(), &header.to);
    build_addrlist((char *) inst->EditorGetCC(), &header.cc);

    /* Do the subject and the date */
    strcpy(header.subject, inst->EditorGetSubject());
    nxmail_parse_dateval(time(0), &header.date);

    bzero(addrstr, 100);

    /* Finally, construct the from address using the mail settings */
    sprintf(addrstr, "%s@%s", inst->EngineGetSettings("smtpname"),
	    inst->EngineGetSettings("smtpserver"));

    printf("The message is from %s <%d>\n", addrstr, strlen(addrstr));

    rfc822_parse_address(&header.from, addrstr);

    /* Ok, so we have the header, and we have the body. */
    /* lets light this candle */

    sprintf(outmsg, "Sending message to %s...",
	    inst->EngineGetSettings("smtpserver"));

    inst->MainSetStatus(outmsg);
    r = inst->EngineSendMessage(inst->EngineGetSettings("smtpserver"), 25,
				&header, (char *) inst->EditorGetMsg(),
				inst->EditorGetMsgSize());

    if (r != MAIL_SUCCESS) {
	handle_mail_error(r);
	//    return;
    } else
	inst->MainSetStatus("Message sent!");

    inst->MainShowWindow();

}

void
get_message_callback()
{

    if (Fl::event_clicks()) {
	NxMail *inst = NxMail::Inst();

	char messagestr[50];

	int msg = get_message_number(inst->MainGetMList());

	MAILERROR r;

	sprintf(messagestr, "Getting message %d...", msg);

	inst->MainSetStatus(messagestr);

	r = inst->EngineOpenSession();

	if (r != MAIL_SUCCESS) {
	    handle_mail_error(r);
	    return;
	}

	/* Get the current message # */

	if (msg <= 0 || msg == 0xFFFF) {
	    if (msg == 0)
		inst->MainSetStatus("");
	    if (msg == 0xFFFF)
		inst->MainSetStatus("Message has been deleted!");
	} else {
	    get_message(msg, 1);
	    inst->MainSetStatus("");
	}

	inst->EngineCloseSession();
    }

    Fl::event_clicks(0);
}

void
save_message_callback()
{

    NxMail *inst = NxMail::Inst();

    Fl_Browser *mimelist = inst->ViewerGetMimeWidget();

    int list = mimelist->value();
    int sect = (int) mimelist->data(list);

    save_section_to_file(sect, "blah");

    inst->MainShowWindow();
}

void
view_message_callback()
{

    NxMail *inst = NxMail::Inst();

    Fl_Browser *mimelist = inst->ViewerGetMimeWidget();

    int list = mimelist->value();
    int sect = (int) mimelist->data(list);

    printf("Calling do mime_viewer with %d\n", sect);

    do_mime_viewer(sect);
}
