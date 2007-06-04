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


//
// Implementation of base text editor class for FLTK.
//

#include <FL/Fl_Editor.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "nxapp.h"
#include "nxscrollbar.h"

// ugly workaround for drawing bug with FLTK 2.0 CVS                                                                                                    
#if (FL_MAJOR_VERSION > 1)
#define FULL_REDRAW_ALWAYS 1
#endif

// **************************************************
//                                              Callbacks
// **************************************************

static void
dragtimer(void *o)
{
    Fl_Editor *ed = (Fl_Editor *) o;
    if (ed->TimerEnabled())
	ed->handle(FL_DRAG);
}

static void
cb_EditScroller(NxScrollbar * o, void *udata)
{
    Fl_Editor *ed = (Fl_Editor *) udata;
    if (ed)
	ed->HandleScroll();
}

// **************************************************
//                                              Engine functions
// **************************************************

FL_API short
Fl_EditorEngine::MeasureTextWidth(const char *t)
{
#if (FL_MAJOR_VERSION > 1)
    Fl_Font cfont = fl_font();
    unsigned csize = fl_size();
#else
    uchar cfont = fl_font();
    uchar csize = fl_size();
#endif
    short lwide = 0;
    {
	fl_font(this->textfont(), this->textsize());
	lwide = (short) fl_width(t);
	fl_font(cfont, csize);
    }
    return (lwide);
}

FL_API short
Fl_EditorEngine::MeasureTextHeight(const char *t)
{
    int maxheight = this->textsize() + fl_descent();
#if (FL_MAJOR_VERSION > 1)
    Fl_Font cfont = fl_font();
    unsigned csize = fl_size();
#else
    uchar cfont = fl_font();
    uchar csize = fl_size();
#endif
    {
	fl_font(this->textfont(), this->textsize());
	maxheight = this->textsize() + fl_descent();
	fl_font(cfont, csize);
    }
    return (maxheight);
}


// **************************************************
//                                              Widget functions
// **************************************************

FL_API Fl_Editor::Fl_Editor(int x, int y, int w, int h, const char *cap)
:
Fl_Group(x, y, w, h, cap)
{
    save_h = h;
    move = true;
    int
	bdx,
	bdy,
	bdh,
	bdw,
	ebdx,
	ebdy;
    myHideCursor = false;
    clear_output();
    StartLine = 0;
    overstrike = 0;
    VisibleLines = (h > 5 ? (h - 6) / 12 : 0);
    marking = 0;
    selectlines = false;
    DrawState = NULL;
    engine = NULL;
    Readonly = Browsemode = false;
    _scrollspeed = 5;		// default to 5 lines / sec.
    box(FL_DOWN_BOX);		// default to down box

    color(NxApp::Instance()->getGlobalColor(EDITOR_BG));	// default to white background
    textcolor(NxApp::Instance()->getGlobalColor(EDITOR_FG));
    selection_color(NxApp::Instance()->getGlobalColor(EDITOR_SEL));
#if (FL_MAJOR_VERSION > 1)
    textfont(FL_TIMES);
#else
    textfont(FL_TIMES);		// default to 12-point Times font
#endif
    textsize(12);
    setfont();
    this->begin();

#if (FL_MAJOR_VERSION > 1)
    bdx = box()->dx();
    bdy = box()->dy();
    bdw = box()->dw();
    bdh = box()->dh();
#else
    bdx = Fl::box_dx(box());
    bdy = Fl::box_dy(box());
    bdw = Fl::box_dw(box());
    bdh = Fl::box_dh(box());
#endif
    editbox =
	new Fl_Box(x + bdx, y + bdy, w - (SCROLLERWIDTH + 2 * bdw),
		   h - (2 * bdh));
    editbox->box(FL_NO_BOX);
    editbox->hide();
#if (FL_MAJOR_VERSION > 1)
    ebdx = editbox->box()->dx();
    ebdy = editbox->box()->dy();
#else
    ebdx = Fl::box_dx(editbox->box());
    ebdy = Fl::box_dy(editbox->box());
#endif
    //this->resizable(editbox);

    EditWidth = editbox->w() - LEFTMARGIN - SCROLLERWIDTH;
    engine = new Fl_EditorEngine(EditWidth);
    engine->textfont(textfont());
    engine->textsize(textsize());

    // add the scroller 
    scroller =
	new NxScrollbar((x + w) - (SCROLLERWIDTH + bdx), y + bdy,
			SCROLLERWIDTH, (h - bdy));
    scroller->callback((Fl_Callback *) cb_EditScroller);
    scroller->user_data(this);
    scroller->value(StartLine, 10, 0, engine->GetLineCount() * 10);
    //      scroller->color(FL_GRAY);
    scroller->hide();


    __Cursor =
	new Fl_CursorBox(editbox->x() + LEFTMARGIN,
			 editbox->y() + TOPMARGIN + fl_descent(), 2,
			 textsize(), "");
    if (__Cursor) {
	__Cursor->box(FL_FLAT_BOX);
	__Cursor->color(FL_BLACK);
	__Cursor->hide();
    }
    this->end();

    fleditor_DrawState
	ds(textcolor(), color(), selection_color(), textfont(), textsize());
    PushState(ds);
}

