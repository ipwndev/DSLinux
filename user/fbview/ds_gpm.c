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

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <gpm.h>

#include "ds_gpm.h"

/** gpm connection descriptor. */
Gpm_Connect conn;

/** mouse event. */
struct Gpm_Event gpm_ev;

/** read stream buffer for mouse */
fd_set rfds;

/** timeval, for select's timeout */
struct timeval tv = {0};

/** current position of mouse */
int curr_x = 1, curr_y = 1;

/** initialize device.
 *  @return 0 when success. 1 when failure.
 */
int gpm_init()
{
	/* Want to know only about mouse moving */
        conn.eventMask = GPM_MOVE;

	/* don't handle anything by default */
        conn.defaultMask = 0;

	/* want everything */
        conn.minMod = 0;

	/* all modifiers included */
        conn.maxMod = ~0;

	if(Gpm_Open(&conn, 0) == -1)
		return 1;
	else
		return 0;
}

/**
 * uninitialize device.
 * @return always zero
 */
int gpm_uninit()
{
	Gpm_Close();
	return 0;
}

/**
 * checks if mouse changed its position.
 * @return 1 when position is changed, otherwise 0
 */
int gpm_is_moved()
{
	int new_x, new_y;
	new_x = curr_x;
	new_y = curr_y;

	FD_SET (gpm_fd, &rfds);

	while (select(gpm_fd+1, &rfds, NULL, NULL, &tv) > 0) {
		Gpm_GetEvent (&gpm_ev);
		Gpm_FitEvent (&gpm_ev);
	
		if(gpm_ev.type & GPM_MOVE) {
			new_x = gpm_ev.x;
			new_y = gpm_ev.y;
		}
	}

	if (new_x == curr_x && new_y == curr_y) return 0;

	curr_x = new_x;
	curr_y = new_y;
	return 1;
}

/**
 * gets the current mouse position.
 * @return float value of position x,y belongs to [0.00; 1.00].
 */
void gpm_get_position(float * x, float * y)
{
	*x = (float)(curr_x-1) / (gpm_mx - 1);
	*y = (float)(curr_y-1) / (gpm_my - 1);
}
