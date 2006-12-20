/* mpn_sqrtrem (root_ptr, rem_ptr, op_ptr, op_size)

   Write the square root of {OP_PTR, OP_SIZE} at ROOT_PTR.
   Write the remainder at REM_PTR, if REM_PTR != NULL.
   Return the size of the remainder.
   (The size of the root is always half of the size of the operand.)

   OP_PTR and ROOT_PTR may not point to the same object.
   OP_PTR and REM_PTR may point to the same object.

   If REM_PTR is NULL, only the root is computed and the return value of
   the function is 0 if OP is a perfect square, and *any* non-zero number
   otherwise.

Copyright (C) 1993, 1994, 1996, 1997, 1998, 1999 Free Software Foundation,
Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

/* This code is just correct if "unsigned char" has at least 8 bits.  It
   doesn't help to use CHAR_BIT from limits.h, as the real problem is
   the static arrays.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

/* Square root algorithm:

   1. Shift OP (the input) to the left an even number of bits s.t. there
      are an even number of words and either (or both) of the most
      significant bits are set.  This way, sqrt(OP) has exactly half as
      many words as OP, and has its most significant bit set.

   2. Get a 9-bit approximation to sqrt(OP) using the pre-computed tables.
      This approximation is used for the first single-precision
      iterations of Newton's method, yielding a full-word approximation
      to sqrt(OP).

   3. Perform multiple-precision Newton iteration until we have the
      exact result.  Only about half of the input operand is used in
      this calculation, as the square root is perfectly determinable
      from just the higher half of a number.  */

/* Define this macro for IEEE P854 machines with a fast sqrt instruction.  */
#if defined __GNUC__ && ! defined __SOFT_FLOAT

#if defined (__sparc__) && BITS_PER_MP_LIMB == 32
#define SQRT(a) \
  ({									\
    double __sqrt_res;							\
    asm ("fsqrtd %1,%0" : "=f" (__sqrt_res) : "f" (a));			\
    __sqrt_res;								\
  })
#endif

#if defined (__HAVE_68881__)
#define SQRT(a) \
  ({									\
    double __sqrt_res;							\
    asm ("fsqrtx %1,%0" : "=f" (__sqrt_res) : "f" (a));			\
    __sqrt_res;								\
  })
#endif

#if defined (__hppa) && BITS_PER_MP_LIMB == 32
#define SQRT(a) \
  ({									\
    double __sqrt_res;							\
    asm ("fsqrt,dbl %1,%0" : "=fx" (__sqrt_res) : "fx" (a));		\
    __sqrt_res;								\
  })
#endif

#if defined (_ARCH_PWR2) && BITS_PER_MP_LIMB == 32
#define SQRT(a) \
  ({									\
    double __sqrt_res;							\
    asm ("fsqrt %0,%1" : "=f" (__sqrt_res) : "f" (a));			\
    __sqrt_res;								\
  })
#endif

#if 0
#if defined (__i386__) || defined (__i486__)
#define SQRT(a) \
  ({									\
    double __sqrt_res;							\
    asm ("fsqrt" : "=t" (__sqrt_res) : "0" (a));			\
    __sqrt_res;								\
  })
#endif
#endif

#endif

#ifndef SQRT

/* Tables for initial approximation of the square root.  These are
   indexed with bits 1-8 of the operand for which the square root is
   calculated, where bit 0 is the most significant non-zero bit.  I.e.
   the most significant one-bit is not used, since that per definition
   is one.  Likewise, the tables don't return the highest bit of the
   result.  That bit must be inserted by or:ing the returned value with
   0x100.  This way, we get a 9-bit approximation from 8-bit tables!  */

/* Table to be used for operands with an even total number of bits.
   (Exactly as in the decimal system there are similarities between the
   square root of numbers with the same initial digits and an even
   difference in the total number of digits.  Consider the square root
   of 1, 10, 100, 1000, ...)  */
