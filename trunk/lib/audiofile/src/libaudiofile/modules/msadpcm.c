/*
	Audio File Library
	Copyright (C) 2001, Silicon Graphics, Inc.

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
	msadpcm.c

	This module implements Microsoft ADPCM compression.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <assert.h>

#include <audiofile.h>

#include "afinternal.h"
#include "modules.h"
#include "units.h"
#include "compression.h"
#include "byteorder.h"
#include "util.h"

#include "msadpcm.h"

#define CHNK(X)

static _AFmodule ms_adpcm_decompress;

typedef struct ms_adpcm_state
{
	u_int8_t	predictor;
	u_int16_t	delta;
	int16_t		sample1, sample2;
} ms_adpcm_state;

typedef struct ms_adpcm_data
{
	_Track *track;
	AFvirtualfile *fh;

	/*
		We set framesToIgnore during a reset1 and add it to
		framesToIgnore during a reset2.
	*/
	AFframecount	framesToIgnore;

	int		blockAlign, samplesPerBlock;

	/* a is an array of numCoefficients ADPCM coefficient pairs. */
	int	numCoefficients;
	int16_t	coefficients[256][2];
} ms_adpcm_data;

/*
	Compute a linear PCM value from the given differential coded
	value.
*/
static int16_t ms_adpcm_decode_sample (struct ms_adpcm_state *state,
	u_int8_t code, const int16_t *coefficient)
{
	const int32_t MAX_INT16 = 32767, MIN_INT16 = -32768;
	const int32_t adaptive[] =
	{
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};
	int32_t	linearSample, delta;

	linearSample = ((state->sample1 * coefficient[0]) +
		(state->sample2 * coefficient[1])) / 256;

	if (code & 0x08)
		linearSample += state->delta * (code-0x10);
	else
		linearSample += state->delta * code;

	/* Clamp linearSample to a signed 16-bit value. */
	if (linearSample < MIN_INT16)
		linearSample = MIN_INT16;
	else if (linearSample > MAX_INT16)
		linearSample = MAX_INT16;

	delta = ((int32_t) state->delta * adaptive[code])/256;
	if (delta < 16)
	{
		delta = 16;
	}

	state->delta = delta;
	state->sample2 = state->sample1;
	state->sample1 = linearSample;

	/*
		Because of earlier range checking, new_sample will be
		in the range of an int16_t.
	*/
	return (int16_t) linearSample;
}

/* Decode one block of MS ADPCM data. */
static int ms_adpcm_decode_block (ms_adpcm_data *msadpcm, u_int8_t *encoded,
	int16_t *decoded)
{
	int		i, outputLength, samplesRemaining;
	int		channelCount;
	int16_t		*coefficient[2];
	ms_adpcm_state	decoderState[2];
	ms_adpcm_state	*state[2];

	/* Calculate the number of bytes needed for decoded data. */
	outputLength = msadpcm->samplesPerBlock * sizeof (int16_t) *
		msadpcm->track->f.channelCount;

	channelCount = msadpcm->track->f.channelCount;

	state[0] = &decoderState[0];
	if (channelCount == 2)
		state[1] = &decoderState[1];
	else
		state[1] = &decoderState[0];

	/* Initialize predictor. */
	for (i=0; i<channelCount; i++)
	{
		state[i]->predictor = *encoded++;
		assert(state[i]->predictor < msadpcm->numCoefficients);
	}

	/* Initialize delta. */
	for (i=0; i<channelCount; i++)
	{
		state[i]->delta = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (u_int16_t);
	}

	/* Initialize first two samples. */
	for (i=0; i<channelCount; i++)
	{
		state[i]->sample1 = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (u_int16_t);
	}

	for (i=0; i<channelCount; i++)
	{
		state[i]->sample2 = (encoded[1]<<8) | encoded[0];
		encoded += sizeof (u_int16_t);
	}

	coefficient[0] = msadpcm->coefficients[state[0]->predictor];
	coefficient[1] = msadpcm->coefficients[state[1]->predictor];

	for (i=0; i<channelCount; i++)
		*decoded++ = state[i]->sample2;

	for (i=0; i<channelCount; i++)
		*decoded++ = state[i]->sample1;

	/*
		The first two samples have already been 'decoded' in
		the block header.
	*/
	samplesRemaining = (msadpcm->samplesPerBlock - 2) *
		msadpcm->track->f.channelCount;

	while (samplesRemaining > 0)
	{
		u_int8_t	code;
		int16_t		newSample;

		code = *encoded >> 4;
		newSample = ms_adpcm_decode_sample(state[0], code,
			coefficient[0]);
		*decoded++ = newSample;

		code = *encoded & 0x0f;
		newSample = ms_adpcm_decode_sample(state[1], code,
			coefficient[1]);
		*decoded++ = newSample;

		encoded++;
		samplesRemaining -= 2;
	}

	return outputLength;
}

bool _af_ms_adpcm_format_ok (_AudioFormat *f)
{
	if (f->channelCount != 1 && f->channelCount != 2)
	{
		_af_error(AF_BAD_COMPRESSION,
		       "MS ADPCM compression requires 1 or 2 channels");
		return AF_FALSE;
	}

	if (f->sampleFormat != AF_SAMPFMT_TWOSCOMP || f->sampleWidth != 16)
	{
		_af_error(AF_BAD_COMPRESSION,
		       "MS ADPCM compression requires 16-bit signed integer format");
		f->sampleFormat = AF_SAMPFMT_TWOSCOMP;
		f->sampleWidth = 16;
		/* non-fatal */
	}

	if (f->byteOrder != AF_BYTEORDER_BIGENDIAN)
	{
		_af_error(AF_BAD_COMPRESSION,
		       "MS ADPCM compression requires big endian format");
		f->byteOrder = AF_BYTEORDER_BIGENDIAN;
		/* non-fatal */
	}

	return AF_TRUE;
}

