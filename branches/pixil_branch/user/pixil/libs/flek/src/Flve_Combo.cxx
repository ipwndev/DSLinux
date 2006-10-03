//      ======================================================================
//      File:    Flve_Combo.cxx - Flve_Combo implementation
//      Library: flvw - FLTK Virtual widget library
//      Version: 0.1.0
//      Started: 01/12/2000
//
//      Copyright (C) 1999 Laurence Charlton
//
//      Description:
//      Flve_Combo implements combo box functionality
//      Included are select from list, text with list, incremental search
//      ======================================================================

#include <FL/Fl.H>
#include <Flek/Flve_Combo.H>
#include <FL/fl_draw.H>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if FL_MAJOR_VERSION == 1
#define text_color() FL_BLACK
#define text_font() FL_HELVETICA
#define text_size() 12
#define fl_contrast(x,y) FL_BLACK
#define selection_text_color() FL_WHITE
#define FL_DAMAGE_HIGHLIGHT FL_DAMAGE_CHILD
#define label_font labelfont
#define label_size labelsize
#define label_color labelcolor
#define label_type labeltype
#endif

#define ADDSIZE 10
#define BUTTON_WIDTH 17

#ifdef WIN32
#define STRCASECMP  stricmp
#define STRNCASECMP strncmpi
#else
#define STRCASECMP  strcasecmp
#define STRNCASECMP strncasecmp
#endif

char *
get_value(int R)
{
    static char buf[20];
    sprintf(buf, "%d", R);
    return buf;
}

class Flvl_Drop:public Flv_List
{
  public:
    int last_row;
      Flvl_Drop(int x, int y, int w, int h, const char *l =
		0):Flv_List(x, y, w, h, l)
    {
	combo = 0;
	last_row = -1;
    };
    Flve_Combo *combo;
  protected:
    void draw_row(int Offset, int &X, int &Y, int &W, int &H, int R);
};

class Flvw_Drop:public Fl_Window
{
  public:
    int key;
    Flvl_Drop *drop_list;
    Flve_Combo *combo;
    Fl_Widget *pushed;

      Flvw_Drop(int w, int h, const char *l = 0);
     ~Flvw_Drop();
  protected:
    int handle(int event);
};

class Flvt_Input:public Fl_Input
{
  public:
    Flvt_Input(int x, int y, int w, int h, const char *l =
	       0):Fl_Input(x, y, w, h, l)
    {
    };
  protected:
    int handle(int event);
    void draw(void);
};

Flvw_Drop::Flvw_Drop(int w, int h, const char *l):
Fl_Window(w, h, l)
{
    drop_list = new Flvl_Drop(0, 0, w, h);
    ((Flvl_Drop *) drop_list)->has_scrollbar(FLVS_VERTICAL);
    pushed = 0;
}

Flvw_Drop::~Flvw_Drop()
{
    if (drop_list)
	delete drop_list;
}

int
Flvw_Drop::handle(int event)
{
    int ex, ey, stat, r;

    ex = Fl::event_x();
    ey = Fl::event_y();
    r = drop_list->row();
    switch (event) {
    case FL_PUSH:
	if (ex < 0 || ex > w() || ey < 0 || ey > h()) {
	    key = 0;
	    hide();
	    return 1;
	}
	break;
    case FL_KEYBOARD:
	stat = Fl::event_key();
	if (Fl::event_ctrl())
	    stat = flv_ctrl(stat);
	if (Fl::event_alt())
	    stat = flv_alt(stat);
	if (Fl::event_shift())
	    stat = flv_shift(stat);
	if (stat == combo->drop_key()) {
	    key = 0;
	    hide();
	    return 1;
	}
	switch (Fl::event_key()) {
	    // modified behavior of escape key to restore original value
	    // of the input widget.
	    // TAB and Enter both accept the new value
	    // D.Freese (dfreese@intrepid.net)
	case FL_Escape:
	    key = 0;
	    hide();
	    return 1;
	case FL_Enter:
	case FL_Tab:
	    combo->item.index(drop_list->row());
	    key = Fl::event_key();
	    //if (key==FL_Escape)
	    //      key = 0;
	    hide();
	    return 1;
	}
	break;
    }
    if (pushed && (event == FL_DRAG || event == FL_RELEASE) &&
	contains(pushed) && pushed != this)
	stat = pushed->handle(event);
    else
	stat = ((Fl_Widget *) drop_list)->handle(event);

    if (!stat && event == FL_KEYBOARD)
	stat = ((Fl_Widget *) combo)->handle(event);

    pushed = Fl::pushed();
    if (event == FL_PUSH && r == drop_list->row() && pushed == this) {
	combo->item.index(drop_list->row());
	key = 0;
	hide();
	return 1;
    }
    return stat;
}

