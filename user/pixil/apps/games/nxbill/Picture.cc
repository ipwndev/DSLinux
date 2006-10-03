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

#include "Picture.h"
#include "objects.h"

void
Picture::load(const char *name, int index)
{
    static char *dir =
	strdup(access(XBILL_HOME "images/logo.xpm", R_OK) ? "" : XBILL_HOME);
    int i;
    char file[255];
    Pixmap mask;
    XpmAttributes attr;
    unsigned long gcmask;
    XGCValues gcval;
    gcmask = GCForeground | GCBackground | GCGraphicsExposures;
    gcval.graphics_exposures = False;
    attr.valuemask = XpmCloseness | XpmReturnPixels | XpmColormap | XpmDepth;
    attr.closeness = 65535;
    attr.colormap = ui.colormap;
    attr.depth = ui.depth;
    if (index >= 0)
	sprintf(file, "%simages/%s_%d.xpm", dir, name, index);
    else
	sprintf(file, "%simages/%s.xpm", dir, name);
    i = XpmReadFileToPixmap(ui.display, ui.rootwindow, file, &pix,
			    &mask, &attr);
    if (i < 0) {
	printf("cannot open %s\n", file);
	exit(1);
    }
    gc = XCreateGC(ui.display, ui.offscreen, gcmask, &gcval);
    XSetClipMask(ui.display, gc, mask);
    width = attr.width;
    height = attr.height;
}
