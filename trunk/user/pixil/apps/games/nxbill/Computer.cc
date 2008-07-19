 /* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "objects.h"

int
Computer::check_intersect(int i, int x, int y)
{
    for (int j = 0; j < i; j++) {
	if (game.INTERSECT(x, y,
			   net.width - bill.list[0].XOFFSET + bill.width,
			   net.height, net.computers[j].x,
			   net.computers[j].y,
			   net.width - bill.list[0].XOFFSET + bill.width,
			   net.height))
	    return (j);
    }

    return (-1);
}

int
Computer::setup(int i)
{
    int count;
    int p;

    for (count = 0; count < 4000; count++) {
	/* Get an inital X and Y for the computer */
	x = game.RAND(BORDER, game.playwidth - BORDER - net.width);
	y = game.RAND(BORDER, game.playheight - BORDER - net.height);

	/* Check to see if we intersect */
	if ((p = check_intersect(i, x, y)) == -1)
	    break;

	/* Try to shift X one way or the other */

	if (x > (BORDER + net.width))
	    x = x - net.width;
	else
	    x = x + net.width;

	if ((p = check_intersect(i, x, y)) == -1)
	    break;

	/* One more check, try our y */

	if (y > (BORDER + net.height))
	    y = y - net.height;
	else
	    y = y + net.height;

	if ((p = check_intersect(i, x, y)) == -1)
	    break;

	/* Doh!  Go back around and try to pick an entirely new x and y */
    }

    if (count == 4000) {
	return (0);
    }

    type = game.RAND(1, net.NUM_SYS);
    os = determineOS();
    busy = 0;
    return (1);
}

#ifdef NOTUSED
int
Computer::setup(int i)
{
    int j, counter = 0, flag;
    do {
	if (++counter > 4000)
	    return 0;
	x = game.RAND(BORDER, game.playwidth - BORDER - net.width);
	y = game.RAND(BORDER, game.playheight - BORDER - net.height);
	flag = 1;
	/*checks for conflicting computer placement */
	for (j = 0; j < i && flag; j++)
	    if (game.INTERSECT(x, y,
			       net.width - bill.list[0].XOFFSET + bill.width,
			       net.height, net.computers[j].x,
			       net.computers[j].y,
			       net.width - bill.list[0].XOFFSET + bill.width,
			       net.height))
		flag = 0;
    } while (!flag);
    type = game.RAND(1, net.NUM_SYS);
    os = determineOS();
    busy = 0;
    return 1;
}
#endif

int
Computer::find_stray()
{
    int i;
    for (i = 0; i < bill.MAX_BILLS; i++) {
	if (bill.list[i].state != bill.list[i].STRAY)
	    continue;
	if (game.INTERSECT(x, y, net.width, net.height, bill.list[i].x,
			   bill.list[i].y, OS.width, OS.height))
	    return i;
    }
    return -1;
}

int
Computer::oncomputer(int locx, int locy)
{
    return (abs(locx - x) < net.width && abs(locy - y) < net.height);
}

int
Computer::compatible(int system)
{
    return (type == system || (type >= PC && system >= OS.PC));
}

int
Computer::determineOS()
{
    if (type < PC)
	return type;
    else
	return game.RAND(OS.PC, OS.NUM_OS);
}

void
Computer::draw()
{
    ui.draw(&(net.pictures[type]), x, y);
    if (os != OS.OFF)
	ui.draw(&(OS.os[os]), x + OFFSET, y + OFFSET);
}
