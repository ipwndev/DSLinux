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


#ifndef _NXWEEK_H_
#define _NXWEEK_H_

#include <time.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <nxbox.h>
#include <nxdb.h>
#include <nxbutton.h>

#include <stdio.h>
#define BUTTON_W 15
#define MAX_COUNT 2

#define APPT_MASK  	0x0F
#define MIN_INC			5
#define HOUR_INC    12
#define MAX_VAL			4

enum Appt_Overlay
{
    NO_OVERLAP,
    SINGLE = 1,
    LEFT = 4,
    RIGHT = 8,
};

class WeekGrid;

typedef struct appt_
{				// appointmet structure
    float x;
    float y;
    float w;
    float h;
    int overlay;
    int recno;
    time_t start_time;
    time_t end_time;
    appt_ *next_appt;
    appt_ *prev_appt;
}
appt;

class GridButton:public NxButton
{
    WeekGrid *grid_;
    void draw();
  public:
      GridButton(int X, int Y, int W, int H, const char *L):NxButton(X, Y, W,
								     H,
								     (char *)
								     L)
    {
	;
    }
    void SetGrid(WeekGrid * grid)
    {
	grid_ = grid;
    }
};

class WeekGrid:public Fl_Widget
{
    int save_h;
    short info_draw;
    uchar appt_array[7][288];
    appt *first_appt;
    float pix_ratio;
    float day_pix;
    NxDb *db;
    char db_name[255];
    void draw();
    int handle(int);
    float GetOffSet(float val);
    void DrawAppointments();
    void DrawOverlaps(int dy);
    void DrawRects(int dy);
    void GetAppointments();
    void ShowInfo(int X, int Y);
    int Overlap(appt * p_Appt);
    int GetGridVal();
    void GetOverlap();
    void GetOverlapDim(appt * p_Appt, appt * p_Prev);
    void GetApptDim(appt * p_Appt);
    void SetInfoBox(appt * p_Appt);
    bool MakeAppt(appt * p_Appt, int recno, time_t start_time,
		  time_t end_time);
    bool IsForWeek(time_t start_time, time_t end_time);
    int scroll_count;
    int b_height;
    int b_width;
    int font_;
    int f_size_;
    uchar color_;
    uchar txt_color_;
    uchar line_color_;
    time_t date_sunday;
    time_t date_mark;
    int num_recs;
    NxBox *output_box;
  public:
      WeekGrid(int X, int Y, int H, int W, const char *L = 0);
     ~WeekGrid();
    void color(uchar col)
    {
	color_ = col;
    }
    Fl_Color color() const
    {
	return (Fl_Color) color_;
    }
    void txt_color(uchar col)
    {
	txt_color_ = col;
    }
    Fl_Color txt_color()
    {
	return (Fl_Color) txt_color_;
    }
    void line_color(uchar col)
    {
	line_color_ = col;
    }
    Fl_Color line_color() const
    {
	return (Fl_Color) line_color_;
    }
    void SetDb(NxDb * pDb, char *name)
    {
	db = pDb;
	strcpy(db_name, name);
    }
    int h()
    {
	return (int) (6.5 * b_height);
    }
    void SetFont(int font, int size)
    {
	font_ = font;
	f_size_ = size;
    }
    void SetDateSunday(time_t date);
    void SetOutputBox(NxBox * out_)
    {
	output_box = out_;
    }
    int UpCount();
    int DownCount();
    virtual void resize(int X, int Y, int W, int H);

};

void free_appts(appt * app);
appt *appt_alloc(void);

#endif
