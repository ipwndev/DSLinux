/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002-2004 Matt Johnston
 * Copyright (c) 2004 by Mihnea Stoenescu
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

#include "includes.h"
#include "session.h"
#include "dbutil.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "kex.h"
#include "ssh.h"
#include "packet.h"
#include "bignum.h"
#include "random.h"
#include "runopts.h"
#include "signkey.h"


static void checkhostkey(unsigned char* keyblob, unsigned int keybloblen);
#define MAX_KNOWNHOSTS_LINE 4500

void send_msg_kexdh_init() {

	cli_ses.dh_e = (mp_int*)m_malloc(sizeof(mp_int));
	cli_ses.dh_x = (mp_int*)m_malloc(sizeof(mp_int));
	m_mp_init_multi(cli_ses.dh_e, cli_ses.dh_x, NULL);

	gen_kexdh_vals(cli_ses.dh_e, cli_ses.dh_x);

	CHECKCLEARTOWRITE();
	buf_putbyte(ses.writepayload, SSH_MSG_KEXDH_INIT);
	buf_putmpint(ses.writepayload, cli_ses.dh_e);
	encrypt_packet();
	ses.requirenext = SSH_MSG_KEXDH_REPLY;
}

/* Handle a diffie-hellman key exchange reply. */
void recv_msg_kexdh_reply() {

	DEF_MP_INT(dh_f);
	sign_key *hostkey = NULL;
	unsigned int type, keybloblen;
	unsigned char* keyblob = NULL;


	TRACE(("enter recv_msg_kexdh_reply"))

	if (cli_ses.kex_state != KEXDH_INIT_SENT) {
		dropbear_exit("Received out-of-order kexdhreply");
	}
	m_mp_init(&dh_f);
	type = ses.newkeys->algo_hostkey;
	TRACE(("type is %d", type))

	hostkey = new_sign_key();
	keybloblen = buf_getint(ses.payload);

	keyblob = buf_getptr(ses.payload, keybloblen);
	if (!ses.kexstate.donefirstkex) {
		/* Only makes sense the first time */
		checkhostkey(keyblob, keybloblen);
	}

	if (buf_get_pub_key(ses.payload, hostkey, &type) != DROPBEAR_SUCCESS) {
		TRACE(("failed getting pubkey"))
		dropbear_exit("Bad KEX packet");
	}

	if (buf_getmpint(ses.payload, &dh_f) != DROPBEAR_SUCCESS) {
		TRACE(("failed getting mpint"))
		dropbear_exit("Bad KEX packet");
	}

	kexdh_comb_key(cli_ses.dh_e, cli_ses.dh_x, &dh_f, hostkey);
	mp_clear(&dh_f);
	mp_clear_multi(cli_ses.dh_e, cli_ses.dh_x, NULL);
	m_free(cli_ses.dh_e);
	m_free(cli_ses.dh_x);

	if (buf_verify(ses.payload, hostkey, ses.hash, SHA1_HASH_SIZE) 
			!= DROPBEAR_SUCCESS) {
		dropbear_exit("Bad hostkey signature");
	}

	sign_key_free(hostkey);
	hostkey = NULL;

	send_msg_newkeys();
	ses.requirenext = SSH_MSG_NEWKEYS;
	TRACE(("leave recv_msg_kexdh_init"))
}

static void ask_to_confirm(unsigned char* keyblob, unsigned int keybloblen) {

	char* fp = NULL;
	FILE *tty = NULL;
	char response = 'z';

	fp = sign_key_fingerprint(keyblob, keybloblen);
	fprintf(stderr, "\nHost '%s' is not in the trusted hosts file.\n(fingerprint %s)\nDo you want to continue connecting? (y/n)\n", 
			cli_opts.remotehost, 
			fp);

	tty = fopen(_PATH_TTY, "r");
	if (tty) {
		response = getc(tty);
		fclose(tty);
	} else {
		response = getc(stdin);
	}

	if (response == 'y') {
		m_free(fp);
		return;
	}

	dropbear_exit("Didn't validate host key");
}

