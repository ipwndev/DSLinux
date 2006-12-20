/* mpf_set_q (mpf_t rop, mpq_t op) -- Convert the rational op to the float rop.

Copyright (C) 1996, 1999 Free Software Foundation, Inc.

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

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

void
#if __STDC__
mpf_set_q (mpf_t r, mpq_srcptr q)
#else
mpf_set_q (r, q)
     mpf_t r;
     mpq_srcptr q;
#endif
{
  mp_ptr np, dp;
  mp_ptr rp;
  mp_size_t nsize, dsize;
  mp_size_t qsize, rsize;
  mp_size_t sign_quotient;
  unsigned normalization_steps;
  mp_limb_t qlimb;
  mp_ptr qp;
  mp_size_t prec;
  mp_exp_t exp;
  TMP_DECL (marker);

  nsize = SIZ (&q->_mp_num);
  dsize = SIZ (&q->_mp_den);

  if (nsize == 0)
    {
      SIZ (r) = 0;
      EXP (r) = 0;
      return;
    }

  prec = PREC (r) + 1;

  TMP_MARK (marker);

  qp = PTR (r);

  sign_quotient = nsize ^ dsize;
  nsize = ABS (nsize);
  dsize = ABS (dsize);
  np = PTR (&q->_mp_num);
  dp = PTR (&q->_mp_den);

  exp = nsize - dsize;

  if (nsize > prec)
    {
      np += nsize - prec;
      nsize = prec;
    }
  if (dsize > prec)
    {
      dp += dsize - prec;
      dsize = prec;
    }

  rsize = MAX (nsize, dsize);
  rp = (mp_ptr) TMP_ALLOC ((rsize + 1) * BYTES_PER_MP_LIMB);

  count_leading_zeros (normalization_steps, dp[dsize - 1]);

  /* Normalize the denominator, i.e. make its most significant bit set by
     shifting it NORMALIZATION_STEPS bits to the left.  Also shift the
     numerator the same number of steps (to keep the quotient the same!).  */
  if (normalization_steps != 0)
    {
      mp_ptr tp;
      mp_limb_t nlimb;

      /* Shift up the denominator setting the most significant bit of
	 the most significant limb.  Use temporary storage not to clobber
	 the original contents of the denominator.  */
      tp = (mp_ptr) TMP_ALLOC (dsize * BYTES_PER_MP_LIMB);
      mpn_lshift (tp, dp, dsize, normalization_steps);
      dp = tp;

      if (rsize != nsize)
	{
	  MPN_ZERO (rp, rsize - nsize);
	  nlimb = mpn_lshift (rp + (rsize - nsize),
			      np, nsize, normalization_steps);
	}
      else
	{
	  nlimb = mpn_lshift (rp, np, rsize, normalization_steps);
	}
      if (nlimb != 0)
	{
	  rp[rsize] = nlimb;
	  exp++;
	  /* Don't just increase rsize, chop off rp at the low end instead.  */
	  if (rsize == prec)
	    rp++;
	  else
	    rsize++;
	}
    }
  else
    {
      if (rsize != nsize)
	{
	  MPN_ZERO (rp, rsize - nsize);
	  MPN_COPY (rp + (rsize - nsize), np, nsize);
	}
      else
	{
	  MPN_COPY (rp, np, rsize);
	}
    }

  qlimb = mpn_divrem (qp, prec - 1 - (rsize - dsize), rp, rsize, dp, dsize);
  qsize = prec - 1;
  if (qlimb)
    {
      qp[qsize] = qlimb;
      qsize++;
      exp++;
    }

  EXP (r) = exp;
  SIZ (r) = sign_quotient >= 0 ? qsize : -qsize;

  TMP_FREE (marker);
}
