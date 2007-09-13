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


#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include "nxweek.h"
#include <stdio.h>
#include <stdlib.h>
#include "nxschedule.h"

#include <nxapp.h>

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

WeekGrid::WeekGrid(int X, int Y, int W, int H, const char *L):
Fl_Widget(X, Y, W, H, L)
{
    info_draw = 0;
    b_height = H / 24;
    Fl_Widget::h((int) (6.5 * b_height));
    save_h = Fl_Widget::h();
    b_width = w() / 7;
    scroll_count = 1;
    color_ = FL_WHITE;
    line_color_ = txt_color_ = FL_BLACK;
    font_ = FL_HELVETICA;
    f_size_ = FL_NORMAL_SIZE;
    date_sunday = 0;
    date_mark = 0;
    memset(db_name, 0, sizeof(db_name));
    db = NULL;
    first_appt = NULL;
    pix_ratio = (float) b_height / 7200;	//pix per second
    day_pix = 86400 * pix_ratio;
    num_recs = 0;
    memset(appt_array, 0, sizeof(appt_array));
    output_box = NULL;
}

WeekGrid::~WeekGrid()
{
    if (first_appt)
	free_appts(first_appt);
}

void
WeekGrid::resize(int X, int Y, int W, int H)
{

}

void
WeekGrid::draw()
{
    int X;
    int Y;
    char buf[10];

    if (info_draw) {
	ShowInfo(Fl::event_x(), Fl::event_y());
	return;
    }

    fl_color(color_);
    fl_rectf(x(), y(), w(), (int) (6.5 * b_height));
    fl_color(line_color_);
    fl_rect(x(), y(), w(), (int) (6.5 * b_height));

    for (X = 1; X < 7; X++)
	fl_line(x() + (X * b_width), y(), x() + (X * b_width),
		(int) (6.5 * b_height) + y() - 1);
    for (Y = 1; Y < 7; Y++)
	fl_line(x(), y() + (Y * b_height), x() + w() - 1,
		y() + (Y * b_height));

    int idx = 0;

    switch (scroll_count) {
    case 0:
	idx = 0;
	break;
    case 1:
	idx = 8;
	break;
    case 2:
	idx = 12;
	break;
    default:
	idx = 0;
	break;
    }

    int count = 0;
    for (; idx <= 24 && count < 7; idx++) {
	int val = idx % 12;
	if (0 != idx % 2)
	    continue;
	if (0 == val)
	    val = 12;
	if (idx < 12 || idx == 24)
	    sprintf(buf, "%d:00 am", val);
	else
	    sprintf(buf, "%d:00 pm", val);
	//fl_font(font_, f_size_);
	fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
	fl_draw(buf, 25, y() + ((count * b_height) - ((int) fl_height()) - 2),
		2 * BUTTON_W, 2 * BUTTON_W, FL_ALIGN_RIGHT);
	count++;
    }

    // need to get the appointmets and put them on the grid
    DrawAppointments();
}

int
WeekGrid::UpCount()
{
    if (MAX_COUNT == scroll_count)
	return 0;
    scroll_count++;
    return 1;
}

int
WeekGrid::DownCount()
{
    if (0 == scroll_count)
	return 0;
    scroll_count--;
    return 1;
}

int
WeekGrid::Overlap(appt * p_Appt)
{
    time_t new_time = 0;
    tm *tt;
    int max_val = 0;
    int low_val = 0;
    int hi_val = 0;
    int t_hour = 0;
    int t_min = 0;
    int col = 0;
    int row = 0;
    int h1 = 0;
    int idx = 0;

    if (NULL == p_Appt)
	return 0;

    new_time = p_Appt->start_time;
    tt = localtime(&new_time);
    t_hour = tt->tm_hour;
    t_min = tt->tm_min;
    col = tt->tm_wday;
    row = (t_min / MIN_INC) + (t_hour * HOUR_INC);

    new_time = p_Appt->end_time;
    tt = localtime(&new_time);
    t_hour = tt->tm_hour;
    t_min = tt->tm_min;
    h1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

    h1 = h1 - row;

    for (idx = 0; idx < h1; idx++) {
	if (LEFT & p_Appt->overlay) {	// go hi
	    hi_val = appt_array[col][row + idx] >> 4;
	    if (hi_val > max_val)
		max_val = hi_val;
	} else {		// go low
	    low_val = appt_array[col][row + idx] & APPT_MASK;
	    if (low_val > max_val)
		max_val = low_val;
	}
    }
    return max_val;
}