FL_API Fl_Editor::~Fl_Editor()
{
    hide();
    //      if(__Cursor)
    //              delete __Cursor;
    if (scroller)
	delete
	    scroller;
    if (engine)
	delete
	    engine;
    struct fleditor_DrawState *
	ds =
	DrawState;
    while (ds) {
	ds = DrawState->next;
	delete
	    DrawState;
	DrawState = ds;
    }
}

FL_API void
Fl_Editor::setfont() const
{
//      if ( (fl_font() != this->textfont()) || (fl_size() != this->textsize()) )
    fl_font(this->textfont(), this->textsize());
}

FL_API short
Fl_Editor::CursorX()
{
    // TODO: Update this for variable width characters in a line.  Should probably use MeasureTextWidth anyway
    short left;
    wString curline = engine->GetLine(engine->CursorRow());
    curline.ChopAt(engine->CursorColumn());
    left = editbox->x() + LEFTMARGIN + (short) fl_width(curline.Get());
    return (left);
}

FL_API short
Fl_Editor::CursorY()
{
    long top;
// TODO: Update this for lines with variable height
    top =
	editbox->y() + TOPMARGIN +
	((engine->MeasureTextHeight("X")) *
	 (engine->CursorRow() - StartLine));
    return ((short) top);
}

FL_API void
Fl_Editor::HandleScroll()
{
    long oldstart = StartLine;
    long newstart = (long) scroller->value() / 10;
    if (newstart > engine->GetLineCount())
	newstart = engine->GetLineCount();

    engine->Command(MOVE_ABSOLUTE_ROW, newstart);
    StartLine = newstart;
    if (oldstart != newstart) {
	this->window()->make_current();
	draw_clip(this, editbox->x(), editbox->y(), editbox->w(),
		  editbox->h());
	// Added to redraw on scrollbar drags....10/30/01 JW
	damage(FL_DAMAGE_ALL);
    }
}

FL_API void
Fl_Editor::ShowCursor(CursorStyles show)
{
    this->setfont();
    switch (show) {
    case CURSOR_HIDE:
	__Cursor->hide();
	break;
    case CURSOR_UNHIDE:
	if (Fl::focus() == this)
	    __Cursor->show();
	if (overstrike)
	    ShowCursor(CURSOR_OVERSTRIKE);
	else
	    ShowCursor(CURSOR_NORMAL);
	break;
    case CURSOR_NORMAL:
	__Cursor->setsize(CursorX(), CursorY() + fl_descent(), 2, textsize());
	break;
    case CURSOR_OVERSTRIKE:
	__Cursor->setsize(CursorX(), CursorY() + fl_descent(), 6, textsize());
	break;
    }

    // now update the scrollbar
    long SafeTotal = engine->GetLineCount()? engine->GetLineCount() : 1;
    if (SafeTotal <= VisibleLines) {
	SafeTotal = VisibleLines;
	if (scroller->visible())
	    scroller->hide();
    } else if (!scroller->visible())
	scroller->show();
    long SafeStart = StartLine;
    long SafeSize = VisibleLines;
    if ((SafeStart + SafeSize) > SafeTotal)
	SafeStart = 0;
    if (SafeStart < 0)
	SafeStart = 0;
    if ((scroller->value() / 10) == SafeStart)
	SafeStart = scroller->value();
    else
	SafeStart = SafeStart * 10;
#if 0
    long curscroll = scroller->value();
#endif

    scroller->value(SafeStart, SafeSize * 10, 0, SafeTotal * 10);
}

#define FULL_REDRAW_ALWAYS 1

