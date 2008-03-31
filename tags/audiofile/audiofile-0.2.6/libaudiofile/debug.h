/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>

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
	debug.h

	This header file declares debugging functions for the Audio
	File Library.
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <sys/types.h>
#include "audiofile.h"
#include "afinternal.h"

void _af_printid (u_int32_t id);
void _af_print_filehandle (AFfilehandle filehandle);
void _af_print_tracks (AFfilehandle filehandle);
void _af_print_channel_matrix (double *matrix, int fchans, int vchans);
void _af_print_pvlist (AUpvlist list);

void _af_print_audioformat (_AudioFormat *format);
void _af_print_chunk (_AFchunk *chunk);
void _af_print_frame (AFframecount frameno, double *frame, int nchannels,
	char *formatstring, int numberwidth,
	double slope, double intercept, double minclip, double maxclip);

#endif
