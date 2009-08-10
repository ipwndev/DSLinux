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

#ifndef ds_gpm_h
#define ds_gpm_h

#define GPM_SUPPORT

/** initialize device.
 *  @return 0 when success. 1 when failure.
 */
extern int gpm_init();

/**
 * uninitialize device.
 * @return always zero
 */
extern int gpm_uninit();

/**
 * checks if mouse changed its position.
 * @return 1 when position is changed, otherwise 0
 */
extern int gpm_is_moved();

/**
 * gets the current mouse position.
 * @return float value of position x,y belongs to [0.00; 1.00].
 */
extern void gpm_get_position(float * x, float * y);

#endif