void
Flvl_Drop::draw_row(int Offset, int &X, int &Y, int &W, int &H, int R)
{
    Flv_Style s;

    get_style(s, R);
    Flv_List::draw_row(Offset, X, Y, W, H, R);
    fl_draw(combo->item[R].item(), X - Offset, Y, W, H, s.align());
    if (last_row != row()) {
	combo->value(combo->item[row()].item());
	last_row = row();
    }
}

void
Flvt_Input::draw(void)
{

    Fl_Widget *f = Fl::focus();
    //      Kludge so that events don't get fired all over the place
    //      We just want the input to draw correctly as though it had the
    //      focus.
    if (f && f != parent())
	f = NULL;
    if (f)
	Fl::focus_ = this;
    Fl_Input::draw();
    if (f)
	Fl::focus_ = f;
}

int
Flvt_Input::handle(int event)
{
    int stat, t, i = 0;
    Flve_Combo *w;

    w = (Flve_Combo *) parent();
    if (w)
	i = w->item.index();
    switch (event) {
    case FL_FOCUS:
    case FL_UNFOCUS:
	Fl_Input::handle(event);
	return 0;
    }
    t = (position() == mark());
    stat = Fl_Input::handle(event);
    if (event != FL_KEYBOARD)
	return stat;

    switch (Fl::event_key()) {
    case FL_BackSpace:
	//        If nothing was selected, Backspace already worked
	if (t)
	    break;
	Fl_Input::handle(event);
	break;
    case FL_Pause:
    case FL_Scroll_Lock:
    case FL_Num_Lock:
    case FL_Caps_Lock:
    case FL_Shift_L:
    case FL_Shift_R:
    case FL_Control_L:
    case FL_Control_R:
    case FL_Meta_L:
    case FL_Meta_R:
    case FL_Alt_L:
    case FL_Alt_R:
	return stat;
    }

    //printf("At this point, my value is [%s]\n", value());

    if (!w) {
#ifdef PIXIL
	Fl_Input::redraw();
#endif
	return stat;
    }
    if (!w->incremental_search()) {
#ifdef PIXIL
	Fl_Input::redraw();
#endif
	return stat;
    }

    if (w->item.findi(value()) > -1) {
	t = position();
	if (w->list) {
	    ((Flvw_Drop *) w->list)->drop_list->row(w->item.index());
	    ((Flvw_Drop *) w->list)->drop_list->last_row =
		((Flvw_Drop *) w->list)->drop_list->row();
	}
	value(w->item[w->item.index()].item());
	position(t + 1, size());

    } else if (w->list_only()) {
	t = position();
	value(w->item[i].item());
	position((t ? t - 1 : t), size());
    }
#ifdef PIXIL
    Fl_Input::redraw();
#endif
    return stat;
}

Flve_Combo::Flve_Combo(int x, int y, int w, int h, const char *l):
Fl_Widget(x, y, w, h, l)
{
    vlist_only = true;
    vincremental_search = true;
    vlist_title = 0;
    Fl_Group *gsave = Fl_Group::current();
    Fl_Group::current(0);
    input = new Flvt_Input(x, y, w, h);
    Fl_Group::current(gsave);
    box(input->box());
    input->box(FL_NO_BOX);
    resize(x, y, w, h);
    color((Fl_Color) (input->color()));
    vdisplay_rows = 8;
    vdrop_key = flv_ctrl('E');	//      Control+E
}

Flve_Combo::~Flve_Combo()
{
    if (vlist_title)
	delete vlist_title;
}