void
WeekGrid::GetOverlapDim(appt * p_Appt, appt * p_Prev)
{
    time_t new_time = 0;
    tm *tt;
    int t_hour = 0;
    int t_min = 0;
    int h1 = 0;
    int h2 = 0;
    int col = 0;
    int row1 = 0;
    int row2 = 0;
    int idx = 0;
    uchar hi_val = 0;
    uchar low_val = 0;
    uchar val = 0;

    if (NULL == p_Appt && NULL == p_Prev)
	return;

    if (NULL == p_Appt)		// single appt only
	new_time = p_Prev->start_time;
    else
	new_time = p_Appt->start_time;

    tt = localtime(&new_time);
    t_hour = tt->tm_hour;
    t_min = tt->tm_min;
    col = tt->tm_wday;
    row1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

    if (NULL == p_Appt) {	// single only
	new_time = p_Prev->end_time;
	tt = localtime(&new_time);
	t_hour = tt->tm_hour;
	t_min = tt->tm_min;
	h1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);
	h1 = h1 - row1;
	for (idx = 0; idx < h1; idx++) {
	    val = appt_array[col][row1 + idx];
	    val &= APPT_MASK;
	    if (val < MAX_VAL) {
		appt_array[col][row1 + idx]++;
	    }
	    low_val = appt_array[col][row1 + idx] & APPT_MASK;
	    val = appt_array[col][row1 + idx] >> 4;
	    if (val < MAX_VAL)
		val++;
	    val = val << 4;
	    appt_array[col][row1 + idx] = val | low_val;
	}
    } else if (NULL == p_Prev) {	// float on top of float
	new_time = p_Appt->end_time;
	tt = localtime(&new_time);
	t_hour = tt->tm_hour;
	t_min = tt->tm_min;
	h1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);
	h1 = h1 - row1;
	for (idx = 0; idx < h1; idx++) {
	    if (RIGHT & p_Appt->overlay) {
		hi_val = appt_array[col][row1 + idx] >> 4;
		val = appt_array[col][row1 + idx] & APPT_MASK;
		if (MAX_VAL > val) {
		    val++;
		}
		hi_val = hi_val << 4;
		appt_array[col][row1 + idx] = hi_val | val;
	    } else {
		low_val = appt_array[col][row1 + idx] & APPT_MASK;
		val = appt_array[col][row1 + idx] >> 4;
		if (val < MAX_VAL)
		    val++;
		val = val << 4;
		appt_array[col][row1 + idx] = val | low_val;
	    }
	}
    } else {			// single on top of single
	if (p_Appt->y >= p_Prev->y) {
	    p_Appt->overlay = RIGHT;
	    p_Prev->overlay = LEFT;
	} else {
	    p_Appt->overlay = LEFT;
	    p_Prev->overlay = RIGHT;
	}

	// zero out a side and add other appt
	if (p_Appt->overlay & RIGHT) {
	    new_time = p_Prev->start_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    row1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    new_time = p_Prev->end_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    h1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    h1 = h1 - row1;
	    for (idx = 0; idx < h1; idx++) {	// clear out bits
		val = appt_array[col][row1 + idx] & APPT_MASK;
		if (val > 0)
		    appt_array[col][row1 + idx]--;
	    }

	    new_time = p_Appt->start_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    row2 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    new_time = p_Appt->end_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    h2 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    h2 = h2 - row2;
	    for (idx = 0; idx < h2; idx++) {	// add bits
		val = appt_array[col][row2 + idx] & APPT_MASK;
		if (val < MAX_VAL) {
		    appt_array[col][row2 + idx]++;
		}
	    }
	} else {
	    new_time = p_Prev->start_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    row1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    new_time = p_Prev->end_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    h1 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    h1 = h1 - row1;
	    for (idx = 0; idx < h1; idx++) {	// clear out bits
		low_val = appt_array[col][row1 + idx] & APPT_MASK;
		val = appt_array[col][row1 + idx] >> 4;
		if (val > 0)
		    val--;
		val = val << 4;
		appt_array[col][row1 + idx] = val | low_val;
	    }

	    new_time = p_Appt->start_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    row2 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    new_time = p_Appt->end_time;
	    tt = localtime(&new_time);
	    t_hour = tt->tm_hour;
	    t_min = tt->tm_min;
	    h2 = (t_min / MIN_INC) + (t_hour * HOUR_INC);

	    h2 = h2 - row2;
	    for (idx = 0; idx < h2; idx++) {	// add bits
		low_val = appt_array[col][row2 + idx] & APPT_MASK;
		val = appt_array[col][row2 + idx] >> 4;
		if (val < MAX_VAL)
		    val++;
		val = val << 4;
		appt_array[col][row2 + idx] = val | low_val;
	    }
	}
    }
}