FL_API void
Fl_Editor::draw()
{
    //  printf("Fl_Editor::draw()\n");
    ShowCursor(CURSOR_HIDE);

#if 0
    //printf("FULL_REDRAW_ALWAYS\n");
    // total redraw
    // draw base widget outline and label, if any

#if (FL_MAJOR_VERSION > 1)
    Fl_Group::draw();
    //              draw_box(x(), y(), w(), h(), flags());
    //              draw_label(x(), y(), w(), h(), flags());
#else
    draw_box();
    draw_label();
#endif

    // draw text on background
    int X, Y, W, H;
    setfont();
    fl_clip_box(editbox->x(), editbox->y(), editbox->w(), editbox->h(), X, Y,
		W, H);
    draw_clip(this, X, Y, W, H);
    // now draw all the children atop the background:

#if (FL_MAJOR_VERSION > 1)
    //              for (int i = children(); i > 0; i--)
    //                      draw_child(*child(i));
#else
    Fl_Widget *const *a = array();
    for (int i = children(); i--; a++)
	draw_child(**a);
#endif

#else
    //      printf("Use DAMAGE masks\n");
    if (damage() & FL_DAMAGE_ALL) {
	//  printf("FL_DAMAGE_ALL\n");
	// total redraw
	// draw base widget outline and label, if any

#if (FL_MAJOR_VERSION > 1)
	Fl_Group::draw();
	//          draw_box(x(), y(), w(), h(), flags());
	//          draw_label(x(), y(), w(), h(), flags());
#else
	draw_box();
	draw_label();
#endif

	// draw text on background
	int X, Y, W, H;
	setfont();
	fl_clip_box(editbox->x(), editbox->y(), editbox->w(), editbox->h(), X,
		    Y, W, H);
	draw_clip(this, X, Y, W, H);
	// now draw all the children atop the background:

#if (FL_MAJOR_VERSION > 1)
	//          for (int i = children(); i > 0; i--)
	//                  draw_child(*child(i));
#else
	Fl_Widget *const *a = array();
	for (int i = children(); i--; a++)
	    draw_child(**a);
#endif

    } else {
	if (damage() & DAMAGE_EDITOR) {
	    //              printf("DAMAGE_EDITOR\n");
	    //  redraw editing area only.
	    int X, Y, W, H;
	    setfont();
	    fl_clip_box(editbox->x(), editbox->y(), editbox->w(),
			editbox->h(), X, Y, W, H);
	    draw_clip(this, X, Y, W, H);
	    // update scroll bar 
	    draw_child(*scroller);
	} else if ((damage() & DAMAGE_LINE) || (damage() & DAMAGE_PARTIAL)) {
	    //              printf("DAMAGE_LINE\n");
	    EditPosition curpos = engine->GetPosition();
	    EditPosition originalpos = DrawFrom;
	    SortPositions(DrawFrom, curpos);	// put positions in right order
	    // For each line visible in editor, check to see if it's in the range.  
	    // If it is, draw it using DrawLine

	    setfont();
	    int drawy = engine->MeasureTextHeight("X") + TOPMARGIN;
	    int drawx = editbox->x() + LEFTMARGIN;
	    for (long curline = StartLine; curline < StartLine + VisibleLines;
		 curline++) {
		wString current = engine->GetLine(curline);
		if (curline >= DrawFrom.Row && curline <= curpos.Row)
		    DrawLine(curline, drawx, editbox->y() + drawy, current,
			     *DrawState);
		drawy += engine->MeasureTextHeight(current);
	    }
	    DrawFrom = originalpos;
	}
	if (damage() & FL_DAMAGE_CHILD) {
	    //              printf("FL_DAMAGE_CHILD\n");
#if (FL_MAJOR_VERSION > 1)
	    //            for (int i = children(); i > 0; i--)
	    //                draw_child(*child(i));
#else
	    Fl_Widget *const *a = array();
	    for (int i = children(); i--; a++)
		draw_child(**a);
#endif
	}
    }
#endif // FULL_REDRAW_ALWAYS
    //      printf("Endof draw()\n");
    ShowCursor(CURSOR_UNHIDE);
    draw_child(*__Cursor);
}


FL_API void
Fl_Editor::draw_clip(void *v, int X, int Y, int W, int H)
{
    Fl_Editor *S = (Fl_Editor *) v;

    // now figure out the text area dimensions, 
    // leaving some room for the bounding box.
    short left, top, wide, high, lineheight;
    left = S->editbox->x() + LEFTMARGIN;
    top = S->editbox->y() + TOPMARGIN;
    wide = S->editbox->w() - LEFTMARGIN;
    high = S->editbox->h() - TOPMARGIN;
    lineheight = S->engine->MeasureTextHeight("X");
    S->VisibleLines = high / lineheight;

    S->setfont();

    // now print the lines from the editor to the window.
    short line;
    long editorline = S->StartLine;

    short linestart = 0;
    while ((top + linestart + (2 * lineheight) < Y)
	   && (linestart + lineheight < high))
	linestart += lineheight;
    if (linestart > 2 * lineheight)
	linestart -= 2 * lineheight;

    wString current;
    for (line = lineheight + linestart; line <= high; line += lineheight) {
	current = S->engine->GetLine(editorline + ((linestart) / lineheight));
	S->DrawLine(editorline++, left, top + line, current, *(S->DrawState));
    }
}



FL_API long
Fl_Editor::FindColumn(short screenX)
{
#if (FL_MAJOR_VERSION > 1)
    int ebdx = editbox->box()->dx();
#else
    int ebdx = Fl::box_dx(editbox->box());
#endif
    setfont();
    screenX -= (editbox->x() + LEFTMARGIN + ebdx);
    wString curline = engine->GetLine(engine->CursorRow());
    long max = curline.Length();
    char *lineptr = (char *) curline.Get();
    long col = 0;
    for (; col < max; col++) {
	char holder = lineptr[col];
	lineptr[col] = '\0';
	if (fl_width(lineptr) == screenX)
	    break;		// if exactly between characters, stop here
	else if (fl_width(lineptr) > screenX) {
	    if (col)		// else stop before character under the mouse
		col--;
	    break;
	}
	lineptr[col] = holder;
    }
    if (col > max)
	col = max;
    engine->Command(MOVE_ABSOLUTE_COLUMN, col);
    return (col);
}

FL_API long
Fl_Editor::FindRow(short screenY)
{
#if (FL_MAJOR_VERSION > 1)
    int ebdy = editbox->box()->dy();
#else
    int ebdy = Fl::box_dy(editbox->box());
#endif
    setfont();
    long target = -1;
    screenY -= (editbox->y() + TOPMARGIN + ebdy);
    //TODO: Change this for variable height lines
    short lineheight = engine->MeasureTextHeight("X");
    long row;
    for (row = 0; row * lineheight < screenY; row++);	// don't need to do anything but loop
    if (row)
	row--;
    if (StartLine + row <= engine->GetLineCount())
	target = StartLine + row;
    else
	row = engine->GetLineCount() - StartLine;
    if ((row >= 0) && (row < VisibleLines)) {
	engine->Command(MOVE_ABSOLUTE_ROW, StartLine + row);
    }
    return (target);
}

