/*
 *  $Id$
 *
 *  version.c -- dialog's version string
 *
 *  Copyright 2003,2005	Thomas E. Dickey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to
 *	Free Software Foundation, Inc.
 *	51 Franklin St., Fifth Floor
 *	Boston, MA 02110, USA.
 */
#include <dialog.h>

#define quoted(a)	#a
#define concat(a,b)	a "-" quoted(b)
#define DLG_VERSION	concat(DIALOG_VERSION,DIALOG_PATCHDATE)

char *
dialog_version(void)
{
    return DLG_VERSION;
}
