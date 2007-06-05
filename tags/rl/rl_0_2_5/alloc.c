/*
   alloc.c - functions for clean memory management

   Copyright (C) 2001, 2002 Arthur de Jong

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rl.h"
#include "alloc.h"


/* allocate memory with error-checking */
void *
xmalloc(size_t size)
{
  void *res;
  res=malloc(size);
  if (res==NULL)
  {
    if (!quiet)
      fprintf(stderr,_("Error allocating memory!\n"));
    exit(1);
  }
#ifdef DEBUG
  fprintf(stderr,"DEBUG: allocated %p (%d)\n",res,size);
#endif /* DEBUG */
  return res;
}


/* resize allocated memory */
void *
xrealloc(void *ptr,size_t size)
{
  void *res;
  res=realloc(ptr,size);
  if ((res==NULL)&&(size!=0))
  {
    if (!quiet)
      fprintf(stderr,_("Error allocating memory!\n"));
    exit(1);
  }
#ifdef DEBUG
  fprintf(stderr,"DEBUG: reallocated %p to %p (%d)\n",ptr,res,size);
#endif /* DEBUG */
  return res;
}


/* free the memory */
void
xfree(void *ptr)
{
  free(ptr);
#ifdef DEBUG
  fprintf(stderr,"DEBUG: freed %p\n",ptr);
#endif /* DEBUG */
}


/* strdup() wrapper */
char *
xstrdup(const char *str)
{
  char *res;
  res=strdup(str);
  if (res==NULL)
  {
    if (!quiet)
      fprintf(stderr,_("Error allocating memory!\n"));
    exit(1);
  }
  return res;
}
