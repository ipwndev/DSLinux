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

#include "includes.h"
#include "dbutil.h"
#include "signkey.h"
#include "buffer.h"
#include "ssh.h"

/* malloc a new sign_key and set the dss and rsa keys to NULL */
sign_key * new_sign_key() {

	sign_key * ret;

	ret = (sign_key*)m_malloc(sizeof(sign_key));
#ifdef DROPBEAR_DSS
	ret->dsskey = NULL;
#endif
#ifdef DROPBEAR_RSA
	ret->rsakey = NULL;
#endif
	return ret;

}

/* Returns "ssh-dss" or "ssh-rsa" corresponding to the type. Exits fatally
 * if the type is invalid */
const char* signkey_name_from_type(int type, int *namelen) {

#ifdef DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		*namelen = SSH_SIGNKEY_RSA_LEN;
		return SSH_SIGNKEY_RSA;
	}
#endif
#ifdef DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		*namelen = SSH_SIGNKEY_DSS_LEN;
		return SSH_SIGNKEY_DSS;
	}
#endif
	dropbear_exit("bad key type %d", type);
	return NULL; /* notreached */
}

/* Returns DROPBEAR_SIGNKEY_RSA, DROPBEAR_SIGNKEY_DSS, 
 * or DROPBEAR_SIGNKEY_NONE */
int signkey_type_from_name(const char* name, int namelen) {

#ifdef DROPBEAR_RSA
	if (namelen == SSH_SIGNKEY_RSA_LEN
			&& memcmp(name, SSH_SIGNKEY_RSA, SSH_SIGNKEY_RSA_LEN) == 0) {
		return DROPBEAR_SIGNKEY_RSA;
	}
#endif
#ifdef DROPBEAR_DSS
	if (namelen == SSH_SIGNKEY_DSS_LEN
			&& memcmp(name, SSH_SIGNKEY_DSS, SSH_SIGNKEY_DSS_LEN) == 0) {
		return DROPBEAR_SIGNKEY_DSS;
	}
#endif

	return DROPBEAR_SIGNKEY_NONE;
}

/* returns DROPBEAR_SUCCESS on success, DROPBEAR_FAILURE on fail.
 * type should be set by the caller to specify the type to read, and
 * on return is set to the type read (useful when type = _ANY) */
int buf_get_pub_key(buffer *buf, sign_key *key, int *type) {

	unsigned char* ident;
	unsigned int len;
	int keytype;
	int ret = DROPBEAR_FAILURE;

	TRACE(("enter buf_get_pub_key"))

	ident = buf_getstring(buf, &len);
	keytype = signkey_type_from_name(ident, len);
	m_free(ident);

	if (*type != DROPBEAR_SIGNKEY_ANY && *type != keytype) {
		return DROPBEAR_FAILURE;
	}

	*type = keytype;

	/* Rewind the buffer back before "ssh-rsa" etc */
	buf_incrpos(buf, -len - 4);

#ifdef DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		dss_key_free(key->dsskey);
		key->dsskey = (dss_key*)m_malloc(sizeof(dss_key));
		ret = buf_get_dss_pub_key(buf, key->dsskey);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->dsskey);
		}
	}
#endif
#ifdef DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		rsa_key_free(key->rsakey);
		key->rsakey = (rsa_key*)m_malloc(sizeof(rsa_key));
		ret = buf_get_rsa_pub_key(buf, key->rsakey);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->rsakey);
		}
	}
#endif

	TRACE(("leave buf_get_pub_key"))

	return ret;
	
}

/* returns DROPBEAR_SUCCESS on success, DROPBEAR_FAILURE on fail.
 * type should be set by the caller to specify the type to read, and
 * on return is set to the type read (useful when type = _ANY) */
int buf_get_priv_key(buffer *buf, sign_key *key, int *type) {

	unsigned char* ident;
	unsigned int len;
	int keytype;
	int ret = DROPBEAR_FAILURE;

	TRACE(("enter buf_get_priv_key"))

	ident = buf_getstring(buf, &len);
	keytype = signkey_type_from_name(ident, len);
	m_free(ident);

	if (*type != DROPBEAR_SIGNKEY_ANY && *type != keytype) {
		TRACE(("wrong key type: %d %d", *type, keytype))
		return DROPBEAR_FAILURE;
	}

	*type = keytype;

	/* Rewind the buffer back before "ssh-rsa" etc */
	buf_incrpos(buf, -len - 4);

#ifdef DROPBEAR_DSS
	if (keytype == DROPBEAR_SIGNKEY_DSS) {
		dss_key_free(key->dsskey);
		key->dsskey = (dss_key*)m_malloc(sizeof(dss_key));
		ret = buf_get_dss_priv_key(buf, key->dsskey);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->dsskey);
		}
	}
