/*
	Audio File Library
	Copyright (C) 1999-2000, Michael Pruett <michael@68k.org>
	Copyright (C) 2000, Silicon Graphics, Inc.

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
	compression.c

	This file contains routines for configuring compressed audio.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include "audiofile.h"
#include "afinternal.h"
#include "aupvlist.h"
#include "units.h"
#include "util.h"

extern _CompressionUnit _af_compression[];

int _af_compression_index_from_id (int compressionid)
{
	int	i;

	for (i=0; i<_AF_NUM_COMPRESSION; i++)
	{
		if (_af_compression[i].compressionID == compressionid)
			return i;
	}

	_af_error(AF_BAD_COMPTYPE, "compression type %d not available", compressionid);

	return -1;
}

static _CompressionUnit *findCompression (int compressionid)
{
	int	compressionno;

	compressionno = _af_compression_index_from_id(compressionid);
	if (compressionno != -1)
		return &_af_compression[compressionno];

	return NULL;
}

int afGetCompression (AFfilehandle file, int trackid)
{
	_Track	*track;

	if (!_af_filehandle_ok(file))
		return -1;

	if ((track = _af_filehandle_get_track(file, trackid)) == NULL)
		return -1;

	return track->f.compressionType;
}

void afInitCompression (AFfilesetup setup, int trackid, int compression)
{
	_TrackSetup	*track;

	if (!_af_filesetup_ok(setup))
		return;

	if ((track = _af_filesetup_get_tracksetup(setup, trackid)) == NULL)
		return;

	if (findCompression(compression) == NULL)
		return;

	track->f.compressionType = compression;
}

#if 0
int afGetCompressionParams (AFfilehandle file, int trackid,
	int *compression, AUpvlist pvlist, int numitems)
{
	assert(file);
	assert(trackid == AF_DEFAULT_TRACK);
}

void afInitCompressionParams (AFfilesetup setup, int trackid,
	int compression, AUpvlist pvlist, int numitems)
{
	assert(setup);
	assert(trackid == AF_DEFAULT_TRACK);
}
#endif
