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

/* Process a pubkey auth request */

#include "includes.h"
#include "session.h"
#include "dbutil.h"
#include "buffer.h"
#include "signkey.h"
#include "auth.h"
#include "authpubkey.h"
#include "ssh.h"
#include "packet.h"
#include "algo.h"

#ifdef DROPBEAR_PUBKEY_AUTH

#define MIN_AUTHKEYS_LINE 10 /* "ssh-rsa AB" - short but doesn't matter */
#define INIT_AUTHKEYS_LINE 1200 /* initial length of a line in authkeys */
#define MAX_AUTHKEYS_LINE 4200 /* max length of a line in authkeys */
#define INCR_AUTHKEYS_LINE 1000 /* increment to length of a line in authkeys */

static int checkpubkey(unsigned char* algo, unsigned int algolen,
		unsigned char* keyblob, unsigned int keybloblen);
static int checkpubkeyperms();
static void send_msg_userauth_pk_ok(unsigned char* algo, unsigned int algolen,
		unsigned char* keyblob, unsigned int keybloblen);
static int checkfileperm(char * filename);
static int getauthline(buffer * line, FILE * authfile);

/* process a pubkey auth request, sending success or failure message as
 * appropriate */
void pubkeyauth() {

	unsigned char testkey; /* whether we're just checking if a key is usable */
	unsigned char* algo = NULL; /* pubkey algo */
	unsigned int algolen;
	unsigned char* keyblob = NULL;
	unsigned int keybloblen;
	buffer * signbuf = NULL;
	unsigned int sigoffset;
	sign_key * key = NULL;
	char* fp = NULL;
	int type = -1;

	TRACE(("enter pubkeyauth"));

	/* 0 indicates user just wants to check if key can be used, 1 is an
	 * actual attempt*/
	testkey = (buf_getbyte(ses.payload) == 0);

	algo = buf_getstring(ses.payload, &algolen);
	keybloblen = buf_getint(ses.payload);
	keyblob = buf_getptr(ses.payload, keybloblen);

	/* check if the key is valid */
	if (checkpubkey(algo, algolen, keyblob, keybloblen) == DROPBEAR_FAILURE) {
		send_msg_userauth_failure(0, 0);
		goto out;
	}

	/* let them know that the key is ok to use */
	if (testkey) {
		send_msg_userauth_pk_ok(algo, algolen, keyblob, keybloblen);
		goto out;
	}

	/* now we can actually verify the signature */
	
	/* get the key */
	key = new_sign_key();
	type = DROPBEAR_SIGNKEY_ANY;
	if (buf_get_pub_key(ses.payload, key, &type) == DROPBEAR_FAILURE) {
		send_msg_userauth_failure(0, 1);
		goto out;
	}

	/* create the data which has been signed - this a string containing
	 * session_id, concatenated with the payload packet up to the signature */
	signbuf = buf_new(ses.payload->pos + 4 + SHA1_HASH_SIZE);
	buf_putstring(signbuf, ses.session_id, SHA1_HASH_SIZE);
	sigoffset = ses.payload->pos;
	buf_setpos(ses.payload, 0);
	memcpy(buf_getwriteptr(signbuf, sigoffset),
			buf_getptr(ses.payload, sigoffset), sigoffset);
	buf_incrwritepos(signbuf, sigoffset);
	buf_setpos(ses.payload, sigoffset);

	buf_setpos(signbuf, 0);
	/* ... and finally verify the signature */
	fp = sign_key_fingerprint(key, type);
	if (buf_verify(ses.payload, key, buf_getptr(signbuf, signbuf->len),
				signbuf->len) == DROPBEAR_SUCCESS) {
		dropbear_log(LOG_NOTICE,
				"pubkey auth succeeded for '%s' with key %s",
				ses.authstate.printableuser, fp);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"pubkey auth bad signature for '%s' with key %s",
				ses.authstate.printableuser, fp);
		send_msg_userauth_failure(0, 1);
	}
	m_free(fp);

out:
	/* cleanup stuff */
	if (signbuf) {
		buf_free(signbuf);
	}
	if (algo) {
		m_free(algo);
	}
	if (key) {
		sign_key_free(key);
		key = NULL;
	}
	TRACE(("leave pubkeyauth"));
}

/* Reply that the key is valid for auth, this is sent when the user sends
 * a straight copy of their pubkey to test, to avoid having to perform
 * expensive signing operations with a worthless key */
static void send_msg_userauth_pk_ok(unsigned char* algo, unsigned int algolen,
		unsigned char* keyblob, unsigned int keybloblen) {

	TRACE(("enter send_msg_userauth_pk_ok"));
	CHECKCLEARTOWRITE();

	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_PK_OK);
	buf_putstring(ses.writepayload, algo, algolen);
	buf_putstring(ses.writepayload, keyblob, keybloblen);

	encrypt_packet();
	TRACE(("leave send_msg_userauth_pk_ok"));

}

