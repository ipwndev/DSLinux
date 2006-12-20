/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2004, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN
#endif

#include "common.h"
#include "sha1.h"


void sha1_mac(unsigned char *key, unsigned int key_len,
	      unsigned char *data, unsigned int data_len,
	      unsigned char *mac)
{
	SHA1_CTX context;
	SHA1Init(&context);
	SHA1Update(&context, key, key_len);
	SHA1Update(&context, data, data_len);
	SHA1Update(&context, key, key_len);
	SHA1Final(mac, &context);
}


/* HMAC code is based on RFC 2104 */
void hmac_sha1_vector(unsigned char *key, unsigned int key_len,
		      size_t num_elem, unsigned char *addr[],
		      unsigned int *len, unsigned char *mac)
{
	SHA1_CTX context;
	unsigned char k_ipad[65]; /* inner padding - key XORd with ipad */
	unsigned char k_opad[65]; /* outer padding - key XORd with opad */
	unsigned char tk[20];
	int i;

        /* if key is longer than 64 bytes reset it to key = SHA1(key) */
        if (key_len > 64) {
		SHA1Init(&context);
		SHA1Update(&context, key, key_len);
		SHA1Final(tk, &context);

		key = tk;
		key_len = 20;
        }

	/* the HMAC_SHA1 transform looks like:
	 *
	 * SHA1(K XOR opad, SHA1(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected */

	/* start out by storing key in pads */
	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memcpy(k_ipad, key, key_len);
	memcpy(k_opad, key, key_len);

	/* XOR key with ipad and opad values */
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* perform inner SHA1 */
	SHA1Init(&context);                   /* init context for 1st pass */
	SHA1Update(&context, k_ipad, 64);     /* start with inner pad */
	/* then text of datagram; all fragments */
	for (i = 0; i < num_elem; i++) {
		SHA1Update(&context, addr[i], len[i]);
	}
	SHA1Final(mac, &context);             /* finish up 1st pass */

	/* perform outer SHA1 */
	SHA1Init(&context);                   /* init context for 2nd pass */
	SHA1Update(&context, k_opad, 64);     /* start with outer pad */
	SHA1Update(&context, mac, 20);        /* then results of 1st hash */
	SHA1Final(mac, &context);             /* finish up 2nd pass */
}


void hmac_sha1(unsigned char *key, unsigned int key_len,
	       unsigned char *data, unsigned int data_len,
	       unsigned char *mac)
{
	hmac_sha1_vector(key, key_len, 1, &data, &data_len, mac);
}


void sha1_prf(unsigned char *key, unsigned int key_len,
	      char *label, unsigned char *data, unsigned int data_len,
	      unsigned char *buf, size_t buf_len)
{
	char zero = 0, counter = 0;
	size_t pos, plen;
	u8 hash[SHA1_MAC_LEN];
	size_t label_len = strlen(label);
	unsigned char *addr[] = { label, &zero, data, &counter };
	unsigned int len[] = { label_len, 1, data_len, 1 };

	pos = 0;
	while (pos < buf_len) {
		plen = buf_len - pos;
		if (plen >= SHA1_MAC_LEN) {
			hmac_sha1_vector(key, key_len, 4, addr, len,
					 &buf[pos]);
			pos += SHA1_MAC_LEN;
		} else {
			hmac_sha1_vector(key, key_len, 4, addr, len,
					 hash);
			memcpy(&buf[pos], hash, plen);
			break;
		}
		counter++;
	}
}


static void pbkdf2_sha1_f(char *passphrase, char *ssid,
			  size_t ssid_len, int iterations, int count,
			  unsigned char *digest)
{
	unsigned char tmp[SHA1_MAC_LEN], tmp2[SHA1_MAC_LEN];
	int i, j;
	unsigned char count_buf[4];
	unsigned char *addr[] = { ssid, count_buf };
	unsigned int len[] = { ssid_len, 4 };
	size_t passphrase_len = strlen(passphrase);

	/* F(P, S, c, i) = U1 xor U2 xor ... Uc
	 * U1 = PRF(P, S || i)
	 * U2 = PRF(P, U1)
	 * Uc = PRF(P, Uc-1)
	 */

	count_buf[0] = (count >> 24) & 0xff;
	count_buf[1] = (count >> 16) & 0xff;
	count_buf[2] = (count >> 8) & 0xff;
	count_buf[3] = count & 0xff;
	hmac_sha1_vector(passphrase, passphrase_len, 2, addr, len, tmp);
	memcpy(digest, tmp, SHA1_MAC_LEN);

	for (i = 1; i < iterations; i++) {
		hmac_sha1(passphrase, passphrase_len, tmp, SHA1_MAC_LEN,
			  tmp2);
		memcpy(tmp, tmp2, SHA1_MAC_LEN);
		for (j = 0; j < SHA1_MAC_LEN; j++)
			digest[j] ^= tmp2[j];
	}
}


