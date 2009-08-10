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

#include <stdio.h>
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <unistd.h>

/** old and new keyboard setting */
static struct termios initial_settings, new_settings;

/** read stream buffer for keyboard */
fd_set kb_rfds;

/** timeval, for select's timeout */
struct timeval kb_tv = {0};

/**
 * Initializes keyboard.
 */
void init_keyboard()
{
	tcgetattr(0,&initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	
	new_settings.c_lflag &= ~ISIG;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	
	tcsetattr(0, TCSANOW, &new_settings);
}

/**
 * Uninitializes keyboard.
 */
void close_keyboard()
{
	tcsetattr(0, TCSANOW, &initial_settings);
}

/**
 * Checks if user has pressed a button.
 * @return 0 if not, 1 if he does.
 */
int kbhit()
{
	FD_SET (0, &kb_rfds);

	if (select(1, &kb_rfds, NULL, NULL, &kb_tv) > 0) return 1;
	else return 0;
}

/**
 * Returns the last pressed button by user.
 * @return Button code, that user has pressed.
 */
int readch()
{
	char ch[3];
	int nread;
	
	nread = read(0, &ch, 3);
	return (nread == 1 ? ch[0] : ch[2]);
}
