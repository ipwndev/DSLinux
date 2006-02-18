/*
 *  Copyright (C) 2004 Nigel Horne <njh@bandsman.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Change History:
 * $Log$
 * Revision 1.1.2.1  2006/02/18 16:27:12  stsp2
 * adding uClinux-dist CVS snapshot from 2005-10-14
 *
 * Revision 1.3  2005/03/22 21:26:27  kojm
 * add support for old fashioned tar archives
 *
 * Revision 1.2  2004/09/05 18:58:22  nigelhorne
 * Extract files completed
 *
 * Revision 1.1  2004/09/05 15:28:10  nigelhorne
 * First draft
 *
 */
int cli_untar(const char *dir, int desc, unsigned int posix);