void
WeekGrid::GetOverlap()
{
    appt *p_PrevAppt = NULL;
    appt *p_CurAppt = NULL;
    appt *temp_appt = NULL;
    appt *mark_appt = NULL;
    appt *r_mark_appt = NULL;
    appt *l_mark_appt = NULL;
    tm *t1;
    int yday = 0;
    int yday2 = 0;
    time_t new_time = 0;
    int overlay = 0;
    int left = 0;
    int temp_left;
    int single = 1;

    p_CurAppt = first_appt;

    while (p_CurAppt) {
	single = 1;
	new_time = p_CurAppt->start_time;
	t1 = localtime(&new_time);
	yday = t1->tm_yday;
	left = !left;
	if (yday != yday2) {	// first for day
	    left = 0;
	    yday2 = yday;
	    GetOverlapDim(NULL, p_CurAppt);
	    p_CurAppt = p_CurAppt->next_appt;
	    continue;
	}
	p_PrevAppt = p_CurAppt->prev_appt;
	while (p_PrevAppt) {
	    if ((p_CurAppt->y >= p_PrevAppt->y)
		&& (p_CurAppt->y < (p_PrevAppt->y + p_PrevAppt->h))) {
		single = 0;
		r_mark_appt = NULL;
		l_mark_appt = NULL;

		overlay = p_PrevAppt->overlay;
		temp_appt = p_CurAppt->prev_appt;

		temp_left = left;
		temp_appt = p_CurAppt->prev_appt;
		mark_appt = NULL;

		if ((overlay & LEFT) || (overlay & RIGHT)) {	// go on top of shorter overlay
		    if (left) {	// can it go left alone check
			while (temp_appt) {
			    if ((LEFT & temp_appt->overlay)
				&& (p_CurAppt->y >= temp_appt->y)
				&& (p_CurAppt->y <
				    (temp_appt->y + temp_appt->h))) {
				l_mark_appt = temp_appt;
				break;
			    }
			    temp_appt = temp_appt->prev_appt;
			}
			// else check to see if it can go right
			if (l_mark_appt) {
			    left = 0;
			    temp_appt = p_CurAppt->prev_appt;
			    while (temp_appt) {
				if ((RIGHT & temp_appt->overlay)
				    && (p_CurAppt->y >= temp_appt->y)
				    && (p_CurAppt->y <
					(temp_appt->y + temp_appt->h))) {
				    r_mark_appt = temp_appt;
				    break;
				}
				temp_appt = temp_appt->prev_appt;
			    }
			    if (r_mark_appt) {	// can't go right either which one ends first prev or mark?
				if ((l_mark_appt->y + l_mark_appt->h) >
				    (r_mark_appt->y + r_mark_appt->h)) {
				    mark_appt = r_mark_appt;
				    left = 0;
				} else {
				    mark_appt = l_mark_appt;
				    left = 1;
				}
			    }
			}
		    } else {	// check to see if it can go right alone 
			DPRINT("going right\n");
			temp_appt = p_CurAppt->prev_appt;
			while (temp_appt) {
			    if ((RIGHT & temp_appt->overlay)
				&& (p_CurAppt->y >= temp_appt->y)
				&& (p_CurAppt->y <
				    (temp_appt->y + temp_appt->h))) {
				r_mark_appt = temp_appt;
				break;
			    }
			    temp_appt = temp_appt->prev_appt;
			}
			// else check to see if can go left alone       
			if (r_mark_appt) {
			    left = 1;
			    temp_appt = p_CurAppt->prev_appt;
			    while (temp_appt) {
				if ((LEFT & temp_appt->overlay)
				    && (p_CurAppt->y >= temp_appt->y)
				    && (p_CurAppt->y <
					(temp_appt->y + temp_appt->h))) {
				    l_mark_appt = temp_appt;
				    break;
				}
				temp_appt = temp_appt->prev_appt;
			    }
			    if (l_mark_appt) {	// can't go left either which one ends first?
				if ((l_mark_appt->y + l_mark_appt->h) >
				    (r_mark_appt->y + r_mark_appt->h)) {
				    mark_appt = r_mark_appt;
				    left = 0;
				} else {
				    mark_appt = l_mark_appt;
				    left = 1;
				}
			    }
			}
		    }
		    if (l_mark_appt && r_mark_appt) {	// deterine which has bigger overlay
			int l_size = 0;
			int r_size = 0;

			r_size = Overlap(r_mark_appt);
			l_size = Overlap(l_mark_appt);

			if (r_size < l_size) {
			    left = 0;
			    mark_appt = r_mark_appt;
			} else if (r_size > l_size) {
			    mark_appt = l_mark_appt;
			    left = 1;
			} else
			    left = temp_left;
		    }

		    DPRINT("IN DOUBLE\n");
		    if (mark_appt && (temp_left != left)) {	// went the other way on top of another
			DPRINT("other way on top\n");
			if (left)
			    p_CurAppt->overlay = LEFT;
			else {
			    p_CurAppt->overlay = RIGHT;
			}
		    } else if (!mark_appt && (temp_left != left)) {	// went the other way alone
			DPRINT("other way alone\n");
			if (left)
			    p_CurAppt->overlay = LEFT;
			else {
			    p_CurAppt->overlay = RIGHT;
			}
		    } else if (!mark_appt && (left == temp_left)) {	// same way alone
			DPRINT("same alone\n");
			if (left)
			    p_CurAppt->overlay = LEFT;
			else {
			    p_CurAppt->overlay = RIGHT;
			}
		    } else {	// went the same way on top
			DPRINT("same on top\n");
			if (left)
			    p_CurAppt->overlay = LEFT;
			else {
			    p_CurAppt->overlay = RIGHT;
			}
		    }
		    GetOverlapDim(p_CurAppt, NULL);
		    break;
		} else {	// single overlay
		    GetOverlapDim(p_CurAppt, p_PrevAppt);
		    break;
		}
	    }
	    p_PrevAppt = p_PrevAppt->prev_appt;
	}
	if (single) {
	    DPRINT("GetOverlap: setting single appt overlay dim\n");
	    GetOverlapDim(NULL, p_CurAppt);
	}
	p_CurAppt = p_CurAppt->next_appt;
    }

}

