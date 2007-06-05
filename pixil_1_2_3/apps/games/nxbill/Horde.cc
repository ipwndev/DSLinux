 /*
 * This program is free software; you can redistribute it and/or modify
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

void
Horde::setup()
{
    for (int i = 0; i < MAX_BILLS; i++)
	list[i].state = list[i].OFF;
    off_screen = on(game.level);
    on_screen = 0;
}

/*  Launches Bills whenever called  */
void
Horde::launch(int max)
{
    int i, n;
    if (!max || !off_screen)
	return;
    n = game.RAND(1, game.MIN(max, off_screen));
    for (i = 0; n; n--) {
	for (i++; i < MAX_BILLS; i++)
	    if (list[i].state == list[i].OFF)
		break;
	if (i == MAX_BILLS)
	    return;
	list[i++].enter();
    }
}

int
Horde::on(unsigned int lev)
{
    return game.MIN(8 + 3 * lev, MAX_BILLS);
}

int
Horde::max_at_once(unsigned int lev)
{
    return game.MIN(2 + lev / 4, 12);
}

int
Horde::between(unsigned int lev)
{
    return game.MAX(14 - lev / 3, 10);
}

void
Horde::load_pix()
{
    int i;
    for (i = 0; i < WCELS - 1; i++) {
	lcels[i].load("billL", i);
	rcels[i].load("billR", i);
    }
    lcels[WCELS - 1] = lcels[1];
    rcels[WCELS - 1] = rcels[1];

    for (i = 0; i < DCELS; i++)
	dcels[i].load("billD", i);
    width = dcels[0].width;
    height = dcels[0].height;

    for (i = 0; i < ACELS; i++)
	acels[i].load("billA", i);
}

void
Horde::update()
{
    int i;
    if (!(game.iteration % between(game.level)))
	launch(max_at_once(game.level));
    for (i = 0; i < MAX_BILLS; i++)
	list[i].update();
}

void
Horde::draw()
{
    int i;
    for (i = 0; i < MAX_BILLS; i++)
	list[i].draw();
}
