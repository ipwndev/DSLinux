/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.org
 */
#include "tomcrypt.h"

/**
   @file ctr_start.c
   CTR implementation, start chain, Tom St Denis
*/


#ifdef CTR

/**
   Initialize a CTR context
   @param cipher      The index of the cipher desired
   @param IV          The initial vector
   @param key         The secret key 
   @param keylen      The length of the secret key (octets)
   @param num_rounds  Number of rounds in the cipher desired (0 for default)
   @param ctr_mode    The counter mode (CTR_COUNTER_LITTLE_ENDIAN or CTR_COUNTER_BIG_ENDIAN)
   @param ctr         The CTR state to initialize
   @return CRYPT_OK if successful
*/
int ctr_start(               int   cipher, 
              const unsigned char *IV, 
              const unsigned char *key,       int keylen, 
                             int  num_rounds, int ctr_mode,
                   symmetric_CTR *ctr)
{
   int x, err;

   LTC_ARGCHK(IV  != NULL);
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(ctr != NULL);

   /* bad param? */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   /* setup cipher */
   if ((err = cipher_descriptor[cipher].setup(key, keylen, num_rounds, &ctr->key)) != CRYPT_OK) {
      return err;
   }

   /* copy ctr */
   ctr->blocklen = cipher_descriptor[cipher].block_length;
   ctr->cipher   = cipher;
   ctr->padlen   = 0;
   ctr->mode     = ctr_mode;
   for (x = 0; x < ctr->blocklen; x++) {
       ctr->ctr[x] = IV[x];
   }
   cipher_descriptor[ctr->cipher].ecb_encrypt(ctr->ctr, ctr->pad, &ctr->key);
   return CRYPT_OK;
}

#endif

/* $Source$ */
/* $Revision$ */
/* $Date$ */