void
WeekGrid::GetAppointments()
{
    int rec_array[255];
    int idx = 0;
    int jdx = 0;
    int numRecs = db->NumRecs(db_name);
    int recCount = 0;
    bool ret = false;
    appt *p_Appt = NULL;
    time_t start_time;
    time_t end_time;
    int wday = 0;
    tm *tt = NULL;
    NxTodo *note = new NxTodo;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());


    for (idx = 0; idx < 255; idx++)
	rec_array[idx] = -1;

    db->Select(db_name, rec_array, 255);

    idx = -1;

    //if(date_sunday != date_mark || numRecs != num_recs) { 
    memset(appt_array, 0, sizeof(appt_array));
    if (first_appt) {
	free_appts(first_appt);
	first_appt = NULL;
    }
    for (idx = 0; idx < 255; idx++) {
	if (-1 != rec_array[idx]) {
	    for (jdx = 0; jdx < 7; jdx++) {
		pThis->ExtractRecord(note, rec_array[idx]);
		ret = pThis->IsForToday(note, date_sunday + (jdx * 86400),
					date_sunday + ((jdx + 1) * 86400) -
					1);
		if (ret) {
		    tt = localtime(&note->startTime);
		    wday = tt->tm_wday;
		    start_time = note->startTime + ((jdx - wday) * 86400);
		    end_time = note->endTime + ((jdx - wday) * 86400);
		    MakeAppt(p_Appt, rec_array[idx], start_time, end_time);
		}
		if (recCount < numRecs && ret) {
		    p_Appt = appt_alloc();
		}
	    }
	    recCount++;
	}
    }
    date_mark = date_sunday;
    num_recs = numRecs;
    GetOverlap();

    note = NULL;
    delete note;
    //}
}

bool WeekGrid::MakeAppt(appt * p_Appt, int recno, time_t start_time,
			time_t end_time)
{
    appt *
	temp_appt =
	NULL;
    appt
	swap_appt;

    memset(&swap_appt, 0, sizeof(swap_appt));
    swap_appt.recno = recno;
    swap_appt.start_time = start_time;
    swap_appt.end_time = end_time;
    swap_appt.overlay = SINGLE;
    if (!first_appt) {
	first_appt = appt_alloc();
	memcpy(first_appt, &swap_appt, sizeof(swap_appt));
	GetApptDim(first_appt);
	return true;
    }
    memcpy(p_Appt, &swap_appt, sizeof(swap_appt));
    GetApptDim(p_Appt);
    temp_appt = first_appt;
    while (temp_appt) {
	if (temp_appt->start_time >= p_Appt->start_time) {
	    if (temp_appt->prev_appt)
		temp_appt->prev_appt->next_appt = p_Appt;
	    p_Appt->prev_appt = temp_appt->prev_appt;
	    p_Appt->next_appt = temp_appt;
	    temp_appt->prev_appt = p_Appt;
	    if (temp_appt == first_appt) {
		first_appt = p_Appt;
	    }
	    break;
	}
	if (!temp_appt->next_appt) {	// insert here at end
	    p_Appt->prev_appt = temp_appt;
	    temp_appt->next_appt = p_Appt;
	    break;
	}
	temp_appt = temp_appt->next_appt;
    }
    return true;
}

bool WeekGrid::IsForWeek(time_t start_time, time_t end_time)
{
    time_t
	date_sat =
	date_sunday + (7 * 86400) -
	1;

    DPRINT("date_sunday [%ld] date_sat [%ld]\n", date_sunday, date_sat);

    if (start_time >= date_sunday && start_time <= date_sat)
	return true;
    else if (end_time <= date_sat && end_time >= date_sunday)
	return true;
    else
	return false;
}


void
WeekGrid::GetApptDim(appt * p_Appt)
{
    time_t new_time = p_Appt->start_time;
    int hour;
    int min;
    int wday;
    float h;
    float y;
    char buf[16];
    tm *tt = localtime(&new_time);

    p_Appt->x = (b_width * tt->tm_wday);

    hour = tt->tm_hour;
    min = tt->tm_min;
    wday = tt->tm_wday;

    y = (wday * day_pix) + (hour * 3600) * pix_ratio + (min * 60) * pix_ratio;
    sprintf(buf, "%.0f", y);
    p_Appt->y = strtol(buf, NULL, 10);

    new_time = p_Appt->end_time;
    tt = localtime(&new_time);

    hour = tt->tm_hour;
    min = tt->tm_min;
    wday = tt->tm_wday;

    h = (wday * day_pix) + (hour * 3600) * pix_ratio + (min * 60) * pix_ratio;
    h = h - p_Appt->y;
    sprintf(buf, "%.0f", h);
    p_Appt->h = strtol(buf, NULL, 10);
    DPRINT("p_Appt->y [%f] p_Appt->h [%f]\n", p_Appt->y, p_Appt->h);
}

appt *
appt_alloc(void)
{
    appt *p_appt;

    p_appt = (appt *) malloc(sizeof(appt));
    memset(p_appt, 0, sizeof(appt));
    return p_appt;
}

void
free_appts(appt * app)
{
    appt *swap_appt;

    DPRINT("FREEING APPTS\n");
    while (app) {
	swap_appt = app;
	app = app->next_appt;
	DPRINT("FREEING [%d]\n", swap_appt->recno);
	free(swap_appt);
	swap_appt = NULL;
    }
}

float
WeekGrid::GetOffSet(float val)
{
    int num = (int) (val / day_pix);
    float close = num * day_pix;
    float rem = val - close;

    return rem;
}

int
WeekGrid::GetGridVal()
{
    int dy = 0;

    switch (scroll_count) {
    case 0:
	dy = 0;
	break;
    case 1:
	dy = 4 * b_height;
	break;
    case 2:
	dy = 6 * b_height;
	break;
    default:
	dy = 0;
    }
    return dy;
}

void
WeekGrid::DrawAppointments()
{
    int dy = 0;

    if (NULL == db)
	return;

    GetAppointments();
    dy = GetGridVal();
    DrawOverlaps(dy);
    DrawRects(dy);
}

