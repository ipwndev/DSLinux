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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nxmail.h"
#include "callbacks.h"
#include "ui.h"
#include "mailengine.h"
#include "settings.h"
#include "mime.h"

/* LOCAL VALUES AND DEFINES */

#define BUFFER_SIZE 2048	/* 2K of message */
#define MAX_WIDTH 26

static struct
{
    nxmail_header_t *header;
    nxmail_body_t *body;
}
current_message;
static int msgCount = 0;

char *listBuffer = 0;
static char mimeBuffer[100][10];

static void
mail_concat_subject(char *in, char *out, int size)
{
    char *sptr;

    strncpy(out, in, size);
    out[(size / 2)] = 0;

    strcat(out, "...");
    sptr = in + strlen(in) - (size / 2);
    strcat(out, sptr);
}

static void
mail_build_fields(nxmail_header_t * header, char *output)
{
    char fromstr[100];

    sprintf(output, "Subject: %s\n    From: %s\n       To: %s\n\n",
	    header->subject, fromstr, header->to.name);
}

static void
struct_to_addrstr(nxmail_address_t * addr, char *output)
{
    char addrstr[100];

    if (strlen(addr->host))
	sprintf(addrstr, "%s@%s", addr->mailbox, addr->host);
    else
	strcpy(addrstr, addr->mailbox);


    if (strlen(addr->name))
	sprintf(output, "%s <%s>", addr->name, addrstr);
    else
	strcpy(output, addrstr);
}

static void
mail_build_address(nxmail_address_t * addr, char *output)
{
    nxmail_address_t *ptr = addr;

    while (ptr) {
	char addrstr[100];

	if (!strlen(ptr->mailbox))
	    break;

	struct_to_addrstr(ptr, addrstr);

	strcat(output, addrstr);

	if (ptr->next)
	    strcat(output, ",\n");
	ptr = ptr->next;
    }
}

static nxmail_body_t *
get_body_section(int section, nxmail_body_t * head)
{
    nxmail_body_t *ptr = head;

    for (int i = 1; i < section; i++) {
	if (!ptr)
	    return (0);
	ptr = ptr->next;
    }

    return (ptr);
}

static void
mview_insert_field(Fl_Multiline_Output * mview, char *fieldname, char *value)
{
    char addstr[1024];

    bzero(addstr, 1024);

    sprintf(addstr, "%s ", fieldname);

    strncat(addstr, value, 1022 - strlen(addstr));
    strcat(addstr, "\n");

    mview->insert(addstr);
}

void
get_message_list(void)
{

    NxMail *inst = NxMail::Inst();

    char messagestr[50];

    int m = 0;
    int message_count = inst->EngineGetMsgCount();
    Fl_Browser *mlist = inst->MainGetMList();


    //if (listBuffer) free(listBuffer);

    //listBuffer = (char *) calloc(message_count * 50, 1);

    /* Clear the list */
    mlist->clear();

    msgCount = 0;

    //char *listptr = listBuffer;

    for (m = 1; m <= message_count; m++) {
	char listptr[50];

	char outsub[MAX_WIDTH + 5];

	nxmail_header_t *header = inst->EngineFetchHeader(m);

	if (!header)
	    continue;

	if (strlen(header->subject) > MAX_WIDTH)
	    mail_concat_subject(header->subject, outsub, MAX_WIDTH);
	else
	    strcpy(outsub, header->subject);

	msgCount++;

	sprintf(listptr, "%2.2d/%2.2d %2.2d:%2.2d\t%s",
		header->date.month + 1, header->date.day,
		header->date.hour, header->date.min, outsub);

	if (strlen(listptr) > 49)
	    listptr[49] = 0;

	mlist->add((const char *) listptr, (void *) m);
    }

    if (message_count == 0)
	sprintf(messagestr, "No messages on server!");
    else
	sprintf(messagestr, "Got %d messages!", message_count);

    inst->MainSetStatus(messagestr);
}