bool
Flve_Combo::list_only(bool v)
{
    if (v != vlist_only) {
	vlist_only = v;
	if (v) {
	    input->value(item[item.index()].item());
	    input->position(0, input->size());
	    input->set_changed();
	}
    }
    return vlist_only;
}

void
Flve_Combo::list_title(const char *v)
{
    if (vlist_title)
	delete vlist_title;
    vlist_title = 0;
    if (v) {
	vlist_title = new char[strlen(v) + 1];
	strcpy(vlist_title, v);
    }
}

void
Flve_Combo::resize(int X, int Y, int W, int H)
{
    Fl_Widget::resize(X, Y, W, H);
    X += (Fl::box_dx(box()));
    Y += (Fl::box_dy(box()));
    W -= (Fl::box_dw(box()));
    H -= (Fl::box_dh(box()));
    W -= BUTTON_WIDTH;

    input->resize(X, Y, W, H);
}

int
Flve_Combo::handle(int event)
{
    int stat, ex, ey, X, Y, W, H;
    Fl_Widget *wid;

    switch (event) {
    case FL_UNFOCUS:
	if (Fl::focus() == input)
	    take_focus();
	redraw();
    case FL_FOCUS:
	input->handle(event);
//                      input->position(0, input->size() );
	return 1;

    case FL_KEYBOARD:
	X = Fl::event_key();
	if (Fl::event_ctrl())
	    X = flv_ctrl(X);
	if (Fl::event_alt())
	    X = flv_alt(X);
	if (Fl::event_shift())
	    X = flv_shift(X);
	if (X == vdrop_key) {
	    open_list();
	    return 1;
	}
#ifdef PIXIL
	//      Kludge to make input think it's focused
	wid = Fl::focus_;
	Fl::focus_ = input;
	stat = input->handle(event);
	input->redraw();

	Fl::focus_ = wid;
#endif

	//      Shouldn't be doing searching if the value couldn't have changed!

	break;

    case FL_PUSH:
	//      Test for button push
	ex = Fl::event_x();
	ey = Fl::event_y();
	X = x() + w() - BUTTON_WIDTH;
	Y = y();
	W = BUTTON_WIDTH;
	H = h();
	if (ex >= X && ex < X + W && ey >= Y && ey <= Y + H) {
	    open_list();
	    return 1;
	}
	return input->handle(event);
    default:
	wid = Fl::focus_;
	Fl::focus_ = input;
	stat = input->handle(event);
	Fl::focus_ = wid;
    }
    return stat;
}

void
Flve_Combo::draw(void)
{

    int X, Y, W, H, bx;

    // modified to allow a zero length string string in input widget
    //  if input widget is modifiable

    if (list_only() == true)
	if (*(input->value()) == 0)
	    input->value(item[item.index()].item());

    if ((damage() & FL_DAMAGE_ALL) != 0)
	draw_box();

    X = x() + (Fl::box_dx(box()));
    Y = y() + (Fl::box_dy(box()));
    W = w() - (Fl::box_dw(box()));
    H = h() - (Fl::box_dh(box()));

    bx = X + W - BUTTON_WIDTH;

#if FL_MAJOR_VERSION == 2
    FL_UP_BOX->draw(bx + 1, Y, BUTTON_WIDTH, H,
		    (Fl_Color) (FL_GRAY_RAMP + 17));
#else
    draw_box(FL_UP_BOX, bx + 1, Y, BUTTON_WIDTH, H,
	     (Fl_Color) (FL_GRAY_RAMP + 17));
#endif
    fl_draw_symbol("@#2>", bx + 3, Y, BUTTON_WIDTH - 5, H, FL_BLACK);
    fl_color((Fl_Color) (FL_GRAY_RAMP + 17));
    fl_yxline(bx, Y, Y + H);
    input->draw();
}

