/* mpz_cdiv_ui -- Division rounding the quotient towards +infinity.  The
   remainder gets the opposite sign as the denominator.  In order to make it
   always fit into the return type, the negative of the true remainder is
   returned.

Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.

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

unsigned long int
#if __STDC__
mpz_cdiv_ui (mpz_srcptr dividend, unsigned long int divisor)
#else
mpz_cdiv_ui (dividend, divisor)
     mpz_srcptr dividend;
     unsigned long int divisor;
#endif
{
  mp_size_t dividend_size;
  mp_size_t size;
  mp_limb_t remainder_limb;

  dividend_size = dividend->_mp_size;
  size = ABS (dividend_size);

  remainder_limb = mpn_mod_1 (dividend->_mp_d, size, (mp_limb_t) divisor);

  if (remainder_limb != 0 && dividend_size >= 0)
    remainder_limb = divisor - remainder_limb;

  return remainder_limb;
}