static const unsigned char even_approx_tab[256] =
{
  0x6a, 0x6a, 0x6b, 0x6c, 0x6c, 0x6d, 0x6e, 0x6e,
  0x6f, 0x70, 0x71, 0x71, 0x72, 0x73, 0x73, 0x74,
  0x75, 0x75, 0x76, 0x77, 0x77, 0x78, 0x79, 0x79,
  0x7a, 0x7b, 0x7b, 0x7c, 0x7d, 0x7d, 0x7e, 0x7f,
  0x80, 0x80, 0x81, 0x81, 0x82, 0x83, 0x83, 0x84,
  0x85, 0x85, 0x86, 0x87, 0x87, 0x88, 0x89, 0x89,
  0x8a, 0x8b, 0x8b, 0x8c, 0x8d, 0x8d, 0x8e, 0x8f,
  0x8f, 0x90, 0x90, 0x91, 0x92, 0x92, 0x93, 0x94,
  0x94, 0x95, 0x96, 0x96, 0x97, 0x97, 0x98, 0x99,
  0x99, 0x9a, 0x9b, 0x9b, 0x9c, 0x9c, 0x9d, 0x9e,
  0x9e, 0x9f, 0xa0, 0xa0, 0xa1, 0xa1, 0xa2, 0xa3,
  0xa3, 0xa4, 0xa4, 0xa5, 0xa6, 0xa6, 0xa7, 0xa7,
  0xa8, 0xa9, 0xa9, 0xaa, 0xaa, 0xab, 0xac, 0xac,
  0xad, 0xad, 0xae, 0xaf, 0xaf, 0xb0, 0xb0, 0xb1,
  0xb2, 0xb2, 0xb3, 0xb3, 0xb4, 0xb5, 0xb5, 0xb6,
  0xb6, 0xb7, 0xb7, 0xb8, 0xb9, 0xb9, 0xba, 0xba,
  0xbb, 0xbb, 0xbc, 0xbd, 0xbd, 0xbe, 0xbe, 0xbf,
  0xc0, 0xc0, 0xc1, 0xc1, 0xc2, 0xc2, 0xc3, 0xc3,
  0xc4, 0xc5, 0xc5, 0xc6, 0xc6, 0xc7, 0xc7, 0xc8,
  0xc9, 0xc9, 0xca, 0xca, 0xcb, 0xcb, 0xcc, 0xcc,
  0xcd, 0xce, 0xce, 0xcf, 0xcf, 0xd0, 0xd0, 0xd1,
  0xd1, 0xd2, 0xd3, 0xd3, 0xd4, 0xd4, 0xd5, 0xd5,
  0xd6, 0xd6, 0xd7, 0xd7, 0xd8, 0xd9, 0xd9, 0xda,
  0xda, 0xdb, 0xdb, 0xdc, 0xdc, 0xdd, 0xdd, 0xde,
  0xde, 0xdf, 0xe0, 0xe0, 0xe1, 0xe1, 0xe2, 0xe2,
  0xe3, 0xe3, 0xe4, 0xe4, 0xe5, 0xe5, 0xe6, 0xe6,
  0xe7, 0xe7, 0xe8, 0xe8, 0xe9, 0xea, 0xea, 0xeb,
  0xeb, 0xec, 0xec, 0xed, 0xed, 0xee, 0xee, 0xef,
  0xef, 0xf0, 0xf0, 0xf1, 0xf1, 0xf2, 0xf2, 0xf3,
  0xf3, 0xf4, 0xf4, 0xf5, 0xf5, 0xf6, 0xf6, 0xf7,
  0xf7, 0xf8, 0xf8, 0xf9, 0xf9, 0xfa, 0xfa, 0xfb,
  0xfb, 0xfc, 0xfc, 0xfd, 0xfd, 0xfe, 0xfe, 0xff,
};

/* Table to be used for operands with an odd total number of bits.
   (Further comments before previous table.)  */