FL_API int
Fl_Editor::handle(int event)
{
    int handled = 0;
    long curstart = StartLine;

    TimerEnable = false;
    DrawFrom = engine->GetPosition();	// save current position before action is performed, optimize redraw
    switch (event) {
    case FL_FOCUS:
	if (myHideCursor)
	    HideCursor(myHideCursor = false);
	ShowCursor(CURSOR_UNHIDE);
	setfont();
	handled = 1;
	break;
    case FL_UNFOCUS:
#ifndef WIN32
	if (marking)
	    CopySelection(false);	// set selection() so normal X mark-then-paste works.
#endif
	if (!HideCursor()) {
	    //HideCursor(myHideCursor = true);
	    ShowCursor(CURSOR_HIDE);
	    damage(DAMAGE_PARTIAL);
	}
	handled = 1;
	break;
#if (FL_MAJOR_VERSION > 1)
    case FL_KEY:
#else
    case FL_KEYBOARD:
#endif
	selectlines = false;
	if (!keeptabs && Fl::event_key() == FL_Tab)
	    handled = Fl_Group::handle(event);
	else
	    handled =
		HandleKey(Fl::event_key(), Fl::event_text(),
			  Fl::event_length(), Fl::event_state());
	break;
    case FL_PUSH:
	do_callback();
	if ((Fl::event_x() > scroller->x()) && (Fl::event_y() > scroller->y())
	    && (Fl::event_x() < (scroller->x() + SCROLLERWIDTH))
	    && (Fl::event_y() < (scroller->y() + scroller->h()))) {
	    break;
	}

	selectlines = false;
	if (Fl::event_button() == 2)	// middle button paste
	{
#ifndef WIN32
	    if (marking) {
		engine->Command(CLEAR_MARKS);
		selectlines = 0;
		marking = 0;
	    }
	    FindCursor(Fl::event_x(), Fl::event_y());
	    if (!Readonly) {
		Fl::paste(*this);
		//if (Fl::focus()==this)  handled = 1;  //remove for Motif behavior
		damage(DAMAGE_EDITOR);
	    }
#endif
	} else if ((Fl::event_button() == 1) && Fl::event_is_click()) {
	    if (Fl::focus() != this)
		Fl::focus(this);
	    if (marking) {
		engine->Command(CLEAR_MARKS);
		selectlines = (marking = false);
		damage(DAMAGE_EDITOR);
	    }
	    switch (Fl::event_clicks() % 3)	// clicking four, five or six times is the same as one, two, or three...
	    {
	    case 2:		// triple click
		handled = 1;
		selectlines = true;
		engine->MoveTo(engine->CursorRow(), 0);
		marking = 1;
		Mark.Column = 0;
		Mark.Row = engine->CursorRow();
		engine->Command(MARK_BEGIN);
		engine->Command(MOVE_DOWN);
		if (engine->CursorRow() == Mark.Row)
		    engine->Command(MOVE_EOL);
		engine->Command(MARK_END);
		damage(DAMAGE_PARTIAL);
		break;
	    case 1:		// double click
		handled = 1;
		marking = 1;
		if (!isspace
		    (engine->
		     GetCharacterAt(engine->CursorRow(),
				    engine->CursorColumn()))) {
		    if (engine->CursorColumn()
			&& !isspace(engine->
				    GetCharacterAt(engine->CursorRow(),
						   engine->CursorColumn() -
						   1)))
			engine->Command(MOVE_LEFT_WORD);
		    Mark.Row = engine->CursorRow();
		    Mark.Column = engine->CursorColumn();
		    engine->Command(MARK_BEGIN);
		    engine->Command(MOVE_RIGHT_WORD);
		    while (engine->CursorColumn()
			   && isspace(engine->
				      GetCharacterAt(engine->CursorRow(),
						     engine->CursorColumn() -
						     1)))
			engine->Command(MOVE_LEFT);
		    engine->Command(MARK_END);
		}
		damage(DAMAGE_PARTIAL);
		break;
	    case 0:		// single click
		handled = 1;
		//ShowCursor(CURSOR_HIDE);                                              
		FindCursor(Fl::event_x(), Fl::event_y());
		if (marking) {
		    marking = selectlines = 0;
		    engine->Command(CLEAR_MARKS);
		}
		if (Browsemode)
		    engine->Command(MOVE_SOL);
		damage(DAMAGE_PARTIAL);	// force repaint of cursor
		//ShowCursor(CURSOR_UNHIDE);
		break;
	    }
	}
	break;
    case FL_DRAG:
	if (Fl::event_state() & FL_BUTTON1) {
	    if (!marking) {
		marking = 1;
		engine->Command(CLEAR_MARKS);
		engine->Command(MARK_BEGIN, 1);
		Mark.Row = engine->CursorRow();
		Mark.Column = engine->CursorColumn();
	    }
	    DrawFrom = engine->GetPosition();	// try to optimize the draw a little...
//                              ShowCursor(CURSOR_HIDE);
	    FindCursor(Fl::event_x(), Fl::event_y());
	    if (marking && selectlines)
		if (Mark.Row > engine->CursorRow()) {	// mark and drag upward
		    if (!Mark.Column) {
			EditPosition pos = engine->GetPosition();
			Mark.Column = engine->Line(Mark.Row)->Length() - 1;
			engine->MoveTo(Mark.Row, Mark.Column);
			engine->Command(MARK_BEGIN, 1);
			engine->MoveTo(pos.Row, pos.Column);
		    }
		    engine->Command(MOVE_SOL);
		} else {
		    if (Mark.Column) {
			EditPosition pos = engine->GetPosition();
			Mark.Column = 0;
			engine->MoveTo(Mark.Row, Mark.Column);
			engine->Command(MARK_BEGIN, 1);
			engine->MoveTo(pos.Row, pos.Column);
		    }
		    engine->Command(MOVE_EOL);
		}
	    if (marking) {
		if ((Fl::event_y() > y() + h())
		    && (StartLine + VisibleLines < engine->GetLineCount())) {
		    StartLine++;
		    engine->Command(MOVE_DOWN);
		    TimerEnable = true;
		    Fl::add_timeout(1.0 / (float) _scrollspeed, dragtimer,
				    (void *) this);
		    damage(DAMAGE_EDITOR);
		} else if ((Fl::event_y() < y()) && StartLine) {
		    StartLine--;
		    engine->Command(MOVE_UP);
		    TimerEnable = true;
		    Fl::add_timeout(1.0 / (float) _scrollspeed, dragtimer,
				    (void *) this);
		    damage(DAMAGE_EDITOR);
		} else
		    TimerEnable = false;
		engine->Command(MARK_END);
	    }
//                              ShowCursor(CURSOR_UNHIDE);
	    damage(DAMAGE_PARTIAL);
	    handled = 1;
	}
	break;
    case FL_RELEASE:
	damage(DAMAGE_PARTIAL);
	handled = 1;
#ifndef WIN32
	CopySelection(false);	// set selection() so normal X mark-then-paste works.
#endif
	selectlines = false;
	break;
    case FL_PASTE:
	if (marking) {
	    engine->Command(DELETE_REGION);
	    engine->MoveTo(Mark.Row, Mark.Column);
	}
	marking = 0;
	selectlines = false;
	// insert from the paste buffer at the cursor
	if (!Readonly) {
	    engine->LoadFrom(Fl::event_text());
	    // and move the cursor to the end of what was just inserted
	    engine->Command(MOVE_RIGHT, Fl::event_length());
	}
	damage(DAMAGE_EDITOR);
	handled = 1;
	break;
    }
    if (!handled)
	handled = Fl_Group::handle(event);
    if (StartLine != curstart)	// did we scroll during this event?
	damage(DAMAGE_EDITOR);
    return (handled);
}

