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

#include <dirent.h>
#include "fspl_panel.h"

#include <FL/Fl_Pixmap.H>
#include <FL/filename.H>
#include <FL/Fl_Double_Window.H>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mpegsound.h>

// sound card
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <icons/speaker.xpm>
#include <icons/speaker_loud.xpm>
#include <icons/note.xpm>

/* IOCTL */
#ifdef SOUND_VERSION
#define IOCTL(a,b,c)            ioctl(a,b,&c)
#else
#define IOCTL(a,b,c)            (c = ioctl(a,b,c) )
#endif

NxWindow *
    fspl_panel::window;
NxPimWindow *
    fspl_panel::playWindow;
int
    fspl_panel::selPlay;
int
    fspl_panel::curPlaying;
int
    fspl_panel::stopPressed;
int
    fspl_panel::pausePressed;
int
    fspl_panel::doubleClicked;
int
    fspl_panel::nodes;
int
    fspl_panel::listNum;
int
    fspl_panel::menuValue;

char *
    fspl_panel::paths[255];
char *
    fspl_panel::titles[255];
char *
    fspl_panel::playList[255];
char *
    fspl_panel::defaultMusicPath;

Fl_Pixmap *
    fspl_panel::musicNote;

Mp3_Browser *
    fspl_panel::mp3Browser;

NxScroll *
    fspl_panel::musicScroll;

mp3_button *
    fspl_panel::backButton;
mp3_button *
    fspl_panel::forwardButton;
mp3_button *
    fspl_panel::stopButton;
mp3_button *
    fspl_panel::pauseButton;
mp3_button *
    fspl_panel::playButton;

NxMenuButton *
    fspl_panel::playListMenu;

Mp3_Node *
    fspl_panel::playNode;

NxSlider *
    fspl_panel::volumeSlider;

fspl_panel::fspl_panel(char *app)
{

    selPlay = 1;
    curPlaying = 0;
    pausePressed = 0;
    stopPressed = 1;
    nodes = 0;
    listNum = 0;
    menuValue = 0;

    defaultMusicPath = new char[255];
    int path_ok = 0;

#ifdef CONFIG_PAR
    /* See if PAR has a different path */

    db_handle *par_db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (par_db) {
	int ret = par_getAppPref(par_db, "fltksplay", "paths", "mp3dir",
				 defaultMusicPath, 255);

	if (ret >= 0)
	    path_ok = 1;
	else
	    printf
		("Warning - couldn't get the MP3 path from the PAR database.\n");

	db_closeDB(par_db);
    } else
	printf("Warning - couldn't open the par database %s\n",
	       db_getDefaultDB());
#endif

    /* If we couldn't get the path from PAR, then just use the
       default 
     */

    if (!path_ok)
	strcpy(defaultMusicPath, "/usr/pixil/mp3/");

////////////////////////////////////////
// Create song window and timer
////////////////////////////////////////
    window = new NxWindow(W_W, W_H, app);
    MakePlayWindow();

    musics.SetParent(window);
    window->end();

    window->show();

    get_volume();

    Fl::add_timeout(1.0, timer_callback);
////////////////////////////////////////
// Song information and progression
////////////////////////////////////////
    load_browser();

    mp3Browser->select_range(mp3Browser->traverse_start(),
			     mp3Browser->traverse_start());


////////////////////////////////////////
// Music control buttons
////////////////////////////////////////

////////////////////////////////////////
// Play list buttons and browser
////////////////////////////////////////

    musicNote = new Fl_Pixmap(note);
}

fspl_panel::~fspl_panel()
{
    for (int idx = 0; idx < 255; idx++) {
	if (titles[idx])
	    free(titles[idx]);
	if (playList[idx])
	    free(playList[idx]);
	if (paths[idx])
	    free(paths[idx]);
    }
    delete[]defaultMusicPath;
    music_stop();
    clear_browser();
}

void
fspl_panel::free_play_list()
{
    int index = 0;

    for (int idx = 0; idx < nodes; idx++) {
	index = musics.SearchPlayList(titles[idx]);
	if (-1 != index)
	    musics.DelPlayList(index);
    }
}

