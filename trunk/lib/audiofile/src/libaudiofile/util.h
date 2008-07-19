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
	util.h

	This file contains some general utility functions for the Audio
	File Library.
*/

#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include "audiofile.h"
#include "afinternal.h"

bool _af_filesetup_ok (AFfilesetup setup);
bool _af_filehandle_ok (AFfilehandle file);

bool _af_filehandle_can_read (AFfilehandle file);

void *_af_malloc (size_t size);
void *_af_realloc (void *ptr, size_t size);
void *_af_calloc (size_t nmemb, size_t size);
char *_af_strdup (char *s);

AUpvlist _af_pv_long (long val);
AUpvlist _af_pv_double (double val);
AUpvlist _af_pv_pointer (void *val);

bool _af_pv_getlong (AUpvlist pvlist, int param, long *l);
bool _af_pv_getdouble (AUpvlist pvlist, int param, double *d);
bool _af_pv_getptr (AUpvlist pvlist, int param, void **v);

_TrackSetup *_af_filesetup_get_tracksetup (AFfilesetup setup, int trackid);
_Track *_af_filehandle_get_track (AFfilehandle file, int trackid);

bool _af_unique_ids (int *ids, int nids, char *idname, int iderr);

float _af_format_frame_size (_AudioFormat *format, bool stretch3to4);
int _af_format_frame_size_uncompressed (_AudioFormat *format, bool stretch3to4);
float _af_format_sample_size (_AudioFormat *format, bool stretch3to4);
int _af_format_sample_size_uncompressed (_AudioFormat *format, bool stretch3to4);

status _af_set_sample_format (_AudioFormat *f, int sampleFormat, int sampleWidth);

bool _af_filehandle_can_read (AFfilehandle file);
bool _af_filehandle_can_write (AFfilehandle file);

status af_read_uint32_be (u_int32_t *value, AFvirtualfile *vf);
status af_read_uint32_le (u_int32_t *value, AFvirtualfile *vf);
status af_read_uint16_be (u_int16_t *value, AFvirtualfile *vf);
status af_read_uint16_le (u_int16_t *value, AFvirtualfile *vf);

status af_write_uint32_be (const u_int32_t *value, AFvirtualfile *vf);
status af_write_uint32_le (const u_int32_t *value, AFvirtualfile *vf);
status af_write_uint16_be (const u_int16_t *value, AFvirtualfile *vf);
status af_write_uint16_le (const u_int16_t *value, AFvirtualfile *vf);

#endif