FL_API int
Fl_Editor::HandleKey(int key, const char *text, int len, int state)
{
    //    ShowCursor(CURSOR_HIDE);
    long starting_row = engine->CursorRow();
    long starting_count = engine->GetLineCount();
    bool iscursorkey = false;
    int handled = 0;

    if (key < 256) {

	// standard ASCII key.  
	if (state & FL_CTRL)
	    switch (key) {
	    case 'x':
		if (Readonly)
		    Copy();
		else
		    Cut();
		handled = 1;
		break;
	    case 'c':
		Copy();
		handled = 1;
		break;
	    case 'v':
		if (!Readonly)
		    Paste();
		handled = 1;
		break;
	    }

	if (handled)
	    damage(DAMAGE_EDITOR);

	if ((!handled) && !(state & (FL_CTRL | FL_ALT))) {

	    if (marking) {
		marking = selectlines = 0;

		if (!Readonly)
		    engine->Command(DELETE_REGION);

		engine->Command(CLEAR_MARKS);
		engine->MoveTo(Mark.Row, Mark.Column);
		damage(DAMAGE_EDITOR);
	    }

	    if (!Readonly) {

		if (overstrike && !Readonly) {
		    for (int count = 0; count < len; count++) {
			engine->Command(DELETE_CHARACTER);
		    }
		}

		if (text[1])	// if more than one character, insert as string
		{
		    //              printf("Insert as string.\n");
		    engine->Command(INSERT_STRING_MOVE, text);
		} else {
		    //              printf("Insert as character.\n");
		    engine->InsertCharacter(text[0]);
		}

	    }

	    damage(DAMAGE_LINE);

	    handled = 1;

	}

    } else if (key >= FL_KP && key <= FL_KP_Last) {

	// numeric keypad key
	key -= FL_KP;

	if (!(state & FL_NUM_LOCK)) {
	    // translate to cursor key and process
	    switch (key) {
	    case '1':
		key = FL_End;
		break;
	    case '2':
		key = FL_Down;
		break;
	    case '3':
		key = FL_Page_Down;
		break;
	    case '4':
		key = FL_Left;
		break;
	    case '6':
		key = FL_Right;
		break;
	    case '7':
		key = FL_Home;
		break;
	    case '8':
		key = FL_Up;
		break;
	    case '9':
		key = FL_Page_Up;
		break;
	    }

	    handled = HandleKey(key, text, len, state);

	} else if (!Readonly) {

	    if (overstrike)

		for (int count = 0; count < len; count++)
		    engine->Command(DELETE_CHARACTER);

	    engine->Command(INSERT_STRING, text);
	    engine->Command(MOVE_RIGHT, len);
	    damage(DAMAGE_LINE);
	    handled = 1;

	}

    } else {

	// other key
	handled = 1;
	int unmark = 0;

	switch (key) {
	case FL_Enter:
	case FL_KP_Enter:
	    unmark = marking;

	    if (!Readonly)
		engine->InsertCharacter(HARD_RETURN);

	    damage(DAMAGE_PARTIAL);
	    break;
	case FL_BackSpace:

	    if (!Readonly)
		if (marking) {
		    engine->DeleteRegion();
		    engine->MoveTo(Mark.Row, Mark.Column);
		    damage(DAMAGE_EDITOR);
		} else {
		    if (engine->CursorColumn() == 0) {
			// move up one line and to the end of that line
			engine->MoveTo(engine->CursorRow() - 1,
				       engine->GetLine(engine->CursorRow() -
						       1).Length());
			if (engine->GetLine(engine->CursorRow() + 1).
			    Length() <= 1)
			    if ((engine->GetLine(engine->CursorRow() + 1).
				 Get())[0] == '\n')
				engine->RemoveLine(engine->CursorRow() + 1);
		    } else {
			engine->BackSpaceCharacter();
			damage(DAMAGE_LINE);
		    }
		}

	    break;
	case FL_Delete:

	    if (!Readonly)
		if (state & FL_SHIFT) {
		    Cut();
		    damage(DAMAGE_EDITOR);
		} else if (marking) {
		    engine->DeleteRegion();
		    engine->MoveTo(Mark.Row, Mark.Column);
		    damage(DAMAGE_EDITOR);
		} else {
		    engine->DeleteCharacter();
		    damage(DAMAGE_LINE);
		}

	    unmark = marking;
	    break;
	case FL_Tab:
	    if (keeptabs) {
		unmark = marking;
		engine->Command(CLEAR_MARKS);

		if (!Readonly)
		    engine->InsertCharacter('\t');

		damage(DAMAGE_LINE);
	    }
	    break;
	case FL_Left:
	case FL_Right:
	case FL_Up:
	case FL_Down:
	case FL_Home:
	case FL_End:
	case FL_Page_Up:
	case FL_Page_Down:
	    iscursorkey = true;
	    handled = HandleCursorKey(key, text, len, state);
	    break;
	case FL_Insert:

	    if (state & FL_CTRL) {
		Copy();
		damage(DAMAGE_EDITOR);
	    } else if (state & FL_SHIFT) {

		if (!Readonly)
		    Paste();

		damage(DAMAGE_EDITOR);
	    } else
		overstrike = !overstrike;
	    break;
	case FL_Escape:
	    break;
	default:
	    handled = 0;
	    break;
	}

	if (unmark) {
	    marking = selectlines = 0;
	    engine->Command(CLEAR_MARKS);
	}

    }

    if (engine->CursorRow() < StartLine) {
	StartLine = engine->CursorRow();
	redraw();
    }

    if (engine->CursorRow() >= StartLine + VisibleLines) {
	StartLine = engine->CursorRow() - (VisibleLines - 1);
	redraw();
    }
    //    ShowCursor(CURSOR_UNHIDE);
    // check for a few special cases that really need a partial draw
    long ending_row = engine->CursorRow();
    long ending_count = engine->GetLineCount();

    if (!iscursorkey)
	if ((starting_row != ending_row) || (starting_count != ending_count)
	    || engine->wrapped()) {
	    damage(DAMAGE_PARTIAL);
	    redraw();
	}

    return (handled);

}