void
Flve_Combo::open_list()
{
    int W, H, r;
    Fl_Window *win;
    Flvw_Drop *lst;

    fl_font(text_font(), text_size());
    fl_measure("X", W, H);

    r = item.count();
    if (vlist_title)
	r++;
    if (r > display_rows())
	r = display_rows();
    list = lst = new Flvw_Drop(w(), H * r + 4 + (vlist_title ? 4 : 0));
    list->box(FL_DOWN_BOX);
    list->end();

    // This is a horrible hack .. This should have a parent of the root to allow
    // it to pop up over the application window, instead of being contained
    // within it. However, the screentop and window managers pick up this window
    // and do Bad Things (TM) with it. Therefore, I've made the temporary change 
    // to make this a child of the application, and thus, it may look weird.

    // This need's fixed.

    // -Anonymous (anon@fixthisyourself.net)

    list->parent(parent());
    //      list->parent(NULL);
    lst->drop_list->rows(item.count());
    if (vlist_title)
	lst->drop_list->label(vlist_title);
    list->clear_border();
    list->set_modal();
    lst->drop_list->combo = this;
    lst->combo = this;

    // highlight the present contents of the input widget
    // if input contains a value and it can be found in the list
    // added by Dave Freese (dfreese@intrepid.net)

    if (item.findi(input->value()) > -1)
	lst->drop_list->row(item.index());
    else
	lst->drop_list->row(0);
    lst->drop_list->last_row = lst->drop_list->row();

    // end hightlight code

    win = window();
    int nx = win->x();
    int ny = win->y();

    while (win && win->parent()) {
	win = win->window();
	nx += win->x();
	ny += win->y();
    }
    if (win)
	list->position(nx + x(), ny + y() + h() - 3);

    Fl::grab(list);
    list->show();
    while (list->shown())
	Fl::wait();
    Fl::release();
    take_focus();
    // ****
//  if (((Flvw_Drop *)list)->key)
    value(item[item.index()].item());
    do_callback();

    if (win && ((Flvw_Drop *) list)->key)
	Fl::handle(FL_KEYBOARD, win);

    delete list;
    list = 0;
}

const char *
Flve_Combo::value()
{
    return input->value();
}

void
Flve_Combo::value(const char *v)
{
    input->value(v);
    input->set_changed();
    input->mark(256);
//      input->position(0, input->size() );
}

void
Flve_Combo::display_rows(int v)
{
    vdisplay_rows = v;
}

//      =================================================================
//      Flv_Combo_Item
//      =================================================================
Flv_Combo_Item::Flv_Combo_Item()
{
    vitem = 0;
    vvalue = 0;
}

Flv_Combo_Item::~Flv_Combo_Item()
{
    if (vitem)
	delete vitem;
}

const char *
Flv_Combo_Item::item(void)
{
    return (vitem ? vitem : "");
}

void
Flv_Combo_Item::item(const char *v)
{
    if (vitem)
	delete vitem;
    vitem = 0;
    if (v) {
	vitem = new char[strlen(v) + 1];
	strcpy(vitem, v);
    }
}

long
Flv_Combo_Item::value(void)
{
    return vvalue;
}

void
Flv_Combo_Item::value(long v)
{
    vvalue = v;
}

//      =================================================================
//      Flv_Combo_Items
//      =================================================================

Flv_Combo_Items::Flv_Combo_Items()
{
    list = 0;
    vcount = 0;
    vallocated = 0;
    vcurrent = 0;
    nodups = true;
}

Flv_Combo_Items::~Flv_Combo_Items()
{
    clear();
}

void
Flv_Combo_Items::add(const char *item, long v)
{
    Flv_Combo_Item *i;

    if (vcount == vallocated)
	make_room_for(vallocated + 10);
    if (vcount == vallocated)
	return;
    if (strlen(item) == 0)
	return;
    if (nodups)
	if (find(item) != -1)
	    return;

    i = new Flv_Combo_Item();
    i->item(item);
    i->value(v);
    list[vcount++] = i;
}

void
Flv_Combo_Items::insert(int index, const char *item, long v)
{
    int t;
    Flv_Combo_Item *i;

    if (vcount == vallocated)
	make_room_for(vallocated + 10);
    if (vcount == vallocated)
	return;
    if (strlen(item) == 0)
	return;
    if (nodups)
	if (find(item) != -1)
	    return;

    i = new Flv_Combo_Item();
    i->item(item);
    i->value(v);

    if (index < 0)
	index = 0;
    if (index > vcount)
	index = vcount;
    for (t = vcount; t > index; t--)
	list[t] = list[t - 1];
    list[index] = i;
    vcount++;
}

