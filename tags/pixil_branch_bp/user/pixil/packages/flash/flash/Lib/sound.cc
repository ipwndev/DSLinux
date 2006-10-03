/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998,1999 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
//  Author : Olivier Debon  <odebon@club-internet.fr>
//

#include "swf.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if	SUN_SOUND
#include <sys/audioio.h>
#endif
#if	OSS_SOUND
#ifdef __NetBSD__
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#endif

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

#define PRINT 0

/* There are some defined missing in NetBSD's audioio.h */
#if	SUN_SOUND
#ifndef	AUDIO_CHANNELS_STEREO
#define	AUDIO_CHANNELS_STEREO	2
#endif
#ifndef	AUDIO_PRECISION_16
#define	AUDIO_PRECISION_16	16
#endif
#endif

//////////// SOUND

Sound::Sound(long id) : Character(SoundType, id)
{
	samples = 0;
	stereo = 0;
	soundRate = 0;
	sampleSize = 1;
}

Sound::~Sound()
{
	if (samples) {
		delete samples;
	}
}

void
Sound::setSoundFlags(long f) {
	switch (GET_SOUND_RATE_CODE(f)) {
		case 0:
			soundRate = 5500;
			break;
		case 1:
			soundRate = 11000;
			break;
		case 2:
			soundRate = 22000;
			break;
		case 3:
			soundRate = 44000;
			break;
	}
	if (f & soundIs16bit) {
		sampleSize = 2;
	}
	if (f & soundIsStereo) {
		stereo = 1;
	}

#if PRINT
	printf("-----\nFlags = %2x\n", f);
	printf("Rate = %d kHz  ", soundRate);
	printf("SampleSize = %d byte(s) ", sampleSize);
	if (f & soundIsStereo) {
		printf("Stereo  ");
	} else {
		printf("Mono  ");
	}
	switch (f & soundFormatMaske) {
	case soundFormatADPCMCompressed:
		printf("ADPCM\n");
		break;
	case soundFormatMP3Compressed:
		printf("MP3\n");
		break;
	case soundFormatRaw:
		printf("Raw\n");
		break;
	default:
	    	printf("unknown sound format %x\n", f);
		break;
	}
#endif
}

char *
Sound::setNbSamples(long n) {
	long size;

	nbSamples = n;

	size = nbSamples * (stereo ? 2 : 1) * sampleSize;

	samples = new char[ size ];

	memset((char *)samples,0, size);

	return samples;
}

long
Sound::getRate() {
	return soundRate;
}

long
Sound::getChannel() {
	return stereo ? 2 : 1;
}

long
Sound::getNbSamples() {
	return nbSamples;
}

long
Sound::getSampleSize() {
	return sampleSize;
}

char *
Sound::getSamples() {
	return samples;
}

//////////// SOUND MIXER

long  SoundMixer::dsp = -1;	// Init of descriptor
long  SoundMixer::blockSize = 0;	// Driver sound buffer size
long  SoundMixer::nbInst = 0;	// Nb SoundMixer instances
long  SoundMixer::sampleSize = 0;
long  SoundMixer::stereo = 0;
long  SoundMixer::soundRate = 0;
char *SoundMixer::buffer = 0;

