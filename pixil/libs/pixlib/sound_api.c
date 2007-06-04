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



#include <pixlib/pixlib.h>
#include <pixlib/pixio_proto.h>

extern pix_io_functions_t pix_io_functions;

int
pix_io_play_soundfile(int type, char *filename)
{
    switch (type) {
    case PIXIO_WAV_FILE:
	if (pix_io_functions.sysdep_play_wav_file)
	    return (pix_io_functions.sysdep_play_wav_file(filename));
	else
	    return (PIXIO_NOT_IMPLEMENTED);
	break;

    default:
	return (PIXIO_NOT_IMPLEMENTED);
    }
}

int
pix_io_save_sound_file(int type, char *filename, pix_io_audio_t * settings,
		       unsigned char *buffer)
{
    switch (type) {
    case PIXIO_WAV_FILE:
	if (pix_io_functions.sysdep_save_wav_file)
	    return (pix_io_functions.
		    sysdep_save_wav_file(filename, settings, buffer));
	else
	    return (PIXIO_NOT_IMPLEMENTED);
	break;

    default:
	return (PIXIO_NOT_IMPLEMENTED);
    }
}

int
pix_io_load_sound_file(int type, char *filename, pix_io_audio_t * settings,
		       unsigned char *buffer, int size)
{
    switch (type) {
    case PIXIO_WAV_FILE:
	if (pix_io_functions.sysdep_load_wav_file)
	    return (pix_io_functions.
		    sysdep_load_wav_file(filename, settings, buffer, size));
	else
	    return (PIXIO_NOT_IMPLEMENTED);
	break;

    default:
	return (PIXIO_NOT_IMPLEMENTED);
    }
}

int
pix_io_open_stream(int direction, pix_io_audio_t * settings)
{
    if (pix_io_functions.sysdep_open_stream)
	return (pix_io_functions.sysdep_open_stream(direction, settings));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_stream_record(int fd, unsigned char *buffer, int size)
{
    if (pix_io_functions.sysdep_stream_record)
	return (pix_io_functions.sysdep_stream_record(fd, buffer, size));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_stream_play(int fd, unsigned char *buffer, int size)
{
    if (pix_io_functions.sysdep_stream_play)
	return (pix_io_functions.sysdep_stream_play(fd, buffer, size));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_get_wav_stats(char *filename, pix_io_audio_t * settings)
{
    if (pix_io_functions.sysdep_get_wav_stats)
	return (pix_io_functions.sysdep_get_wav_stats(filename, settings));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_get_mixer_devices(unsigned long *bitmask)
{
    if (pix_io_functions.sysdep_get_mixer_devices)
	return (pix_io_functions.sysdep_get_mixer_devices(bitmask));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_set_mixer_level(int device, pix_io_level_t * level)
{
    if (pix_io_functions.sysdep_set_mixer_level)
	return (pix_io_functions.sysdep_set_mixer_level(device, level));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}

int
pix_io_get_mixer_level(int device, pix_io_level_t * level)
{
    if (pix_io_functions.sysdep_get_mixer_level)
	return (pix_io_functions.sysdep_get_mixer_level(device, level));
    else
	return (PIXIO_NOT_IMPLEMENTED);
}