void
WeekGrid::DrawOverlaps(int dy)
{
    int col = 0;
    int row = 0;
    float h_val = 0;
    float dec = 0;
    int last_val = 0;
    int low_val;
    int hi_val;
    int H;
    int start_row = 0;
    float y_val;
    int Y;
    int X;
    int x_off;
    int next_val;
    int width;
    float rem = 0;
    int offset = 0;

    // go low
    for (col = 0; col <= 6; col++) {
	for (row = 0; row <= 287; row++) {
	    if (0 == (appt_array[col][row] & APPT_MASK)) {
		last_val = 0;
		h_val = 0;
		continue;
	    } else {
		low_val = appt_array[col][row] & APPT_MASK;
		next_val = low_val;
		h_val = 1;
		start_row = row;
		H = Y = 0;
		while ((next_val == low_val) && (row <= 287)) {
		    DPRINT("h_val [%f]\n", h_val);
		    h_val++;
		    row++;
		    next_val = appt_array[col][row] & APPT_MASK;
		}

		width = b_width / 2 - 4;
		X = (col * b_width);
		x_off = X + width;

		y_val = (5 * start_row) * 60 * pix_ratio;
		Y = (int) y_val;
		dec = y_val - Y;
		if (dec > .5)
		    y_val++;
		Y = (int) y_val;

		h_val = (5 * h_val) * 60 * pix_ratio;
		H = (int) h_val;
		dec = h_val - H;
		if (dec > .5)
		    h_val++;
		H = (int) h_val;

		DPRINT("Y [%d] x_off [%d] w [%d] H [%d]\n", Y, x_off, width,
		       H);
		DPRINT("dy [%d]\n", dy);
		if (dy > Y - 2) {
		    DPRINT("HERE\n");
		    if ((Y + H) > dy) {
			DPRINT("H big\n");
			H = (Y + H) - dy;
			Y = 0;
		    } else
			H = 0;
		} else {
		    DPRINT("Y [%d] H [%d]\n", Y, H);
		    // correct Y

		    rem = GetOffSet((float) Y);
		    offset = Y;
		    Y = (int) rem - dy;
		    if (offset >= (dy + (int) (6.5 * b_height)))
			H = 0;
		    else if ((offset + H) <= dy)
			H = 0;
		    else if ((offset + H) > (dy + (int) (6.5 * b_height))) {
			H = (int) (6.5 * b_height) - Y;
		    }

		    if (H > (int) (6.5 * b_height))
			H = (int) (6.5 * b_height);

		}

		switch (low_val) {
		case 0:
		    break;
		case 1:
		    fl_color(FL_LIGHT3);
		    break;
		case 2:
		    fl_color(FL_LIGHT1);
		    break;
		case 3:
		    fl_color(FL_DARK1);
		    break;
		case 4:
		    fl_color(FL_DARK3);
		    break;
		default:
		    fl_color(FL_LIGHT3);
		    break;
		}
		DPRINT("Y [%d] x_off [%d] w [%d] H [%d]\n", Y, x_off, width,
		       H);
		if (H > 0 && low_val)
		    fl_rectf(x_off + 4 + x(), Y + y(), width, H - 1);
	    }
	}
    }

    // go high
    for (col = 0; col <= 6; col++) {
	for (row = 0; row <= 287; row++) {
	    if (0 == (appt_array[col][row] >> 4)) {
		last_val = 0;
		h_val = 0;
		continue;
	    } else {
		hi_val = appt_array[col][row] >> 4;
		next_val = hi_val;
		h_val = 1;
		start_row = row;
		while ((next_val == hi_val) && (row <= 287)) {
		    h_val++;
		    row++;
		    next_val = appt_array[col][row] >> 4;
		}

		width = b_width / 2 - 4;
		X = (col * b_width);
		x_off = X;

		y_val = (5 * start_row) * 60 * pix_ratio;
		Y = (int) y_val;
		dec = y_val - Y;
		if (dec > .5)
		    y_val++;
		Y = (int) y_val;

		h_val = (5 * h_val) * 60 * pix_ratio;
		H = (int) h_val;
		dec = h_val - H;
		if (dec > .5)
		    h_val++;
		H = (int) h_val;

		if (dy > Y - 2) {
		    if ((Y + H) > dy) {
			H = (Y + H) - dy;
			Y = 0;
		    } else
			H = 0;
		} else {
		    rem = GetOffSet((float) Y);
		    offset = Y;
		    Y = (int) rem - dy;
		    if (offset >= (dy + (int) (6.5 * b_height)))
			H = 0;
		    else if ((offset + H) <= dy)
			H = 0;
		    else if ((offset + H + 2) > (dy + (int) (6.5 * b_height))) {
			H = (int) (6.5 * b_height) - Y;
		    }

		    if (H > (int) (6.5 * b_height))
			H = (int) (6.5 * b_height);

		}

		switch (hi_val) {
		case 0:
		    break;
		case 1:
		    fl_color(FL_LIGHT3);
		    break;
		case 2:
		    fl_color(FL_LIGHT1);
		    break;
		case 3:
		    fl_color(FL_DARK1);
		    break;
		case 4:
		    fl_color(FL_DARK3);
		    break;
		default:
		    fl_color(FL_LIGHT3);
		    break;
		}

		if (H > 0 && hi_val)
		    fl_rectf(x_off + 4 + x(), Y + y(), width, H - 1);
	    }
	}
    }
}