FL_API void
Fl_Editor::resize(int nx, int ny, int nw, int nh)
{

    if (nh == save_h)
	nh = save_h;
    else
	nh = nh - (this->y() - ny) - 5;

#if (FL_MAJOR_VERSION > 1)
    int bdw = box()->dw();
    int bdh = box()->dh();
    int bdx = box()->dx();
    int bdy = box()->dy();
#else
    int bdw = Fl::box_dw(box());
    int bdh = Fl::box_dh(box());
    int bdx = Fl::box_dx(box());
    int bdy = Fl::box_dy(box());
#endif
    ShowCursor(CURSOR_HIDE);
    if (move)
	Fl_Group::resize(nx, ny, nw, nh);
    else
	Fl_Group::resize(this->x(), this->y(), nw, nh);

    editbox->size(nw - (SCROLLERWIDTH + 2 * bdw), nh - (2 * bdh));

    __Cursor->setsize(__Cursor->x(), __Cursor->y(), __Cursor->w(),
		      textsize());
    int lineheight = 12;
    if (Fl::focus() != this) {
#if (FL_MAJOR_VERSION > 1)
	Fl_Font cfont = fl_font();
	int csize = fl_size();
	setfont();
	lineheight = engine->MeasureTextHeight("X");
	fl_font(cfont, csize);
#else
	uchar cfont = fl_font();
	uchar csize = fl_size();
	setfont();
	lineheight = engine->MeasureTextHeight("X");
	fl_font(cfont, csize);
#endif
    } else
	lineheight = engine->MeasureTextHeight("X");
    if (!lineheight)
	lineheight = 12;	// wild guess at line height if can't figure it out.
    VisibleLines = editbox->h() / lineheight;

    if (move)
	scroller->resize((nx + nw) - (scroller->w() + bdx), ny + bdy,
			 SCROLLERWIDTH, h() - (2 * bdy));
    else
	scroller->resize((this->x() + nw) - (scroller->w() + bdx),
			 this->y() + bdy, SCROLLERWIDTH, h() - (2 * bdy));

    engine->ChangeWidth(EditWidth = editbox->w() - LEFTMARGIN);
    damage(FL_DAMAGE_ALL);
    redraw();
    ShowCursor(CURSOR_UNHIDE);
}

