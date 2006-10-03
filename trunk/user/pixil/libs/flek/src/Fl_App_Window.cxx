#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>	// needed by FLTK2
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <Flek/Fl_App_Window.H>

// #define FL_APP_WINDOW_DEBUG

Fl_App_Window::Fl_App_Window(int x, int y, int w, int h, const char *l):
Fl_Window(x, y, w, h, l)
{
    create_app_window(w, h, l);
}

Fl_App_Window::Fl_App_Window(int w, int h, const char *l):
Fl_Window(w, h, l)
{
    create_app_window(w, h, l);
}

Fl_App_Window::~Fl_App_Window()
{
    free(dockable_windows);
}

void
Fl_App_Window::create_app_window(int w, int h, const char *l)
{
#ifdef FL_APP_WINDOW_DEBUG
    printf("Fl_App_Window::create_app_window win %p %dx%d\n", this, w, h);
#endif
    // Make room for the list of dockable windows.
    dockable_windows_capacity = 4;
    dockable_windows =
	(Fl_Dockable_Window **) malloc(sizeof(Fl_Dockable_Window *) *
				       dockable_windows_capacity);
    dockable_windows_size = 0;

    // Contents window and docked windows will complete obscure the
    // toplevel window, so set it to no box.
    Fl_Window::box(FL_NO_BOX);

    // Create the pack that holds the contents window and docked windows.
    // Contents window and docked windows will complete obscure the
    // the pack area, so set it to no box.
    _pack = new Fl_Pack(0, 0, w, h, "Fl_App_Window::pack");
    _pack->type(Fl_Pack::VERTICAL);
    _pack->box(FL_NO_BOX);

    // Create the contents window.
    _contents = new Fl_Window(0, 0, w, h, "Fl_App_Window::contents");

    // Contents area is the resizable for the pack group.
    // As dockables get added and removed, contents resizes.
    // Now, wouldn't that be just dandy?
    // However, Fl_Pack apparently doesn't work that way,
    // at least AFAICT at this point.
    // _pack->resizable(_contents);

    // Pack group is the resizable for the window.
    // Maybe this isn't so cool, as it makes the window resizable,
    // which maybe isn't what everyone wants.
    // Code in resize below emulates this instead.
    // resizable(_pack);

#ifdef FL_APP_WINDOW_DEBUG
    // Turn on borders so we can see what is what.
    Fl_Window::box(FL_BORDER_BOX);
    Fl_Window::color(FL_WHITE);
    _pack->box(FL_DOWN_BOX);
    _contents->box(FL_UP_BOX);
    _contents->color(FL_LIGHT3);
#endif
}

int
Fl_App_Window::handle(int event)
{

    if (event == FL_UNDOCK) {
#ifdef FL_APP_WINDOW_DEBUG
	printf("Fl_App_Window::handle - undock\n");
#endif
	repack();
	return 1;
    }

    if (event == FL_DOCK) {
	// Isn't it in this windows list of dockable widows?
	if (!may_dock(Fl_Dockable_Window::current))
	    // Don't dock it.
	    return 1;

	int dx = Fl::event_x_root();
	int dy = Fl::event_y_root();
	int ex = x_root();
	int ey = y_root();

	// Check to see if we can dock along any of 
	// the pack boundaries.

	// This code should probably go into Pack_2's handle:
//    Fl_Widget*const* a = _pack->array();
	for (int i = 0; i <= _pack->children(); i++) {
	    int cY;

	    if (i == _pack->children())
		cY = h();
	    else {
//        Fl_Widget* o = *a++;
		Fl_Widget *o = _pack->child(i);
		cY = o->y();
	    }

	    if ((dx < (ex + FL_DOCK_DELTA + w()))	// xmax
		&& (dx > (ex - FL_DOCK_DELTA))	// xmin 
		&& (dy < (ey + FL_DOCK_DELTA + cY))	// ymax
		&& (dy > (ey - FL_DOCK_DELTA + cY)))	// ymin
	    {
#ifdef FL_APP_WINDOW_DEBUG
		printf("Fl_App_Window::handle - dock\n");
#endif
		add_dockable(Fl_Dockable_Window::current, i);
		return 1;
	    }
	}
	return 0;
    }
    return Fl_Window::handle(event);
}

void
Fl_App_Window::add(Fl_Widget * w)
{
    // Anything added to Fl_App should actually get 
    // added to the contents group.
    _contents->add(w);
}

