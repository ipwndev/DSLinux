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

    if (direction == PIXIO_PLAY) {
	printf("set mode wronly\n");
	omode = O_WRONLY;
    } else
	omode = O_RDONLY | O_NONBLOCK;

    fd = open(AUDIO_FILE, omode, 0);

    if (fd == -1)
	perror("OPEN_AUDIO");

    return (fd);
}

int
dspOpenStream(int direction, pix_io_audio_t * settings)
{
    int err = PIXIO_ERROR;
    int tmp, bsize;
    int fd;

    fd = OpenAudioDevice(direction);

    if (fd < 0)
	return (err);

    /* Sync the device */
    if (ioctl(fd, SNDCTL_DSP_SYNC, NULL) < 0) {
	perror("OPEN_STREAM (SYNC)");
	goto exit_dspOpenStream;
    }

    /* Set the sample size */
    tmp = settings->sample;

    ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &tmp);

    if (tmp != settings->sample) {
	/* Failure - Try setting it to 8 bits */
	tmp = 8;
	ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &tmp);
	if (tmp != 8) {
	    perror("OPEN_STREAM (SIZE)");
	    goto exit_dspOpenStream;
	}
    }

    /* Set the sample rate */
    printf("speed [%d]\n", settings->speed);
    if (ioctl(fd, SNDCTL_DSP_SPEED, &settings->speed) < 0) {
	perror("OPEN_STREAM (SPEED)");
	goto exit_dspOpenStream;
    }

    /* Finally, set the stereo setting */
    if (settings->stereo)
	if (ioctl(fd, SNDCTL_DSP_STEREO, &settings->stereo) < 0)
	    settings->stereo = 0;

    /* Last step, grab the block size and return it to the caller */
    /* Get the biggest block we can read */

    bsize = -1;
    ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &bsize);

    if (bsize < 4 || bsize > 65536) {
	if (bsize == -1) {
	    perror("OPEN_STREAM (BLOCKSIZE)");
	    goto exit_dspOpenStream;
	}
	bsize = 1024;
    }

    settings->blocksize = bsize;
    err = PIXIO_OK;
    return (fd);

  exit_dspOpenStream:
    if (fd >= 0)
	close(fd);

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

static int
write_mixer(int device, unsigned int level)
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

int
mixer_set_level(int device, pix_io_level_t * level)
{
    int linux_device;
    unsigned int set_level;

    if (device > PIXIO_MIXER_COUNT)
	return (PIXIO_BAD_DEVICE);
    else
	linux_device = pix_io2linux_values[device];

    set_level = ((level->right * 0x7f / 100)) << 8;
    set_level |= (level->left * 0x7f / 100);


    if (write_mixer(linux_device, (unsigned int) set_level) == -1)
	return (PIXIO_ERROR);
    else
	return (PIXIO_OK);
}

int
mixer_get_level(int device, pix_io_level_t * level)
{
    int linux_device;
    int levelin;
    int l, r;

    if (device > PIXIO_MIXER_COUNT)
	return (PIXIO_BAD_DEVICE);
    else
	linux_device = pix_io2linux_values[device];

    if ((levelin = read_mixer(linux_device)) == -1)
	return (PIXIO_ERROR);

    l = levelin & 0x7f;
    r = (levelin >> 8) & 0x7f;

    level->left = l * 100 / 0x7f;
    level->right = r * 100 / 0x7f;

    return (PIXIO_OK);
}
