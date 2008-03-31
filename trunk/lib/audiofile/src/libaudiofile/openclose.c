/*
	Audio File Library
	Copyright (C) 2000-2001, Silicon Graphics, Inc.

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <audiofile.h>

#include "afinternal.h"
#include "units.h"
#include "util.h"
#include "modules.h"

#if defined(WIN32) || defined(__CYGWIN__)
#include <io.h>
#include <fcntl.h>
#define SETBINARYMODE(fp) 	_setmode(_fileno(fp), _O_BINARY)
#else
#define SETBINARYMODE(x)
#endif /* WIN32 || __CYGWIN__ */

extern _Unit _af_units[];

static void freeFileHandle (AFfilehandle filehandle);
static void freeInstParams (AFPVu *values, int fileFormat);
static status _afOpenFile (int access, AFvirtualfile *vf, const char *filename,
	AFfilehandle *file, AFfilesetup filesetup);

int _af_identify (AFvirtualfile *vf, int *implemented)
{
	AFfileoffset	curpos;
	int		i;

	curpos = af_ftell(vf);

	for (i=0; i<_AF_NUM_UNITS; i++)
	{
		if (_af_units[i].read.recognize &&
			_af_units[i].read.recognize(vf))
		{
			if (implemented != NULL)
				*implemented = _af_units[i].implemented;
			af_fseek(vf, curpos, SEEK_SET);
			return _af_units[i].fileFormat;
		}
	}

	af_fseek(vf, curpos, SEEK_SET);

	if (implemented != NULL)
		*implemented = AF_FALSE;

	return AF_FILE_UNKNOWN;
}

int afIdentifyFD (int fd)
{
	FILE		*fp;
	AFvirtualfile	*vf;
	int		result;

	/*
		Duplicate the file descriptor since otherwise the
		original file descriptor would get closed when we close
		the virtual file below.
	*/
	fd = dup(fd);

	fp = fdopen(fd, "r");
	if (fp == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file");
		return AF_FILE_UNKNOWN;
	}

	SETBINARYMODE(fp);

	vf = af_virtual_file_new_for_file(fp);
	if (vf == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file");
		return AF_FILE_UNKNOWN;
	}

	result = _af_identify(vf, NULL);

	af_fclose(vf);

	return result;
}

int afIdentifyNamedFD (int fd, const char *filename, int *implemented)
{
	FILE		*fp;
	AFvirtualfile	*vf;
	int		result;

	/*
		Duplicate the file descriptor since otherwise the
		original file descriptor would get closed when we close
		the virtual file below.
	*/
	fd = dup(fd);

	fp = fdopen(fd, "r");
	if (fp == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_FILE_UNKNOWN;
	}

	SETBINARYMODE(fp);

	vf = af_virtual_file_new_for_file(fp);
	if (vf == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_FILE_UNKNOWN;
	}

	result = _af_identify(vf, implemented);

	af_fclose(vf);

	return result;
}

AFfilehandle afOpenFD (int fd, const char *mode, AFfilesetup setup)
{
	FILE		*fp;
	AFvirtualfile	*vf;
	AFfilehandle	filehandle;
	int		access;

	if (mode == NULL)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	if (mode[0] == 'r')
		access = _AF_READ_ACCESS;
	else if (mode[0] == 'w')
		access = _AF_WRITE_ACCESS;
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	if ((fp = fdopen(fd, mode)) == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file");
		return AF_NULL_FILEHANDLE;
	}

	SETBINARYMODE(fp);

	vf = af_virtual_file_new_for_file(fp);

	if (_afOpenFile(access, vf, NULL, &filehandle, setup) != AF_SUCCEED)
		af_fclose(vf);

	return filehandle;
}