bool
Fl_App_Window::may_dock(Fl_Dockable_Window * W)
{
    // Is the dockable window in the dockable_windows list?
    for (int i = 0; i < dockable_windows_size; i++)
	if (dockable_windows[i] == W)
	    return true;
    return false;
}

void
Fl_App_Window::accept_dockable(Fl_Dockable_Window * W)
{
    // Does this dockable window already have permission to dock?
    // In other words, is it already in the list?
    if (may_dock(W))
	// Already in the list, don't add it again.
	return;

    // Is there room left in the list?
    if (dockable_windows_size >= dockable_windows_capacity) {
	// Out of space.  That's okay.  We'll make more.
	dockable_windows_capacity *= 2;
	dockable_windows = (Fl_Dockable_Window **) realloc(dockable_windows,
							   sizeof
							   (Fl_Dockable_Window
							    *) *
							   dockable_windows_capacity);
    }
    // Add it to the list.
    dockable_windows[dockable_windows_size++] = W;
}

void
Fl_App_Window::add_dockable(Fl_Dockable_Window * W, int pos)
{
    // Add the dockable window to this window's list of dockable windows.
    accept_dockable(W);

    // Dock the dockable window on this window.
    Fl_Dockable_Window::current = W;

    if (W->shown())
	W->hide();

    _pack->insert(*W, pos);
    repack();

    W->docked(1);

    if (shown())
	W->show();
}

void
Fl_App_Window::show()
{
#ifdef FL_APP_WINDOW_DEBUG
    printf("Fl_App_Window::show() win %p\n", this);
#endif

    // Show ourself.
    Fl_Window::show();

    // Show our dockable windows.
    for (int i = 0; i < dockable_windows_size; i++)
	dockable_windows[i]->show();
}

void
Fl_App_Window::hide()
{
    for (int i = 0; i < dockable_windows_size; i++)
	dockable_windows[i]->hide();
    Fl_Window::hide();
}

void
Fl_App_Window::flush()
{
#ifdef FL_APP_WINDOW_DEBUG
    printf("Fl_App_Window::flush() win %p\n", this);
#endif

    make_current();
    Fl_X *i = Fl_X::i(this);
    fl_clip_region(i->region);
    i->region = 0;
    // Based on my testing, this draw() is required.  bdl
    draw();
}

void
Fl_App_Window::repack()
{
    // Figure how much space everything but the contents takes up.
    int children_w = 0;
    int children_h = 0;
    for (int i = 0; i < _pack->children(); i++) {
	if (_pack->child(i) == _contents)
	    continue;
	// HACK: don't consider width since packing is vertical.
	// children_w += _pack->child(i)->w();
	children_h += _pack->child(i)->h();
#ifdef FL_APP_WINDOW_DEBUG
	printf("Fl_App_Window::repack win %p child %d size %dx%d\n",
	       this, i, _pack->child(i)->w(), _pack->child(i)->h());
#endif
    }

    // Make contents fill all remaining space.
    _contents->resize(0, 0, w() - children_w, h() - children_h);

    // Resize the pack to fill the window.
    _pack->resize(0, 0, w(), h());

#ifdef FL_APP_WINDOW_DEBUG
    printf("Fl_App_Window::repack win %p pack %dx%d contents %dx%d\n",
	   this, w(), h(), w() - children_w, h() - children_h);
#endif
}

void
Fl_App_Window::resize(int X, int Y, int W, int H)
{
#ifdef FL_APP_WINDOW_DEBUG
    printf("Fl_App_Window::resize() win %p %dx%d+%d+%d\n", this, W, H, X, Y);
#endif
    Fl_Widget::resize(X, Y, W, H);
    repack();
}

#if FL_MAJOR_VERSION == 2
void
Fl_App_Window::layout()
{
    printf("layout called\n");
    printf("1AppWin- x:%d y:%d w:%d h:%d\n", x(), y(), w(), h());
    printf("1pack- x:%d y:%d w:%d h:%d\n", _pack->x(), _pack->y(), _pack->w(),
	   _pack->h());
    printf("1contents- x:%d y:%d w:%d h:%d\n", _contents->x(), _contents->y(),
	   _contents->w(), _contents->h());

    repack();

    printf("2AppWin- x:%d y:%d w:%d h:%d\n", x(), y(), w(), h());
    printf("2pack- x:%d y:%d w:%d h:%d\n", _pack->x(), _pack->y(), _pack->w(),
	   _pack->h());
    printf("2contents- x:%d y:%d w:%d h:%d\n", _contents->x(), _contents->y(),
	   _contents->w(), _contents->h());
}
#endif
