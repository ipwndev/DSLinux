/*  Functions from hack's utils library.
    Copyright (C) 1989, 1990, 1991, 1998, 1999, 2003
    Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "config.h"

#include <stdio.h>

#include <errno.h>
#ifndef errno
  extern int errno;
#endif

#ifdef HAVE_STRINGS_H
# include <strings.h>
#else
# include <string.h>
#endif /* HAVE_STRINGS_H */

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "utils.h"

const char *myname;

/* Store information about files opened with ck_fopen
   so that error messages from ck_fread, ck_fwrite, etc. can print the
   name of the file that had the error */

struct open_file
  {
    FILE *fp;
    char *name;
    struct open_file *link;
    unsigned temp : 1;
  };

static struct open_file *open_files = NULL;
static void do_ck_fclose P_((FILE *fp));

/* Print an error message and exit */
#if !defined __STDC__ || !(__STDC__-0)
# include <varargs.h>
# define VSTART(l,a)	va_start(l)
void
panic(str, va_alist)
  char *str;
  va_dcl
#else /*__STDC__*/
# include <stdarg.h>
# define VSTART(l,a)	va_start(l, a)
void
panic(const char *str, ...)
#endif /* __STDC__ */
{
  va_list iggy;

  fprintf(stderr, "%s: ", myname);
  VSTART(iggy, str);
#ifndef HAVE_VPRINTF
# ifndef HAVE_DOPRNT
  fputs(str, stderr);	/* not great, but perhaps better than nothing... */
# else /* HAVE_DOPRNT */
  _doprnt(str, &iggy, stderr);
# endif /* HAVE_DOPRNT */
#else /* HAVE_VFPRINTF */
  vfprintf(stderr, str, iggy);
#endif /* HAVE_VFPRINTF */
  va_end(iggy);
  putc('\n', stderr);

  /* Unlink the temporary files.  */
  while (open_files)
    {
      if (open_files->temp)
	{
	  int fd = fileno (open_files->fp);
	  fclose (open_files->fp);
	  errno = 0;
	  unlink (open_files->name);
          if (errno != 0)
            fprintf (stderr, _("cannot remove %s: %s"), open_files->name, strerror (errno));
	}

      open_files = open_files->link;
    }

  exit(4);
}


/* Internal routine to get a filename from open_files */
static const char *utils_fp_name P_((FILE *fp));
static const char *
utils_fp_name(fp)
  FILE *fp;
{
  struct open_file *p;

  for (p=open_files; p; p=p->link)
    if (p->fp == fp)
      return p->name;
  if (fp == stdin)
    return "stdin";
  else if (fp == stdout)
    return "stdout";
  else if (fp == stderr)
    return "stderr";

  return "<unknown>";
}

/* Panic on failing fopen */
FILE *
ck_fopen(name, mode, fail)
  const char *name;
  const char *mode;
  bool fail;
{
  FILE *fp;
  struct open_file *p;

  fp = fopen (name, mode);
  if (!fp)
    {
      if (fail)
        panic(_("couldn't open file %s: %s"), name, strerror(errno));

      return NULL;
    }

  for (p=open_files; p; p=p->link)
    {
      if (fp == p->fp)
	{
	  FREE(p->name);
	  break;
	}
    }
  if (!p)
    {
      p = MALLOC(1, struct open_file);
      p->link = open_files;
      open_files = p;
    }
  p->name = ck_strdup(name);
  p->fp = fp;
  p->temp = false;
  return fp;
}

FILE *
ck_mkstemp (p_filename, tmpdir, base)
  char **p_filename;
  char *base, *tmpdir;
{
  char *template;
  FILE *fp;
  int fd;
  struct open_file *p;

  if (tmpdir == NULL)
    tmpdir = getenv("TMPDIR");
  if (tmpdir == NULL)
    {
      tmpdir = getenv("TMP");
      if (tmpdir == NULL)
#ifdef P_tmpdir
	tmpdir = P_tmpdir;
#else
	tmpdir = "/tmp";
#endif
    }

  template = xmalloc (strlen (tmpdir) + strlen (base) + 8);
  sprintf (template, "%s/%sXXXXXX", tmpdir, base);

  fd = mkstemp (template);
  if (fd == -1)
    panic(_("couldn't open temporary file %s: %s"), template, strerror(errno));

  *p_filename = template;
  fp = fdopen (fd, "w");

  p = MALLOC(1, struct open_file);
  p->name = ck_strdup (template);
  p->fp = fp;
  p->temp = true;
  p->link = open_files;
  open_files = p;
  return fp;
}

/* Panic on failing fwrite */
void
ck_fwrite(ptr, size, nmemb, stream)
  const VOID *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
{
  clearerr(stream);
  if (size && fwrite(ptr, size, nmemb, stream) != nmemb)
    panic(ngettext("couldn't write %d item to %s: %s",
		   "couldn't write %d items to %s: %s", nmemb), 
		nmemb, utils_fp_name(stream), strerror(errno));
}

/* Panic on failing fread */
size_t
ck_fread(ptr, size, nmemb, stream)
  VOID *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
{
  clearerr(stream);
  if (size && (nmemb=fread(ptr, size, nmemb, stream)) <= 0 && ferror(stream))
    panic(_("read error on %s: %s"), utils_fp_name(stream), strerror(errno));

  return nmemb;
}

size_t
ck_getline(text, buflen, stream)
  char **text;
  size_t *buflen;
  FILE *stream;
{
  int result;
  if (!ferror (stream))
    result = getline (text, buflen, stream);

  if (ferror (stream))
    panic (_("read error on %s: %s"), utils_fp_name(stream), strerror(errno));

  return result;
}

