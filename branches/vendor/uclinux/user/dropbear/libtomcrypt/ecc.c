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

/* Implements ECC over Z/pZ for curve y^2 = x^3 - 3x + b
 *
 * All curves taken from NIST recommendation paper of July 1999
 * Available at http://csrc.nist.gov/cryptval/dss.htm
 */

#include "mycrypt.h"

#ifdef MECC

/* This holds the key settings.  ***MUST*** be organized by size from smallest to largest. */
static const struct {
   int size;
   char *name, *prime, *B, *order, *Gx, *Gy;
} sets[] = {
#ifdef ECC160
{
   20,
   "ECC-160",
   /* prime */
   "G00000000000000000000000007",
   /* B */
   "1oUV2vOaSlWbxr6",
   /* order */
   "G0000000000004sCQUtDxaqDUN5",
   /* Gx */
   "jpqOf1BHus6Yd/pyhyVpP",
   /* Gy */
   "D/wykuuIFfr+vPyx7kQEPu8MixO",
},
#endif
#ifdef ECC192
{
    24,
   "ECC-192",
   /* prime */
   "/////////////////////l//////////",

   /* B */
   "P2456UMSWESFf+chSYGmIVwutkp1Hhcn",

   /* order */
   "////////////////cTxuDXHhoR6qqYWn",

   /* Gx */
   "68se3h0maFPylo3hGw680FJ/2ls2/n0I",

   /* Gy */
   "1nahbV/8sdXZ417jQoJDrNFvTw4UUKWH"
},
#endif
#ifdef ECC224
{
   28,
   "ECC-224",

   /* prime */
   "400000000000000000000000000000000000BV",

   /* B */
   "21HkWGL2CxJIp",

   /* order */
   "4000000000000000000Kxnixk9t8MLzMiV264/",

   /* Gx */
   "jpqOf1BHus6Yd/pyhyVpP",

   /* Gy */
   "3FCtyo2yHA5SFjkCGbYxbOvNeChwS+j6wSIwck",
},
#endif
#ifdef ECC256
{
   32,
   "ECC-256",
   /* Prime */
   "F////y000010000000000000000////////////////",

   /* B */
   "5h6DTYgEfFdi+kzLNQOXhnb7GQmp5EmzZlEF3udqc1B",

   /* Order */
   "F////y00000//////////+yvlgjfnUUXFEvoiByOoLH",

   /* Gx */
   "6iNqVBXB497+BpcvMEaGF9t0ts1BUipeFIXEKNOcCAM",

   /* Gy */
   "4/ZGkB+6d+RZkVhIdmFdXOhpZDNQp5UpiksG6Wtlr7r"
},
#endif
#ifdef ECC384
{
   48,
   "ECC-384",
   /* prime */
   "//////////////////////////////////////////x/////00000000003/"
   "////",

   /* B */
   "ip4lf+8+v+IOZWLhu/Wj6HWTd6x+WK4I0nG8Zr0JXrh6LZcDYYxHdIg5oEtJ"
   "x2hl",

   /* Order */
   "////////////////////////////////nsDDWVGtBTzO6WsoIB2dUkpi6MhC"
   "nIbp",

   /* Gx and Gy */
   "geVA8hwB1JUEiSSUyo2jT6uTEsABfvkOMVT1u89KAZXL0l9TlrKfR3fKNZXo"
   "TWgt",

   "DXVUIfOcB6zTdfY/afBSAVZq7RqecXHywTen4xNmkC0AOB7E7Nw1dNf37NoG"
   "wWvV"
},
#endif
#ifdef ECC521
{
   65,
   "ECC-521",
   /* prime */
   "V///////////////////////////////////////////////////////////"
   "///////////////////////////",

   /* B */
   "56LFhbXZXoQ7vAQ8Q2sXK3kejfoMvcp5VEuj8cHZl49uLOPEL7iVfDx5bB0l"
   "JknlmSrSz+8FImqyUz57zHhK3y0",

   /* Order */
   "V//////////////////////////////////////////+b66XuE/BvPhVym1I"
   "FS9fT0xjScuYPn7hhjljnwHE6G9",

   /* Gx and Gy */
   "CQ5ZWQt10JfpPu+osOZbRH2d6I1EGK/jI7uAAzWQqqzkg5BNdVlvrae/Xt19"
   "wB/gDupIBF1XMf2c/b+VZ72vRrc",

   "HWvAMfucZl015oANxGiVHlPcFL4ILURH6WNhxqN9pvcB9VkSfbUz2P0nL2v0"
   "J+j1s4rF726edB2G8Y+b7QVqMPG",
},
#endif
{
   0,
   NULL, NULL, NULL, NULL, NULL, NULL
}
};