static void ms_adpcm_decompress_describe (_AFmoduleinst *i)
{
/*	XXXmpruett this is probably the correct way to go, but other things
	need to be changed first.

	i->outc->f.byteOrder = _AF_BYTEORDER_NATIVE;
*/
	i->outc->f.compressionType = AF_COMPRESSION_NONE;
	i->outc->f.compressionParams = AU_NULL_PVLIST;
}

_AFmoduleinst _af_ms_adpcm_init_decompress (_Track *track, AFvirtualfile *fh,
	bool seekok, bool headerless, AFframecount *chunkframes)
{
	_AFmoduleinst	ret = _AFnewmodinst(&ms_adpcm_decompress);
	ms_adpcm_data	*d;
	AUpvlist	pv;
	int		i;
	long		l;
	void		*v;

	assert(af_ftell(fh) == track->fpos_first_frame);

	d = (ms_adpcm_data *) _af_malloc(sizeof (ms_adpcm_data));

	d->track = track;
	d->fh = fh;

	d->track->frames2ignore = 0;
	d->track->fpos_next_frame = d->track->fpos_first_frame;

	pv = d->track->f.compressionParams;
	if (_af_pv_getlong(pv, _AF_MS_ADPCM_NUM_COEFFICIENTS, &l))
		d->numCoefficients = l;
	else
		_af_error(AF_BAD_CODEC_CONFIG, "number of coefficients not set");

	if (_af_pv_getptr(pv, _AF_MS_ADPCM_COEFFICIENTS, &v))
		memcpy(d->coefficients, v, sizeof (int16_t) * 256 * 2);
	else
		_af_error(AF_BAD_CODEC_CONFIG, "coefficient array not set");

	if (_af_pv_getlong(pv, _AF_SAMPLES_PER_BLOCK, &l))
		d->samplesPerBlock = l;
	else
		_af_error(AF_BAD_CODEC_CONFIG, "samples per block not set");

	if (_af_pv_getlong(pv, _AF_BLOCK_SIZE, &l))
		d->blockAlign = l;
	else
		_af_error(AF_BAD_CODEC_CONFIG, "block size not set");

	*chunkframes = d->samplesPerBlock / d->track->f.channelCount;

	ret.modspec = d;
	return ret;
}

static void ms_adpcm_run_pull (_AFmoduleinst *module)
{
	ms_adpcm_data	*d = (ms_adpcm_data *) module->modspec;
	AFframecount	frames2read = module->outc->nframes;
	AFframecount	nframes = 0;
	int		i, framesPerBlock, blockCount;
	ssize_t		blocksRead, bytesDecoded;

	framesPerBlock = d->samplesPerBlock / d->track->f.channelCount;
	assert(module->outc->nframes % framesPerBlock == 0);
	blockCount = module->outc->nframes / framesPerBlock;

	/* Read the compressed frames. */
	blocksRead = af_fread(module->inc->buf, d->blockAlign, blockCount, d->fh);

	/* Decompress into module->outc. */
	for (i=0; i<blockCount; i++)
	{
		bytesDecoded = ms_adpcm_decode_block(d,
			(u_int8_t *) module->inc->buf + i * d->blockAlign,
			(int16_t *) module->outc->buf + i * d->samplesPerBlock);

		nframes += framesPerBlock;
	}

	d->track->nextfframe += nframes;

	if (blocksRead > 0)
		d->track->fpos_next_frame += blocksRead * d->blockAlign;

	assert(af_ftell(d->fh) == d->track->fpos_next_frame);

	/*
		If we got EOF from read, then we return the actual amount read.

		Complain only if there should have been more frames in the file.
	*/

	if (d->track->totalfframes != -1 && nframes != frames2read)
	{
		/* Report error if we haven't already */
		if (d->track->filemodhappy)
		{
			_af_error(AF_BAD_READ,
				"file missing data -- read %d frames, should be %d",
				d->track->nextfframe,
				d->track->totalfframes);
			d->track->filemodhappy = AF_FALSE;
		}
	}

	module->outc->nframes = nframes;
}

static void ms_adpcm_reset1 (_AFmoduleinst *i)
{
	ms_adpcm_data	*d = (ms_adpcm_data *) i->modspec;
	AFframecount	nextTrackFrame;
	int		framesPerBlock;

	framesPerBlock = d->samplesPerBlock / d->track->f.channelCount;

	nextTrackFrame = d->track->nextfframe;
	d->track->nextfframe = (nextTrackFrame / framesPerBlock) *
		framesPerBlock;

	d->framesToIgnore = nextTrackFrame - d->track->nextfframe;
	/* postroll = frames2ignore */
}

static void ms_adpcm_reset2 (_AFmoduleinst *i)
{
	ms_adpcm_data	*d = (ms_adpcm_data *) i->modspec;
	int		framesPerBlock;

	framesPerBlock = d->samplesPerBlock / d->track->f.channelCount;

	d->track->fpos_next_frame = d->track->fpos_first_frame +
		d->blockAlign * (d->track->nextfframe / framesPerBlock);
	d->track->frames2ignore += d->framesToIgnore;

	assert(d->track->nextfframe % framesPerBlock == 0);
}

static _AFmodule ms_adpcm_decompress =
{
	"ms_adpcm_decompress",
	ms_adpcm_decompress_describe,
	AF_NULL, AF_NULL,
	ms_adpcm_run_pull, ms_adpcm_reset1, ms_adpcm_reset2,
	AF_NULL, AF_NULL, AF_NULL,
	AF_NULL,
	_AFfreemodspec
};
