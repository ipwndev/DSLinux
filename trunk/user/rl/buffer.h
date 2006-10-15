/*
   buffer.h - functions for handling buffers

   Copyright (C) 2002, 2005 Arthur de Jong

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


#ifndef _BUFFER_H
#define _BUFFER_H 1


/* structure for holding a string */
struct buffer
{
  char *buf;      /* pointer to allocated memory */
  int alloc;      /* size of allocated memory */
  int len;        /* amount of memory in use */
  int linenumber; /* number of the line */
};


/* intialize struct buffer and allocate memory for it */
void buffer_init(struct buffer *line,int alloc);


/* change the size of the line */
void buffer_grow(struct buffer *line,int alloc);


/* free allocated memory */
void buffer_free(struct buffer *line);


/* make a copy of the sc line into dst line
   optionaly growing of allocationg dst
   the orriginal content of dst is lost and a pointer
   to the result (dst or a new line if dst==NULL) */
struct buffer *buffer_copy(struct buffer *dst,struct buffer *src);


/* read a single line from the file 
   this function returns a struct buffer structure containing
   the read line with newline or NULL on EOF
   lines are delimited by the delim char */
struct buffer *buffer_readline(FILE *in,struct buffer *line,char delim);


/* read a complete file into a buffer */
struct buffer *buffer_readfile(FILE *in,struct buffer *buf);


#endif /* _BUFFER_H */
