/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://math.libtomcrypt.org
 */
#include <tommath.h>

/* Greatest Common Divisor using the binary method */
int mp_gcd (mp_int * a, mp_int * b, mp_int * c)
{
  mp_int  u, v;
  int     k, u_lsb, v_lsb, res;

  /* either zero than gcd is the largest */
  if (mp_iszero (a) == 1 && mp_iszero (b) == 0) {
    return mp_abs (b, c);
  }
  if (mp_iszero (a) == 0 && mp_iszero (b) == 1) {
    return mp_abs (a, c);
  }

  /* optimized.  At this point if a == 0 then
   * b must equal zero too
   */
  if (mp_iszero (a) == 1) {
    mp_zero(c);
    return MP_OKAY;
  }

  /* get copies of a and b we can modify */
  if ((res = mp_init_copy (&u, a)) != MP_OKAY) {
    return res;
  }

  if ((res = mp_init_copy (&v, b)) != MP_OKAY) {
    goto __U;
  }

  /* must be positive for the remainder of the algorithm */
  u.sign = v.sign = MP_ZPOS;

  /* B1.  Find the common power of two for u and v */
  u_lsb = mp_cnt_lsb(&u);
  v_lsb = mp_cnt_lsb(&v);
  k     = MIN(u_lsb, v_lsb);

  if (k > 0) {
     /* divide the power of two out */
     if ((res = mp_div_2d(&u, k, &u, NULL)) != MP_OKAY) {
        goto __V;
     }

     if ((res = mp_div_2d(&v, k, &v, NULL)) != MP_OKAY) {
        goto __V;
     }
  }

  /* divide any remaining factors of two out */
  if (u_lsb != k) {
     if ((res = mp_div_2d(&u, u_lsb - k, &u, NULL)) != MP_OKAY) {
        goto __V;
     }
  }

  if (v_lsb != k) {
     if ((res = mp_div_2d(&v, v_lsb - k, &v, NULL)) != MP_OKAY) {
        goto __V;
     }
  }

  while (mp_iszero(&v) == 0) {
     /* make sure v is the largest */
     if (mp_cmp_mag(&u, &v) == MP_GT) {
        /* swap u and v to make sure v is >= u */
        mp_exch(&u, &v);
     }
     
     /* subtract smallest from largest */
     if ((res = s_mp_sub(&v, &u, &v)) != MP_OKAY) {
        goto __V;
     }
     
     /* Divide out all factors of two */
     if ((res = mp_div_2d(&v, mp_cnt_lsb(&v), &v, NULL)) != MP_OKAY) {
        goto __V;
     } 
  } 

  /* multiply by 2**k which we divided out at the beginning */
  if ((res = mp_mul_2d (&u, k, c)) != MP_OKAY) {
     goto __V;
  }
  c->sign = MP_ZPOS;
  res = MP_OKAY;
__V:mp_clear (&u);
__U:mp_clear (&v);
  return res;
}