static void
show_message_header(Fl_Multiline_Output * mview, nxmail_header_t * header)
{
    char fromstr[1024];

    mview_insert_field(mview, "Subject: ", header->subject);

    fromstr[0] = 0;
    mail_build_address(&header->from, fromstr);
    mview_insert_field(mview, "    From: ", fromstr);

    fromstr[0] = 0;
    mail_build_address(&header->to, fromstr);
    mview_insert_field(mview, "      To: ", fromstr);

    fromstr[0] = 0;
    mail_build_address(&header->cc, fromstr);

    if (strlen(fromstr))
	mview_insert_field(mview, "      CC: ", fromstr);
}

static void
fill_mime_browser(Fl_Browser * mbrow, nxmail_body_t * body, int cursection)
{
    nxmail_body_t *bodyptr = body;
    int count = 1;

    mbrow->clear();

    while (bodyptr) {
	if (!strlen(bodyptr->mimeheader.type))
	    sprintf(mimeBuffer[count], "%d: (%d bytes) Message", count,
		    bodyptr->size);
	else
	    sprintf(mimeBuffer[count], "%d: (%d bytes) %s", count,
		    bodyptr->size, bodyptr->mimeheader.type);

	if (strlen(bodyptr->mimeheader.description)) {
	    strcat(mimeBuffer[count], " :");
	    strcat(mimeBuffer[count], bodyptr->mimeheader.description);
	}

	mbrow->add(mimeBuffer[count], (void *) count);
	bodyptr = bodyptr->next;
	count++;
    }

    mbrow->select(cursection);
}

void
delete_message(int msgno)
{
    NxMail *inst = NxMail::Inst();

    Fl_Browser *mlist = inst->MainGetMList();

    int item = mlist->value();

    inst->MainShowWindow();

    if (inst->EngineDeleteMsg(msgno) == NXMAIL_OK) {
	char newline[128];
	sprintf(newline, "@C1@.%s", mlist->text(item));

	mlist->text(item, newline);
	mlist->data(item, (void *) 0xFFFF);

	// Need to update the list rest of the list and subtract 1 from the message id
	int tcount = mlist->size();
	for (int i = item + 1; i <= tcount; i++) {
	    int mno = (int) mlist->data(i);
	    if (mno <= mlist->size()) {
		mno -= 1;
		mlist->data(i, (void *) mno);
	    }			// end of if
	}			// end of for 

	inst->MainSetStatus("Message deleted.");
	return;
    }

    inst->MainSetStatus("Error!");
}

void
get_message(int msgno, int section)
{

    NxMail *inst = NxMail::Inst();
    MIMESUPPORT supported;

    Fl_Browser *mimelist = inst->ViewerGetMimeWidget();
    Fl_Multiline_Output *mview = inst->ViewerGetMView();

    /* Get the header for the current message */
    nxmail_header_t *remotehdr = inst->EngineFetchHeader(msgno);

    /* Store the pointer locally */
    current_message.header = remotehdr;

    /* Save it locally for replies and such */
    mview->value("");

    if (section == 1)
	show_message_header(mview, remotehdr);

    /* Get the linked list of sections */

    nxmail_body_t *body = inst->EngineFetchMsg(msgno);
    if (!body) {
	current_message.header = 0;
	return;
    }

    current_message.body = body;

    nxmail_body_t *bodypointer = get_body_section(section, body);

    if (!bodypointer) {
	bodypointer = body;
	section = 1;
    }

    if (!strlen(bodypointer->mimeheader.type))
	supported = check_mime_support(remotehdr->mimeheader.type);
    else
	supported = check_mime_support(bodypointer->mimeheader.type);

    switch (supported) {
    case MIME_NOT_SUPPORTED:
    case MIME_SAVE:
	mview->
	    insert
	    ("Sorry!\nThis section of the message is of the\n unsupported type\n");
	mview->insert(bodypointer->mimeheader.type);
	mview->insert("\n");
	mview->position(1);
	break;

    case MIME_TEXT:
	mview->insert("\n");

	mview->insert(bodypointer->text, bodypointer->size);
	mview->position(1);
	break;

    default:
	break;
    }

    fill_mime_browser(mimelist, body, section);

#ifdef NOTUSED

    if (body->next) {

	inst->ViewerShowMimeWidget();
	/* Fill the mime browser */
	fill_mime_browser(mimelist, body, section);
    } else {
	inst->ViewerHideMimeWidget();
    }

#endif

    /* Finally, show the message window */
    inst->ViewerShowWindow();

}