/* Checks whether a specified publickey (and associated algorithm) is an
 * acceptable key for authentication */
/* Returns DROPBEAR_SUCCESS if key is ok for auth, DROPBEAR_FAILURE otherwise */
static int checkpubkey(unsigned char* algo, unsigned int algolen,
		unsigned char* keyblob, unsigned int keybloblen) {

	FILE * authfile = NULL;
	char * filename = NULL;
	int ret = DROPBEAR_FAILURE;
	buffer * line = NULL;
	buffer * decodekey = NULL;
	unsigned long decodekeylen;
	unsigned char* filealgo = NULL;
	unsigned int filealgolen;
	unsigned int len, pos;
	
	TRACE(("enter checkpubkey"));

	/* check that we can use the algo */
	if (have_algo(algo, algolen, sshhostkey) == DROPBEAR_FAILURE) {
		dropbear_log(LOG_WARNING,
				"pubkey auth attempt with unknown algo for '%s'",
				ses.authstate.printableuser);
		goto out;
	}

	/* check file permissions, also whether file exists */
	if (checkpubkeyperms() == DROPBEAR_FAILURE) {
		TRACE(("bad authorized_keys permissions, or file doesn't exist"));
		goto out;
	}

	/* we don't need to check pw and pw_dir for validity, since
	 * its been done in checkpubkeyperms. */
	len = strlen(ses.authstate.pw->pw_dir);
	/* allocate max required pathname storage,
	 * = path + "/.ssh/authorized_keys" + '\0' = pathlen + 22 */
	filename = m_malloc(len + 22);
	strncpy(filename, ses.authstate.pw->pw_dir, len + 1);
	strncat(filename, "/.ssh/authorized_keys", 21);

	/* open the file */
	authfile = fopen(filename, "r");
	if (authfile == NULL) {
		goto out;
	}
	TRACE(("checkpubkey: opened authorized_keys OK"));

	line = buf_new(INIT_AUTHKEYS_LINE);

	/* iterate through the lines */
	do {
		char* thealgo  = NULL;
		/* free reused vars */
		if (decodekey) {
			buf_free(decodekey);
			decodekey = NULL;
		}
		m_free(filealgo);

		if (getauthline(line, authfile) == DROPBEAR_FAILURE) {
			/* EOF reached */
			TRACE(("checkpubkey: authorized_keys EOF reached"));
			break;
		}

		if (line->len < MIN_AUTHKEYS_LINE) {
			TRACE(("checkpubkey: line too short"));
			continue; /* line is too short for it to be a valid key */
		}

		/* check the key type - this also stops us from using keys
		 * which have options with them */
		thealgo = buf_getptr(line, algolen);
		if (strncmp(thealgo, algo, algolen) != 0) {
			TRACE(("unexpected algo, got %s we expected %s", thealgo, algo));
			continue;
		}
		buf_incrpos(line, algolen);
		
		/* check for space (' ') character */
		if (buf_getbyte(line) != ' ') {
			TRACE(("checkpubkey: space character expected, isn't there"));
			continue;
		}

		/* truncate the line at the space after the base64 data */
		pos = line->pos;
		for (len = 0; line->pos < line->len; len++) {
			if (buf_getbyte(line) == ' ') break;
		}	
		buf_setpos(line, pos);
		buf_setlen(line, line->pos + len);

		TRACE(("checkpubkey: line pos = %d len = %d", line->pos, line->len));

		/* now we have the actual data */
		decodekeylen = (line->len - line->pos) * 2;
		decodekey = buf_new(decodekeylen);
		if (base64_decode(buf_getptr(line, line->len - line->pos),
					line->len - line->pos,
					buf_getwriteptr(decodekey, decodekey->size),
					&decodekeylen) != CRYPT_OK) {
			TRACE(("checkpubkey: base64 decode failed"));
			continue;
		}
		TRACE(("checkpubkey: base64_decode success"));
		buf_incrlen(decodekey, decodekeylen);
		
		/* compare the keys */
		if (decodekeylen != keybloblen || memcmp(
					buf_getptr(decodekey, decodekey->len),
					keyblob, decodekey->len) != 0) {
			TRACE(("checkpubkey: compare failed"));
			continue;
		}

		/* and also check that the algo specified and the algo in the key
		 * itself match */
		filealgo = buf_getstring(decodekey, &filealgolen);
		if (filealgolen != algolen || memcmp(filealgo, algo, algolen) != 0) {
			TRACE(("checkpubkey: algo match failed")); 
			continue;
		}

		/* now we know this key is good */
		ret = DROPBEAR_SUCCESS;
		break;
		
	} while (1);

out:
	if (authfile) {
		fclose(authfile);
	}
	if (line) {
		buf_free(line);
	}
	if (decodekey) {
		buf_free(decodekey);
	}
	m_free(filename);
	m_free(filealgo);
	TRACE(("leave checkpubkey: ret=%d", ret));
	return ret;
}

