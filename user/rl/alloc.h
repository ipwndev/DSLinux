/*
   alloc.h - functions for clean memory management

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


#ifndef _ALLOC_H
#define _ALLOC_H 1


/* allocate memory with error-checking */
void *xmalloc(size_t size);


/* allocate size for a specific type */
#define xxmalloc(type,size) (type *)xmalloc(sizeof(type)*(size))


/* resize allocated memory */
void *xrealloc(void *ptr,size_t size);


/* realloc for a specific type */
#define xxrealloc(ptr,type,size) (type *)xrealloc(ptr,sizeof(type)*(size))


/* free the memory */
void xfree(void *ptr);


/* strdup() wrapper */
char *xstrdup(const char *s);


#endif /* _ALLOC_H */
