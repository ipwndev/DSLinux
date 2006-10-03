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


#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include "fspl_panel.h"
#include "comm.h"
#include "mp3utils.h"

#define MAX_NAME_LENGTH 1024
#define DEFAULT_MUSIC_PATH "/MP3/"

fspl_panel *MainPanel;
extern class_musics musics;
extern class_music music;
Fl_Hold_Browser *plBrowser;
Fl_Button *plHideShow;
Fl_Button *plAdd;
Fl_Button *plAddAll;
Fl_Button *plDel;
Fl_Button *plDelAll;
Mp3Utils mp3;
int curPlaying = 0;
int stopPressed = 1;

extern "C"
{
    void pause_callback(Fl_Widget *, void *);
    void stop_callback(Fl_Widget * o = 0, void *vp = 0);
    void play_callback(Fl_Widget * o = 0, void *vp = 0);
    void back_callback(Fl_Widget *, void *);
    void forward_callback(Fl_Widget *, void *);
    void volume_callback(Fl_Widget *, void *);
    void progress_callback(Fl_Widget * o, void *);
    void timer_callback(void *);
    void feedback_callback(Fl_Widget *, void *);
    void normal_callback(Fl_Widget *, void *);
    void add_callback(Fl_Widget *, void *);
    void addall_callback(Fl_Widget *, void *);
    void del_callback(Fl_Widget *, void *);
    void delall_callback(Fl_Widget *, void *);
    void hideshowpl_callback(Fl_Widget *, void *);
}

#endif				// CALLBACKS_H
