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



#ifndef COMM_H
#define COMM_H

//
// Communication between threads.  This is ugly and not thought out at
// all, but seems to work OK.
//

#include <pthread.h>
#include <FL/Fl_Window.H>
//#include <iostream.h>
#include <string.h>

// Client DSP plug-ins definitions
#define FEEDBACK 1

class class_music
{
  public:
    bool pause, quit;
    bool setframeflag;
    int setframenumber;
};

class class_musics
{

    struct feedback_struct
    {
	int Hz;
	int SR;
	Fl_Window *parent;
    };

    struct playlist_struct
    {
	int key;
	char *title;
	char *path;
    };

  public:
      class_musics();
     ~class_musics();

  public:
      bool stop;
    bool restart;
    int move, currentrun;
    pthread_mutex_t movelock;
    bool errorflag;
    int errorcode;

    //  comm area for xplay to communicate to fltk widgets
    int volumeflag;
    int newvolume;
    int updatestats;
    int version;
    int layer;
    int freq;
    int bitrate;
    int pcmperframe;
    int currentframe;
    int maxframe;
    char filename[1024];	// disgusting hack to accept mp3 file on cmd line
    int tracksplayed;		// disgusting hack to play multiple files
    char title[1024];
    char ch_bitrate[64];
    char ch_freq[64];

    char *song;
    feedback_struct *feedback;
    int dsp;

    void AddPlayList(char *title, char *path);
    void DelPlayList(int index);
    int SearchPlayList(char *title);
    char *GetPlayListPath(int index)
    {
	return playlist[index].path;
    }

    void SetParent(Fl_Window * o);
    Fl_Window *GetParent();

    void Setcurrentmpegstatus(int ver, int ly, int fr, int bit, char *t)
    {
	updatestats = 1;
	version = ver;
	layer = ly;
	freq = fr;
	bitrate = bit;
	strcpy(title, t);
    }

  private:
    playlist_struct * playlist;	// Play List
    int add_index;		// index to add new song to playlist
    int play_index;		// index for next song to play 
    Fl_Window *parent;

};

/**********************/
/* Setting music flag */
/**********************/

// Control music
void music_done(void);
bool music_isstop(void);
bool music_ispause(void);
void music_restart(void);
void music_stop(void);
void music_play(void);
void music_pause(void);
void music_unpause(void);
void music_previous(void);
void music_next(void);

// Client Play List
void SetPlayList(char *newSong);
char *GetPlayList();

// Client DSP access points
void SetParent(Fl_Window * o);
Fl_Window *GetParent();
void SetDsp(int newDsp);
int GetDsp();
void *GetArgs();
extern void *dsp_rawdata;
inline void
SetDspData(void *next_rawdata)
{
    dsp_rawdata = next_rawdata;
}
inline void *
GetDspData()
{
    return dsp_rawdata;
}

#endif
