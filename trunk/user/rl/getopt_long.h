/*
   getopt_long.h - definition of getopt_long() for systems that lack it

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


#ifndef _GETOPT_LONG_H
#define _GETOPT_LONG_H 1


#define no_argument            0
#define required_argument      1
#define optional_argument      2


struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};


/* this is a (poor) getopt_long() replacement for systems that don't have it
   (this is generaly a GNU extention)
   this implementation is by no meens flawless, especialy the optional arguments
   to options and options following filenames is not quite right, allso
   minimal error checking
   */
int getopt_long(int argc,char * const argv[],
                const char *optstring,
                const struct option *longopts,int *longindex);


#endif /* _GETOPT_LONG_H */
