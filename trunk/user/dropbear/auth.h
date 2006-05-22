/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef _AUTH_H_
#define _AUTH_H_

#include "includes.h"

void svr_authinitialise();
void cli_authinitialise();

/* Server functions */
void recv_msg_userauth_request();
void send_msg_userauth_failure(int partial, int incrfail);
void send_msg_userauth_success();
void svr_auth_password();
void svr_auth_pubkey();
void svr_auth_pam();

/* Client functions */
void recv_msg_userauth_failure();
void recv_msg_userauth_success();
void recv_msg_userauth_specific_60();
void recv_msg_userauth_pk_ok();
void recv_msg_userauth_info_request();
void cli_get_user();
void cli_auth_getmethods();
void cli_auth_try();
void recv_msg_userauth_banner();
void cli_pubkeyfail();
void cli_auth_password();
int cli_auth_pubkey();
void cli_auth_interactive();
char* getpass_or_cancel();


#define MAX_USERNAME_LEN 25 /* arbitrary for the moment */

#define AUTH_TYPE_NONE      1
#define AUTH_TYPE_PUBKEY    1 << 1
#define AUTH_TYPE_PASSWORD  1 << 2
#define AUTH_TYPE_INTERACT  1 << 3

#define AUTH_METHOD_NONE "none"
#define AUTH_METHOD_NONE_LEN 4
#define AUTH_METHOD_PUBKEY "publickey"
#define AUTH_METHOD_PUBKEY_LEN 9
#define AUTH_METHOD_PASSWORD "password"
#define AUTH_METHOD_PASSWORD_LEN 8
#define AUTH_METHOD_INTERACT "keyboard-interactive"
#define AUTH_METHOD_INTERACT_LEN 20



/* This structure is shared between server and client - it contains
 * relatively little extraneous bits when used for the client rather than the
 * server */
struct AuthState {

	char *username; /* This is the username the client presents to check. It
					   is updated each run through, used for auth checking */
	unsigned char authtypes; /* Flags indicating which auth types are still 
								valid */
	unsigned int failcount; /* Number of (failed) authentication attempts.*/
	unsigned authdone : 1; /* 0 if we haven't authed, 1 if we have. Applies for
							  client and server (though has differing [obvious]
							  meanings). */
	unsigned perm_warn : 1; /* Server only, set if bad permissions on 
							   ~/.ssh/authorized_keys have already been
							   logged. */

	/* These are only used for the server */
	char *printableuser; /* stripped of control chars, used for logs etc */
	struct passwd * pw;

};

struct SignKeyList;
/* A singly linked list of signing keys */
struct SignKeyList {

	sign_key *key;
	int type; /* The type of key */
	struct SignKeyList *next;
	/* filename? or the buffer? for encrypted keys, so we can later get
	 * the private key portion */

};

#endif /* _AUTH_H_ */
