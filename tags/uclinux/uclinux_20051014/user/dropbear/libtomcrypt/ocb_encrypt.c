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

/* OCB Implementation by Tom St Denis */
#include "mycrypt.h"

#ifdef OCB_MODE

int ocb_encrypt(ocb_state *ocb, const unsigned char *pt, unsigned char *ct)
{
   unsigned char Z[MAXBLOCKSIZE], tmp[MAXBLOCKSIZE];
   int err, x;

   _ARGCHK(ocb != NULL);
   _ARGCHK(pt  != NULL);
   _ARGCHK(ct  != NULL);
   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      return err;
   }
   if (ocb->block_len != cipher_descriptor[ocb->cipher].block_length) {
      return CRYPT_INVALID_ARG;
   }

   /* compute checksum */
   for (x = 0; x < ocb->block_len; x++) {
       ocb->checksum[x] ^= pt[x];
   }

   /* Get Z[i] value */
   ocb_shift_xor(ocb, Z);

   /* xor pt in, encrypt, xor Z out */
   for (x = 0; x < ocb->block_len; x++) {
       tmp[x] = pt[x] ^ Z[x];
   }
   cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, ct, &ocb->key);
   for (x = 0; x < ocb->block_len; x++) {
       ct[x] ^= Z[x];
   }

#ifdef CLEAN_STACK
   zeromem(Z, sizeof(Z));
   zeromem(tmp, sizeof(tmp));
#endif
   return CRYPT_OK;
}

#endif
