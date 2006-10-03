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

int
Spark::delay(unsigned int lev)
{
    return game.MAX(20 - lev, 0);
}

void
Spark::load_pix()
{
    int i;
    for (i = 0; i < 2; i++)
	pictures[i].load("spark", i);
    width = pictures[0].width;
    height = pictures[0].height;
}