static const unsigned char odd_approx_tab[256] =
{
  0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03,
  0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
  0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b,
  0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f,
  0x0f, 0x10, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12,
  0x13, 0x13, 0x14, 0x14, 0x15, 0x15, 0x16, 0x16,
  0x16, 0x17, 0x17, 0x18, 0x18, 0x19, 0x19, 0x1a,
  0x1a, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1d,
  0x1e, 0x1e, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x21,
  0x21, 0x22, 0x22, 0x23, 0x23, 0x23, 0x24, 0x24,
  0x25, 0x25, 0x26, 0x26, 0x27, 0x27, 0x27, 0x28,
  0x28, 0x29, 0x29, 0x2a, 0x2a, 0x2a, 0x2b, 0x2b,
  0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f,
  0x2f, 0x30, 0x30, 0x30, 0x31, 0x31, 0x32, 0x32,
  0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x35,
  0x36, 0x36, 0x37, 0x37, 0x37, 0x38, 0x38, 0x39,
  0x39, 0x39, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c,
  0x3c, 0x3d, 0x3d, 0x3d, 0x3e, 0x3e, 0x3f, 0x3f,
  0x40, 0x40, 0x40, 0x41, 0x41, 0x41, 0x42, 0x42,
  0x43, 0x43, 0x43, 0x44, 0x44, 0x45, 0x45, 0x45,
  0x46, 0x46, 0x47, 0x47, 0x47, 0x48, 0x48, 0x49,
  0x49, 0x49, 0x4a, 0x4a, 0x4b, 0x4b, 0x4b, 0x4c,
  0x4c, 0x4c, 0x4d, 0x4d, 0x4e, 0x4e, 0x4e, 0x4f,
  0x4f, 0x50, 0x50, 0x50, 0x51, 0x51, 0x51, 0x52,
  0x52, 0x53, 0x53, 0x53, 0x54, 0x54, 0x54, 0x55,
  0x55, 0x56, 0x56, 0x56, 0x57, 0x57, 0x57, 0x58,
  0x58, 0x59, 0x59, 0x59, 0x5a, 0x5a, 0x5a, 0x5b,
  0x5b, 0x5b, 0x5c, 0x5c, 0x5d, 0x5d, 0x5d, 0x5e,
  0x5e, 0x5e, 0x5f, 0x5f, 0x60, 0x60, 0x60, 0x61,
  0x61, 0x61, 0x62, 0x62, 0x62, 0x63, 0x63, 0x63,
  0x64, 0x64, 0x65, 0x65, 0x65, 0x66, 0x66, 0x66,
  0x67, 0x67, 0x67, 0x68, 0x68, 0x68, 0x69, 0x69,
};
#endif


mp_size_t
#if __STDC__
mpn_sqrtrem (mp_ptr root_ptr, mp_ptr rem_ptr, mp_srcptr op_ptr, mp_size_t op_size)
#else
mpn_sqrtrem (root_ptr, rem_ptr, op_ptr, op_size)
     mp_ptr root_ptr;
     mp_ptr rem_ptr;
     mp_srcptr op_ptr;
     mp_size_t op_size;
