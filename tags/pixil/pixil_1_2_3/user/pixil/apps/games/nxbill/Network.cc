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

/*sets up network for each level*/
void
Network::setup()
{
    int i;
    units = on(game.level);

    i = 0;

    while (1) {
	if (i == units)
	    break;

	if (!net.computers[i].setup(i))
	    units--;
	else
	    i++;
    }

    base = units;

    off = win = 0;
    ncables = game.MIN(game.level, units / 2);
    for (i = 0; i < ncables; i++)
	cables[i].setup();
}

/*redraws the computers at their location with the proper image*/
void
Network::draw()
{
    int i;
    for (i = 0; i < ncables; i++)
	cables[i].draw();
    for (i = 0; i < units; i++)
	computers[i].draw();
}

void
Network::update()
{
    for (int i = 0; i < ncables; i++)
	cables[i].update();
}

void
Network::toasters()
{
    for (int i = 0; i < units; i++) {
	computers[i].type = computers[i].TOASTER;
	computers[i].os = OS.OFF;
    }
    ncables = 0;
}

int
Network::on(int lev)
{
    return game.MIN(8 + lev, MAX_COMPUTERS);
}

void
Network::load_pix()
{
    int i;
    char *name[] = { "toaster", "maccpu", "nextcpu", "sgicpu", "suncpu",
	"os2cpu", "bsdcpu"
    };
    for (i = 0; i <= NUM_SYS; i++)
	pictures[i].load(name[i]);
    width = pictures[0].width;
    height = pictures[0].height;
}
