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

int eax_done(eax_state *eax, unsigned char *tag, unsigned long *taglen)
{
   int           err;
   unsigned char headermac[MAXBLOCKSIZE], ctmac[MAXBLOCKSIZE];
   unsigned long x, len;

   _ARGCHK(eax    != NULL);
   _ARGCHK(tag    != NULL);
   _ARGCHK(taglen != NULL);

   /* finish ctomac */
   len = sizeof(ctmac);
   if ((err = omac_done(&eax->ctomac, ctmac, &len)) != CRYPT_OK) {
      return err;
   }

   /* finish headeromac */

   /* note we specifically don't reset len so the two lens are minimal */

   if ((err = omac_done(&eax->headeromac, headermac, &len)) != CRYPT_OK) {
      return err;
   }

   /* compute N xor H xor C */
   for (x = 0; x < len && x < *taglen; x++) {
       tag[x] = eax->N[x] ^ headermac[x] ^ ctmac[x];
   }
   *taglen = x;

#ifdef CLEAN_STACK
   zeromem(ctmac, sizeof(ctmac));
   zeromem(headermac, sizeof(headermac));
   zeromem(eax, sizeof(*eax));
#endif

   return CRYPT_OK;
}

#endif