#endif
{
  /* R (root result) */
  mp_ptr rp;			/* Pointer to least significant word */
  mp_size_t rsize;		/* The size in words */

  /* T (OP shifted to the left a.k.a. normalized) */
  mp_ptr tp;			/* Pointer to least significant word */
  mp_size_t tsize;		/* The size in words */
  mp_ptr t_end_ptr;		/* Pointer right beyond most sign. word */
  mp_limb_t t_high0, t_high1;	/* The two most significant words */

  /* TT (temporary for numerator/remainder) */
  mp_ptr ttp;			/* Pointer to least significant word */

  /* X (temporary for quotient in main loop) */
  mp_ptr xp;			/* Pointer to least significant word */
  mp_size_t xsize;		/* The size in words */

  unsigned cnt;
  mp_limb_t initial_approx;	/* Initially made approximation */
  mp_size_t tsizes[BITS_PER_MP_LIMB];	/* Successive calculation precisions */
  mp_size_t tmp;
  mp_size_t i;

  mp_limb_t cy_limb;
  TMP_DECL (marker);

  /* If OP is zero, both results are zero.  */
  if (op_size == 0)
    return 0;

  count_leading_zeros (cnt, op_ptr[op_size - 1]);
  tsize = op_size;
  if ((tsize & 1) != 0)
    {
      cnt += BITS_PER_MP_LIMB;
      tsize++;
    }

  rsize = tsize / 2;
  rp = root_ptr;

  TMP_MARK (marker);

  /* Shift OP an even number of bits into T, such that either the most or
     the second most significant bit is set, and such that the number of
     words in T becomes even.  This way, the number of words in R=sqrt(OP)
     is exactly half as many as in OP, and the most significant bit of R
     is set.

     Also, the initial approximation is simplified by this up-shifted OP.

     Finally, the Newtonian iteration which is the main part of this
     program performs division by R.  The fast division routine expects
     the divisor to be "normalized" in exactly the sense of having the
     most significant bit set.  */

  tp = (mp_ptr) TMP_ALLOC (tsize * BYTES_PER_MP_LIMB);

  if ((cnt & ~1) % BITS_PER_MP_LIMB != 0)
    t_high0 = mpn_lshift (tp + cnt / BITS_PER_MP_LIMB, op_ptr, op_size,
			  (cnt & ~1) % BITS_PER_MP_LIMB);
  else
    MPN_COPY (tp + cnt / BITS_PER_MP_LIMB, op_ptr, op_size);

  if (cnt >= BITS_PER_MP_LIMB)
    tp[0] = 0;

  t_high0 = tp[tsize - 1];
  t_high1 = tp[tsize - 2];	/* Never stray.  TSIZE is >= 2.  */

/* Is there a fast sqrt instruction defined for this machine?  */
#ifdef SQRT
  {
    initial_approx = SQRT (t_high0 * MP_BASE_AS_DOUBLE + t_high1);
    /* If t_high0,,t_high1 is big, the result in INITIAL_APPROX might have
       become incorrect due to overflow in the conversion from double to
       mp_limb_t above.  It will typically be zero in that case, but might be
       a small number on some machines.  The most significant bit of
       INITIAL_APPROX should be set, so that bit is a good overflow
       indication.  */
    if ((mp_limb_signed_t) initial_approx >= 0)
      initial_approx = ~(mp_limb_t)0;
  }
#else
  /* Get a 9 bit approximation from the tables.  The tables expect to
     be indexed with the 8 high bits right below the highest bit.
     Also, the highest result bit is not returned by the tables, and
     must be or:ed into the result.  The scheme gives 9 bits of start
     approximation with just 256-entry 8 bit tables.  */

  if ((cnt & 1) == 0)
    {
      /* The most significant bit of t_high0 is set.  */
      initial_approx = t_high0 >> (BITS_PER_MP_LIMB - 8 - 1);
      initial_approx &= 0xff;
      initial_approx = even_approx_tab[initial_approx];
    }
  else
    {
      /* The most significant bit of t_high0 is unset,
	 the second most significant is set.  */
      initial_approx = t_high0 >> (BITS_PER_MP_LIMB - 8 - 2);
      initial_approx &= 0xff;
      initial_approx = odd_approx_tab[initial_approx];
    }
  initial_approx |= 0x100;
  initial_approx <<= BITS_PER_MP_LIMB - 8 - 1;

  /* Perform small precision Newtonian iterations to get a full word
     approximation.  For small operands, these iterations will do the
     entire job.  */
  if (t_high0 == ~(mp_limb_t)0)
    initial_approx = t_high0;
  else
    {
      mp_limb_t quot;

      if (t_high0 >= initial_approx)
	initial_approx = t_high0 + 1;

      /* First get about 18 bits with pure C arithmetics.  */
      quot = t_high0 / (initial_approx >> BITS_PER_MP_LIMB/2) << BITS_PER_MP_LIMB/2;
      initial_approx = (initial_approx + quot) / 2;
      initial_approx |= (mp_limb_t) 1 << (BITS_PER_MP_LIMB - 1);

      /* Now get a full word by one (or for > 36 bit machines) several
	 iterations.  */
      for (i = 18; i < BITS_PER_MP_LIMB; i <<= 1)
	{
	  mp_limb_t ignored_remainder;

	  udiv_qrnnd (quot, ignored_remainder,
		      t_high0, t_high1, initial_approx);
	  initial_approx = (initial_approx + quot) / 2;
	  initial_approx |= (mp_limb_t) 1 << (BITS_PER_MP_LIMB - 1);
	}
    }
#endif

  rp[0] = initial_approx;
  rsize = 1;

#ifdef DEBUG
	  printf ("\n\nT = ");
	  mpn_dump (tp, tsize);
#endif

  if (tsize > 2)
    {
      /* Determine the successive precisions to use in the iteration.  We
	 minimize the precisions, beginning with the highest (i.e. last
	 iteration) to the lowest (i.e. first iteration).  */

      xp = (mp_ptr) TMP_ALLOC (tsize * BYTES_PER_MP_LIMB);
      ttp = (mp_ptr) TMP_ALLOC (tsize * BYTES_PER_MP_LIMB);

      t_end_ptr = tp + tsize;

      tmp = tsize / 2;
      for (i = 0;; i++)
	{
	  tsize = (tmp + 1) / 2;
	  if (tmp == tsize)
	    break;
	  tsizes[i] = tsize + tmp;
	  tmp = tsize;
	}

      /* Main Newton iteration loop.  For big arguments, most of the
	 time is spent here.  */

      /* It is possible to do a great optimization here.  The successive
	 divisors in the mpn_divmod call below have more and more leading
	 words equal to its predecessor.  Therefore the beginning of
	 each division will repeat the same work as did the last
	 division.  If we could guarantee that the leading words of two
	 consecutive divisors are the same (i.e. in this case, a later
	 divisor has just more digits at the end) it would be a simple
	 matter of just using the old remainder of the last division in
	 a subsequent division, to take care of this optimization.  This
	 idea would surely make a difference even for small arguments.  */

      /* Loop invariants:

	 R <= shiftdown_to_same_size(floor(sqrt(OP))) < R + 1.
	 X - 1 < shiftdown_to_same_size(floor(sqrt(OP))) <= X.
	 R <= shiftdown_to_same_size(X).  */

      while (--i >= 0)
	{
	  mp_limb_t cy;
#ifdef DEBUG
	  mp_limb_t old_least_sign_r = rp[0];
	  mp_size_t old_rsize = rsize;

	  printf ("R = ");
	  mpn_dump (rp, rsize);
#endif
	  tsize = tsizes[i];

	  /* Need to copy the numerator into temporary space, as
	     mpn_divmod overwrites its numerator argument with the
	     remainder (which we currently ignore).  */
	  MPN_COPY (ttp, t_end_ptr - tsize, tsize);
	  cy = mpn_divmod (xp, ttp, tsize, rp, rsize);
	  xsize = tsize - rsize;

#ifdef DEBUG
	  printf ("X =%d ", cy);
	  mpn_dump (xp, xsize);
#endif

	  /* Add X and R with the most significant limbs aligned,
	     temporarily ignoring at least one limb at the low end of X.  */
	  tmp = xsize - rsize;
	  cy += mpn_add_n (xp + tmp, rp, xp + tmp, rsize);

	  /* If T begins with more than 2 x BITS_PER_MP_LIMB of ones, we get
	     intermediate roots that'd need an extra bit.  We don't want to
	     handle that since it would make the subsequent divisor
	     non-normalized, so round such roots down to be only ones in the
	     current precision.  */
	  if (cy == 2)
	    {
	      mp_size_t j;
	      for (j = xsize; j >= 0; j--)
		xp[j] = ~(mp_limb_t)0;
	    }

	  /* Divide X by 2 and put the result in R.  This is the new
	     approximation.  Shift in the carry from the addition.  */
	  mpn_rshift (rp, xp, xsize, 1);
	  rp[xsize - 1] |= ((mp_limb_t) 1 << (BITS_PER_MP_LIMB - 1));
	  rsize = xsize;
#ifdef DEBUG
	  if (old_least_sign_r != rp[rsize - old_rsize])
	    printf (">>>>>>>> %d: %0*lX, %0*lX <<<<<<<<\n",
		    i, 2 * BYTES_PER_MP_LIMB, old_least_sign_r,
		    2 * BYTES_PER_MP_LIMB, rp[rsize - old_rsize]);
#endif
	}
    }

#ifdef DEBUG
  printf ("(final) R = ");
  mpn_dump (rp, rsize);
#endif

  /* We computed the square root of OP * 2**(2*floor(cnt/2)).
     This has resulted in R being 2**floor(cnt/2) to large.
     Shift it down here to fix that.  */
  if (cnt / 2 != 0)
    {
      mpn_rshift (rp, rp, rsize, cnt/2);
      rsize -= rp[rsize - 1] == 0;
    }

  /* Calculate the remainder.  */
  mpn_mul_n (tp, rp, rp, rsize);
  tsize = rsize + rsize;
  tsize -= tp[tsize - 1] == 0;
  if (op_size < tsize
      || (op_size == tsize && mpn_cmp (op_ptr, tp, op_size) < 0))
    {
      /* R is too large.  Decrement it.  */

      /* These operations can't overflow.  */
      cy_limb  = mpn_sub_n (tp, tp, rp, rsize);
      cy_limb += mpn_sub_n (tp, tp, rp, rsize);
      mpn_decr_u (tp + rsize, cy_limb);
      mpn_incr_u (tp, (mp_limb_t) 1);

      mpn_decr_u (rp, (mp_limb_t) 1);

#ifdef DEBUG
      printf ("(adjusted) R = ");
      mpn_dump (rp, rsize);
#endif
    }

  if (rem_ptr != NULL)
    {
      cy_limb = mpn_sub (rem_ptr, op_ptr, op_size, tp, tsize);
      MPN_NORMALIZE (rem_ptr, op_size);
      TMP_FREE (marker);
      return op_size;
    }
  else
    {
      int res;
      res = op_size != tsize || mpn_cmp (op_ptr, tp, op_size);
      TMP_FREE (marker);
      return res;
    }
}