FL_API int
Fl_Editor::HandleCursorKey(int key, const char *text, int len, int state)
{
    int handled = 0;
    int ismarking = 0;

    if ((state & FL_SHIFT) && !Browsemode)
	ismarking = 1;
    else
	marking = ismarking = 0;
    if (ismarking && !marking) {
	engine->Command(CLEAR_MARKS);
	Mark.Row = engine->CursorRow();
	Mark.Column = engine->CursorColumn();
	marking = 1;
	engine->Command(MARK_BEGIN, 1);
    }
    if (Browsemode)
	switch (key) {
	case FL_Up:
	    if (StartLine) {
		StartLine--;
		engine->MoveTo(StartLine, 0);
		SyncDisplay();
		handled = 1;
	    }
	    break;
	case FL_Down:
	    if (engine->GetLineCount() > StartLine + VisibleLines) {
		StartLine++;
		engine->MoveTo(StartLine, 0);
		SyncDisplay();
		handled = 1;
	    }
	    break;
	case FL_Home:
	case FL_End:
	case FL_Left:
	case FL_Right:
	    // do nothing except skip the key...
	    handled = 1;
	    break;
	}
    if (!handled) {
	handled = 1;
	switch (key) {
	case FL_Left:
	    if (state & FL_CTRL)
		engine->Command(MOVE_LEFT_WORD);
	    else
		engine->Command(MOVE_LEFT);
	    break;
	case FL_Right:
	    if (state & FL_CTRL)
		engine->Command(MOVE_RIGHT_WORD);
	    else
		engine->Command(MOVE_RIGHT);
	    break;
	case FL_Up:
	    {
		short curx = CursorX();
		engine->Command(MOVE_UP);
		FindColumn(curx);
	    }
	    break;
	case FL_Down:
	    {
		short curx = CursorX();
		engine->Command(MOVE_DOWN);
		FindColumn(curx);
	    }
	    break;
	case FL_Home:
	    if (state & FL_CTRL)
		engine->Command(MOVE_TOF);
	    else
		engine->Command(MOVE_SOL);
	    break;
	case FL_End:
	    if (state & FL_CTRL)
		engine->Command(MOVE_BOF);
	    else
		engine->Command(MOVE_EOL);
	    break;
	case FL_Page_Up:
	    {
		StartLine -= VisibleLines;
		if (StartLine < 0)
		    StartLine = 0;
		long newrow = engine->CursorRow() - VisibleLines;
		if (newrow < 0)
		    newrow = 0;
		engine->Command(MOVE_ABSOLUTE_ROW, newrow);
	    }
	    break;
	case FL_Page_Down:
	    {
		StartLine += VisibleLines;
		if (StartLine > (engine->GetLineCount() - VisibleLines))
		    StartLine = (engine->GetLineCount() - VisibleLines);
		if (engine->GetLineCount() < VisibleLines)
		    StartLine = 0;
		long newrow = engine->CursorRow() + VisibleLines;
		if (newrow > engine->GetLineCount())
		    newrow = engine->GetLineCount();

		engine->Command(MOVE_ABSOLUTE_ROW, newrow);
	    }
	    break;
	default:
	    handled = 0;
	    break;
	}
    }
    if (handled && marking)
	engine->Command(MARK_END);
    if (!ismarking)
	engine->Command(CLEAR_MARKS);
    if (engine->CursorRow() < StartLine)
	StartLine = engine->CursorRow();
    if (engine->CursorRow() >= StartLine + VisibleLines)
	StartLine = engine->CursorRow() - (VisibleLines - 1);
    damage(DAMAGE_EDITOR);
    return (handled);
}

FL_API void
Fl_Editor::Cut()
{
    Copy();
    EditPosition edp = engine->GetPosition();
    SortPositions(Mark, edp);
    engine->Command(DELETE_REGION);
    engine->Command(CLEAR_MARKS);
    engine->MoveTo(Mark.Row, Mark.Column);
    marking = selectlines = 0;
    damage(DAMAGE_EDITOR);
}


