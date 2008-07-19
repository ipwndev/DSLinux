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


#include "dsp_feedback.h"
#include <iostream.h>
#include <pthread.h>
#include <string.h>

#include "/usr/include/math.h"

fft_state *
    Dsp_Feedback::state;

Dsp_Feedback::Dsp_Feedback()
{
    //cout << "Dsp_Feedback(): " << (void*)this << endl;
    feedback = new feedback_struct;
    //cout << "new feedback" << (void*)feedback << endl;

    init = 0;
    if (!init) {
	Init();
    }

    state = NULL;
    state = fft_init();
    scale = 100 / log(256);

    dataFrameSkip = 5;

}

Dsp_Feedback::~Dsp_Feedback()
{
    //printf("~Dsp_Feedback()\n");
    //printf("\t- delete feedback\n");
    delete feedback;

}

void
Dsp_Feedback::Init()
{

    ////////////////////////////////////////
    // Spectragraph
    ////////////////////////////////////////

    int EQ_X = 30;
    int EQ_Y = 205;

    eq0 = new Fl_Eq(EQ_X, EQ_Y, 11, 62);
    eq0->type(2);
    eq0->range(0, 100);
    //cout << "allocated eq0 addr: " << (void*)eq0 << endl;

    eq1 = new Fl_Eq(EQ_X + 12, EQ_Y, 11, 62);
    eq1->type(2);
    eq1->range(0, 100);
    //cout << "allocated eq1 addr: " << (void*)eq1 << endl;

    eq2 = new Fl_Eq(EQ_X + 24, EQ_Y, 11, 62);
    eq2->type(2);
    eq2->range(0, 100);
    //cout << "allocated eq2 addr: " << (void*)eq2 << endl;

    eq3 = new Fl_Eq(EQ_X + 36, EQ_Y, 11, 62);
    eq3->type(2);
    eq3->range(0, 100);
    //cout << "allocated eq3 addr: " << (void*)eq3 << endl;

    eq4 = new Fl_Eq(EQ_X + 48, EQ_Y, 11, 62);
    eq4->type(2);
    eq4->range(0, 100);
    //cout << "allocated eq4 addr: " << (void*)eq4 << endl;

    eq5 = new Fl_Eq(EQ_X + 60, EQ_Y, 11, 62);
    eq5->type(2);
    eq5->range(0, 100);
    //cout << "allocated eq5 addr: " << (void*)eq5 << endl;

    eq6 = new Fl_Eq(EQ_X + 72, EQ_Y, 11, 62);
    eq6->type(2);
    eq6->range(0, 100);
    //cout << "allocated eq6 addr: " << (void*)eq6 << endl;

    eq7 = new Fl_Eq(EQ_X + 84, EQ_Y, 11, 62);
    eq7->type(2);
    eq7->range(0, 100);
    //cout << "allocated eq7 addr: " << (void*)eq7 << endl;

    eq8 = new Fl_Eq(EQ_X + 96, EQ_Y, 11, 62);
    eq8->type(2);
    eq8->range(0, 100);
    //cout << "allocated eq8 addr: " << (void*)eq8 << endl;

    eq9 = new Fl_Eq(EQ_X + 108, EQ_Y, 11, 62);
    eq9->type(2);
    eq9->range(0, 100);
    //cout << "allocated eq9 addr: " << (void*)eq9 << endl;

    eq10 = new Fl_Eq(EQ_X + 120, EQ_Y, 11, 62);
    eq10->type(2);
    eq10->range(0, 100);
    //cout << "allocated eq10 addr: " << (void*)eq10 << endl;

    eq11 = new Fl_Eq(EQ_X + 132, EQ_Y, 11, 62);
    eq11->type(2);
    eq11->range(0, 100);
    //cout << "allocated eq11 addr: " << (void*)eq11 << endl;

    eq12 = new Fl_Eq(EQ_X + 144, EQ_Y, 11, 62);
    eq12->type(2);
    eq12->range(0, 100);
    //cout << "allocated eq12 addr: " << (void*)eq12 << endl;

    eq13 = new Fl_Eq(EQ_X + 156, EQ_Y, 11, 62);
    eq13->type(2);
    eq13->range(0, 100);
    //cout << "allocated eq13 addr: " << (void*)eq13 << endl;

    eq14 = new Fl_Eq(EQ_X + 168, EQ_Y, 11, 62);
    eq14->type(2);
    eq14->range(0, 100);
    //cout << "allocated eq14 addr: " << (void*)eq14 << endl;

    init = 1;

}