#endif
#ifdef DROPBEAR_RSA
	if (keytype == DROPBEAR_SIGNKEY_RSA) {
		rsa_key_free(key->rsakey);
		key->rsakey = (rsa_key*)m_malloc(sizeof(rsa_key));
		ret = buf_get_rsa_priv_key(buf, key->rsakey);
		if (ret == DROPBEAR_FAILURE) {
			m_free(key->rsakey);
		}
	}
#endif

	TRACE(("leave buf_get_priv_key"))

	return ret;
	
}

/* type is either DROPBEAR_SIGNKEY_DSS or DROPBEAR_SIGNKEY_RSA */
void buf_put_pub_key(buffer* buf, sign_key *key, int type) {

	buffer *pubkeys;

	TRACE(("enter buf_put_pub_key"))
	pubkeys = buf_new(MAX_PUBKEY_SIZE);
	
#ifdef DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_pub_key(pubkeys, key->dsskey);
	}
#endif
#ifdef DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_pub_key(pubkeys, key->rsakey);
	}
#endif
	if (pubkeys->len == 0) {
		dropbear_exit("bad key types in buf_put_pub_key");
	}

	buf_setpos(pubkeys, 0);
	buf_putstring(buf, buf_getptr(pubkeys, pubkeys->len),
			pubkeys->len);
	
	buf_free(pubkeys);
	TRACE(("leave buf_put_pub_key"))
}

/* type is either DROPBEAR_SIGNKEY_DSS or DROPBEAR_SIGNKEY_RSA */
void buf_put_priv_key(buffer* buf, sign_key *key, int type) {

	TRACE(("enter buf_put_priv_key"))
	TRACE(("type is %d", type))

#ifdef DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_priv_key(buf, key->dsskey);
	TRACE(("leave buf_put_priv_key: dss done"))
	return;
	}
#endif
#ifdef DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_priv_key(buf, key->rsakey);
	TRACE(("leave buf_put_priv_key: rsa done"))
	return;
	}
#endif
	dropbear_exit("bad key types in put pub key");
}

void sign_key_free(sign_key *key) {

	TRACE(("enter sign_key_free"))

#ifdef DROPBEAR_DSS
	dss_key_free(key->dsskey);
	key->dsskey = NULL;
#endif
#ifdef DROPBEAR_RSA
	rsa_key_free(key->rsakey);
	key->rsakey = NULL;
#endif

	m_free(key);
	TRACE(("leave sign_key_free"))
}

static char hexdig(unsigned char x) {

	if (x > 0xf)
		return 'X';

	if (x < 10)
		return '0' + x;
	else
		return 'a' + x - 10;
}

/* Since we're not sure if we'll have md5 or sha1, we present both.
 * MD5 is used in preference, but sha1 could still be useful */
#ifdef DROPBEAR_MD5_HMAC
static char * sign_key_md5_fingerprint(unsigned char* keyblob,
		unsigned int keybloblen) {

	char * ret;
	hash_state hs;
	unsigned char hash[MD5_HASH_SIZE];
	unsigned int i;
	unsigned int buflen;

	md5_init(&hs);

	/* skip the size int of the string - this is a bit messy */
	md5_process(&hs, keyblob, keybloblen);

	md5_done(&hs, hash);

	/* "md5 hexfingerprinthere\0", each hex digit is "AB:" etc */
	buflen = 4 + 3*MD5_HASH_SIZE;
	ret = (char*)m_malloc(buflen);

	memset(ret, 'Z', buflen);
	strcpy(ret, "md5 ");

	for (i = 0; i < MD5_HASH_SIZE; i++) {
		unsigned int pos = 4 + i*3;
		ret[pos] = hexdig(hash[i] >> 4);
		ret[pos+1] = hexdig(hash[i] & 0x0f);
		ret[pos+2] = ':';
	}
	ret[buflen-1] = 0x0;

	return ret;
}

#else /* use SHA1 rather than MD5 for fingerprint */
static char * sign_key_sha1_fingerprint(unsigned char* keyblob, 
		unsigned int keybloblen) {

	char * ret;
	hash_state hs;
	unsigned char hash[SHA1_HASH_SIZE];
	unsigned int i;
	unsigned int buflen;

	sha1_init(&hs);

	/* skip the size int of the string - this is a bit messy */
	sha1_process(&hs, keyblob, keybloblen);

	sha1_done(&hs, hash);

	/* "sha1 hexfingerprinthere\0", each hex digit is "AB:" etc */
	buflen = 5 + 3*SHA1_HASH_SIZE;
	ret = (char*)m_malloc(buflen);

	strcpy(ret, "sha1 ");

	for (i = 0; i < SHA1_HASH_SIZE; i++) {
		unsigned int pos = 5 + 3*i;
		ret[pos] = hexdig(hash[i] >> 4);
		ret[pos+1] = hexdig(hash[i] & 0x0f);
		ret[pos+2] = ':';
	}
	ret[buflen-1] = 0x0;

	return ret;
}

