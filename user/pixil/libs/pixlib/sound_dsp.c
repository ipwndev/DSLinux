/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>

#include <pixlib/pixlib.h>
#include <sound_dsp.h>

#define AUDIO_FILE "/dev/dsp"

/* This array ties the PIXIO_MIXER defines to the */
/* LINUX defines */

static int pix_io2linux_values[PIXIO_MIXER_COUNT] = {
    SOUND_MIXER_VOLUME,
    SOUND_MIXER_BASS,
    SOUND_MIXER_TREBLE,
    SOUND_MIXER_MIC,
    SOUND_MIXER_LINE1,
    SOUND_MIXER_LINE2
};

static int
OpenAudioDevice(int direction)
{
    int omode;
    int fd;

    if (direction == PIXIO_PLAY)
	omode = O_WRONLY;
    else
	omode = O_RDONLY | O_NONBLOCK;

    fd = open(AUDIO_FILE, omode, 0);

    if (fd == -1)
	perror("OPEN_AUDIO");

    return (fd);
}

void
ParseWAVHeader(dspWaveHeader * hdr, pix_io_audio_t * playback)
{
    playback->stereo = (hdr->modus == WAVE_STEREO) ? 1 : 0;
    playback->sample = hdr->bits_per_sample;
    playback->speed = hdr->spd;
    playback->size = hdr->datalen;
}

int
dspStreamRecord(int fd, unsigned char *buffer, int size)
{
    int len = read(fd, buffer, size);

    if (len < 0) {
	if (errno == EAGAIN)
	    return (PIXIO_AGAIN);

	perror("dspStreamRecord");
	return (PIXIO_ERROR);
    }

    return (len);
}

int
dspStreamPlay(int fd, unsigned char *buffer, int size)
{
    int len = write(fd, buffer, size);

    if (len < 0) {
	if (errno == EAGAIN)
	    return (PIXIO_AGAIN);

	perror("PLAY_STREAM");
	return (PIXIO_ERROR);
    }

    return (len);
}

int
dspGetWAVFileStats(char *filename, pix_io_audio_t * settings)
{
    dspWaveHeader wavheader;
    int infd;

    if ((infd = open(filename, O_RDONLY, 0)) == -1) {
	perror("OPEN_WAV");
	return (PIXIO_ERROR);
    }

    /* Read and parse the header */
    if (read(infd, (char *) &wavheader, sizeof(dspWaveHeader))
	< sizeof(dspWaveHeader)) {
	perror("READ_WAV");
	close(infd);
	return (PIXIO_ERROR);
    }

    ParseWAVHeader(&wavheader, settings);
    close(infd);
    return (PIXIO_OK);
}

#define RIFF            0x46464952
#define WAVE            0x45564157
#define FMT             0x20746D66
#define DATA            0x61746164
#define PCM_CODE        1

int
dspWriteWAVFile(char *filename, pix_io_audio_t * settings,
		unsigned char *buffer)
{
    int fd;
    dspWaveHeader hdr;

    hdr.main_chunk = RIFF;
    hdr.length = settings->size + sizeof(dspWaveHeader) - 8;
    hdr.chunk_type = WAVE;
    hdr.sub_chunk = FMT;
    hdr.sc_len = 16;
    hdr.format = PCM_CODE;
    hdr.modus = settings->stereo ? 2 : 1;
    hdr.spd = settings->speed;
    hdr.bytes_per_sample =
	((settings->sample == 8) ? 1 : 2) * (settings->stereo ? 2 : 1);
    hdr.bytes_per_sec = settings->speed * hdr.modus * hdr.bytes_per_sample;
    hdr.bits_per_sample = settings->sample;
    hdr.data_chunk = DATA;
    hdr.datalen = settings->size;

    if ((fd = open(filename, O_WRONLY | O_CREAT, S_IREAD | S_IWRITE)) == -1) {
	perror("OPEN WAV");
	return (PIXIO_ERROR);
    }

    write(fd, &hdr, sizeof(dspWaveHeader));
    write(fd, buffer, settings->size);
    close(fd);
    return (PIXIO_OK);
}

