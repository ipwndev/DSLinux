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



#ifndef FSPL_PANEL_H
#define FSPL_PANEL_H

#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Timer.H>
#include <FL/Fl_Hold_Browser.H>

#include <nxapp.h>
#include <nxwindow.h>
#include <nxdb.h>

#include <nxbutton.h>
#include <nxwindow.h>
#include <nxscroll.h>
#include <nxvalueslider.h>
#include <nxmenubutton.h>
#include <nxbox.h>

//#include <iostream.h>
#include <string.h>

#include "Mp3_Browser.h"
#include "comm.h"
#include "mp3utils.h"

#define WIDTH 240
#define HEIGHT 290
#define EQ_X 30
#define EQ_Y 5
#define MAX_NAME_LENGTH 1024

#define APP_NAME "MP3 Player"

enum mp3_type
{
    MP3_FORWARD = 0,
    MP3_PLAY = 1,
    MP3_STOP = 2,
    MP3_BACK = 3,
    MP3_PAUSE = 4
};

extern class_musics musics;
extern class_music music;


class mp3_button:public NxButton
{
    int type_;
  public:
      mp3_button(int x, int y, int w, int h, int type);
     ~mp3_button();
    void type(int t)
    {
	type_ = t;
    }
    virtual void draw();
};

class fspl_panel:public NxApp
{
  private:
    struct
    {
	char name[1024];
	char artist[1024];
	char album[1024];
	char year[1024];
	char comment[1024];
	int length;
    }
    info;

    static char *titles[255];
    static char *playList[255];
    static char *paths[255];
    static Mp3_Node *playNode;
    static int nodes;
    static int listNum;
    static int menuValue;

    void free_paths();
    void free_titles();
    void free_play_list();
    int get_song_length(char *filename);
    void get_song_info(char *filename);
    void get_volume();
    void get_files_from_list(char *file);
    void get_files(char *dir);
    void reload_browser(char *file);
    void load_browser();
    void clear_browser();

  public:
      fspl_panel(char *app);
     ~fspl_panel();

    void MakePlayWindow();

    static int curPlaying;
    static int selPlay;
    static int doubleClicked;
    static int stopPressed;
    static int pausePressed;
    static char *defaultMusicPath;

    Fl_Pixmap *speakerSoft;
    Fl_Pixmap *speakerLoud;
    static NxWindow *window;
    static NxPimWindow *playWindow;
    static Mp3_Browser *mp3Browser;
    static Fl_Pixmap *musicNote;
    static NxScroll *musicScroll;
    static mp3_button *backButton;
    static mp3_button *forwardButton;
    static mp3_button *stopButton;
    static mp3_button *pauseButton;
    static mp3_button *playButton;
    static NxSlider *volumeSlider;
    static NxMenuButton *playListMenu;

    // Callbacks
    static void menu_callback(Fl_Widget * w, void *o);
    static void pause_callback(Fl_Widget *, void *);
    static void stop_callback(Fl_Widget * o = 0, void *vp = 0);
    static void play_callback(Fl_Widget * o = 0, void *vp = 0);
    static void back_callback(Fl_Widget *, void *);
    static void forward_callback(Fl_Widget * w = 0, void *o = 0);
    static void volume_callback(Fl_Widget *, void *);
    static void sspectra_callback(Fl_Widget *, void *);
    static void progress_callback(Fl_Widget * o, void *);
    static void timer_callback(void *);
    static void normal_callback(Fl_Widget *, void *);
    static void add_callback(Fl_Widget *, void *);
    static void fileok_callback(Fl_Widget * o = 0, void *vp = 0);
    static void filecan_callback(Fl_Widget *, void *);
    static void addall_callback(Fl_Widget *, void *);
    static void del_callback(Fl_Widget *, void *);
    static void delall_callback(Fl_Widget *, void *);
    static void hideshowpl_callback(Fl_Widget *, void *);
    static void browser_callback(Fl_Widget *, void *);
    static void http_callback(Fl_Widget *, void *);

    // Strip codes of browser text string
    void StripFormatCodes(char *title);

    //  ugly hack because apparently we can't write into xplay's memory space
    char kbps_title[64];
    char khz_title[64];

    void SetDefaultMusicPath(char *dmp)
    {
	strcpy(defaultMusicPath, dmp);
	strcat(defaultMusicPath, "/");
    }

};

#endif // FSPL_PANEL_H
