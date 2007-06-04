/*
   random.c - implementation of all random number related functions

   Copyright (C) 2001, 2002, 2004 Arthur de Jong

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


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

#include "rl.h"
#include "random.h"


#if defined(USE_RAND_RNG)


/* initialize the random number generator with a sensible value
   this calls srand() so rand() can be used */
void
randomize()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand(((unsigned int)(tv.tv_usec))^((unsigned int)getpid()));
}


/* returns a random number x where 0 <= x < max */
int
random_below(int max)
{
  int i;
  int j;
  i=RAND_MAX/max;
  /* there realy should be a better way for this */
  /* TODO: fix with something smart so that this even works for large values
           (on systems with a low RAND_MAX maybe two calls 
            to rand() could be used) */
  /* TODO: add range checking 0<max<RAND_MAX */
  do
  {
    j=rand()/i;
  }
  while (j>=max);
  return j;
}


#elif defined(USE_DEV_RNG)


#define RANDOM_DEVICE "/dev/urandom"

/* the file pointer that is used to read from the random device */
static FILE *random_fp;


/* open random device for reading */
void
randomize()
{
  random_fp=fopen(RANDOM_DEVICE,"rb");
}


/* returns a random number x where 0 <= x < max */
int
random_below(int max)
{
  int i;
  int j;
  int r;
  i=INT_MAX/max;
  /* TODO: check same problems as above */
  do
  {
    if (fread(&r,sizeof(int),1,random_fp)!=1)
    {
      fprintf(stderr,_("Error reading from %s\n"),RANDOM_DEVICE);
      exit(1);
    }
    j=(r&INT_MAX)/i;
  }
  while (j>=max);
  return j;
}


#else


#error invalid random number generator selected


#endif

/* thow the dice and return true (1)
   with a chance of x/y (false=0) */
int
random_draw(int x,int y)
{
  return random_below(y)<x;
}

