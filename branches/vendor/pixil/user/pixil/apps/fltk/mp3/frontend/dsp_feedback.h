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



// dsp_feedback.h

#ifndef DSP_FEEDBACK_H
#define DSP_FEEDBACK_H

#include "dsp.h"
#include "fft.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Eq.H>

#include <stdio.h>
#include <iostream.h>

// Server DSP plug-ins definitions
#define FEEDBACK 1

struct feedback_struct
{
    int Hz;
    int SR;
    Fl_Window *parent;
};

class Dsp_Feedback:public Dsp
{

  private:
    feedback_struct * feedback;
    int init;

    static fft_state *state;
    int tmp_out[257];
    int dest[257];
    short int sample[512];
    float scale;

    Fl_Eq *eq0;
    Fl_Eq *eq1;
    Fl_Eq *eq2;
    Fl_Eq *eq3;
    Fl_Eq *eq4;
    Fl_Eq *eq5;
    Fl_Eq *eq6;
    Fl_Eq *eq7;
    Fl_Eq *eq8;
    Fl_Eq *eq9;
    Fl_Eq *eq10;
    Fl_Eq *eq11;
    Fl_Eq *eq12;
    Fl_Eq *eq13;
    Fl_Eq *eq14;


    int dataFrameSkip;

  protected:
    void FeedbackFilter(void *data);
    void Init();

  public:
      Dsp_Feedback();
     ~Dsp_Feedback();
    virtual void *ApplyDsp(void *data, int skip);
    void SetArgs(void *args);

};

extern Dsp_Feedback *feedbackFilter;

#endif // DSP_FEEDBACK_H
