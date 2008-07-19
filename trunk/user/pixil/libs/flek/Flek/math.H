/* -*-C++-*- 
   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

#ifndef _INLINES_HH_
#define _INLINES_HH_

#include <math.h>
#include <Flek/types.H>

/*
 * Predefined constants, and macros
 * To change the values of some of these macros, define them before including
 * this file. Or dont include this file at all and define them on your own
 */

/**
 * @package libflek_core
 */

#ifndef _INLINE_ABS_
#define _INLINE_ABS_

/** 
 * Returns the absolute value.
 */
inline int absolute (int x)
{
  return (x < 0) ? -x : x;
}

/**
 * Returns the absolute value.
 */
inline float absolute (float x)
{
  return (x < 0.0) ? -x : x;
}

#endif

// Zero '0' value to be used for comparison and assignment
#ifndef ZERO
#define ZERO 1.0e-10
#endif

// float infinity to be used for comparison and assignment
#ifndef FLT_INF
#define FLT_INF 1.0e35
#endif

// double infinity to be used for comparison and assignment
#ifndef DBL_INF
#define DBL_INF 1.0e100
#endif

#ifndef _INLINE_ZERO_CHECKS_
#define _INLINE_ZERO_CHECKS_

/**
 * Check if passed number is non-zero.
 */
inline bool is_non_zero (int num) {
  if ( num != 0 ) return true;
  return false;
}

/**
 * Check if passed number is non-zero.  (Uses ZERO bounds).
 */
inline bool is_non_zero (float num) {
  if ( (num < -ZERO) || (num > ZERO) ) return true;
  return false;
}

#endif

#ifndef _INLINE_FP_EQUAL_
#define _INLINE_FP_EQUAL_

inline bool are_equal (float x1, float x2) {
  return ( ( absolute (x1-x2) < ZERO ) ? true : false );
}

#endif // #ifndef _INLINE_FP_EQUAL_


#ifndef _INLINE_ODD_EVEN_
#define _INLINE_ODD_EVEN_

/**
 * Check if passed number is odd.
 */
inline bool is_odd (int num) {
  if ( num % 2 ) return true;
  return false;
}

/**
 * Check if passed number is even.
 */
inline bool is_even (int num) {
  if ( num % 2 ) return false;
  return true;
}

/**
 * Check if passed number is odd.
 */
inline bool is_odd (uint num) {
  if ( num % 2 ) return true;
  return false;
}

/**
 * Check if passed number is even.
 */
inline bool is_even (uint num) {
  if ( num % 2 ) return false;
  return true;
}

#endif

#ifndef _INLINE_MATH_
#define _INLINE_MATH_

/**
 * Square the number.
 */
inline int sqr (int x) {
  return x*x;
}

/**
 * Cube the number.
 */
inline int cube (int x) {
  return x*x*x;
}

/**
 * Square the number.
 */
inline uint sqr (uint x) {
  return x*x;
}

/**
 * Cube the number.
 */
inline uint cube (uint x) {
  return x*x*x;
}

/**
 * Square the number.
 */
inline float sqr (float x) {
  return x*x;
}

/**
 * Cube the number.
 */
inline float cube (float x) {
  return x*x*x;
}

#endif

#ifndef _INLINE_MIN_MAX_
#define _INLINE_MIN_MAX_

/**
 * Minimum of two numbers.
 */
inline int min (int x, int y) {
  return ( (x < y) ? x : y );
}

/**
 * Maximum of two numbers.
 */
inline int max (int x, int y) {
  return ( (x > y) ? x : y );
}

/**
 * Minimum of two numbers.
 */
inline uint min (uint x, uint y) {
  return ( (x < y) ? x : y );
}

/**
 * Maximum of two numbers.
 */
inline uint max (uint x, uint y) {
  return ( (x > y) ? x : y );
}

inline ushort min (ushort x, ushort y) {
  return ( (x < y) ? x : y );
}

inline ushort max (ushort x, ushort y) {
  return ( (x > y) ? x : y );
}

/**
 * Minimum of two numbers.
 */
inline float min (float x, float y) {
  return ( (x < y) ? x : y );
}

/**
 * Maximum of two numbers.
 */
inline float max (float x, float y) {
  return ( (x > y) ? x : y );
}

/**
 * Minimum of two unsigned chars.
 */
inline unsigned char min (unsigned char x, unsigned char y) {
  return ( (x < y) ? x : y );
}

/**
 * Maximum of two unsigned chars.
 */
inline unsigned char max (unsigned char x, unsigned char y) {
  return ( (x > y) ? x : y );
}


#endif


#ifndef _INLINE_SWAP_
#define _INLINE_SWAP_

/**
 * Swap two numbers.
 */
inline void swap (int& x, int& y) {
  int t = x;
  x = y; y = t;
}

/**
 * Swap two numbers.
 */
inline void swap (uint& x, uint& y) {
  uint t = x;
  x = y; y = t;
}

/**
 * Swap two numbers.
 */
inline void swap (float& x, float& y) {
  float t = x;
  x = y; y = t;
}

#endif

#ifndef _INLINE_DEG_RAD_
#define _INLINE_DEG_RAD_

/**
 * Degrees to radians.
 */
inline float deg2rad (float deg) {
  return deg*M_PI/180.0;
}

/**
 * Radians to degrees.
 */
inline float rad2deg (float rad) {
  return rad*180.0/M_PI;
}

#endif

#ifndef _INLINE_SIGN_
#define _INLINE_SIGN_

/**
 * Return the sign of a number.  1, 0, or -1.
 */
inline int sign (int x) {
  return ( (x < 0) ? -1 : ((x > 0) ? 1 : 0) );
}

/**
 * Return the sign of a number.  1, 0, or -1.
 */
inline float sign (float x) {
  return ( (x < 0.0) ? -1.0 : ((x > 0.0) ? 1.0 : 0.0) );
}

/**
 * For matrices - sign of cofactor. 1 if i+j is even, -1 if i+j is odd
 */
inline int cofsign (uint i, uint j) {
  return ( ((i+j)%2) ? -1 : 1 );
}

#endif

/**
 * Clamp a value to an upper bound.
 * 
 * @param Value The value to clamp.
 * @param Clamp The upper bound.
 */
inline int clamp_upper (int Value, int Clamp) {
  if (Value > Clamp) return Clamp;
  return Value;
}

/**
 * Clamp a value to a lower bound.
 * 
 * @param Value The value to clamp.
 * @param Clamp The lower bound.
 */
inline int clamp_lower (int x, int Clamp)
{
  if (x < Clamp) return Clamp;
  return x;
}

#endif // #ifndef _INLINES_HH_