void pbkdf2_sha1(char *passphrase, char *ssid, size_t ssid_len, int iterations,
		 unsigned char *buf, size_t buflen)
{
	int count = 0;
	unsigned char *pos = buf;
	size_t left = buflen, plen;
	unsigned char digest[SHA1_MAC_LEN];

	while (left > 0) {
		count++;
		pbkdf2_sha1_f(passphrase, ssid, ssid_len, iterations, count,
			      digest);
		plen = left > SHA1_MAC_LEN ? SHA1_MAC_LEN : left;
		memcpy(pos, digest, plen);
		pos += plen;
		left -= plen;
	}
}


void sha1_transform(u8 *state, u8 data[64])
{
#ifdef EAP_TLS_FUNCS
	SHA_CTX context;
	memset(&context, 0, sizeof(context));
	memcpy(&context.h0, state, 5 * 4);
	SHA1_Transform(&context, data);
	memcpy(state, &context.h0, 5 * 4);
#else /* EAP_TLS_FUNCS */
	SHA1Transform((u32 *) state, data);
#endif /* EAP_TLS_FUNCS */
}


#ifndef EAP_TLS_FUNCS

/* ===== start - public domain SHA1 implementation ===== */

/*
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

-----------------
Modified 7/98 
By James H. Brown <jbrown@burgoyne.com>
Still 100% Public Domain

Corrected a problem which generated improper hash values on 16 bit machines
Routine SHA1Update changed from
	void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int
len)
to
	void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned
long len)

The 'len' parameter was declared an int which works fine on 32 bit machines.
However, on 16 bit machines an int is too small for the shifts being done
against
it.  This caused the hash function to generate incorrect values if len was
greater than 8191 (8K - 1) due to the 'len << 3' on line 3 of SHA1Update().

Since the file IO in main() reads 16K at a time, any file 8K or larger would
be guaranteed to generate the wrong hash (e.g. Test Vector #3, a million
"a"s).

I also changed the declaration of variables i & j in SHA1Update to 
unsigned long from unsigned int for the same reason.

These changes should make no difference to any 32 bit implementations since
an
int and a long are the same size in those environments.

--
I also corrected a few compiler warnings generated by Borland C.
1. Added #include <process.h> for exit() prototype
2. Removed unused variable 'j' in SHA1Final
3. Changed exit(0) to return(0) at end of main.

ALL changes I made can be located by searching for comments containing 'JHB'
-----------------
Modified 8/98
By Steve Reid <sreid@sea-to-sky.net>
Still 100% public domain

1- Removed #include <process.h> and used return() instead of exit()
2- Fixed overwriting of finalcount in SHA1Final() (discovered by Chris Hall)
3- Changed email address from steve@edmweb.com to sreid@sea-to-sky.net

-----------------
Modified 4/01
By Saul Kravitz <Saul.Kravitz@celera.com>
Still 100% PD
Modified to run on Compaq Alpha hardware.  

-----------------
Modified 4/01
By Jouni Malinen <jkmaline@cc.hut.fi>
Minor changes to match the coding style used in Dynamics.

Modified September 24, 2004
By Jouni Malinen <jkmaline@cc.hut.fi>
Fixed alignment issue in SHA1Transform when SHA1HANDSOFF is defined.

*/

/*
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#define SHA1HANDSOFF

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifndef WORDS_BIGENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | \
	(rol(block->l[i], 8) & 0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
	block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R1(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R2(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
	w = rol(w, 30);
#define R4(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
	w=rol(w, 30);


#ifdef VERBOSE  /* SAK */
void SHAPrintContext(SHA1_CTX *context, char *msg)
{
	printf("%s (%d,%d) %x %x %x %x %x\n",
	       msg,
	       context->count[0], context->count[1], 
	       context->state[0],
	       context->state[1],
	       context->state[2],
	       context->state[3],
	       context->state[4]);
}
#endif