void
reply_to_message(ReplyType type)
{

    NxMail *inst = NxMail::Inst();

    char buffer[2048];

    Fl_Browser *mlist = inst->MainGetMList();

    /* Get the current message */
    int msgno = (int) mlist->data(mlist->value());

    nxmail_header_t *header = inst->EngineFetchHeader(msgno);
    nxmail_body_t *body = inst->EngineFetchMsg(msgno);

    char substr[256];
    char tostr[256];
    char ccstr[256];

    tostr[0] = 0;
    ccstr[0] = 0;

    if (!header && !body)
	return;

    /* Build the address lists if needed */

    if (type == REPLY_SINGLE) {
	mail_build_address(&header->from, tostr);
    }

    if (type == REPLY_ALL) {
	mail_build_address(&header->from, tostr);
	mail_build_address(&header->to, ccstr);
	mail_build_address(&header->cc, ccstr);
    }

    /* Next, build the subject */

    switch (type) {
    case REPLY_ALL:
    case REPLY_SINGLE:
	sprintf(substr, "Re: %s", header->subject);
	break;
    case REPLY_FORWARD:
	sprintf(substr, "Fwd: %s", header->subject);
	break;
    }

    inst->EditorSetFields(tostr, "", substr);

    if (body->size > 2048)
	strncpy(buffer, body->text, 2048);
    else
	strncpy(buffer, body->text, body->size);

    char *textptr = buffer;

    while ((int) (textptr - buffer) < body->size) {
	char linestr[1024];

	bzero(linestr, 1024);
	sscanf(textptr, "%1023[^\n]\n", linestr);

	inst->EditorIndentText(linestr);

	textptr += strlen(linestr);
	textptr++;
    }

    inst->EditorShowWindow();
}

#ifdef NOTUSED

static int
save_section_to_fd(int section, int fd)
{
    nxmail_body_t *sect = get_body_section(section, current_message.body);

    if (sect->mimeheader.encoding == NXMAIL_ENCODING_BASE64) {
	char *outbuffer = (char *) malloc(sect->size);

	if (!outbuffer)
	    return (-1);

	int elen = decode_base64(sect->text, outbuffer, sect->size);

	write(fd, outbuffer, elen);
	free(outbuffer);
    } else {
	write(fd, sect->text, sect->size);
    }

    return 0;
}

#endif

int
save_section_to_file(int section, char *filename)
{
#ifdef NOTUSED
    /* Get the appropriate section for viewing */
    nxmail_body_t *sect = get_body_section(section, current_message.body);

    if (!sect)
	return (-1);

    FILE *fstream = open(filename, F_WRONLY, O_CREAT);
    if (!fstream)
	return (-1);

    if (sect->mimeheader.encoding == NXMAIL_ENCODING_BASE64) {
	char *outbuffer = (char *) malloc(sect->size);

	if (!outbuffer) {
	    fclose(fstream);
	    return;
	}

	int elen = decode_base64(sect->text, outbuffer, sect->size);

	fwrite(outbuffer, elen, 1, fstream);
	free(outbuffer);
    } else {
	fwrite(sect->text, 1, sect->size, fstream);
    }

    fclose(fstream);
#endif

    return 0;
}

void
do_mime_viewer(int section)
{
}
