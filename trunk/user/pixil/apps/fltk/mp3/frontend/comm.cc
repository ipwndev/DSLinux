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

#include <pixil_config.h>

#include "comm.h"
//#include <iostream.h>
#include <stdio.h>

class_musics::class_musics()
{

    // initializations
    volumeflag = 0;
    newvolume = 0;
    version = 2;
    layer = 3;
    bitrate = 0;
    freq = 44100;
    pcmperframe = 0;
    currentframe = 0;
    maxframe = 1;
    strcpy(filename, "\0");
    tracksplayed = 0;
    strcpy(title, "\0");
    strcpy(ch_bitrate, "\0");
    strcpy(ch_freq, "\0");

    song = new char[1024];
    feedback = new feedback_struct;
    playlist = new playlist_struct[255];
    for (int i = 0; i < 255; i++) {
	playlist[i].key = i;
	playlist[i].title = new char[1024];
	playlist[i].path = new char[1024];
    }

    add_index = 0;
    play_index = 0;

}

class_musics::~class_musics()
{
    delete song;
    delete feedback;
    for (int i = 0; i < 255; i++) {
	delete[]playlist[i].title;
	delete[]playlist[i].path;
    }
    delete[]playlist;
    delete parent;
}

void
class_musics::AddPlayList(char *title, char *path)
{

    if (add_index < 255) {
	strcpy(playlist[add_index].title, title);
	strcpy(playlist[add_index].path, path);
	add_index++;
    } else {			// Check for empty slot

	for (int i = 0; i < 255; i++) {

	    if (playlist[i].title == "") {
		strcpy(playlist[add_index].title, title);
		strcpy(playlist[add_index].path, path);
		break;
	    }

	}
    }

}

void
class_musics::DelPlayList(int index)
{

    strcpy(playlist[index].title, "\0");

}

int
class_musics::SearchPlayList(char *title)
{

    int match = 0;

    for (int i = 0; i < 255; i++) {
	match = strcmp(playlist[i].title, title);
	if (match == 0)
	    return i;
    }

    return (-1);

}

void
class_musics::SetParent(Fl_Window * o)
{
    feedback->parent = o;
}

Fl_Window *
class_musics::GetParent()
{
    return feedback->parent;
}


class_musics musics;
class_music music;

// internal
static void
music_move(int value)
{
    pthread_mutex_lock(&musics.movelock);
    musics.move += value;
    pthread_mutex_unlock(&musics.movelock);
}

void
music_done(void)
{
    music_move(1);
}

inline void
music_term(void)
{
    music.pause = false;
    music.quit = true;
}

// external 
bool
music_isstop(void)
{
    return musics.stop;
}

bool
music_ispause(void)
{
    return music.pause;
}

void
music_restart(void)
{
    musics.restart = true;
}

void
music_stop(void)
{
    musics.stop = true;
    music_term();
}

void
music_play(void)
{
    music.pause = false;
    music.quit = false;
    musics.stop = false;
}

void
music_pause(void)
{
    music.pause = true;
}

void
music_unpause(void)
{
    music.pause = false;
}

void
music_previous(void)
{
    music_move(-1);
    music_term();
}

void
music_next(void)
{
    music_move(1);
    music_term();
}

////////////////////////////////////////
// Client Play List
////////////////////////////////////////

void
SetPlayList(char *newSong)
{
    strcpy(musics.song, newSong);
#ifdef CONFIG_DEBUG
    printf("SetPlayList: %s\n", musics.song);
#endif
}

char *
GetPlayList()
{
    int index = musics.SearchPlayList(musics.song);
    return (musics.GetPlayListPath(index));
}

////////////////////////////////////////
// Client DSP plug-ins
////////////////////////////////////////

void
SetDsp(int newDsp)
{
    musics.dsp = newDsp;
}

int
GetDsp()
{
    return musics.dsp;
}

void *
GetArgs()
{

    if (musics.dsp == FEEDBACK) {
	return (void *) musics.feedback;
    } else {
	return 0;
    }

}

void *dsp_rawdata;