#if 0

/* you plug in a prime and B value and it finds a pseudo-random base point */
void ecc_find_base(void)
{
   static char *prime = "26959946667150639794667015087019630673637144422540572481103610249951";
   static char *order = "26959946667150639794667015087019637467111563745054605861463538557247";
   static char *b     = "9538957348957353489587";
   mp_int pp, p, r, B, tmp1, tmp2, tx, ty, x, y;
   char buf[4096];
   int i;

   mp_init_multi(&tx, &ty, &x, &y, &p, &pp, &r, &B, &tmp1, &tmp2, NULL);
   mp_read_radix(&p, prime, 10);
   mp_read_radix(&r, order, 10);
   mp_read_radix(&B, b, 10);

   /* get (p+1)/4 */
   mp_add_d(&p, 1, &pp);
   mp_div_2(&pp, &pp);
   mp_div_2(&pp, &pp);

   buf[0] = 0;
   do {
      printf("."); fflush(stdout);
      /* make a random value of x */
      for (i = 0; i < 16; i++) buf[i+1] = rand() & 255;
      mp_read_raw(&x, buf, 17);
      mp_copy(&x, &tx);

      /* now compute x^3 - 3x + b */
      mp_expt_d(&x, 3, &tmp1);
      mp_mul_d(&x, 3, &tmp2);
      mp_sub(&tmp1, &tmp2, &tmp1);
      mp_add(&tmp1, &B, &tmp1);
      mp_mod(&tmp1, &p, &tmp1);

      /* now compute sqrt via x^((p+1)/4) */
      mp_exptmod(&tmp1, &pp, &p, &tmp2);
      mp_copy(&tmp2, &ty);

      /* now square it */
      mp_sqrmod(&tmp2, &p, &tmp2);

      /* tmp2 should equal tmp1 */
   } while (mp_cmp(&tmp1, &tmp2));

   /* now output values in way that libtomcrypt wants */
   mp_todecimal(&p, buf);
   printf("\n\np==%s\n", buf);
   mp_tohex(&B, buf);
   printf("b==%s\n", buf);
   mp_todecimal(&r, buf);
   printf("r==%s\n", buf);
   mp_tohex(&tx, buf);
   printf("Gx==%s\n", buf);
   mp_tohex(&ty, buf);
   printf("Gy==%s\n", buf);

   mp_clear_multi(&tx, &ty, &x, &y, &p, &pp, &r, &B, &tmp1, &tmp2, NULL);
}
 
#endif




static int is_valid_idx(int n)
{
   int x;

   for (x = 0; sets[x].size != 0; x++);
   if ((n < 0) || (n >= x)) {
      return 0;
   }
   return 1;
}

static ecc_point *new_point(void)
{
   ecc_point *p;
   p = XMALLOC(sizeof(ecc_point));
   if (p == NULL) {
      return NULL;
   }
   if (mp_init_multi(&p->x, &p->y, NULL) != MP_OKAY) {
      XFREE(p);
      return NULL;
   }
   return p;
}

static void del_point(ecc_point *p)
{
   /* prevents free'ing null arguments */
   if (p != NULL) {
      mp_clear_multi(&p->x, &p->y, NULL);
      XFREE(p);
   }
}

