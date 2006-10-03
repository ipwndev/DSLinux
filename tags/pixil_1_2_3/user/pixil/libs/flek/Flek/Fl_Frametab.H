/* Fl_Frametab for WidgetSet, Copyright (c) 1998 curtis Edwards (curt1@jps.net)
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  All work developed as a consequence of the use of
 * this program should duly acknowledge such use. No representations are
 * made about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
   Description	-  Raps widgets in a managable group giving
                hiding and showing of grouped widgets
*/

#ifndef Fl_FTAB_H
#define Fl_FTAB_H

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Group.H>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MINFRAMEHEIGHT      26
#define MINFRAMEPIXHEIGHT   8   //horz mode
#define MINFRAMEPIXWIDTH    36
#define MINFRAMEPIXINDENT   46 //for label
#define MINFRAMEINDENT      12


class Fl_Frametab : public Fl_Group 
{
public:
  enum { // values for type(int)
    TRIANGLE = 0,
    BITMAP   = 1
  };
   int        _hilight;          //is hilighted (FL_ENTER/LEAVE) version 2- Curtis99
   int        _hilight_col;     //storage for hilight color
   int        _openSize;        //size when tab closed version 2- Curtis99
   int        _openLabelHide;   //true/false if label should be hidden when open/expanded
   int        _isopen;          //TRUE/FALSE if frame open/closed

   void  draw();          //main draw
   int   handle(int);     //events,for pushing of frame area
   void  close();
   void  open();
   int   isOpen()                 {return _isopen;       }
   void  hilight_color(int col)   { _hilight_col = col;  }
   int   hilight_color()          { return _hilight_col; }
   void  openSize(int siz)        { _openSize = siz;     }
   int   openSize()               {return _openSize;     }
   void  openLabelHide(int yn)    { _openLabelHide = yn; }
   int   openLabelHide()          {return _openLabelHide;}
   int   openDiff()               {if(!_isopen) return (_openSize-(type() ? MINFRAMEPIXHEIGHT : MINFRAMEHEIGHT))*-1; else return (_openSize-(type() ? MINFRAMEPIXHEIGHT : MINFRAMEHEIGHT));}
   Fl_Frametab(int x,int y,int w,int h,char *);

};
#endif
