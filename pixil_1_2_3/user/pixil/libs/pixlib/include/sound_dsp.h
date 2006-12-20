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


#ifndef SOUND_DSP_H
#define SOUND_DSP_H

#define WAVE_MONO	1
#define WAVE_STEREO	2

#include <pixlib/pixlib.h>

/* Wave header */
typedef struct
{
    unsigned long main_chunk;
    unsigned long length;
    unsigned long chunk_type;

    unsigned long sub_chunk;
    unsigned long sc_len;
    unsigned short format;
    unsigned short modus;
    unsigned long spd;
    unsigned long bytes_per_sec;
    unsigned short bytes_per_sample;
    unsigned short bits_per_sample;
    unsigned long data_chunk;
    unsigned long datalen;
}
dspWaveHeader;


extern int dspOpenStream(int direction, pix_io_audio_t * settings);
int dspStreamRecord(int fd, unsigned char *buffer, int size);
int dspStreamPlay(int fd, unsigned char *buffer, int size);

int dspGetWAVFileStats(char *filename, pix_io_audio_t * settings);
int dspWriteWAVFile(char *filename, pix_io_audio_t * settings,
		    unsigned char *buffer);
int dspReadWAVFile(char *filename, pix_io_audio_t * settings,
		   unsigned char *buffer, int size);

int dspPlayWAVFile(char *filename);

int mixer_get_devices(unsigned long *);
extern int mixer_set_level(int, pix_io_level_t *);
extern int mixer_get_level(int, pix_io_level_t *);

#endif