/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(u32 state[5], unsigned char buffer[64])
{
	u32 a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		u32 l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
	u32 workspace[16];
	block = (CHAR64LONG16 *) workspace;
	memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16 *) buffer;
#endif
	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
	memset(block, 0, 64);
#endif
}


/* SHA1Init - Initialize new context */

void SHA1Init(SHA1_CTX* context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Update(SHA1_CTX* context, unsigned char* data, u32 len)
{
	u32 i, j;

#ifdef VERBOSE
	SHAPrintContext(context, "before");
#endif
	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3) < (len << 3))
		context->count[1]++;
	context->count[1] += (len >> 29);
	if ((j + len) > 63) {
		memcpy(&context->buffer[j], data, (i = 64-j));
		SHA1Transform(context->state, context->buffer);
		for ( ; i + 63 < len; i += 64) {
			SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	}
	else i = 0;
	memcpy(&context->buffer[j], &data[i], len - i);
#ifdef VERBOSE
	SHAPrintContext(context, "after ");
#endif
}


/* Add padding and return the message digest. */

void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
	u32 i;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++) {
		finalcount[i] = (unsigned char)
			((context->count[(i >= 4 ? 0 : 1)] >>
			  ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	SHA1Update(context, (unsigned char *) "\200", 1);
	while ((context->count[0] & 504) != 448) {
		SHA1Update(context, (unsigned char *) "\0", 1);
	}
	SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform()
					      */
	for (i = 0; i < 20; i++) {
		digest[i] = (unsigned char)
			((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
			 255);
	}
	/* Wipe variables */
	i = 0;
	memset(context->buffer, 0, 64);
	memset(context->state, 0, 20);
	memset(context->count, 0, 8);
	memset(finalcount, 0, 8);
}

/* ===== end - public domain SHA1 implementation ===== */

#endif /* EAP_TLS_FUNCS */


#ifdef TEST_MAIN
static u8 key0[] =
{
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0b, 0x0b, 0x0b, 0x0b
};
static u8 data0[] = "Hi There";
static u8 prf0[] =
{
	0xbc, 0xd4, 0xc6, 0x50, 0xb3, 0x0b, 0x96, 0x84,
	0x95, 0x18, 0x29, 0xe0, 0xd7, 0x5f, 0x9d, 0x54,
	0xb8, 0x62, 0x17, 0x5e, 0xd9, 0xf0, 0x06, 0x06,
	0xe1, 0x7d, 0x8d, 0xa3, 0x54, 0x02, 0xff, 0xee,
	0x75, 0xdf, 0x78, 0xc3, 0xd3, 0x1e, 0x0f, 0x88,
	0x9f, 0x01, 0x21, 0x20, 0xc0, 0x86, 0x2b, 0xeb,
	0x67, 0x75, 0x3e, 0x74, 0x39, 0xae, 0x24, 0x2e,
	0xdb, 0x83, 0x73, 0x69, 0x83, 0x56, 0xcf, 0x5a
};

static u8 key1[] = "Jefe";
static u8 data1[] = "what do ya want for nothing?";
static u8 prf1[] =
{
	0x51, 0xf4, 0xde, 0x5b, 0x33, 0xf2, 0x49, 0xad,
	0xf8, 0x1a, 0xeb, 0x71, 0x3a, 0x3c, 0x20, 0xf4,
	0xfe, 0x63, 0x14, 0x46, 0xfa, 0xbd, 0xfa, 0x58,
	0x24, 0x47, 0x59, 0xae, 0x58, 0xef, 0x90, 0x09,
	0xa9, 0x9a, 0xbf, 0x4e, 0xac, 0x2c, 0xa5, 0xfa,
	0x87, 0xe6, 0x92, 0xc4, 0x40, 0xeb, 0x40, 0x02,
	0x3e, 0x7b, 0xab, 0xb2, 0x06, 0xd6, 0x1d, 0xe7,
	0xb9, 0x2f, 0x41, 0x52, 0x90, 0x92, 0xb8, 0xfc
};


static u8 key2[] =
{
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa
};
static u8 data2[] =
{
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	0xdd, 0xdd
};
static u8 prf2[] =
{
	0xe1, 0xac, 0x54, 0x6e, 0xc4, 0xcb, 0x63, 0x6f,
	0x99, 0x76, 0x48, 0x7b, 0xe5, 0xc8, 0x6b, 0xe1,
	0x7a, 0x02, 0x52, 0xca, 0x5d, 0x8d, 0x8d, 0xf1,
	0x2c, 0xfb, 0x04, 0x73, 0x52, 0x52, 0x49, 0xce,
	0x9d, 0xd8, 0xd1, 0x77, 0xea, 0xd7, 0x10, 0xbc,
	0x9b, 0x59, 0x05, 0x47, 0x23, 0x91, 0x07, 0xae,
	0xf7, 0xb4, 0xab, 0xd4, 0x3d, 0x87, 0xf0, 0xa6,
	0x8f, 0x1c, 0xbd, 0x9e, 0x2b, 0x6f, 0x76, 0x07
};


struct passphrase_test {
	char *passphrase;
	char *ssid;
	char psk[32];
};

static struct passphrase_test passphrase_tests[] =
{
	{
		"password",
		"IEEE",
		{
			0xf4, 0x2c, 0x6f, 0xc5, 0x2d, 0xf0, 0xeb, 0xef,
			0x9e, 0xbb, 0x4b, 0x90, 0xb3, 0x8a, 0x5f, 0x90,
			0x2e, 0x83, 0xfe, 0x1b, 0x13, 0x5a, 0x70, 0xe2,
			0x3a, 0xed, 0x76, 0x2e, 0x97, 0x10, 0xa1, 0x2e
		}
	},
	{
		"ThisIsAPassword",
		"ThisIsASSID",
		{
			0x0d, 0xc0, 0xd6, 0xeb, 0x90, 0x55, 0x5e, 0xd6,
			0x41, 0x97, 0x56, 0xb9, 0xa1, 0x5e, 0xc3, 0xe3,
			0x20, 0x9b, 0x63, 0xdf, 0x70, 0x7d, 0xd5, 0x08,
			0xd1, 0x45, 0x81, 0xf8, 0x98, 0x27, 0x21, 0xaf
		}
	},
	{
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
		"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
		{
			0xbe, 0xcb, 0x93, 0x86, 0x6b, 0xb8, 0xc3, 0x83,
			0x2c, 0xb7, 0x77, 0xc2, 0xf5, 0x59, 0x80, 0x7c,
			0x8c, 0x59, 0xaf, 0xcb, 0x6e, 0xae, 0x73, 0x48,
			0x85, 0x00, 0x13, 0x00, 0xa9, 0x81, 0xcc, 0x62
		}
	},
};

#define NUM_PASSPHRASE_TESTS \
(sizeof(passphrase_tests) / sizeof(passphrase_tests[0]))


int main(int argc, char *argv[])
{
	u8 res[512];
	int ret = 0, i;

	printf("PRF-SHA1 test cases:\n");

	sha1_prf(key0, sizeof(key0), "prefix", data0, sizeof(data0) - 1,
		 res, sizeof(res));
	if (memcmp(res, prf0, sizeof(prf0)) == 0)
		printf("Test case 0 - OK\n");
	else {
		printf("Test case 0 - FAILED!\n");
		ret++;
	}

	sha1_prf(key1, sizeof(key1) - 1, "prefix", data1, sizeof(data1) - 1,
		 res, sizeof(res));
	if (memcmp(res, prf1, sizeof(prf1)) == 0)
		printf("Test case 1 - OK\n");
	else {
		printf("Test case 1 - FAILED!\n");
		ret++;
	}

	sha1_prf(key2, sizeof(key2), "prefix", data2, sizeof(data2),
		 res, sizeof(res));
	if (memcmp(res, prf2, sizeof(prf2)) == 0)
		printf("Test case 2 - OK\n");
	else {
		printf("Test case 2 - FAILED!\n");
		ret++;
	}

	printf("PBKDF2-SHA1 Passphrase test cases:\n");
	for (i = 0; i < NUM_PASSPHRASE_TESTS; i++) {
		u8 psk[32];
		struct passphrase_test *test = &passphrase_tests[i];
		pbkdf2_sha1(test->passphrase,
			    test->ssid, strlen(test->ssid),
			    4096, psk, 32);
		if (memcmp(psk, test->psk, 32) == 0)
			printf("Test case %d - OK\n", i);
		else {
			printf("Test case %d - FAILED!\n", i);
			ret++;
		}
	}

	return ret;
}
#endif /* TEST_MAIN */
