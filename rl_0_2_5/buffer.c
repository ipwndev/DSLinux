/*
   buffer.c - functions for handling buffers

   Copyright (C) 2001, 2002, 2003, 2005 Arthur de Jong

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
#include "buffer.h"
#include "alloc.h"


/* intialize struct buffer and allocate memory for it */
void
buffer_init(struct buffer *line,int alloc)
{
  line->alloc=alloc;
  line->buf=xxmalloc(char,line->alloc);
  line->len=0;
}


/* change the size of the line */
void
buffer_grow(struct buffer *line,int alloc)
{
  line->alloc=alloc;
  line->buf=xxrealloc(line->buf,char,line->alloc);
}


/* free allocated memory */
void
buffer_free(struct buffer *line)
{
  if (line->buf==NULL)
    return; /* not allocated so not freed */
  xfree(line->buf);
  line->buf=NULL;
  line->alloc=0;
}


/* make a copy of the src buffer into dst buffer
   optionaly growing or allocating dst
   the orriginal content of dst is lost and a pointer
   to the result (dst or a new buffer if dst==NULL)
   is returned */
struct buffer *
buffer_copy(struct buffer *dst,struct buffer *src)
{
  if (dst==NULL)
  {
    dst=xxmalloc(struct buffer,1);
    buffer_init(dst,src->len+AVGLINELEN/2);
  }
  else if (dst->alloc<src->len)
  {
    buffer_grow(dst,src->len+AVGLINELEN/2);
  }
  memcpy(dst->buf,src->buf,src->len);
  dst->len=src->len;
  dst->linenumber=src->linenumber;
  return dst;
}


/* read a single line from the file
   this function returns a buffer structure containing
   the read line with newline or NULL on EOF
   lines are delimited by the delim char
   if line is not NULL a new buffer is initialized
   and returned, otherwise the lines is read in the line buffer */
struct buffer *
buffer_readline(FILE *in,struct buffer *line,char delim)
{
  int c; /* the char read */
  struct buffer *orr;
  /* initialize new line if NULL */
  orr=line;
  if (line==NULL)
  {
    /* make a new line */
    line=xxmalloc(struct buffer,1);
    buffer_init(line,AVGLINELEN*2);
  }
  line->len=0;
  while ( ((c=fgetc(in))!=EOF) )
  {
    /* check allocated memory */
    if (line->alloc<(line->len+1))
      buffer_grow(line,line->alloc*2);
    /* store */
    line->buf[line->len++]=c;
    /* check for delim */
    if (c==delim)
      break; /* exit while */
  }
  /* check if anything was read */
  if (line->len==0)
  {
    /* free the line if it was created here */
    if (orr==NULL)
    {
      buffer_free(line);
      xfree(line);
    }
    return NULL;
  }
  /* the line was read */
  return line;
}


/* read a complete file into a buffer */
struct buffer *
buffer_readfile(FILE *in,struct buffer *buf)
{
  int i;
  /* initialize buffer */
  if (buf==NULL)
  {
    buf=xxmalloc(struct buffer,1);
    buffer_init(buf,BLOCKSIZE*2);
  }
  /* read the file */
  buf->len=0;
  do
  {
    /* grow buffer if needed */
    if (buf->alloc<=buf->len+BLOCKSIZE)
      buffer_grow(buf,buf->alloc*2);
    i=fread(buf->buf+buf->len,sizeof(char),BLOCKSIZE,in);
    buf->len+=i;
  }
  while (i==BLOCKSIZE);
  return buf;
}