SoundMixer::SoundMixer(char *device)
{
#ifndef NOSOUND
	int status;

	list = 0;	// No sound to play

	if (nbInst++) {
		// Device is already open
		return;
	}

	dsp = open(device,O_WRONLY);
	if (dsp < 0) {
		perror("open dsp");
		return;
	}

#if	SUN_SOUND
	audio_info_t audio_info;

	// Init audio_info to "harmless" values
	AUDIO_INITINFO(&audio_info);

	// Set sound rate in Hertz, Stereo, 16-bit PCM
	audio_info.play.sample_rate = 11000;
	audio_info.play.channels  = AUDIO_CHANNELS_STEREO;
	audio_info.play.precision = AUDIO_PRECISION_16;
	audio_info.play.encoding  = AUDIO_ENCODING_LINEAR;

	// Configure the audio device
	status = ioctl(dsp, AUDIO_SETINFO, &audio_info);
	if (status < 0) perror("ioctl AUDIO_SETINFO");

	sampleSize = audio_info.play.precision / 8;
	stereo     = audio_info.play.channels >= AUDIO_CHANNELS_STEREO;
	soundRate  = audio_info.play.sample_rate;
	blockSize  = audio_info.play.buffer_size;

	if (blockSize < 1024)
	    blockSize = 32768;

#else	/* !SUN_SOUND */
#if	OSS_SOUND
	// Reset device
	status = ioctl(dsp, SNDCTL_DSP_RESET, 0);
	if (status < 0) perror("ioctl SNDCTL_DSP_RESET");

	// Set sample size
	long fmt = AFMT_S16_LE;
	sampleSize = 2;
	status = ioctl(dsp, SNDCTL_DSP_SETFMT, &fmt);
	if (status < 0) perror("ioctl SNDCTL_DSP_SETFMT");

	if (status) {
		fmt = AFMT_U8;
		sampleSize = 1;
		status = ioctl(dsp, SNDCTL_DSP_SETFMT, &fmt);
		if (status < 0) perror("ioctl SNDCTL_DSP_SETFMT");
	}

	// Set stereo channel
	stereo = 1;
	status = ioctl(dsp, SNDCTL_DSP_STEREO, &stereo);

	if (status) {
		stereo = 0;
	}

	// Set sound rate in Hertz
	soundRate = 11000;
	status = ioctl(dsp, SNDCTL_DSP_SPEED, &soundRate);
	if (status < 0) perror("ioctl SNDCTL_DSP_SPEED");

	// Get device buffer size
	status = ioctl(dsp, SNDCTL_DSP_GETBLKSIZE, &blockSize);
	if (status < 0) perror("ioctl SNDCTL_DSP_GETBLKSIZE");
	if (blockSize < 1024) {
		blockSize = 32768;
	}
#endif	/* OSS_SOUND */
#endif	/* SUN_SOUND */
	blockSize *= 2;

	buffer = (char *)malloc(blockSize);
	if (buffer == 0) {
		close(dsp);
		dsp = -1;
	}

#if PRINT
#if	OSS_SOUND
	int caps;

	ioctl(dsp,SNDCTL_DSP_GETCAPS, &caps);
	printf("Audio capabilities = %x\n", caps);
#endif
	printf("Sound Rate  = %d\n", soundRate);
	printf("Stereo      = %d\n", stereo);
	printf("Sample Size = %d\n", sampleSize);
	printf("Buffer Size = %d\n", blockSize);
#endif /* PRINT */

#endif	/* NOSOUND */
}

SoundMixer::~SoundMixer()
{
	if (--nbInst == 0) {
		if (dsp > 0) {
			close(dsp);
			free(buffer);
		}
	}
}

void
SoundMixer::stopSounds()
{
#ifndef NOSOUND
	SoundList *sl,*del;

	for(sl = list; sl; ) {
		del = sl;
		sl = sl->next;
		delete del;
	}
	list = 0;
#endif
}

void
SoundMixer::startSound(Sound *sound)
{
#ifndef NOSOUND
	SoundList *sl;

	if (sound) {
		// Add sound in list
		sl = new SoundList;
		sl->rate = sound->getRate();
		sl->stereo = (sound->getChannel() == 2);
		sl->sampleSize = sound->getSampleSize();
		sl->current = sound->getSamples();
		sl->remaining = sound->getSampleSize()*sound->getNbSamples()*sound->getChannel();
		sl->next = NULL;
		// Append new sound at end of list
		if (list != NULL) {
		    SoundList *last = list;
		    while (last->next != NULL)
			last = last->next;
		    last->next = sl;
		} else {
		    list = sl;
		}
	}
#endif
}

