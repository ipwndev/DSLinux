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

/* determines if mp_reduce_2k can be used */
int mp_reduce_is_2k(mp_int *a)
{
   int ix, iy, iz, iw;
   
   if (a->used == 0) {
      return 0;
   } else if (a->used == 1) {
      return 1;
   } else if (a->used > 1) {
      iy = mp_count_bits(a);
      iz = 1;
      iw = 1;
    
      /* Test every bit from the second digit up, must be 1 */
      for (ix = DIGIT_BIT; ix < iy; ix++) {
          if ((a->dp[iw] & iz) == 0) {
             return 0;
          }
          iz <<= 1;
          if (iz > (int)MP_MASK) {
             ++iw;
             iz = 1;
          }
      }
   }
   return 1;
}

