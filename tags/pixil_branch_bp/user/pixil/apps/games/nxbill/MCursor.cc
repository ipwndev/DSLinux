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

#include "MCursor.h"
#include "objects.h"

void
MCursor::load(char *data, int width, height)
{
    static char *dir =
	strdup(access(XBILL_HOME "images/logo.xpm", R_OK) ? "" : XBILL_HOME);

    Pixmap bitmap, mask;
    int i, xh, yh;
    unsigned width, height;
    char file[255];
    char mfile[255];
    sprintf(file, "%simages/%s.xbm", dir, name);

    i = NXReadBitmapFile(ui.rootwindow, file, &width, &height, &bitmap, &xh,
			 &yh);

    i = XReadBitmapFile(ui.display, ui.rootwindow, file,
			&width, &height, &bitmap, &xh, &yh);

    if (i == BitmapOpenFailed) {
	printf("cannot open %s\n", file);
	exit(1);
    }
    if (masked == SEP_MASK) {
	sprintf(mfile, "%simages/%s_mask.xbm", dir, name);
	i = XReadBitmapFile(ui.display, ui.rootwindow,
			    mfile, &width, &height, &mask, &xh, &yh);
    } else
	mask = bitmap;
    if (i == BitmapOpenFailed) {
	printf("cannot open %s\n", file);
	exit(1);
    }
    cursor = XCreatePixmapCursor(ui.display, bitmap, mask,
				 &ui.black, &ui.white, width / 2, height / 2);
}