void
Flv_Combo_Items::remove(int index)
{
    int t;

    if (index < 0 || index >= vcount)
	return;
    if (list[index])
	delete list[index];
    for (t = index; t < vcount - 1; t++)
	list[t] = list[t + 1];
    list[t] = 0;
    vcount--;
    if (vcurrent >= vcount && vcurrent)
	vcurrent--;
}

void
Flv_Combo_Items::change(int i, const char *item, long v)
{
    if (i < 0 || i > vcount)
	return;
    list[i]->item(item);
    list[i]->value(v);
}

void
Flv_Combo_Items::change(int i, const char *item)
{
    if (i < 0 || i > vcount)
	return;
    list[i]->item(item);
}

void
Flv_Combo_Items::change(int i, long v)
{
    if (i < 0 || i > vcount)
	return;
    list[i]->value(v);
}

#define C(x) ((Flv_Combo_Item *)(x))

static int
cmp(const void *a, const void *b)
{
    Flv_Combo_Item *ai = *((Flv_Combo_Item **) (a)),
	*bi = *((Flv_Combo_Item **) (b));
    const char *a_item = ai->item(), *b_item = bi->item();
    long a_value = ai->value(), b_value = bi->value();

    int status = STRCASECMP(a_item, b_item);
    if (status)
	return status;
    return (a_value - b_value);
}

void
Flv_Combo_Items::sort(void)	//      Sort list
{
    if (list && vcount > 1)
	qsort(list, vcount, sizeof(Flv_Combo_Item *), cmp);
}

void
Flv_Combo_Items::clear(void)	//      Clear list
{
    int t;
    for (t = 0; t < vcount; t++)
	if (list[t])
	    delete list[t];
    if (list)
	delete[]list;
    list = 0;
    vallocated = 0;
    vcount = 0;
    vcurrent = 0;
}

void
Flv_Combo_Items::index(int i)	//      Set current index
{
    if (i < 0 || i >= vcount)
	return;
    vcurrent = i;
}

int
Flv_Combo_Items::findi(const char *v)	//      Find text return index (-1 not found)
{
    int t;
    // added for case where v is zero length
    if (*v == 0)
	return -1;
    for (t = 0; t < vcount; t++) {
	if (STRNCASECMP(list[t]->item(), v, strlen(v)) == 0) {
	    vcurrent = t;
	    return t;
	}
    }
    return -1;
}

int
Flv_Combo_Items::find(const char *v)	//      Find text return index (-1 not found)
{
    int t;
    // added for case where v is zero length
    if (*v == 0)
	return -1;
    for (t = 0; t < vcount; t++) {
	if (STRCASECMP(list[t]->item(), v) == 0) {
	    vcurrent = t;
	    return t;
	}
    }
    return -1;
}

int
Flv_Combo_Items::find(long v)	//      Find value return index (-1 not found)
{
    int t;
    for (t = 0; t < vcount; t++) {
	if (list[t]->value() == v) {
	    vcurrent = t;
	    return t;
	}
    }
    return -1;
}


Flv_Combo_Item *
Flv_Combo_Items::current(void)
{
    if (list)
	return list[vcurrent];
    return NULL;
}

Flv_Combo_Item & Flv_Combo_Items::operator[](int index) {
    static Flv_Combo_Item
	x;
    if (index < 0 || index >= vcount)
	return x;
    return *(list[index]);
}

void
Flv_Combo_Items::make_room_for(int n)
{
    if (n >= vallocated) {
	Flv_Combo_Item **a = new Flv_Combo_Item *[n];
	if (!a)
	    return;
	//      Wasted CPU cycles, but list is pretty
	memset(a, 0, sizeof(Flv_Style *) * (vallocated + ADDSIZE));
	if (vcount)
	    memcpy(a, list, sizeof(Flv_Combo_Item *) * vcount);
	vallocated += ADDSIZE;
	if (list)
	    delete[]list;
	list = a;
    }
}
