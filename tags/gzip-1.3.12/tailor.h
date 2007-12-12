/* tailor.h -- target dependent definitions

   Copyright (C) 1997, 1998, 1999, 2002, 2006 Free Software Foundation, Inc.
   Copyright (C) 1992-1993 Jean-loup Gailly

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

/* The target dependent definitions should be defined here only.
 * The target dependent functions should be defined in tailor.c.
 */

/* $Id: tailor.h,v 1.5 2006/12/07 06:58:13 eggert Exp $ */

#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif

#if defined(__OS2__) && !defined(OS2)
#  define OS2
#endif

#if defined(OS2) && defined(MSDOS) /* MS C under OS/2 */
#  undef MSDOS
#endif

#ifdef MSDOS
#  ifdef __GNUC__
     /* DJGPP version 1.09+ on MS-DOS.
      * The DJGPP 1.09 stat() function must be upgraded before gzip will
      * fully work.
      * No need for HAVE_DIRENT_H, since <unistd.h> defines POSIX_SOURCE which
      * implies HAVE_DIRENT_H.
      */
#    define near
#  else
#    define MAXSEG_64K
#    ifdef __TURBOC__
#      define off_t long
#      ifdef __BORLANDC__
#        define HAVE_DIRENT_H
#      endif
#      define HAVE_UTIME_H
#    else /* MSC */
#      define HAVE_SYS_UTIME_H
#    endif
#  endif
#  define PATH_SEP2 '\\'
#  define PATH_SEP3 ':'
#  define MAX_PATH_LEN  128
#  define NO_MULTIPLE_DOTS
#  define MAX_EXT_CHARS 3
#  define Z_SUFFIX "z"
#  define PROTO
#  define STDC_HEADERS
#  define NO_SIZE_CHECK
#  define UNLINK_READONLY_BUG
#  define casemap(c) tolow(c) /* Force file names to lower case */
#  include <io.h>
#  define OS_CODE  0x00
#  define SET_BINARY_MODE(fd) setmode(fd, O_BINARY)
#  if !defined(NO_ASM) && !defined(ASMV)
#    define ASMV
#  endif
#else
#  define near
#endif

#ifdef OS2
#  define PATH_SEP2 '\\'
#  define PATH_SEP3 ':'
#  define MAX_PATH_LEN  260
#  ifdef OS2FAT
#    define NO_MULTIPLE_DOTS
#    define MAX_EXT_CHARS 3
#    define Z_SUFFIX "z"
#    define casemap(c) tolow(c)
#  endif
#  define PROTO
#  define STDC_HEADERS
#  define UNLINK_READONLY_BUG
#  include <io.h>
#  define OS_CODE  0x06
#  define SET_BINARY_MODE(fd) setmode(fd, O_BINARY)
#  ifdef _MSC_VER
#    define HAVE_SYS_UTIME_H
#    define MAXSEG_64K
#    undef near
#    define near _near
#  endif
#  ifdef __EMX__
#    define HAVE_SYS_UTIME_H
#    define HAVE_DIRENT_H
#    define EXPAND(argc,argv) \
       {_response(&argc, &argv); _wildcard(&argc, &argv);}
#  endif
#  ifdef __BORLANDC__
#    define HAVE_DIRENT_H
#    define HAVE_UTIME_H
#  endif
#  ifdef __ZTC__
#    define NO_DIR 1
#    include <dos.h>
#    define EXPAND(argc,argv) \
       {response_expand(&argc, &argv);}
#  endif
#endif

#ifdef WIN32 /* Windows NT */
#  define HAVE_SYS_UTIME_H
#  define PATH_SEP2 '\\'
#  define PATH_SEP3 ':'
#  define MAX_PATH_LEN  260
#  define PROTO
#  define STDC_HEADERS
#  define SET_BINARY_MODE(fd) setmode(fd, O_BINARY)
#  define UNLINK_READONLY_BUG
#  include <io.h>
#  include <malloc.h>
#  ifdef NTFAT
#    define NO_MULTIPLE_DOTS
#    define MAX_EXT_CHARS 3
#    define Z_SUFFIX "z"
#    define casemap(c) tolow(c) /* Force file names to lower case */
#  endif
#  define OS_CODE  0x0b
#endif

#ifdef MSDOS
#  ifdef __TURBOC__
#    include <alloc.h>
#    define DYN_ALLOC
     /* Turbo C 2.0 does not accept static allocations of large arrays */
     void * fcalloc (unsigned items, unsigned size);
     void fcfree (void *ptr);
#  else /* MSC */
#    include <malloc.h>
#    define fcalloc(nitems,itemsize) halloc((long)(nitems),(itemsize))
#    define fcfree(ptr) hfree(ptr)
#  endif
#else
#  ifdef MAXSEG_64K
#    define fcalloc(items,size) calloc((items),(size))
#  else
#    define fcalloc(items,size) malloc((size_t)(items)*(size_t)(size))
#  endif
#  define fcfree(ptr) free(ptr)
#endif