static void checkhostkey(unsigned char* keyblob, unsigned int keybloblen) {

	char * filename = NULL;
	FILE *hostsfile = NULL;
	int readonly = 0;
	struct passwd *pw = NULL;
	unsigned int hostlen, algolen;
	unsigned long len;
	const char *algoname = NULL;
	buffer * line = NULL;
	int ret;
	
	pw = getpwuid(getuid());

	if (pw == NULL) {
		dropbear_exit("Failed to get homedir");
	}

	len = strlen(pw->pw_dir);
	filename = m_malloc(len + 18); /* "/.ssh/known_hosts" and null-terminator*/

	snprintf(filename, len+18, "%s/.ssh", pw->pw_dir);
	/* Check that ~/.ssh exists - easiest way is just to mkdir */
	if (mkdir(filename, S_IRWXU) != 0) {
		if (errno != EEXIST) {
			dropbear_log(LOG_INFO, "Warning: failed creating ~/.ssh: %s",
					strerror(errno));
			TRACE(("mkdir didn't work: %s", strerror(errno)))
			ask_to_confirm(keyblob, keybloblen);
			goto out; /* only get here on success */
		}
	}

	snprintf(filename, len+18, "%s/.ssh/known_hosts", pw->pw_dir);
	hostsfile = fopen(filename, "a+");
	
	if (hostsfile != NULL) {
		fseek(hostsfile, 0, SEEK_SET);
	} else {
		/* We mightn't have been able to open it if it was read-only */
		if (errno == EACCES || errno == EROFS) {
				TRACE(("trying readonly: %s", strerror(errno)))
				readonly = 1;
				hostsfile = fopen(filename, "r");
		}
	}

	if (hostsfile == NULL) {
		TRACE(("hostsfile didn't open: %s", strerror(errno)))
		ask_to_confirm(keyblob, keybloblen);
		goto out; /* We only get here on success */
	}

	line = buf_new(MAX_KNOWNHOSTS_LINE);
	hostlen = strlen(cli_opts.remotehost);
	algoname = signkey_name_from_type(ses.newkeys->algo_hostkey, &algolen);

	do {
		if (buf_getline(line, hostsfile) == DROPBEAR_FAILURE) {
			TRACE(("failed reading line: prob EOF"))
			break;
		}

		/* The line is too short to be sensible */
		/* "30" is 'enough to hold ssh-dss plus the spaces, ie so we don't
		 * buf_getfoo() past the end and die horribly - the base64 parsing
		 * code is what tiptoes up to the end nicely */
		if (line->len < (hostlen+30) ) {
			TRACE(("line is too short to be sensible"))
			continue;
		}

		/* Compare hostnames */
		if (strncmp(cli_opts.remotehost, buf_getptr(line, hostlen),
					hostlen) != 0) {
			TRACE(("hosts don't match"))
			continue;
		}

		buf_incrpos(line, hostlen);
		if (buf_getbyte(line) != ' ') {
			/* there wasn't a space after the hostname, something dodgy */
			TRACE(("missing space afte matching hostname"))
			continue;
		}

		if ( strncmp(buf_getptr(line, algolen), algoname, algolen) != 0) {
			TRACE(("algo doesn't match"))
			continue;
		}

		buf_incrpos(line, algolen);
		if (buf_getbyte(line) != ' ') {
			TRACE(("missing space after algo"))
			continue;
		}

		/* Now we're at the interesting hostkey */
		ret = cmp_base64_key(keyblob, keybloblen, algoname, algolen, line);

		if (ret == DROPBEAR_SUCCESS) {
			/* Good matching key */
			TRACE(("good matching key"))
			goto out;
		}

		/* The keys didn't match. eep. */
	} while (1); /* keep going 'til something happens */

	/* Key doesn't exist yet */
	ask_to_confirm(keyblob, keybloblen);

	/* If we get here, they said yes */

	if (readonly) {
		TRACE(("readonly"))
		goto out;
	}

	/* put the new entry in the file */
	fseek(hostsfile, 0, SEEK_END); /* In case it wasn't opened append */
	buf_setpos(line, 0);
	buf_setlen(line, 0);
	buf_putbytes(line, ses.remotehost, hostlen);
	buf_putbyte(line, ' ');
	buf_putbytes(line, algoname, algolen);
	buf_putbyte(line, ' ');
	len = line->size - line->pos;
	TRACE(("keybloblen %d, len %d", keybloblen, len))
	/* The only failure with base64 is buffer_overflow, but buf_getwriteptr
	 * will die horribly in the case anyway */
	base64_encode(keyblob, keybloblen, buf_getwriteptr(line, len), &len);
	buf_incrwritepos(line, len);
	buf_putbyte(line, '\n');
	buf_setpos(line, 0);
	fwrite(buf_getptr(line, line->len), line->len, 1, hostsfile);
	/* We ignore errors, since there's not much we can do about them */

out:
	if (hostsfile != NULL) {
		fclose(hostsfile);
	}
	m_free(filename);
	if (line != NULL) {
		buf_free(line);
	}
}