/* Panic on failing fflush */
void
ck_fflush(stream)
  FILE *stream;
{
  clearerr(stream);
  if (fflush(stream) == EOF && errno != EBADF)
    panic("couldn't flush %s: %s", utils_fp_name(stream), strerror(errno));
}

/* Panic on failing fclose */
void
ck_fclose(stream)
  FILE *stream;
{
  struct open_file r;
  struct open_file *prev;
  struct open_file *cur;

  /* a NULL stream means to close all files */
  r.link = open_files;
  prev = &r;
  while ( (cur = prev->link) )
    {
      if (!stream || stream == cur->fp)
	{
	  do_ck_fclose (cur->fp);
	  prev->link = cur->link;
	  FREE(cur->name);
	  FREE(cur);
	}
      else
	prev = cur;
    }

  open_files = r.link;

  /* Also care about stdout, because if it is redirected the
     last output operations might fail and it is important
     to signal this as an error (perhaps to make). */
  if (!stream)
    {
      do_ck_fclose (stdout);
      do_ck_fclose (stderr);
    }
}

/* Close a single file. */
void
do_ck_fclose(fp)
  FILE *fp;
{
  int fd;
  ck_fflush(fp);
  clearerr(fp);

  /* We want to execute both arms, so use | not ||.  */
  if (fclose(fp) == EOF)
    panic("couldn't close %s: %s", utils_fp_name(fp), strerror(errno));
}


/* Panic on failing rename */
void
ck_rename (from, to, unlink_if_fail)
  const char *from, *to;
  const char *unlink_if_fail;
{
  int rd = rename (from, to);
  if (rd != -1)
    return;

  if (unlink_if_fail)
    {
      int save_errno = errno;
      errno = 0;
      unlink (unlink_if_fail);

      /* Failure to remove the temporary file is more severe, so trigger it first.  */
      if (errno != 0)
        panic (_("cannot remove %s: %s"), unlink_if_fail, strerror (errno));

      errno = save_errno;
    }

  panic (_("cannot rename %s: %s"), from, strerror (errno));
}




/* Panic on failing malloc */
VOID *
ck_malloc(size)
  size_t size;
{
  VOID *ret = calloc(1, size ? size : 1);
  if (!ret)
    panic("couldn't allocate memory");
  return ret;
}

/* Panic on failing malloc */
VOID *
xmalloc(size)
  size_t size;
{
  return ck_malloc(size);
}

/* Panic on failing realloc */
VOID *
ck_realloc(ptr, size)
  VOID *ptr;
  size_t size;
{
  VOID *ret;

  if (size == 0)
    {
      FREE(ptr);
      return NULL;
    }
  if (!ptr)
    return ck_malloc(size);
  ret = realloc(ptr, size);
  if (!ret)
    panic("couldn't re-allocate memory");
  return ret;
}

/* Return a malloc()'d copy of a string */
char *
ck_strdup(str)
  const char *str;
{
  char *ret = MALLOC(strlen(str)+1, char);
  return strcpy(ret, str);
}

/* Return a malloc()'d copy of a block of memory */
VOID *
ck_memdup(buf, len)
  const VOID *buf;
  size_t len;
{
  VOID *ret = ck_malloc(len);
  return memcpy(ret, buf, len);
}

/* Release a malloc'd block of memory */
void
ck_free(ptr)
  VOID *ptr;
{
  if (ptr)
    free(ptr);
}


/* Implement a variable sized buffer of `stuff'.  We don't know what it is,
nor do we care, as long as it doesn't mind being aligned by malloc. */

struct buffer
  {
    size_t allocated;
    size_t length;
    char *b;
  };

#define MIN_ALLOCATE 50

struct buffer *
init_buffer()
{
  struct buffer *b = MALLOC(1, struct buffer);
  b->b = MALLOC(MIN_ALLOCATE, char);
  b->allocated = MIN_ALLOCATE;
  b->length = 0;
  return b;
}

char *
get_buffer(b)
  struct buffer *b;
{
  return b->b;
}

size_t
size_buffer(b)
  struct buffer *b;
{
  return b->length;
}

static void resize_buffer P_((struct buffer *b, size_t newlen));
static void
resize_buffer(b, newlen)
  struct buffer *b;
  size_t newlen;
{
  char *try = NULL;
  size_t alen = b->allocated;

  if (newlen <= alen)
    return;
  alen *= 2;
  if (newlen < alen)
    try = realloc(b->b, alen);	/* Note: *not* the REALLOC() macro! */
  if (!try)
    {
      alen = newlen;
      try = REALLOC(b->b, alen, char);
    }
  b->allocated = alen;
  b->b = try;
}

char *
add_buffer(b, p, n)
  struct buffer *b;
  const char *p;
  size_t n;
{
  char *result;
  if (b->allocated - b->length < n)
    resize_buffer(b, b->length+n);
  result = memcpy(b->b + b->length, p, n);
  b->length += n;
  return result;
}

char *
add1_buffer(b, c)
  struct buffer *b;
  int c;
{
  /* This special case should be kept cheap;
   *  don't make it just a mere convenience
   *  wrapper for add_buffer() -- even "builtin"
   *  versions of memcpy(a, b, 1) can become
   *  expensive when called too often.
   */
  if (c != EOF)
    {
      char *result;
      if (b->allocated - b->length < 1)
	resize_buffer(b, b->length+1);
      result = b->b + b->length++;
      *result = c;
      return result;
    }

  return NULL;
}

void
free_buffer(b)
  struct buffer *b;
{
  if (b)
    FREE(b->b);
  FREE(b);
}
