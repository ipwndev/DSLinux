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
Library::load_pix()
{
    int i;
    char *name[] = { "wingdows", "apple", "next", "sgi", "sun", "os2",
	"bsd", "linux", "redhat", "hurd"
    };
    for (i = 0; i <= NUM_OS; i++) {
	os[i].load(name[i]);
#ifndef PDA
	if (i)
	    cursor[i].load(name[i], cursor[i].OWN_MASK);
#endif
    }
    width = os[0].width;
    height = os[0].height;
}