void
WeekGrid::DrawRects(int dy)
{

    int H;
    int X;
    int Y;
    float rem;
    tm *tt;
    int width = 0;
    int x_off = 0;
    int offset = 0;
    int top_hash = 0;
    int bot_hash = 0;

    appt *p_Appt = first_appt;

    while (p_Appt) {
	time_t new_time = p_Appt->start_time;

	top_hash = 0;
	bot_hash = 0;

	tt = localtime(&new_time);

	rem = GetOffSet(p_Appt->y);
	Y = (int) rem;

	if (dy > Y) {
	    if ((Y + (int) p_Appt->h) > dy) {
		H = (Y + (int) p_Appt->h) - dy;
		Y = 0;
	    } else
		H = 0;
	    top_hash = 1;
	} else {
	    offset = Y;		// off set into dy

	    Y = (int) rem - dy;	// Y value corrected
	    if (offset >= (dy + (int) (6.5 * b_height))) {
		bot_hash = 1;
		H = 0;
	    } else if ((offset + p_Appt->h) <= dy) {
		H = 0;
		top_hash = 1;
	    } else if ((offset + p_Appt->h) > (dy + (int) (6.5 * b_height))) {
		bot_hash = 1;
		H = (int) (6.5 * b_height) - Y;
	    } else
		H = (int) p_Appt->h;

	    if (H > (int) (6.5 * b_height)) {
		bot_hash = 1;
		H = (int) (6.5 * b_height);
	    }

	}
	X = (b_width * tt->tm_wday);

	int inc = (int) b_width / 4;
	if (top_hash) {
	    fl_color(FL_BLACK);
	    fl_rectf(x() + X + inc, y() - 5, b_width - (2 * inc), 2);
	}
	if (bot_hash) {
	    fl_color(FL_BLACK);
	    fl_rectf(x() + X + inc, y() + h() + 3, b_width - (2 * inc), 2);
	}
	DPRINT("DrawRects X [%d]\n", X);

	if (0 < H) {

	    if (H > (int) (6.5 * b_height))
		H = (int) (6.5 * b_height);
	    switch (p_Appt->overlay) {
	    case NO_OVERLAP:	// don't draw
		break;
	    case LEFT:
		DPRINT("In Left\n");
		x_off = X;
		width = b_width / 2 - 4;
		fl_color(FL_BLACK);
		fl_rect(x_off + 4 + x(), Y + y(), width, H);
		break;

	    case RIGHT:
		DPRINT("In Right\n");
		width = b_width / 2 - 4;
		x_off = X + width;
		fl_color(FL_BLACK);
		fl_rect(x_off + 4 + x(), Y + y(), width, H);
		break;

	    default:
		DPRINT("In default\n");
		width = b_width - 7;
		x_off = X;
		fl_color(FL_BLACK);
		fl_rect(x_off + 4 + x(), Y + y(), width, H);
		break;
	    }
	}
	p_Appt = p_Appt->next_appt;
    }
}