/* double a point R = 2P, R can be P*/
static int dbl_point(ecc_point *P, ecc_point *R, mp_int *modulus, mp_int *mu)
{
   mp_int s, tmp, tmpx;
   int err;

   if ((err = mp_init_multi(&s, &tmp, &tmpx, NULL)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
   }

   /* s = (3Xp^2 + a) / (2Yp) */
   if ((err = mp_mul_2(&P->y, &tmp)) != MP_OKAY)                   { goto error; } /* tmp = 2*y */
   if ((err = mp_invmod(&tmp, modulus, &tmp)) != MP_OKAY)          { goto error; } /* tmp = 1/tmp mod modulus */
   if ((err = mp_sqr(&P->x, &s)) != MP_OKAY)                       { goto error; } /* s = x^2  */
   if ((err = mp_reduce(&s, modulus, mu)) != MP_OKAY)              { goto error; }
   if ((err = mp_mul_d(&s,(mp_digit)3, &s)) != MP_OKAY)            { goto error; } /* s = 3*(x^2) */
   if ((err = mp_sub_d(&s,(mp_digit)3, &s)) != MP_OKAY)            { goto error; } /* s = 3*(x^2) - 3 */
   if (mp_cmp_d(&s, 0) == MP_LT) {                                         /* if s < 0 add modulus */
      if ((err = mp_add(&s, modulus, &s)) != MP_OKAY)              { goto error; }
   }
   if ((err = mp_mul(&s, &tmp, &s)) != MP_OKAY)                    { goto error; } /* s = tmp * s mod modulus */
   if ((err = mp_reduce(&s, modulus, mu)) != MP_OKAY)              { goto error; }

   /* Xr = s^2 - 2Xp */
   if ((err = mp_sqr(&s,  &tmpx)) != MP_OKAY)                      { goto error; } /* tmpx = s^2  */
   if ((err = mp_reduce(&tmpx, modulus, mu)) != MP_OKAY)           { goto error; } /* tmpx = tmpx mod modulus */
   if ((err = mp_sub(&tmpx, &P->x, &tmpx)) != MP_OKAY)             { goto error; } /* tmpx = tmpx - x */
   if ((err = mp_submod(&tmpx, &P->x, modulus, &tmpx)) != MP_OKAY) { goto error; } /* tmpx = tmpx - x mod modulus */

   /* Yr = -Yp + s(Xp - Xr)  */
   if ((err = mp_sub(&P->x, &tmpx, &tmp)) != MP_OKAY)              { goto error; } /* tmp = x - tmpx */
   if ((err = mp_mul(&tmp, &s, &tmp)) != MP_OKAY)                  { goto error; } /* tmp = tmp * s */
   if ((err = mp_submod(&tmp, &P->y, modulus, &R->y)) != MP_OKAY)  { goto error; } /* y = tmp - y mod modulus */
   if ((err = mp_copy(&tmpx, &R->x)) != MP_OKAY)                   { goto error; } /* x = tmpx */

   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   mp_clear_multi(&tmpx, &tmp, &s, NULL);
   return err;
}

/* add two different points over Z/pZ, R = P + Q, note R can equal either P or Q */
static int add_point(ecc_point *P, ecc_point *Q, ecc_point *R, mp_int *modulus, mp_int *mu)
{
   mp_int s, tmp, tmpx;
   int err;

   if ((err = mp_init(&tmp)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
   }

   /* is P==Q or P==-Q? */
   if (((err = mp_neg(&Q->y, &tmp)) != MP_OKAY) || ((err = mp_mod(&tmp, modulus, &tmp)) != MP_OKAY)) {
      mp_clear(&tmp);
      return mpi_to_ltc_error(err);
   }

   if (mp_cmp(&P->x, &Q->x) == MP_EQ)
      if (mp_cmp(&P->y, &Q->y) == MP_EQ || mp_cmp(&P->y, &tmp) == MP_EQ) {
         mp_clear(&tmp);
         return dbl_point(P, R, modulus, mu);
      }

   if ((err = mp_init_multi(&tmpx, &s, NULL)) != MP_OKAY) {
      mp_clear(&tmp);
      return mpi_to_ltc_error(err);
   }

   /* get s = (Yp - Yq)/(Xp-Xq) mod p */
   if ((err = mp_sub(&P->x, &Q->x, &tmp)) != MP_OKAY)                 { goto error; } /* tmp = Px - Qx mod modulus */
   if (mp_cmp_d(&tmp, 0) == MP_LT) {                                          /* if tmp<0 add modulus */
      if ((err = mp_add(&tmp, modulus, &tmp)) != MP_OKAY)             { goto error; }
   }
   if ((err = mp_invmod(&tmp, modulus, &tmp)) != MP_OKAY)             { goto error; } /* tmp = 1/tmp mod modulus */
   if ((err = mp_sub(&P->y, &Q->y, &s)) != MP_OKAY)                   { goto error; } /* s = Py - Qy mod modulus */
   if (mp_cmp_d(&s, 0) == MP_LT) {                                            /* if s<0 add modulus */
      if ((err = mp_add(&s, modulus, &s)) != MP_OKAY)                 { goto error; }
   }
   if ((err = mp_mul(&s, &tmp, &s)) != MP_OKAY)                       { goto error; } /* s = s * tmp mod modulus */
   if ((err = mp_reduce(&s, modulus, mu)) != MP_OKAY)                 { goto error; }

   /* Xr = s^2 - Xp - Xq */
   if ((err = mp_sqr(&s, &tmp)) != MP_OKAY)                           { goto error; } /* tmp = s^2 mod modulus */
   if ((err = mp_reduce(&tmp, modulus, mu)) != MP_OKAY)               { goto error; }
   if ((err = mp_sub(&tmp, &P->x, &tmp)) != MP_OKAY)                  { goto error; } /* tmp = tmp - Px */
   if ((err = mp_sub(&tmp, &Q->x, &tmpx)) != MP_OKAY)                 { goto error; } /* tmpx = tmp - Qx */

   /* Yr = -Yp + s(Xp - Xr) */
   if ((err = mp_sub(&P->x, &tmpx, &tmp)) != MP_OKAY)                 { goto error; } /* tmp = Px - tmpx */
   if ((err = mp_mul(&tmp, &s, &tmp)) != MP_OKAY)                     { goto error; } /* tmp = tmp * s */
   if ((err = mp_submod(&tmp, &P->y, modulus, &R->y)) != MP_OKAY)     { goto error; } /* Ry = tmp - Py mod modulus */
   if ((err = mp_mod(&tmpx, modulus, &R->x)) != MP_OKAY)              { goto error; } /* Rx = tmpx mod modulus */

   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   mp_clear_multi(&s, &tmpx, &tmp, NULL);
   return err;
}

/* size of sliding window, don't change this! */
#define WINSIZE 4

/* perform R = kG where k == integer and G == ecc_point */
static int ecc_mulmod(mp_int *k, ecc_point *G, ecc_point *R, mp_int *modulus)
{
   ecc_point *tG, *M[8];
   int i, j, err;
   mp_int mu;
   mp_digit buf;
   int     first, bitbuf, bitcpy, bitcnt, mode, digidx;

  /* init barrett reduction */
  if ((err = mp_init(&mu)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
  }
  if ((err = mp_reduce_setup(&mu, modulus)) != MP_OKAY) {
      mp_clear(&mu);
      return mpi_to_ltc_error(err);
  }

  /* alloc ram for window temps */
  for (i = 0; i < 8; i++) {
      M[i] = new_point();
      if (M[i] == NULL) {
         for (j = 0; j < i; j++) {
             del_point(M[j]);
         }
         mp_clear(&mu);
         return CRYPT_MEM;
      }
  }

   /* make a copy of G incase R==G */
   tG = new_point();
   if (tG == NULL)                                                            { err = CRYPT_MEM; goto done; }

   /* tG = G */
   if ((err = mp_copy(&G->x, &tG->x)) != MP_OKAY)                             { goto error; }
   if ((err = mp_copy(&G->y, &tG->y)) != MP_OKAY)                             { goto error; }
   
   /* calc the M tab, which holds kG for k==8..15 */
   /* M[0] == 8G */
   if ((err = dbl_point(G, M[0], modulus, &mu)) != CRYPT_OK)                  { goto done; }
   if ((err = dbl_point(M[0], M[0], modulus, &mu)) != CRYPT_OK)               { goto done; }
   if ((err = dbl_point(M[0], M[0], modulus, &mu)) != CRYPT_OK)               { goto done; }

   /* now find (8+k)G for k=1..7 */
   for (j = 9; j < 16; j++) {
       if ((err = add_point(M[j-9], G, M[j-8], modulus, &mu)) != CRYPT_OK)    { goto done; }
   }

   /* setup sliding window */
   mode   = 0;
   bitcnt = 1;
   buf    = 0;
   digidx = k->used - 1;
   bitcpy = bitbuf = 0;
   first  = 1;

   /* perform ops */
   for (;;) {
     /* grab next digit as required */
     if (--bitcnt == 0) {
       if (digidx == -1) {
          break;
       }
       buf = k->dp[digidx--];
       bitcnt = (int) DIGIT_BIT;
     }

     /* grab the next msb from the multiplicand */
     i = (buf >> (DIGIT_BIT - 1)) & 1;
     buf <<= 1;

     /* skip leading zero bits */
     if (mode == 0 && i == 0) {
        continue;
     }

     /* if the bit is zero and mode == 1 then we double */
     if (mode == 1 && i == 0) {
        if ((err = dbl_point(R, R, modulus, &mu)) != CRYPT_OK)                { goto done; }
        continue;
     }

     /* else we add it to the window */
     bitbuf |= (i << (WINSIZE - ++bitcpy));
     mode = 2;

     if (bitcpy == WINSIZE) {
       /* if this is the first window we do a simple copy */
       if (first == 1) {
          /* R = kG [k = first window] */
          if ((err = mp_copy(&M[bitbuf-8]->x, &R->x)) != MP_OKAY)             { goto error; }
          if ((err = mp_copy(&M[bitbuf-8]->y, &R->y)) != MP_OKAY)             { goto error; }
          first = 0;
       } else {
         /* normal window */
         /* ok window is filled so double as required and add  */
         /* double first */
         for (j = 0; j < WINSIZE; j++) {
           if ((err = dbl_point(R, R, modulus, &mu)) != CRYPT_OK)             { goto done; }
         }

         /* then add, bitbuf will be 8..15 [8..2^WINSIZE] guaranteed */
         if ((err = add_point(R, M[bitbuf-8], R, modulus, &mu)) != CRYPT_OK)  { goto done; }
       }
       /* empty window and reset */
       bitcpy = bitbuf = 0;
       mode = 1;
    }
  }

   /* if bits remain then double/add */
   if (mode == 2 && bitcpy > 0) {
     /* double then add */
     for (j = 0; j < bitcpy; j++) {
       /* only double if we have had at least one add first */
       if (first == 0) {
          if ((err = dbl_point(R, R, modulus, &mu)) != CRYPT_OK)               { goto done; }
       }

       bitbuf <<= 1;
       if ((bitbuf & (1 << WINSIZE)) != 0) {
         if (first == 1){
            /* first add, so copy */
            if ((err = mp_copy(&tG->x, &R->x)) != MP_OKAY)                     { goto error; }
            if ((err = mp_copy(&tG->y, &R->y)) != MP_OKAY)                     { goto error; }
            first = 0;
         } else {
            /* then add */
            if ((err = add_point(R, tG, R, modulus, &mu)) != CRYPT_OK)         { goto done; }
         }
       }
     }
   }
   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   del_point(tG);
   for (i = 0; i < 8; i++) {
       del_point(M[i]);
   }
   mp_clear(&mu);
   return err;
}

#undef WINSIZE

int ecc_test(void)
{
   mp_int     modulus, order;
   ecc_point  *G, *GG;
   int i, err, primality;

   if ((err = mp_init_multi(&modulus, &order, NULL)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
   }

   G   = new_point();
   GG  = new_point();
   if (G == NULL || GG == NULL) {
      mp_clear_multi(&modulus, &order, NULL);
      del_point(G);
      del_point(GG);
      return CRYPT_MEM;
   }

   for (i = 0; sets[i].size; i++) {
       #if 0
          printf("Testing %d\n", sets[i].size);
       #endif
       if ((err = mp_read_radix(&modulus, (char *)sets[i].prime, 64)) != MP_OKAY)   { goto error; }
       if ((err = mp_read_radix(&order, (char *)sets[i].order, 64)) != MP_OKAY)     { goto error; }

       /* is prime actually prime? */
       if ((err = is_prime(&modulus, &primality)) != CRYPT_OK)                      { goto done; }
       if (primality == 0) {
          err = CRYPT_FAIL_TESTVECTOR;
          goto done;
       }

       /* is order prime ? */
       if ((err = is_prime(&order, &primality)) != CRYPT_OK)                        { goto done; }
       if (primality == 0) {
          err = CRYPT_FAIL_TESTVECTOR;
          goto done;
       }

       if ((err = mp_read_radix(&G->x, (char *)sets[i].Gx, 64)) != MP_OKAY)         { goto error; }
       if ((err = mp_read_radix(&G->y, (char *)sets[i].Gy, 64)) != MP_OKAY)         { goto error; }

       /* then we should have G == (order + 1)G */
       if ((err = mp_add_d(&order, 1, &order)) != MP_OKAY)                          { goto error; }
       if ((err = ecc_mulmod(&order, G, GG, &modulus)) != CRYPT_OK)                 { goto done; }
       if (mp_cmp(&G->x, &GG->x) != 0 || mp_cmp(&G->y, &GG->y) != 0) {
          err = CRYPT_FAIL_TESTVECTOR;
          goto done;
       }
   }
   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   del_point(GG);
   del_point(G);
   mp_clear_multi(&order, &modulus, NULL);
   return err;
}

void ecc_sizes(int *low, int *high)
{
 int i;
 _ARGCHK(low  != NULL);
 _ARGCHK(high != NULL);

 *low = INT_MAX;
 *high = 0;
 for (i = 0; sets[i].size != 0; i++) {
     if (sets[i].size < *low)  {
        *low  = sets[i].size;
     }
     if (sets[i].size > *high) {
        *high = sets[i].size;
     }
 }
}

int ecc_make_key(prng_state *prng, int wprng, int keysize, ecc_key *key)
{
   int x, err;
   ecc_point *base;
   mp_int prime;
   unsigned char buf[128];

   _ARGCHK(key != NULL);

   /* good prng? */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   /* find key size */
   for (x = 0; (keysize > sets[x].size) && (sets[x].size != 0); x++);
   keysize = sets[x].size;

   if (sets[x].size == 0) {
      return CRYPT_INVALID_KEYSIZE;
   }
   key->idx = x;

   /* make up random string */
   if (prng_descriptor[wprng].read(buf, (unsigned long)keysize, prng) != (unsigned long)keysize) {
      return CRYPT_ERROR_READPRNG;
   }

   /* setup the key variables */
   if ((err = mp_init_multi(&key->pubkey.x, &key->pubkey.y, &key->k, &prime, NULL)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
   }
   base = new_point();
   if (base == NULL) {
      mp_clear_multi(&key->pubkey.x, &key->pubkey.y, &key->k, &prime, NULL);
      return CRYPT_MEM;
   }

   /* read in the specs for this key */
   if ((err = mp_read_radix(&prime, (char *)sets[key->idx].prime, 64)) != MP_OKAY)      { goto error; }
   if ((err = mp_read_radix(&base->x, (char *)sets[key->idx].Gx, 64)) != MP_OKAY)       { goto error; }
   if ((err = mp_read_radix(&base->y, (char *)sets[key->idx].Gy, 64)) != MP_OKAY)       { goto error; }
   if ((err = mp_read_unsigned_bin(&key->k, (unsigned char *)buf, keysize)) != MP_OKAY) { goto error; }

   /* make the public key */
   if ((err = ecc_mulmod(&key->k, base, &key->pubkey, &prime)) != CRYPT_OK)             { goto done; }
   key->type = PK_PRIVATE;

   /* shrink key */
   if ((err = mp_shrink(&key->k)) != MP_OKAY)                                           { goto error; }
   if ((err = mp_shrink(&key->pubkey.x)) != MP_OKAY)                                    { goto error; }
   if ((err = mp_shrink(&key->pubkey.y)) != MP_OKAY)                                    { goto error; }

   /* free up ram */
   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   del_point(base);
   mp_clear(&prime);
#ifdef CLEAN_STACK
   zeromem(buf, sizeof(buf));
#endif
   return err;
}

void ecc_free(ecc_key *key)
{
   _ARGCHK(key != NULL);
   mp_clear_multi(&key->pubkey.x, &key->pubkey.y, &key->k, NULL);
}

static int compress_y_point(ecc_point *pt, int idx, int *result)
{
   mp_int tmp, tmp2, p;
   int err;

   _ARGCHK(pt     != NULL);
   _ARGCHK(result != NULL);

   if ((err = mp_init_multi(&tmp, &tmp2, &p, NULL)) != MP_OKAY) {
      return mpi_to_ltc_error(err);
   }

   /* get x^3 - 3x + b */
   if ((err = mp_read_radix(&p, (char *)sets[idx].B, 64)) != MP_OKAY) { goto error; } /* p = B */
   if ((err = mp_expt_d(&pt->x, 3, &tmp)) != MP_OKAY)                 { goto error; } /* tmp = pX^3  */
   if ((err = mp_mul_d(&pt->x, 3, &tmp2)) != MP_OKAY)                 { goto error; } /* tmp2 = 3*pX^3 */
   if ((err = mp_sub(&tmp, &tmp2, &tmp)) != MP_OKAY)                  { goto error; } /* tmp = tmp - tmp2 */
   if ((err = mp_add(&tmp, &p, &tmp)) != MP_OKAY)                     { goto error; } /* tmp = tmp + p */
   if ((err = mp_read_radix(&p, (char *)sets[idx].prime, 64)) != MP_OKAY)  { goto error; } /* p = prime */
   if ((err = mp_mod(&tmp, &p, &tmp)) != MP_OKAY)                     { goto error; } /* tmp = tmp mod p */

   /* now find square root */
   if ((err = mp_add_d(&p, 1, &tmp2)) != MP_OKAY)                     { goto error; } /* tmp2 = p + 1 */
   if ((err = mp_div_2d(&tmp2, 2, &tmp2, NULL)) != MP_OKAY)           { goto error; } /* tmp2 = (p+1)/4 */
   if ((err = mp_exptmod(&tmp, &tmp2, &p, &tmp)) != MP_OKAY)          { goto error; } /* tmp  = (x^3 - 3x + b)^((p+1)/4) mod p */

   /* if tmp equals the y point give a 0, otherwise 1 */
   if (mp_cmp(&tmp, &pt->y) == 0) {
      *result = 0;
   } else {
      *result = 1;
   }

   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   mp_clear_multi(&p, &tmp, &tmp2, NULL);
   return err;
}

static int expand_y_point(ecc_point *pt, int idx, int result)
{
   mp_int tmp, tmp2, p;
   int err;

   _ARGCHK(pt != NULL);

   if ((err = mp_init_multi(&tmp, &tmp2, &p, NULL)) != MP_OKAY) {
      return CRYPT_MEM;
   }

   /* get x^3 - 3x + b */
   if ((err = mp_read_radix(&p, (char *)sets[idx].B, 64)) != MP_OKAY) { goto error; } /* p = B */
   if ((err = mp_expt_d(&pt->x, 3, &tmp)) != MP_OKAY)                 { goto error; } /* tmp = pX^3 */
   if ((err = mp_mul_d(&pt->x, 3, &tmp2)) != MP_OKAY)                 { goto error; } /* tmp2 = 3*pX^3 */
   if ((err = mp_sub(&tmp, &tmp2, &tmp)) != MP_OKAY)                  { goto error; } /* tmp = tmp - tmp2 */
   if ((err = mp_add(&tmp, &p, &tmp)) != MP_OKAY)                     { goto error; } /* tmp = tmp + p */
   if ((err = mp_read_radix(&p, (char *)sets[idx].prime, 64)) != MP_OKAY)  { goto error; } /* p = prime */
   if ((err = mp_mod(&tmp, &p, &tmp)) != MP_OKAY)                     { goto error; } /* tmp = tmp mod p */

   /* now find square root */
   if ((err = mp_add_d(&p, 1, &tmp2)) != MP_OKAY)                     { goto error; } /* tmp2 = p + 1 */
   if ((err = mp_div_2d(&tmp2, 2, &tmp2, NULL)) != MP_OKAY)           { goto error; } /* tmp2 = (p+1)/4 */
   if ((err = mp_exptmod(&tmp, &tmp2, &p, &tmp)) != MP_OKAY)          { goto error; } /* tmp  = (x^3 - 3x + b)^((p+1)/4) mod p */

   /* if result==0, then y==tmp, otherwise y==p-tmp */
   if (result == 0) {
      if ((err = mp_copy(&tmp, &pt->y) != MP_OKAY))                   { goto error; }
   } else {
      if ((err = mp_sub(&p, &tmp, &pt->y) != MP_OKAY))                { goto error; }
   }

   err = CRYPT_OK;
   goto done;
error:
   err = mpi_to_ltc_error(err);
done:
   mp_clear_multi(&p, &tmp, &tmp2, NULL);
   return err;
}

int ecc_export(unsigned char *out, unsigned long *outlen, int type, ecc_key *key)
{
   unsigned long y, z;
   int cp, err;

   _ARGCHK(out    != NULL);
   _ARGCHK(outlen != NULL);
   _ARGCHK(key    != NULL);
   
   /* can we store the static header?  */
   if (*outlen < (PACKET_SIZE + 3)) {
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* type valid? */
   if (key->type != PK_PRIVATE && type == PK_PRIVATE) {
      return CRYPT_PK_TYPE_MISMATCH;
   }

   /* output type and magic byte */
   y = PACKET_SIZE;
   out[y++] = (unsigned char)type;
   out[y++] = (unsigned char)sets[key->idx].size;

   /* output x coordinate */
   OUTPUT_BIGNUM(&(key->pubkey.x), out, y, z);

   /* compress y and output it  */
   if ((err = compress_y_point(&key->pubkey, key->idx, &cp)) != CRYPT_OK) {
      return err;
   }
   out[y++] = (unsigned char)cp;

   if (type == PK_PRIVATE) {
      OUTPUT_BIGNUM(&key->k, out, y, z);
   }

   /* store header */
   packet_store_header(out, PACKET_SECT_ECC, PACKET_SUB_KEY);
   *outlen = y;

   return CRYPT_OK;
}

int ecc_import(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
   unsigned long x, y, s;
   int err;

   _ARGCHK(in  != NULL);
   _ARGCHK(key != NULL);

   /* check length */
   if ((3+PACKET_SIZE) > inlen) {
      return CRYPT_INVALID_PACKET;
   }

   /* check type */
   if ((err = packet_valid_header((unsigned char *)in, PACKET_SECT_ECC, PACKET_SUB_KEY)) != CRYPT_OK) {
      return err;
   }

   /* init key */
   if (mp_init_multi(&key->pubkey.x, &key->pubkey.y, &key->k, NULL) != MP_OKAY) {
      return CRYPT_MEM;
   }

   y = PACKET_SIZE;
   key->type = (int)in[y++];
   s = (unsigned long)in[y++];

   for (x = 0; (s > (unsigned long)sets[x].size) && (sets[x].size != 0); x++);
   if (sets[x].size == 0) {
      err = CRYPT_INVALID_KEYSIZE;
      goto error;
   }
   key->idx = (int)x;

   /* type check both values */
   if ((key->type != PK_PUBLIC) && (key->type != PK_PRIVATE))  {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* is the key idx valid? */
   if (is_valid_idx(key->idx) != 1) {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* load x coordinate */
   INPUT_BIGNUM(&key->pubkey.x, in, x, y, inlen);

   /* load y */
   x = (unsigned long)in[y++];
   if ((err = expand_y_point(&key->pubkey, key->idx, (int)x)) != CRYPT_OK) {
       goto error;
   }

   if (key->type == PK_PRIVATE) {
      /* load private key */
      INPUT_BIGNUM(&key->k, in, x, y, inlen);
   }

   /* eliminate private key if public */
   if (key->type == PK_PUBLIC) {
      mp_clear(&key->k);
   }

   return CRYPT_OK;
error:
   mp_clear_multi(&key->pubkey.x, &key->pubkey.y, &key->k, NULL);
   return err;
}

int ecc_shared_secret(ecc_key *private_key, ecc_key *public_key,
                      unsigned char *out, unsigned long *outlen)
{
   unsigned long x, y;
   ecc_point *result;
   mp_int prime;
   int err;

   _ARGCHK(private_key != NULL);
   _ARGCHK(public_key  != NULL);
   _ARGCHK(out         != NULL);
   _ARGCHK(outlen      != NULL);

   /* type valid? */
   if (private_key->type != PK_PRIVATE) {
      return CRYPT_PK_NOT_PRIVATE;
   }

   if (private_key->idx != public_key->idx) {
      return CRYPT_PK_TYPE_MISMATCH;
   }

   /* make new point */
   result = new_point();
   if (result == NULL) {
      return CRYPT_MEM;
   }

   if ((err = mp_init(&prime)) != MP_OKAY) {
      del_point(result);
      return mpi_to_ltc_error(err);
   }

   if ((err = mp_read_radix(&prime, (char *)sets[private_key->idx].prime, 64)) != MP_OKAY)   { goto error; }
   if ((err = ecc_mulmod(&private_key->k, &public_key->pubkey, result, &prime)) != CRYPT_OK) { goto done1; }

   x = (unsigned long)mp_unsigned_bin_size(&result->x);
   y = (unsigned long)mp_unsigned_bin_size(&result->y);

   if (*outlen < (x+y)) {
      err = CRYPT_BUFFER_OVERFLOW;
      goto done1;
   }
   *outlen = x+y;
   if ((err = mp_to_unsigned_bin(&result->x, out))   != MP_OKAY)          { goto error; }
   if ((err = mp_to_unsigned_bin(&result->y, out+x)) != MP_OKAY)          { goto error; }

   err = CRYPT_OK;
   goto done1;
error:
   err = mpi_to_ltc_error(err);
done1:
   mp_clear(&prime);
   del_point(result);
   return err;
}

int ecc_get_size(ecc_key *key)
{
   _ARGCHK(key != NULL);
   if (is_valid_idx(key->idx))
      return sets[key->idx].size;
   else
      return INT_MAX; /* large value known to cause it to fail when passed to ecc_make_key() */
}

#include "ecc_sys.c"

#endif