long
SoundMixer::playSounds()
{
#ifndef NOSOUND
	long		 nbBytes, n;
	SoundList	*sl;
	int		 status;

	// Init failed
	if (dsp < 0) return 0;

	// No sound to play
	if (list == 0) return 0;

#if	SUN_SOUND
	fd_set wrset;
	struct timeval timeout;

	FD_ZERO(&wrset);
	FD_SET(dsp, &wrset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	status = select(dsp+1, NULL, &wrset, NULL, &timeout);

	// Cannot output sound data without blocking
	// But there are still sounds to play. We must wait.
	if (status != 1 || !FD_ISSET(dsp, &wrset)) return 1;
#endif
#if	OSS_SOUND
	audio_buf_info	 bufInfo;

	// Get free DMA buffer space
	status = ioctl(dsp, SNDCTL_DSP_GETOSPACE, &bufInfo);

	// Free space is not large enough to output data without blocking
	// But there are still sounds to play. We must wait.
	if (bufInfo.bytes < blockSize) return 1;
#endif

	nbBytes = 0;

	// Fill buffer with silence.
	memset((void*)buffer, 0, blockSize);

	sl = list;
	while (sl && nbBytes < blockSize) {

		// Ask sound to fill the buffer
		// according to device capabilities
		n = fillSoundBuffer(sl, buffer + nbBytes, blockSize - nbBytes);

		// Remember amount of data written
		nbBytes += n;

		// No more samples for this sound
		if (sl->remaining == 0) {
		        list = sl->next;
			delete sl;
			sl = list;
		}
	}

	if (nbBytes) {
		// At last ! Play It !
	    	if (write(dsp,buffer,nbBytes) != nbBytes)
			perror("write sound data");
#if	OSS_SOUND
		status = ioctl(dsp, SNDCTL_DSP_POST, 0);
#endif
	}

	return nbBytes;
#else
	return 0;
#endif
}

long
SoundMixer::fillSoundBuffer(SoundList *sl, char *buff, long buffSize)
{
	long sampleLeft, sampleRight;
	long skipOut, skipOutInit;
	long skipIn, skipInInit;
	long freqRatio;
	long totalOut = 0;

	sampleLeft = sampleRight = 0;
	skipOutInit = skipInInit = 0;

	freqRatio = sl->rate / soundRate;
	if (freqRatio) {
		skipOutInit = freqRatio - 1;
		skipInInit = 0;
	}

	freqRatio = soundRate / sl->rate;
	if (freqRatio) {
		skipInInit = freqRatio - 1;
		skipOutInit = 0;
	}

	skipOut = skipOutInit;
	skipIn = skipInInit;
	while (buffSize && sl->remaining) {
		if (skipIn-- == 0) {
			// Get sampleLeft
			if (sl->sampleSize == 2) {
				sampleLeft = (long)(*(short *)(sl->current));
				if (sampleSize == 1) {
					sampleLeft = (sampleLeft >> 8) &0xff;
				}
			} else {
				sampleLeft = (long)*(sl->current);
				if (sampleSize == 2) {
					sampleLeft <<= 8;
				}
			}
			sl->current += sl->sampleSize;
			sl->remaining -= sl->sampleSize;

			if (sl->stereo) {
				// Get sampleRight
				if (sl->sampleSize == 2) {
					sampleRight = (long)(*(short *)(sl->current));
					if (sampleSize == 1) {
						sampleRight = (sampleRight >> 8) &0xff;
					}
				} else {
					sampleRight = (long)*(sl->current);
					if (sampleSize == 2) {
						sampleRight <<= 8;
					}
				}
				sl->current += sl->sampleSize;
				sl->remaining -= sl->sampleSize;

			} else {
				sampleRight = sampleLeft;
			}
			
			skipIn = skipInInit;
		}

		if (skipOut-- == 0) {
			// Output
			if (stereo) {
				if (sampleSize == 2) {
					*((short *)buff) += sampleLeft/2;
					buffSize -= sampleSize;
					buff += sampleSize;
					*((short *)buff) += sampleRight/2;
					buffSize -= sampleSize;
					buff += sampleSize;
				} else {
					*((char *)buff) += sampleLeft/2;
					buffSize -= sampleSize;
					buff += sampleSize;
					*((char *)buff) += sampleRight/2;
					buffSize -= sampleSize;
					buff += sampleSize;
				}
				totalOut += 2*sampleSize;
			} else {
				if (sampleSize == 2) {
					*((short *)buff) += (sampleLeft+sampleRight)>>2;
					buffSize -= sampleSize;
					buff += sampleSize;
				} else {
					*((char *)buff) += (sampleLeft+sampleRight)>>2;
					buffSize -= sampleSize;
					buff += sampleSize;
				}
				totalOut += sampleSize;
			}

			skipOut = skipOutInit;
		}
	}

	return totalOut;
}
