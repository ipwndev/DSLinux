/*
   random.h - definitions of all random number related functions

   Copyright (C) 2002 Arthur de Jong

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/


#ifndef _RANDOM_H
#define _RANDOM_H 1


/* initialize the random number generator with a sensible value
   this calls srand() so rand() can be used */
void randomize(void);


/* returns a random number x where 0 <= x < max */
int random_below(int max);


/* thow the dice and return true (1)
   with a chance of x/y (false=0) */
int random_draw(int x,int y);


#endif /* _RANDOM_H */