#endif /* MD5/SHA1 switch */

/* This will return a freshly malloced string, containing a fingerprint
 * in either sha1 or md5 */
char * sign_key_fingerprint(unsigned char* keyblob, unsigned int keybloblen) {

#ifdef DROPBEAR_MD5_HMAC
	return sign_key_md5_fingerprint(keyblob, keybloblen);
#else
	return sign_key_sha1_fingerprint(keyblob, keybloblen);
#endif
}

void buf_put_sign(buffer* buf, sign_key *key, int type, 
		const unsigned char *data, unsigned int len) {

	buffer *sigblob;

	sigblob = buf_new(MAX_PUBKEY_SIZE);

#ifdef DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		buf_put_dss_sign(sigblob, key->dsskey, data, len);
	}
#endif
#ifdef DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		buf_put_rsa_sign(sigblob, key->rsakey, data, len);
	}
#endif
	if (sigblob->len == 0) {
		dropbear_exit("non-matching signing type");
	}

	buf_setpos(sigblob, 0);
	buf_putstring(buf, buf_getptr(sigblob, sigblob->len),
			sigblob->len);
			
	buf_free(sigblob);

}

#ifdef DROPBEAR_SIGNKEY_VERIFY
/* Return DROPBEAR_SUCCESS or DROPBEAR_FAILURE.
 * If FAILURE is returned, the position of
 * buf is undefined. If SUCCESS is returned, buf will be positioned after the
 * signature blob */
int buf_verify(buffer * buf, sign_key *key, const unsigned char *data,
		unsigned int len) {
	
	unsigned int bloblen;
	unsigned char * ident = NULL;
	unsigned int identlen = 0;

	TRACE(("enter buf_verify"))

	bloblen = buf_getint(buf);
	ident = buf_getstring(buf, &identlen);

#ifdef DROPBEAR_DSS
	if (bloblen == DSS_SIGNATURE_SIZE &&
			memcmp(ident, SSH_SIGNKEY_DSS, identlen) == 0) {
		m_free(ident);
		if (key->dsskey == NULL) {
			dropbear_exit("no dss key to verify signature");
		}
		return buf_dss_verify(buf, key->dsskey, data, len);
	}
#endif

#ifdef DROPBEAR_RSA
	if (memcmp(ident, SSH_SIGNKEY_RSA, identlen) == 0) {
		m_free(ident);
		if (key->rsakey == NULL) {
			dropbear_exit("no rsa key to verify signature");
		}
		return buf_rsa_verify(buf, key->rsakey, data, len);
	}
#endif

	m_free(ident);
	dropbear_exit("non-matching signing type");
	return DROPBEAR_FAILURE;
}
#endif /* DROPBEAR_SIGNKEY_VERIFY */

#ifdef DROPBEAR_KEY_LINES /* ie we're using authorized_keys or known_hosts */

/* Returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE when given a buffer containing
 * a key, a key, and a type. The buffer is positioned at the start of the
 * base64 data, and contains no trailing data */
int cmp_base64_key(const unsigned char* keyblob, unsigned int keybloblen, 
					const unsigned char* algoname, unsigned int algolen, 
					buffer * line) {

	buffer * decodekey = NULL;
	int ret = DROPBEAR_FAILURE;
	unsigned int len, filealgolen;
	unsigned long decodekeylen;
	unsigned char* filealgo = NULL;

	/* now we have the actual data */
	len = line->len - line->pos;
	decodekeylen = len * 2; /* big to be safe */
	decodekey = buf_new(decodekeylen);

	if (base64_decode(buf_getptr(line, len), len,
				buf_getwriteptr(decodekey, decodekey->size),
				&decodekeylen) != CRYPT_OK) {
		TRACE(("checkpubkey: base64 decode failed"))
		goto out;
	}
	TRACE(("checkpubkey: base64_decode success"))
	buf_incrlen(decodekey, decodekeylen);
	
	/* compare the keys */
	if ( ( decodekeylen != keybloblen )
			|| memcmp( buf_getptr(decodekey, decodekey->len),
						keyblob, decodekey->len) != 0) {
		TRACE(("checkpubkey: compare failed"))
		goto out;
	}

	/* ... and also check that the algo specified and the algo in the key
	 * itself match */
	filealgolen = buf_getint(decodekey);
	filealgo = buf_getptr(decodekey, filealgolen);
	if (filealgolen != algolen || memcmp(filealgo, algoname, algolen) != 0) {
		TRACE(("checkpubkey: algo match failed")) 
		goto out;
	}

	/* All checks passed */
	ret = DROPBEAR_SUCCESS;

out:
	buf_free(decodekey);
	decodekey = NULL;
	return ret;
}
#endif
