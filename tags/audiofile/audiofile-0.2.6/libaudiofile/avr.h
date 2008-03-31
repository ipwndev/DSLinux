/*
	Audio File Library
	Copyright (C) 2004, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307  USA.
*/

/*
	avr.h

	This file contains headers and constants related to the AVR
	(Audio Visual Research) sound file format.
*/

#include "afinternal.h"

#ifndef AVR_H
#define AVR_H

bool _af_avr_recognize (AFvirtualfile *fh);
status _af_avr_read_init (AFfilesetup, AFfilehandle);
status _af_avr_write_init (AFfilesetup, AFfilehandle);
status _af_avr_update (AFfilehandle);
AFfilesetup _af_avr_complete_setup (AFfilesetup);

#endif
