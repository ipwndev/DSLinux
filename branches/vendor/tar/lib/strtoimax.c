/* Convert string representation of a number into an intmax_t value.

   Copyright (C) 1999, 2001, 2002, 2003, 2004, 2006 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by Paul Eggert. */

#include <config.h>

/* Verify interface.  */
#include <inttypes.h>

#include <stdlib.h>

#include "verify.h"

#ifdef UNSIGNED
# ifndef HAVE_DECL_STRTOULL
"this configure-time declaration test was not run"
# endif
# if !HAVE_DECL_STRTOULL && HAVE_UNSIGNED_LONG_LONG_INT
unsigned long long strtoull (char const *, char **, int);
# endif

#else

# ifndef HAVE_DECL_STRTOLL
"this configure-time declaration test was not run"
# endif
# if !HAVE_DECL_STRTOLL && HAVE_UNSIGNED_LONG_LONG_INT
long long strtoll (char const *, char **, int);
# endif
#endif

#ifdef UNSIGNED
# undef HAVE_LONG_LONG_INT
# define HAVE_LONG_LONG_INT HAVE_UNSIGNED_LONG_LONG_INT
# define INT uintmax_t
# define strtoimax strtoumax
# define strtol strtoul
# define strtoll strtoull
#else
# define INT intmax_t
#endif

INT
strtoimax (char const *ptr, char **endptr, int base)
{
#if HAVE_LONG_LONG_INT
  verify (sizeof (INT) == sizeof (long int)
	  || sizeof (INT) == sizeof (long long int));

  if (sizeof (INT) != sizeof (long int))
    return strtoll (ptr, endptr, base);
#else
  verify (sizeof (INT) == sizeof (long int));
#endif

  return strtol (ptr, endptr, base);
}