#if defined(VAXC) || defined(VMS)
#  define PATH_SEP ']'
#  define PATH_SEP2 ':'
#  define SUFFIX_SEP ';'
#  define NO_MULTIPLE_DOTS
#  define Z_SUFFIX "-gz"
#  define RECORD_IO 1
#  define casemap(c) tolow(c)
#  define OS_CODE  0x02
#  define OPTIONS_VAR "GZIP_OPT"
#  define STDC_HEADERS
#  define EXPAND(argc,argv) vms_expand_args(&argc,&argv);
#  include <file.h>
#  define unlink delete
#  ifdef VAXC
#    include <unixio.h>
#  else
#    define HAVE_FCNTL_H
#  endif
#endif

#ifdef AMIGA
#  define PATH_SEP2 ':'
#  define STDC_HEADERS
#  define OS_CODE  0x01
#  define ASMV
#  ifdef __GNUC__
#    define HAVE_DIRENT_H
#    define HAVE_FCNTL_H
#    define HAVE_UNISTD_H
#    define HAVE_CHOWN
#    define HAVE_LSTAT
#  else /* SASC */
#    define NO_STDIN_FSTAT
#    define HAVE_SYS_DIR_H
#    include <fcntl.h> /* for read() and write() */
#    define direct dirent
     extern void _expand_args(int *argc, char ***argv);
#    define EXPAND(argc,argv) _expand_args(&argc,&argv);
#    undef  O_BINARY /* disable useless --ascii option */
#  endif
#endif

#if defined(ATARI) || defined(atarist)
#  ifndef STDC_HEADERS
#    define STDC_HEADERS
#    define HAVE_UNISTD_H
#    define HAVE_DIRENT_H
#  endif
#  define ASMV
#  define OS_CODE  0x05
#  ifdef TOSFS
#    define PATH_SEP2 '\\'
#    define PATH_SEP3 ':'
#    define MAX_PATH_LEN  128
#    define NO_MULTIPLE_DOTS
#    define MAX_EXT_CHARS 3
#    define Z_SUFFIX "z"
#    define casemap(c) tolow(c) /* Force file names to lower case */
#  endif
#endif

#ifdef MACOS
#  define PATH_SEP ':'
#  define DYN_ALLOC
#  define PROTO
#  define NO_STDIN_FSTAT
#  define chmod(file, mode) (0)
#  define OPEN(name, flags, mode) open(name, flags)
#  define OS_CODE  0x07
#  ifdef MPW
#    define isatty(fd) ((fd) <= 2)
#  endif
#endif

#ifdef __50SERIES /* Prime/PRIMOS */
#  define PATH_SEP '>'
#  define STDC_HEADERS
#  define NO_STDIN_FSTAT
#  define NO_SIZE_CHECK
#  define RECORD_IO  1
#  define casemap(c)  tolow(c) /* Force file names to lower case */
#  define put_char(c) put_byte((c) & 0x7F)
#  define get_char(c) ascii2pascii(get_byte())
#  define OS_CODE  0x0F    /* temporary, subject to change */
#  ifdef SIGTERM
#    undef SIGTERM         /* We don't want a signal handler for SIGTERM */
#  endif
#endif

#if defined(pyr) && !defined(NOMEMCPY) /* Pyramid */
#  define NOMEMCPY /* problem with overlapping copies */
#endif

#ifdef TOPS20
#  define OS_CODE  0x0a
#endif


	/* Common defaults */

#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef PATH_SEP
#  define PATH_SEP '/'
#endif

#ifndef casemap
#  define casemap(c) (c)
#endif

#ifndef OPTIONS_VAR
#  define OPTIONS_VAR "GZIP"
#endif

#ifndef Z_SUFFIX
#  define Z_SUFFIX ".gz"
#endif

#ifdef MAX_EXT_CHARS
#  define MAX_SUFFIX  MAX_EXT_CHARS
#else
#  define MAX_SUFFIX  30
#endif

#ifndef MAKE_LEGAL_NAME
#  ifdef NO_MULTIPLE_DOTS
#    define MAKE_LEGAL_NAME(name)   make_simple_name(name)
#  else
#    define MAKE_LEGAL_NAME(name)
#  endif
#endif

#ifndef MIN_PART
#  define MIN_PART 3
   /* keep at least MIN_PART chars between dots in a file name. */
#endif

#ifndef EXPAND
#  define EXPAND(argc,argv)
#endif

#ifndef RECORD_IO
#  define RECORD_IO 0
#endif

#ifndef SET_BINARY_MODE
#  define SET_BINARY_MODE(fd)
#endif

#ifndef OPEN
#  define OPEN(name, flags, mode) open_safer (name, flags, mode)
#endif

#ifndef get_char
#  define get_char() get_byte()
#endif

#ifndef put_char
#  define put_char(c) put_byte(c)
#endif