void
WeekGrid::ShowInfo(int X, int Y)
{
    int dy = 0;
    int day = 0;
    appt *p_Appt = first_appt;;
    appt *small_appt = NULL;
    tm *tt = 0;
    time_t new_time;
    int overlap = 0;
    int big_overlap = 0;
    int x_off = 0;
    int left = 0;
    float rem = 0;
    int H = 0;
    int offset = 0;
    int width = 0;

    DPRINT("Got X [%d] GOt Y [%d]\n", X, Y);

    dy = GetGridVal();

    Y = dy + Y - y();
    day = ((int) ((X - x()) / b_width));
    x_off = X - (day * b_width) - x();
    Y = (day * (int) day_pix) + Y;
    X = (day * b_width);

    DPRINT("day [%d] x_off [%d]\n", day, x_off);
    DPRINT(" X [%d] Y [%d]\n", X, Y);
    if (x_off < (int) (b_width / 2))
	left = 1;

    DPRINT("left [%d]\n", left);
    while (p_Appt) {		// find the appt with the largest overlap 
	new_time = p_Appt->start_time;
	tt = localtime(&new_time);
	if (tt->tm_wday != day) {
	    p_Appt = p_Appt->next_appt;
	    continue;
	}
	if (tt->tm_wday > day)
	    break;

	DPRINT("p_Appt->y [%.0f] p_Appt->h [%.0f]\n", p_Appt->y, p_Appt->h);
	if ((Y >= p_Appt->y) && (Y < (p_Appt->y + p_Appt->h))) {
	    DPRINT("checking p_Appt\n");
	    if ((p_Appt->overlay & LEFT) || (p_Appt->overlay & RIGHT)) {
		DPRINT("Not Single\n");
		if (left && (LEFT & p_Appt->overlay)) {
		    DPRINT("going LEFT\n");
		    overlap = Overlap(p_Appt);
		    if ((overlap >= big_overlap)) {
			big_overlap = overlap;
			if (NULL == small_appt)
			    small_appt = p_Appt;
			if ((p_Appt->h < small_appt->h)) {
			    small_appt = p_Appt;
			}
		    }
		}
		if (!left && (RIGHT & p_Appt->overlay)) {
		    DPRINT("going RIGHT\n");
		    overlap = Overlap(p_Appt);
		    if ((overlap >= big_overlap)) {
			big_overlap = overlap;
			if (NULL == small_appt)
			    small_appt = p_Appt;
			if ((p_Appt->h < small_appt->h)) {
			    small_appt = p_Appt;
			}
		    }
		}
	    } else {
		overlap = Overlap(p_Appt);
		if ((overlap >= big_overlap)) {
		    big_overlap = overlap;
		    if (NULL == small_appt)
			small_appt = p_Appt;
		    if ((p_Appt->h < small_appt->h)) {
			small_appt = p_Appt;
		    }
		}
	    }
	}
	p_Appt = p_Appt->next_appt;
    }

    if (small_appt) {
	SetInfoBox(small_appt);
	DPRINT("Have small appt\n");
	rem = GetOffSet(small_appt->y);
	Y = (int) rem;

	if (dy > Y) {
	    if ((Y + (int) small_appt->h) > dy) {
		H = (Y + (int) small_appt->h) - dy;
		Y = 0;
	    } else
		H = 0;
	} else {
	    offset = Y;		// off set into dy

	    Y = (int) rem - dy;	// Y value corrected
	    if (offset >= (dy + (int) (6.5 * b_height))) {
		H = 0;
	    } else if ((offset + small_appt->h) <= dy) {
		H = 0;
	    } else if ((offset + small_appt->h) >
		       (dy + (int) (6.5 * b_height))) {
		H = (int) (6.5 * b_height) - Y;
	    } else
		H = (int) small_appt->h;

	    if (H > (int) (6.5 * b_height)) {
		H = (int) (6.5 * b_height);
	    }
	}

	fl_color(FL_BLACK);

	DPRINT("X [%d]\n", X);
	DPRINT("Y [%d] H [%d]\n", Y, H);
	DPRINT("height [%d]\n", (int) (6.5 * b_height));
	if ((Y + H) == (int) (6.5 * b_height)) {
	    int new_X = 0;
	    int new_Y = 0;
	    switch (small_appt->overlay) {
	    case RIGHT:
		width = b_width / 2 - 4;
		x_off = X + width;
		new_X = x_off + x();
		new_Y = Y + y();
		fl_color(FL_BLACK);

		//draw the top
		fl_line(new_X + 1, new_Y - 1, new_X + width + 6, new_Y - 1);
		fl_line(new_X + 1, new_Y - 2, new_X + width + 6, new_Y - 2);
		fl_line(new_X + 1, new_Y - 3, new_X + width + 6, new_Y - 3);

		// draw the sides
		fl_line(new_X + 3, new_Y - 1, new_X + 3, new_Y + H - 1);
		fl_line(new_X + 2, new_Y - 1, new_X + 2, new_Y + H - 1);
		fl_line(new_X + 1, new_Y - 1, new_X + 1, new_Y + H - 1);

		new_X += width;
		fl_line(new_X + 4, new_Y - 1, new_X + 4, new_Y + H - 1);
		fl_line(new_X + 5, new_Y - 1, new_X + 5, new_Y + H - 1);
		fl_line(new_X + 6, new_Y - 1, new_X + 6, new_Y + H - 1);

		break;
	    case LEFT:
		x_off = X;
		width = b_width / 2 - 4;
		new_X = x_off + x();
		new_Y = Y + y();
		fl_color(FL_BLACK);

		//draw the top
		fl_line(new_X + 1, new_Y - 1, new_X + width + 6, new_Y - 1);
		fl_line(new_X + 1, new_Y - 2, new_X + width + 6, new_Y - 2);
		fl_line(new_X + 1, new_Y - 3, new_X + width + 6, new_Y - 3);

		// draw the sides
		fl_line(new_X + 3, new_Y - 1, new_X + 3, new_Y + H - 1);
		fl_line(new_X + 2, new_Y - 1, new_X + 2, new_Y + H - 1);
		fl_line(new_X + 1, new_Y - 1, new_X + 1, new_Y + H - 1);

		new_X += width;
		fl_line(new_X + 4, new_Y - 1, new_X + 4, new_Y + H - 1);
		fl_line(new_X + 5, new_Y - 1, new_X + 5, new_Y + H - 1);
		fl_line(new_X + 6, new_Y - 1, new_X + 6, new_Y + H - 1);
		break;
	    case NO_OVERLAP:
		break;
	    default:
		width = b_width - 7;
		x_off = X;
		new_X = x_off + x();
		new_Y = Y + y();
		fl_color(FL_BLACK);

		//draw the top
		fl_line(new_X + 1, new_Y - 1, new_X + width + 6, new_Y - 1);
		fl_line(new_X + 1, new_Y - 2, new_X + width + 6, new_Y - 2);
		fl_line(new_X + 1, new_Y - 3, new_X + width + 6, new_Y - 3);

		// draw the sides
		fl_line(new_X + 3, new_Y - 1, new_X + 3, new_Y + H - 1);
		fl_line(new_X + 2, new_Y - 1, new_X + 2, new_Y + H - 1);
		fl_line(new_X + 1, new_Y - 1, new_X + 1, new_Y + H - 1);

		new_X += width;
		fl_line(new_X + 4, new_Y - 1, new_X + 4, new_Y + H - 1);
		fl_line(new_X + 5, new_Y - 1, new_X + 5, new_Y + H - 1);
		fl_line(new_X + 6, new_Y - 1, new_X + 6, new_Y + H - 1);
		break;
	    }
	} else {
	    switch (small_appt->overlay) {
	    case RIGHT:
		width = b_width / 2 - 4;
		x_off = X + width;
		fl_color(FL_BLACK);
		fl_rect(x_off + 3 + x(), Y + y() - 1, width + 2, H + 2);
		fl_rect(x_off + 2 + x(), Y + y() - 2, width + 4, H + 4);
		fl_rect(x_off + 1 + x(), Y + y() - 3, width + 6, H + 6);
		break;
	    case LEFT:
		x_off = X;
		width = b_width / 2 - 4;
		fl_color(FL_BLACK);
		fl_rect(x_off + 3 + x(), Y + y() - 1, width + 2, H + 2);
		fl_rect(x_off + 2 + x(), Y + y() - 2, width + 4, H + 4);
		fl_rect(x_off + 1 + x(), Y + y() - 3, width + 6, H + 6);
		break;
	    case NO_OVERLAP:
		break;
	    default:
		width = b_width - 7;
		x_off = X;
		fl_color(FL_BLACK);
		fl_rect(x_off + 3 + x(), Y + y() - 1, width + 2, H + 2);
		fl_rect(x_off + 2 + x(), Y + y() - 2, width + 4, H + 4);
		fl_rect(x_off + 1 + x(), Y + y() - 3, width + 6, H + 6);
		break;
	    }
	}
    }
}