AFfilehandle afOpenNamedFD (int fd, const char *mode, AFfilesetup setup,
	const char *filename)
{
	FILE		*fp;
	AFvirtualfile	*vf;
	AFfilehandle	filehandle;
	int		access;

	if (mode == NULL)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	if (mode[0] == 'r')
		access = _AF_READ_ACCESS;
	else if (mode[0] == 'w')
		access = _AF_WRITE_ACCESS;
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	if ((fp = fdopen(fd, mode)) == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_NULL_FILEHANDLE;
	}

	SETBINARYMODE(fp);

	vf = af_virtual_file_new_for_file(fp);

	if (_afOpenFile(access, vf, filename, &filehandle, setup) != AF_SUCCEED)
		af_fclose(vf);

	return filehandle;
}

AFfilehandle afOpenFile (const char *filename, const char *mode, AFfilesetup setup)
{
	FILE		*fp;
	AFvirtualfile	*vf;
	AFfilehandle	filehandle;
	int		access;

	if (mode == NULL)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	if (mode[0] == 'r')
		access = _AF_READ_ACCESS;
	else if (mode[0] == 'w')
		access = _AF_WRITE_ACCESS;
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	if ((fp = fopen(filename, mode)) == NULL)
	{
		_af_error(AF_BAD_OPEN, "could not open file '%s'", filename);
		return AF_NULL_FILEHANDLE;
	}

	SETBINARYMODE(fp);

	vf = af_virtual_file_new_for_file(fp);

	if (_afOpenFile(access, vf, filename, &filehandle, setup) != AF_SUCCEED)
		af_fclose(vf);

	return filehandle;
}

AFfilehandle afOpenVirtualFile (AFvirtualfile *vfile, const char *mode,
	AFfilesetup setup)
{
	AFfilehandle	filehandle;
	int		access; 

	if (vfile == NULL)
	{
		_af_error(AF_BAD_FILEHANDLE, "null virtual filehandle");
		return AF_NULL_FILEHANDLE;
	}

	if (mode == NULL)
	{
		_af_error(AF_BAD_ACCMODE, "null access mode");
		return AF_NULL_FILEHANDLE;
	}

	if (mode[0] == 'r')
		access = _AF_READ_ACCESS;
	else if (mode[0] == 'w')
		access = _AF_WRITE_ACCESS;
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode '%s'", mode);
		return AF_NULL_FILEHANDLE;
	}

	if (_afOpenFile(access, vfile, NULL, &filehandle, setup) != AF_SUCCEED)
		af_fclose(vfile);

	return filehandle;
}

