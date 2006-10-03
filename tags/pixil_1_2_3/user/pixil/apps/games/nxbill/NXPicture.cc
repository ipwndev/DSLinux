 /* Copyright (C) 200-2002 Century Embedded Techonlogies
 *
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

#include "NXPicture.h"
#include "objects.h"

extern char xbill_path[256];

NXPicture::~NXPicture()
{
    if (gc)
	GrDestroyGC(gc);

    if (image)
	GrFreeImage(image);

    if (pixmap)
	GrDestroyWindow(pixmap);
}

void
NXPicture::load(char *name, int index)
{
    GR_IMAGE_INFO iinfo;
    char file[255];

    if (index >= 0)
	sprintf(file, "%s%s_%d.xpm", xbill_path, name, index);
    else
	sprintf(file, "%s%s.xpm", xbill_path, name);

    /* Ok, load the pix */
    image = GrLoadImageFromFile(file, 0);

    if (image) {
	GrGetImageInfo(image, &iinfo);

	width = iinfo.width;
	height = iinfo.height;

	/* Create a new pixmap for off screen drawing */


	pixmap = GrNewPixmap(width + 3, height + 3, NULL);
	gc = GrNewGC();

	GrSetGCBackground(gc, WHITE);
	GrFillRect(pixmap, gc, 0, 0, width, height);

	/* And draw our boy there */
	GrDrawImageToFit(pixmap, gc, 0, 0, width, height, image);
    }
}
