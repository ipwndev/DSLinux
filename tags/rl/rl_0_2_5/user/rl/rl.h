/*
   rl.h - definitions of all functions

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


#ifndef _RL_H
#define _RL_H 1


/* from system.h: */
#ifdef ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else /* not ENABLE_NLS */
# undef bindtextdomain
# define bindtextdomain(Domain, Directory)
# undef textdomain
# define textdomain(Domain)
# define _(Text) Text
#endif /* not ENABLE_NLS */


/* the average length of a line */
#define AVGLINELEN 50


/* the size in which to load files */
#define BLOCKSIZE 8192


/* from rl.c */


/* flag to indicate output */
extern int quiet;


#endif /* _RL_H */