static status _afOpenFile (int access, AFvirtualfile *vf, const char *filename,
	AFfilehandle *file, AFfilesetup filesetup)
{
	int	fileFormat = AF_FILE_UNKNOWN;
	bool	implemented = AF_TRUE;
	char	*formatName;
	status	(*initfunc) (AFfilesetup, AFfilehandle);

	int		userSampleFormat = 0;
	double		userSampleRate = 0.0;
	_PCMInfo	userPCM;
	bool		userFormatSet = AF_FALSE;

	int	t;

	AFfilehandle	filehandle = AF_NULL_FILEHANDLE;
	AFfilesetup	completesetup = AF_NULL_FILESETUP;

	*file = AF_NULL_FILEHANDLE;

	if (access == _AF_WRITE_ACCESS || filesetup != AF_NULL_FILESETUP)
	{
		if (!_af_filesetup_ok(filesetup))
			return AF_FAIL;

		fileFormat = filesetup->fileFormat;
		if (access == _AF_READ_ACCESS && fileFormat != AF_FILE_RAWDATA)
		{
			_af_error(AF_BAD_FILESETUP,
				"warning: opening file for read access: "
				"ignoring file setup with non-raw file format");
			filesetup = AF_NULL_FILESETUP;
			fileFormat = _af_identify(vf, &implemented);
		}
	}
	else if (filesetup == AF_NULL_FILESETUP)
		fileFormat = _af_identify(vf, &implemented);

	if (fileFormat == AF_FILE_UNKNOWN)
	{
		if (filename != NULL)
			_af_error(AF_BAD_NOT_IMPLEMENTED,
				"'%s': unrecognized audio file format",
				filename);
		else
			_af_error(AF_BAD_NOT_IMPLEMENTED,
				"unrecognized audio file format");
		return AF_FAIL;
	}

	formatName = _af_units[fileFormat].name;

	if (implemented == AF_FALSE)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED,
			"%s format not currently supported", formatName);
	}

	assert(_af_units[fileFormat].completesetup != NULL);
	assert(_af_units[fileFormat].read.init != NULL);

	if (access == _AF_WRITE_ACCESS &&
		_af_units[fileFormat].write.init == NULL)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED,
			"%s format is currently supported for reading only",
			formatName);
		return AF_FAIL;
	}

	completesetup = NULL;
	if (filesetup != AF_NULL_FILESETUP)
	{
		userSampleFormat = filesetup->tracks[0].f.sampleFormat;
		userPCM = filesetup->tracks[0].f.pcm;
		userSampleRate = filesetup->tracks[0].f.sampleRate;
		userFormatSet = AF_TRUE;
		if ((completesetup = _af_units[fileFormat].completesetup(filesetup)) == NULL)
			return AF_FAIL;
	}

	filehandle = _af_malloc(sizeof (_AFfilehandle));
	if (filehandle == NULL)
	{
		if (completesetup)
			afFreeFileSetup(completesetup);
		return AF_FAIL;
	}
	memset(filehandle, 0, sizeof (_AFfilehandle));

	filehandle->valid = _AF_VALID_FILEHANDLE;
	filehandle->fh = vf;
	filehandle->access = access;
	if (filename != NULL)
		filehandle->fileName = strdup(filename);
	else
		filehandle->fileName = NULL;
	filehandle->fileFormat = fileFormat;
	filehandle->formatSpecific = NULL;

	initfunc = (access == _AF_READ_ACCESS) ?
		_af_units[fileFormat].read.init : _af_units[fileFormat].write.init;

	if (initfunc(completesetup, filehandle) != AF_SUCCEED)
	{
		freeFileHandle(filehandle);
		filehandle = AF_NULL_FILEHANDLE;
		if (completesetup)
			afFreeFileSetup(completesetup);
		return AF_FAIL;
	}

	if (completesetup)
	{
		afFreeFileSetup(completesetup);
		completesetup = NULL;
	}

	/*
		Initialize virtual format.
	*/
	for (t=0; t<filehandle->trackCount; t++)
	{
		_Track	*track = &filehandle->tracks[t];

		track->v = track->f;

		if (userFormatSet)
		{
			track->v.sampleFormat = userSampleFormat;
			track->v.pcm = userPCM;
			track->v.sampleRate = userSampleRate;
		}

		track->v.compressionType = AF_COMPRESSION_NONE;
		track->v.compressionParams = NULL;

#if WORDS_BIGENDIAN
		track->v.byteOrder = AF_BYTEORDER_BIGENDIAN;
#else
		track->v.byteOrder = AF_BYTEORDER_LITTLEENDIAN;
#endif

		if (_AFinitmodules(filehandle, track) == AF_FAIL)
		{
			freeFileHandle(filehandle);
			return AF_FAIL;
		}
	}

	*file = filehandle;

	return AF_SUCCEED;
}

int afSyncFile (AFfilehandle handle)
{
	if (!_af_filehandle_ok(handle))
		return -1;

	if (handle->access == _AF_WRITE_ACCESS)
	{
		int	filefmt = handle->fileFormat;
		int	trackno;

		/* Finish writes on all tracks. */
		for (trackno = 0; trackno < handle->trackCount; trackno++)
		{
			_Track	*track = &handle->tracks[trackno];

			if (track->ms.modulesdirty)
			{
				if (_AFsetupmodules(handle, track) == AF_FAIL)
					return -1;
			}

			if (_AFsyncmodules(handle, track) != AF_SUCCEED)
				return -1;
		}

		/* Update file headers. */
		if (_af_units[filefmt].write.update != NULL &&
			_af_units[filefmt].write.update(handle) != AF_SUCCEED)
			return AF_FAIL;
	}
	else if (handle->access == _AF_READ_ACCESS)
	{
		/* Do nothing. */
	}
	else
	{
		_af_error(AF_BAD_ACCMODE, "unrecognized access mode %d",
			handle->access);
		return AF_FAIL;
	}

	return AF_SUCCEED;
}