int
dspReadWAVFile(char *filename, pix_io_audio_t * settings,
	       unsigned char *buffer, int maxsize)
{
    int infd, len;
    dspWaveHeader wavheader;

    if ((infd = open(filename, O_RDONLY, 0)) == -1) {
	perror("OPEN WAV");
	return (PIXIO_ERROR);
    }

    /* Read and parse the header */
    if (read(infd, (char *) &wavheader, sizeof(dspWaveHeader))
	< sizeof(dspWaveHeader)) {
	perror("READ WAV");
	close(infd);
	return (PIXIO_ERROR);
    }

    ParseWAVHeader(&wavheader, settings);

    if (settings->size > maxsize) {
	close(infd);
	return (PIXIO_FILE_TOO_BIG);
    }

    len = read(infd, buffer, settings->size);

    if (len == -1 || len != settings->size) {
	if (len == -1)
	    perror("READ WAV");

	close(infd);
	return (PIXIO_ERROR);
    } else {
	close(infd);
	return (len);
    }
}

int
dspPlayWAVFile(char *filename)
{
    int err = PIXIO_ERROR;
    char *audiobuffer;

    dspWaveHeader wavheader;

    int dev = 0;
    int infd = 0;

    pix_io_audio_t playback;

    /* Read the WAV file */

    if ((infd = open(filename, O_RDONLY, 0)) == -1) {
	perror("OPEN WAV");
	return (PIXIO_ERROR);
    }

    /* Read and parse the header */
    if (read(infd, (char *) &wavheader, sizeof(dspWaveHeader))
	< sizeof(dspWaveHeader)) {
	perror("READ WAV");
	close(infd);
	return (PIXIO_ERROR);
    }

    ParseWAVHeader(&wavheader, &playback);

    /* Now open up the stream */
    dev = dspOpenStream(PIXIO_PLAY, &playback);

    if (dev == PIXIO_ERROR) {
	close(infd);
	return (PIXIO_ERROR);
    }

    audiobuffer = (char *) malloc(playback.blocksize);

    while (1) {
	int inlen;

	/* Read a block in from the file */
	inlen = read(infd, (char *) audiobuffer, playback.blocksize);

	if (inlen == -1) {
	    perror("READ_WAV");
	    break;
	}

	if (inlen == 0) {
	    err = PIXIO_OK;	/* No error */
	    break;
	}

	if (write(dev, (char *) audiobuffer, inlen) != inlen) {
	    perror("WRITE_AUDIO");
	    break;
	}

	if (inlen < playback.blocksize) {
	    err = PIXIO_OK;	/* No error */
	    break;
	}
    }

    close(dev);
    close(infd);
    return (err);

    return (err);
}

static int
open_mixer()
{
    int fd;

    fd = open("/dev/mixer", O_RDWR);

    if (fd == -1)
	perror("OPEN_MIXER");

    return (fd);
}

static unsigned long
read_mixer(int device)
{
    int mixer = 0;
    int level = -1;

    if ((mixer = open_mixer()) == -1)
	return (-1);

    if (ioctl(mixer, MIXER_READ(device), &level) == -1) {
	perror("MIXER_READ (IOCTL)");
	level = -1;
    }

    close(mixer);
    return (level);
}

#ifdef NOTUSED

static int
write_mixer(int device, unsigned long level)
{
    int err = -1;
    int mixer = 0;

    if ((mixer = open_mixer()) == -1)
	return (-1);

    if (ioctl(mixer, MIXER_WRITE(device), &level) == -1) {
	perror("MIXER_WRITE (IOCTL)");
	err = -1;
    } else
	err = 0;

    close(mixer);
    return (err);
}
#endif

int
mixer_get_devices(unsigned long *bitmask)
{
    int i, j;
    unsigned long readin;

    if ((readin = read_mixer(SOUND_MIXER_DEVMASK)) == -1)
	return (PIXIO_ERROR);

    *bitmask = 0;

    for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
	if (readin & (1 << i)) {
	    for (j = 0; j < PIXIO_MIXER_COUNT; j++) {
		if (pix_io2linux_values[j] == i)
		    *bitmask |= (1 << j);
	    }
	}
    }

    return (PIXIO_OK);
}
