/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
 */
/* Submited by Dobes Vandermeer  (dobes@smartt.com) */

#include "mycrypt.h"

/*
    (1) append zeros to the end of K to create a B byte string
        (e.g., if K is of length 20 bytes and B=64, then K will be
         appended with 44 zero bytes 0x00)
    (2) XOR (bitwise exclusive-OR) the B byte string computed in step
        (1) with ipad (ipad = the byte 0x36 repeated B times)
    (3) append the stream of data 'text' to the B byte string resulting
        from step (2)
    (4) apply H to the stream generated in step (3)
    (5) XOR (bitwise exclusive-OR) the B byte string computed in
        step (1) with opad (opad = the byte 0x5C repeated B times.)
    (6) append the H result from step (4) to the B byte string
        resulting from step (5)
    (7) apply H to the stream generated in step (6) and output
        the result
*/

#ifdef HMAC

#define HMAC_BLOCKSIZE hash_descriptor[hash].blocksize

int hmac_done(hmac_state *hmac, unsigned char *hashOut, unsigned long *outlen)
{
    unsigned char buf[MAXBLOCKSIZE];
    unsigned char isha[MAXBLOCKSIZE];
    unsigned long hashsize, i;
    int hash, err;

    _ARGCHK(hmac != NULL);
    _ARGCHK(hashOut != NULL);

    hash = hmac->hash;
    if((err = hash_is_valid(hash)) != CRYPT_OK) {
        return err;
    }

    /* get the hash message digest size */
    hashsize = hash_descriptor[hash].hashsize;

    // Get the hash of the first HMAC vector plus the data
    if ((err = hash_descriptor[hash].done(&hmac->md, isha)) != CRYPT_OK) {
       return err;
    }

    // Create the second HMAC vector vector for step (3)
    for(i=0; i < HMAC_BLOCKSIZE; i++) {
        buf[i] = hmac->key[i] ^ 0x5C;
    }

    // Now calculate the "outer" hash for step (5), (6), and (7)
    hash_descriptor[hash].init(&hmac->md);
    hash_descriptor[hash].process(&hmac->md, buf, HMAC_BLOCKSIZE);
    hash_descriptor[hash].process(&hmac->md, isha, hashsize);
    hash_descriptor[hash].done(&hmac->md, buf);

    // copy to output 
    for (i = 0; i < hashsize && i < *outlen; i++) {
        hashOut[i] = buf[i];
    }
    *outlen = i;

#ifdef CLEAN_STACK
    zeromem(isha, sizeof(buf));
    zeromem(buf,  sizeof(isha));
    zeromem(hmac, sizeof(*hmac));
#endif
    return CRYPT_OK;
}

#endif