int afCloseFile (AFfilehandle file)
{
	int	err;

	if (!_af_filehandle_ok(file))
		return -1;

	afSyncFile(file);

	err = af_fclose(file->fh);
	if (err < 0)
		_af_error(AF_BAD_CLOSE, "close returned %d", err);

	freeFileHandle(file);

	return 0;
}

static void freeFileHandle (AFfilehandle filehandle)
{
	int	fileFormat;
	if (filehandle == NULL || filehandle->valid != _AF_VALID_FILEHANDLE)
	{
		_af_error(AF_BAD_FILEHANDLE, "bad filehandle");
		return;
	}

	filehandle->valid = 0;

	if (filehandle->fileName != NULL)
		free(filehandle->fileName);

	fileFormat = filehandle->fileFormat;

	if (filehandle->formatSpecific != NULL)
	{
		free(filehandle->formatSpecific);
		filehandle->formatSpecific = NULL;
	}

	if (filehandle->tracks)
	{
		int	i;
		for (i=0; i<filehandle->trackCount; i++)
		{
			/* Free the compression parameters. */
			if (filehandle->tracks[i].f.compressionParams)
			{
				AUpvfree(filehandle->tracks[i].f.compressionParams);
				filehandle->tracks[i].f.compressionParams = AU_NULL_PVLIST;
			}

			if (filehandle->tracks[i].v.compressionParams)
			{
				AUpvfree(filehandle->tracks[i].v.compressionParams);
				filehandle->tracks[i].v.compressionParams = AU_NULL_PVLIST;
			}

			/* Free the track's modules. */

			_AFfreemodules(&filehandle->tracks[i]);

			if (filehandle->tracks[i].channelMatrix)
			{
				free(filehandle->tracks[i].channelMatrix);
				filehandle->tracks[i].channelMatrix = NULL;
			}

			if (filehandle->tracks[i].markers)
			{
				int	j;
				for (j=0; j<filehandle->tracks[i].markerCount; j++)
				{
					if (filehandle->tracks[i].markers[j].name)
					{
						free(filehandle->tracks[i].markers[j].name);
						filehandle->tracks[i].markers[j].name = NULL;
					}
					if (filehandle->tracks[i].markers[j].comment)
					{
						free(filehandle->tracks[i].markers[j].comment);
						filehandle->tracks[i].markers[j].comment = NULL;
					}

				}

				free(filehandle->tracks[i].markers);
				filehandle->tracks[i].markers = NULL;
			}
		}

		free(filehandle->tracks);
		filehandle->tracks = NULL;
	}
	filehandle->trackCount = 0;

	if (filehandle->instruments)
	{
		int	i;
		for (i=0; i<filehandle->instrumentCount; i++)
		{
			if (filehandle->instruments[i].loops)
			{
				free(filehandle->instruments[i].loops);
				filehandle->instruments[i].loops = NULL;
			}
			filehandle->instruments[i].loopCount = 0;

			if (filehandle->instruments[i].values)
			{
				freeInstParams(filehandle->instruments[i].values, fileFormat);
				filehandle->instruments[i].values = NULL;
			}
		}
		free(filehandle->instruments);
		filehandle->instruments = NULL;
	}
	filehandle->instrumentCount = 0;

	if (filehandle->miscellaneous)
	{
		free(filehandle->miscellaneous);
		filehandle->miscellaneous = NULL;
	}
	filehandle->miscellaneousCount = 0;

	memset(filehandle, 0, sizeof (_AFfilehandle));
	free(filehandle);
}

static void freeInstParams (AFPVu *values, int fileFormat)
{
	int	i;
	int	parameterCount = _af_units[fileFormat].instrumentParameterCount;

	for (i=0; i<parameterCount; i++)
	{
		if (_af_units[fileFormat].instrumentParameters[i].type == AU_PVTYPE_PTR)
			if (values[i].v != NULL)
				free(values[i].v);
	}

	free(values);
}
