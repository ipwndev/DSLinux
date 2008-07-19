
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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_TRANSFERS 20

#define STATE_NEGOTIATE 1
#define STATE_CONNECT   2
#define STATE_DOWNLOAD  3
#define STATE_UPLOAD    4
#define STATE_COMPLETE  5
#define STATE_CANCEL    6
#define STATE_ERROR     7

#define DWORD unsigned int
#define QWORD unsigned long long

struct p2p_transfer {
    char *from, *ip;
    int sd, state;
    QWORD size, offset;
};

struct p2p_header {
    DWORD sid;
    DWORD b_ident;
    QWORD offset;
    QWORD tot_len;
    DWORD msg_len;
    DWORD flag;
    DWORD ack_ident;
    DWORD ack_uid;
    QWORD ack_tot_len;
};

#define INVITE 1
#define BYE    2

struct msg_header {
    char *to, *from, *cid, *ctype;
    int method, cseq, max_forward, clen;
};
