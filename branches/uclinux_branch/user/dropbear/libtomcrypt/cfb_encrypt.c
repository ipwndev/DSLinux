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

#ifdef CFB

int cfb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CFB *cfb)
{
   int err;

   _ARGCHK(pt != NULL);
   _ARGCHK(ct != NULL);
   _ARGCHK(cfb != NULL);

   if ((err = cipher_is_valid(cfb->cipher)) != CRYPT_OK) {
       return err;
   }

   /* is blocklen/padlen valid? */
   if (cfb->blocklen < 0 || cfb->blocklen > (int)sizeof(cfb->IV) ||
       cfb->padlen   < 0 || cfb->padlen   > (int)sizeof(cfb->pad)) {
      return CRYPT_INVALID_ARG;
   }

   while (len-- > 0) {
       if (cfb->padlen == cfb->blocklen) {
          cipher_descriptor[cfb->cipher].ecb_encrypt(cfb->pad, cfb->IV, &cfb->key);
          cfb->padlen = 0;
       }
       cfb->pad[cfb->padlen] = (*ct = *pt ^ cfb->IV[cfb->padlen]);
       ++pt; 
       ++ct;
       ++cfb->padlen;
   }
   return CRYPT_OK;
}

#endif