void
WeekGrid::SetInfoBox(appt * p_Appt)
{
    time_t new_time = 0;
    tm *tt = NULL;
    char desc[100];
    char temp_desc[100];
    static char buf[150];
    int width = 0;
    char from_date[30];
    char to_date[10];
    unsigned int idx = 0;
    int dot_width = 0;

    memset(from_date, 0, sizeof(from_date));
    memset(to_date, 0, sizeof(to_date));
    memset(temp_desc, 0, sizeof(temp_desc));

    DPRINT("SetInfoBox\n");
    if (output_box) {
	if (p_Appt) {
	    db->Extract(db_name, p_Appt->recno, 10, desc);
	    DPRINT("desc [%s]\n", desc);
	    fl_font(output_box->labelfont(), output_box->labelsize());
	    width = (int) fl_width(desc);
	    if (width > (output_box->w() - 10)) {
		dot_width = (int) fl_width("...");
		idx = 0;
		while (width > (output_box->w() - 10)) {
		    idx++;
		    memset(temp_desc, 0, sizeof(temp_desc));
		    strncpy(temp_desc, desc, strlen(desc) - idx);
		    width = (int) fl_width(temp_desc) + dot_width;
		}
		sprintf(desc, "%s...", temp_desc);
	    }
	    new_time = p_Appt->start_time;
	    tt = localtime(&new_time);
	    strftime(from_date, 29, "%a %m/%d/%Y", tt);
	    strcat(buf, from_date);
//                      strcat(buf, "    ");
	    strftime(from_date, 29, "%I:%M%P", tt);
	    strcat(buf, from_date);

	    new_time = p_Appt->end_time;
	    tt = localtime(&new_time);
	    strftime(to_date, 29, "%I:%M%P", tt);
	    strcat(buf, "-");
	    strcat(buf, to_date);
	    strcat(buf, "\n");
	    // change \n to spaces
	    for (idx = 0; idx <= strlen(desc); idx++) {
		if ('\n' == desc[idx])
		    desc[idx] = ' ';
	    }
	    strcat(buf, desc);
	    output_box->label(buf);
	} else
	    sprintf(buf, " ");
	output_box->label(buf);
    }
}

void
WeekGrid::SetDateSunday(time_t date)
{
    tm *tt = localtime(&date);

    tt->tm_sec = 0;
    tt->tm_min = 0;
    tt->tm_hour = 0;

    date_sunday = mktime(tt);

}

int
WeekGrid::handle(int event)
{
    switch (event) {
    case FL_PUSH:
	if (Fl::event_inside(this)) {
	    info_draw = 1;
	    redraw();
	    DPRINT("Go push\n");
	    output_box->redraw();
	}
	return 1;
    case FL_RELEASE:
	info_draw = 0;
	damage(FL_DAMAGE_ALL);
	SetInfoBox(NULL);
	output_box->redraw();
	DPRINT("Release\n");
	return 1;
    default:
	return 0;
    }
}

void
GridButton::draw()
{

    Fl_Color col = value()? NxApp::Instance()->getGlobalColor(BUTTON_PUSH) :
	NxApp::Instance()->getGlobalColor(BUTTON_FACE);
#ifndef PDA
    box(FL_BORDER_BOX);
#endif
    if (damage() & FL_DAMAGE_ALL)
	draw_box();
    int X = x();
    int Y = y();
    int W = w();
    int H = h();

    int w1 = (W - 1) | 1;
    int h1 = (H - 1) | 1;
    int W1 = w1 / 3;
    int H1 = h1 / 3;
    int Y1 = Y + H / 3;
    int X1 = X + w1 - w() / 2;

    if (damage()) {
#ifdef PDA
	draw_box(FL_FLAT_BOX, X, Y, W, H, col);
	if (value())
	    fl_color(NxApp::Instance()->getGlobalColor(BUTTON_FACE));
	else
	    fl_color(NxApp::Instance()->getGlobalColor(BUTTON_TEXT));
#else
	draw_box(value()? (down_box()? down_box() : down(box())) : box(),
		 col);
#endif
	if (0 == strcmp(label(), "@UpArrow"))
	    fl_polygon(X1, Y1, X1 - W1, Y1 + H1, X1 + W1, Y1 + H1);
	if (0 == strcmp(label(), "@DownArrow"))
	    fl_polygon(X1, Y1 + H1, X1 - W1, Y1, X1 + W1, Y1);
    }
}
