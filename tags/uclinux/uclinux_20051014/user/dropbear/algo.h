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

#ifndef _ALGO_H_

#define _ALGO_H_

#include "includes.h"
#include "buffer.h"

struct Algo_Type {

	unsigned char *name; /* identifying name */
	char val; /* a value for this cipher, or -1 for invalid */
	void *data; /* algorithm specific data */
	unsigned usable : 1; /* whether we can use this algorithm */

};

typedef struct Algo_Type algo_type;

/* lists mapping ssh types of algorithms to internal values */
extern algo_type sshkex[];
extern algo_type sshhostkey[];
extern algo_type sshciphers[];
extern algo_type sshhashes[];
extern algo_type sshcompress[];

extern const struct dropbear_cipher dropbear_nocipher;
extern const struct dropbear_hash dropbear_nohash;

struct dropbear_cipher {
	const struct _cipher_descriptor *cipherdesc;
	unsigned long keysize;
	unsigned char blocksize;
};

struct dropbear_hash {
	const struct _hash_descriptor *hashdesc;
	unsigned long keysize;
	unsigned char hashsize;
};

void crypto_init();
int have_algo(char* algo, size_t algolen, algo_type algos[]);
algo_type * buf_match_algo(buffer* buf, algo_type localalgos[],
		int *goodguess);
void buf_put_algolist(buffer * buf, algo_type localalgos[]);

#endif /* _ALGO_H_ */
