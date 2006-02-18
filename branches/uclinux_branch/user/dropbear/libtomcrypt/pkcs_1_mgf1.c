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
#include "mycrypt.h"

/* The Mask Generation Function (MGF1) for PKCS #1 -- Tom St Denis */

#ifdef PKCS_1

int pkcs_1_mgf1(const unsigned char *seed, unsigned long seedlen,
                      int            hash_idx,
                      unsigned char *mask, unsigned long masklen)
{
   unsigned long hLen, counter, x;
   int           err;
   hash_state    md;
   unsigned char buf[MAXBLOCKSIZE];
 
   _ARGCHK(seed != NULL);
   _ARGCHK(mask != NULL);

   /* ensure valid hash */
   if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) { 
      return err;
   }

   /* get hash output size */
   hLen = hash_descriptor[hash_idx].hashsize;

   /* start counter */
   counter = 0;

   while (masklen > 0) {
       /* handle counter */
       STORE32H(counter, buf);
       ++counter;

       /* get hash of seed || counter */
       hash_descriptor[hash_idx].init(&md);
       if ((err = hash_descriptor[hash_idx].process(&md, seed, seedlen)) != CRYPT_OK) {
          return err;
       }
       if ((err = hash_descriptor[hash_idx].process(&md, buf, 4)) != CRYPT_OK) {
          return err;
       }
       if ((err = hash_descriptor[hash_idx].done(&md, buf)) != CRYPT_OK) {
          return err;
       }

       /* store it */
       for (x = 0; x < hLen && masklen > 0; x++, masklen--) {
          *mask++ = buf[x];
       }
   }

   return CRYPT_OK;
}

#endif /* PKCS_1 */
