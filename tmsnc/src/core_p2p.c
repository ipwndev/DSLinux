
/*
 * TMSNC - Textbased MSN Client Copyright (C) 2004 The IR Developer Group
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the IR Public Domain License as published by the IR Group;
 * either version 1.6 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the IR Public Domain License along with
 * this program; if not, write to sanoix@gmail.com.
 */

#include "core_p2p.h"
#include "config.h"
#include "core.h"

struct p2p_transfer transfer[MAX_TRANSFERS];
int number_of_transfers;

void
MSN_free_transfer(num)
     int num;
{
    int i;
    
    if (transfer[num].from != NULL)
        free(transfer[num].from);
        
    if (transfer[num].ip != NULL)
        free(transfer[num].ip);

    close(transfer[num].sd);
    transfer[num].state = 0;
    
    for (i=0; i < (number_of_transfers - num); i++) {
        transfer[i] = transfer[i+1];
    }
}

void
MSN_free_all_tranfers(void)
{
    int i;
    
    for (i=0; i<number_of_transfers; i++) {
        if (transfer[i].from != NULL)
            free(transfer[i].from);
        
        if (transfer[i].ip != NULL)
            free(transfer[i].ip);

        close(transfer[i].sd);
        transfer[i].state = 0;
    }
}

void
swapP2PHeader(head)
     struct p2p_header *head;
{
    head->sid = swapInt(head->sid);
    head->b_ident = swapInt(head->b_ident);
    head->offset = swapLongLong(head->offset);
    head->tot_len = swapLongLong(head->tot_len);
    head->msg_len = swapInt(head->msg_len);
    head->flag = swapInt(head->flag);
    head->ack_ident = swapInt(head->ack_ident);
    head->ack_uid = swapInt(head->ack_uid);
    head->ack_tot_len = swapLongLong(head->ack_tot_len);
}

int
readMsgHeader(sd, msg_head)
     int sd;
     struct msg_header *msg_head;
{
    char buf[1024], *ptr;

    msg_head->method = msg_head->cseq = msg_head->max_forward = msg_head->clen = 0;
    msg_head->to = msg_head->from = msg_head->cid = msg_head->ctype = NULL;

    while(1) {
        if (wait_for_input(sd, 2) < 1)
            return -1;
        if (getline(buf, sizeof(buf) - 1, sd) < 0)
            return -1;
        if (strcmp(buf, "\r\n") == 0)
            break;

        if ((ptr = strstr(buf, "\r\n")) != NULL)
            *ptr = '\0';

        if (strncmp(buf, "INVITE", 6) == 0)
            msg_head->method = INVITE;
        else if (strncmp(buf, "BYE", 3) == 0)
            msg_head->method = BYE;
        else if (strncmp(buf, "To: ", 4) == 0)
            msg_head->to = strdup(&buf[4]);
        else if (strncmp(buf, "From: ", 6) == 0)
            msg_head->from = strdup(&buf[6]);
        else if (strncmp(buf, "Call-ID: ", 9) == 0)
            msg_head->cid = strdup(&buf[9]);
        else if (strncmp(buf, "Content-Type: ", 14) == 0)
            msg_head->ctype = strdup(&buf[14]);
        else if (strncmp(buf, "CSeq: ", 6) == 0)
            msg_head->cseq = atoi(&buf[6]);
        else if (strncmp(buf, "Max-Forwards: ", 14) == 0)
            msg_head->max_forward = atoi(&buf[14]);
        else if (strncmp(buf, "Content-Length: ", 16) == 0)
            msg_head->clen = atoi(&buf[16]);
    }
    return 0;
}

void
freeMsgHeader(msg_head)
     struct msg_header *msg_head;
{
    if (msg_head->to != NULL)
        free(msg_head->to);
    if (msg_head->from != NULL)
        free(msg_head->from);
    if (msg_head->cid != NULL)
        free(msg_head->cid);
    if (msg_head->ctype != NULL)
        free(msg_head->ctype);
}

int
MSN_handle_p2p_msg(sd)
     int sd;
{
    char buf[1024];

    struct p2p_header head;
    struct msg_header msg_head;

    int len, sessionid;
    DWORD footer;

    while(1) {
        if (wait_for_input(sd, 2) < 1)
            return -1;
        if (getline(buf, sizeof(buf) - 1, sd) < 0)
            return -1;
        if (strcmp(buf, "\r\n") == 0)
            break;
    }

    if (read(sd, &head, sizeof(head)) < sizeof(head))
        return -1;

    if(isBigEndian())
        swapP2PHeader(&head);

#ifdef DEBUG
    debug_log("P2P_headers\nsid: %u\nb_ident: %u\noffset: %llu\n"
              "tot_len: %llu\nmsg_len: %u\nflag: %u\nack_ident: %u\n"
              "ack_uid: %u\nack_tot_len: %llu\n\n", head.sid, head.b_ident,
              head.offset, head.tot_len, head.msg_len, head.flag,
              head.ack_ident, head.ack_uid, head.ack_tot_len);
#endif

    if (readMsgHeader(sd, &msg_head) == -1)
        return -1;

#ifdef DEBUG
    debug_log("P2P_msg_headers\nmethod: %d\ncseq: %d\nmax_forward: %d\n"
              "clen: %d\nto: %s\nfrom: %s\ncid: %s\nctype: %s\n\n",
              msg_head.method, msg_head.cseq, msg_head.max_forward, msg_head.clen,
              msg_head.to, msg_head.from, msg_head.cid, msg_head.ctype);
#endif

    /*
     * Only process invitation commands
     */
    while(1) {
        if (wait_for_input(sd, 2) < 1)
            return -1;
        if (getline(buf, sizeof(buf) - 1, sd) < 0)
            return -1;
        if (strncmp(buf, "EUF-GUID: ", 10) == 0) {
            if(strcmp(&buf[10], "{5D3E02AB-6190-11D3-BBBB-00C04F795683}\r\n") != 0)
                return -1; 
        }
        else if (strncmp(buf, "AppID: ", 7) == 0) {
            if(strcmp(&buf[7], "2\r\n") != 0)
                return -1; 
        }
        else if (strncmp(buf, "SessionID: ", 11) == 0) {
            sessionid = atoi(&buf[11]);
        }
        else if (strcmp(buf, "\r\n") == 0) {
            break;
        }
    }
    if (read(sd, &footer, sizeof(DWORD)) < sizeof(DWORD))
        return -1;
    if (isBigEndian())
        footer = swapInt(footer);

#ifdef DEBUG
    debug_log("SessionID: %d\nFooter: %d\n\n", sessionid, footer);
#endif
    
    return 0;
}
