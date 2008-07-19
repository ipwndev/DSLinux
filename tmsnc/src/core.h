
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
 * this program; if not, write to sanoix@gmail.com
 */

#define NAME_LEN   128
#define PSM_LEN    128
#define CMEDIA_LEN 128
#define ADDR_LEN   64
#define GUID_LEN   64

#define FL 1
#define AL 2
#define BL 4
#define RL 8
#define PL 16

#define PRODUCT_KEY   "CFHUR$52U_{VIX5T"
#define PRODUCT_ID    "PROD0101{0RM?UBW"

typedef struct _MSN_contact {
    char addr[ADDR_LEN];
    char name[NAME_LEN];
    char custom_name[NAME_LEN];
    char cmedia[CMEDIA_LEN];
    char psm[PSM_LEN];
    char guid[GUID_LEN];
    int ingroup;	//its first group, used by sort, added gfhuang
    int status;
    int listnum;
} MSN_contact;

typedef struct _MSN_msgformat {
    char font[15];
    char color[8];
    char effect;
} MSN_msgformat;

typedef struct _MSN_group {		//added by gfhuang
    char name[NAME_LEN];
    char guid[GUID_LEN];
} MSN_group;

typedef struct _MSN_session {
    int sd;
    char gtc;
    char blp;
    int num_contacts;
    int csc;
    MSN_contact me;
    MSN_contact **contact;
    MSN_msgformat format;

    int num_groups;		//added by gfhuang
    MSN_group **group;		//added by gfhuang
} MSN_session;

/* net.c */
int MSN_conversation_initiate(MSN_session *, char *, void (*cb) (int, void *));
int MSN_conversation_sendmsg(int, int *, char *, MSN_session *);
int MSN_conversation_handle(int, int *, char *, int, MSN_session *);
int MSN_conversation_call(int, int *, char *);
int MSN_list2int(char *);
int MSN_block_contact(char *, MSN_session *, char *, int);
int MSN_unblock_contact(char *, MSN_session *, char *, int);
int MSN_add_contact(char *, MSN_session *, char, char *, int);
int MSN_remove_contact(MSN_contact *, MSN_session *, char *, int);
void MSN_change_nick(char *, MSN_session *);
int MSN_server_handle(MSN_session *, char *, int);
char *MSN_status2str(int);
int MSN_status2int(char *);
int MSN_load_userlist(MSN_session *, void (*cb) (int, void *));
void MSN_logout(int);
int MSN_set_status(MSN_session *, char *);
int MSN_init_session(char *, int, MSN_session *, char *,
                     void (*cb) (int, void *));
int tcp_connect(char *, int);
void MSN_send_uux(MSN_session *);
int MSN_set_personal_message(char *, MSN_session *);
int MSN_set_current_media(char *, MSN_session *);

/* misc.c */
void MSN_init(MSN_session *);
int MSN_is_valid_address(char *);
char *MSN_xml_encode(char *, char *, int);
char *MSN_url_encode(char *, char *, int);
char *MSN_url_decode(char *, char *, int);
void MSN_error2str(int, char *, int);
MSN_contact **MSN_resize_contact_array(MSN_session *);
MSN_contact *MSN_allocate_contact(void);
MSN_group **MSN_resize_group_array(MSN_session *);	//added by gfhuang
MSN_group *MSN_allocate_group(void);			//added by gfhuang
void MSN_free_contact(MSN_contact *);
void MSN_handle_chl(char *, char *);
int isBigEndian(void);
unsigned int swapInt(unsigned int);
unsigned long long swapLongLong(unsigned long long);
int wait_for_input(int, unsigned int);
char *split(char *, char, int);
int getline(char *, int, int);

/* ssl.c */
int https_auth(char *, int, char *, char *, char *, char *, char *, int,
               void (*cb) (int, void *));

/* translate.c */
int convert_from_utf8(char *, size_t, char *, size_t);
int convert_to_utf8(char *, size_t, char *, size_t);

/* debug.c */
int debug_init(void);
int debug_log(char *, ...);
void debug_destroy(void);

/* p2p.c */
int MSN_handle_p2p_msg(int);
