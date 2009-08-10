/*
 *  (C) Copyright 2008-2009 Kamil Kopec <kamil_kopec@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ds_types_h
#define ds_types_h

#define HAVE_SYS_TYPES_H
#define HAVE_CTYPE_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_CTYPE_H
# include <ctype.h>
#endif

/* Types are the same as in SDL library */
typedef int8_t		Sint8;
typedef u_int8_t	Uint8;
typedef int16_t		Sint16;
typedef u_int16_t	Uint16;
typedef int32_t		Sint32;
typedef u_int32_t	Uint32;

#if __GLIBC_HAVE_LONG_LONG
typedef int64_t		Sint64;
typedef u_int64_t	Uint64;
#else
typedef struct {
	Uint32 hi;
	Uint32 lo;
} Uint64, Sint64;
#endif

/* Make sure the types really have the right sizes */
/*
#if (sizeof(Uint8) != 1) || (sizeof(Sint8) != 1)
#error Precompilation error, please post this bug id:00001 to programmer.
#endif

#if (sizeof(Uint16) != 2) || (sizeof(Sint16) != 2)
#error Precompilation error, please post this bug id:00001 to programmer.
#endif

#if (sizeof(Uint32) != 4) || (sizeof(Sint32) != 4)
#error Precompilation error, please post this bug id:00001 to programmer.
#endif

#if (sizeof(Uint64) != 8) || (sizeof(Sint64) != 8)
#error Precompilation error, please post this bug id:00001 to programmer.
#endif
*/
#endif