void
fspl_panel::free_paths()
{
    for (int idx = 0; idx < 255; idx++) {
	if (paths[idx]) {
	    free(paths[idx]);
	}
	paths[idx] = 0;
    }
}

void
fspl_panel::free_titles()
{
    for (int idx = 0; idx < 255; idx++) {
	if (titles[idx])
	    free(titles[idx]);
	titles[idx] = 0;
    }
}

void
fspl_panel::MakePlayWindow()
{
    playWindow = new NxPimWindow(0, 0, W_W, W_H);
    {
	playListMenu =
	    new NxMenuButton(BUTTON_X, 5, BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	playListMenu->add("All");
	playListMenu->label("All");
	playListMenu->callback(menu_callback);
	playListMenu->when(FL_WHEN_CHANGED);
	playWindow->add((Fl_Widget *) playListMenu);
    }
    {
	backButton = new mp3_button(5, BUTTON_Y - 11, BUTTON_WIDTH / 2,
				    BUTTON_HEIGHT + 5, MP3_BACK);
	backButton->callback(back_callback);
	backButton->movable(false);
	playWindow->add((Fl_Widget *) backButton);
    }
    {
	pauseButton =
	    new mp3_button((5 + BUTTON_WIDTH / 2) + 5, BUTTON_Y - 11,
			   BUTTON_WIDTH / 2, BUTTON_HEIGHT + 5, MP3_PAUSE);
	playWindow->add((Fl_Widget *) pauseButton);
	pauseButton->movable(false);
	pauseButton->callback(pause_callback);
    }
    {
	stopButton =
	    new mp3_button((5 + BUTTON_WIDTH / 2) * 2 + 5, BUTTON_Y - 11,
			   BUTTON_WIDTH / 2, BUTTON_HEIGHT + 5, MP3_STOP);
	stopButton->hide();
	stopButton->movable(false);
	playWindow->add((Fl_Widget *) stopButton);
	stopButton->callback(stop_callback, (void *) 1);
	stopButton->hide();
    }
    {
	playButton =
	    new mp3_button((5 + BUTTON_WIDTH / 2) * 2 + 5, BUTTON_Y - 11,
			   BUTTON_WIDTH / 2, BUTTON_HEIGHT + 5, MP3_PLAY);
	playButton->movable(false);
	playWindow->add((Fl_Widget *) playButton);
	playButton->callback(play_callback, this);
    }
    {
	forwardButton =
	    new mp3_button((5 + BUTTON_WIDTH / 2) * 3 + 5, BUTTON_Y - 11,
			   BUTTON_WIDTH / 2, BUTTON_HEIGHT + 5, MP3_FORWARD);
	forwardButton->callback(forward_callback);
	forwardButton->movable(false);
	playWindow->add((Fl_Widget *) forwardButton);
    }
    {
	volumeSlider =
	    new NxSlider((5 + BUTTON_WIDTH / 2) * 4 + 10, BUTTON_Y - 15, 70,
			 BUTTON_HEIGHT);
	volumeSlider->callback(volume_callback);
	playWindow->add((Fl_Widget *) volumeSlider);
	volumeSlider->box(FL_NO_BOX);
	volumeSlider->minimum(0.0);
	volumeSlider->maximum(100.0);
	volumeSlider->step(1.0);
	volumeSlider->type(FL_HOR_NICE_SLIDER);
	volumeSlider->when(FL_WHEN_CHANGED);
	volumeSlider->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	volumeSlider->movable(false);
    }
    {
	musicScroll = new NxScroll(-1, BUTTON_HEIGHT + 10, W_W + 2, 221);
	musicScroll->resize(false);
	{
	    mp3Browser = new Mp3_Browser(0, BUTTON_HEIGHT + 10, W_W, 10);
	    mp3Browser->callback(browser_callback);
	}
	musicScroll->end();

	playWindow->add((Fl_Widget *) musicScroll);
    }
    {
	NxBox *o =
	    new NxBox((5 + BUTTON_WIDTH / 2) * 4 + 10, BUTTON_Y + 3, 8, 11);
	(new Fl_Pixmap(speaker))->label(o);
	o->movable(false);
	playWindow->add((Fl_Widget *) o);
    }
    {
	NxBox *o =
	    new NxBox((5 + BUTTON_WIDTH / 2) * 4 + 63, BUTTON_Y + 1, 17, 15);
	(new Fl_Pixmap(speaker_loud))->label(o);
	o->movable(false);
	playWindow->add((Fl_Widget *) o);
    }

}

////////////////////////////////////////////////////////////////////////////////
//
// Methods
//
////////////////////////////////////////////////////////////////////////////////

void
fspl_panel::clear_browser()
{
    Fl_Toggle_Node *n = mp3Browser->traverse_start();
    while (n) {
	mp3Browser->remove(n);
	n = mp3Browser->traverse_start();
    }
    nodes = 0;
}

void
fspl_panel::get_volume()
{
    int volume;
    int handle;
    int r;

    handle = open("/dev/mixer", O_RDWR);
    ioctl(handle, MIXER_READ(SOUND_MIXER_VOLUME), &r);
    close(handle);

    volume = r >> 8;
    volumeSlider->value(volume);
    volumeSlider->hide();
    volumeSlider->show();

}

static void
change_char(char *buf, int len)
{
    int idx;

    buf[len] = 0;

    for (idx = len - 1; idx >= 0; idx--)
	if (((unsigned char) buf[idx]) < 26 || buf[idx] == ' ')
	    buf[idx] = 0;
	else
	    break;
}

int
fspl_panel::get_song_length(char *filename)
{
    int err;
    char *device = Rawplayer::defaultdevice;
    Soundinputstream *loader = Soundinputstream::hopen(filename, &err);
    Rawplayer *player;

    if (loader == NULL) {
	perror("fspl_panel::get_song_length LOADER");
	info.length = -1;
	return -1;
    }
    if (device == NULL)
	device = Rawplayer::defaultdevice;
    if (device[0] != '/')
	device = Rawplayer::defaultdevice;
    player = new Rawplayer;

    player->initialize(device);

    Mpegtoraw *server = new Mpegtoraw(loader, player);

    if (!server) {
	perror("fspl_panel::get_song_length SERVER");
	info.length = -1;
	return -1;
    }
    server->initialize(filename);

    info.length =
	server->gettotalframe() * server->getpcmperframe() /
	server->getfrequency();
    if (info.length)
	info.length++;

    delete loader;
    delete player;
    delete server;
    loader = 0;
    player = 0;
    server = 0;

    return info.length;

}

void
fspl_panel::get_song_info(char *filename)
{
    struct stat buf;
    unsigned long size;

    FILE *fp = fopen(filename, "r");

    if (NULL == fp)
	return;

    stat(filename, &buf);
    size = buf.st_size;

    memset(&info, 0, sizeof(info));
    fseek(fp, size - 128, SEEK_SET);
    if (getc(fp) == 0x54)
	if (getc(fp) == 0x41)
	    if (getc(fp) == 0x47) {
		fread(info.name, 30, 1, fp);
		change_char(info.name, 30);
		fread(info.artist, 30, 1, fp);
		change_char(info.artist, 30);
		fread(info.album, 30, 1, fp);
		change_char(info.album, 30);
		fread(info.year, 7, 1, fp);
		change_char(info.year, 7);
		fread(info.comment, 30, 1, fp);
		change_char(info.comment, 30);
	    }

    if (fp) {
	fclose(fp);
    }

    get_song_length(filename);

}

void
fspl_panel::get_files_from_list(char *file)
{

    FILE *fp = fopen(file, "r");

    if (!fp)
	return;

    char buf[MAX_NAME_LENGTH];
    char name[MAX_NAME_LENGTH];
    char time_label[16];

    free_titles();

    while (fgets(buf, MAX_NAME_LENGTH, fp)) {
	if (nodes >= 254)
	    break;
	buf[strlen(buf) - 1] = '\0';

	int len = strlen(buf);

	// get the filename
	strcpy(name, buf);
	for (int idx = len; idx >= 0; idx--) {
	    if (buf[idx] == '/') {
		strcpy(name, &buf[idx + 1]);
		break;
	    }
	}

	int index = musics.SearchPlayList(name);

	if (index != -1) {
	    char *path = musics.GetPlayListPath(index);

	    titles[nodes] = (char *) calloc(strlen(name) + 2, sizeof(char));
	    paths[nodes] = (char *) calloc(strlen(path) + 1, sizeof(char));
	    strcpy(titles[nodes], name);
	    strcpy(paths[nodes], path);
	    get_song_info(paths[nodes]);
	    nodes++;
	    if (info.artist && (0 != strcmp(info.artist, ""))) {
		strcpy(name, info.artist);
		if (info.name) {
		    strcat(name, " - ");
		    strcat(name, info.name);
		}
	    }
	    if (-1 != info.length) {
		int s = 0;
		int m = 0;
		m = info.length / 60;
		s = info.length % 60;
		if (s < 10)
		    sprintf(time_label, "%d:0%d", m, s);
		else
		    sprintf(time_label, "%d:%d", m, s);
	    }
	    mp3Browser->add_next(name, time_label, 0, 0);
	}
    }

    fclose(fp);
}

void
fspl_panel::get_files(char *dir)
{

    int n = 0;
    char name[MAX_NAME_LENGTH];
    char time_label[16];
    int m = 0;
    int s = 0;
    struct dirent **namelist;

    n = scandir(dir, &namelist, 0, 0);
    if (n < 0) {
	perror("scandir");
	return;
    } else {
	for (int idx = 0; idx < n; idx++) {
	    if (strstr(namelist[idx]->d_name, ".mp3")
		|| strstr(namelist[idx]->d_name, ".MP3")) {
		if (nodes >= 254)
		    continue;
		strcpy(name, dir);
		strcat(name, "/");
		strcat(name, namelist[idx]->d_name);
		titles[nodes] =
		    (char *) calloc(strlen(namelist[idx]->d_name) + 2,
				    sizeof(char));
		paths[nodes] =
		    (char *) calloc(strlen(name) + 2, sizeof(char));
		strcpy(titles[nodes], namelist[idx]->d_name);
		strcpy(paths[nodes], name);
		nodes++;
		//musics.AddPlayList(namelist[idx]->d_name, name);
		get_song_info(name);
		if (info.artist && (0 != strcmp(info.artist, ""))) {
		    strcpy(name, info.artist);
		    if (info.name) {
			strcat(name, " - ");
			strcat(name, info.name);
		    }
		} else {
		    strcpy(name, namelist[idx]->d_name);
		}
		if (-1 != info.length) {
		    m = info.length / 60;
		    s = info.length % 60;
		    if (s < 10)
			sprintf(time_label, "%d:0%d", m, s);
		    else
			sprintf(time_label, "%d:%d", m, s);
		}
		mp3Browser->add_next(name, time_label, 0, 0);
	    }
	}
	for (int idx = 0; idx < n; idx++) {
	    if (listNum >= 254) {
		free(namelist[idx]);
		continue;
	    }
	    char *f_ext;
	    if (strstr(namelist[idx]->d_name, ".m3u") ||
		strstr(namelist[idx]->d_name, ".M3U")) {
		strcpy(name, dir);
		strcat(name, namelist[idx]->d_name);
		playList[listNum] =
		    (char *) calloc(strlen(namelist[idx]->d_name) +
				    strlen(dir) + 2, sizeof(char));

		char list_name[255];
		strcpy(list_name, namelist[idx]->d_name);
		strcpy(playList[listNum], dir);
		strcat(playList[listNum], "/");
		strcat(playList[listNum], namelist[idx]->d_name);
		f_ext = strstr(list_name, ".m3u");
		if (!f_ext)
		    f_ext = strstr(list_name, ".M3U");
		f_ext[0] = '\0';
		playListMenu->add(list_name);
		listNum++;
	    }
	    free(namelist[idx]);
	}
    }
    free(namelist);

    struct dirent *dent;
    DIR *p_dir = opendir(dir);
    struct stat stat_buf;
    char *f_name;

    while ((dent = readdir(p_dir)) != NULL) {
	if (0 == strcmp("..", dent->d_name) || 0 == strcmp(".", dent->d_name))
	    continue;
	f_name =
	    (char *) calloc(strlen(dir) + strlen(dent->d_name) + 2,
			    sizeof(char));
	sprintf(f_name, "%s/%s", dir, dent->d_name);
	stat(f_name, &stat_buf);
	if (S_ISDIR(stat_buf.st_mode))
	    get_files(f_name);
	if (NULL != f_name)
	    free(f_name);
    }
    if (p_dir)
	closedir(p_dir);

}

void
fspl_panel::reload_browser(char *file)
{
    get_files_from_list(file);

    free_play_list();

    for (int idx = 0; idx < nodes; idx++) {
	musics.AddPlayList(titles[idx], paths[idx]);
    }
}

static char *
get_mnt_pt(char *mount)
{
    int idx = 0;

    if (mount) {
	while (mount[idx] != ' ')
	    idx++;
	mount[idx] = '\0';
	return mount;
    }

    return NULL;

}

void
fspl_panel::load_browser()
{
    get_files(defaultMusicPath);

    char buf[MAX_NAME_LENGTH];
    FILE *fp = fopen("/proc/mounts", "r");
    char *mount;

    if (fp) {
	while (fgets(buf, MAX_NAME_LENGTH, fp)) {
	    if ((mount = strstr(buf, "/mnt/cf"))) {
		if ((mount = get_mnt_pt(mount)))
		    get_files(mount);
	    }
	    if ((mount = strstr(buf, "/fs/hd/"))) {
		if ((mount = get_mnt_pt(mount)))
		    get_files(mount);
	    }
	}
    }
    if (fp)
	fclose(fp);

    free_play_list();

    for (int idx = 0; idx < nodes; idx++) {
	musics.AddPlayList(titles[idx], paths[idx]);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Callback methods
////////////////////////////////////////////////////////////////////////////////


void
fspl_panel::menu_callback(Fl_Widget * w, void *o)
{

    fspl_panel *p_This = (fspl_panel *) (NxApp::Instance());
    NxMenuButton *m_Button = (NxMenuButton *) w;

    if (menuValue == m_Button->value())
	return;

    stop_callback(stopButton, (void *) 1);
    usleep(200000);
    menuValue = m_Button->value();

    m_Button->label(m_Button->text());

    m_Button->hide();
    m_Button->show();

    //p_This->free_play_list();

    p_This->clear_browser();
    p_This->free_paths();

    if (0 != strcmp("All", m_Button->label()))
	p_This->reload_browser(playList[m_Button->value() - 1]);
    else
	p_This->load_browser();

    mp3Browser->select_range(mp3Browser->traverse_start(),
			     mp3Browser->traverse_start());

    if (mp3Browser->traverse_start())
	selPlay = 1;
    else
	selPlay = 0;
}

//--------------------------------------------------------------------------------

void
fspl_panel::pause_callback(Fl_Widget *, void *)
{
    if (music_ispause()) {
	music_unpause();
    } else {
	music_pause();
    }
}

//--------------------------------------------------------------------------------

void
fspl_panel::stop_callback(Fl_Widget * w, void *o)
{

    int *val = (int *) o;
    fspl_panel *p_This = (fspl_panel *) (NxApp::Instance());

    music_stop();

    if (val)
	stopPressed = 1;

    if (playNode) {
	int index = musics.SearchPlayList(titles[curPlaying - 1]);
	if (-1 != index) {
	    char file_name[MAX_NAME_LENGTH];
	    char time_label[16];
	    char *path = musics.GetPlayListPath(index);

	    strcpy(file_name, path);

	    int s = p_This->get_song_length(file_name);

	    if (-1 != s) {
		int m = s / 60;

		s = s % 60;
		if (s < 10)
		    sprintf(time_label, "%d:0%d", m, s);
		else
		    sprintf(time_label, "%d:%d", m, s);
	    } else
		sprintf(time_label, "--:--");

	    playNode->pixmap(NULL);
	    playNode->time_label(time_label);
	    mp3Browser->redraw();
	}
    }

    curPlaying = 0;
    playNode = NULL;
    p_This->stopButton->hide();
    p_This->playButton->show();

}

//--------------------------------------------------------------------------------

void
fspl_panel::play_callback(Fl_Widget *, void *o)
{
    if (!musics.stop) {
	return;
    }

    if (selPlay <= 0 || mp3Browser->selected() == NULL)
	return;

    //Fl_ToggleNode *sel_node = mp3Browser->selected();
    fspl_panel *p_This = (fspl_panel *) o;

    char *playing = new char[MAX_NAME_LENGTH];
    char *buffer = new char[MAX_NAME_LENGTH];

    if (doubleClicked || curPlaying) {
	strcpy(playing, titles[curPlaying - 1]);
    } else {
	Fl_Toggle_Node *node = mp3Browser->traverse_start();
	int idx = 1;

	while (node) {
	    if (idx == selPlay) {
		playNode = (Mp3_Node *) node;
		playNode->pixmap(musicNote);
		mp3Browser->select_range(playNode, playNode);
		break;
	    }
	    node = mp3Browser->traverse_forward();
	    idx++;
	}
	strcpy(playing, titles[selPlay - 1]);
	if (!node) {
	    delete[]playing;
	    delete[]buffer;
	    buffer = playing = 0;
	    return;
	}
	curPlaying = idx;
	strcpy(playing, titles[selPlay - 1]);
    }

    p_This->StripFormatCodes(playing);
    strncpy(buffer, "@C4@s@.", MAX_NAME_LENGTH - 1);
    strcat(buffer, playing);
    SetPlayList(playing);
    music_play();
    stopPressed = 0;
    doubleClicked = 0;

    delete[]playing;
    playing = 0;
    delete[]buffer;
    buffer = 0;

    p_This->playButton->hide();
    p_This->stopButton->show();

}

//--------------------------------------------------------------------------------

void
fspl_panel::back_callback(Fl_Widget *, void *)
{

    if (curPlaying) {
	int play_me = curPlaying;

	stop_callback(stopButton, 0);
	usleep(200000);
	if (play_me == 1) {
	    curPlaying = nodes;
	    Fl_Toggle_Node *node = mp3Browser->traverse_start();
	    int idx = 1;
	    while (node) {
		if (idx == nodes)
		    break;
		node = mp3Browser->traverse_forward();
		idx++;
	    }
	    playNode = (Mp3_Node *) (node);

	} else {
	    int idx = 1;
	    Fl_Toggle_Node *node;

	    curPlaying = --play_me;
	    node = mp3Browser->traverse_start();
	    while (node) {
		if (play_me == idx) {
		    playNode = (Mp3_Node *) node;
		    break;
		}
		idx++;
		node = mp3Browser->traverse_forward();
	    }
	}
	selPlay = curPlaying;
	mp3Browser->select_range(playNode, playNode);
	playNode->pixmap(musicNote);
	play_callback();
    } else {
	Fl_Toggle_Node *node = mp3Browser->traverse_start();

	if (selPlay == 1) {
	    int idx = 1;
	    while (node) {
		if (idx == nodes)
		    break;
		node = mp3Browser->traverse_forward();
		idx++;
	    }
	    selPlay = nodes;
	    mp3Browser->select_range(node, node);
	} else {
	    int idx = 1;
	    selPlay--;
	    while (node) {
		if (idx == selPlay) {
		    mp3Browser->select_range(node, node);
		    break;
		}
		node = mp3Browser->traverse_forward();
		idx++;
	    }
	}
    }
    mp3Browser->redraw();
}

//--------------------------------------------------------------------------------

void
fspl_panel::forward_callback(Fl_Widget * w, void *o)
{

    if (curPlaying) {
	int play_me = curPlaying;

	stop_callback(stopButton, 0);
	usleep(200000);
	if (play_me == nodes) {
	    curPlaying = 1;
	    playNode = (Mp3_Node *) (mp3Browser->traverse_start());
	} else {
	    int idx = 1;
	    Fl_Toggle_Node *node;

	    curPlaying = ++play_me;
	    node = mp3Browser->traverse_start();
	    while (node) {
		if (play_me == idx) {
		    playNode = (Mp3_Node *) node;
		    break;
		}
		idx++;
		node = mp3Browser->traverse_forward();
	    }
	}
	selPlay = curPlaying;
	mp3Browser->select_range(playNode, playNode);
	playNode->pixmap(musicNote);
	play_callback();
    } else {
	Fl_Toggle_Node *node = mp3Browser->traverse_start();

	if (selPlay == nodes) {
	    selPlay = 1;
	    mp3Browser->select_range(node, node);
	} else {
	    int idx = 1;
	    selPlay++;
	    while (node) {
		if (idx == selPlay) {
		    mp3Browser->select_range(node, node);
		    break;
		}
		node = mp3Browser->traverse_forward();
		idx++;
	    }
	}
    }
    mp3Browser->redraw();
}

//--------------------------------------------------------------------------------

void
fspl_panel::volume_callback(Fl_Widget * o, void *)
{

    // FIXME need to get value off of a valuator    
    int volume = (int) ((NxSlider *) o)->value();
    int handle;
    int r;

    handle = open("/dev/mixer", O_RDWR);
    if (volume > 100)
	volume = 100;
    if (volume >= 0) {
	r = (volume << 8) | volume;

	ioctl(handle, MIXER_WRITE(SOUND_MIXER_VOLUME), &r);
    }
    ioctl(handle, MIXER_READ(SOUND_MIXER_VOLUME), &r);

    close(handle);
}

//--------------------------------------------------------------------------------

void
fspl_panel::sspectra_callback(Fl_Widget * o, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::progress_callback(Fl_Widget * o, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::timer_callback(void *)
{

    Fl::add_timeout(0.25, timer_callback);

    char time_label[10];

    if (1 == musics.updatestats) {
	musics.updatestats = 0;

    }
    if (!musics.stop && curPlaying) {
	int s = musics.currentframe * musics.pcmperframe / musics.freq;
	int ms = musics.maxframe * musics.pcmperframe / musics.freq;

	s = ms - s;
	int m = s / 60;
	s = s % 60;

	if (s < 10)
	    sprintf(time_label, "%d:0%d", m, s);
	else
	    sprintf(time_label, "%d:%d", m, s);

	if (playNode)
	    playNode->time_label(time_label);
	mp3Browser->redraw();

    } else if (musics.stop && curPlaying) {
	forward_callback();
    } else {
	if (curPlaying) {
	    stop_callback(stopButton, (void *) 0);
	}
    }
}

//--------------------------------------------------------------------------------

void
fspl_panel::normal_callback(Fl_Widget *, void *)
{
    SetDsp(0);
}

//--------------------------------------------------------------------------------

void
fspl_panel::add_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::fileok_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::filecan_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::addall_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------
void
fspl_panel::del_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::delall_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------

void
fspl_panel::hideshowpl_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------
void
fspl_panel::browser_callback(Fl_Widget *, void *)
{

    Fl_Toggle_Node *sel_node = mp3Browser->selected();
    Fl_Toggle_Node *node = mp3Browser->traverse_start();
    int idx = 1;

    if (Fl::event_clicks()) {
	stop_callback(stopButton, (void *) 0);
	usleep(200000);
	doubleClicked = 1;
	while (node) {
	    if (node == sel_node) {
		playNode = (Mp3_Node *) node;
		curPlaying = idx;
		selPlay = idx;
	    }
	    node->pixmap(NULL);
	    node = mp3Browser->traverse_forward();
	    idx++;
	}

	if (sel_node) {
	    sel_node->pixmap(musicNote);
	}

	play_callback();
    } else {
	doubleClicked = 0;
	while (node) {
	    if (node == sel_node) {
		selPlay = idx;
		break;
	    }
	    node = mp3Browser->traverse_forward();
	    idx++;
	}
    }
}

//--------------------------------------------------------------------------------

void
fspl_panel::http_callback(Fl_Widget *, void *)
{
}

//--------------------------------------------------------------------------------
void
fspl_panel::StripFormatCodes(char *title)
{

    char *buffer = new char[MAX_NAME_LENGTH];

    for (int i = 0; i < MAX_NAME_LENGTH; i++) {

	if (title[i] == '@') {

	    if (title[i + 1] == '.') {

		int k = 0;

		for (int j = i + 2; j < MAX_NAME_LENGTH + 2; j++, k++) {
		    buffer[k] = title[j];
		}

		strcpy(title, buffer);
		break;
	    }

	}

    }
    delete[]buffer;
    buffer = 0;
}
