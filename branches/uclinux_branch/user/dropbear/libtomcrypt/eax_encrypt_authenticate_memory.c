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

/* EAX Implementation by Tom St Denis */
#include "mycrypt.h"

#ifdef EAX_MODE

int eax_encrypt_authenticate_memory(int cipher,
    const unsigned char *key,    unsigned long keylen,
    const unsigned char *nonce,  unsigned long noncelen,
    const unsigned char *header, unsigned long headerlen,
    const unsigned char *pt,     unsigned long ptlen,
          unsigned char *ct,
          unsigned char *tag,    unsigned long *taglen)
{
   int err;
   eax_state eax;

   if ((err = eax_init(&eax, cipher, key, keylen, nonce, noncelen, header, headerlen)) != CRYPT_OK) {
      return err;
   }

   if ((err = eax_encrypt(&eax, pt, ct, ptlen)) != CRYPT_OK) {
      return err;
   }
 
   if ((err = eax_done(&eax, tag, taglen)) != CRYPT_OK) {
      return err;
   }

   return CRYPT_OK;
}

#endif