void *
Dsp_Feedback::ApplyDsp(void *data, int skip)
{

    dataFrameSkip = skip;

    if (data != 0)
	FeedbackFilter(data);

    return (void *) buffer;
}

void
Dsp_Feedback::SetArgs(void *args)
{
    feedback_struct *tmp = (feedback_struct *) args;
    feedback->Hz = tmp->Hz;
    feedback->SR = tmp->SR;
    feedback->parent = tmp->parent;
    tmp = 0;
}


void
Dsp_Feedback::FeedbackFilter(void *data)
{


    buffer = (short int *) data;

    /*
       float F, Pi;
       int L, Hz, SR;

       Hz = feedback->Hz;
       SR = feedback->SR;
       L = RAWDATASIZE;
       Pi = 3.1415;

       F = (2 * Pi * Hz) / SR;

       for ( int T = 0; T < RAWDATASIZE
       buffer[T] = int(buffer[T]*sin(F*T));
     */


    // static fft_state *state = NULL;
    //float tmp_out[257];
    //int dest[257];
    int i;
    //short int sample[512];
    //float scale = 100/log(256);

    for (i = 0; i < RAWDATASIZE; i += 512) {

	memcpy((short int *) &sample[0], (short int *) &buffer[i], 512);

	//if(!state)
	//state = fft_init();

	fft_perform(sample, tmp_out, state, dataFrameSkip);

	for (int j = 0; j < 256; j++) {
	    dest[j] = ((int) sqrt(tmp_out[j + 1]) >> 8);
	}

	int k, c;
	int y;
	int xscale[] =
	    { 0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 50, 101, 202, 237, 248, 255 };
	//int xscale[] = {0, 1, 2, 3, 5, 7};


	for (k = 0; k < 15; k++) {

	    for (c = xscale[k], y = 0; c < xscale[k + 1]; c++) {
		if (dest[c] > y)
		    y = dest[c];
	    }

	    if (y != 0) {

		y = (int) (log(y) * scale);

		if (y > 100)
		    y = 100;
	    }			// if

	    if (y >= 0) {

		switch (c) {
		case 1:
		    eq0->value(y);
		    eq0->redraw();
		    break;
		case 2:
		    eq1->value(y);
		    eq1->redraw();
		    break;
		case 3:
		    eq2->value(y);
		    eq2->redraw();
		    break;
		case 5:
		    eq3->value(y);
		    eq3->redraw();
		    break;
		case 7:
		    eq4->value(y);
		    eq4->redraw();
		    break;
		case 10:
		    eq5->value(y);
		    eq5->redraw();
		    break;
		case 14:
		    eq6->value(y);
		    eq6->redraw();
		    break;
		case 20:
		    eq7->value(y);
		    eq7->redraw();
		    break;
		case 28:
		    eq8->value(y);
		    eq8->redraw();
		    break;
		case 50:
		    eq9->value(y);
		    eq9->redraw();
		    break;
		case 101:
		    eq10->value(y);
		    eq10->redraw();
		    break;
		case 202:
		    eq11->value(y);
		    eq11->redraw();
		    break;
		case 237:
		    eq12->value(y);
		    eq12->redraw();
		    break;
		case 248:
		    eq13->value(y);
		    eq13->redraw();
		    break;
		case 255:
		    eq14->value(y);
		    eq14->redraw();
		    break;
		}


	    }

	}			// for 16

	//cout << endl;

    }				// for 512
}