FL_API void
Fl_Editor::CopySelection(bool clrmarks)
{
    char temp[50];
    sprintf(temp, "%p", engine);
    if (marking) {
	char *copybuffer = NULL;
	long size = engine->GetSelectionSize();
	if (size && (copybuffer = (char *) malloc(size + 1))) {
	    engine->CopyRegionTo(copybuffer);
	    //      printf("Fl_Editor size = %d, copybuffer = %s\n", size, copybuffer);
#if(FL_MAJOR_VERSION > 1)
	    Fl::copy(copybuffer, size);
#else
	    Fl::selection(*this, copybuffer, size);
#endif
	}
	//if(NULL != copybuffer)
	free(copybuffer);
	if (clrmarks)
	    marking = selectlines = 0;
    }
}

#if (FL_MAJOR_VERSION > 1)
FL_API void
Fl_Editor::textfont(Fl_Font s)
{
    textfont_ = s;
    if (engine)
	engine->textfont(s);
}
#else
FL_API void
Fl_Editor::textfont(uchar s)
{
    textfont_ = s;
    if (engine)
	engine->textfont(s);
}
#endif

FL_API void
Fl_Editor::textsize(uchar s)
{
    textsize_ = s;
    if (engine)
	engine->textsize(s);
}


FL_API void
Fl_Editor::DrawLine(long editorlineno, int startleft, int starttop,
		    wString & current, fleditor_DrawState & state)
{
    // starttop is where the BASELINE of the text is drawn.  
    // That's why "boxtop" is set back by the height of the line
    long start, end;
    short boxtop =
	(starttop - engine->MeasureTextHeight(current)) + fl_descent();
    int X = startleft;
    int Y = starttop;
    int W = (editbox->w() - (X - editbox->x())) + 2;
    setfont();
    // if line in selection, draw in selection color.  
    fl_color(textcolor());
    int selpos = engine->FindSelectPos(editorlineno, &start, &end);
    if ((current.LastCharacter() == HARD_RETURN)
	|| (current.LastCharacter() == SOFT_RETURN))
	current.LastCharacter() = '\0';	// don't print the carriage returns
    if (selpos && marking) {	// part of line is selected
	short myleft = X;
	wString sleft = current.Copy(0, start);
	wString scenter = current.Copy(start, end - start);
	wString sright = current.Copy(end, current.Length() - end);
	bool fulldraw = false;
	if (sleft.Length() || sright.Length())
	    fulldraw = true;
	else if (!scenter.Length() && (editorlineno == engine->CursorRow()))
	    fulldraw = true;
	if (fulldraw) {		// if there is anything *not* selected, draw with normal background
	    fl_color(color());
	    fl_rectf(myleft, boxtop, W, engine->MeasureTextHeight(current));
	    fl_color(textcolor());
	    // draw unselected part
	    fl_draw(sleft.Get(), myleft, Y);
	    // draw selected part
	    fl_color(selection_color());
	    myleft += (short) fl_width(sleft.Get());
	    if (sright == "")	// if rest of line selected, draw to margin
		fl_rectf(myleft, boxtop, W - (int) fl_width(sleft.Get()),
			 engine->MeasureTextHeight(current));
	    else
		fl_rectf(myleft, boxtop, (int) fl_width(scenter.Get()),
			 engine->MeasureTextHeight(current));

	    fl_color(contrast(FL_BLACK, selection_color()));
	    fl_draw(scenter.Get(), myleft, Y);
	    // draw remainder of unselected part
	    myleft += (short) fl_width(scenter.Get());
	    fl_color(textcolor());
	    fl_draw(sright.Get(), myleft, Y);
	} else {		// else if whole line is selected, draw whole line 
	    // (including blank space) in selection color
	    fl_color(selection_color());
	    fl_rectf(myleft, boxtop, W - myleft,
		     engine->MeasureTextHeight(current));
	    fl_color(contrast(FL_BLACK, selection_color()));
	    fl_draw(scenter.Get(), myleft, Y);
	}
    } else {			// none of line is selected
	fl_color(color());
	fl_rectf(X, boxtop, W, engine->MeasureTextHeight(current));
	fl_color(textcolor());
	fl_draw(current.Get(), X, Y);
    }
    fl_color(textcolor());
}

FL_API void
Fl_Editor::PushState(const fleditor_DrawState & state)
{
    fleditor_DrawState *ds =
	new fleditor_DrawState(state.fg, state.bg, state.sel, state.font,
			       state.size, state.attrib);
    if (ds) {
	ds->next = DrawState;
	DrawState = ds;
    }
}

FL_API void
Fl_Editor::PopState()
{
    if (DrawState && DrawState->next) {
	fleditor_DrawState *ds = DrawState;
	DrawState = ds->next;
	delete ds;
    }
}

FL_API void
Fl_Editor::SyncDisplay()
{
    if (engine->CursorRow() < StartLine)
	StartLine = engine->CursorRow();
    if (engine->CursorRow() >= StartLine + VisibleLines)
	StartLine = engine->CursorRow() - (VisibleLines - 1);
    damage(DAMAGE_EDITOR);
}

FL_API void
Fl_Editor::browse(bool setit)
{
    Browsemode = setit;
    Readonly = setit;
    HideCursor(setit);
    damage(DAMAGE_LINE);
};
