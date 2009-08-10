/*
 *  (C) Copyright 2008-2009 Kamil Kopec <kamil_kopec@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ds_keyboard_h
#define ds_keyboard_h

#include <stdio.h>

#define DS_KEY_ESC 27
#define DS_KEY_UP 119
#define DS_KEY_DOWN 115
#define DS_KEY_LEFT 97
#define DS_KEY_RIGHT 100

#define DS_ARROW_UP 65
#define DS_ARROW_DOWN 66
#define DS_ARROW_LEFT 68
#define DS_ARROW_RIGHT 67

/**
 * Initializes keyboard.
 */
extern void init_keyboard();

/**
 * Uninitializes keyboard.
 */
extern void close_keyboard();

/**
 * Checks if user has pressed a button.
 * @return 0 if not, 1 if he does.
 */
extern int kbhit();

/**
 * Returns the last pressed button by user.
 * @return Button code, that user has pressed.
 */
extern int readch();

#endif
