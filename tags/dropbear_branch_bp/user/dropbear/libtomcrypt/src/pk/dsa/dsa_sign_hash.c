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
   @file dsa_sign_hash.c
   DSA implementation, sign a hash, Tom St Denis
*/

#ifdef MDSA

/**
  Sign a hash with DSA
  @param in       The hash to sign
  @param inlen    The length of the hash to sign
  @param r        The "r" integer of the signature (caller must initialize with mp_init() first)
  @param s        The "s" integer of the signature (caller must initialize with mp_init() first)
  @param prng     An active PRNG state
  @param wprng    The index of the PRNG desired
  @param key      A private DSA key
  @return CRYPT_OK if successful
*/
int dsa_sign_hash_raw(const unsigned char *in,  unsigned long inlen,
                                   mp_int *r,   mp_int *s,
                               prng_state *prng, int wprng, dsa_key *key)
{
   mp_int         k, kinv, tmp;
   unsigned char *buf;
   int            err;

   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(r   != NULL);
   LTC_ARGCHK(s   != NULL);
   LTC_ARGCHK(key != NULL);

   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }
   if (key->type != PK_PRIVATE) {
      return CRYPT_PK_NOT_PRIVATE;
   }

   /* check group order size  */
   if (key->qord >= MDSA_MAX_GROUP) {
      return CRYPT_INVALID_ARG;
   }

   buf = XMALLOC(MDSA_MAX_GROUP);
   if (buf == NULL) {
      return CRYPT_MEM;
   }

   /* Init our temps */
   if ((err = mp_init_multi(&k, &kinv, &tmp, NULL)) != MP_OKAY)               { goto error; }

retry:

   do {
      /* gen random k */
      if (prng_descriptor[wprng].read(buf, key->qord, prng) != (unsigned long)key->qord) {
         err = CRYPT_ERROR_READPRNG;
         goto LBL_ERR;
      }

      /* read k */
      if ((err = mp_read_unsigned_bin(&k, buf, key->qord)) != MP_OKAY)                { goto error; }

      /* k > 1 ? */
      if (mp_cmp_d(&k, 1) != MP_GT)                                                   { goto retry; }

      /* test gcd */
      if ((err = mp_gcd(&k, &key->q, &tmp)) != MP_OKAY)                               { goto error; }
   } while (mp_cmp_d(&tmp, 1) != MP_EQ);

   /* now find 1/k mod q */
   if ((err = mp_invmod(&k, &key->q, &kinv)) != MP_OKAY)                              { goto error; }

   /* now find r = g^k mod p mod q */
   if ((err = mp_exptmod(&key->g, &k, &key->p, r)) != MP_OKAY)                        { goto error; }
   if ((err = mp_mod(r, &key->q, r)) != MP_OKAY)                                      { goto error; }

   if (mp_iszero(r) == MP_YES)                                                        { goto retry; }

   /* now find s = (in + xr)/k mod q */
   if ((err = mp_read_unsigned_bin(&tmp, (unsigned char *)in, inlen)) != MP_OKAY)     { goto error; }
   if ((err = mp_mul(&key->x, r, s)) != MP_OKAY)                                      { goto error; }
   if ((err = mp_add(s, &tmp, s)) != MP_OKAY)                                         { goto error; }
   if ((err = mp_mulmod(s, &kinv, &key->q, s)) != MP_OKAY)                            { goto error; }

   if (mp_iszero(s) == MP_YES)                                                        { goto retry; }

   err = CRYPT_OK;
   goto LBL_ERR;

error: 
   err = mpi_to_ltc_error(err);
LBL_ERR: 
   mp_clear_multi(&k, &kinv, &tmp, NULL);
#ifdef LTC_CLEAN_STACK
   zeromem(buf, MDSA_MAX_GROUP);
#endif
   XFREE(buf);
   return err;
}

/**
  Sign a hash with DSA
  @param in       The hash to sign
  @param inlen    The length of the hash to sign
  @param out      [out] Where to store the signature
  @param outlen   [in/out] The max size and resulting size of the signature
  @param prng     An active PRNG state
  @param wprng    The index of the PRNG desired
  @param key      A private DSA key
  @return CRYPT_OK if successful
*/
int dsa_sign_hash(const unsigned char *in,  unsigned long inlen,
                        unsigned char *out, unsigned long *outlen,
                        prng_state *prng, int wprng, dsa_key *key)
{
   mp_int        r, s;
   int           err;

   LTC_ARGCHK(in      != NULL);
   LTC_ARGCHK(out     != NULL);
   LTC_ARGCHK(outlen  != NULL);
   LTC_ARGCHK(key     != NULL);

   if (mp_init_multi(&r, &s, NULL) != MP_OKAY) {
      return CRYPT_MEM;
   }

   if ((err = dsa_sign_hash_raw(in, inlen, &r, &s, prng, wprng, key)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = der_encode_sequence_multi(out, outlen, 
                             LTC_ASN1_INTEGER, 1UL, &r, 
                             LTC_ASN1_INTEGER, 1UL, &s, 
                             LTC_ASN1_EOL,     0UL, NULL);

LBL_ERR:
   mp_clear_multi(&r, &s, NULL);
   return err;
}

#endif

/* $Source$ */
/* $Revision$ */
/* $Date$ */