/* get a line from the file into buffer in the style expected for an
 * authkeys file.
 * Will return DROPBEAR_SUCCESS if data is read, or DROPBEAR_FAILURE on EOF.*/
static int getauthline(buffer * line, FILE * authfile) {

	int c = EOF;

	TRACE(("enter getauthline"));

	buf_setpos(line, 0);
	buf_setlen(line, 0);

	while (line->pos < line->size) {
		c = fgetc(authfile); /*getc() is weird with some uClibc systems*/
		if (c == EOF || c == '\n' || c == '\r') {
			goto out;
		}
		buf_putbyte(line, (unsigned char)c);
		/* we only grow as big as we need */
		if (line->len == line->size && line->size < MAX_AUTHKEYS_LINE) {
			TRACE(("resizing line from %d to %d", line->size, 
						line->size + INIT_AUTHKEYS_LINE));
			buf_resize(line, line->size + INCR_AUTHKEYS_LINE);
		}
	}

	TRACE(("leave getauthline: line too long"));
	return DROPBEAR_FAILURE;

out:

	buf_setpos(line, 0);

	/* if we didn't read anything before EOF or error, exit */
	if (c == EOF && line->pos == 0) {
		TRACE(("leave getauthline: failure"));
		return DROPBEAR_FAILURE;
	} else {
		TRACE(("leave getauthline: success"));
		return DROPBEAR_SUCCESS;
	}

	TRACE(("leave getauthline"));
}	

/* Returns DROPBEAR_SUCCESS if file permissions for pubkeys are ok,
 * DROPBEAR_FAILURE otherwise.
 * Checks that the user's homedir, ~/.ssh, and
 * ~/.ssh/authorized_keys are all owned by either root or the user, and are
 * g-w, o-w */
static int checkpubkeyperms() {

	char* filename = NULL; 
	int ret = DROPBEAR_FAILURE;
	unsigned int len;

	TRACE(("enter checkpubkeyperms"));

	assert(ses.authstate.pw);
	if (ses.authstate.pw->pw_dir == NULL) {
		goto out;
	}

	if ((len = strlen(ses.authstate.pw->pw_dir)) == 0) {
		goto out;
	}

	/* allocate max required pathname storage,
	 * = path + "/.ssh/authorized_keys" + '\0' = pathlen + 22 */
	filename = m_malloc(len + 22);
	strncpy(filename, ses.authstate.pw->pw_dir, len+1);

	/* check ~ */
	if (checkfileperm(filename) != DROPBEAR_SUCCESS) {
		goto out;
	}

	/* check ~/.ssh */
	strncat(filename, "/.ssh", 5); /* strlen("/.ssh") == 5 */
	if (checkfileperm(filename) != DROPBEAR_SUCCESS) {
		goto out;
	}

	/* now check ~/.ssh/authorized_keys */
	strncat(filename, "/authorized_keys", 16);
	if (checkfileperm(filename) != DROPBEAR_SUCCESS) {
		goto out;
	}

	/* file looks ok, return success */
	ret = DROPBEAR_SUCCESS;
	
out:
	m_free(filename);

	TRACE(("leave checkpubkeyperms"));
	return ret;
}

/* Checks that a file is owned by the user or root, and isn't writable by
 * group or other */
/* returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
static int checkfileperm(char * filename) {
	struct stat filestat;

	TRACE(("enter checkfileperm(%s)", filename));

	if (stat(filename, &filestat) != 0) {
		TRACE(("leave checkfileperm: stat() != 0"));
		return DROPBEAR_FAILURE;
	}
	/* check ownership - user or root only*/
	if (filestat.st_uid != ses.authstate.pw->pw_uid
			&& filestat.st_uid != 0) {
		TRACE(("leave checkfileperm: wrong ownership"));
		return DROPBEAR_FAILURE;
	}
	/* check permissions - don't want group or others +w */
	if (filestat.st_mode & (S_IWGRP | S_IWOTH)) {
		TRACE(("leave checkfileperm: wrong perms"));
		return DROPBEAR_FAILURE;
	}
	TRACE(("leave checkfileperm: success"));
	return DROPBEAR_SUCCESS;
}


#endif /* DROPBEAR_PUBKEY_AUTH */
